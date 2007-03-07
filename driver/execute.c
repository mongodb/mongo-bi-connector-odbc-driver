/* Copyright (C) 1995-2006 MySQL AB

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   There are special exceptions to the terms and conditions of the GPL as it
   is applied to this software. View the full text of the exception in file
   EXCEPTIONS in the directory of this software distribution.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/***************************************************************************
 * EXECUTE.C								   *
 *									   *
 * @description: Driver code for executing SQL Commands			   *
 *									   *
 * @author     : MySQL AB(monty@mysql.com, venu@mysql.com)		   *
 * @date       : 2001-Aug-15						   *
 * @product    : myodbc3						   *
 *									   *
 ****************************************************************************/

/***************************************************************************
 * The following ODBC APIs are implemented in this file:		   *
 *									   *
 *   SQLExecute		 (ISO 92)					   *
 *   SQLExecDirect	 (ISO 92)					   *
 *   SQLParamData	 (ISO 92)					   *
 *   SQLPutData		 (ISO 92)					   *
 *   SQLCancel		 (ISO 92)					   *
 *									   *
 ****************************************************************************/

#include "myodbc3.h"
#include <locale.h>

/*
  @type    : myodbc3 internal
  @purpose : internal function to execute query and return result
  frees query if query != stmt->query
*/

SQLRETURN do_query(STMT FAR *stmt,char *query)
{
    int error= SQL_ERROR;

    MYODBCDbgEnter;

    if ( !query )
        MYODBCDbgReturnReturn(error);       /* Probably error from insert_param */

    if ( stmt->stmt_options.max_rows && stmt->stmt_options.max_rows !=
         (SQLINTEGER) ~0L )
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
                    my_free((gptr) query,MYF(0));
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
            MYODBCDbgInfo( "affected rows: %d", stmt->affected_rows );
            goto exit;
        }
        MYODBCDbgInfo( "%s", "client failed to return resultset" );
        set_error(stmt,MYERR_S1000,mysql_error(&stmt->dbc->mysql),
                  mysql_errno(&stmt->dbc->mysql));
        goto exit;
    }
    MYODBCDbgInfo( "result set columns: %d", stmt->result->field_count );
    MYODBCDbgInfo( "result set rows: %lld", stmt->result->row_count );
    fix_result_types(stmt);
    error= SQL_SUCCESS;

    exit:
    pthread_mutex_unlock(&stmt->dbc->lock);
    if ( query != stmt->query )
        my_free((gptr) query,MYF(0));

    MYODBCDbgReturnReturn( error );
}



/*
  @type    : myodbc3 internal
  @purpose : help function to enlarge buffer if necessary
*/

char *extend_buffer(NET *net, char *to, ulong length)
{
    ulong need= 0;

    MYODBCDbgEnter;

    MYODBCDbgInfo( "current_length: %ld", (ulong)(to - (char *)net->buff) );
    MYODBCDbgInfo( "length: %ld", (ulong) length );
    MYODBCDbgInfo( "buffer_length: %ld", (ulong) net->max_packet );

    need= (ulong)(to - (char *)net->buff) + length;
    if (!to || need > net->max_packet - 10)
    {
        if (net_realloc(net, need))
        {
            MYODBCDbgReturn(0);
        }

        to= (char *)net->buff + need - length;
    }
    MYODBCDbgReturn(to);
}


/*
  @type    : myodbc3 internal
  @purpose : help function to extend the buffer and copy the data
*/

char *add_to_buffer(NET *net,char *to,char *from,ulong length)
{
    MYODBCDbgEnter;

    MYODBCDbgInfo( "from: '%-.32s'", from ? from: "<null>" );
    MYODBCDbgInfo( "length: %ld", (long int) length );
    if ( !(to= extend_buffer(net,to,length)) )
        MYODBCDbgReturn(0);

    memcpy(to,from,length);

    MYODBCDbgReturn( to+length );
}


/*
  @type    : myodbc3 internal
  @purpose : help function to extend the buffer
*/

static char *extend_escape_buffer(void *net, char *to, ulong *length)
{
    if ( (to= extend_buffer((NET*) net, to, *length)) )
    {
        /*
          Buffer has been extended;  We now need to return in length the
          ammount of space available in the buffer.
          'max_packet' is total length of buffer,
          'to' is next available spot within 'buff' to place data,
          'buff' is start of buffer
          so.... return "total space" less the "amount already used".
        */
        *length= (((NET*) net)->max_packet -
                  (ulong) (to - (char*) ((NET*) net)->buff));
    }
    return to;
}


