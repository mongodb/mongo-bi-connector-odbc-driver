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
#include <wchar.h>


DECLARE_TEST(sqlconnect)
{
  HDBC hdbc1;

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));
  ok_con(hdbc1, SQLConnectW(hdbc1,
                            WC(mydsn), SQL_NTS,
                            WC(myuid), SQL_NTS,
                            WC(mypwd), SQL_NTS));
  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


DECLARE_TEST(sqlprepare)
{
  HDBC hdbc1;
  HSTMT hstmt1;
  SQLINTEGER data;
  SQLWCHAR wbuff[MAX_ROW_DATA_LEN+1];

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));
  ok_con(hdbc1, SQLConnectW(hdbc1,
                            WC(mydsn), SQL_NTS,
                            WC(myuid), SQL_NTS,
                            WC(mypwd), SQL_NTS));

  ok_con(hdbc, SQLAllocStmt(hdbc1, &hstmt1));

  ok_stmt(hstmt1, SQLPrepareW(hstmt1,
                              W(L"SELECT '\x30a1' FROM DUAL WHERE 1 = ?"),
                              SQL_NTS));

  ok_stmt(hstmt1, SQLBindParameter(hstmt1, 1, SQL_PARAM_INPUT, SQL_C_LONG,
                                   SQL_INTEGER, 0, 0, &data, 0, NULL));
  data= 0;
  ok_stmt(hstmt1, SQLExecute(hstmt1));

  expect_stmt(hstmt1, SQLFetch(hstmt1), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  data= 1;
  ok_stmt(hstmt1, SQLExecute(hstmt1));

  ok_stmt(hstmt1, SQLFetch(hstmt1));

  is_wstr(my_fetch_wstr(hstmt1, wbuff, 1), L"\x30a1", 1);

  expect_stmt(hstmt1, SQLFetch(hstmt1), SQL_NO_DATA_FOUND);

  /*
    Some driver managers (like iODBC) will always do the character conversion
    themselves.
  */
  if (!using_dm(hdbc))
  {
    ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

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

    is_wstr(my_fetch_wstr(hstmt1, wbuff, 1), L"\x00e3", 1);

    expect_stmt(hstmt1, SQLFetch(hstmt1), SQL_NO_DATA_FOUND);
  }

  ok_stmt(hstmt, SQLFreeStmt(hstmt1, SQL_DROP));

  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


/**
  Test calling SQLPrepareW() on an ANSI connection. Only tested when
  we're not using a driver manager, otherwise the driver manager does
  Unicode to ANSI translations.
*/
DECLARE_TEST(sqlprepare_ansi)
{
  SQLINTEGER data;
  SQLWCHAR wbuff[MAX_ROW_DATA_LEN+1];

  if (using_dm(hdbc))
    skip("not relevant when using driver manager");

  /* Now try SQLPrepareW with an ANSI connection. */
  ok_stmt(hstmt, SQLPrepareW(hstmt,
                             W(L"SELECT '\x00e3' FROM DUAL WHERE 1 = ?"),
                             SQL_NTS));

  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG,
                                  SQL_INTEGER, 0, 0, &data, 0, NULL));
  data= 0;
  ok_stmt(hstmt, SQLExecute(hstmt));

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  data= 1;
  ok_stmt(hstmt, SQLExecute(hstmt));

  ok_stmt(hstmt, SQLFetch(hstmt));

  is_wstr(my_fetch_wstr(hstmt, wbuff, 1), L"\x00e3", 1);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  /* Now try SQLPrepareW with a character that doesn't translate. */
  expect_stmt(hstmt, SQLPrepareW(hstmt,
                                 W(L"SELECT '\x30a1' FROM DUAL WHERE 1 = ?"),
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
  SQLWCHAR wbuff[MAX_ROW_DATA_LEN+1];
  wchar_t wcdata[]= L"S\x00e3o Paolo";

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));
  ok_con(hdbc1, SQLConnectW(hdbc1,
                            WC(mydsn), SQL_NTS,
                            WC(myuid), SQL_NTS,
                            WC(mypwd), SQL_NTS));
  ok_con(hdbc, SQLAllocStmt(hdbc1, &hstmt1));

  ok_stmt(hstmt1, SQLPrepareW(hstmt1, W(L"SELECT ? FROM DUAL"), SQL_NTS));

  ok_stmt(hstmt1, SQLBindParameter(hstmt1, 1, SQL_PARAM_INPUT, SQL_C_CHAR,
                                   SQL_WVARCHAR, 0, 0, data, sizeof(data),
                                   NULL));
  ok_stmt(hstmt1, SQLExecute(hstmt1));

  ok_stmt(hstmt1, SQLFetch(hstmt1));
  is_str(my_fetch_str(hstmt1, buff, 1), data, sizeof(data));

  expect_stmt(hstmt1, SQLFetch(hstmt1), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  /* Do it again so we can try as SQLWCHAR */
  ok_stmt(hstmt1, SQLExecute(hstmt1));

  ok_stmt(hstmt1, SQLFetch(hstmt1));

  is_wstr(my_fetch_wstr(hstmt1, wbuff, 1), wcdata, 9);

  expect_stmt(hstmt1, SQLFetch(hstmt1), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt1, SQL_DROP));

  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


