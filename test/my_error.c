/*
  Copyright (c) 2003, 2013, Oracle and/or its affiliates. All rights reserved.

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


DECLARE_TEST(t_odbc3_error)
{
  DECLARE_BASIC_HANDLES(henv1, hdbc1, hstmt1);
  SQLINTEGER ov_version;

  is(OK == alloc_basic_handles(&henv1, &hdbc1, &hstmt1));

  ok_env(henv1, SQLGetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                              (SQLPOINTER)&ov_version, 0, 0));
  is_num(ov_version, SQL_OV_ODBC3);


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

  free_basic_handles(&henv1, &hdbc1, &hstmt1);

  return OK;
}


DECLARE_TEST(t_odbc2_error)
{
  DECLARE_BASIC_HANDLES(henv1, hdbc1, hstmt1);
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

  expect_stmt(hstmt1, SQLSetStmtAttr(hstmt1, SQL_ATTR_ENABLE_AUTO_IPD,
                                     (SQLPOINTER)SQL_TRUE, 0),
              SQL_ERROR);
  if (check_sqlstate(hstmt1, "S1C00") != OK)
    return FAIL;

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  ok_sql(hstmt1, "DROP TABLE IF EXISTS t_error");

  ok_con(hdbc1, SQLDisconnect(hdbc1));

  free_basic_handles(&henv1, &hdbc1, &hstmt1);

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

  /* MSSQL returns SQL_SUCCESS in similar situations */
  expect_stmt(hstmt, SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate,
                                   &native_err, message, 0, &msglen),
              SQL_SUCCESS);

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
  SQLCHAR dummy[10];
  ok_sql(hstmt, "select 1,2,3,4");

  /* test out of range column number */
  expect_stmt(hstmt, SQLBindCol(hstmt, 10, SQL_C_CHAR, "", 4, NULL), SQL_ERROR);
  is_num(check_sqlstate(hstmt, "07009"), OK);

  /* test (unsupported) bookmark column number */
  expect_stmt(hstmt, SQLBindCol(hstmt, 0, SQL_C_BOOKMARK, "", 4, NULL),
              SQL_ERROR);
  is_num(check_sqlstate(hstmt, "07009"), OK);

  /* SQLDescribeCol() */
  expect_stmt(hstmt, SQLDescribeCol(hstmt, 0, dummy, sizeof(dummy), NULL, NULL,
                                    NULL, NULL, NULL), SQL_ERROR);
  /* Older versions of iODBC return the wrong result (S1002) here. */
  is(check_sqlstate(hstmt, "07009") == OK ||
     check_sqlstate(hstmt, "S1002") == OK);

  expect_stmt(hstmt, SQLDescribeCol(hstmt, 5, dummy, sizeof(dummy), NULL,
                                    NULL, NULL, NULL, NULL), SQL_ERROR);
  is_num(check_sqlstate(hstmt, "07009"), OK);

  /* SQLColAttribute() */
  expect_stmt(hstmt, SQLColAttribute(hstmt, 0, SQL_DESC_NAME, NULL, 0,
                                     NULL, NULL), SQL_ERROR);
  is_num(check_sqlstate(hstmt, "07009"), OK);

  expect_stmt(hstmt, SQLColAttribute(hstmt, 7, SQL_DESC_NAME, NULL, 0,
                                     NULL, NULL), SQL_ERROR);
  is_num(check_sqlstate(hstmt, "07009"), OK);

  return OK;
}


/*
 * Test the error given when not enough params are bound to execute
 * the statement.
 */
DECLARE_TEST(bind_notenoughparam1)
{
  SQLINTEGER i= 0;
  ok_stmt(hstmt, SQLPrepare(hstmt, (SQLCHAR *)"select ?, ?", SQL_NTS));

  ok_stmt(hstmt, SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG,
                                  SQL_INTEGER, 0, 0, &i, 0, NULL));
  expect_stmt(hstmt, SQLExecute(hstmt), SQL_ERROR);
  return check_sqlstate(hstmt, "07001");
}


/*
 * Test the error given when not enough params are bound to execute
 * the statement, also given that a pre-execute happens (due to calling
 * SQLNumResultCols).
 */
