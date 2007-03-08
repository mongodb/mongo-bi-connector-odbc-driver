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

/***************************************************************************
 * HANDLE.C								   *
 *									   *
 * @description: Allocation and freeing of handles			   *
 *									   *
 * @author     : MySQL AB(monty@mysql.com, venu@mysql.com)		   *
 * @date       : 2001-Nov-07						   *
 * @product    : myodbc3						   *
 *									   *
****************************************************************************/

/***************************************************************************
 * The following ODBC APIs are implemented in this file:		   *
 *									   *
 *   SQLAllocHandle	 (ISO 92)					   *
 *   SQLFreeHandle	 (ISO 92)					   *
 *   SQLAllocEnv	 (ODBC, Deprecated)				   *
 *   SQLAllocConnect	 (ODBC, Deprecated)				   *
 *   SQLAllocStmt	 (ODBC, Deprecated)				   *
 *   SQLFreeEnv		 (ODBC, Deprecated)				   *
 *   SQLFreeConnect	 (ODBC, Deprecated)				   *
 *   SQLFreeStmt	 (ISO 92)					   *
 *									   *
****************************************************************************/

#include "myodbc3.h"

/*
  @type    : myodbc3 internal
  @purpose : to allocate the environment handle and to maintain
       its list
*/

SQLRETURN SQL_API my_SQLAllocEnv(SQLHENV FAR *phenv)
{
#ifdef _UNIX_
    myodbc_init(); /* This is done in LibMain on XP so it probably needs to be in this func only when in UNIX - PAH */
#endif

    MYODBCDbgEnter;

#ifndef _UNIX_
    {
        HGLOBAL henv= GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof (ENV));
        if (henv == NULL || (*phenv= (SQLHENV)GlobalLock(henv)) == NULL)
        {
            GlobalFree (henv);      /* Free it if lock fails */
            *phenv= SQL_NULL_HENV;
            MYODBCDbgReturnReturn( SQL_ERROR );
        }
    }
#else
    if (!(*phenv= (SQLHENV) my_malloc(sizeof(ENV),MYF(MY_ZEROFILL))))
    {
        *phenv= SQL_NULL_HENV;
        MYODBCDbgReturnReturn( SQL_ERROR );
    }
#endif /* _UNIX_ */

    MYODBCDbgReturnReturn( SQL_SUCCESS );
}


/*
  @type    : ODBC 1.0 API
  @purpose : to allocate the environment handle
*/

SQLRETURN SQL_API SQLAllocEnv(SQLHENV FAR *phenv)
{
  SQLRETURN rc;
  MYODBCDbgEnter;
  MYODBCDbgInfo( "phenv: %p", phenv );

  rc= my_SQLAllocEnv(phenv);
  if (rc == SQL_SUCCESS)
  {
/* --- if OS=WIN32, set default env option for SQL_ATTR_ODBC_VERSION */
#ifdef WIN32
    ((ENV FAR*) *phenv)->odbc_ver= SQL_OV_ODBC3;
#else
    ((ENV FAR*) *phenv)->odbc_ver= SQL_OV_ODBC2;
#endif /* WIN32 */
  }

  MYODBCDbgReturnReturn(rc);
}


/*
  @type    : myodbc3 internal
  @purpose : to free the environment handle
*/

SQLRETURN SQL_API my_SQLFreeEnv(SQLHENV henv)
{
#ifndef _UNIX_
    GlobalUnlock(GlobalHandle((HGLOBAL) henv));
    GlobalFree(GlobalHandle((HGLOBAL) henv));
#else
    if (henv) my_free((char*) henv,MYF(0));
    myodbc_end();
#endif /* _UNIX_ */
    return(SQL_SUCCESS);
}


/*
  @type    : ODBC 1.0 API
  @purpose : to free the environment handle
*/
SQLRETURN SQL_API SQLFreeEnv(SQLHENV henv)
{
    MYODBCDbgEnter;
    MYODBCDbgInfo( "henv: %p", henv );

    MYODBCDbgReturnReturn( my_SQLFreeEnv(henv) );
}

#ifndef _UNIX_
SQLRETURN my_GetLastError(ENV *henv)
{
    SQLRETURN ret;
    LPVOID    msg;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,
                  GetLastError(),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR) &msg,
                  0,
                  NULL );

    ret = set_env_error(henv,MYERR_S1001,msg,0);
    LocalFree(msg);

    return ret;
}
#endif

/*
  @type    : myodbc3 internal
  @purpose : to allocate the connection handle and to
       maintain the connection list
*/