/*
  @type    : myodbc3 internal
  @purpose : insert sql params at parameter positions
*/

char *insert_params(STMT FAR *stmt)
{
    char *query= stmt->query,*to;
    uint i,length;
    NET *net;

    MYODBCDbgEnter;

    pthread_mutex_lock(&stmt->dbc->lock);
    net= &stmt->dbc->mysql.net;
    to= (char*) net->buff;
    if ( !(stmt->dbc->flag & FLAG_NO_LOCALE) )
        setlocale(LC_NUMERIC,"English");  /* force use of '.' as decimal point */
    for ( i= 0; i < stmt->param_count; i++ )
    {
        PARAM_BIND *param= dynamic_element(&stmt->params,i,PARAM_BIND*);
        char *pos;

        if ( !param->used )
        {
            if ( !(stmt->dbc->flag & FLAG_NO_LOCALE) )
                setlocale(LC_NUMERIC,default_locale);
            set_error(stmt,MYERR_07001,NULL,0);
            pthread_mutex_unlock(&stmt->dbc->lock);
            MYODBCDbgReturn(0);
        }
        pos= param->pos_in_query;
        length= (uint) (pos-query);
        MYODBCDbgInfo( "pos_in_query: %p", pos );
        MYODBCDbgInfo( "query: %p", query );
        if ( !(to= add_to_buffer(net,to,query,length)) )
            goto error;
        query= pos+1;  /* Skipp '?' */
        if ( !(to= insert_param(&stmt->dbc->mysql,to,param)) )
            goto error;
    }
    length= (uint) (stmt->query_end - query);
    if ( !(to= add_to_buffer(net,to,query,length+1)) )
        goto error;
    if ( !(to= (char*) my_memdup((char*) net->buff,
                                 (uint) (to - (char*) net->buff),MYF(0))) )
    {
        if ( !(stmt->dbc->flag & FLAG_NO_LOCALE) )
            setlocale(LC_NUMERIC,default_locale);
        set_error(stmt,MYERR_S1001,NULL,4001);
        pthread_mutex_unlock(&stmt->dbc->lock);
        MYODBCDbgReturn(0);
    }

    if ( stmt->stmt_options.paramProcessedPtr )
        *stmt->stmt_options.paramProcessedPtr= 1; /* We don't support PARAMSET */

    pthread_mutex_unlock(&stmt->dbc->lock);
    if ( !(stmt->dbc->flag & FLAG_NO_LOCALE) )
        setlocale(LC_NUMERIC,default_locale);
    MYODBCDbgReturn(to);

    error:      /* Too much data */
    pthread_mutex_unlock(&stmt->dbc->lock);
    if ( !(stmt->dbc->flag & FLAG_NO_LOCALE) )
        setlocale(LC_NUMERIC,default_locale);
    set_error(stmt,MYERR_S1001,NULL,4001);
    MYODBCDbgReturn(0);
}


/*
  @type    : myodbc3 internal
  @purpose : insert sql param the specified parameter position
*/

