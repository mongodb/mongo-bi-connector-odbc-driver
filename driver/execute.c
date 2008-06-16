/*
  Copyright (C) 1995-2007 MySQL AB

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  There are special exceptions to the terms and conditions of the GPL
  as it is applied to this software. View the full text of the exception
  in file LICENSE.exceptions in the top-level directory of this software
  distribution.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/**
  @file  execute.c
  @brief Statement execution functions.
*/

#include "driver.h"
#include <locale.h>

/*
  @type    : myodbc3 internal
  @purpose : internal function to execute query and return result
  frees query if query != stmt->query
*/

SQLRETURN do_query(STMT FAR *stmt,char *query)
{
    int error= SQL_ERROR;

    if ( !query )
        return error;       /* Probably error from insert_param */

    if (stmt->stmt_options.max_rows &&
        stmt->stmt_options.max_rows != (SQLULEN)~0L)
    {
        /* Add limit to select statement */
        char *pos,*tmp_buffer;
        for ( pos= query; isspace(*pos) ; pos++ ) ;
        if ( !myodbc_casecmp(pos,"select",6) )
        {
            uint length= strlen(pos);
            if ( (tmp_buffer= my_malloc(length+30,MYF(0))) )
            {
                memcpy(tmp_buffer,pos,length);
                sprintf(tmp_buffer+length, " limit %lu",
                        (unsigned long)stmt->stmt_options.max_rows);
                if ( query != stmt->query )
                    my_free(query,MYF(0));
                query= tmp_buffer;
            }
        }
    }
    MYLOG_QUERY(stmt, query);
    pthread_mutex_lock(&stmt->dbc->lock);
    if ( check_if_server_is_alive( stmt->dbc ) )
    {
        set_stmt_error( stmt, "08S01" /* "HYT00" */, mysql_error( &stmt->dbc->mysql ), mysql_errno( &stmt->dbc->mysql ) );
        translate_error( stmt->error.sqlstate,MYERR_08S01 /* S1000 */, mysql_errno( &stmt->dbc->mysql ) );
        goto exit;
    }

    if ( mysql_query(&stmt->dbc->mysql,query) )
    {
        set_stmt_error(stmt,"HY000",mysql_error(&stmt->dbc->mysql),
                       mysql_errno(&stmt->dbc->mysql));

        translate_error(stmt->error.sqlstate,MYERR_S1000,
                        mysql_errno(&stmt->dbc->mysql));
        goto exit;
    }


    /* We can't use USE_RESULT because SQLRowCount will fail in this case! */
    if ( if_forward_cache(stmt) )
        stmt->result= mysql_use_result(&stmt->dbc->mysql);
    else
        stmt->result= mysql_store_result(&stmt->dbc->mysql);
    if ( !stmt->result )
    {
        if ( !mysql_field_count(&stmt->dbc->mysql) )
        {
            error= SQL_SUCCESS;     /* no result set */
            stmt->state= ST_EXECUTED;
            stmt->affected_rows= mysql_affected_rows(&stmt->dbc->mysql);
            goto exit;
        }
        set_error(stmt,MYERR_S1000,mysql_error(&stmt->dbc->mysql),
                  mysql_errno(&stmt->dbc->mysql));
        goto exit;
    }
    fix_result_types(stmt);
    error= SQL_SUCCESS;

    exit:
    pthread_mutex_unlock(&stmt->dbc->lock);
    if ( query != stmt->query )
        my_free(query,MYF(0));

    /*
      If the original query was modified, we reset stmt->query so that the
      next execution re-starts with the original query.
    */
    if (stmt->orig_query)
    {
        my_free(stmt->query,MYF(0));
        stmt->query= stmt->orig_query;
        stmt->query_end= stmt->orig_query_end;
        stmt->orig_query= NULL;
    }

    return error;
}



/*
  @type    : myodbc3 internal
  @purpose : help function to enlarge buffer if necessary
*/