SQLRETURN SQL_API my_SQLAllocConnect(SQLHENV henv, SQLHDBC FAR *phdbc)
{
    DBC FAR *dbc;
    ENV FAR *penv= (ENV FAR*) henv;

#if MYSQL_VERSION_ID >= 40016
    if (mysql_get_client_version() < MIN_MYSQL_VERSION)
    {
        char buff[255];
        sprintf(buff, "Wrong libmysqlclient library version: %ld.  MyODBC needs at least version: %ld", mysql_get_client_version(), MIN_MYSQL_VERSION);
        return(set_env_error(henv, MYERR_S1000, buff, 0));
    }
#endif

    if (!penv->odbc_ver)
    {
        return set_env_error(henv, MYERR_S1010,
                             "Can't allocate connection "
                             "until ODBC version specified.", 0);
    }

#ifndef _UNIX_
    {
        HGLOBAL hdbc= GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof (DBC));
        if (!hdbc)
        {
            *phdbc= SQL_NULL_HENV;
            return(my_GetLastError(henv));
        }

        if ((*phdbc= (SQLHDBC)GlobalLock(hdbc)) == SQL_NULL_HDBC)
        {
            *phdbc= SQL_NULL_HENV;
            return(my_GetLastError(henv));
        }
    }
#else
    if (!(*phdbc= (SQLHDBC) my_malloc(sizeof(DBC),MYF(MY_ZEROFILL))))
    {
        *phdbc= SQL_NULL_HDBC;
        return(set_env_error(henv,MYERR_S1001,NULL,0));
    }
#endif /* _UNIX_ */

/* --- if OS=WIN32, set default env option for SQL_ATTR_ODBC_VERSION */
#ifdef WIN32
/* This was a fix for BDE (and or Crystal Reports) but messes other apps up. see BUG 8363. */
/* penv->odbc_ver= SQL_OV_ODBC3; */
#endif /* WIN32 */

    dbc= (DBC FAR*) *phdbc;
    dbc->mysql.net.vio= 0;     /* Marker if open */
    dbc->flag= 0;
    dbc->commit_flag= 0;
    dbc->stmt_options.max_rows= dbc->stmt_options.max_length= 0L;
    dbc->stmt_options.bind_type= SQL_BIND_BY_COLUMN;
    dbc->stmt_options.rows_in_set= 1;
    dbc->stmt_options.cursor_type= SQL_CURSOR_FORWARD_ONLY;  /* ODBC default */
    dbc->login_timeout= 0;
    dbc->last_query_time= (time_t) time((time_t*) 0);
    dbc->txn_isolation= DEFAULT_TXN_ISOLATION;
    dbc->env= penv;
    penv->connections= list_add(penv->connections,&dbc->list);
    dbc->list.data= dbc;
    pthread_mutex_init(&dbc->lock,NULL);
    pthread_mutex_lock(&dbc->lock);
    myodbc_ov_init(penv->odbc_ver); /* Initialize based on ODBC version */
    pthread_mutex_unlock(&dbc->lock);
    return(SQL_SUCCESS);
}


/*
  @type    : ODBC 1.0 API
  @purpose : to allocate the connection handle and to
       maintain the connection list
*/

SQLRETURN SQL_API SQLAllocConnect(SQLHENV henv, SQLHDBC FAR *phdbc)
{
    MYODBCDbgEnter;
    MYODBCDbgInfo( "henv: %p", henv );
    MYODBCDbgInfo( "phdbc: %p", phdbc );

    MYODBCDbgReturnReturn( my_SQLAllocConnect(henv, phdbc) );
}


/*
  @type    : myodbc3 internal
  @purpose : to allocate the connection handle and to
       maintain the connection list
*/

SQLRETURN SQL_API my_SQLFreeConnect(SQLHDBC hdbc)
{
    DBC FAR *dbc= (DBC FAR*) hdbc;

    MYODBCDbgEnter;

    dbc->env->connections= list_delete(dbc->env->connections,&dbc->list);
    my_free(dbc->dsn,MYF(MY_ALLOW_ZERO_PTR));
    my_free(dbc->database,MYF(MY_ALLOW_ZERO_PTR));
    my_free(dbc->server,MYF(MY_ALLOW_ZERO_PTR));
    my_free(dbc->user,MYF(MY_ALLOW_ZERO_PTR));
    my_free(dbc->password,MYF(MY_ALLOW_ZERO_PTR));
    pthread_mutex_destroy(&dbc->lock);

#ifndef _UNIX_
    GlobalUnlock(GlobalHandle((HGLOBAL) hdbc));
    GlobalFree(GlobalHandle((HGLOBAL) hdbc));
#else
    my_free((char*) hdbc,MYF(0));
#endif
    MYODBCDbgReturnReturn(SQL_SUCCESS);
}


/*
  @type    : ODBC 1.0 API
  @purpose : to allocate the connection handle and to
       maintain the connection list
*/
SQLRETURN SQL_API SQLFreeConnect(SQLHDBC hdbc)
{
    MYODBCDbgEnter;
    MYODBCDbgInfo( "hdbc: %p", hdbc );

    MYODBCDbgReturnReturn( my_SQLFreeConnect(hdbc) );
}