char *insert_param(MYSQL *mysql, char *to,PARAM_BIND *param)
{
    int length;
    char buff[128],*data;
    my_bool convert= 0;
    NET *net= &mysql->net;

    if ( !param->actual_len || *(param->actual_len) == SQL_NTS )
    {
        if ( (data= param->buffer) )
        {
            if ( param->actual_len &&  *(param->actual_len) == SQL_NTS )
                length= strlen(data);
            else if ( param->ValueMax )
            {
#ifndef strnlen
                length=strlen(data);
                if ( length > param->ValueMax )
                    length = param->ValueMax;
#else
                length=strnlen(data,param->ValueMax);
#endif
            }
            else
                length=strlen(data);
        }
        else
        {
            MYODBCDbgWarning( "%s", "data is a null pointer" );
            length= 0;     /* This is actually an error */
        }
    }
    else if ( *(param->actual_len) == SQL_NULL_DATA )
    {
        return add_to_buffer(net,to,"NULL",4);
    }
    else if ( *param->actual_len == SQL_DATA_AT_EXEC ||
              *param->actual_len <= SQL_LEN_DATA_AT_EXEC_OFFSET )
    {
        length= param->value_length;
        if ( !(data= param->value) )
            return add_to_buffer(net,to,"NULL",4);
    }
    else
    {
        data= param->buffer;
        length= *param->actual_len;
    }
    MYODBCDbgInfo( "param: 0x%lx", param );
    MYODBCDbgInfo( "ctype: %d", param->CType );
    MYODBCDbgInfo( "SqlType: %d", param->SqlType );
    MYODBCDbgInfo( "data: 0x%lx", data );
    MYODBCDbgInfo( "length: %d", length );
    MYODBCDbgInfo( "actual_len: %d", param->actual_len ? *param->actual_len : 0 );
    MYODBCDbgInfo( "pos_in_query: %p", param->pos_in_query );

    switch ( param->CType )
    {
        case SQL_C_BINARY:
        case SQL_C_CHAR:
            convert= 1;
            break;
        case SQL_C_BIT:
        case SQL_C_TINYINT:
        case SQL_C_STINYINT:
#if MYSQL_VERSION_ID >= 40100
            length= int2str((long) *((signed char*) data),buff,-10,0) -buff;
#else
            length= int2str((long) *((signed char*) data),buff,-10) -buff;
#endif
            data= buff;
            break;
        case SQL_C_UTINYINT:
#if MYSQL_VERSION_ID >= 40100
            length= int2str((long) *((unsigned char*) data),buff,-10,0) -buff;
#else
            length= int2str((long) *((unsigned char*) data),buff,-10) -buff;
#endif
            data= buff;
            break;
        case SQL_C_SHORT:
        case SQL_C_SSHORT:
#if MYSQL_VERSION_ID >= 40100
            length= int2str((long) *((short int*) data),buff,-10,0) -buff;
#else
            length= int2str((long) *((short int*) data),buff,-10) -buff;
#endif
            data= buff;
            break;
        case SQL_C_USHORT:
#if MYSQL_VERSION_ID >= 40100
            length= int2str((long) *((unsigned short int*) data),buff,-10,0) -buff;
#else
            length= int2str((long) *((unsigned short int*) data),buff,-10) -buff;
#endif
            data= buff;
            break;
        case SQL_C_LONG:
        case SQL_C_SLONG:
#if MYSQL_VERSION_ID >= 40100
            length= int2str(*((SQLINTEGER*) data),buff,-10,0) -buff;
#else
            length= int2str(*((SQLINTEGER*) data),buff,-10) -buff;
#endif
            data= buff;
            break;
        case SQL_C_ULONG:
#if MYSQL_VERSION_ID >= 40100
            length= int2str(*((SQLUINTEGER*) data),buff,10,0) -buff;
#else
            length= int2str(*((SQLUINTEGER*) data),buff,10) -buff;
#endif
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
            if ( param->SqlType != SQL_NUMERIC && param->SqlType != SQL_DECIMAL )
                sprintf(buff,"%.17e",*((float*) data));
            else
                /* We should perpare this data for string comparison */
                sprintf(buff,"%.15e",*((float*) data));
            length= strlen(data= buff);
            break;
        case SQL_C_DOUBLE:
            if ( param->SqlType != SQL_NUMERIC && param->SqlType != SQL_DECIMAL )
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
                sprintf(buff,"%04d%02d%02d",date->year,date->month,date->day);
                data= buff;
                length= 8;
                break;
            }
        case SQL_C_TIME:
        case SQL_C_TYPE_TIME:
            {
                TIME_STRUCT *time= (TIME_STRUCT*) data;
                sprintf(buff,"%02d%02d%02d",time->hour,time->minute,time->second);
                data= buff;
                length= 6;
                break;
            }
        case SQL_C_TIMESTAMP:
        case SQL_C_TYPE_TIMESTAMP:
            {
                TIMESTAMP_STRUCT *time= (TIMESTAMP_STRUCT*) data;
                sprintf(buff,"%04d%02d%02d%02d%02d%02d",time->year,time->month,time->day,
                        time->hour,time->minute,time->second);
                data= buff;
                length= 14;
                break;
            }
    }
    switch ( param->SqlType )
    {
        case SQL_DATE:
        case SQL_TYPE_DATE:
        case SQL_TYPE_TIMESTAMP:
        case SQL_TIMESTAMP:
            if ( data[0] == '{' )       /* Of type {d date } */
                return add_to_buffer(net,to,data,length);
            /* else threat as a string */
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_LONGVARCHAR:
        case SQL_BINARY:
        case SQL_VARBINARY:
        case SQL_LONGVARBINARY:
        case SQL_WCHAR:
            {
                to= add_to_buffer(net,to,"'",1);
                to= mysql_odbc_escape_string(mysql,
                                             to, (net->max_packet -
                                                  (ulong) (to - (char*) net->buff)),
                                             data, length,
                                             (void*) net, extend_escape_buffer);
                if ( to ) /* escape was ok */
                {
                    to= add_to_buffer(net,to,"'",1);
                }
                return to;
            }
        case SQL_TIME:
        case SQL_TYPE_TIME:
            if ( param->CType == SQL_C_TIMESTAMP ||
                 param->CType == SQL_C_TYPE_TIMESTAMP )
            {
                TIMESTAMP_STRUCT *time= (TIMESTAMP_STRUCT*) param->buffer;
                sprintf(buff,"'%02d:%02d:%02d'",time->hour,time->minute,time->second);
                return add_to_buffer(net,to,buff,10);
            }
            else
            {
                ulong time= str_to_time_as_long(data,length);
                sprintf(buff,"'%02d:%02d:%02d'",
                        (int) time/10000,
                        (int) time/100%100,
                        (int) time%100);
                return add_to_buffer(net,to,buff,10);
            }
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
            }
            /* Fall through */
        default:
            return add_to_buffer(net,to,data,length);
    }
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
    MYODBCDbgEnter;
    MYODBCDbgReturnReturn( my_SQLExecute( (STMT FAR*)hstmt ) );
}


