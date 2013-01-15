/*
  Copyright (c) 2007, 2009, Oracle and/or its affiliates. All rights reserved.

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
  SQLWCHAR    *preservedDatabase= params->database;
  BOOL        preservedNoCatalog= params->no_catalog;

  params->database= NULL;
  params->no_catalog= FALSE;

  nReturn= Connect(&hDbc, &hEnv, params);

  params->database= preservedDatabase;
  params->no_catalog= preservedNoCatalog;

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


LIST *mygetcharsets(HWND hwnd, DataSource* params)
{
  SQLHENV     hEnv= SQL_NULL_HENV;
  SQLHDBC     hDbc= hDBC;
  SQLHSTMT    hStmt;
  SQLRETURN   nReturn;
  SQLWCHAR    szCharset[MYODBC_DB_NAME_MAX];
  SQLLEN      nCharset;
  LIST        *csl= NULL;
  SQLWCHAR    *preservedDatabase= params->database;
  BOOL        preservedNoCatalog= params->no_catalog;

  params->database= NULL;
  params->no_catalog= FALSE;

  nReturn= Connect(&hDbc, &hEnv, params);

  params->database= preservedDatabase;
  params->no_catalog= preservedNoCatalog;

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


  nReturn = SQLExecDirectW( hStmt, L"SHOW CHARACTER SET", SQL_NTS);
  if (nReturn != SQL_SUCCESS)
    ShowDiagnostics(nReturn, SQL_HANDLE_STMT, hStmt);
  if (!SQL_SUCCEEDED(nReturn))
  {
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    Disconnect(hDbc, hEnv);
    return NULL;
  }

  nReturn= SQLBindCol(hStmt, 1, SQL_C_WCHAR, szCharset, MYODBC_DB_NAME_MAX,
                      &nCharset);
  while (TRUE)
  {
    nReturn= SQLFetch(hStmt);

    if (nReturn == SQL_NO_DATA)
      break;
    else if (nReturn != SQL_SUCCESS)
      ShowDiagnostics(nReturn, SQL_HANDLE_STMT, hStmt);
    if (SQL_SUCCEEDED(nReturn))
    {
#ifdef DRIVER_ANSI
      /* Skip undesired charsets */
      if(!sqlwcharcasecmp(szCharset, sqlwchardup(L"utf16", SQL_NTS)) ||
         !sqlwcharcasecmp(szCharset, sqlwchardup(L"utf32", SQL_NTS)) ||
         !sqlwcharcasecmp(szCharset, sqlwchardup(L"ucs2", SQL_NTS)))
        continue;
#endif
      csl= list_cons(sqlwchardup(szCharset, SQL_NTS), csl);
    }
    else
      break;
  }

  SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
  Disconnect(hDbc, hEnv);

  return list_reverse(csl);
}
