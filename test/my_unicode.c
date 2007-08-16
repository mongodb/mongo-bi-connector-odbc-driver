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

#include "odbctap.h"
#include <sqlucode.h>


DECLARE_TEST(sqlconnect)
{
  HDBC hdbc1;

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));
  ok_con(hdbc1, SQLConnectW(hdbc1,
                            W(L"myodbc3"), SQL_NTS,
                            W(L"root"), SQL_NTS,
                            W(L""), SQL_NTS));
  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


DECLARE_TEST(sqlprepare)
{
  HDBC hdbc1;
  HSTMT hstmt1;
  SQLINTEGER data;
  SQLWCHAR wbuff[30];

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));
  ok_con(hdbc1, SQLConnectW(hdbc1,
                            W(L"myodbc3"), SQL_NTS,
                            W(L"root"), SQL_NTS,
                            W(L""), SQL_NTS));

  ok_con(hdbc, SQLAllocStmt(hdbc1, &hstmt1));

  ok_stmt(hstmt1, SQLPrepareW(hstmt1,
                              W(L"SELECT '\u30a1' FROM DUAL WHERE 1 = ?"),
                              SQL_NTS));

  ok_stmt(hstmt1, SQLBindParameter(hstmt1, 1, SQL_PARAM_INPUT, SQL_C_LONG,
                                   SQL_INTEGER, 0, 0, &data, 0, NULL));
  data= 0;
  ok_stmt(hstmt1, SQLExecute(hstmt1));

  expect_stmt(hstmt1, SQLFetch(hstmt1), SQL_NO_DATA_FOUND);

  data= 1;
  ok_stmt(hstmt1, SQLExecute(hstmt1));

  ok_stmt(hstmt1, SQLFetch(hstmt1));

  is_wstr(my_fetch_wstr(hstmt1, wbuff, 1), L"\u30a1", 1);

  expect_stmt(hstmt1, SQLFetch(hstmt1), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt1, SQL_CLOSE));

  /* Now try ANSI SQLPrepare. */
  ok_stmt(hstmt1, SQLPrepare(hstmt1,
                             (SQLCHAR *)"SELECT '\xe3' FROM DUAL WHERE 1 = ?",
                             SQL_NTS));

  data= 0;
  ok_stmt(hstmt1, SQLExecute(hstmt1));

  expect_stmt(hstmt1, SQLFetch(hstmt1), SQL_NO_DATA_FOUND);

  data= 1;
  ok_stmt(hstmt1, SQLExecute(hstmt1));

  ok_stmt(hstmt1, SQLFetch(hstmt1));

  is_wstr(my_fetch_wstr(hstmt1, wbuff, 1), L"\u00e3", 1);

  expect_stmt(hstmt1, SQLFetch(hstmt1), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt1, SQL_DROP));

  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  /* Now try SQLPrepareW with an ANSI connection. */
  ok_stmt(hstmt, SQLPrepareW(hstmt,
                             W(L"SELECT '\u00e3' FROM DUAL WHERE 1 = ?"),
                             SQL_NTS));

  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG,
                                  SQL_INTEGER, 0, 0, &data, 0, NULL));
  data= 0;
  ok_stmt(hstmt, SQLExecute(hstmt));

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  data= 1;
  ok_stmt(hstmt, SQLExecute(hstmt));

  ok_stmt(hstmt, SQLFetch(hstmt));

  is_wstr(my_fetch_wstr(hstmt, wbuff, 1), L"\u00e3", 1);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  /* Now try SQLPrepareW with a character taht doesn't translate. */
  expect_stmt(hstmt, SQLPrepareW(hstmt,
                                 W(L"SELECT '\u30a1' FROM DUAL WHERE 1 = ?"),
                                 SQL_NTS),
              SQL_ERROR);
  is(check_sqlstate(hstmt, "22018") == OK);

  return OK;
}


DECLARE_TEST(sqlchar)
{
  HDBC hdbc1;
  HSTMT hstmt1;
  SQLCHAR data[]= "S\xe3o Paolo", buff[30];
  SQLWCHAR wbuff[30];
  wchar_t wcdata[]= L"S\u00e3o Paolo";

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));
  ok_con(hdbc1, SQLConnectW(hdbc1,
                            W(L"myodbc3"), SQL_NTS,
                            W(L"root"), SQL_NTS,
                            W(L""), SQL_NTS));
  ok_con(hdbc, SQLAllocStmt(hdbc1, &hstmt1));

  ok_stmt(hstmt1, SQLPrepareW(hstmt1, W(L"SELECT ? FROM DUAL"), SQL_NTS));

  ok_stmt(hstmt1, SQLBindParameter(hstmt1, 1, SQL_PARAM_INPUT, SQL_C_CHAR,
                                   SQL_WVARCHAR, 0, 0, data, sizeof(data),
                                   NULL));
  ok_stmt(hstmt1, SQLExecute(hstmt1));

  ok_stmt(hstmt1, SQLFetch(hstmt1));
  is_str(my_fetch_str(hstmt1, buff, 1), data, sizeof(data));
  is_wstr(my_fetch_wstr(hstmt1, wbuff, 1), wcdata, 9);

  expect_stmt(hstmt1, SQLFetch(hstmt1), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt1, SQL_DROP));

  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


BEGIN_TESTS
  ADD_TEST(sqlconnect)
  ADD_TEST(sqlprepare)
  ADD_TEST(sqlchar)
END_TESTS


RUN_TESTS