/*
  @type    : myodbc3 internal
  @purpose : executes a prepared statement, using the current values
  of the parameter marker variables if any parameter markers
  exist in the statement
*/

SQLRETURN my_SQLExecute( STMT FAR *pStmt )
{
    char *      query;
    uint        i;
    uint        nIndex;
    PARAM_BIND *param;
    STMT FAR *  pStmtCursor = pStmt;

    MYODBCDbgEnter;

    MYODBCDbgInfo( "pStmt: 0x%lx", pStmt );

    if ( !pStmt )
        MYODBCDbgReturnReturn( SQL_ERROR );

    CLEAR_STMT_ERROR( pStmt );

    if ( !pStmt->query )
        MYODBCDbgReturnReturn( set_error( pStmt, MYERR_S1010, "No previous SQLPrepare done", 0 ) );

    if ( check_if_positioned_cursor_exists( pStmt, &pStmtCursor ) )
    {
        MYODBCDbgReturnReturn( do_my_pos_cursor( pStmt, pStmtCursor ) );
    }

    for ( nIndex= 0 ; nIndex < pStmt->param_count ; )
    {
        param= dynamic_element(&pStmt->params,nIndex++,PARAM_BIND*);
        if ( param->real_param_done == FALSE && param->used == 1 )
        {
            pthread_mutex_lock(&pStmt->dbc->lock);
            mysql_free_result(pStmt->result);
            pthread_mutex_unlock(&pStmt->dbc->lock);
            break;
        }
    }

    if ( pStmt->dummy_state == ST_DUMMY_EXECUTED )
        pStmt->state= ST_PREPARED;
    if ( pStmt->state == ST_PRE_EXECUTED )
    {
        pStmt->state= ST_EXECUTED;
        MYODBCDbgReturnReturn(SQL_SUCCESS);
    }
    my_SQLFreeStmt((SQLHSTMT)pStmt,MYSQL_RESET_BUFFERS);
    query= pStmt->query;

    if ( pStmt->stmt_options.paramProcessedPtr )
        *pStmt->stmt_options.paramProcessedPtr= 0;

    if ( pStmt->param_count )
    {
        /*
         * If any parameters are required at execution time, cannot perform the
         * statement. It will be done throught SQLPutData() and SQLParamData().
         */
        for ( i= 0; i < pStmt->param_count; i++ )
        {
            PARAM_BIND *param= dynamic_element(&pStmt->params,i,PARAM_BIND*);
            if ( param->actual_len &&
                 (*param->actual_len == (long) SQL_DATA_AT_EXEC ||
                  *param->actual_len <= SQL_LEN_DATA_AT_EXEC_OFFSET) )
            {
                pStmt->current_param= i;      /* Fix by Giovanni */
                param->value= 0;
                param->alloced= 0;
                MYODBCDbgReturnReturn(SQL_NEED_DATA);
            }
        }
        query= insert_params(pStmt);     /* Checked in do_query */
    }

    MYODBCDbgReturnReturn(do_query(pStmt, query));
}


/*
  @type    : ODBC 1.0 API
  @purpose : executes a preparable statement, using the current values of
  the parameter marker variables if any parameters exist in the
  statement
*/

