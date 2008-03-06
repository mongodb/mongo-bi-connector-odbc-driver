/*
  Copyright (C) 1995-2006 MySQL AB

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
  @file  error.c
  @brief Error handling functions.
*/

/***************************************************************************
 * The following ODBC APIs are implemented in this file:		   *
 *									   *
 *   SQLGetDiagField	 (ISO 92)					   *
 *   SQLGetDiagRec	 (ISO 92)					   *
 *   SQLError		 (ODBC, Deprecated)				   *
 *									   *
 ****************************************************************************/

#include "myodbc3.h"
#include "mysqld_error.h"
#include "errmsg.h"

/*
  @type    : myodbc3 internal error structure
  @purpose : set of internal errors, in the following order
  - SQLSTATE2 (when version is SQL_OV_ODBC2)
  - SQLSTATE3 (when version is SQL_OV_ODBC3)
  - error message text
  - return code
*/

static MYODBC3_ERR_STR myodbc3_errors[]=
{
  {"01000","General warning", SQL_SUCCESS_WITH_INFO},
  {"01004","String data, right truncated", SQL_SUCCESS_WITH_INFO},
  {"01S02","Option value changed", SQL_SUCCESS_WITH_INFO},
  {"01S03","No rows updated/deleted", SQL_SUCCESS_WITH_INFO},
  {"01S04","More than one row updated/deleted", SQL_SUCCESS_WITH_INFO},
  {"01S06","Attempt to fetch before the result set returned the first rowset",
    SQL_SUCCESS_WITH_INFO},
  {"07001","SQLBindParameter not used for all parameters", SQL_ERROR},
  {"07005","Prepared statement not a cursor-specification", SQL_ERROR},
  {"07006","Restricted data type attribute violation", SQL_ERROR},
  {"07009","Invalid descriptor index", SQL_ERROR},
  {"08002","Connection name in use", SQL_ERROR},
  {"08003","Connection does not exist", SQL_ERROR},
  {"24000","Invalid cursor state", SQL_ERROR},
  {"25000","Invalid transaction state", SQL_ERROR},
  {"25S01","Transaction state unknown", SQL_ERROR},
  {"34000","Invalid cursor name", SQL_ERROR},
  {"HYT00","Timeout expired", SQL_ERROR},
  {"HY000","General driver defined error", SQL_ERROR},
  {"HY001","Memory allocation error", SQL_ERROR},
  {"HY002","Invalid column number", SQL_ERROR},
  {"HY003","Invalid application buffer type", SQL_ERROR},
  {"HY004","Invalid SQL data type", SQL_ERROR},
  {"HY009","Invalid use of null pointer", SQL_ERROR},
  {"HY010","Function sequence error", SQL_ERROR},
  {"HY011","Attribute can not be set now", SQL_ERROR},
  {"HY012","Invalid transaction operation code", SQL_ERROR},
  {"HY013","Memory management error", SQL_ERROR},
  {"HY015","No cursor name available", SQL_ERROR},
  {"HY024","Invalid attribute value", SQL_ERROR},
  {"HY090","Invalid string or buffer length", SQL_ERROR},
  {"HY091","Invalid descriptor field identifier", SQL_ERROR},
  {"HY092","Invalid attribute/option identifier", SQL_ERROR},
  {"HY093","Invalid parameter number", SQL_ERROR},
  {"HY095","Function type out of range", SQL_ERROR},
  {"HY106","Fetch type out of range", SQL_ERROR},
  {"HY107","Row value out of range", SQL_ERROR},
  {"HY109","Invalid cursor position", SQL_ERROR},
  {"HYC00","Optional feature not implemented", SQL_ERROR},
  /* server related..*/
  {"21S01","Column count does not match value count", SQL_ERROR},
  {"23000","Integrity constraint violation", SQL_ERROR},
  {"42000","Syntax error or access violation", SQL_ERROR},
  {"42S01","Base table or view already exists ", SQL_ERROR},
  {"42S02","Base table or view not found", SQL_ERROR},
  {"42S12","Index not found", SQL_ERROR},
  {"42S21","Column already exists", SQL_ERROR},
  {"42S22","Column not found", SQL_ERROR},
  {"08S01","Communication link failure", SQL_ERROR},
};


/*
  @type    : myodbc3 internal
  @purpose : If ODBC version is SQL_OV_ODBC2, initialize old SQLSTATEs
*/