/*
  @type    : myodbc3 internal
  @purpose : allocates the statement handle
*/

SQLRETURN SQL_API my_SQLAllocStmt(SQLHDBC hdbc,SQLHSTMT FAR *phstmt)
{
#ifndef _UNIX_
    HGLOBAL  hstmt;
#endif
    STMT FAR *stmt;
    DBC FAR *dbc= (DBC FAR*) hdbc;

    MYODBCDbgEnter;

#ifndef _UNIX_
    hstmt= GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(STMT));
    if (!hstmt || (*phstmt= (SQLHSTMT)GlobalLock(hstmt)) == SQL_NULL_HSTMT)
    {
        GlobalFree(hstmt);
        *phstmt= SQL_NULL_HSTMT;
        MYODBCDbgReturnReturn(SQL_ERROR);
    }
#else
    *phstmt= (SQLHSTMT) my_malloc(sizeof (STMT), MYF(MY_ZEROFILL | MY_WME));
    if (*phstmt == SQL_NULL_HSTMT)
    {
        *phstmt= SQL_NULL_HSTMT;
        MYODBCDbgReturnReturn(SQL_ERROR);
    }
#endif /* IS UNIX */
    stmt= (STMT FAR*) *phstmt;
    stmt->dbc= dbc;
    dbc->statements= list_add(dbc->statements,&stmt->list);
    stmt->list.data= stmt;
    stmt->stmt_options= dbc->stmt_options;
    stmt->state= ST_UNKNOWN;
#if !defined(DBUG_OFF) && defined(my_init_dynamic_array)
    my_init_dynamic_array(&stmt->params,sizeof(PARAM_BIND),32,64);
#else
    init_dynamic_array(&stmt->params,sizeof(PARAM_BIND),32,64);
#endif
    MYODBCDbgReturnReturn(SQL_SUCCESS);
}


/*
  @type    : ODBC 1.0 APO
  @purpose : allocates the statement handle
*/

SQLRETURN SQL_API SQLAllocStmt(SQLHDBC hdbc,SQLHSTMT FAR *phstmt)
{
    MYODBCDbgEnter;
    MYODBCDbgInfo( "hdbc: %p", hdbc );
    MYODBCDbgInfo( "phstmt: %p", phstmt );

    MYODBCDbgReturnReturn( my_SQLAllocStmt(hdbc,phstmt) );
}


/*
  @type    : ODBC 1.0 API
  @purpose : stops processing associated with a specific statement,
       closes any open cursors associated with the statement,
       discards pending results, or, optionally, frees all
       resources associated with the statement handle
*/

SQLRETURN SQL_API SQLFreeStmt(SQLHSTMT hstmt,SQLUSMALLINT fOption)
{
    MYODBCDbgEnter;
    MYODBCDbgInfo( "hstmt: %p", hstmt );
    MYODBCDbgInfo( "fOption: %d", fOption );

    MYODBCDbgReturnReturn( my_SQLFreeStmt(hstmt,fOption) );
}


void odbc_reset_stmt_options(STMT_OPTIONS *options)
{
    reset_ptr(options->paramProcessedPtr);
    reset_ptr(options->paramStatusPtr);
    reset_ptr(options->rowOperationPtr);
    reset_ptr(options->rowsFetchedPtr);
    reset_ptr(options->rowStatusPtr);
}

/*
  @type    : myodbc3 internal
  @purpose : stops processing associated with a specific statement,
       closes any open cursors associated with the statement,
       discards pending results, or, optionally, frees all
       resources associated with the statement handle
*/

