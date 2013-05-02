/*
  Copyright (c) 2000, 2013, Oracle and/or its affiliates. All rights reserved.

  The MySQL Connector/ODBC is licensed under the terms of the GPLv2
  <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
  MySQL Connectors. There are special exceptions to the terms and
  conditions of the GPLv2 as it is applied to this software, see the
  FLOSS License Exception
  <http://www.mysql.com/about/legal/licensing/foss-exception.html>.
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.
  
  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
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
SQLRETURN do_query(STMT FAR *stmt,char *query, SQLULEN query_length)
{
    int error= SQL_ERROR, native_error= 0;

    if (!query)
    {
      return error;       /* Probably error from insert_param */
    }

    if(!SQL_SUCCEEDED(set_sql_select_limit(stmt->dbc, stmt->stmt_options.max_rows)))
    {
      /* if setting sql_select_limit fails, the query will probably fail anyway too */
      return error;
    }

    if (query_length == 0)
    {
      query_length= strlen(query);
    }

    MYLOG_QUERY(stmt, query);
    pthread_mutex_lock(&stmt->dbc->lock);

    if ( check_if_server_is_alive( stmt->dbc ) )
    {
      set_stmt_error( stmt, "08S01" /* "HYT00" */,
                      mysql_error(&stmt->dbc->mysql),
                      mysql_errno(&stmt->dbc->mysql));
      translate_error(stmt->error.sqlstate, MYERR_08S01 /* S1000 */,
                      mysql_errno(&stmt->dbc->mysql));
      goto exit;
    }

    /* Simplifying task so far - we will do "LIMIT" scrolling forward only
     * and when no musltiple statements is allowed - we can't now parse query
     * that well to detect multiple queries.
     */
    if (stmt->dbc->ds->cursor_prefetch_number > 0
      && !stmt->dbc->ds->allow_multiple_statements
      && stmt->stmt_options.cursor_type == SQL_CURSOR_FORWARD_ONLY
      && scrollable(stmt, query, query+query_length))
    {
      /* we might want to read primary key info at this point, but then we have to
         know if we have a select from a single table...
       */
      ssps_close(stmt);
      scroller_reset(stmt);

      stmt->scroller.row_count= calc_prefetch_number(
                                      stmt->dbc->ds->cursor_prefetch_number,
                                      stmt->ard->array_size,
                                      stmt->stmt_options.max_rows);

      scroller_create(stmt, query, query_length);
      scroller_move(stmt);
      MYLOG_QUERY(stmt, stmt->scroller.query);

      native_error= mysql_real_query(&stmt->dbc->mysql, stmt->scroller.query,
                                  (unsigned long)stmt->scroller.query_len);
    }
      /* Not using ssps for scroller so far. Relaxing a bit condition
       if allow_multiple_statements option selected by primitive check if
       this is a batch of queries */
    else if (ssps_used(stmt))
    {
      native_error= mysql_stmt_bind_param(stmt->ssps,
                                        (MYSQL_BIND*)stmt->param_bind->buffer);
      if (native_error == 0)
      {
        native_error= mysql_stmt_execute(stmt->ssps);
      }
      else
      {
        set_stmt_error(stmt, "HY000",
                       mysql_stmt_error(stmt->ssps),
                       mysql_stmt_errno(stmt->ssps));

        /* For some errors - translating to more appropriate status */
        translate_error(stmt->error.sqlstate, MYERR_S1000,
                        mysql_stmt_errno(stmt->ssps));
        goto exit;
      }
      MYLOG_QUERY(stmt, "ssps has been executed");
    }
    else
    {
      MYLOG_QUERY(stmt, "Using direct execution");
      /* Need to close ps handler if it is open as our relsult will be generated
         by direct execution. and ps handler may create some chaos */
      ssps_close(stmt);
      native_error= mysql_real_query(&stmt->dbc->mysql,query,query_length);
    }

    MYLOG_QUERY(stmt, "query has been executed");

    if (native_error)
    {
      MYLOG_QUERY(stmt, mysql_error(&stmt->dbc->mysql));
      set_stmt_error(stmt, "HY000", mysql_error(&stmt->dbc->mysql),
                     mysql_errno(&stmt->dbc->mysql));

      /* For some errors - translating to more appropriate status */
      translate_error(stmt->error.sqlstate, MYERR_S1000,
                      mysql_errno(&stmt->dbc->mysql));
      goto exit;
    }

    if (!get_result_metadata(stmt, FALSE))
    {
      /* Query was supposed to return result, but result is NULL*/
      if (returned_result(stmt))
      {
        set_error(stmt, MYERR_S1000, mysql_error(&stmt->dbc->mysql),
                mysql_errno(&stmt->dbc->mysql));
        goto exit;
      }
      else /* Query was not supposed to return a result */
      {
        error= SQL_SUCCESS;     /* no result set */
        stmt->state= ST_EXECUTED;
        update_affected_rows(stmt);
        goto exit;
      }
    }

    /* If the only resultset is OUT params, then we can only detect
       corresponding server_status right after execution.
       If the RS is OUT params - we do not need to do store_result obviously */
    if (IS_PS_OUT_PARAMS(stmt))
    {
      /* For out parameters resultset we do not need to get result and bind it */
      fix_result_types(stmt);
      /* This status(SERVER_PS_OUT_PARAMS) can be only if we used PS */
      ssps_get_out_params(stmt);
    }
    else
    {
      if (bind_result(stmt) || get_result(stmt))
      {
          set_error(stmt, MYERR_S1000, mysql_error(&stmt->dbc->mysql),
                  mysql_errno(&stmt->dbc->mysql));
          goto exit;
      }
      /* Caching row counts for queries returning resultset as well */
      //update_affected_rows(stmt);
      fix_result_types(stmt);
    }

    error= SQL_SUCCESS;

    exit:
    pthread_mutex_unlock(&stmt->dbc->lock);
    if (query != GET_QUERY(&stmt->query))
    {
      x_free(query);
    }

    /*
      If the original query was modified, we reset stmt->query so that the
      next execution re-starts with the original query.
    */
    if (GET_QUERY(&stmt->orig_query))
    {
      copy_parsed_query(&stmt->orig_query, &stmt->query);
      reset_parsed_query(&stmt->orig_query, NULL, NULL, NULL);
    }

    return error;
}