DECLARE_TEST(sqldriverconnect)
{
  HDBC hdbc1;
  HSTMT hstmt1;
  wchar_t conn_in[512];
  wchar_t dummy[256];
  SQLWCHAR conn_out[512];
  SQLSMALLINT conn_out_len;

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));

  *conn_in= L'\0';
  wcscat(conn_in, L"DRIVER=");
  mbstowcs(dummy, (char *)mydriver, sizeof(dummy));
  wcscat(conn_in, dummy);
  wcscat(conn_in, L";UID=");
  mbstowcs(dummy, (char *)myuid, sizeof(dummy));
  wcscat(conn_in, dummy);
  wcscat(conn_in, L";PWD=");
  mbstowcs(dummy, (char *)mypwd, sizeof(dummy));
  wcscat(conn_in, L";DATABASE=");
  mbstowcs(dummy, (char *)mydb, sizeof(dummy));
  wcscat(conn_in, dummy);
  wcscat(conn_in, L";SERVER=");
  mbstowcs(dummy, (char *)myserver, sizeof(dummy));
  wcscat(conn_in, dummy);
  if (mysock != NULL)
  {
    wcscat(conn_in, L";SOCKET=");
    mbstowcs(dummy, (char *)mysock, sizeof(dummy));
    wcscat(conn_in, dummy);
  }

  ok_con(hdbc1, SQLDriverConnectW(hdbc1, NULL, WL(conn_in, wcslen(conn_in)),
                                  wcslen(conn_in), conn_out, sizeof(conn_out),
                                  &conn_out_len, SQL_DRIVER_NOPROMPT));

  ok_con(hdbc, SQLAllocStmt(hdbc1, &hstmt1));

  ok_stmt(hstmt1, SQLExecDirectW(hstmt1, W(L"SELECT 1234"), SQL_NTS));
  ok_stmt(hstmt1, SQLFetch(hstmt1));
  is_num(my_fetch_int(hstmt1, 1), 1234);

  ok_stmt(hstmt, SQLFreeStmt(hstmt1, SQL_DROP));

  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


DECLARE_TEST(sqlnativesql)
{
  HDBC hdbc1;
  SQLWCHAR    out[128];
  wchar_t in[]= L"SELECT * FROM venu";
  SQLINTEGER len;

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));
  ok_con(hdbc1, SQLConnectW(hdbc1,
                            WC(mydsn), SQL_NTS,
                            WC(myuid), SQL_NTS,
                            WC(mypwd), SQL_NTS));

  ok_con(hdbc1, SQLNativeSqlW(hdbc1, W(in), SQL_NTS, out, sizeof(out), &len));
  is_num(len, sizeof(in) / sizeof(wchar_t) - 1);
  is_wstr(sqlwchar_to_wchar_t(out), in, sizeof(in) / sizeof(wchar_t) - 1);

  ok_con(hdbc1, SQLNativeSqlW(hdbc1, W(in), SQL_NTS, out, 8, &len));
  is_num(len, sizeof(in) / sizeof(wchar_t) - 1);
  is_wstr(sqlwchar_to_wchar_t(out), in, 7);
  is(out[7] == 0);

  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


DECLARE_TEST(sqlsetcursorname)
{
  HDBC hdbc1;
  HSTMT hstmt1, hstmt_pos;
  SQLLEN  nRowCount;
  SQLCHAR data[10];

  ok_sql(hstmt, "DROP TABLE IF EXISTS my_demo_cursor");
  ok_sql(hstmt, "CREATE TABLE my_demo_cursor (id INT, name VARCHAR(20))");
  ok_sql(hstmt, "INSERT INTO my_demo_cursor VALUES (0,'MySQL0'),(1,'MySQL1'),"
         "(2,'MySQL2'),(3,'MySQL3'),(4,'MySQL4')");

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));
  ok_con(hdbc1, SQLConnectW(hdbc1, WC(mydsn), SQL_NTS, WC(myuid), SQL_NTS,
                            WC(mypwd), SQL_NTS));

  ok_con(hdbc1, SQLAllocStmt(hdbc1, &hstmt1));

  ok_stmt(hstmt1, SQLSetStmtAttrW(hstmt1, SQL_ATTR_CURSOR_TYPE,
                                  (SQLPOINTER)SQL_CURSOR_DYNAMIC, 0));

  ok_stmt(hstmt1, SQLSetCursorNameW(hstmt1, W(L"a\x00e3b"), SQL_NTS));

  /* Open the resultset of table 'my_demo_cursor' */
  ok_sql(hstmt1, "SELECT * FROM my_demo_cursor");

  /* goto the last row */
  ok_stmt(hstmt1, SQLFetchScroll(hstmt1, SQL_FETCH_LAST, 1L));

  /* create new statement handle */
  ok_con(hdbc1, SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt_pos));

  /* now update the name field to 'updated' using positioned cursor */
  ok_stmt(hstmt_pos,
          SQLExecDirectW(hstmt_pos,
                         W(L"UPDATE my_demo_cursor SET name='updated' "
                           L"WHERE CURRENT OF a\x00e3b"), SQL_NTS));

  ok_stmt(hstmt_pos, SQLRowCount(hstmt_pos, &nRowCount));
  is_num(nRowCount, 1);

  ok_stmt(hstmt_pos, SQLFreeStmt(hstmt_pos, SQL_CLOSE));
  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  /* Now delete 2nd row */
  ok_sql(hstmt1, "SELECT * FROM my_demo_cursor");

  /* goto the second row */
  ok_stmt(hstmt1, SQLFetchScroll(hstmt1, SQL_FETCH_ABSOLUTE, 2L));

  /* now delete the current row */
  ok_stmt(hstmt_pos,
          SQLExecDirectW(hstmt_pos,
                         W(L"DELETE FROM my_demo_cursor "
                           L"WHERE CURRENT OF a\x00e3b"), SQL_NTS));

  ok_stmt(hstmt_pos, SQLRowCount(hstmt_pos, &nRowCount));
  is_num(nRowCount, 1);

  /* free the statement cursor */
  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  /* Free the statement 'hstmt_pos' */
  ok_stmt(hstmt_pos, SQLFreeHandle(SQL_HANDLE_STMT, hstmt_pos));

  /* Now fetch and verify the data */
  ok_sql(hstmt1, "SELECT * FROM my_demo_cursor");

  ok_stmt(hstmt1, SQLFetch(hstmt1));
  is_num(my_fetch_int(hstmt1, 1), 0);
  is_str(my_fetch_str(hstmt1, data, 2), "MySQL0", 6);

  ok_stmt(hstmt1, SQLFetch(hstmt1));
  is_num(my_fetch_int(hstmt1, 1), 2);
  is_str(my_fetch_str(hstmt1, data, 2), "MySQL2", 6);

  ok_stmt(hstmt1, SQLFetch(hstmt1));
  is_num(my_fetch_int(hstmt1, 1), 3);
  is_str(my_fetch_str(hstmt1, data, 2), "MySQL3", 6);

  ok_stmt(hstmt1, SQLFetch(hstmt1));
  is_num(my_fetch_int(hstmt1, 1), 4);
  is_str(my_fetch_str(hstmt1, data, 2), "updated", 7);

  expect_stmt(hstmt1, SQLFetch(hstmt1), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS my_demo_cursor");

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));

  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


