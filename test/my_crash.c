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
#include "../VersionInfo.h"

/*
  Bug #69950 Visual Studio 2010 crashes when reading rows from any 
  table in Server Explorer
*/
DECLARE_TEST(t_bug69950)
{
  /* Make sure such table does not exist! */
  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug69950");

  /* Create an EMPTY fake result set */
  ok_stmt(hstmt, SQLTables(hstmt, mydb, SQL_NTS, NULL, 0, 
                           "t_bug69950", SQL_NTS, "TABLE", SQL_NTS));

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);
  expect_stmt(hstmt, SQLMoreResults(hstmt), SQL_NO_DATA_FOUND);

  /* CRASH! */
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  return OK;
}


/**
  Bug 17587913 Connect crash if the catalog name given to SQLSetConnectAttr 
  IS INVALID
*/
DECLARE_TEST(t_bug17587913)
{
  SQLHDBC hdbc1;
  SQLCHAR str[1024]={0};
  SQLLEN len= 0;
  SQLCHAR *DatabaseName = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  /* 
    We are not going to use alloc_basic_handles() for a special purpose:
    SQLSetConnectAttr() is to be called before the connection is made
  */  
  ok_env(henv, SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1));

  /* getting error here */
  get_connection(&hdbc1, NULL, NULL, NULL, DatabaseName, NULL);

  ok_con(hdbc1, SQLSetConnectAttr(hdbc1, SQL_ATTR_CURRENT_CATALOG,
                                  DatabaseName, strlen(DatabaseName)));

  /* Expecting error here */
  SQLGetConnectAttr(hdbc1, SQL_ATTR_CURRENT_CATALOG, str, 100, &len);

  /* The driver crashes here on getting connected */
  ok_con(hdbc1, SQLConnect(hdbc1, mydsn, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS));  
  ok_con(hdbc1, SQLGetConnectAttr(hdbc1, SQL_ATTR_CURRENT_CATALOG, str, 100, &len));

  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


DECLARE_TEST(t_bug17358838)
{
  SQLCHAR    colName[512];
  SQLCHAR message[SQL_MAX_MESSAGE_LENGTH + 1];
  SQLCHAR sqlstate[SQL_SQLSTATE_SIZE + 1];
  SQLINTEGER error;
  SQLSMALLINT len;

  memset(colName, '\0', 512);
  memset(colName, 'a', 500);

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug17358838");
  ok_sql(hstmt, "CREATE TABLE t_bug17358838 (a INT)");

  expect_stmt(hstmt, SQLColumns(hstmt, mydb, SQL_NTS, NULL, SQL_NTS,
    (SQLCHAR *)"t_bug17358838", SQL_NTS, colName, SQL_NTS), SQL_ERROR);

  ok_stmt(hstmt, SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate, &error,
                               message, sizeof(message), &len));

  is_str(sqlstate, "HY090", 5);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE t_bug17358838");

  return OK;
}


/**
  Bug #17854697 SEGMENTATION FAULT IN SQLSPECIALCOLUMNS IF TABLE NAME IS 
  INVALID
*/
DECLARE_TEST(t_bug17854697)
{
  SQLCHAR *any_name = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  SQLCHAR buf[1024]= {0};

  int len= strlen(any_name);

  /* lets check all catalog functions */
  expect_stmt(hstmt, SQLColumnPrivileges(hstmt, any_name, SQL_NTS, NULL, 0,
                                         any_name, SQL_NTS, any_name,
                                         SQL_NTS), SQL_ERROR);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  expect_stmt(hstmt, SQLColumns(hstmt, any_name, SQL_NTS, NULL, 0,
                                any_name, SQL_NTS, any_name,
                                SQL_NTS), SQL_ERROR);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  expect_stmt(hstmt, SQLForeignKeys(hstmt, any_name, SQL_NTS, NULL, 0,
                                any_name, SQL_NTS, any_name, SQL_NTS,
                                any_name, SQL_NTS, any_name, SQL_NTS), 
                     SQL_ERROR);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  expect_stmt(hstmt, SQLPrimaryKeys(hstmt, any_name, SQL_NTS, NULL, 0,
                                    any_name, SQL_NTS), SQL_ERROR);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  expect_stmt(hstmt, SQLProcedureColumns(hstmt, any_name, SQL_NTS, NULL, 0,
                                any_name, SQL_NTS, any_name, SQL_NTS), 
                     SQL_ERROR);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  expect_stmt(hstmt, SQLProcedures(hstmt, any_name, SQL_NTS, NULL, 0,
                                any_name, SQL_NTS), SQL_ERROR);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  expect_stmt(hstmt, SQLSpecialColumns(hstmt, SQL_BEST_ROWID, any_name, SQL_NTS, 
                                NULL, 0, any_name, SQL_NTS, SQL_SCOPE_SESSION, 
                                SQL_NULLABLE), SQL_ERROR);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  expect_stmt(hstmt, SQLStatistics(hstmt, any_name, SQL_NTS, NULL, 0,
                                any_name, SQL_NTS, SQL_INDEX_ALL, SQL_QUICK),
                     SQL_ERROR);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  expect_stmt(hstmt, SQLTablePrivileges(hstmt, any_name, SQL_NTS, NULL, 0,
                                any_name, SQL_NTS), SQL_ERROR);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  expect_stmt(hstmt, SQLTables(hstmt, any_name, SQL_NTS, NULL, 0,
                                any_name, SQL_NTS, any_name, SQL_NTS),
                     SQL_ERROR);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  return OK;
}


/* Bug #17999659 CRASH IN ODBC DRIVER WITH CHARSET=WRONGCHARSET  */
DECLARE_TEST(t_bug17999659)
{
  SQLHDBC hdbc1;

  ok_env(henv, SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1));

  /* getting error here */
  expect_dbc(hdbc1, get_connection(&hdbc1, NULL, NULL, NULL, NULL, 
             "CHARSET=wrongcharset"), SQL_ERROR);

  ok_con(hdbc1, SQLFreeConnect(hdbc1));
  return OK;
}

/*
  Bug #17966018 DRIVER AND MYODBC-INSTALLER CRASH WITH LONG OPTION 
  NAMES (>100) AND VALUES (>255)
*/
DECLARE_TEST(t_bug17966018)
{
  char opt_buff[2048];
  int result_connect;

  DECLARE_BASIC_HANDLES(henv1, hdbc1, hstmt1);

  /* This makes a really long SETUP=000000...0;OPTION_0000000000...0=1 */
  sprintf(opt_buff, "OPTION_%0*d=1", 2000, 0);

  result_connect= alloc_basic_handles_with_opt(&henv1, &hdbc1, &hstmt1, NULL, 
                                               NULL, NULL, NULL, opt_buff);
  is_num(result_connect, FAIL);

  free_basic_handles(&henv1, &hdbc1, &hstmt1);
  return OK;
}


BEGIN_TESTS
  ADD_TEST(t_bug69950)
  ADD_TEST(t_bug17587913)
  ADD_TEST(t_bug17358838)
  ADD_TEST(t_bug17854697)
  ADD_TEST(t_bug17999659)
  ADD_TEST(t_bug17966018)
END_TESTS


RUN_TESTS