void myodbc_sqlstate2_init(void)
{
    /*
      As accessing the SQLSTATE2 is very rare, set this to
      global when required .. one time ..for quick processing
      in set_error/set_conn_error
    */
    uint  i;
    for ( i= MYERR_S1000; i <= MYERR_S1C00; i++ )
    {
        myodbc3_errors[i].sqlstate[0]= 'S';
        myodbc3_errors[i].sqlstate[1]= '1';
    }
    strmov(myodbc3_errors[MYERR_07005].sqlstate,"24000");
    strmov(myodbc3_errors[MYERR_42000].sqlstate,"37000");
    strmov(myodbc3_errors[MYERR_42S01].sqlstate,"S0001");
    strmov(myodbc3_errors[MYERR_42S02].sqlstate,"S0002");
    strmov(myodbc3_errors[MYERR_42S12].sqlstate,"S0012");
    strmov(myodbc3_errors[MYERR_42S21].sqlstate,"S0021");
    strmov(myodbc3_errors[MYERR_42S22].sqlstate,"S0022");
}


/*
  @type    : myodbc3 internal
  @purpose : If ODBC version is SQL_OV_ODBC3, initialize to original SQLSTATEs
*/

void myodbc_sqlstate3_init(void)
{
    uint i;

    for ( i= MYERR_S1000; i <= MYERR_S1C00; i++ )
    {
        myodbc3_errors[i].sqlstate[0]= 'H';
        myodbc3_errors[i].sqlstate[1]= 'Y';
    }
    strmov(myodbc3_errors[MYERR_07005].sqlstate,"07005");
    strmov(myodbc3_errors[MYERR_42000].sqlstate,"42000");
    strmov(myodbc3_errors[MYERR_42S01].sqlstate,"42S01");
    strmov(myodbc3_errors[MYERR_42S02].sqlstate,"42S02");
    strmov(myodbc3_errors[MYERR_42S12].sqlstate,"42S12");
    strmov(myodbc3_errors[MYERR_42S21].sqlstate,"42S21");
    strmov(myodbc3_errors[MYERR_42S22].sqlstate,"42S22");
}


/*
  @type    : myodbc3 internal
  @purpose : copies error from one handdle to other
*/

SQLRETURN copy_stmt_error(STMT FAR *dst,STMT FAR *src)
{
    strmov(dst->error.sqlstate,src->error.sqlstate);
    strmov(dst->error.message, src->error.message);
    dst->error.native_error= src->error.native_error;
    dst->error.retcode= src->error.retcode;
    return(SQL_SUCCESS);
}


/*
  @type    : myodbc3 internal
  @purpose : sets the connection level errors, which doesn't need any
  conversion
*/

SQLRETURN set_dbc_error(DBC FAR *dbc,char *state,
                        const char *message,uint errcode)
{
    strmov(dbc->error.sqlstate,state);
    strxmov(dbc->error.message,MYODBC3_ERROR_PREFIX,message,NullS);
    dbc->error.native_error= errcode;
    return SQL_ERROR;
}


/*
  @type    : myodbc3 internal
  @purpose : sets the statement level errors, which doesn't need any
  conversion
*/

SQLRETURN set_stmt_error( STMT FAR *    stmt,
                          char *        state,
                          const char *  message,
                          uint          errcode )
{
    strmov( stmt->error.sqlstate, state );
    strxmov( stmt->error.message, stmt->dbc->st_error_prefix, message, NullS );
    stmt->error.native_error = errcode;

    return SQL_ERROR;
}


/*
  @type    : myodbc3 internal
  @purpose : translates SQL error to ODBC error
*/

void translate_error(char *save_state,myodbc_errid errid,uint mysql_err)
{
    char *state= myodbc3_errors[errid].sqlstate;

    switch ( mysql_err )
    {
        case ER_WRONG_VALUE_COUNT:
            state= "21S01";
            break;
        case ER_DUP_ENTRY:
        case ER_DUP_KEY:
            state= "23000";
            break;
        case ER_NO_DB_ERROR:
            state= "3D000";
            break;
        case ER_PARSE_ERROR:
#ifdef ER_SP_DOES_NOT_EXIST
        case ER_SP_DOES_NOT_EXIST:
#endif
            state= myodbc3_errors[MYERR_42000].sqlstate;
            break;
        case ER_TABLE_EXISTS_ERROR:
            state= myodbc3_errors[MYERR_42S01].sqlstate;
            break;
        case ER_FILE_NOT_FOUND:
        case ER_NO_SUCH_TABLE:
        case ER_CANT_OPEN_FILE:
        case ER_BAD_TABLE_ERROR:
            state= myodbc3_errors[MYERR_42S02].sqlstate;
            break;
        case ER_NO_SUCH_INDEX:
        case ER_CANT_DROP_FIELD_OR_KEY:
            state= myodbc3_errors[MYERR_42S12].sqlstate;
            break;
        case ER_DUP_FIELDNAME:
            state= myodbc3_errors[MYERR_42S21].sqlstate;
            break;
        case ER_BAD_FIELD_ERROR:
            state= myodbc3_errors[MYERR_42S22].sqlstate;
            break;
        case CR_SERVER_HANDSHAKE_ERR:
        case CR_CONNECTION_ERROR:
        case CR_SERVER_GONE_ERROR:
        case CR_SERVER_LOST:
            state= "08S01";
            break;
        default: break;
    }
    strmov(save_state,state);
}