DECLARE_TEST(sqlgetcursorname)
{
  SQLRETURN rc;
  HDBC hdbc1;
  SQLHSTMT hstmt1,hstmt2,hstmt3;
  SQLWCHAR curname[50];
  SQLSMALLINT nlen;


  ok_env(henv, SQLAllocConnect(henv, &hdbc1));
  ok_con(hdbc1, SQLConnectW(hdbc1, WC(mydsn), SQL_NTS, WC(myuid), SQL_NTS,
                            WC(mypwd), SQL_NTS));

  ok_con(hdbc1, SQLAllocStmt(hdbc1, &hstmt1));
  ok_con(hdbc1, SQLAllocStmt(hdbc1, &hstmt2));
  ok_con(hdbc1, SQLAllocStmt(hdbc1, &hstmt3));

  rc= SQLGetCursorNameW(hstmt1, curname, 50, &nlen);
  if (SQL_SUCCEEDED(rc))
  {
    is_num(nlen, 8);
    is_wstr(sqlwchar_to_wchar_t(curname), L"SQL_CUR0", 8);

    ok_stmt(hstmt3,  SQLGetCursorNameW(hstmt3, curname, 50, &nlen));

    expect_stmt(hstmt3,  SQLGetCursorNameW(hstmt1, curname, 4, &nlen),
                SQL_SUCCESS_WITH_INFO);
    is_num(nlen, 8);
    is_wstr(sqlwchar_to_wchar_t(curname), L"SQL", 4);

    expect_stmt(hstmt3,  SQLGetCursorNameW(hstmt1, curname, 0, &nlen),
                SQL_SUCCESS_WITH_INFO);
    rc = SQLGetCursorNameW(hstmt1, curname, 0, &nlen);
    mystmt_err(hstmt1,rc == SQL_SUCCESS_WITH_INFO, rc);
    is_num(nlen, 8);

    expect_stmt(hstmt1, SQLGetCursorNameW(hstmt1, curname, 8, &nlen),
                SQL_SUCCESS_WITH_INFO);
    is_num(nlen, 8);
    is_wstr(sqlwchar_to_wchar_t(curname), L"SQL_CUR", 8);

    ok_stmt(hstmt1, SQLGetCursorNameW(hstmt1, curname, 9, &nlen));
    is_num(nlen, 8);
    is_wstr(sqlwchar_to_wchar_t(curname), L"SQL_CUR0", 8);
  }

  ok_stmt(hstmt1,  SQLSetCursorNameW(hstmt1, W(L"venucur123"), 7));

  ok_stmt(hstmt1,  SQLGetCursorNameW(hstmt1, curname, 8, &nlen));
  is_num(nlen, 7);
  is_wstr(sqlwchar_to_wchar_t(curname), L"venucur", 8);

  ok_stmt(hstmt3, SQLFreeStmt(hstmt3, SQL_DROP));
  ok_stmt(hstmt2, SQLFreeStmt(hstmt2, SQL_DROP));
  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));

  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