DECLARE_TEST(bind_notenoughparam2)
{
  SQLINTEGER i= 0;
  SQLSMALLINT cols= 0;
  ok_stmt(hstmt, SQLPrepare(hstmt, (SQLCHAR *)"select ?, ?", SQL_NTS));

  /* trigger pre-execute */
  ok_stmt(hstmt, SQLNumResultCols(hstmt, &cols));
  is_num(cols, 2);

  ok_stmt(hstmt, SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG,
                                  SQL_INTEGER, 0, 0, &i, 0, NULL));
  expect_stmt(hstmt, SQLExecute(hstmt), SQL_ERROR);
  return check_sqlstate(hstmt, "07001");
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


/*
   Handle-specific tests for env and dbc diagnostics
*/
DECLARE_TEST(t_handle_err)
{
  SQLHANDLE henv1, hdbc1;

  ok_env(henv1, SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv1));
  ok_env(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                              (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER));
  ok_env(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &hdbc1));

  /* we have to connect for the DM to pass the calls to the driver */
  ok_con(hdbc1, SQLConnect(hdbc1, mydsn, SQL_NTS, myuid, SQL_NTS,
                           mypwd, SQL_NTS));

  expect_env(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                  (SQLPOINTER)SQL_OV_ODBC3, 0), SQL_ERROR);
  is_num(check_sqlstate_ex(henv1, SQL_HANDLE_ENV, "HY010"), OK);

  expect_dbc(hdbc1, SQLSetConnectAttr(hdbc1, SQL_ATTR_ASYNC_ENABLE,
                                      (SQLPOINTER)SQL_ASYNC_ENABLE_ON,
                                      SQL_IS_INTEGER), SQL_SUCCESS_WITH_INFO);
  is_num(check_sqlstate_ex(hdbc1, SQL_HANDLE_DBC, "01S02"), OK);

  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeHandle(SQL_HANDLE_DBC, hdbc1));
  ok_env(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

  return OK;
}


DECLARE_TEST(sqlerror)
{
  SQLCHAR message[SQL_MAX_MESSAGE_LENGTH + 1];
  SQLCHAR sqlstate[SQL_SQLSTATE_SIZE + 1];
  SQLINTEGER error;
  SQLSMALLINT len;

  expect_sql(hstmt, "SELECT * FROM tabledoesnotexist", SQL_ERROR);

  ok_stmt(hstmt, SQLError(henv, hdbc, hstmt, sqlstate, &error,
                          message, sizeof(message), &len));

  /* Message has been consumed. */
  expect_stmt(hstmt, SQLError(henv, hdbc, hstmt, sqlstate, &error,
                              message, sizeof(message), &len),
              SQL_NO_DATA_FOUND);

  /* But should still be available using SQLGetDiagRec. */
  ok_stmt(hstmt, SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate, &error,
                               message, sizeof(message), &len));

  return OK;
}


/**
 Bug #27158: MyODBC 3.51/ ADO cmd option adCmdUnknown won't work for tables - regression
*/
DECLARE_TEST(t_bug27158)
{
  expect_sql(hstmt, "{ CALL test.does_not_exist }", SQL_ERROR);
  return check_sqlstate(hstmt, "42000");
}


DECLARE_TEST(t_bug13542600)
{
  SQLINTEGER i;

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  ok_sql(hstmt, "select 1 as i , null as j ");
  ok_stmt(hstmt, SQLBindCol(hstmt, 2, SQL_C_LONG, &i, 0, NULL));

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_ERROR);

  return check_sqlstate(hstmt, "22002");
}


DECLARE_TEST(t_bug14285620)
{
  SQLSMALLINT data_type, dec_digits, nullable, cblen;
  SQLUINTEGER info, col_size; 
  SQLINTEGER timeout= 20, cbilen;
  SQLCHAR szData[255]={0};

  /* Numeric attribute */
  expect_dbc(hdbc, SQLGetConnectAttr(hdbc, SQL_ATTR_LOGIN_TIMEOUT, NULL, 0, NULL), SQL_SUCCESS);
  expect_dbc(hdbc, SQLGetConnectAttr(hdbc, SQL_ATTR_LOGIN_TIMEOUT, &timeout, 0, NULL), SQL_SUCCESS);
  is_num(timeout, 0);
  
  /* Character attribute */
  /* 
    In this particular case MSSQL always returns SQL_SUCCESS_WITH_INFO
    apparently because the driver manager always provides the buffer even
    when the client application passes NULL
  */
#ifdef _WIN32
  /* 
    This check is only relevant to Windows Driver Manager 
  */
  expect_dbc(hdbc, SQLGetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, NULL, 0, NULL), SQL_SUCCESS_WITH_INFO);