/*
  @type    : myodbc3 internal
  @purpose : copy error to error structure
*/

static SQLRETURN copy_error(MYERROR *error, myodbc_errid errid,
                            const char *errtext, SQLINTEGER errcode,
                            char *prefix)
{
    SQLRETURN   sqlreturn;
    SQLCHAR     *errmsg;
    SQLINTEGER  code;

    errmsg= (errtext ? (SQLCHAR *)errtext :
             (SQLCHAR *)myodbc3_errors[errid].message);
    code=   errcode ? (myodbc_errid) errcode : errid + MYODBC_ERROR_CODE_START;

    sqlreturn= error->retcode= myodbc3_errors[errid].retcode;  /* RETCODE */
    error->native_error= code;                     /* NATIVE */
    strmov(error->sqlstate, myodbc3_errors[errid].sqlstate);   /* SQLSTATE */
    strxmov(error->message,prefix,errmsg,NullS);           /* MESSAGE */

    return sqlreturn;
}


/*
  @type    : myodbc3 internal
  @purpose : sets the error information to environment handle.
*/

SQLRETURN set_env_error(ENV *env, myodbc_errid errid, const char *errtext,
                        SQLINTEGER errcode)
{
    return copy_error(&env->error,errid,errtext,errcode,MYODBC3_ERROR_PREFIX);
}


/*
  @type    : myodbc3 internal
  @purpose : sets the error information to connection handle.
*/

SQLRETURN set_conn_error(DBC *dbc, myodbc_errid errid, const char *errtext,
                         SQLINTEGER errcode)
{
    return copy_error(&dbc->error,errid,errtext,errcode,MYODBC3_ERROR_PREFIX);
}


/*
  @type    : myodbc3 internal
  @purpose : sets the error information to statement handle.
*/

SQLRETURN set_error(STMT *stmt, myodbc_errid errid, const char *errtext,
                    SQLINTEGER errcode)
{
    return copy_error(&stmt->error,errid,errtext,errcode,
                      stmt->dbc->st_error_prefix);
}


/*
  @type    : myodbc3 internal
  @purpose : sets a my_malloc() failure on a MYSQL* connection
*/
void set_mem_error(MYSQL *mysql)
{
  mysql->net.last_errno= CR_OUT_OF_MEMORY;
  strmov(mysql->net.last_error, "Memory allocation failed");
  strmov(mysql->net.sqlstate, "HY001");
}


/**
  Handle a connection-related error.

  @param[in]  stmt  Statement
*/
SQLRETURN handle_connection_error(STMT *stmt)
{
  unsigned int err= mysql_errno(&stmt->dbc->mysql);
  switch (err) {
  case CR_SERVER_GONE_ERROR:
  case CR_SERVER_LOST:
    return set_stmt_error(stmt, "08S01", mysql_error(&stmt->dbc->mysql), err);
  case CR_OUT_OF_MEMORY:
    return set_stmt_error(stmt, "HY001", mysql_error(&stmt->dbc->mysql), err);
  case CR_COMMANDS_OUT_OF_SYNC:
  case CR_UNKNOWN_ERROR:
  default:
    return set_stmt_error(stmt, "HY000", mysql_error(&stmt->dbc->mysql), err);
  }
}


/*
  @type    : myodbc3 internal
  @purpose : sets the error information to appropriate handle.
  it also sets the SQLSTATE based on the ODBC VERSION
*/