char *extend_buffer(NET *net, char *to, ulong length)
{
    ulong need= 0;

    need= (ulong)(to - (char *)net->buff) + length;
    if (!to || need > net->max_packet - 10)
    {
        if (net_realloc(net, need))
        {
            return 0;
        }

        to= (char *)net->buff + need - length;
    }
    return to;
}


/*
  @type    : myodbc3 internal
  @purpose : help function to extend the buffer and copy the data
*/

char *add_to_buffer(NET *net,char *to,const char *from,ulong length)
{
    if ( !(to= extend_buffer(net,to,length)) )
        return 0;

    memcpy(to,from,length);

    return to+length;
}


/*
  @type    : myodbc3 internal
  @purpose : insert sql params at parameter positions
*/

SQLRETURN insert_params(STMT FAR *stmt, char **finalquery)
{
    char *query= stmt->query,*to;
    uint i,length;
    NET *net;
    SQLRETURN rc= SQL_SUCCESS;

    pthread_mutex_lock(&stmt->dbc->lock);
    net= &stmt->dbc->mysql.net;
    to= (char*) net->buff;
    if ( !(stmt->dbc->flag & FLAG_NO_LOCALE) )
      setlocale(LC_NUMERIC, "C");  /* force use of '.' as decimal point */
    for ( i= 0; i < stmt->param_count; i++ )
    {
        DESCREC *aprec= desc_get_rec(stmt->apd, i, FALSE);
        DESCREC *iprec= desc_get_rec(stmt->ipd, i, FALSE);
        char *pos;

        assert(aprec && iprec);

        if (stmt->dummy_state != ST_DUMMY_PREPARED &&
            !aprec->par.real_param_done)
        {
            rc= set_error(stmt,MYERR_07001,NULL,0);
            goto error;
        }
        get_dynamic(&stmt->param_pos, (void *)&pos, i);
        length= (uint) (pos-query);
        if ( !(to= add_to_buffer(net,to,query,length)) )
            goto memerror;
        query= pos+1;  /* Skip '?' */
        if (!SQL_SUCCEEDED(rc= insert_param(stmt,&to,aprec,iprec,0)))
            goto error;
    }
    length= (uint) (stmt->query_end - query);
    if ( !(to= add_to_buffer(net,to,query,length+1)) )
        goto memerror;
    if ( !(to= (char*) my_memdup((char*) net->buff,
                                 (uint) (to - (char*) net->buff),MYF(0))) )
        goto memerror;

    /* TODO We don't *yet* support PARAMSET */
    if ( stmt->apd->rows_processed_ptr )
        *stmt->apd->rows_processed_ptr= 1;

    pthread_mutex_unlock(&stmt->dbc->lock);
    if ( !(stmt->dbc->flag & FLAG_NO_LOCALE) )
        setlocale(LC_NUMERIC,default_locale);
    *finalquery= to;
    return rc;

memerror:      /* Too much data */
    rc= set_error(stmt,MYERR_S1001,NULL,4001);
error:
    pthread_mutex_unlock(&stmt->dbc->lock);
    if ( !(stmt->dbc->flag & FLAG_NO_LOCALE) )
        setlocale(LC_NUMERIC,default_locale);
    return rc;
}


