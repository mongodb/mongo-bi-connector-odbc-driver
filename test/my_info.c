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


DECLARE_TEST(sqlgetinfo)
{
  SQLCHAR   rgbValue[100];
  SQLSMALLINT pcbInfo;

  ok_con(hdbc, SQLGetInfo(hdbc, SQL_DRIVER_ODBC_VER, rgbValue,
                          sizeof(rgbValue), &pcbInfo));

  is_num(pcbInfo, 5);
  is_str(rgbValue, "03.51", 5);

  return OK;
}


DECLARE_TEST(t_stmt_attr_status)
{
  SQLUSMALLINT rowStatusPtr[3];
  SQLULEN      rowsFetchedPtr;

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
  is_str(catalog, "test", 4);

  ok_con(hdbc, SQLGetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, catalog,
                                 sizeof(catalog), &len));
  is_num(len, 4);
  is_str(catalog, "test", 4);

  return OK;
}


/**
  Bug #28657: ODBC Connector returns FALSE on SQLGetTypeInfo with DATETIME (wxWindows latest)
*/
DECLARE_TEST(t_bug28657)
{
#ifdef WIN32
  /*
   The Microsoft Windows ODBC driver manager automatically maps a request
   for SQL_DATETIME to SQL_TYPE_DATE, which means our little workaround to
   get all of the SQL_DATETIME types at once does not work on there.
  */
  skip("test doesn't work with Microsoft Windows ODBC driver manager");
#else
  ok_stmt(hstmt, SQLGetTypeInfo(hstmt, SQL_DATETIME));

  is(myresult(hstmt) > 1);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  return OK;
#endif
}

DECLARE_TEST(t_bug14639)
{
  SQLINTEGER connection_id;
  SQLUINTEGER is_dead;
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

  /* Check that connection is alive */
  ok_con(hdbc2, SQLGetConnectAttr(hdbc2, SQL_ATTR_CONNECTION_DEAD, &is_dead,
                                 sizeof(is_dead), 0));
  is(is_dead == SQL_CD_FALSE)

  /* From another connection, kill the connection created above */
  sprintf(buf, "KILL %d", connection_id);
  ok_stmt(hstmt, SQLExecDirect(hstmt, (SQLCHAR *)buf, SQL_NTS));

  /* Now check that the connection killed returns the right state */
  ok_con(hdbc, SQLGetConnectAttr(hdbc2, SQL_ATTR_CONNECTION_DEAD, &is_dead,
                                 sizeof(is_dead), 0));
  is(is_dead == SQL_CD_TRUE)

  return OK;
}


/**
 Bug #31055: Uninitiated memory returned by SQLGetFunctions() with
 SQL_API_ODBC3_ALL_FUNCTION
*/
DECLARE_TEST(t_bug31055)
{
  SQLUSMALLINT funcs[SQL_API_ODBC3_ALL_FUNCTIONS_SIZE];

  /*
     The DM will presumably return true for all functions that it
     can satisfy in place of the driver. This test will only work
     when linked directly to the driver.
  */
  if (using_dm(hdbc))
    return OK;

  memset(funcs, 0xff, sizeof(SQLUSMALLINT) * SQL_API_ODBC3_ALL_FUNCTIONS_SIZE);

  ok_con(hdbc, SQLGetFunctions(hdbc, SQL_API_ODBC3_ALL_FUNCTIONS, funcs));

  is(!SQL_FUNC_EXISTS(funcs, SQL_API_SQLALLOCHANDLESTD));

  return OK;
}


/*
   Bug 3780, reading or setting ADODB.Connection.DefaultDatabase 
   is not supported
*/
DECLARE_TEST(t_bug3780)
{
  HDBC hdbc1;
  HSTMT hstmt1;
  SQLCHAR   conn[256], conn_out[256];
  SQLSMALLINT conn_out_len;
  SQLCHAR   rgbValue[MAX_NAME_LEN];
  SQLSMALLINT pcbInfo;
  SQLINTEGER attrlen;

  /* The connection string must not include DATABASE. */
  sprintf((char *)conn, "DRIVER=%s;SERVER=%s;" \
                        "UID=%s;PASSWORD=%s", mydriver, myserver, myuid, mypwd);
  if (mysock != NULL)
  {
    strcat((char *)conn, ";SOCKET=");
    strcat((char *)conn, (char *)mysock);
  }
  if (myport)
  {
    char pbuff[20];
    sprintf(pbuff, ";PORT=%d", myport);
    strcat((char *)conn, pbuff);
  }

  ok_env(henv, SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1));

  ok_con(hdbc1, SQLDriverConnect(hdbc1, NULL, conn, sizeof(conn), conn_out,
                                 sizeof(conn_out), &conn_out_len,
                                 SQL_DRIVER_NOPROMPT));
  ok_con(hdbc1, SQLAllocStmt(hdbc1, &hstmt1));

  ok_con(hdbc1, SQLGetInfo(hdbc1, SQL_DATABASE_NAME, rgbValue, 
                           MAX_NAME_LEN, &pcbInfo));

  is_num(pcbInfo, 4);
  is_str(rgbValue, "null", pcbInfo);

  ok_con(hdbc1, SQLGetConnectAttr(hdbc1, SQL_ATTR_CURRENT_CATALOG,
                                  rgbValue, MAX_NAME_LEN, &attrlen));

  is_num(attrlen, 4);
  is_str(rgbValue, "null", attrlen);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));
  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeHandle(SQL_HANDLE_DBC, hdbc1));

  return OK;
}


/*
  Bug#16653
  MyODBC 3 / truncated UID when performing Data Import in MS Excel
*/
DECLARE_TEST(t_bug16653)
{
  SQLHANDLE hdbc1;
  SQLCHAR buf[50];

  /*
    Driver managers handle SQLGetConnectAttr before connection in
    inconsistent ways.
  */
  if (using_dm(hdbc))
    return OK;

  ok_env(henv, SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1));

  /* this would cause a crash if we arent connected */
  ok_con(hdbc1, SQLGetConnectAttr(hdbc1, SQL_ATTR_CURRENT_CATALOG,
                                  buf, 50, NULL));
  is_str(buf, "null", 1);

  ok_con(hdbc1, SQLFreeHandle(SQL_HANDLE_DBC, hdbc1));

  return OK;
}


BEGIN_TESTS
  ADD_TEST(sqlgetinfo)
  ADD_TEST(t_gettypeinfo)
  ADD_TEST(t_stmt_attr_status)
  ADD_TEST(t_msdev_bug)
  ADD_TEST(t_bug28657)
  ADD_TEST(t_bug14639)
  ADD_TEST(t_bug31055)
  ADD_TEST(t_bug3780)
  ADD_TEST(t_bug16653)
END_TESTS


RUN_TESTS