SQLRETURN set_handle_error(SQLSMALLINT HandleType, SQLHANDLE handle,
                           myodbc_errid errid, const char *errtext,
                           SQLINTEGER errcode)
{
    switch ( HandleType )
    {
        
        case SQL_HANDLE_ENV:
            return copy_error(&((ENV *)handle)->error,errid,errtext,
                              errcode,MYODBC3_ERROR_PREFIX);

        case SQL_HANDLE_DBC:
            return copy_error(&((DBC *)handle)->error,errid,errtext,
                              errcode,MYODBC3_ERROR_PREFIX);

        default:
            return copy_error(&((STMT *)handle)->error,errid,errtext,
                              errcode,((STMT *)handle)->dbc->st_error_prefix);
    }
}


/*
  @type    : myodbc3 internal
  @purpose : returns the current values of multiple fields of a diagnostic
  record that contains error, warning, and status information.
  Unlike SQLGetDiagField, which returns one diagnostic field
  per call, SQLGetDiagRec returns several commonly used fields
  of a diagnostic record, including the SQLSTATE, the native
  error code, and the diagnostic message text
*/

SQLRETURN my_SQLGetDiagRec(SQLSMALLINT HandleType,
                           SQLHANDLE Handle,
                           SQLSMALLINT RecNumber,
                           SQLCHAR *Sqlstate,
                           SQLINTEGER  *NativeErrorPtr,
                           SQLCHAR *MessageText,
                           SQLSMALLINT BufferLength,
                           SQLSMALLINT *TextLengthPtr)
{
    char *errmsg,tmp_state[6];
    SQLSMALLINT tmp_size;
    SQLINTEGER  tmp_error;

    if ( !TextLengthPtr )
        TextLengthPtr= &tmp_size;

    if ( !Sqlstate )
        Sqlstate= (SQLCHAR*) tmp_state;

    if ( !NativeErrorPtr )
        NativeErrorPtr= &tmp_error;

    if ( RecNumber <= 0 || BufferLength < 0 || !Handle )
        return SQL_ERROR;

    /*
      Currently we are not supporting error list, so
      if RecNumber > 1, return no data found
    */

    if ( RecNumber > 1 )
        return SQL_NO_DATA_FOUND;

    switch ( HandleType )
    {
        case SQL_HANDLE_STMT:
            errmsg= ((STMT FAR*) Handle)->error.message;
            strmov((char*) Sqlstate,((STMT FAR*) Handle)->error.sqlstate);
            *NativeErrorPtr= ((STMT FAR*) Handle)->error.native_error;
            break;

        case SQL_HANDLE_DBC:
            errmsg= ((DBC FAR*) Handle)->error.message;
            strmov((char*) Sqlstate,((DBC FAR*) Handle)->error.sqlstate);
            *NativeErrorPtr= ((DBC FAR*) Handle)->error.native_error;

            break;

        case SQL_HANDLE_ENV:
            errmsg= ((ENV FAR*) Handle)->error.message;
            strmov((char*) Sqlstate,((ENV FAR*) Handle)->error.sqlstate);
            *NativeErrorPtr= ((ENV FAR*) Handle)->error.native_error;
            break;

        default:
            return SQL_INVALID_HANDLE;
    }
    if ( !errmsg || !errmsg[0] )
    {
        *TextLengthPtr= 0;
        strmov((char*) Sqlstate, "00000");
        return SQL_NO_DATA_FOUND;
    }

    return copy_str_data(HandleType, Handle, MessageText, BufferLength,
                         TextLengthPtr, (char *)errmsg);
}


/*
  @type    : ODBC 3.0 API
  @purpose : returns the current value of a field of a record of the
  diagnostic data structure (associated with a specified handle)
  that contains error, warning, and status information
*/

