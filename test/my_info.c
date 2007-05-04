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

#include "odbctap.h"


DECLARE_TEST(t_gettypeinfo)
{
  SQLSMALLINT pccol;

  ok_stmt(hstmt, SQLGetTypeInfo(hstmt, SQL_ALL_TYPES));

  ok_stmt(hstmt, SQLNumResultCols(hstmt, &pccol));
  is_num(pccol, 19);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  return OK;
}


DECLARE_TEST(t_getinfo)
{
  SQLCHAR   rgbValue[100];
  SQLSMALLINT pcbInfo;

  ok_con(hdbc, SQLGetInfo(hdbc, SQL_DRIVER_ODBC_VER, rgbValue,
                          sizeof(rgbValue), &pcbInfo));

  printMessage("SQL_DRIVER_ODBC_VER: %s (%d)\n",  rgbValue, pcbInfo);

  return OK;
}


DECLARE_TEST(t_stmt_attr_status)
{
  SQLUSMALLINT rowStatusPtr[3];
  SQLUINTEGER rowsFetchedPtr;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_stmtstatus");
  ok_sql(hstmt, "CREATE TABLE t_stmtstatus (id INT, name CHAR(20))");
  ok_sql(hstmt, "INSERT INTO t_stmtstatus VALUES (10,'data1'),(20,'data2')");

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_SCROLLABLE,
                                (SQLPOINTER)SQL_NONSCROLLABLE, 0));

  ok_sql(hstmt, "SELECT * FROM t_stmtstatus");

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_ROWS_FETCHED_PTR,
                                &rowsFetchedPtr, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_STATUS_PTR,
                                rowStatusPtr, 0));

  expect_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, 2), SQL_ERROR);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_SCROLLABLE,
                                (SQLPOINTER)SQL_SCROLLABLE, 0));

  ok_sql(hstmt, "SELECT * FROM t_stmtstatus");

  ok_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, 2));

  is_num(rowsFetchedPtr, 1);
  is_num(rowStatusPtr[0], 0);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_ROWS_FETCHED_PTR, NULL, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_STATUS_PTR, NULL, 0));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_stmtstatus");

  return OK;
}


DECLARE_TEST(t_msdev_bug)
{
  SQLCHAR    catalog[30];
  SQLINTEGER len;

  ok_con(hdbc, SQLGetConnectOption(hdbc, SQL_CURRENT_QUALIFIER, catalog));
  printMessage("SQL_CURRENT_QUALIFIER: %s\n", catalog);

  ok_con(hdbc, SQLGetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, catalog,
                                 sizeof(catalog), &len));
  printMessage("SQL_ATTR_CURRENT_CATALOG: %s (%d)\n", catalog, len);

  return OK;
}


/**
  Bug #27591: SQLProcedureColumns ]Driver doesn't support this yet
*/
DECLARE_TEST(t_bug27591)
{
  SQLUSMALLINT supported= SQL_TRUE;
  ok_con(hdbc, SQLGetFunctions(hdbc, SQL_API_SQLPROCEDURECOLUMNS, &supported));
  is_num(supported, SQL_FALSE);
  return OK;
}


BEGIN_TESTS
  ADD_TEST(t_gettypeinfo)
  ADD_TEST(t_getinfo)
  ADD_TEST(t_stmt_attr_status)
  ADD_TEST(t_msdev_bug)
  ADD_TEST(t_bug27591)
END_TESTS


RUN_TESTS