/*
  Add the value of parameter to a string buffer.

  @param[in]      mysql
  @param[in,out]  toptr
  @param[in]      aprec The APD record of the parameter
  @param[in]      iprec The IPD record of the parameter
*/
SQLRETURN insert_param(STMT *stmt, char **toptr, DESCREC *aprec, DESCREC *iprec,
                       SQLULEN row)
{
    int length;
    char buff[128], *data= NULL;
    my_bool convert= FALSE, free_data= FALSE;
    DBC *dbc= stmt->dbc;
    NET *net= &dbc->mysql.net;
    SQLLEN *octet_length_ptr= NULL;
    char *to= *toptr;

    if (aprec->octet_length_ptr)
    {
      octet_length_ptr= ptr_offset_adjust(aprec->octet_length_ptr,
                                          stmt->apd->bind_offset_ptr,
                                          stmt->apd->bind_type,
                                          sizeof(SQLLEN), row);
      length= *octet_length_ptr;
    }

    if (aprec->data_ptr)
    {
      SQLINTEGER default_size= bind_length(aprec->concise_type,
                                           aprec->octet_length);
      data= ptr_offset_adjust(aprec->data_ptr, stmt->apd->bind_offset_ptr,
                              stmt->apd->bind_type, default_size, row);
    }

    if (!octet_length_ptr || *octet_length_ptr == SQL_NTS)
    {
      if (data)
      {
        if (aprec->concise_type == SQL_C_WCHAR)
          length= sqlwcharlen((SQLWCHAR *)data) * sizeof(SQLWCHAR);
        else /* TODO this is stupid, check condition above, shouldn't we be checking only octet_length, not ptr? */
          length= strlen(data);

        if (!octet_length_ptr && aprec->octet_length > 0 &&
            aprec->octet_length != SQL_SETPARAM_VALUE_MAX)
          length= myodbc_min(length, aprec->octet_length);
      }
      else
      {
        length= 0;     /* TODO? This is actually an error */
      }
    }
    else if ( *octet_length_ptr == SQL_NULL_DATA )
    {
        *toptr= add_to_buffer(net,*toptr,"NULL",4);
        return SQL_SUCCESS;
    }
    /*
      We may see SQL_COLUMN_IGNORE from bulk INSERT operations, where we
      may have been told to ignore a column in one particular row. So we
      try to insert DEFAULT, or NULL for really old servers.

      In case there are less parameters than result columns we have to
      insert NULL or DEFAULT.
    */
    else if (*octet_length_ptr == SQL_COLUMN_IGNORE ||
             /* empty values mean it's an unbound column */
             (*octet_length_ptr == 0 &&
              aprec->concise_type == SQL_C_DEFAULT &&
              aprec->par.value == NULL))
    {
      if (is_minimum_version(dbc->mysql.server_version, "4.0.3", 5))
        *toptr= add_to_buffer(net,*toptr,"DEFAULT",7);
      else
        *toptr= add_to_buffer(net,*toptr,"NULL",4);
      return SQL_SUCCESS;
    }
    else if (IS_DATA_AT_EXEC(octet_length_ptr))
    {
        length= aprec->par.value_length;
        if ( !(data= aprec->par.value) )
        {
            *toptr= add_to_buffer(net,*toptr,"NULL",4);
            return SQL_SUCCESS;
        }
    }

    switch ( aprec->concise_type )
    {
        case SQL_C_BINARY:
        case SQL_C_CHAR:
            convert= 1;
            break;

        case SQL_C_WCHAR:
          {
            /* Convert SQL_C_WCHAR (utf-16 or utf-32) to utf-8. */
            char *to;
            int i= 0;
            int utf8len, has_utf8_maxlen4= 0;

            /* length is in bytes, we want chars */
            length= length / sizeof(SQLWCHAR);

            /* Use buff if it is big enough, otherwise alloc some space. */
            if (sizeof(buff) >= (size_t)length * 4)
              to= buff;
            else
            {
              if (!(to= (char *)my_malloc(length * 4, MYF(0))))
                goto memerror;
              free_data= TRUE;
            }

            if (sizeof(SQLWCHAR) == 4)
            {
              UTF32 *in= (UTF32 *)data;
              data= to;
              while (i < length)
              {
                to+= (utf8len= utf32toutf8(in[i++], (UTF8 *)to));
                if (utf8len == 4)
                  has_utf8_maxlen4= 1;
              }
            }
            else
            {
              UTF16 *in= (UTF16 *)data;
              data= to;
              while (i < length)
              {
                UTF32 c;
                i+= utf16toutf32(in + i, &c);
                to+= (utf8len= utf32toutf8(c, (UTF8 *)to));
                if (utf8len == 4)
                  has_utf8_maxlen4= 1;
              }
            }

            if (has_utf8_maxlen4 &&
                !is_minimum_version(dbc->mysql.server_version, "6.0.4", 5))
              return set_stmt_error(stmt, "HY000",
                                    "Server does not support 4-byte encoded "
                                    "UTF8 characters.", 0);

            length= to - data;

            break;
          }

        case SQL_C_BIT:
        case SQL_C_TINYINT:
        case SQL_C_STINYINT:
            length= my_int2str((long)*((signed char *)data),buff,-10,0) -buff;
            data= buff;
            break;
        case SQL_C_UTINYINT:
            length= my_int2str((long)*((unsigned char *)data),buff,-10,0) -buff;
            data= buff;
            break;
        case SQL_C_SHORT:
        case SQL_C_SSHORT:
            length= my_int2str((long)*((short int *)data),buff,-10,0) -buff;
            data= buff;
            break;
        case SQL_C_USHORT:
            length= my_int2str((long)*((unsigned short int *)data),buff,-10,0) -buff;
            data= buff;
            break;
        case SQL_C_LONG:
        case SQL_C_SLONG:
            length= my_int2str(*((SQLINTEGER*) data),buff,-10,0) -buff;
            data= buff;
            break;
        case SQL_C_ULONG:
            length= my_int2str(*((SQLUINTEGER*) data),buff,10,0) -buff;
            data= buff;
            break;
        case SQL_C_SBIGINT:
            length= longlong2str(*((longlong*) data),buff, -10) - buff;
            data= buff;
            break;
        case SQL_C_UBIGINT:
            length= longlong2str(*((ulonglong*) data),buff, 10) - buff;
            data= buff;
            break;
        case SQL_C_FLOAT:
            if ( iprec->concise_type != SQL_NUMERIC && iprec->concise_type != SQL_DECIMAL )
                sprintf(buff,"%.17e",*((float*) data));
            else
                /* We should perpare this data for string comparison */
                sprintf(buff,"%.15e",*((float*) data));
            length= strlen(data= buff);
            break;
        case SQL_C_DOUBLE:
            if ( iprec->concise_type != SQL_NUMERIC && iprec->concise_type != SQL_DECIMAL )
                sprintf(buff,"%.17e",*((double*) data));
            else
                /* We should perpare this data for string comparison */
                sprintf(buff,"%.15e",*((double*) data));
            length= strlen(data= buff);
            break;
        case SQL_C_DATE:
        case SQL_C_TYPE_DATE:
            {
                DATE_STRUCT *date= (DATE_STRUCT*) data;
                if ((dbc->flag & FLAG_MIN_DATE_TO_ZERO) &&
                    !date->year && (date->month == date->day == 1))
                  sprintf(buff, "0000-00-00");
                else
                  sprintf(buff, "%04d-%02d-%02d",
                          date->year, date->month, date->day);
                data= buff;
                length= 10;
                break;
            }
        case SQL_C_TIME:
        case SQL_C_TYPE_TIME:
            {
                TIME_STRUCT *time= (TIME_STRUCT*) data;
                sprintf(buff, "%02d:%02d:%02d",
                        time->hour, time->minute, time->second);
                data= buff;
                length= 8;
                break;
            }
        case SQL_C_TIMESTAMP:
        case SQL_C_TYPE_TIMESTAMP:
            {
                TIMESTAMP_STRUCT *time= (TIMESTAMP_STRUCT*) data;
                if ((dbc->flag & FLAG_MIN_DATE_TO_ZERO) &&
                    !time->year && (time->month == time->day == 1))
                  sprintf(buff, "0000-00-00 %02d:%02d:%02d",
                          time->hour, time->minute, time->second);
                else
                  sprintf(buff, "%04d-%02d-%02d %02d:%02d:%02d",
                          time->year, time->month, time->day,
                          time->hour, time->minute, time->second);
                data= buff;
                length= 19;
                break;
            }
        case SQL_C_NUMERIC:
            {
              int trunc;
              SQL_NUMERIC_STRUCT *sqlnum= (SQL_NUMERIC_STRUCT *) data;
              sqlnum_to_str(sqlnum, (SQLCHAR *)(buff + sizeof(buff) - 1),
                            (SQLCHAR **) &data,
                            (SQLCHAR) iprec->precision,
                            (SQLSCHAR) iprec->scale, &trunc);
              length= strlen(data);
              /* TODO no way to return an error here? */
              if (trunc == SQLNUM_TRUNC_FRAC)
              {/* 01S07 SQL_SUCCESS_WITH_INFO */}
              else if (trunc == SQLNUM_TRUNC_WHOLE)
              {/* 22003 SQL_ERROR */
                return SQL_ERROR;
              }
            }
    }
    switch ( iprec->concise_type )
    {
        case SQL_DATE:
        case SQL_TYPE_DATE:
        case SQL_TYPE_TIMESTAMP:
        case SQL_TIMESTAMP:
            if (data[0] == '{')       /* Of type {d date } */
            {
              to= add_to_buffer(net, to, data, length);
              goto out;
            }
            /* else treat as a string */
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_LONGVARCHAR:
        case SQL_BINARY:
        case SQL_VARBINARY:
        case SQL_LONGVARBINARY:
        case SQL_WCHAR:
        case SQL_WVARCHAR:
        case SQL_WLONGVARCHAR:
            {
              if (aprec->concise_type == SQL_C_WCHAR &&
                  dbc->cxn_charset_info->number != UTF8_CHARSET_NUMBER)
                to= add_to_buffer(net, to, "_utf8", 5);
              else if (aprec->concise_type != SQL_C_WCHAR &&
                       dbc->cxn_charset_info->number !=
                       dbc->ansi_charset_info->number)
              {
                to= add_to_buffer(net, to, "_", 1);
                to= add_to_buffer(net, to, dbc->ansi_charset_info->csname,
                                  strlen(dbc->ansi_charset_info->csname));
              }
              /* We have only added the introducer, data is added below. */
              break;
            }
        case SQL_TIME:
        case SQL_TYPE_TIME:
            if ( aprec->concise_type == SQL_C_TIMESTAMP ||
                 aprec->concise_type == SQL_C_TYPE_TIMESTAMP )
            {
                TIMESTAMP_STRUCT *time= (TIMESTAMP_STRUCT*) aprec->data_ptr;
                sprintf(buff,"'%02d:%02d:%02d'",time->hour,time->minute,time->second);
                to= add_to_buffer(net, to, buff, 10);
            }
            else
            {
                ulong time= str_to_time_as_long(data,length);
                sprintf(buff,"'%02d:%02d:%02d'",
                        (int) time/10000,
                        (int) time/100%100,
                        (int) time%100);
                to= add_to_buffer(net, to, buff, 10);
            }
            goto out;
        case SQL_FLOAT:
        case SQL_REAL:
        case SQL_DOUBLE:
            /* If we have string -> float ; Fix locale characters for number */
            if ( convert )
            {
                char *to= buff, *from= data;
                char *end= from+length;
                while ( *from && from < end )
                {
                    if ( from[0] == thousands_sep[0] && is_prefix(from,thousands_sep) )
                        from+= thousands_sep_length;
                    else if ( from[0] == decimal_point[0] && is_prefix(from,decimal_point) )
                    {
                        from+= decimal_point_length;
                        *to++='.';
                    }
                    else
                        *to++= *from++;
                }
                if ( to == buff )
                    *to++='0';    /* Fix for empty strings */
                data= buff; length= (uint) (to-buff);

                convert= 0;

            }
            /* Fall through */
        default:
          if (!convert)
          {
            to= add_to_buffer(net, to, data, length);
            goto out;
          }
    }

    to= add_to_buffer(net,to,"'",1);
    /* Make sure we have room for a fully-escaped string. */
    if (!(to= extend_buffer(net, to, length * 2)))
      return 0;
    to+= mysql_real_escape_string(&dbc->mysql, to, data, length);
    to= add_to_buffer(net, to, "'", 1);

out:
    if (free_data)
      my_free(data, MYF(0));

    *toptr= to;
    return SQL_SUCCESS;

memerror:
    return set_error(stmt, MYERR_S1001, NULL, 4001);
}


