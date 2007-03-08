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

/***************************************************************************
 * TRANSACT.C								   *
 *									   *
 * @description: For processing transactions				   *
 *									   *
 * @author     : MySQL AB(monty@mysql.com, venu@mysql.com)		   *
 * @date       : 2001-Aug-15						   *
 * @product    : myodbc3						   *
 *									   *
 ****************************************************************************/

/***************************************************************************
 * The following ODBC APIs are implemented in this file:		   *
 *									   *
 *   SQLEndTran		 (ISO 92)					   *
 *   SQLTransact	 (ODBC, Deprecated)				   *
 *									   *
 ****************************************************************************/

#include "myodbc3.h"

/*
  @type    : internal
  @purpose : to do the transaction at the connection level
*/

SQLRETURN my_transact(SQLHDBC hdbc,SQLSMALLINT CompletionType)
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


/*
  @type    : ODBC 3.0
  @purpose : requests a commit or rollback operation for all active
  operations on all statements associated with a connection
*/

SQLRETURN SQL_API
SQLEndTran(SQLSMALLINT HandleType,
	   SQLHANDLE   Handle,
	   SQLSMALLINT CompletionType)
{
  SQLRETURN result= SQL_SUCCESS;
  ENV FAR *henv;
  LIST *current;

  MYODBCDbgEnter;

  MYODBCDbgInfo( "type: %s", MYODBCDbgHandleTypeString( HandleType ) );
  MYODBCDbgInfo( "handle: 0x%x", Handle );
  MYODBCDbgInfo( "option: %s", MYODBCDbgTransactionTypeString( CompletionType ) );

  switch (HandleType) {
  case SQL_HANDLE_ENV:
    henv = (ENV*)Handle;
    current= henv->connections;
    do
    {
        my_transact( (DBC*)current->data, CompletionType );
    }while( current = current->next );
    break;

  case SQL_HANDLE_DBC:
    result= my_transact(Handle,CompletionType);
    break;

  default:
    result= SQL_ERROR;
    set_error(Handle,MYERR_S1092,NULL,0);
    break;
  }
  MYODBCDbgReturnReturn(result);
}


/*
  @type    : ODBC 1.0
  @purpose : Requests a commit or rollback operation for all active
  operations on all statement handles (hstmts) associated
  with a connection or for all connections associated
  with the environment handle, henv
*/

SQLRETURN SQL_API SQLTransact(SQLHENV henv __attribute__((unused)),
			      SQLHDBC hdbc,
			      SQLUSMALLINT fType)
{
  SQLRETURN result= SQL_SUCCESS;

  MYODBCDbgEnter;
  MYODBCDbgInfo( "henv: 0x%x", henv );
  MYODBCDbgInfo( "hdbc: 0x%x", hdbc );
  MYODBCDbgInfo( "option: %d", fType );

  if (hdbc)
    result= my_transact(hdbc,fType);

  MYODBCDbgReturnReturn(result);
}