DECLARE_TEST(sqlcolattribute)
{
  HDBC hdbc1;
  HSTMT hstmt1;
  SQLWCHAR wbuff[MAX_ROW_DATA_LEN+1];
  SQLSMALLINT len;

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));
  ok_con(hdbc1, SQLConnectW(hdbc1, WC(mydsn), SQL_NTS, WC(myuid), SQL_NTS,
                            WC(mypwd), SQL_NTS));

  ok_con(hdbc1, SQLAllocStmt(hdbc1, &hstmt1));

  ok_sql(hstmt1, "DROP TABLE IF EXISTS t_colattrib");
  ok_stmt(hstmt1, SQLExecDirectW(hstmt1,
                                 W(L"CREATE TABLE t_colattrib (a\x00e3g INT)"),
                                 SQL_NTS));

  ok_sql(hstmt1, "SELECT * FROM t_colattrib AS b");

  ok_stmt(hstmt1, SQLColAttributeW(hstmt1, 1, SQL_DESC_NAME,
                                   wbuff, sizeof(wbuff), &len, NULL));
  is_num(len, 3 * sizeof(SQLWCHAR));
  is_wstr(sqlwchar_to_wchar_t(wbuff), L"a\x00e3g", 4);

  expect_stmt(hstmt1, SQLColAttributeW(hstmt1, 1, SQL_DESC_BASE_TABLE_NAME,
                                       wbuff, 5 * sizeof(SQLWCHAR), &len, NULL),
              SQL_SUCCESS_WITH_INFO);
  is_num(len, 11 * sizeof(SQLWCHAR));
  is_wstr(sqlwchar_to_wchar_t(wbuff), L"t_co", 5);

  ok_stmt(hstmt1, SQLColAttributeW(hstmt1, 1, SQL_DESC_TYPE_NAME,
                                   wbuff, sizeof(wbuff), &len, NULL));
  is_num(len, 7 * sizeof(SQLWCHAR));
  is_wstr(sqlwchar_to_wchar_t(wbuff), L"integer", 8);

  ok_sql(hstmt1, "DROP TABLE IF EXISTS t_colattrib");

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));
  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


DECLARE_TEST(sqldescribecol)
{
  HDBC hdbc1;
  HSTMT hstmt1;
  SQLWCHAR wbuff[MAX_ROW_DATA_LEN+1];
  SQLSMALLINT len;

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));
  ok_con(hdbc1, SQLConnectW(hdbc1, WC(mydsn), SQL_NTS, WC(myuid), SQL_NTS,
                            WC(mypwd), SQL_NTS));

  ok_con(hdbc1, SQLAllocStmt(hdbc1, &hstmt1));

  ok_sql(hstmt1, "DROP TABLE IF EXISTS t_desc");
  ok_stmt(hstmt1, SQLExecDirectW(hstmt1,
                                 W(L"CREATE TABLE t_desc (a\x00e3g INT)"),
                                 SQL_NTS));

  ok_sql(hstmt1, "SELECT * FROM t_desc");

  ok_stmt(hstmt1, SQLDescribeColW(hstmt1, 1, wbuff,
                                  sizeof(wbuff) / sizeof(wbuff[0]), &len,
                                  NULL, NULL, NULL, NULL));
  is_num(len, 3);
  is_wstr(sqlwchar_to_wchar_t(wbuff), L"a\x00e3g", 4);

  expect_stmt(hstmt1, SQLDescribeColW(hstmt1, 1, wbuff, 3, &len,
                                      NULL, NULL, NULL, NULL),
              SQL_SUCCESS_WITH_INFO);
  is_num(len, 3);
  is_wstr(sqlwchar_to_wchar_t(wbuff), L"a\x00e3", 3);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  ok_sql(hstmt1, "DROP TABLE IF EXISTS t_desc");

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));
  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


DECLARE_TEST(sqlgetconnectattr)
{
  HDBC hdbc1;
  HSTMT hstmt1;
  SQLWCHAR wbuff[MAX_ROW_DATA_LEN+1];
  SQLINTEGER len;

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));
  ok_con(hdbc1, SQLConnectW(hdbc1, WC(mydsn), SQL_NTS, WC(myuid), SQL_NTS,
                            WC(mypwd), SQL_NTS));

  ok_con(hdbc1, SQLAllocStmt(hdbc1, &hstmt1));

  ok_stmt(hstmt1, SQLGetConnectAttrW(hdbc1, SQL_ATTR_CURRENT_CATALOG, wbuff,
                                     sizeof(wbuff), &len));
  is_num(len, 4 * sizeof(SQLWCHAR));
  is_wstr(sqlwchar_to_wchar_t(wbuff), L"test", 5);

  expect_stmt(hstmt1, SQLGetConnectAttrW(hdbc1, SQL_ATTR_CURRENT_CATALOG,
                                         wbuff, 3 * sizeof(SQLWCHAR), &len),
              SQL_SUCCESS_WITH_INFO);
  is_num(len, 4 * sizeof(SQLWCHAR));
  is_wstr(sqlwchar_to_wchar_t(wbuff), L"te", 3);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));
  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


DECLARE_TEST(sqlgetdiagrec)
{
  SQLWCHAR   sqlstate[6]= {0};
  SQLWCHAR   message[255]= {0};
  SQLINTEGER native_err= 0;
  SQLSMALLINT msglen= 0;
  HDBC hdbc1;
  HSTMT hstmt1;

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));
  ok_con(hdbc1, SQLConnectW(hdbc1, WC(mydsn), SQL_NTS, WC(myuid), SQL_NTS,
                            WC(mypwd), SQL_NTS));

  ok_con(hdbc1, SQLAllocStmt(hdbc1, &hstmt1));


  expect_sql(hstmt1, "DROP TABLE t_odbc3_non_existent_table", SQL_ERROR);