#endif
  expect_dbc(hdbc, SQLGetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, szData, 0, NULL), SQL_SUCCESS_WITH_INFO);

  /*
  MSDN Says about the last parameter &cblen for SQLGetInfo,
  functions:
  
     If InfoValuePtr is NULL, StringLengthPtr will still return the 
     total number of bytes (excluding the null-termination character 
     for character data) available to return in the buffer pointed 
     to by InfoValuePtr.

     http://msdn.microsoft.com/en-us/library/ms710297%28v=vs.85%29
  */

  expect_dbc(hdbc, SQLGetInfo(hdbc, SQL_AGGREGATE_FUNCTIONS, NULL, 0, NULL), SQL_SUCCESS);
  expect_dbc(hdbc, SQLGetInfo(hdbc, SQL_AGGREGATE_FUNCTIONS, &info, 0, &cblen), SQL_SUCCESS);
  is_num(cblen, 4);

  is_num(info, (SQL_AF_ALL | SQL_AF_AVG | SQL_AF_COUNT | SQL_AF_DISTINCT | 
             SQL_AF_MAX | SQL_AF_MIN | SQL_AF_SUM));

  /* Get database name for further checks */
  expect_dbc(hdbc, SQLGetInfo(hdbc, SQL_DATABASE_NAME, szData, sizeof(szData), NULL), SQL_SUCCESS);
  expect_dbc(hdbc, SQLGetInfo(hdbc, SQL_DATABASE_NAME, NULL, 0, &cblen), SQL_SUCCESS);

#ifdef _WIN32  
  /* Windows uses unicode driver by default */
  is_num(cblen, strlen(szData)*sizeof(SQLWCHAR));
#else
  is_num(cblen, strlen(szData));
#endif

  expect_dbc(hdbc, SQLGetInfo(hdbc, SQL_DATABASE_NAME, szData, 0, NULL), SQL_SUCCESS);

  /* Get the native string for further checks */
  expect_dbc(hdbc, SQLNativeSql(hdbc, "SELECT 10", SQL_NTS, szData, sizeof(szData), NULL), SQL_SUCCESS);
  expect_dbc(hdbc, SQLNativeSql(hdbc, "SELECT 10", SQL_NTS, NULL, 0, &cbilen), SQL_SUCCESS);
  
  /* Do like MSSQL, which does calculate as char_count*sizeof(SQLWCHAR) */
  is_num(cbilen, strlen(szData));

  expect_dbc(hdbc, SQLNativeSql(hdbc, "SELECT 10", SQL_NTS, szData, 0, NULL), SQL_SUCCESS_WITH_INFO);

  /* Get the cursor name for further checks */
  expect_stmt(hstmt, SQLGetCursorName(hstmt, szData, sizeof(szData), NULL), SQL_SUCCESS);
  expect_stmt(hstmt, SQLGetCursorName(hstmt, NULL, 0, &cblen), SQL_SUCCESS);
  
  /* Do like MSSQL, which does calculate as char_count*sizeof(SQLWCHAR) */
  is_num(cblen, strlen(szData));

  expect_stmt(hstmt, SQLGetCursorName(hstmt, szData, 0, NULL), SQL_SUCCESS_WITH_INFO);

  expect_stmt(hstmt, SQLExecDirect(hstmt, "ERROR SQL QUERY", SQL_NTS), SQL_ERROR);

  expect_stmt(hstmt, SQLGetDiagField(SQL_HANDLE_STMT, hstmt, 1, SQL_DIAG_SQLSTATE, NULL, 0, NULL), SQL_SUCCESS);

  {
    SQLCHAR sqlstate[30], message[255];
    SQLINTEGER native_error= 0;
    SQLSMALLINT text_len= 0;
    /* try with the NULL pointer for Message */
    expect_stmt(hstmt, SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate,
                                    &native_error, NULL, 0, &cblen), SQL_SUCCESS);
    /* try with the non-NULL pointer for Message */
    expect_stmt(hstmt, SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate,
                                    &native_error, message, 0, NULL), SQL_SUCCESS);
  }

  SQLExecDirect(hstmt, "drop table bug14285620", SQL_NTS);

  ok_stmt(hstmt, SQLExecDirect(hstmt, "CREATE TABLE bug14285620 (id INT)", SQL_NTS));
  ok_stmt(hstmt, SQLExecDirect(hstmt, "INSERT INTO bug14285620 (id) VALUES (1)", SQL_NTS));
  ok_stmt(hstmt, SQLExecDirect(hstmt, "SELECT * FROM bug14285620", SQL_NTS));

  expect_stmt(hstmt, SQLDescribeCol(hstmt, 1, NULL, 0, NULL, &data_type, &col_size, &dec_digits, &nullable), SQL_SUCCESS);
  expect_stmt(hstmt, SQLDescribeCol(hstmt, 1, szData, 0, &cblen, &data_type, &col_size, &dec_digits, &nullable), SQL_SUCCESS_WITH_INFO);

  expect_stmt(hstmt, SQLColAttribute(hstmt,1, SQL_DESC_TYPE, NULL, 0, NULL, NULL), SQL_SUCCESS);
  expect_stmt(hstmt, SQLColAttribute(hstmt,1, SQL_DESC_TYPE, &data_type, 0, NULL, NULL), SQL_SUCCESS);

  expect_stmt(hstmt, SQLColAttribute(hstmt,1, SQL_DESC_TYPE_NAME, NULL, 0, NULL, NULL), SQL_SUCCESS);
  expect_stmt(hstmt, SQLColAttribute(hstmt,1, SQL_DESC_TYPE_NAME, szData, 0, NULL, NULL), SQL_SUCCESS_WITH_INFO);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  return OK;
}

