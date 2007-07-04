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


/** Test passing an SQL_C_CHAR to a SQL_WCHAR field. */
DECLARE_TEST(basic)
{
  /* Note: this is an SQLCHAR, so it is 'ANSI' data. */
  SQLCHAR data[]= "S\xe3o Paolo", buff[30];
  SQLWCHAR wbuff[30];
  wchar_t wcdata[]= L"S\u00e3o Paolo";

  ok_sql(hstmt, "DROP TABLE IF EXISTS t1");
  ok_sql(hstmt, "CREATE TABLE t1 (a VARCHAR(30)) DEFAULT CHARSET utf8");

  ok_stmt(hstmt, SQLPrepare(hstmt, (SQLCHAR *)
                            "INSERT INTO t1 VALUES (?)", SQL_NTS));
  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR,
                                  SQL_WVARCHAR, 0, 0, data, sizeof(data),
                                  NULL));
  ok_stmt(hstmt, SQLExecute(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_RESET_PARAMS));

  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR,
                                  SQL_WVARCHAR, 0, 0, W(wcdata), sizeof(wcdata),
                                  NULL));
  ok_stmt(hstmt, SQLExecute(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_RESET_PARAMS));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "SELECT HEX(a) FROM t1");

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_str(my_fetch_str(hstmt, buff, 1), "53C3A36F2050616F6C6F", 20);

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_str(my_fetch_str(hstmt, buff, 1), "53C3A36F2050616F6C6F", 20);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "SELECT a FROM t1");

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_str(my_fetch_str(hstmt, buff, 1), data, sizeof(data));

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_str(my_fetch_str(hstmt, buff, 1), data, sizeof(data));

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "SELECT a FROM t1");

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_wstr(my_fetch_wstr(hstmt, wbuff, 1), wcdata, 9);

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_wstr(my_fetch_wstr(hstmt, wbuff, 1), wcdata, 9);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t1");
  return OK;
}


BEGIN_TESTS
  ADD_TEST(basic)
END_TESTS


RUN_TESTS