/*
  @type    : myodbc3 internal
  @purpose : positioned cursor update/delete
*/

SQLRETURN do_my_pos_cursor( STMT FAR *pStmt, STMT FAR *pStmtCursor )
{
    char *          pszQuery   = pStmt->query;
    DYNAMIC_STRING  dynQuery;
    SQLRETURN       nReturn;

    if ( pStmt->error.native_error == ER_INVALID_CURSOR_NAME )
    {
        return set_stmt_error( pStmt, "HY000", "ER_INVALID_CURSOR_NAME", 0 );
    }

    while ( isspace( *pszQuery ) )
        pszQuery++;

    if ( init_dynamic_string( &dynQuery, pszQuery, 1024, 1024 ) )
        return set_error( pStmt, MYERR_S1001, NULL, 4001 );

    if ( !myodbc_casecmp( pszQuery, "delete", 6 ) )
    {
        nReturn = my_pos_delete( pStmtCursor, pStmt, 1, &dynQuery );
    }
    else if ( !myodbc_casecmp( pszQuery, "update", 6 ) )
    {
        nReturn = my_pos_update( pStmtCursor, pStmt, 1, &dynQuery );
    }
    else
    {
        nReturn = set_error( pStmt, MYERR_S1000, "Specified SQL syntax is not supported", 0 );
    }

    if ( SQL_SUCCEEDED( nReturn ) )
        pStmt->state = ST_EXECUTED;

    dynstr_free( &dynQuery );

    return( nReturn );
}