#if UNIXODBC_BUG_FIXED
  /*
   This should report no data found, but unixODBC doesn't even pass this
   down to the driver.
  */
  expect_stmt(hstmt1, SQLGetDiagRecW(SQL_HANDLE_STMT, hstmt1, 2, sqlstate,
                                     &native_err, message, 0, &msglen),
              SQL_NO_DATA_FOUND);
#endif

  ok_stmt(hstmt, SQLGetDiagRecW(SQL_HANDLE_STMT, hstmt1, 1, sqlstate,
                                &native_err, message, 255, &msglen));

  expect_stmt(hstmt1, SQLGetDiagRecW(SQL_HANDLE_STMT, hstmt1, 1, sqlstate,
                                     &native_err, message, 0, &msglen),
              SQL_SUCCESS_WITH_INFO);

  expect_stmt(hstmt1, SQLGetDiagRecW(SQL_HANDLE_STMT, hstmt1, 1, sqlstate,
                                     &native_err, message, 10, &msglen),
              SQL_SUCCESS_WITH_INFO);

  expect_stmt(hstmt1, SQLGetDiagRecW(SQL_HANDLE_STMT, hstmt1, 1, sqlstate,
                                     &native_err, message, -1, &msglen),
              SQL_ERROR);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));
  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


DECLARE_TEST(sqlgetdiagfield)
{
  SQLWCHAR   message[255]= {0};
  SQLSMALLINT len;
  SQLINTEGER data;
  HDBC hdbc1;
  HSTMT hstmt1;

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));
  ok_con(hdbc1, SQLConnectW(hdbc1, WC(mydsn), SQL_NTS, WC(myuid), SQL_NTS,
                            WC(mypwd), SQL_NTS));

  ok_con(hdbc1, SQLAllocStmt(hdbc1, &hstmt1));


  expect_sql(hstmt1, "DROP TABLE t_odbc3_non_existent_table", SQL_ERROR);

  ok_stmt(hstmt, SQLGetDiagFieldW(SQL_HANDLE_STMT, hstmt1, 0,
                                  SQL_DIAG_NUMBER, &data, 0, NULL));
  is_num(data, 1);

  ok_stmt(hstmt, SQLGetDiagFieldW(SQL_HANDLE_STMT, hstmt1, 1,
                                  SQL_DIAG_CLASS_ORIGIN, message,
                                  sizeof(message), &len));
  is_num(len, 8 * sizeof(SQLWCHAR));
  is_wstr(sqlwchar_to_wchar_t(message), L"ISO 9075", 9);

  expect_stmt(hstmt, SQLGetDiagFieldW(SQL_HANDLE_STMT, hstmt1, 1,
                                      SQL_DIAG_SQLSTATE, message,
                                      4 * sizeof(SQLWCHAR), &len),
              SQL_SUCCESS_WITH_INFO);
  is_num(len, 5 * sizeof(SQLWCHAR));
  is_wstr(sqlwchar_to_wchar_t(message), L"42S", 4);

  ok_stmt(hstmt, SQLGetDiagFieldW(SQL_HANDLE_STMT, hstmt1, 1,
                                  SQL_DIAG_SUBCLASS_ORIGIN, message,
                                  sizeof(message), &len));
  is_num(len, 8 * sizeof(SQLWCHAR));
  is_wstr(sqlwchar_to_wchar_t(message), L"ODBC 3.0", 9);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));
  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


DECLARE_TEST(sqlcolumns)
{
  HDBC hdbc1;
  HSTMT hstmt1;
  SQLWCHAR wbuff[MAX_ROW_DATA_LEN+1];

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));
  ok_con(hdbc1, SQLConnectW(hdbc1, WC(mydsn), SQL_NTS, WC(myuid), SQL_NTS,
                            WC(mypwd), SQL_NTS));

  ok_con(hdbc1, SQLAllocStmt(hdbc1, &hstmt1));

  ok_sql(hstmt1, "DROP TABLE IF EXISTS t_columns");
  ok_stmt(hstmt1, SQLExecDirectW(hstmt1,
                                 W(L"CREATE TABLE t_columns (a\x00e3g INT)"),
                                 SQL_NTS));

  ok_stmt(hstmt1, SQLColumnsW(hstmt1, NULL, 0, NULL, 0,
                              W(L"t_columns"), SQL_NTS,
                              W(L"a\x00e3g"), SQL_NTS));

  ok_stmt(hstmt1, SQLFetch(hstmt1));

  is_wstr(my_fetch_wstr(hstmt1, wbuff, 4), L"a\x00e3g", 4);

  expect_stmt(hstmt1, SQLFetch(hstmt1), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  ok_sql(hstmt1, "DROP TABLE IF EXISTS t_columns");

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));
  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


