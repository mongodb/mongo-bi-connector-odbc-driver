/****************************************************************************
 *                                                                          *
 * File    :                                                                *
 *                                                                          *
 * Purpose : GUI Callbacks                                                  *
 *                                                                          *
 * History : Date      Reason                                               *
 *           00/00/00  Created                                              *
 *                                                                          *
 ****************************************************************************/


/* TODO no L"" */

#include "callbacks.h"
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
  LIST *dbs= NULL;

  nReturn= Connect(&hDbc, &hEnv, params);

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


void myhelp(HWND hwnd)
{
  /** TODO: Rewrite - Shouldn't be windows stuff here */
  /*MessageBoxW(hwnd, L"HELP", L"Sorry, Help is not Available", MB_OK);*/
}

