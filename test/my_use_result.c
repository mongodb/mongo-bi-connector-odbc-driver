/*
  Copyright (C) 1997-2007 MySQL AB

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

SQLINTEGER my_max_rows= 100;


/* making use of mysql_use_result */
DECLARE_TEST(t_use_result)
{
  SQLINTEGER i, row_count= 0;
  SQLCHAR    ch[]= "MySQL AB";
  SQLRETURN  rc;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_use_result");
  ok_sql(hstmt, "CREATE TABLE t_use_result (id INT, name CHAR(10))");

  ok_stmt(hstmt, SQLPrepare(hstmt, (SQLCHAR *)
                            "INSERT INTO t_use_result VALUES (?,?)", SQL_NTS));

  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG,
                                  SQL_INTEGER, 0, 0, &i, 0, NULL));

  ok_stmt(hstmt, SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR,
                                  SQL_CHAR, 0, 0, ch, sizeof(ch), NULL));

  for (i= 1; i <= my_max_rows; i++)
    ok_stmt(hstmt, SQLExecute(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_RESET_PARAMS));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "SELECT * FROM t_use_result");

  rc= SQLFetch(hstmt);
  while (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
  {
    row_count++;
    rc= SQLFetch(hstmt);
  }

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  is_num(row_count, my_max_rows);

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_use_result");

  return OK;
}


/**
 Bug #4657: "Don't Cache Results" crashes when using catalog functions
*/
DECLARE_TEST(t_bug4657)
{
  SQLCHAR     name[10];
  SQLSMALLINT column_count;
  SQLLEN      name_length;

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE,
                                (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY, 0));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug4657");
  ok_sql(hstmt, "CREATE TABLE t_bug4657 (a INT)");

  ok_stmt(hstmt, SQLTables(hstmt, (SQLCHAR *)"", SQL_NTS,
                           (SQLCHAR *)"", SQL_NTS,
                           (SQLCHAR *)"", SQL_NTS,
                           (SQLCHAR *)"UNKNOWN", SQL_NTS));

  ok_stmt(hstmt, SQLNumResultCols(hstmt, &column_count));
  is_num(column_count, 5);

  ok_stmt(hstmt, SQLBindCol(hstmt, 3, SQL_C_CHAR, name, sizeof(name),
                            &name_length));
  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug4657");

  return OK;
}


BEGIN_TESTS
  ADD_TEST(t_use_result)
  ADD_TEST(t_bug4657)
END_TESTS


SET_DSN_OPTION(1048576);


RUN_TESTS
