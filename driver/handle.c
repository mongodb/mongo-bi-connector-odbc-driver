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
  @file  handle.c
  @brief Allocation and freeing of handles.
*/

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

#include "driver.h"

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

#ifndef _UNIX_
    {
        HGLOBAL henv= GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof (ENV));
        if (henv == NULL || (*phenv= (SQLHENV)GlobalLock(henv)) == NULL)
        {
            GlobalFree (henv);      /* Free it if lock fails */
            *phenv= SQL_NULL_HENV;
            return SQL_ERROR;
        }
    }
#else
    if (!(*phenv= (SQLHENV) my_malloc(sizeof(ENV),MYF(MY_ZEROFILL))))
    {
        *phenv= SQL_NULL_HENV;
        return SQL_ERROR;
    }
#endif /* _UNIX_ */

    return SQL_SUCCESS;
}


/*
  @type    : ODBC 1.0 API
  @purpose : to allocate the environment handle
*/

SQLRETURN SQL_API SQLAllocEnv(SQLHENV FAR *phenv)
{
  SQLRETURN rc;

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

  return rc;
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
    return my_SQLFreeEnv(henv);
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

    if (mysql_get_client_version() < MIN_MYSQL_VERSION)
    {
        char buff[255];
        sprintf(buff, "Wrong libmysqlclient library version: %ld.  MyODBC needs at least version: %ld", mysql_get_client_version(), MIN_MYSQL_VERSION);
        return(set_env_error(henv, MYERR_S1000, buff, 0));
    }

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
    dbc->stmt_options.cursor_type= SQL_CURSOR_FORWARD_ONLY;  /* ODBC default */
    dbc->login_timeout= 0;
    dbc->last_query_time= (time_t) time((time_t*) 0);
    dbc->txn_isolation= DEFAULT_TXN_ISOLATION;
    dbc->env= penv;
    penv->connections= list_add(penv->connections,&dbc->list);
    dbc->list.data= dbc;
    dbc->unicode= 0;
    dbc->ansi_charset_info= dbc->cxn_charset_info= NULL;
    dbc->exp_desc= NULL;
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
    return my_SQLAllocConnect(henv, phdbc);
}


/*
  @type    : myodbc3 internal
  @purpose : to allocate the connection handle and to
       maintain the connection list
*/

SQLRETURN SQL_API my_SQLFreeConnect(SQLHDBC hdbc)
{
    DBC FAR *dbc= (DBC FAR*) hdbc;
    LIST *ldesc;
    LIST *next;

    dbc->env->connections= list_delete(dbc->env->connections,&dbc->list);
    my_free(dbc->dsn,MYF(MY_ALLOW_ZERO_PTR));
    my_free(dbc->database,MYF(MY_ALLOW_ZERO_PTR));
    my_free(dbc->server,MYF(MY_ALLOW_ZERO_PTR));
    my_free(dbc->socket,MYF(MY_ALLOW_ZERO_PTR));
    my_free(dbc->user,MYF(MY_ALLOW_ZERO_PTR));
    my_free(dbc->password,MYF(MY_ALLOW_ZERO_PTR));
    pthread_mutex_destroy(&dbc->lock);

    /* free any remaining explicitly allocated descriptors */
    for (ldesc= dbc->exp_desc; ldesc; ldesc= next)
    {
      next= ldesc->next;
      desc_free((DESC *) ldesc->data);
      x_free(ldesc);
    }

#ifndef _UNIX_
    GlobalUnlock(GlobalHandle((HGLOBAL) hdbc));
    GlobalFree(GlobalHandle((HGLOBAL) hdbc));
#else
    my_free((char*) hdbc,MYF(0));
#endif
    return SQL_SUCCESS;
}


/*
  @type    : ODBC 1.0 API
  @purpose : to allocate the connection handle and to
       maintain the connection list
*/
SQLRETURN SQL_API SQLFreeConnect(SQLHDBC hdbc)
{
    return my_SQLFreeConnect(hdbc);
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

#ifndef _UNIX_
    hstmt= GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(STMT));
    if (!hstmt || (*phstmt= (SQLHSTMT)GlobalLock(hstmt)) == SQL_NULL_HSTMT)
    {
        *phstmt= SQL_NULL_HSTMT;
        GlobalFree(hstmt);
    }
#else
    *phstmt= (SQLHSTMT) my_malloc(sizeof (STMT), MYF(MY_ZEROFILL | MY_WME));