SQLRETURN SQL_API SQLExecDirect(SQLHSTMT hstmt,
                                SQLCHAR FAR *szSqlStr,
                                SQLINTEGER cbSqlStr)
{
    int error;

    MYODBCDbgEnter;

    if ( (error= my_SQLPrepare(hstmt,szSqlStr,cbSqlStr)) )
        MYODBCDbgReturnReturn(error);
    error= my_SQLExecute( (STMT FAR*)hstmt );

    MYODBCDbgReturnReturn(error);
}


/*
  @type    : ODBC 1.0 API
  @purpose : returns the SQL string as modified by the driver
*/

SQLRETURN SQL_API SQLNativeSql(SQLHDBC hdbc,
                               SQLCHAR FAR *szSqlStrIn,
                               SQLINTEGER cbSqlStrIn,
                               SQLCHAR FAR *szSqlStr,
                               SQLINTEGER cbSqlStrMax,
                               SQLINTEGER *pcbSqlStr)
{
    ulong offset= 0;

    MYODBCDbgEnter;

    MYODBCDbgReturnReturn( copy_lresult(SQL_HANDLE_DBC, hdbc,
                                 szSqlStr,cbSqlStrMax,
                                 (SQLLEN *)pcbSqlStr,
                                 (char*) szSqlStrIn, cbSqlStrIn,0L,0L,
                                 &offset,0));
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

    MYODBCDbgEnter;

    for ( i= stmt->current_param; i < stmt->param_count; i++ )
    {
        PARAM_BIND *param= dynamic_element(&stmt->params,i,PARAM_BIND*);
        if ( param->actual_len &&
             (*param->actual_len == (long) SQL_DATA_AT_EXEC ||
              *param->actual_len <= SQL_LEN_DATA_AT_EXEC_OFFSET) )
        {
            stmt->current_param= i+1;
            if ( prbgValue )
                *prbgValue= param->buffer;
            param->value= 0;
            param->alloced= 0;
            MYODBCDbgReturnReturn(SQL_NEED_DATA);
        }
    }
    MYODBCDbgReturnReturn(do_query(stmt,insert_params(stmt)));
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
    PARAM_BIND *param;

    MYODBCDbgEnter;

    if ( !stmt )
        MYODBCDbgReturnReturn(SQL_ERROR);

    if ( cbValue == SQL_NTS )
        cbValue= strlen(rgbValue);
    param= dynamic_element(&stmt->params,stmt->current_param-1,PARAM_BIND*);
    if ( cbValue == SQL_NULL_DATA )
    {
        if ( param->alloced )
            my_free(param->value,MYF(0));
        param->alloced= 0;
        param->value= 0;
        MYODBCDbgReturnReturn(SQL_SUCCESS);
    }
    if ( param->value )
    {
        /* Append to old value */
        if ( param->alloced )
        {
            if ( !(param->value= my_realloc(param->value,
                                            param->value_length + cbValue + 1,
                                            MYF(0))) )
                MYODBCDbgReturnReturn(set_error(stmt,MYERR_S1001,NULL,4001));
        }
        else
        {
            /* This should never happen */
            gptr old_pos= param->value;
            if ( !(param->value= my_malloc(param->value_length+cbValue+1,MYF(0))) )
                MYODBCDbgReturnReturn(set_error(stmt,MYERR_S1001,NULL,4001));
            memcpy(param->value,old_pos,param->value_length);
        }
        memcpy(param->value+param->value_length,rgbValue,cbValue);
        param->value_length+= cbValue;
        param->value[param->value_length]= 0;
        param->alloced= 1;
    }
    else
    {
        /* New value */
        if ( !(param->value= my_malloc(cbValue+1,MYF(0))) )
            MYODBCDbgReturnReturn(set_error(stmt,MYERR_S1001,NULL,4001));
        memcpy(param->value,rgbValue,cbValue);
        param->value_length= cbValue;
        param->value[param->value_length]= 0;
        param->alloced= 1;
    }
    MYODBCDbgReturnReturn(SQL_SUCCESS);
}


/*
  @type    : ODBC 1.0 API
  @purpose : cancels the processing on a statement
*/

SQLRETURN SQL_API SQLCancel(SQLHSTMT hstmt)
{
    MYODBCDbgEnter;
    MYODBCDbgReturnReturn(my_SQLFreeStmt(hstmt,SQL_CLOSE));
}