/*
  @type    : myodbc3 internal
  @purpose : insert sql params at parameter positions
  @param[in]      stmt        Statement
  @param[in]      row         Parameters row
  @param[in,out]  finalquery  if NULL, final query is not copied
  @param[in,out]  length      Length of the query. Pointed value is used as initial offset
*/

SQLRETURN insert_params(STMT FAR *stmt, SQLULEN row, char **finalquery,
                        SQLULEN *finalquery_length)
{
  char *query= GET_QUERY(&stmt->query), *to;
  uint i,length, had_info= 0;
  NET *net;
  SQLRETURN rc= SQL_SUCCESS;

  int mutex_was_locked= pthread_mutex_trylock(&stmt->dbc->lock);

  net= &stmt->dbc->mysql.net;
  to= (char*) net->buff + (finalquery_length!= NULL ? *finalquery_length : 0);

  if (!stmt->dbc->ds->dont_use_set_locale)
  {
    setlocale(LC_NUMERIC, "C");  /* force use of '.' as decimal point */
  }

  if (ssps_used(stmt) && stmt->param_count > stmt->param_bind->max_element)
  {
    uint prev_max_elements= stmt->param_bind->max_element;

    if (allocate_dynamic(stmt->param_bind, stmt->param_count))
    {
      goto memerror;
    }

    /* Need to init newly allocated area with 0s */
    memset(stmt->param_bind->buffer + sizeof(MYSQL_BIND)*prev_max_elements, 0,
      sizeof(MYSQL_BIND) * (stmt->param_bind->max_element - prev_max_elements));
  }

  for ( i= 0; i < stmt->param_count; ++i )
  {
    DESCREC *aprec= desc_get_rec(stmt->apd, i, FALSE);
    DESCREC *iprec= desc_get_rec(stmt->ipd, i, FALSE);
    char *pos;
    MYSQL_BIND * bind;

    assert(aprec && iprec);

    if (stmt->dummy_state != ST_DUMMY_PREPARED &&
        !aprec->par.real_param_done)
    {
      rc= set_error(stmt,MYERR_07001,NULL,0);
      goto error;
    }

    if (ssps_used(stmt))
    {
      bind= (MYSQL_BIND *)stmt->param_bind->buffer + i;
      bind->is_null_value= 0;
      bind->is_unsigned= 0;

      /* as far as looked - this trick is safe */
      bind->is_null=  &bind->is_null_value;
      bind->length=   &bind->length_value;

      rc= insert_param(stmt, (uchar*)bind, stmt->apd, aprec, iprec, row);
    }
    else
    {
      pos= get_param_pos(&stmt->query, i);
      length= (uint) (pos-query);

      if ( !(to= add_to_buffer(net,to,query,length)) )
      {
        goto memerror;
      }

      query= pos+1;  /* Skip '?' */

      rc= insert_param(stmt, (uchar*)&to, stmt->apd, aprec, iprec, row);
    }

    if (!SQL_SUCCEEDED(rc))
    {
      goto error;
    }
    else
    {
      if (rc == SQL_SUCCESS_WITH_INFO)
      {
        had_info= 1;
      }
    }
  }

  /* if any ofr parameters return SQL_SUCCESS_WITH_iNFO - returning it
     SQLSTATE corresponds to last SQL_SUCCESS_WITH_iNFO
  */
  if (had_info)
  {
    rc= SQL_SUCCESS_WITH_INFO;
  }

  if (!ssps_used(stmt))
  {
    length= (uint) (GET_QUERY_END(&stmt->query) - query);

    if ( !(to= add_to_buffer(net, to, query, length + 1)) )
    {
      goto memerror;
    }

    if (finalquery_length!= NULL)
    {
      *finalquery_length= to - (char*)net->buff - 1;
    }

    if (finalquery!=NULL)
    {
      if ( !(to= (char*) my_memdup((char*) net->buff,
        (uint) (to - (char*) net->buff),MYF(0))) )
      {
        goto memerror;
      }
    }

    if (finalquery!=NULL)
    {
      *finalquery= to;
    }
  }

  if (!mutex_was_locked)
  {
    pthread_mutex_unlock(&stmt->dbc->lock);
  }

  if (!stmt->dbc->ds->dont_use_set_locale)
  {
    setlocale(LC_NUMERIC,default_locale);
  }



  return rc;

memerror:      /* Too much data */
  rc= set_error(stmt,MYERR_S1001,NULL,4001);
error:
  /* ! was _already_ locked, when we tried to lock */
  if (!mutex_was_locked)
    pthread_mutex_unlock(&stmt->dbc->lock);
  if (!stmt->dbc->ds->dont_use_set_locale)
    setlocale(LC_NUMERIC,default_locale);
  return rc;
}

static
void put_null_param(STMT *stmt, NET* net, char** toptr, MYSQL_BIND *bind)
{
  if (ssps_used(stmt))
  {
    bind->is_null_value= '\1';
  }
  else
  {
    *toptr= add_to_buffer(net, *toptr, "NULL", 4);
  }
}


static
void put_default_value(STMT *stmt, NET* net, char** toptr, MYSQL_BIND *bind)
{
  if (ssps_used(stmt))
  {
    /* That is actually wrong, but i can't see a good way */
    bind->is_null_value= '\1';
  }
  else
  {
    *toptr= add_to_buffer(net, *toptr, "DEFAULT", 7);
  }
}