DECLARE_TEST(sqltables)
{
  HDBC hdbc1;
  HSTMT hstmt1;
  SQLWCHAR wbuff[MAX_ROW_DATA_LEN+1];

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));
  ok_con(hdbc1, SQLConnectW(hdbc1, WC(mydsn), SQL_NTS, WC(myuid), SQL_NTS,
                            WC(mypwd), SQL_NTS));

  ok_con(hdbc1, SQLAllocStmt(hdbc1, &hstmt1));

  ok_stmt(hstmt1, SQLExecDirectW(hstmt1,
                                 W(L"DROP TABLE IF EXISTS t_a\x00e3g"),
                                 SQL_NTS));
  ok_stmt(hstmt1, SQLExecDirectW(hstmt1,
                                 W(L"CREATE TABLE t_a\x00e3g (a INT)"),
                                 SQL_NTS));

  ok_stmt(hstmt1, SQLTablesW(hstmt1, NULL, 0, NULL, 0,
                             W(L"t_a\x00e3g"), SQL_NTS,
                             NULL, 0));

  ok_stmt(hstmt1, SQLFetch(hstmt1));

  is_wstr(my_fetch_wstr(hstmt1, wbuff, 3), L"t_a\x00e3g", 6);

  expect_stmt(hstmt1, SQLFetch(hstmt1), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  ok_stmt(hstmt1, SQLExecDirectW(hstmt1,
                                 W(L"DROP TABLE IF EXISTS t_a\x00e3g"),
                                 SQL_NTS));

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));
  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


DECLARE_TEST(sqlspecialcolumns)
{
  HDBC hdbc1;
  HSTMT hstmt1;
  SQLWCHAR wbuff[MAX_ROW_DATA_LEN+1];

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));
  ok_con(hdbc1, SQLConnectW(hdbc1, WC(mydsn), SQL_NTS, WC(myuid), SQL_NTS,
                            WC(mypwd), SQL_NTS));

  ok_con(hdbc1, SQLAllocStmt(hdbc1, &hstmt1));

  ok_sql(hstmt1, "DROP TABLE IF EXISTS t_spec");
  ok_stmt(hstmt1,
          SQLExecDirectW(hstmt1,
                         W(L"CREATE TABLE t_spec (a\x00e3g INT PRIMARY KEY)"),
                         SQL_NTS));

  ok_stmt(hstmt1, SQLSpecialColumnsW(hstmt1, SQL_BEST_ROWID, NULL, 0, NULL, 0,
                             W(L"t_spec"), SQL_NTS, SQL_SCOPE_SESSION,
                             SQL_NULLABLE));

  ok_stmt(hstmt1, SQLFetch(hstmt1));

  is_wstr(my_fetch_wstr(hstmt1, wbuff, 2), L"a\x00e3g", 4);

  expect_stmt(hstmt1, SQLFetch(hstmt1), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  ok_sql(hstmt1, "DROP TABLE IF EXISTS t_spec");

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));
  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


DECLARE_TEST(sqlforeignkeys)
{
  HDBC hdbc1;
  HSTMT hstmt1;
  SQLWCHAR wbuff[MAX_ROW_DATA_LEN+1];

  /** @todo re-enable this test when I_S based SQLForeignKeys is done. */
  if (mysql_min_version(hdbc, "5.1", 3))
    skip("can't test foreign keys with 5.1 or later yet");

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));
  ok_con(hdbc1, SQLConnectW(hdbc1, WC(mydsn), SQL_NTS, WC(myuid), SQL_NTS,
                            WC(mypwd), SQL_NTS));

  ok_con(hdbc1, SQLAllocStmt(hdbc1, &hstmt1));

  ok_stmt(hstmt1,
          SQLExecDirectW(hstmt1,
                         W(L"DROP TABLE IF EXISTS t_fk_\x00e5, t_fk_\x00e3"),
                         SQL_NTS));
  ok_stmt(hstmt1,
          SQLExecDirectW(hstmt1,
                         W(L"CREATE TABLE t_fk_\x00e3 (a INT PRIMARY KEY) "
                           L"ENGINE=InnoDB"), SQL_NTS));
  ok_stmt(hstmt1,
          SQLExecDirectW(hstmt1,
                         W(L"CREATE TABLE t_fk_\x00e5 (b INT, parent_id INT,"
                         L"                       FOREIGN KEY (parent_id)"
                         L"                        REFERENCES"
                         L"                          t_fk_\x00e3(a)"
                         L"                        ON DELETE SET NULL)"
                         L" ENGINE=InnoDB"), SQL_NTS));

  ok_stmt(hstmt1, SQLForeignKeysW(hstmt1, NULL, 0, NULL, 0, NULL, 0, NULL, 0,
                                  NULL, 0, W(L"t_fk_\x00e5"), SQL_NTS));

  ok_stmt(hstmt1, SQLFetch(hstmt1));
  is_wstr(my_fetch_wstr(hstmt1, wbuff, 3), L"t_fk_\x00e3", 7);
  is_wstr(my_fetch_wstr(hstmt1, wbuff, 4), L"a", 2);
  is_wstr(my_fetch_wstr(hstmt1, wbuff, 7), L"t_fk_\x00e5", 7);
  is_wstr(my_fetch_wstr(hstmt1, wbuff, 8), L"parent_id", 10);

  expect_stmt(hstmt1, SQLFetch(hstmt1), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  ok_stmt(hstmt1,
          SQLExecDirectW(hstmt1,
                         W(L"DROP TABLE IF EXISTS t_fk_\x00e5, t_fk_\x00e3"),
                         SQL_NTS));

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));
  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