SQLRETURN SQL_API SQLGetDiagField(SQLSMALLINT HandleType,
                                  SQLHANDLE   Handle,
                                  SQLSMALLINT RecNumber,
                                  SQLSMALLINT DiagIdentifier,
                                  SQLPOINTER  DiagInfoPtr,
                                  SQLSMALLINT BufferLength,
                                  SQLSMALLINT *StringLengthPtr)
{
    SQLRETURN   error= SQL_SUCCESS;
    SQLPOINTER  szDiagInfo= NULL;
    SQLSMALLINT tmp_size;

    if ( !StringLengthPtr )
        StringLengthPtr= &tmp_size;

    if ( !DiagInfoPtr )
        DiagInfoPtr= szDiagInfo;

    if ( !Handle ||
         !(HandleType == SQL_HANDLE_STMT ||
           HandleType == SQL_HANDLE_DBC ||
           HandleType == SQL_HANDLE_ENV) )
        return SQL_ERROR;

    if ( RecNumber > 1 )
        return SQL_NO_DATA_FOUND;

    switch ( DiagIdentifier )
    {

        /* DIAG HEADER FIELDS SECTION */
        case SQL_DIAG_DYNAMIC_FUNCTION:
            if ( HandleType != SQL_HANDLE_STMT )
                return SQL_ERROR;

            error= copy_str_data(HandleType, Handle, DiagInfoPtr, BufferLength,
                                 StringLengthPtr, "");
            break;

        case SQL_DIAG_DYNAMIC_FUNCTION_CODE:
            *(SQLINTEGER *) DiagInfoPtr= 0;
            break;

        case SQL_DIAG_ROW_NUMBER:
            *(SQLINTEGER *) DiagInfoPtr= SQL_ROW_NUMBER_UNKNOWN;
            break;

        case SQL_DIAG_NUMBER:
            *(SQLINTEGER *) DiagInfoPtr= 1;
            break;

        case SQL_DIAG_RETURNCODE:
            if ( HandleType == SQL_HANDLE_STMT )
                *(SQLRETURN *) DiagInfoPtr= ((STMT FAR*) Handle)->error.retcode;

            else if ( HandleType == SQL_HANDLE_DBC )
                *(SQLRETURN *) DiagInfoPtr= ((DBC FAR*) Handle)->error.retcode;

            else
                *(SQLRETURN *) DiagInfoPtr= ((ENV FAR*) Handle)->error.retcode;
            break;

        case SQL_DIAG_CURSOR_ROW_COUNT:/* at present, return total rows in rs */
            if ( HandleType != SQL_HANDLE_STMT )
                return SQL_ERROR;

            if ( !((STMT FAR*) Handle)->result )
                *(SQLINTEGER *) DiagInfoPtr= 0;

            else
                *(SQLINTEGER *) DiagInfoPtr= (SQLINTEGER)
                                             mysql_num_rows(((STMT FAR*)  Handle)->result);
            break;

        case SQL_DIAG_ROW_COUNT:
            if ( HandleType != SQL_HANDLE_STMT )
                return SQL_ERROR;

            *(SQLINTEGER *) DiagInfoPtr= (SQLINTEGER)
                                         ((STMT FAR*)  Handle)->affected_rows;
            break;

            /* DIAG RECORD FIELDS SECTION */

        case SQL_DIAG_CLASS_ORIGIN:
        case SQL_DIAG_SUBCLASS_ORIGIN:
            error= copy_str_data(HandleType, Handle, DiagInfoPtr, BufferLength,
                                 StringLengthPtr, "ISO 9075");
            break;

        case SQL_DIAG_COLUMN_NUMBER:
            *(SQLINTEGER *) DiagInfoPtr= SQL_COLUMN_NUMBER_UNKNOWN;
            break;

        case SQL_DIAG_CONNECTION_NAME:
            /*
              When the connection fails, ODBC DM calls this function, so don't
              return dbc->dsn as the connection name instead return empty string
            */

            if ( HandleType == SQL_HANDLE_STMT )
                error= copy_str_data(HandleType, Handle, DiagInfoPtr, BufferLength,
                                     StringLengthPtr,((STMT FAR*) Handle)->dbc->dsn ?
                                     ((STMT FAR*) Handle)->dbc->dsn:"");

            else if ( HandleType == SQL_HANDLE_DBC )
                error= copy_str_data(HandleType, Handle, DiagInfoPtr, BufferLength,
                                     StringLengthPtr,((DBC FAR*) Handle)->dsn ?
                                     ((DBC FAR*) Handle)->dsn:"");

            else
            {
                *(SQLCHAR *) DiagInfoPtr= 0;
                *StringLengthPtr= 0;
            }
            break;

        case SQL_DIAG_MESSAGE_TEXT:
            if ( HandleType == SQL_HANDLE_STMT )
                error= copy_str_data(HandleType, Handle, DiagInfoPtr, BufferLength,
                                     StringLengthPtr,
                                     ((STMT FAR*) Handle)->error.message);

            else if ( HandleType == SQL_HANDLE_DBC )
                error= copy_str_data(HandleType, Handle, DiagInfoPtr, BufferLength,
                                     StringLengthPtr,((DBC FAR*) Handle)->error.message);
            else
                error= copy_str_data(HandleType, Handle, DiagInfoPtr, BufferLength,
                                     StringLengthPtr,((ENV FAR*) Handle)->error.message);
            break;

        case SQL_DIAG_NATIVE:
            if ( HandleType == SQL_HANDLE_STMT )
                *(SQLINTEGER *) DiagInfoPtr= ((STMT FAR*) Handle)->error.native_error;

            else if ( HandleType == SQL_HANDLE_DBC )
                *(SQLINTEGER *) DiagInfoPtr= ((DBC FAR*) Handle)->error.native_error;

            else
                *(SQLINTEGER *) DiagInfoPtr= ((ENV FAR*) Handle)->error.native_error;
            break;

        case SQL_DIAG_SERVER_NAME:
            if ( HandleType == SQL_HANDLE_STMT )
                error= copy_str_data(HandleType, Handle, DiagInfoPtr, BufferLength,
                                     StringLengthPtr,
                                     ((STMT FAR*) Handle)->dbc->server ?
                                     ((STMT FAR*) Handle)->dbc->server : "");

            else if ( HandleType == SQL_HANDLE_DBC )
                error= copy_str_data(HandleType, Handle, DiagInfoPtr, BufferLength,
                                     StringLengthPtr,((DBC FAR*) Handle)->server ?
                                     ((DBC FAR*) Handle)->server : "");
            else
            {
                *(SQLCHAR *) DiagInfoPtr= 0;
                *StringLengthPtr= 0;
            }
            break;

        case SQL_DIAG_SQLSTATE:
            if ( HandleType == SQL_HANDLE_STMT )
                error= copy_str_data(HandleType, Handle, DiagInfoPtr, BufferLength,
                                     StringLengthPtr,
                                     ((STMT FAR*) Handle)->error.sqlstate);

            else if ( HandleType == SQL_HANDLE_DBC )
                error= copy_str_data(HandleType, Handle, DiagInfoPtr, BufferLength,
                                     StringLengthPtr,
                                     ((DBC FAR*) Handle)->error.sqlstate);

            else
                error= copy_str_data(HandleType, Handle, DiagInfoPtr, BufferLength,
                                     StringLengthPtr,
                                     ((ENV FAR*) Handle)->error.sqlstate);
            break;

        default:
            return SQL_ERROR;
    }
    return error;
}


