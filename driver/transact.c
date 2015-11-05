/*
  Copyright (c) 2001, 2014, Oracle and/or its affiliates. All rights reserved.

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
  @file  transact.c
  @brief Transaction processing functions.
*/

#include "driver.h"

/**
  Commit or roll back the transactions associated with a particular
  database connection.

  @param[in] hdbc            Handle of database connection
  @param[in] CompletionType  How to complete the transactions,
                             @c SQL_COMMIT or @c SQL_ROLLBACK
*/
static SQLRETURN my_transact(SQLHDBC hdbc, SQLSMALLINT CompletionType)
{
  SQLRETURN result= SQL_SUCCESS;
  DBC *dbc= (DBC *)hdbc;
  const char *query;
  uint	length;

  if (dbc && dbc->ds && !dbc->ds->disable_transactions)
  {
    switch(CompletionType) {
    case SQL_COMMIT:
      query= "COMMIT";
      length= 6;
      break;

    case SQL_ROLLBACK:
      if (!trans_supported(dbc))
      {
	return set_conn_error(hdbc,MYERR_S1C00,
			      "Underlying server does not support transactions, upgrade to version >= 3.23.38",0);
      }
      query= "ROLLBACK";
      length= 8;
      break;

    default:
      return set_conn_error(hdbc,MYERR_S1012,NULL,0);
    }

    MYLOG_DBC_QUERY(dbc, query);

    myodbc_mutex_lock(&dbc->lock);
    if (check_if_server_is_alive(dbc) ||
	mysql_real_query(&dbc->mysql,query,length))
    {
      result= set_conn_error(hdbc,MYERR_S1000,
			     mysql_error(&dbc->mysql),
			     mysql_errno(&dbc->mysql));
    }
    myodbc_mutex_unlock(&dbc->lock);
  }
  return(result);
}


/**
  Commit or roll back the transactions associated with a particular
  database connection, or all connections in an environment.

  @param[in] HandleType      Type of @a Handle, @c SQL_HANDLE_ENV or
                             @c SQL_HANDLE_DBC
  @param[in] Handle          Handle to database connection or environment
  @param[in] CompletionType  How to complete the transactions,
                             @c SQL_COMMIT or @c SQL_ROLLBACK
*/
static SQLRETURN SQL_API
end_transaction(SQLSMALLINT HandleType,
	   SQLHANDLE   Handle,
	   SQLSMALLINT CompletionType)
{
  SQLRETURN result= SQL_SUCCESS;
  ENV *henv;
  DBC *hdbc;
  LIST *current;

  switch (HandleType) {
  case SQL_HANDLE_ENV:
    henv= (ENV *)Handle;
    myodbc_mutex_lock(&henv->lock);
    for (current= henv->connections; current; current= current->next)
    {
        my_transact((DBC *)current->data, CompletionType);
    }
    myodbc_mutex_unlock(&henv->lock);
    break;

  case SQL_HANDLE_DBC:
    hdbc= (DBC*)Handle;

#ifndef _WIN32
    myodbc_mutex_lock(&hdbc->env->lock);
#endif
    result= my_transact(hdbc, CompletionType);
#ifndef _WIN32
    myodbc_mutex_unlock(&hdbc->env->lock);
#endif
    break;

  default:
    result= SQL_ERROR;
    set_error(Handle,MYERR_S1092,NULL,0);
    break;
  }
  return result;
}


/**
  Commit or roll back the transactions associated with a particular
  database connection, or all connections in an environment.

  @param[in] HandleType      Type of @a Handle, @c SQL_HANDLE_ENV or
                             @c SQL_HANDLE_DBC
  @param[in] Handle          Handle to database connection or environment
  @param[in] CompletionType  How to complete the transactions,
                             @c SQL_COMMIT or @c SQL_ROLLBACK

  @since ODBC 3.0
  @since ISO SQL 92
*/
SQLRETURN SQL_API
SQLEndTran(SQLSMALLINT HandleType,
           SQLHANDLE   Handle,
           SQLSMALLINT CompletionType)
{
  CHECK_HANDLE(Handle);

  return end_transaction(HandleType, Handle, CompletionType);
}


/**
  Commit or roll back the transactions associated with a particular
  database connection, or all connections in an environment.

  @deprecated This function is deprecated, SQLEndTran() should be used instead.

  @param[in] henv            Handle to database environment
  @param[in] hdbc            Handle to database connection
  @param[in] fType           How to complete the transactions,
                             @c SQL_COMMIT or @c SQL_ROLLBACK

  @since ODBC 1.0
*/
SQLRETURN SQL_API SQLTransact(SQLHENV henv,
			      SQLHDBC hdbc,
			      SQLUSMALLINT fType)
{
  if(henv == NULL && hdbc == NULL)
    return SQL_INVALID_HANDLE;

  return end_transaction(hdbc ? SQL_HANDLE_DBC : SQL_HANDLE_ENV,
                         hdbc ? hdbc : henv, fType);
}