DECLARE_TEST(sqlprimarykeys)
{
  HDBC hdbc1;
  HSTMT hstmt1;
  SQLWCHAR wbuff[MAX_ROW_DATA_LEN+1];

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));
  ok_con(hdbc1, SQLConnectW(hdbc1, WC(mydsn), SQL_NTS, WC(myuid), SQL_NTS,
                            WC(mypwd), SQL_NTS));

  ok_con(hdbc1, SQLAllocStmt(hdbc1, &hstmt1));

  ok_stmt(hstmt1, SQLExecDirectW(hstmt1, W(L"DROP TABLE IF EXISTS t_a\x00e3g"),
                                 SQL_NTS));
  ok_stmt(hstmt1,
          SQLExecDirectW(hstmt1,
                         W(L"CREATE TABLE t_a\x00e3g (a INT PRIMARY KEY)"),
                         SQL_NTS));

  ok_stmt(hstmt1, SQLPrimaryKeysW(hstmt1, NULL, 0, NULL, 0,
                                  W(L"t_a\x00e3g"), SQL_NTS));

  ok_stmt(hstmt1, SQLFetch(hstmt1));

  is_wstr(my_fetch_wstr(hstmt1, wbuff, 3), L"t_a\x00e3g", 6);

  expect_stmt(hstmt1, SQLFetch(hstmt1), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  ok_stmt(hstmt1, SQLExecDirectW(hstmt1, W(L"DROP TABLE IF EXISTS t_a\x00e3g"),
                                 SQL_NTS));

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));
  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


DECLARE_TEST(sqlstatistics)
{
  HDBC hdbc1;
  HSTMT hstmt1;
  SQLWCHAR wbuff[MAX_ROW_DATA_LEN+1];

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));
  ok_con(hdbc1, SQLConnectW(hdbc1, WC(mydsn), SQL_NTS, WC(myuid), SQL_NTS,
                            WC(mypwd), SQL_NTS));

  ok_con(hdbc1, SQLAllocStmt(hdbc1, &hstmt1));

  ok_stmt(hstmt1, SQLExecDirectW(hstmt1,
                                 W(L"DROP TABLE IF EXISTS t_a\x00e3g"),
                                 SQL_NTS));
  ok_stmt(hstmt1,
          SQLExecDirectW(hstmt1,
                         W(L"CREATE TABLE t_a\x00e3g (a INT PRIMARY KEY)"),
                         SQL_NTS));

  ok_stmt(hstmt1, SQLStatisticsW(hstmt1, NULL, 0, NULL, 0,
                                 W(L"t_a\x00e3g"), SQL_NTS,
                                 SQL_INDEX_UNIQUE, SQL_QUICK));

  ok_stmt(hstmt1, SQLFetch(hstmt1));

  is_wstr(my_fetch_wstr(hstmt1, wbuff, 3), L"t_a\x00e3g", 6);

  expect_stmt(hstmt1, SQLFetch(hstmt1), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  ok_stmt(hstmt1, SQLExecDirectW(hstmt1,
                                 W(L"DROP TABLE IF EXISTS t_a\x00e3g"),
                                 SQL_NTS));

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));
  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