#endif /* IS UNIX */
    if (*phstmt == SQL_NULL_HSTMT)
        goto error;

    stmt= (STMT FAR*) *phstmt;
    stmt->dbc= dbc;
    pthread_mutex_lock(&stmt->dbc->lock);
    dbc->statements= list_add(dbc->statements,&stmt->list);
    pthread_mutex_unlock(&stmt->dbc->lock);
    stmt->list.data= stmt;
    stmt->stmt_options= dbc->stmt_options;
    stmt->state= ST_UNKNOWN;
    stmt->dummy_state= ST_DUMMY_UNKNOWN;
    strmov(stmt->error.sqlstate, "00000");
    my_init_dynamic_array(&stmt->param_pos, sizeof(char *), 0, 0);

    if (!(stmt->ard= desc_alloc(stmt, SQL_DESC_ALLOC_AUTO,
                                DESC_APP, DESC_ROW)))
        goto error;
    if (!(stmt->ird= desc_alloc(stmt, SQL_DESC_ALLOC_AUTO,
                                DESC_IMP, DESC_ROW)))
        goto error;
    if (!(stmt->apd= desc_alloc(stmt, SQL_DESC_ALLOC_AUTO,
                                DESC_APP, DESC_PARAM)))
        goto error;
    if (!(stmt->ipd= desc_alloc(stmt, SQL_DESC_ALLOC_AUTO,
                                DESC_IMP, DESC_PARAM)))
        goto error;
    stmt->imp_ard= stmt->ard;
    stmt->imp_apd= stmt->apd;

    return SQL_SUCCESS;
error:
    x_free(stmt->ard);
    x_free(stmt->ird);
    x_free(stmt->apd);
    x_free(stmt->ipd);
    return set_dbc_error(dbc, "HY001", "Memory allocation error", MYERR_S1001);
}


/*
  @type    : ODBC 1.0 API
  @purpose : allocates the statement handle
*/

SQLRETURN SQL_API SQLAllocStmt(SQLHDBC hdbc,SQLHSTMT FAR *phstmt)
{
    return my_SQLAllocStmt(hdbc,phstmt);
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
    return my_SQLFreeStmt(hstmt,fOption);
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
  return my_SQLFreeStmtExtended(hstmt,fOption,1);
}

/*
  @type    : myodbc3 internal
  @purpose : stops processing associated with a specific statement,
       closes any open cursors associated with the statement,
       discards pending results, or, optionally, frees all
       resources associated with the statement handle
*/

SQLRETURN SQL_API my_SQLFreeStmtExtended(SQLHSTMT hstmt,SQLUSMALLINT fOption,
                                         uint clearAllResults)
{
    STMT FAR *stmt= (STMT FAR*) hstmt;
    uint i;

    if (fOption == SQL_UNBIND)
    {
      stmt->ard->records.elements= 0;
      stmt->ard->count= 0;
      return SQL_SUCCESS;
    }

    desc_free_paramdata(stmt->apd);

    if (fOption == SQL_RESET_PARAMS)
    {
        /* remove all params and reset count to 0 (per spec) */
        /* http://msdn2.microsoft.com/en-us/library/ms709284.aspx */
        stmt->apd->count= 0;
        return SQL_SUCCESS;
    }

    if (!stmt->fake_result)
    {
      mysql_free_result(stmt->result);
      /* check if there are more resultsets */
      if (clearAllResults)
      {
        while (mysql_more_results(&stmt->dbc->mysql))
        {
          if (!mysql_next_result(&stmt->dbc->mysql))
          {
            stmt->result= mysql_store_result(&stmt->dbc->mysql);
            mysql_free_result(stmt->result);
          }
        }
      }
    }
    else
      x_free(stmt->result);
    x_free(stmt->fields);
    x_free(stmt->array);
    x_free(stmt->result_array);
    stmt->result= 0;
    stmt->fake_result= 0;
    stmt->fields= 0;
    stmt->array= 0;
    stmt->result_array= 0;
    stmt->current_values= 0;   /* For SQLGetData */
    stmt->fix_fields= 0;
    stmt->affected_rows= 0;
    stmt->current_row= stmt->cursor_row= stmt->rows_found_in_set= 0;

    if (fOption == MYSQL_RESET_BUFFERS)
        return SQL_SUCCESS;

    stmt->state= ST_UNKNOWN;

    x_free(stmt->table_name);
    stmt->table_name= 0;
    stmt->dummy_state= ST_DUMMY_UNKNOWN;
    stmt->cursor.pk_validated= FALSE;

    for (i= stmt->cursor.pk_count; i--;)
        stmt->cursor.pkcol[i].bind_done= 0;
    stmt->cursor.pk_count= 0;

    if (fOption == SQL_CLOSE)
        return SQL_SUCCESS;

    /* At this point, only MYSQL_RESET and SQL_DROP left out */
    x_free(stmt->query);
    x_free(stmt->orig_query);
    stmt->query= stmt->orig_query= 0;
    stmt->param_count= 0;

    reset_ptr(stmt->apd->rows_processed_ptr);
    reset_ptr(stmt->ard->rows_processed_ptr);
    reset_ptr(stmt->ipd->array_status_ptr);
    reset_ptr(stmt->ird->array_status_ptr);
    reset_ptr(stmt->apd->array_status_ptr);
    reset_ptr(stmt->ard->array_status_ptr);
    reset_ptr(stmt->stmt_options.rowStatusPtr_ex);

    if (fOption == MYSQL_RESET)
        return SQL_SUCCESS;

    /* explicitly allocated descriptors are affected up until this point */
    desc_remove_stmt(stmt->apd, stmt);
    desc_remove_stmt(stmt->ard, stmt);
    desc_free(stmt->imp_apd);
    desc_free(stmt->imp_ard);
    desc_free(stmt->ipd);
    desc_free(stmt->ird);

    x_free(stmt->cursor.name);

    delete_dynamic(&stmt->param_pos);
    pthread_mutex_lock(&stmt->dbc->lock);
    stmt->dbc->statements= list_delete(stmt->dbc->statements,&stmt->list);
    pthread_mutex_unlock(&stmt->dbc->lock);
#ifndef _UNIX_
    GlobalUnlock(GlobalHandle ((HGLOBAL) hstmt));
    GlobalFree(GlobalHandle((HGLOBAL) hstmt));
#else
    my_free((char*) hstmt,MYF(0));
#endif /* _UNIX_*/
    return SQL_SUCCESS;
}