/*
  @type    : ODBC 3.0 API
  @purpose : returns the current diagnostic record information
*/

SQLRETURN SQL_API SQLGetDiagRec(SQLSMALLINT HandleType,
                                SQLHANDLE   Handle,
                                SQLSMALLINT RecNumber,
                                SQLCHAR     *Sqlstate,
                                SQLINTEGER  *NativeErrorPtr,
                                SQLCHAR     *MessageText,
                                SQLSMALLINT BufferLength,
                                SQLSMALLINT *TextLengthPtr)
{
    return my_SQLGetDiagRec(HandleType, Handle, RecNumber, Sqlstate,
                            NativeErrorPtr, MessageText, BufferLength,
                            TextLengthPtr);
}


/*
  @type    : ODBC 1.0 API - depricated
  @purpose : returns error or status information
*/

SQLRETURN SQL_API SQLError(SQLHENV henv, SQLHDBC hdbc, SQLHSTMT hstmt,
                           SQLCHAR FAR    *szSqlState,
                           SQLINTEGER FAR *pfNativeError,
                           SQLCHAR FAR    *szErrorMsg,
                           SQLSMALLINT    cbErrorMsgMax,
                           SQLSMALLINT FAR *pcbErrorMsg)
{
    SQLRETURN error= SQL_INVALID_HANDLE;

    if ( hstmt )
    {
        error= my_SQLGetDiagRec(SQL_HANDLE_STMT,hstmt,1,szSqlState,
                                pfNativeError, szErrorMsg,
                                cbErrorMsgMax,pcbErrorMsg);
        if ( error == SQL_SUCCESS )
            CLEAR_STMT_ERROR(hstmt);
    }
    else if ( hdbc )
    {
        error= my_SQLGetDiagRec(SQL_HANDLE_DBC,hdbc,1,szSqlState,
                                pfNativeError, szErrorMsg,
                                cbErrorMsgMax,pcbErrorMsg);
        if ( error == SQL_SUCCESS )
            CLEAR_DBC_ERROR(hdbc);
    }
    else if ( henv )
    {
        error= my_SQLGetDiagRec(SQL_HANDLE_ENV,henv,1,szSqlState,
                                pfNativeError, szErrorMsg,
                                cbErrorMsgMax,pcbErrorMsg);
        if ( error == SQL_SUCCESS )
            CLEAR_ENV_ERROR(henv);
    }
    return error;
}