static
BOOL allocate_param_buffer(MYSQL_BIND *bind, unsigned long length)
{
  /* have to be very careful with that. it is probably better to put into
       a separate data structure. and free right after use */
  if (bind->buffer == NULL)
  {
    bind->buffer= my_malloc(length, MYF(0));
    bind->buffer_length= length;
  }
  else if(bind->buffer_length < length)
  {
    bind->buffer= my_realloc(bind->buffer, length, MYF(0));
    bind->buffer_length= length;
  }

  if (bind->buffer == NULL)
  {
    return TRUE;
  }

  return FALSE;
}


/* Buffer has to be allocated - no checks */
static
unsigned long add2param_value(MYSQL_BIND *bind, unsigned long pos,
                              const char *value, unsigned long length)
{
  memcpy((char*)bind->buffer + pos, value, length);

  bind->length_value= pos + length;
  return pos + length;
}


static
BOOL bind_param(MYSQL_BIND *bind, const char *value, unsigned long length,
                enum enum_field_types buffer_type)
{
  if (allocate_param_buffer(bind, length))
  {
    return TRUE;
  }

  memcpy(bind->buffer, value, length);
  bind->buffer_type= buffer_type;
  bind->length_value= length;

  return FALSE;
}

/* TRUE - on memory allocation error */
static
BOOL put_param_value(STMT *stmt, NET* net, char** toptr, MYSQL_BIND *bind,
                     const char * value, unsigned long length)
{
  if (ssps_used(stmt))
  {
    return bind_param(bind, value, length, MYSQL_TYPE_STRING);
  }
  else
  {
    *toptr= add_to_buffer(net, *toptr, value, length);
  }

  return FALSE;
}

/*
              stmt
              ctype       Input(parameter) value C type
              iprec
    [in,out]  rec         Pointer input and output value
              length      Pointer for result length
              buff        Pointer to a buffer for result value
              buff_max    Size of the buffer

*/
static
SQLRETURN convert_c_type2str(STMT *stmt, SQLSMALLINT ctype, DESCREC *iprec,
                             char **res, int *length, char *buff, uint buff_max)
{
  switch ( ctype )
  {
    case SQL_C_BINARY:
    case SQL_C_CHAR:
        break;

    case SQL_C_WCHAR:
      {
        /* Convert SQL_C_WCHAR (utf-16 or utf-32) to utf-8. */
        int has_utf8_maxlen4= 0;

        /* length is in bytes, we want chars */
        *length= *length / sizeof(SQLWCHAR);

        *res= sqlwchar_as_utf8_ext((SQLWCHAR*)*res, length, buff, buff_max,
                                    &has_utf8_maxlen4);


        if (has_utf8_maxlen4 &&
            !is_minimum_version(stmt->dbc->mysql.server_version, "5.5.3"))
        {
          return set_stmt_error(stmt, "HY000",
                                "Server does not support 4-byte encoded "
                                "UTF8 characters.", 0);
        }
        break;
      }

    case SQL_C_BIT:
    case SQL_C_TINYINT:
    case SQL_C_STINYINT:
        *length= my_int2str((long)*((signed char *)*res), buff, -10, 0) - buff;
        *res= buff;
        break;
    case SQL_C_UTINYINT:
        *length= my_int2str((long)*((unsigned char *)*res), buff, -10, 0) - buff;
        *res= buff;
        break;
    case SQL_C_SHORT:
    case SQL_C_SSHORT:
        *length= my_int2str((long)*((short int *)*res), buff, -10, 0) - buff;
        *res= buff;
        break;
    case SQL_C_USHORT:
        *length= my_int2str((long)*((unsigned short int *)*res), buff, -10, 0) -
                  buff;
        *res= buff;
        break;
    case SQL_C_LONG:
    case SQL_C_SLONG:
        *length= my_int2str(*((SQLINTEGER*) *res), buff, -10, 0) - buff;
        *res= buff;
        break;
    case SQL_C_ULONG:
        *length= my_int2str(*((SQLUINTEGER*) *res), buff, 10, 0) - buff;
        *res= buff;
        break;
    case SQL_C_SBIGINT:
        *length= longlong2str(*((longlong*) *res), buff, -10) - buff;
        *res= buff;
        break;
    case SQL_C_UBIGINT:
        *length= longlong2str(*((ulonglong*) *res), buff, 10) - buff;
        *res= buff;
        break;
    case SQL_C_FLOAT:
      if ( iprec->concise_type != SQL_NUMERIC && iprec->concise_type != SQL_DECIMAL )
      {
        sprintf(buff, "%.17e", *((float*) *res));
      }
      else
      {
        /* We should perpare this data for string comparison */
        sprintf(buff, "%.15e", *((float*) *res));
      }
      *length= strlen(*res= buff);
      break;
    case SQL_C_DOUBLE:
      if ( iprec->concise_type != SQL_NUMERIC && iprec->concise_type != SQL_DECIMAL )
      {
        sprintf(buff,"%.17e",*((double*) *res));
      }
      else
      {
        /* We should perpare this data for string comparison */
        sprintf(buff,"%.15e",*((double*) *res));
      }
      *length= strlen(*res= buff);
      break;
    case SQL_C_DATE:
    case SQL_C_TYPE_DATE:
      {
        DATE_STRUCT *date= (DATE_STRUCT*) *res;
        if (stmt->dbc->ds->min_date_to_zero && !date->year
          && (date->month == date->day == 1))
        {
          sprintf(buff, "0000-00-00");
        }
        else
        {
          sprintf(buff, "%04d-%02d-%02d", date->year, date->month, date->day);
        }
        *res= buff;
        *length= 10;
        break;
      }
    case SQL_C_TIME:
    case SQL_C_TYPE_TIME:
      {
        TIME_STRUCT *time= (TIME_STRUCT*) *res;
        sprintf(buff, "%02d:%02d:%02d",
                time->hour, time->minute, time->second);
        *res= buff;
        *length= 8;
        break;
      }
    case SQL_C_TIMESTAMP:
    case SQL_C_TYPE_TIMESTAMP:
      {
        TIMESTAMP_STRUCT *time= (TIMESTAMP_STRUCT*) *res;

        if (stmt->dbc->ds->min_date_to_zero &&
            !time->year && (time->month == time->day == 1))
        {
          sprintf(buff, "0000-00-00 %02d:%02d:%02d", time->hour,
                  time->minute, time->second);
        }
        else
        {
          sprintf(buff, "%04d-%02d-%02d %02d:%02d:%02d",
                    time->year, time->month, time->day,
                    time->hour, time->minute, time->second);
        }

        *length= 19;

        if (time->fraction)
        {
          char *tmp_buf= buff + *length;
          
          /* Start cleaning from the end */
          int tmp_pos= 9;

          sprintf(tmp_buf, ".%09d", time->fraction);
          
          /*
            ODBC specification defines nanoseconds granularity for
            the fractional part of seconds. MySQL only supports 
            microseconds for TIMESTAMP, TIME and DATETIME.

            We are trying to remove the trailing zeros because this 
            does not really modify the data, but often helps to substitute
            9 digits with only 6.
          */
          while (tmp_pos && tmp_buf[tmp_pos] == '0')
          {
            tmp_buf[tmp_pos--]= 0;
          }

          *length+= tmp_pos + 1;
        }

        *res= buff;

        break;
      }
    case SQL_C_NUMERIC:
      {
        int trunc;
        SQL_NUMERIC_STRUCT *sqlnum= (SQL_NUMERIC_STRUCT *) *res;
        sqlnum_to_str(sqlnum, (SQLCHAR *)(buff + buff_max - 1),
                      (SQLCHAR **) res,
                      (SQLCHAR) iprec->precision,
                      (SQLSCHAR) iprec->scale, &trunc);
        *length= strlen(*res);
        /* TODO no way to return an error here? */
        if (trunc == SQLNUM_TRUNC_FRAC)
        {/* 01S07 SQL_SUCCESS_WITH_INFO */
          set_stmt_error(stmt, "01S07", "Fractional truncation", 0);
          return SQL_SUCCESS_WITH_INFO;
        }
        else if (trunc == SQLNUM_TRUNC_WHOLE)
        {/* 22003 SQL_ERROR */
          return SQL_ERROR;
        }
      }
  }
  return SQL_SUCCESS;
}