SQLRETURN SQL_API my_SQLFreeStmt(SQLHSTMT hstmt,SQLUSMALLINT fOption)
{
    STMT FAR *stmt= (STMT FAR*) hstmt;
    uint i;

    MYODBCDbgEnter;

    MYODBCDbgInfo( "stmt: 0x%lx", (long)hstmt );
    MYODBCDbgInfo( "option: %s", MYODBCDbgStmtTypeString(fOption) );

    if (fOption == SQL_UNBIND)
    {
        x_free(stmt->bind);
        stmt->bind= 0;
        stmt->bound_columns= 0;
        MYODBCDbgReturnReturn( SQL_SUCCESS );
    }
    for (i= 0 ; i < stmt->params.elements ; i++)
    {
        PARAM_BIND *param= dynamic_element(&stmt->params,i,PARAM_BIND*);
        if (param->alloced)
        {
            param->alloced= 0;
            my_free(param->value,MYF(0));
        }
        if (fOption == SQL_RESET_PARAMS)
        {
            param->used= 0;
            param->real_param_done= FALSE;
        }
    }
    if (fOption == SQL_RESET_PARAMS)
        MYODBCDbgReturnReturn( SQL_SUCCESS );

    mysql_free_result(stmt->result);
    x_free((gptr) stmt->fields);
    x_free((gptr) stmt->array);
    x_free((gptr) stmt->result_array);
    x_free((gptr) stmt->odbc_types);
    stmt->result= 0;
    stmt->result_lengths= 0;
    stmt->fields= 0;
    stmt->array= 0;
    stmt->result_array= 0;
    stmt->odbc_types= 0;
    stmt->current_values= 0;   /* For SQLGetData */
    stmt->fix_fields= 0;
    stmt->affected_rows= 0;
    stmt->current_row= stmt->cursor_row= stmt->rows_found_in_set= 0;
    stmt->state= ST_UNKNOWN;

    if (fOption == MYSQL_RESET_BUFFERS)
        MYODBCDbgReturnReturn( SQL_SUCCESS );

    x_free((gptr) stmt->table_name);
    stmt->table_name= 0;
    stmt->dummy_state= ST_DUMMY_UNKNOWN;
    stmt->cursor.pk_validated= FALSE;

    for (i= stmt->cursor.pk_count; i--;)
        stmt->cursor.pkcol[i].bind_done= 0;
    stmt->cursor.pk_count= 0;

    if (fOption == SQL_CLOSE)
        MYODBCDbgReturnReturn( SQL_SUCCESS );

    /* At this point, only MYSQL_RESET and SQL_DROP left out */
    x_free((gptr) stmt->query);
    stmt->query= 0;
    stmt->param_count= 0;

    if (fOption == MYSQL_RESET)
        MYODBCDbgReturnReturn( SQL_SUCCESS );

    odbc_reset_stmt_options(&stmt->stmt_options);

    x_free((gptr) stmt->cursor.name);
    x_free((gptr) stmt->bind);
    delete_dynamic(&stmt->params);
    stmt->dbc->statements= list_delete(stmt->dbc->statements,&stmt->list);
#ifndef _UNIX_
    GlobalUnlock(GlobalHandle ((HGLOBAL) hstmt));
    GlobalFree(GlobalHandle((HGLOBAL) hstmt));
#else
    my_free((char*) hstmt,MYF(0));
#endif /* _UNIX_*/
    MYODBCDbgReturnReturn( SQL_SUCCESS );
}


/*
  @type    : ODBC 3.0 API
  @purpose : allocates an environment, connection, statement, or
       descriptor handle
*/

SQLRETURN SQL_API SQLAllocHandle(SQLSMALLINT HandleType,
                                 SQLHANDLE   InputHandle,
                                 SQLHANDLE   *OutputHandlePtr)
{
    SQLRETURN error= SQL_ERROR;

    MYODBCDbgEnter;

    MYODBCDbgInfo( "HandleType: %s", MYODBCDbgHandleTypeString( HandleType ) );
    MYODBCDbgInfo( "HandleType: %d", HandleType );
    MYODBCDbgInfo( "InputHandle: %p", InputHandle );
    MYODBCDbgInfo( "OutputHandlePtr: %p", OutputHandlePtr );

    switch (HandleType)
    {
        case SQL_HANDLE_ENV:
            error= my_SQLAllocEnv(OutputHandlePtr);
            break;

        case SQL_HANDLE_DBC:
            error= my_SQLAllocConnect(InputHandle,OutputHandlePtr);
            break;

        case SQL_HANDLE_STMT:
            error= my_SQLAllocStmt(InputHandle,OutputHandlePtr);
            break;

        default:
            MYODBCDbgReturnReturn( set_conn_error(InputHandle,MYERR_S1C00,NULL,0) );
    }

    MYODBCDbgReturnReturn( error );
}


/*
  @type    : ODBC 3.0 API
  @purpose : frees resources associated with a specific environment,
       connection, statement, or descriptor handle
*/

SQLRETURN SQL_API SQLFreeHandle(SQLSMALLINT HandleType,
                                SQLHANDLE   Handle)
{
    SQLRETURN error= SQL_ERROR;

    MYODBCDbgEnter;
    MYODBCDbgInfo( "HandleType: %s", MYODBCDbgHandleTypeString( HandleType ) );
    MYODBCDbgInfo( "HandleType: %d", HandleType );
    MYODBCDbgInfo( "Handle: %p", Handle );

    switch (HandleType)
    {
        case SQL_HANDLE_ENV:
            error= my_SQLFreeEnv((ENV *)Handle);
            break;

        case SQL_HANDLE_DBC:
            error= my_SQLFreeConnect((DBC *)Handle);
            break;

        case SQL_HANDLE_STMT:
            error= my_SQLFreeStmt((STMT *)Handle, SQL_DROP);
            break;
        default:
            break;
    }

    MYODBCDbgReturnReturn( error );
}