/*
  @type    : ODBC 1.0 API
  @purpose : executes a prepared statement, using the current values
  of the parameter marker variables if any parameter markers
  exist in the statement
*/

SQLRETURN SQL_API SQLExecute(SQLHSTMT hstmt)
{
    return my_SQLExecute((STMT FAR*)hstmt);
}


/*
  @type    : myodbc3 internal
  @purpose : executes a prepared statement, using the current values
  of the parameter marker variables if any parameter markers
  exist in the statement
*/

SQLRETURN my_SQLExecute( STMT FAR *pStmt )
{
    char       *query, *cursor_pos;
    uint        i;
    STMT FAR *  pStmtCursor = pStmt;
    SQLRETURN rc;

    if ( !pStmt )
        return SQL_ERROR;

    CLEAR_STMT_ERROR( pStmt );

    if ( !pStmt->query )
        return set_error(pStmt, MYERR_S1010,
                         "No previous SQLPrepare done", 0);

    if (is_set_names_statement((SQLCHAR *)pStmt->query))
      return set_error(pStmt, MYERR_42000,
                       "SET NAMES not allowed by driver", 0);

    if ( (cursor_pos= check_if_positioned_cursor_exists(pStmt, &pStmtCursor)) )
    {
      /* Save a copy of the query, because we're about to modify it. */
      pStmt->orig_query= my_strdup(pStmt->query, MYF(0));
      if (!pStmt->orig_query)
      {
        return set_error(pStmt,MYERR_S1001,NULL,4001);
      }
      pStmt->orig_query_end= pStmt->orig_query + (pStmt->query_end -
                                                  pStmt->query);

      /* Chop off the 'WHERE CURRENT OF ...' */
      *(char *)cursor_pos= '\0';

      return do_my_pos_cursor(pStmt, pStmtCursor);
    }

    /* If this statement has been executed, there are no
     * parameters, we do not need to execute it again */
    if (pStmt->state == ST_PRE_EXECUTED &&
        pStmt->dummy_state != ST_DUMMY_EXECUTED)
    {
        pStmt->state= ST_EXECUTED;
        return SQL_SUCCESS;
    }
    my_SQLFreeStmt((SQLHSTMT)pStmt,MYSQL_RESET_BUFFERS);
    query= pStmt->query;

    if ( pStmt->apd->rows_processed_ptr )
        *pStmt->apd->rows_processed_ptr= 0;

    if ( pStmt->param_count )
    {
        /*
         * If any parameters are required at execution time, cannot perform the
         * statement. It will be done through SQLPutData() and SQLParamData().
         */
        for ( i= 0; i < pStmt->param_count; i++ )
        {
            DESCREC *aprec= desc_get_rec(pStmt->apd, i, FALSE);
            SQLLEN *octet_length_ptr;
            assert(aprec);

            octet_length_ptr= ptr_offset_adjust(aprec->octet_length_ptr,
                                                pStmt->apd->bind_offset_ptr,
                                                pStmt->apd->bind_type,
                                                sizeof(SQLLEN), /*row*/0);

            if (IS_DATA_AT_EXEC(octet_length_ptr))
            {
                pStmt->current_param= i;
                aprec->par.value= NULL;
                aprec->par.alloced= FALSE;
                return SQL_NEED_DATA;
            }
        }
        if (!SQL_SUCCEEDED(rc= insert_params(pStmt, &query)))
            return rc;
    }

    rc= do_query(pStmt, query);
    if (pStmt->dummy_state == ST_DUMMY_PREPARED)
        pStmt->dummy_state= ST_DUMMY_EXECUTED;
    return rc;
}