#define TIME_FIELDS_NONZERO(ts) (ts.hour||ts.minute||ts.second||ts.fraction)

/*
  Add the value of parameter to a string buffer.

  @param[in]      mysql
  @param[in,out]  toptr - either pointer to a string where to write
                  parameter value, or a pointer to MYSQL_BIND structure.
  @param[in]      apd The APD of the current statement
  @param[in]      aprec The APD record of the parameter
  @param[in]      iprec The IPD record of the parameter
*/
SQLRETURN insert_param(STMT *stmt, uchar *place4param, DESC* apd,
                       DESCREC *aprec, DESCREC *iprec, SQLULEN row)
{
    long length;
    char buff[128], *data= NULL;
    BOOL convert= FALSE, free_data= FALSE;
    DBC *dbc= stmt->dbc;
    NET *net= &dbc->mysql.net;
    SQLLEN *octet_length_ptr= NULL;
    SQLLEN *indicator_ptr= NULL;
    SQLRETURN result= SQL_SUCCESS;

    /* carefully should be used either these 2 or that(bind) ptr. union? */
    char **toptr= (char**)place4param, *to= *toptr;
    MYSQL_BIND * bind= (MYSQL_BIND*)place4param;

    if (aprec->octet_length_ptr)
    {
      octet_length_ptr= ptr_offset_adjust(aprec->octet_length_ptr,
                                          apd->bind_offset_ptr,
                                          apd->bind_type,
                                          sizeof(SQLLEN), row);
      length= *octet_length_ptr;
    }

    indicator_ptr= ptr_offset_adjust(aprec->indicator_ptr,
                                     apd->bind_offset_ptr,
                                     apd->bind_type,
                                     sizeof(SQLLEN), row);

    if (aprec->data_ptr)
    {
      SQLINTEGER default_size= bind_length(aprec->concise_type,
                                           aprec->octet_length);
      data= ptr_offset_adjust(aprec->data_ptr, apd->bind_offset_ptr,
                              apd->bind_type, default_size, row);
    }

    if (indicator_ptr && *indicator_ptr == SQL_NULL_DATA)
    {
      put_null_param(stmt, net, toptr, bind);

      return SQL_SUCCESS;
    }
    /*
      According to http://msdn.microsoft.com/en-us/library/ms710963%28VS.85%29.aspx

      "... If StrLen_or_IndPtr is a null pointer, the driver assumes that all
      input parameter values are non-NULL and that character and *binary* data
      is null-terminated."
    */
    else if (!octet_length_ptr || *octet_length_ptr == SQL_NTS)
    {
      if (data)
      {
        if (aprec->concise_type == SQL_C_WCHAR)
        {
          length= sqlwcharlen((SQLWCHAR *)data) * sizeof(SQLWCHAR);
        }
        else
        {
          length= strlen(data);
        }

        if (!octet_length_ptr && aprec->octet_length > 0 &&
            aprec->octet_length != SQL_SETPARAM_VALUE_MAX)
        {
          length= myodbc_min(length, aprec->octet_length);
        }
      }
      else
      {
        length= 0;     /* TODO? This is actually an error */
      }
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
      put_default_value(stmt, net, toptr, bind);
      return SQL_SUCCESS;
    }
    else if (IS_DATA_AT_EXEC(octet_length_ptr))
    {
        length= aprec->par.value_length;
        if ( !(data= aprec->par.value) )
        {
          put_default_value(stmt, net, toptr, bind);
          return SQL_SUCCESS;
        }
    }

    switch ( aprec->concise_type )
    {
      
      case SQL_C_BINARY:
      case SQL_C_CHAR:
          convert= 1;
          break;

      default:
        switch(convert_c_type2str(stmt, aprec->concise_type, iprec,
                             &data, &length, buff, sizeof(buff)))
        {
        case SQL_ERROR:
          return SQL_ERROR;
        case SQL_SUCCESS_WITH_INFO:
          result= SQL_SUCCESS_WITH_INFO;
        }

        if (data == NULL)
        {
          goto memerror;
        }

        if (!(data >= buff && data < buff + sizeof(buff)))
        {
          free_data= TRUE;
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
            /* TODO: check if we need to check for truncation here as well? */
            if (ssps_used(stmt))
            {
              if (bind_param(bind, data, length,
                          map_sql2mysql_type(iprec->concise_type)))
              {
                goto memerror;
              }
            }
            else
            {
              to= add_to_buffer(net, to, data, length);
            }
            goto out;
          }

          if (iprec->concise_type == SQL_DATE
            || iprec->concise_type == SQL_TYPE_DATE)
          {
            TIMESTAMP_STRUCT ts;

            /* For now I think it is safer to assume a dot is always a
               separator */
            /* aprec->concise_type == SQL_C_TYPE_TIMESTAMP
            || aprec->concise_type == SQL_C_TIMESTAMP
            || stmt->dbc->ds->dont_use_set_locale */
            str_to_ts(&ts, data, length, 1, TRUE);

            /* Overflow also possible if converted from other C types
               http://msdn.microsoft.com/en-us/library/ms709385%28v=vs.85%29.aspx
               (if time fields nonzero sqlstate 22008 )
               http://msdn.microsoft.com/en-us/library/aa937531%28v=sql.80%29.aspx
               (Class values other than 01, except for the class IM,
               indicate an error and are accompanied by a return code
               of SQL_ERROR)
               
               Not sure if fraction should be considered as an overflow.
               In fact specs say about "time fields only"
             */
            if (TIME_FIELDS_NONZERO(ts))
            {
              return set_stmt_error(stmt, "22008", "Date overflow", 0);
            }
          }
          /* else _binary introducer for binary data */
        case SQL_BINARY:
        case SQL_VARBINARY:
        case SQL_LONGVARBINARY:
          {
            if (ssps_used(stmt))
            {
              /* i guess for ssps we do not need introducer */
              convert= 0;
            }
            else
            {
              if (dbc->cxn_charset_info->number !=
                 dbc->ansi_charset_info->number)
              {
                to= add_to_buffer(net, to, "_binary", 7);
              }
            }
            /* We have only added the introducer, data is added below. */
            break;
           }
           /* else treat as a string */
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_LONGVARCHAR:
        case SQL_WCHAR:
        case SQL_WVARCHAR:
        case SQL_WLONGVARCHAR:
          {
            if (aprec->concise_type == SQL_C_WCHAR &&
                dbc->cxn_charset_info->number != UTF8_CHARSET_NUMBER)
            {
              if (ssps_used(stmt))
              {
                 if (bind_param(bind, data, length, MYSQL_TYPE_BLOB))
                {
                  goto memerror;
                }

                goto out;
              }
              else
              {
                to= add_to_buffer(net, to, "_utf8", 5);
              }
            }
            else if (aprec->concise_type != SQL_C_WCHAR &&
                     dbc->cxn_charset_info->number !=
                     dbc->ansi_charset_info->number)
            {
              if (ssps_used(stmt))
              {
                if (bind_param(bind, data, length, MYSQL_TYPE_BLOB))
                {
                  goto memerror;
                }

                goto out;
              }
              else
              {
                to= add_to_buffer(net, to, "_", 1);
                to= add_to_buffer(net, to, dbc->ansi_charset_info->csname,
                                strlen(dbc->ansi_charset_info->csname));
              }
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

            if (time->fraction)
            {
              /* fractional seconds truncated, need to set correct sqlstate 22008
              http://msdn.microsoft.com/en-us/library/ms709385%28v=vs.85%29.aspx */

              return set_stmt_error(stmt, "22008", "Fractional truncation", 0);
            }

            if (ssps_used(stmt))
            {
              sprintf(buff, "%02d:%02d:%02d",time->hour,time->minute,
                      time->second);
              length= 8;
            }
            else
            {
              sprintf(buff, "'%02d:%02d:%02d'",time->hour,time->minute,
                      time->second);
              length= 10;
            }

            if (put_param_value(stmt, net, &to, bind, buff, length))
            {
              goto memerror;
            }
          }
          else
          {
            ulong time;
            SQLUINTEGER fraction;

            /* For now it is safer to assume a dot is always a separator */
            /* stmt->dbc->ds->dont_use_set_locale */
            get_fractional_part(data, length, TRUE, &fraction);

            if (fraction)
            {
              /* truncation need SQL_ERROR and sqlstate 22008*/
              return set_stmt_error(stmt, "22008", "Fractional truncation", 0);
            }

            time= str_to_time_as_long(data,length);

            if (ssps_used(stmt))
            {
              sprintf(buff, "%02d:%02d:%02d",
                    (int) time/10000,
                    (int) time/100%100,
                    (int) time%100);
              length= 8;
            }
            else
            {
              sprintf(buff, "'%02d:%02d:%02d'",
                    (int) time/10000,
                    (int) time/100%100,
                    (int) time%100);
              length= 10;
            }

            if (put_param_value(stmt, net, &to, bind, buff, length))
            {
              goto memerror;
            }
          }

          goto out;

        case SQL_BIT:
          {
            if (ssps_used(stmt))
            {
              char bit_val= atoi(data)!= 0 ? 1 : 0;
              /* Generic ODBC supports only BIT(1) */
              bind_param(bind, &bit_val, 1, MYSQL_TYPE_TINY);
            }
            else if (!convert)
            {
              to= add_to_buffer(net, to, data, 1);
            }
            goto out;
          }
        case SQL_FLOAT:
        case SQL_REAL:
        case SQL_DOUBLE:
          /* If we have string -> float ; Fix locale characters for number */
          if (convert)
          {
            char *to= buff, *from= data;
            char *end= from+length;
            char *local_thousands_sep= thousands_sep;
            char *local_decimal_point= decimal_point;
            uint local_thousands_sep_length= thousands_sep_length;
            uint local_decimal_point_length= decimal_point_length;

            if (!stmt->dbc->ds->dont_use_set_locale)
            {
              /* force use of . as decimal point */
              local_thousands_sep= ",";
              local_thousands_sep_length= 1;
              local_decimal_point= ".";
              local_decimal_point_length= 1;
            }

            while ( *from && from < end )
            {
              if ( from[0] == local_thousands_sep[0] && is_prefix(from,local_thousands_sep) )
              {
                from+= local_thousands_sep_length;
              }
              else if ( from[0] == local_decimal_point[0] && is_prefix(from,local_decimal_point) )
              {
                from+= local_decimal_point_length;
                *to++='.';
              }
              else
              {
                *to++= *from++;
              }
            }
            if ( to == buff )
            {
              *to++='0';    /* Fix for empty strings */
            }
            data= buff;
            length= (uint) (to-buff);

            convert= 0;

          }
          /* Fall through */
        default:
          if (!convert)
          {
            put_param_value(stmt, net, &to, bind, data, length);
            goto out;
          }
    }

    if (ssps_used(stmt))
    {
      bind_param(bind, data, length, MYSQL_TYPE_STRING);
    }
    else
    {
    /* Convert binary data to hex sequence */
      if(is_no_backslashes_escape_mode(stmt->dbc) && 
       is_binary_sql_type(iprec->concise_type))
      {
        SQLLEN transformed_len = 0;
        to= add_to_buffer(net, to, " 0x", 3);
        /* Make sure we have room for a fully-escaped string. */
        if (!(to= extend_buffer(net, to, length * 2)))
        {
          goto memerror;
        }
      
        copy_binhex_result(stmt, to, length * 2 + 1, &transformed_len, 0, data, length);
        to += transformed_len;
      }
      else
      {
        to= add_to_buffer(net,to,"'",1);
        /* Make sure we have room for a fully-escaped string. */
        if ( !(to= extend_buffer(net, to, length * 2)) )
        {
          goto memerror;
        }
      
        to+= mysql_real_escape_string(&dbc->mysql, to, data, length);
        to= add_to_buffer(net, to, "'", 1);
      }
    }

out:
    if (free_data)
    {
      x_free(data);
    }

    *toptr= to;

    return result;

memerror:
    if (free_data)
    {
      x_free(data);
    }
    return set_error(stmt, MYERR_S1001, NULL, 4001);
}


/*
  @type    : myodbc3 internal
  @purpose : positioned cursor update/delete
*/

SQLRETURN do_my_pos_cursor( STMT FAR *pStmt, STMT FAR *pStmtCursor )
{
  char *          pszQuery= GET_QUERY(&pStmt->query);
  DYNAMIC_STRING  dynQuery;
  SQLRETURN       nReturn;

  if ( pStmt->error.native_error == ER_INVALID_CURSOR_NAME )
  {
      return set_stmt_error( pStmt, "HY000", "ER_INVALID_CURSOR_NAME", 0 );
  }

  while ( isspace( *pszQuery ) )
      ++pszQuery;

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


BOOL map_error_to_param_status( SQLUSMALLINT *param_status_ptr, SQLRETURN rc)
{
  if (param_status_ptr)
  {
    switch (rc)
    {
    case SQL_SUCCESS:
      *param_status_ptr= SQL_PARAM_SUCCESS;
      break;
    case SQL_SUCCESS_WITH_INFO:
      *param_status_ptr= SQL_PARAM_SUCCESS_WITH_INFO;
      break;

    default:
      /* SQL_PARAM_ERROR is set at the end of processing for last erroneous paramset
         so we have diagnostics for it */
      *param_status_ptr= SQL_PARAM_DIAG_UNAVAILABLE;
      return TRUE;
    }
  }

  return FALSE;
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
  int         dae_rec, is_select_stmt, one_of_params_not_succeded= 0;
  int         connection_failure= 0;
  STMT FAR *  pStmtCursor = pStmt;
  SQLRETURN   rc;
  SQLULEN     row, length= 0;

  SQLUSMALLINT *param_operation_ptr= NULL, *param_status_ptr= NULL, *lastError= NULL;
  
  /* need to have a flag indicating if all parameters failed */
  int all_parameters_failed= pStmt->apd->array_size > 1 ? 1 : 0;

  if ( !pStmt )
      return SQL_ERROR;

  CLEAR_STMT_ERROR( pStmt );

  if (!GET_QUERY(&pStmt->query))
      return set_error(pStmt, MYERR_S1010,
                       "No previous SQLPrepare done", 0);

  if (is_set_names_statement((SQLCHAR *)GET_QUERY(&pStmt->query)))
  {
    return set_error(pStmt, MYERR_42000,
                     "SET NAMES not allowed by driver", 0);
  }

  if ((cursor_pos= check_if_positioned_cursor_exists(pStmt, &pStmtCursor)))
  {
    /* Save a copy of the query, because we're about to modify it. */
    if (copy_parsed_query(&pStmt->query, &pStmt->orig_query))
    {
      return set_error(pStmt,MYERR_S1001,NULL,4001);
    }

    /* Cursor statement use mysql_use_result - thus any operation
       will couse commands out of sync */
    if (if_forward_cache(pStmtCursor))
    {
      return set_error(pStmt,MYERR_S1010,NULL,0);
    }

    /* Chop off the 'WHERE CURRENT OF ...' - doing it a hard way...*/
    *cursor_pos= '\0';

    return do_my_pos_cursor(pStmt, pStmtCursor);
  }

  my_SQLFreeStmt((SQLHSTMT)pStmt,MYSQL_RESET_BUFFERS);

  query= GET_QUERY(&pStmt->query);

  is_select_stmt= is_select_statement(&pStmt->query);

  /* if ssps is used for select query then convert it to non ssps 
	 single statement using UNION
  */
  if(is_select_stmt && ssps_used(pStmt) && pStmt->apd->array_size > 1)
  {
	ssps_close(pStmt);				
  }

  if ( pStmt->ipd->rows_processed_ptr )
  {
    *pStmt->ipd->rows_processed_ptr= 0;
  }

  /* Locking if we have params array for "SELECT" statemnt */
  /* if param_count is zero, the rest probably are artifacts(not reset
     attributes) from a previously executed statement. besides this lock
     is not needed when there are no params*/
  if (pStmt->param_count && pStmt->apd->array_size > 1 && is_select_stmt)
  {
    pthread_mutex_lock(&pStmt->dbc->lock);
  }

  for (row= 0; row < pStmt->apd->array_size; ++row)
  {
    if ( pStmt->param_count )
    {
      /* "The SQL_DESC_ROWS_PROCESSED_PTR field of the APD points to a buffer
      that contains the number of sets of parameters that have been processed,
      including error sets."
      "If SQL_NEED_DATA is returned, the value pointed to by the SQL_DESC_ROWS_PROCESSED_PTR
      field of the APD is set to the set of parameters that is being processed".
      And actually driver may continue to process paramsets after error.
      We need to decide do we want that.
      (http://msdn.microsoft.com/en-us/library/ms710963%28VS.85%29.aspx
      see "Using Arrays of Parameters")
      */
      if ( pStmt->ipd->rows_processed_ptr )
        *pStmt->ipd->rows_processed_ptr+= 1;

      param_operation_ptr= ptr_offset_adjust(pStmt->apd->array_status_ptr,
                                            NULL,
                                            0/*SQL_BIND_BY_COLUMN*/,
                                            sizeof(SQLUSMALLINT), row);
      param_status_ptr= ptr_offset_adjust(pStmt->ipd->array_status_ptr,
                                            NULL,
                                            0/*SQL_BIND_BY_COLUMN*/,
                                            sizeof(SQLUSMALLINT), row);

      if ( param_operation_ptr
        && *param_operation_ptr == SQL_PARAM_IGNORE)
      {
        /* http://msdn.microsoft.com/en-us/library/ms712631%28VS.85%29.aspx 
           - comments for SQL_ATTR_PARAM_STATUS_PTR */
        if (param_status_ptr)
          *param_status_ptr= SQL_PARAM_UNUSED;

        /* If this is last paramset - we will miss unlock */
        if (pStmt->apd->array_size > 1 && is_select_stmt
            && row == pStmt->apd->array_size - 1)
          pthread_mutex_unlock(&pStmt->dbc->lock);

        continue;
      }

      /*
       * If any parameters are required at execution time, cannot perform the
       * statement. It will be done through SQLPutData() and SQLParamData().
       */
      if ((dae_rec= desc_find_dae_rec(pStmt->apd)) > -1)
      {
        if (pStmt->apd->array_size > 1)
        {
          rc= set_stmt_error(pStmt, "HYC00", "Parameter arrays "
                              "with data at execution are not supported", 0);
          lastError= param_status_ptr;

          /* unlocking since we do break*/
          if (is_select_stmt)
            pthread_mutex_unlock(&pStmt->dbc->lock);

          one_of_params_not_succeded= 1;

          /* For other errors we continue processing of paramsets
             So this creates some inconsistency. But I guess that's better
             that user see diagnostics for this type of error */
          break;
        }

        pStmt->current_param= dae_rec;
        pStmt->dae_type= DAE_NORMAL;

        return SQL_NEED_DATA;
      }

      /* Making copy of the built query if that is not last paramset for select
         query. */
      if (is_select_stmt && row < pStmt->apd->array_size - 1)
      {
        rc= insert_params(pStmt, row, NULL, &length);
      }
      else
      {
        rc= insert_params(pStmt, row, &query, &length);
      }

      /* Setting status for this paramset*/
      if (map_error_to_param_status( param_status_ptr, rc))
      {
        lastError= param_status_ptr;
      }

      if (rc != SQL_SUCCESS)
      {
        one_of_params_not_succeded= 1;
      }

      if (!SQL_SUCCEEDED(rc))
      {
        if (pStmt->apd->array_size > 1 && is_select_stmt
          && row == pStmt->apd->array_size - 1)
          pthread_mutex_unlock(&pStmt->dbc->lock);

        continue/*return rc*/;
      }

      /* For "SELECT" statement constructing single statement using
         "UNION ALL" */
      if (pStmt->apd->array_size > 1 && is_select_stmt)
      {
        if (row < pStmt->apd->array_size - 1)
        {
          const char * stmtsBinder= " UNION ALL ";
          const ulong binderLength= strlen(stmtsBinder);

          add_to_buffer(&pStmt->dbc->mysql.net, (char*)pStmt->dbc->mysql.net.buff + length,
                     stmtsBinder, binderLength);
          length+= binderLength;
        }
        else
        {
          /* last select statement has been constructed - so releasing lock*/
          pthread_mutex_unlock(&pStmt->dbc->lock);
        }
      }
    }

    if (!is_select_stmt || row == pStmt->apd->array_size-1)
    {
      if (!connection_failure)
      {      
        rc= do_query(pStmt, query, length);
      }
      else
      {
        /* with broken connection we always return error for all next queries */
        rc= SQL_ERROR;
      }

      if (is_connection_lost(pStmt->error.native_error)
        && handle_connection_error(pStmt))
      {
        connection_failure= 1;
      }

      if (map_error_to_param_status(param_status_ptr, rc))
      {
        lastError= param_status_ptr;
      }

      /* if we have anything but not SQL_SUCCESS for any paramset, we return SQL_SUCCESS_WITH_INFO
         as the whole operation result */
      if (rc != SQL_SUCCESS)
      {
        one_of_params_not_succeded= 1;
      }
      else
      {
        all_parameters_failed= 0;
      }

      length= 0; 
    }
  }

  /* Changing status for last detected error to SQL_PARAM_ERROR as we have
     diagnostics for it */
  if (lastError != NULL)
  {
    *lastError= SQL_PARAM_ERROR;
  }

  /* Setting not processed paramsets status to SQL_PARAM_UNUSED 
     this is needed if we stop paramsets processing on error.
  */
  if (param_status_ptr != NULL)
  {
    while (++row < pStmt->apd->array_size)
    {
      param_status_ptr= ptr_offset_adjust(pStmt->ipd->array_status_ptr,
                                          NULL,
                                          0/*SQL_BIND_BY_COLUMN*/,
                                          sizeof(SQLUSMALLINT), row);

      *param_status_ptr= SQL_PARAM_UNUSED;
    }
  }

  if (pStmt->dummy_state == ST_DUMMY_PREPARED)
      pStmt->dummy_state= ST_DUMMY_EXECUTED;

  if (pStmt->apd->array_size > 1)
  {
    if (all_parameters_failed)
    {
      return SQL_ERROR;
    }
    else if (one_of_params_not_succeded != 0)
    {
      return SQL_SUCCESS_WITH_INFO;
    }
  }

  return rc;
}


/*
  @type    : ODBC 1.0 API
  @purpose : is used in conjunction with SQLPutData to supply parameter
  data at statement execution time
*/

SQLRETURN SQL_API SQLParamData(SQLHSTMT hstmt, SQLPOINTER *prbgValue)
{
    STMT *stmt= (STMT *) hstmt;
    uint i;
    SQLRETURN rc;
    char *query=  GET_QUERY(&stmt->query);
    DESC *apd;
    uint param_count= stmt->param_count;

    assert(stmt->dae_type);
    /* get the correct APD for the dae type we're handling */
    switch(stmt->dae_type)
    {
    case DAE_NORMAL:
      apd= stmt->apd;
      break;
    case DAE_SETPOS_INSERT:
    case DAE_SETPOS_UPDATE:
      apd= stmt->setpos_apd;
      param_count= stmt->ard->count;
      break;
    default:
      return set_stmt_error(stmt, "HY010",
                            "Invalid data at exec state", 0);
    }

    for ( i= stmt->current_param; i < param_count; ++i )
    {
        DESCREC *aprec= desc_get_rec(apd, i, FALSE);
        SQLLEN *octet_length_ptr;

        assert(aprec);
        octet_length_ptr= ptr_offset_adjust(aprec->octet_length_ptr,
                                            apd->bind_offset_ptr,
                                            apd->bind_type,
                                            sizeof(SQLLEN), 0);

        /* get the "placeholder" pointer the application bound */
        if (IS_DATA_AT_EXEC(octet_length_ptr))
        {
            SQLINTEGER default_size= bind_length(aprec->concise_type,
                                                 aprec->octet_length);
            stmt->current_param= i+1;
            if ( prbgValue )
                *prbgValue= ptr_offset_adjust(aprec->data_ptr,
                                              apd->bind_offset_ptr,
                                              apd->bind_type,
                                              default_size, 0);
            aprec->par.value= NULL;
            aprec->par.alloced= FALSE;
            aprec->par.is_dae= 1;
            return SQL_NEED_DATA;
        }
    }

    /* all data-at-exec params are complete. continue execution */
    switch(stmt->dae_type)
    {
    case DAE_NORMAL:
      if (!SQL_SUCCEEDED(rc= insert_params(stmt, 0, &query, 0)))
        break;
      rc= do_query(stmt, query, 0);
      break;
    case DAE_SETPOS_INSERT:
      stmt->dae_type= DAE_SETPOS_DONE;
      rc= my_SQLSetPos(hstmt, stmt->setpos_row, SQL_ADD, stmt->setpos_lock);
      desc_free(stmt->setpos_apd);
      stmt->setpos_apd= NULL;
      break;
    case DAE_SETPOS_UPDATE:
      stmt->dae_type= DAE_SETPOS_DONE;
      rc= my_SQLSetPos(hstmt, stmt->setpos_row, SQL_UPDATE, stmt->setpos_lock);
      desc_free(stmt->setpos_apd);
      stmt->setpos_apd= NULL;
      break;
    }
    
    stmt->dae_type= 0;
    return rc;
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
    if (stmt->dae_type == DAE_NORMAL)
      aprec= desc_get_rec(stmt->apd, stmt->current_param - 1, FALSE);
    else
      aprec= desc_get_rec(stmt->setpos_apd, stmt->current_param - 1, FALSE);
    assert(aprec);
    if ( cbValue == SQL_NULL_DATA )
    {
        if ( aprec->par.alloced )
        {
            x_free(aprec->par.value);
        }
        aprec->par.alloced= FALSE;
        aprec->par.value= NULL;
        return SQL_SUCCESS;
    }
    if ( aprec->par.value )
    {
        /* Append to old value */
        assert(aprec->par.alloced);
        if ( !(aprec->par.value= my_realloc(aprec->par.value,
                                            aprec->par.value_length + cbValue + 1,
                                            MYF(0))) )
          return set_error(stmt,MYERR_S1001,NULL,4001);

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

  if (!mysql_real_connect(second, dbc->ds->server8, dbc->ds->uid8,
                          dbc->ds->pwd8, NULL, dbc->ds->port,
                          dbc->ds->socket8, 0))
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
