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


DECLARE_TEST(t_odbc3_error)
{
  SQLHENV henv1;
  SQLHDBC hdbc1;
  SQLHSTMT hstmt1;
  SQLINTEGER ov_version;

  ok_env(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
  ok_env(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                              (SQLPOINTER)SQL_OV_ODBC3, 0));

  ok_env(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &hdbc1));

  ok_env(henv1, SQLGetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                              (SQLPOINTER)&ov_version, 0, 0));
  is_num(ov_version, SQL_OV_ODBC3);

  ok_con(hdbc1, SQLConnect(hdbc1, mydsn, SQL_NTS, myuid, SQL_NTS,
                           mypwd, SQL_NTS));

  ok_con(hdbc1, SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1));

  expect_sql(hstmt1, "SELECT * FROM non_existing_table", SQL_ERROR);
  if (check_sqlstate(hstmt1, "42S02") != OK)
    return FAIL;

  ok_sql(hstmt1, "DROP TABLE IF EXISTS t_error");
  ok_sql(hstmt1, "CREATE TABLE t_error (id INT)");

  expect_sql(hstmt1, "CREATE TABLE t_error (id INT)", SQL_ERROR);
  if (check_sqlstate(hstmt1, "42S01") != OK)
    return FAIL;

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  expect_stmt(hstmt1, SQLSetStmtAttr(hstmt1, SQL_ATTR_FETCH_BOOKMARK_PTR,
                                     (SQLPOINTER)NULL, 0),
              SQL_ERROR);
  if (check_sqlstate(hstmt1, "HYC00") != OK)
    return FAIL;

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  ok_sql(hstmt1, "DROP TABLE IF EXISTS t_error");

  ok_con(hdbc1, SQLDisconnect(hdbc1));

  ok_con(hdbc1, SQLFreeHandle(SQL_HANDLE_DBC, hdbc1));

  ok_env(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

  return OK;
}


DECLARE_TEST(t_odbc2_error)
{
  SQLHENV henv1;
  SQLHDBC hdbc1;
  SQLHSTMT hstmt1;
  SQLINTEGER ov_version;

  ok_env(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
  ok_env(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                              (SQLPOINTER)SQL_OV_ODBC2, 0));

  ok_env(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &hdbc1));

  ok_env(henv1, SQLGetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                              (SQLPOINTER)&ov_version, 0, 0));
  is_num(ov_version, SQL_OV_ODBC2);

  ok_con(hdbc1, SQLConnect(hdbc1, mydsn, SQL_NTS, myuid, SQL_NTS,
                           mypwd, SQL_NTS));

  ok_con(hdbc1, SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1));

  expect_sql(hstmt1, "SELECT * FROM non_existing_table", SQL_ERROR);
  if (check_sqlstate(hstmt1, "S0002") != OK)
    return FAIL;

  ok_sql(hstmt1, "DROP TABLE IF EXISTS t_error");
  ok_sql(hstmt1, "CREATE TABLE t_error (id INT)");

  expect_sql(hstmt1, "CREATE TABLE t_error (id INT)", SQL_ERROR);
  if (check_sqlstate(hstmt1, "S0001") != OK)
    return FAIL;

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  expect_stmt(hstmt1, SQLSetStmtAttr(hstmt1, SQL_ATTR_FETCH_BOOKMARK_PTR,
                                     (SQLPOINTER)NULL, 0),
              SQL_ERROR);
  if (check_sqlstate(hstmt1, "S1C00") != OK)
    return FAIL;

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  ok_sql(hstmt1, "DROP TABLE IF EXISTS t_error");

  ok_con(hdbc1, SQLDisconnect(hdbc1));

  ok_con(hdbc1, SQLFreeHandle(SQL_HANDLE_DBC, hdbc1));

  ok_env(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

  return OK;
}


DECLARE_TEST(t_diagrec)
{
  SQLCHAR   sqlstate[6]= {0};
  SQLCHAR   message[255]= {0};
  SQLINTEGER native_err= 0;
  SQLSMALLINT msglen= 0;

  expect_sql(hstmt, "DROP TABLE t_odbc3_non_existent_table", SQL_ERROR);

#if UNIXODBC_BUG_FIXED
  /*
   This should report no data found, but unixODBC doesn't even pass this
   down to the driver.
  */
  expect_stmt(hstmt, SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 2, sqlstate,
                                   &native_err, message, 0, &msglen),
              SQL_NO_DATA_FOUND);
#endif

  ok_stmt(hstmt, SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate,
                               &native_err, message, 255, &msglen));

  expect_stmt(hstmt, SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate,
                                   &native_err, message, 0, &msglen),
              SQL_SUCCESS_WITH_INFO);

  expect_stmt(hstmt, SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate,
                                   &native_err, message, 10, &msglen),
              SQL_SUCCESS_WITH_INFO);

  expect_stmt(hstmt, SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate,
                                   &native_err, message, -1, &msglen),
              SQL_ERROR);

  return OK;
}