/**
 Bug #32161: SQLDescribeColW returns UTF-8 column as SQL_VARCHAR instead of
 SQL_WVARCHAR
*/
DECLARE_TEST(t_bug32161)
{
  HDBC hdbc1;
  HSTMT hstmt1;
  SQLWCHAR wbuff[MAX_ROW_DATA_LEN+1];
  SQLSMALLINT nlen;
  SQLSMALLINT ctype;

  ok_env(henv, SQLAllocConnect(henv, &hdbc1));
  ok_con(hdbc1, SQLConnectW(hdbc1, WC(mydsn), SQL_NTS, WC(myuid), SQL_NTS,
                            WC(mypwd), SQL_NTS));

  ok_con(hdbc1, SQLAllocStmt(hdbc1, &hstmt1));

  ok_sql(hstmt1, "DROP TABLE IF EXISTS t_bug32161");
  ok_sql(hstmt1, "CREATE TABLE t_bug32161 ("
                 "col1 varchar(32),"
                 "col2 char(32),"
                 "col3 tinytext,"
                 "col4 mediumtext,"
                 "col5 text,"
                 "col6 longtext"
                 ") DEFAULT CHARSET=utf8");

  /* Greek word PSARO - FISH */
  ok_stmt(hstmt1,
          SQLExecDirectW(hstmt1,
                         W(L"INSERT INTO t_bug32161 VALUES ("
                         L"\"\x03A8\x0391\x03A1\x039F 1\","
                         L"\"\x03A8\x0391\x03A1\x039F 2\","
                         L"\"\x03A8\x0391\x03A1\x039F 3\","
                         L"\"\x03A8\x0391\x03A1\x039F 4\","
                         L"\"\x03A8\x0391\x03A1\x039F 5\","
                         L"\"\x03A8\x0391\x03A1\x039F 6\")"),
                         SQL_NTS));

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));
  ok_sql(hstmt1, "SELECT * FROM t_bug32161");
  ok_stmt(hstmt1, SQLFetch(hstmt1));

  is_wstr(my_fetch_wstr(hstmt1, wbuff, 1), L"\x03A8\x0391\x03A1\x039F 1", 4);
  ok_stmt(hstmt1, SQLDescribeColW(hstmt1, 1, wbuff, MAX_ROW_DATA_LEN, &nlen,
                                  &ctype, NULL, NULL, NULL));
  is_num(ctype, SQL_WVARCHAR);

  is_wstr(my_fetch_wstr(hstmt1, wbuff, 2), L"\x03A8\x0391\x03A1\x039F 2", 4);
  ok_stmt(hstmt1, SQLDescribeColW(hstmt1, 2, wbuff, MAX_ROW_DATA_LEN, &nlen,
                                  &ctype, NULL, NULL, NULL));
  is_num(ctype, SQL_WCHAR);

  /* All further calls of SQLDescribeColW should return SQL_WLONGVARCHAR */
  is_wstr(my_fetch_wstr(hstmt1, wbuff, 3), L"\x03A8\x0391\x03A1\x039F 3", 4);
  ok_stmt(hstmt1, SQLDescribeColW(hstmt1, 3, wbuff, MAX_ROW_DATA_LEN, &nlen,
                                  &ctype, NULL, NULL, NULL));
  is_num(ctype, SQL_WLONGVARCHAR);

  is_wstr(my_fetch_wstr(hstmt1, wbuff, 4), L"\x03A8\x0391\x03A1\x039F 4", 4);
  ok_stmt(hstmt1, SQLDescribeColW(hstmt1, 4, wbuff, MAX_ROW_DATA_LEN, &nlen,
                                  &ctype, NULL, NULL, NULL));
  is_num(ctype, SQL_WLONGVARCHAR);

  is_wstr(my_fetch_wstr(hstmt1, wbuff, 5), L"\x03A8\x0391\x03A1\x039F 5", 4);
  ok_stmt(hstmt1, SQLDescribeColW(hstmt1, 5, wbuff, MAX_ROW_DATA_LEN, &nlen,
                                  &ctype, NULL, NULL, NULL));
  is_num(ctype, SQL_WLONGVARCHAR);

  is_wstr(my_fetch_wstr(hstmt1, wbuff, 6), L"\x03A8\x0391\x03A1\x039F 6", 4);
  ok_stmt(hstmt1, SQLDescribeColW(hstmt1, 6, wbuff, MAX_ROW_DATA_LEN, &nlen,
                                  &ctype, NULL, NULL, NULL));
  is_num(ctype, SQL_WLONGVARCHAR);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));
  ok_sql(hstmt1, "DROP TABLE IF EXISTS t_bug32161");

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));
  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


/*
  Bug#34672 - Unable to insert surrogate pairs into or fetch surrogate pairs
              from unicode column
*/
DECLARE_TEST(t_bug34672)
{
  SQLWCHAR chars[3];
  SQLINTEGER inchars, i;
  SQLWCHAR result[3];
  SQLLEN reslen;

  if (sizeof(SQLWCHAR) == 2)
  {
    chars[0]= 0xd802;
    chars[1]= 0xdc60;
    inchars= 2;
  }
  else
  {
    chars[0]= (SQLWCHAR) 0x10860;
    inchars= 1;
  }

  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR,
                                  SQL_WCHAR, 0, 0, chars,
                                  inchars * sizeof(SQLWCHAR), NULL));

  if (mysql_min_version(hdbc, "6.0.4", 5))
  {
    ok_stmt(hstmt, SQLExecDirect(hstmt, (SQLCHAR *) "select ?", SQL_NTS));
    ok_stmt(hstmt, SQLFetch(hstmt));
    ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_WCHAR, result,
                              sizeof(result), &reslen));
    is_num(result[2], 0);
    for (i= 0; i < inchars; ++i)
      is_num(result[i], chars[i]);
    is_num(reslen, inchars * sizeof(SQLWCHAR));
  }
  else
  {
    expect_stmt(hstmt, SQLExecDirect(hstmt, (SQLCHAR *) "select ?", SQL_NTS),
                SQL_ERROR);
    return check_sqlstate(hstmt, "HY000");
  }

  return OK;
}


BEGIN_TESTS
  ADD_TEST(sqlconnect)
  ADD_TEST(sqlprepare)
  ADD_TEST(sqlprepare_ansi)
  ADD_TEST(sqlchar)
  ADD_TEST(sqldriverconnect)
  ADD_TEST(sqlnativesql)
  ADD_TEST(sqlsetcursorname)
  ADD_TEST(sqlgetcursorname)
  ADD_TEST(sqlcolattribute)
  ADD_TEST(sqldescribecol)
  ADD_TEST(sqlgetconnectattr)
  ADD_TEST(sqlgetdiagrec)
  ADD_TEST(sqlgetdiagfield)
  ADD_TEST(sqlcolumns)
  ADD_TEST(sqltables)
  ADD_TEST(sqlspecialcolumns)
  ADD_TEST(sqlforeignkeys)
  ADD_TEST(sqlprimarykeys)
  ADD_TEST(sqlstatistics)
  ADD_TEST(t_bug32161)
  ADD_TEST(t_bug34672)
END_TESTS


RUN_TESTS