/*
  @type    : ODBC 1.0 API
  @purpose : is used in conjunction with SQLPutData to supply parameter
  data at statement execution time
*/

SQLRETURN SQL_API SQLParamData(SQLHSTMT hstmt, SQLPOINTER FAR *prbgValue)
{
    STMT FAR *stmt= (STMT FAR*) hstmt;
    uint i;
    SQLRETURN rc;
    char *query;

    for ( i= stmt->current_param; i < stmt->param_count; i++ )
    {
        DESCREC *aprec= desc_get_rec(stmt->apd, i, FALSE);
        SQLLEN *octet_length_ptr= ptr_offset_adjust(aprec->octet_length_ptr,
                                                    stmt->apd->bind_offset_ptr,
                                                    stmt->apd->bind_type,
                                                    sizeof(SQLLEN), 0);
        if (IS_DATA_AT_EXEC(octet_length_ptr))
        {
            SQLINTEGER default_size= bind_length(aprec->concise_type,
                                                 aprec->octet_length);
            stmt->current_param= i+1;
            if ( prbgValue )
                *prbgValue= ptr_offset_adjust(aprec->data_ptr,
                                              stmt->apd->bind_offset_ptr,
                                              stmt->apd->bind_type,
                                              default_size, 0);
            aprec->par.value= NULL;
            aprec->par.alloced= FALSE;
            return SQL_NEED_DATA;
        }
    }
    if (!SQL_SUCCEEDED(rc= insert_params(stmt, &query)))
        return rc;
    return do_query(stmt, query);
}