/*
  Bug49466: SQLMoreResults does not set statement errors correctly
  Wrong error message returned from SQLMoreResults
*/
DECLARE_TEST(t_bug49466)
{
  DECLARE_BASIC_HANDLES(henv1, hdbc1, hstmt1);
  SQLCHAR message[SQL_MAX_MESSAGE_LENGTH + 1];
  SQLCHAR sqlstate[SQL_SQLSTATE_SIZE + 1];
  SQLINTEGER error;
  SQLSMALLINT len;

  is(OK == alloc_basic_handles_with_opt(&henv1, &hdbc1, &hstmt1, NULL,
                                        NULL, NULL, NULL, 
                                        "OPTION=67108864;NO_SSPS=1"));

  ok_stmt(hstmt1, SQLExecDirect(hstmt1, "SELECT 100; CALL t_bug49466proc()", SQL_NTS));

  ok_stmt(hstmt1, SQLFetch(hstmt1));
  is_num(my_fetch_int(hstmt1, 1), 100);

  SQLMoreResults(hstmt1);

  ok_stmt(hstmt, SQLGetDiagRec(SQL_HANDLE_STMT, hstmt1, 1, sqlstate, &error,
                               message, sizeof(message), &len));
  is_num(error, 1305);
  is(strstr((char *)message, "t_bug49466proc does not exist"));

  free_basic_handles(&henv1, &hdbc1, &hstmt1);

  return OK;
}


DECLARE_TEST(t_passwordexpire)
{
  SQLHDBC hdbc1;
  SQLHSTMT hstmt1;

  if (!mysql_min_version(hdbc, "5.6.6", 5))
  {
    skip("The server does not support tested functionality(expired password)");
  }

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_password_expire");
  SQLExecDirect(hstmt, (SQLCHAR *)"DROP USER t_pwd_expire", SQL_NTS);

  ok_sql(hstmt, "GRANT ALL ON *.* TO  t_pwd_expire IDENTIFIED BY 'foo'");
  ok_sql(hstmt, "ALTER USER t_pwd_expire PASSWORD EXPIRE");

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));

  /* Expecting error without OPT_CAN_HANDLE_EXPIRED_PASSWORDS */
  expect_dbc(hdbc1, get_connection(&hdbc1, NULL, "t_pwd_expire", "foo", 
             NULL, NULL), SQL_ERROR);

  {
    SQLCHAR sql_state[6];
    SQLINTEGER  err_code= 0;
    SQLCHAR     err_msg[SQL_MAX_MESSAGE_LENGTH]= {0};
    SQLSMALLINT err_len= 0;

    SQLGetDiagRec(SQL_HANDLE_DBC, hdbc1, 1, sql_state, &err_code, err_msg,
                  SQL_MAX_MESSAGE_LENGTH - 1, &err_len);

    /* ER_MUST_CHANGE_PASSWORD = 1820, ER_MUST_CHANGE_PASSWORD_LOGIN = 1862 */
    if (strncmp(sql_state, "08004", 5) != 0 || !(err_code == 1820 || err_code == 1862))
    {
      printMessage("%s %d %s", sql_state, err_code, err_msg);
      is(FALSE);
    }
  }

  /* Expecting error as password has not been reset */
  ok_con(hdbc1, get_connection(&hdbc1, NULL, "t_pwd_expire", "foo",
                                NULL, "CAN_HANDLE_EXP_PWD=1"));

  /*strcat((char *)conn_in, ";INITSTMT={set password= password('bar')}");*/
  ok_con(hdbc1, SQLAllocStmt(hdbc1, &hstmt1));

  ok_sql(hstmt1, "SET PASSWORD= password('bar')");

  /* Just to verify that we got normal connection */
  ok_sql(hstmt1, "select 1");

  ok_con(hdbc1, SQLFreeStmt(hstmt1, SQL_DROP));

  ok_con(hdbc1, SQLDisconnect(hdbc1));

  /* Checking we can get connection with new credentials */
  ok_con(hdbc1, get_connection(&hdbc1, mydsn, "t_pwd_expire", "bar", NULL,
                               NULL));
  ok_con(hdbc1, SQLAllocStmt(hdbc1, &hstmt1));

  /* Also verifying that we got normal connection */
  ok_sql(hstmt1, "select 1");

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));
  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_password_expire");
  ok_sql(hstmt, "DROP USER t_pwd_expire");

  return OK;
}

