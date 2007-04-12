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

SQLINTEGER my_max_rows= 10000;

/* Clean data */
DECLARE_TEST(t_clean_data)
{
  ok_sql(hstmt, "DROP TABLE IF EXISTS t_use_result");

  return OK;
}


/* Initialize data */
DECLARE_TEST(t_init_data)
{
  SQLINTEGER  i;
  SQLCHAR     ch[]= "MySQL AB";

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

  return OK;
}


/* Fetch and count the time */
int t_fetch_data(SQLHSTMT hstmt)
{
  SQLRETURN rc;
  SQLINTEGER row_count= 0;

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

  return OK;
}


/* making use of mysql_use_result */
DECLARE_TEST(t_use_result)
{
  SQLHENV   henv1;
  SQLHDBC   hdbc1;
  SQLHSTMT  hstmt1;
  int ret;

  SET_DSN_OPTION(131072L * 8);

  alloc_basic_handles(&henv1, &hdbc1, &hstmt1);
  ret= t_fetch_data(hstmt1);
  free_basic_handles(&henv1, &hdbc1, &hstmt1);

  return ret;
}

/* making use of mysql_store_result */
DECLARE_TEST(t_store_result)
{
  SQLHENV   henv1;
  SQLHDBC   hdbc1;
  SQLHSTMT  hstmt1;
  int ret;

  SET_DSN_OPTION(3);

  alloc_basic_handles(&henv1, &hdbc1, &hstmt1);
  /* we re-use option here */
  ret= t_fetch_data(hstmt1);
  free_basic_handles(&henv1, &hdbc1, &hstmt1);

  return ret;
}


BEGIN_TESTS
  ADD_TEST(t_init_data)
  ADD_TEST(t_use_result)
  ADD_TEST(t_store_result)
  ADD_TEST(t_clean_data)
END_TESTS


RUN_TESTS
