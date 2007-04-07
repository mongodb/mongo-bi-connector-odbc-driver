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

/**
  Test transaction behavior using InnoDB tables
*/
DECLARE_TEST(my_transaction)
{
  SQLHDBC hdbc2;
  SQLHSTMT hstmt2;
  SQLHENV henv2;

  /** @todo need a mechanism for outputting skip results */
  if (!server_supports_trans(hdbc))
    return FAIL;

  alloc_basic_handles(&henv2, &hdbc2, &hstmt2);

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
  ok_sql(hstmt2,"SELECT * FROM t1 WHERE col1 = 30");
  expect_stmt(hstmt2, SQLFetch(hstmt2), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt2, SQLFreeStmt(hstmt2,SQL_CLOSE));

  /* Delete a row to check, and commit the transaction using ENV handler */
  ok_sql(hstmt,"DELETE FROM t1 WHERE col1 = 40");
  ok_con(hdbc, SQLEndTran(SQL_HANDLE_ENV, henv, SQL_COMMIT));

  /* test the results now, select should not find any data */
  ok_sql(hstmt2,"SELECT * FROM t1 WHERE col1 = 40");
  expect_stmt(hstmt2, SQLFetch(hstmt2), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt2, SQLFreeStmt(hstmt2,SQL_CLOSE));

  /* drop the table */
  ok_sql(hstmt,"DROP TABLE t1");

  ok_stmt(hstmt, SQLFreeStmt(hstmt,SQL_CLOSE));

  free_basic_handles(&henv2,&hdbc2,&hstmt2);

  return OK;
}


DECLARE_TEST(t_tran)
{
    SQLRETURN rc;

    if (!server_supports_trans(hdbc))
        return SKIP;

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,SQL_AUTOCOMMIT_OFF);
    mycon(hdbc,rc);

    tmysql_exec(hstmt,"drop table if exists t_tran");

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_tran(col1 int ,col2 varchar(30)) TYPE = InnoDB");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"insert into t_tran values(10,'venu')");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"insert into t_tran values(20,'mysql')");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_ROLLBACK);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_tran");
    mystmt(hstmt,rc);

    my_assert( 1 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,SQL_AUTOCOMMIT_ON);
    mycon(hdbc,rc);

  return OK;
}


DECLARE_TEST(t_tran_ddl)
{
    SQLRETURN rc;
    SQLSMALLINT rgbValue,len;

    if (!server_supports_trans(hdbc))
        return SKIP;

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,SQL_AUTOCOMMIT_OFF);
    mycon(hdbc,rc);

    rc = SQLGetInfo(hdbc,SQL_TXN_CAPABLE,&rgbValue,0,&len);
    mycon(hdbc,rc);
    my_assert(rgbValue == SQL_TC_DDL_COMMIT);
    my_assert(len == 2);

    tmysql_exec(hstmt,"drop table if exists t_tran1");

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_tran1(col1 int ,col2 varchar(30)) TYPE = BDB");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_ROLLBACK);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"drop table if exists t_tran1");
    mystmt(hstmt,rc);

    rc = SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,SQL_AUTOCOMMIT_ON);
    mycon(hdbc,rc);

  return OK;
}


BEGIN_TESTS
  ADD_TEST(my_transaction)
  ADD_TEST(t_tran)
  ADD_TEST(t_tran_ddl)
END_TESTS


RUN_TESTS