/*
  @type    : ODBC 1.0 API
  @purpose : allows an application to send data for a parameter or column to
  the driver at statement execution time. This function can be used
  to send character or binary data values in parts to a column with
  a character, binary, or data source specific data type.
*/

SQLRETURN SQL_API SQLPutData( SQLHSTMT      hstmt, 
                              SQLPOINTER    rgbValue,
                              SQLLEN        cbValue )
{
    STMT FAR *stmt= (STMT FAR*) hstmt;
    DESCREC *aprec;

    if ( !stmt )
        return SQL_ERROR;

    if ( cbValue == SQL_NTS )
        cbValue= strlen(rgbValue);
    aprec= desc_get_rec(stmt->apd, stmt->current_param - 1, FALSE);
    assert(aprec);
    if ( cbValue == SQL_NULL_DATA )
    {
        if ( aprec->par.alloced )
            my_free(aprec->par.value,MYF(0));
        aprec->par.alloced= FALSE;
        aprec->par.value= NULL;
        return SQL_SUCCESS;
    }
    if ( aprec->par.value )
    {
        /* Append to old value */
        assert(aprec->par.alloced);
        if ( aprec->par.alloced )
        {
            if ( !(aprec->par.value= my_realloc(aprec->par.value,
                                                aprec->par.value_length + cbValue + 1,
                                                MYF(0))) )
                return set_error(stmt,MYERR_S1001,NULL,4001);
        }
        else
        {
            /* This should never happen */
            char *old_pos= aprec->par.value;
            if ( !(aprec->par.value= my_malloc(aprec->par.value_length+cbValue+1,MYF(0))) )
                return set_error(stmt,MYERR_S1001,NULL,4001);
            memcpy(aprec->par.value,old_pos,aprec->par.value_length);
        }
        memcpy(aprec->par.value+aprec->par.value_length,rgbValue,cbValue);
        aprec->par.value_length+= cbValue;
        aprec->par.value[aprec->par.value_length]= 0;
        aprec->par.alloced= TRUE;
    }
    else
    {
        /* New value */
        if ( !(aprec->par.value= my_malloc(cbValue+1,MYF(0))) )
            return set_error(stmt,MYERR_S1001,NULL,4001);
        memcpy(aprec->par.value,rgbValue,cbValue);
        aprec->par.value_length= cbValue;
        aprec->par.value[aprec->par.value_length]= 0;
        aprec->par.alloced= TRUE;
    }
    return SQL_SUCCESS;
}


