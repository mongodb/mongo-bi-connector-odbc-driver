/*
  Copyright (c) 2003, 2007 MySQL AB, 2010 Sun Microsystems, Inc.
  Use is subject to license terms.

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

#include "odbctap.h"

/**
  Test transaction behavior using InnoDB tables
*/
DECLARE_TEST(my_transaction)
{
  if (!server_supports_trans(hdbc))
    skip("Server does not support transactions.");

  /* set AUTOCOMMIT to OFF */
  ok_con(hdbc, SQLSetConnectAttr(hdbc,SQL_ATTR_AUTOCOMMIT,
                                 (SQLPOINTER)SQL_AUTOCOMMIT_OFF,0));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t1");

  ok_con(hdbc, SQLTransact(NULL,hdbc,SQL_COMMIT));

  /* create the table 't1' using InnoDB */
  ok_sql(hstmt, "CREATE TABLE t1 (col1 INT, col2 VARCHAR(30))"
                " ENGINE = InnoDB");

  /* insert a row and commit the transaction */
  ok_sql(hstmt, "INSERT INTO t1 VALUES(10,'venu')");
  ok_con(hdbc, SQLTransact(NULL,hdbc,SQL_COMMIT));

  /* now insert the second row, but roll back that transaction */
  ok_sql(hstmt,"INSERT INTO t1 VALUES(20,'mysql')");
  ok_con(hdbc, SQLTransact(NULL,hdbc,SQL_ROLLBACK));

  /* delete first row, but roll it back */
  ok_sql(hstmt,"DELETE FROM t1 WHERE col1 = 10");
  ok_con(hdbc, SQLTransact(NULL,hdbc,SQL_ROLLBACK));

  /* Bug #21588: Incomplete ODBC API implementaion */
  /* insert a row, but roll it back using SQLTransact on the environment */
  ok_sql(hstmt,"INSERT INTO t1 VALUES(30,'mysql')");
  ok_con(hdbc, SQLTransact(henv,NULL,SQL_ROLLBACK));

  ok_stmt(hstmt, SQLFreeStmt(hstmt,SQL_CLOSE));

  /* test the results now, only one row should exist */
  ok_sql(hstmt,"SELECT * FROM t1");

  ok_stmt(hstmt, SQLFetch(hstmt));
  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt,SQL_CLOSE));

  /* now insert some more records to check SQLEndTran */
  ok_sql(hstmt,"INSERT INTO t1 "
               "VALUES (30,'test'),(40,'transaction')");
  ok_con(hdbc, SQLTransact(NULL,hdbc,SQL_COMMIT));

  /* Commit the transaction using DBC handler */
  ok_sql(hstmt,"DELETE FROM t1 WHERE col1 = 30");
  ok_con(hdbc, SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT));

  /* test the results now, select should not find any data */
  ok_sql(hstmt,"SELECT * FROM t1 WHERE col1 = 30");
  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt,SQL_CLOSE));

  /* Delete a row to check, and commit the transaction using ENV handler */
  ok_sql(hstmt,"DELETE FROM t1 WHERE col1 = 40");
  ok_con(hdbc, SQLEndTran(SQL_HANDLE_ENV, henv, SQL_COMMIT));

  /* test the results now, select should not find any data */
  ok_sql(hstmt,"SELECT * FROM t1 WHERE col1 = 40");
  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt,SQL_CLOSE));

  /* drop the table */
  ok_sql(hstmt,"DROP TABLE t1");

  ok_stmt(hstmt, SQLFreeStmt(hstmt,SQL_CLOSE));

  return OK;
}


DECLARE_TEST(t_tran)
{
  if (!server_supports_trans(hdbc))
    skip("Server does not support transactions.");

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_tran");
  ok_sql(hstmt, "CREATE TABLE t_tran (a INT, b VARCHAR(30)) ENGINE=InnoDB");

  ok_con(hdbc, SQLSetConnectOption(hdbc, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF));

  ok_sql(hstmt, "INSERT INTO t_tran VALUES (10, 'venu')");
  ok_stmt(hstmt, SQLTransact(NULL, hdbc, SQL_COMMIT));

  ok_sql(hstmt, "INSERT INTO t_tran VALUES (20, 'mysql')");
  ok_stmt(hstmt, SQLTransact(NULL, hdbc, SQL_ROLLBACK));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "SELECT * FROM t_tran");

  is_num(myrowcount(hstmt), 1);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_con(hdbc, SQLSetConnectOption(hdbc, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_ON));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_tran");

  return OK;
}


/**
  Test retrieval and setting of transaction isolation level.
*/
DECLARE_TEST(t_isolation)
{
  SQLINTEGER isolation;
  SQLCHAR    tx_isolation[20];

  if (!server_supports_trans(hdbc))
    skip("Server does not support transactions.");

  /* Check that the default is REPEATABLE READ. */
  ok_con(hdbc, SQLGetConnectAttr(hdbc, SQL_ATTR_TXN_ISOLATION, &isolation,
                                 SQL_IS_POINTER, NULL));
  is_num(isolation, SQL_TXN_REPEATABLE_READ);

  /* Change it to READ UNCOMMITTED. */
  ok_con(hdbc, SQLSetConnectAttr(hdbc, SQL_ATTR_TXN_ISOLATION,
                                 (SQLPOINTER)SQL_TXN_READ_UNCOMMITTED, 0));

  /* Check that the driver has rmeembered the new value. */
  ok_con(hdbc, SQLGetConnectAttr(hdbc, SQL_ATTR_TXN_ISOLATION, &isolation,
                                 SQL_IS_POINTER, NULL));
  is_num(isolation, SQL_TXN_READ_UNCOMMITTED);

  /* Check that it was actually changed on the server. */
  ok_sql(hstmt, "SELECT @@tx_isolation");
  ok_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_CHAR, tx_isolation,
                            sizeof(tx_isolation), NULL));
  ok_stmt(hstmt, SQLFetch(hstmt));
  is_str(tx_isolation, "READ-UNCOMMITTED", 16);

  return OK;
}


BEGIN_TESTS
  ADD_TEST(my_transaction)
  ADD_TEST(t_tran)
  ADD_TEST(t_isolation)
END_TESTS


RUN_TESTS