DECLARE_TEST(t_warning)
{
  SQLCHAR    szData[20];
  SQLLEN     pcbValue;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_warning");
  ok_sql(hstmt, "CREATE TABLE t_warning (col2 CHAR(20))");
  ok_sql(hstmt, "INSERT INTO t_warning VALUES ('Venu Anuganti')");

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY,
                                (SQLPOINTER)SQL_CONCUR_ROWVER, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE,
                                (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, 0));

  /* ignore all columns */
  ok_sql(hstmt, "SELECT * FROM t_warning");

  ok_stmt(hstmt, SQLFetch(hstmt));

  expect_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, szData, 4, &pcbValue),
              SQL_SUCCESS_WITH_INFO);
  is_str(szData, "Ven", 3);
  is_num(pcbValue, 13);

  expect_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, szData, 4, &pcbValue),
              SQL_SUCCESS_WITH_INFO);
  is_str(szData, "u A", 3);
  is_num(pcbValue, 10);

  expect_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, szData, 4, &pcbValue),
              SQL_SUCCESS_WITH_INFO);
  is_str(szData, "nug", 3);
  is_num(pcbValue, 7);

  expect_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, szData, 4, &pcbValue),
              SQL_SUCCESS_WITH_INFO);
  is_str(szData, "ant", 3);
  is_num(pcbValue, 4);

  expect_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, szData, 4, &pcbValue),
              SQL_SUCCESS);
  is_str(szData, "i", 1);
  is_num(pcbValue, 1);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_warning");

  return OK;
}


DECLARE_TEST(t_bug3456)
{
  SQLINTEGER connection_id;
  char buf[100];
  SQLHENV henv2;
  SQLHDBC  hdbc2;
  SQLHSTMT hstmt2;

  /* Create a new connection that we deliberately will kill */
  alloc_basic_handles(&henv2, &hdbc2, &hstmt2);
  ok_sql(hstmt2, "SELECT connection_id()");
  ok_stmt(hstmt2, SQLFetch(hstmt2));
  connection_id= my_fetch_int(hstmt2, 1);
  ok_stmt(hstmt2, SQLFreeStmt(hstmt2, SQL_CLOSE));

  /* From another connection, kill the connection created above */
  sprintf(buf, "KILL %d", connection_id);
  ok_stmt(hstmt, SQLExecDirect(hstmt, (SQLCHAR *)buf, SQL_NTS));

  /* Now check that the connection killed returns the right SQLSTATE */
  expect_sql(hstmt2, "SELECT connection_id()", SQL_ERROR);

  return check_sqlstate(hstmt2, "08S01");
}


/*
 * Bug #16224: Calling SQLGetDiagField with RecNumber 0,DiagIdentifier
 *             NOT 0 returns SQL_ERROR
 */
DECLARE_TEST(t_bug16224)
{
  SQLINTEGER diagcnt;

  expect_sql(hstmt, "This is an invalid Query! (odbc test)", SQL_ERROR);

  ok_stmt(hstmt, SQLGetDiagField(SQL_HANDLE_STMT, hstmt, 0,
                                 SQL_DIAG_NUMBER, &diagcnt,
                                 SQL_IS_INTEGER, NULL));
  is_num(diagcnt, 1);

  return OK;
}


/*
 * Test that binding invalid column returns the appropriate error
 */
DECLARE_TEST(bind_invalidcol)
{
  ok_sql(hstmt, "select 1,2,3,4");

  /* test out of range column number */
  expect_stmt(hstmt, SQLBindCol(hstmt, 10, SQL_C_CHAR, "", 4, NULL), SQL_ERROR);
  is(check_sqlstate(hstmt, "HY002") == OK);

  /* test (unsupported) bookmark column number */
  expect_stmt(hstmt, SQLBindCol(hstmt, 0, SQL_C_BOOKMARK, "", 4, NULL),
              SQL_ERROR);
  is(check_sqlstate(hstmt, "HY002") == OK);

  return OK;
}


/*
 * Test that calling SQLGetData() without a nullind ptr
 * when the data is null returns an error.
 */
DECLARE_TEST(getdata_need_nullind)
{
  SQLINTEGER i;
  ok_sql(hstmt, "select 1 as i , null as j ");
  ok_stmt(hstmt, SQLFetch(hstmt));
  
  /* that that nullind ptr is ok when data isn't null */
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_LONG, &i, 0, NULL));

  /* now that it's an error */
  expect_stmt(hstmt, SQLGetData(hstmt, 2, SQL_C_LONG, &i, 0, NULL),
              SQL_ERROR);
  return check_sqlstate(hstmt, "22002");
}


/**
 Bug #27158: MyODBC 3.51/ ADO cmd option adCmdUnknown won't work for tables - regression
*/
DECLARE_TEST(t_bug27158)
{
  expect_sql(hstmt, "{ CALL test.does_not_exist }", SQL_ERROR);
  return check_sqlstate(hstmt, "42000");
}


BEGIN_TESTS
#ifndef NO_DRIVERMANAGER
  ADD_TEST(t_odbc2_error)
  ADD_TEST(t_odbc3_error)
  /* Run twice to test the driver's handling of switching  */
  ADD_TEST(t_odbc2_error)
#endif
  ADD_TEST(t_diagrec)
  ADD_TEST(t_warning)
  ADD_TEST(t_bug3456)
  ADD_TEST(t_bug16224)
  ADD_TEST(bind_invalidcol)
  ADD_TEST(getdata_need_nullind)
  ADD_TEST(t_bug27158)
END_TESTS

RUN_TESTS