/**
  Cancel the query by opening another connection and using KILL when called
  from another thread while the query lock is being held. Otherwise, treat as
  SQLFreeStmt(hstmt, SQL_CLOSE).

  @param[in]  hstmt  Statement handle

  @return Standard ODBC result code
*/
SQLRETURN SQL_API SQLCancel(SQLHSTMT hstmt)
{
  DBC *dbc= ((STMT *)hstmt)->dbc;
  MYSQL *second= NULL;
  int error;

  error= pthread_mutex_trylock(&dbc->lock);

  /* If there's no query going on, just close the statement. */
  if (error == 0)
  {
    pthread_mutex_unlock(&dbc->lock);
    return my_SQLFreeStmt(hstmt, SQL_CLOSE);
  }

  /* If we got a non-BUSY error, it's just an error. */
  if (error != EBUSY)
    return set_stmt_error((STMT *)hstmt, "HY000",
                          "Unable to get connection mutex status", error);

  /*
    If the mutex was locked, we need to make a new connection and KILL the
    ongoing query.
  */
  second= mysql_init(second);

  /** @todo need to preserve and use ssl params */

  if (!mysql_real_connect(second, dbc->server, dbc->user, dbc->password,
                          NULL, dbc->port, dbc->socket, dbc->flag))
  {
    /* We do not set the SQLSTATE here, per the ODBC spec. */
    return SQL_ERROR;
  }

  {
    char buff[40];
    /* buff is always big enough because max length of %lu is 15 */
    sprintf(buff, "KILL /*!50000 QUERY */ %lu", mysql_thread_id(&dbc->mysql));
    if (mysql_real_query(second, buff, strlen(buff)))
    {
      mysql_close(second);
      /* We do not set the SQLSTATE here, per the ODBC spec. */
      return SQL_ERROR;
    }
  }

  mysql_close(second);

  return SQL_SUCCESS;
}
