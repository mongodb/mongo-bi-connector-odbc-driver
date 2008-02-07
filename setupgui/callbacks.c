/*
  Copyright (C) 2007 MySQL AB

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


/* TODO no L"" */
#include "setupgui.h"
#include "stringutil.h"

SQLWCHAR **errorMsgs= NULL;

SQLHDBC hDBC= SQL_NULL_HDBC;


SQLWCHAR *mytest(HWND hwnd, DataSource *params)
{
  SQLHDBC hDbc= hDBC;
  SQLHENV hEnv= SQL_NULL_HENV;
  SQLWCHAR *rc;

  if (SQL_SUCCEEDED(Connect(&hDbc, &hEnv, params)))
    rc= sqlwchardup(L"Connection successful", SQL_NTS);
  else
  {
    SQLWCHAR state[10];
    SQLINTEGER native;
    SQLSMALLINT len;
    rc= (SQLWCHAR *) my_malloc(512 * sizeof(SQLWCHAR), MYF(0));
    *rc= 0;

    wcscat(rc, L"Connection Failed");
    len= sqlwcharlen(rc);
    if (SQL_SUCCEEDED(SQLGetDiagRecW(SQL_HANDLE_DBC, hDbc, 1, state,
                                     &native, rc + len + 10,
                                     512 - len - 11, &len)))
    {
      wcscat(rc, L": [");
      len= sqlwcharlen(rc);
      sqlwcharncpy(rc + len, state, 6);
      *(rc + sqlwcharlen(rc) + 1)= ' ';
      *(rc + sqlwcharlen(rc))= ']';
    }
  }

  Disconnect(hDbc, hEnv);
  return rc;
}


BOOL mytestaccept(HWND hwnd, DataSource* params)
{
  /* TODO validation */
  return TRUE;
}


LIST *mygetdatabases(HWND hwnd, DataSource* params)
{
  SQLHENV     hEnv= SQL_NULL_HENV;
  SQLHDBC     hDbc= hDBC;
  SQLHSTMT    hStmt;
  SQLRETURN   nReturn;
  SQLWCHAR    szCatalog[MYODBC_DB_NAME_MAX];
  SQLLEN      nCatalog;
  LIST        *dbs= NULL;
  SQLWCHAR    *preserve= params->database;

  params->database= NULL;

  nReturn= Connect(&hDbc, &hEnv, params);

  params->database= preserve;

  if (nReturn != SQL_SUCCESS)
    ShowDiagnostics(nReturn, SQL_HANDLE_DBC, hDbc);
  if (!SQL_SUCCEEDED(nReturn))
  {
    Disconnect(hDbc,hEnv);
    return NULL;
  }

  nReturn= SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
  if (nReturn != SQL_SUCCESS)
    ShowDiagnostics(nReturn, SQL_HANDLE_DBC, hDbc);
  if (!SQL_SUCCEEDED(nReturn))
  {
    Disconnect(hDbc,hEnv);
    return NULL;
  }

  nReturn= SQLTablesW(hStmt, (SQLWCHAR*)SQL_ALL_CATALOGS, SQL_NTS,
                      (SQLWCHAR*)L"", SQL_NTS, (SQLWCHAR*)L"", 0,
                      (SQLWCHAR*)L"", 0);

  if (nReturn != SQL_SUCCESS)
    ShowDiagnostics(nReturn, SQL_HANDLE_STMT, hStmt);
  if (!SQL_SUCCEEDED(nReturn))
  {
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    Disconnect(hDbc, hEnv);
    return NULL;
  }

  nReturn= SQLBindCol(hStmt, 1, SQL_C_WCHAR, szCatalog, MYODBC_DB_NAME_MAX,
                      &nCatalog);
  while (TRUE)
  {
    nReturn= SQLFetch(hStmt);

    if (nReturn == SQL_NO_DATA)
      break;
    else if (nReturn != SQL_SUCCESS)
      ShowDiagnostics(nReturn, SQL_HANDLE_STMT, hStmt);
    if (SQL_SUCCEEDED(nReturn))
      dbs= list_cons(sqlwchardup(szCatalog, SQL_NTS), dbs);
    else
      break;
  }

  SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
  Disconnect(hDbc, hEnv);

  return list_reverse(dbs);
}