/*
  Bug#16445091: CLEARTEXT AUTHENTICATION NOT PRESENT IN ODBC
*/
DECLARE_TEST(t_cleartext_password)
{
  SQLHDBC hdbc1;
  SQLCHAR sql_state[6];
  SQLINTEGER  err_code= 0;                              
  SQLCHAR     err_msg[SQL_MAX_MESSAGE_LENGTH]= {0};
  SQLSMALLINT err_len= 0;
  unsigned int major1= 0, minor1= 0, build1= 0;

  if (!mysql_min_version(hdbc, "5.5.16", 6) )
  {
    skip("The server does not support tested functionality(Cleartext Auth)");
  }

  SQLExecDirect(hstmt, (SQLCHAR *)"DROP USER 't_ct_user'@'%'", SQL_NTS);

  if (!SQL_SUCCEEDED(SQLExecDirect(hstmt, 
            "GRANT ALL ON *.* TO "
            "'t_ct_user'@'%' IDENTIFIED WITH "
            "'authentication_pam'", SQL_NTS))) 
  {
    skip("The authentication_pam plugin not loaded");
  }

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));

  /* 
    Expecting error CR_AUTH_PLUGIN_CANNOT_LOAD_ERROR 
    without option ENABLE_CLEARTEXT_PLUGIN
  */
  if(!SQL_SUCCEEDED(get_connection(&hdbc1, mydsn, "t_ct_user", "t_ct_pass",
                        mydb, NULL)))
  {
    SQLGetDiagRec(SQL_HANDLE_DBC, hdbc1, 1, sql_state, &err_code, err_msg,
                  SQL_MAX_MESSAGE_LENGTH - 1, &err_len);

    printMessage("%s %d %s", sql_state, err_code, err_msg);
    if ((strncmp(sql_state, "08004", 5) != 0 || err_code != 2059))
    {                                                                               
      return FAIL;
    }
  }  

  /* 
    Expecting error other then CR_AUTH_PLUGIN_CANNOT_LOAD_ERROR 
    as option ENABLE_CLEARTEXT_PLUGIN is used
  */
  if(!SQL_SUCCEEDED(get_connection(&hdbc1, mydsn, "t_ct_user", "t_ct_pass",
                        mydb, "ENABLE_CLEARTEXT_PLUGIN=1")))
  {
    SQLGetDiagRec(SQL_HANDLE_DBC, hdbc1, 1, sql_state, &err_code, err_msg,
                  SQL_MAX_MESSAGE_LENGTH - 1, &err_len);
    printMessage("%s %d %s", sql_state, err_code, err_msg);

    if ((strncmp(sql_state, "08004", 5) == 0 && err_code == 2059))
    {                                                                               
      return FAIL;
    }
  }

  ok_sql(hstmt, "DROP USER 't_ct_user'@'%'");

  return OK;
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
  ADD_TEST(bind_notenoughparam1)
  ADD_TEST(bind_notenoughparam2)
  ADD_TEST(getdata_need_nullind)
  ADD_TEST(t_handle_err)
  ADD_TEST(sqlerror)
  ADD_TEST(t_bug27158)
  ADD_TEST(t_bug13542600)
  ADD_TEST(t_bug14285620)
  ADD_TOFIX(t_bug49466)
  ADD_TEST(t_passwordexpire)
  ADD_TEST(t_cleartext_password)
END_TESTS

RUN_TESTS