/*
  Explicitly allocate a descriptor.
*/
SQLRETURN my_SQLAllocDesc(SQLHDBC hdbc, SQLHANDLE *pdesc)
{
  DBC *dbc= (DBC *) hdbc;
  DESC *desc= desc_alloc(NULL, SQL_DESC_ALLOC_USER, DESC_APP, DESC_UNKNOWN);
  LIST *e;

  if (!desc)
    return set_dbc_error(dbc, "HY001", "Memory allocation error", MYERR_S1001);

  desc->exp.dbc= dbc;

  /* add to this connection's list of explicit descriptors */
  e= (LIST *) my_malloc(sizeof(LIST), MYF(0));
  e->data= desc;
  dbc->exp_desc= list_add(dbc->exp_desc, e);

  *pdesc= desc;
  return SQL_SUCCESS;
}


/*
  Free an explicitly allocated descriptor. This resets all statements
  that it was associated with to their implicitly allocated descriptors.
*/
SQLRETURN my_SQLFreeDesc(SQLHANDLE hdesc)
{
  DESC *desc= (DESC *) hdesc;
  DBC *dbc= desc->exp.dbc;
  LIST *lstmt;
  LIST *ldesc;
  LIST *next;

  if (!desc)
    return SQL_ERROR;
  if (desc->alloc_type != SQL_DESC_ALLOC_USER)
    return set_desc_error(desc, "HY017", "Invalid use of an automatically "
                          "allocated descriptor handle.", MYERR_S1017);

  /* remove from DBC */
  for (ldesc= dbc->exp_desc; ldesc; ldesc= ldesc->next)
  {
    if (ldesc->data == desc)
    {
      dbc->exp_desc= list_delete(dbc->exp_desc, ldesc);
      x_free(ldesc);
      break;
    }
  }

  /* reset all stmts it was on - to their implicit desc */
  for (lstmt= desc->exp.stmts; lstmt; lstmt= next)
  {
    STMT *stmt= lstmt->data;
    next= lstmt->next;
    if (IS_APD(desc))
      stmt->apd= stmt->imp_apd;
    else if (IS_ARD(desc))
      stmt->ard= stmt->imp_ard;
    x_free(lstmt);
  }

  desc_free(desc);
  return SQL_SUCCESS;
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

        case SQL_HANDLE_DESC:
            error= my_SQLAllocDesc(InputHandle, OutputHandlePtr);
            break;

        default:
            return set_conn_error(InputHandle,MYERR_S1C00,NULL,0);
    }

    return error;
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

        case SQL_HANDLE_DESC:
            error= my_SQLFreeDesc((DESC *) Handle);
            break;


        default:
            break;
    }

    return error;
}


