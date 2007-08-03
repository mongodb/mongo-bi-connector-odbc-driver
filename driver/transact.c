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
  @file  transact.c
  @brief Transaction processing functions.
*/

#include "myodbc3.h"

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
  DBC FAR *dbc= (DBC FAR *)hdbc;
  const char *query;
  uint	length;

  if (dbc && !(dbc->flag & FLAG_NO_TRANSACTIONS))
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

    pthread_mutex_lock(&dbc->lock);
    if (check_if_server_is_alive(dbc) ||
	mysql_real_query(&dbc->mysql,query,length))
    {
      result= set_conn_error(hdbc,MYERR_S1000,
			     mysql_error(&dbc->mysql),
			     mysql_errno(&dbc->mysql));
    }
    pthread_mutex_unlock(&dbc->lock);
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
  ENV FAR *henv;
  LIST *current;

  switch (HandleType) {
  case SQL_HANDLE_ENV:
    henv= (ENV *)Handle;
    for (current= henv->connections; current; current= current->next)
    {
        my_transact((DBC *)current->data, CompletionType);
    }
    break;

  case SQL_HANDLE_DBC:
    result= my_transact(Handle,CompletionType);
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
  return end_transaction(hdbc ? SQL_HANDLE_DBC : SQL_HANDLE_ENV,
                         hdbc ? hdbc : henv, fType);
}
