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


/*
  Bug #17640929/70642 "Bad memory access when get out params"
  If IN param went last in the list of SP parameters containing any (IN)OUT parameter,
  the driver would access not allocated memory.
  In fact the testcase doesn't reliably hit the problem
*/
DECLARE_TEST(t_bug70642)
{
  SQLSMALLINT i;
  SQLINTEGER  par[]= {10, 20, 1, 0, 1};
  SQLSMALLINT type[]= { SQL_PARAM_OUTPUT, SQL_PARAM_INPUT, SQL_PARAM_INPUT, SQL_PARAM_INPUT,
                        SQL_PARAM_INPUT};

  ok_sql(hstmt, "DROP PROCEDURE IF EXISTS t_bug70642");
  ok_sql(hstmt, "CREATE PROCEDURE t_bug70642("
                "  OUT p_out INT, "
                "  IN p_in INT, IN d1 INT, IN d2 INT, IN d3 INT) "
                "BEGIN "
                "  SET p_in = p_in*10, p_out = p_in*10; "
                "END");

  for (i=0; i < sizeof(par)/sizeof(SQLINTEGER); ++i)
  {
    ok_stmt(hstmt, SQLBindParameter(hstmt, i+1, type[i], SQL_C_LONG, SQL_INTEGER, 0,
      0, &par[i], 0, NULL));
  }

  for (i=0; i < 10; ++i)
  {
    ok_sql(hstmt, "CALL t_bug70642(?, ?, ?, ?, ?)");

    ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  }

  ok_sql(hstmt, "DROP PROCEDURE IF EXISTS t_bug70642");

  return OK;
}


/**
  Bug#31067: SEGMENTATION FAULT IN SQLCOLUMNS IF COLUMN/TABLE NAME IS INVALID
  If column name length is larger then 129 and table name larger then 256 result in 
  segementation fault.
*/
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
  is(strstr((char *)message, "Invalid string or buffer length"));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE t_bug17358838");

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


/**
  Bug #17857204 SQLFETCH() CRASHING WHEN EXECUTE USING 
  UNIXODBC 2.3.2 VERSION
*/
DECLARE_TEST(t_bug17857204)
{
  int i;
  char TmpBuff[256] = {0};
  SQLSMALLINT col_count= 0;
  SQLUINTEGER uintval= 0;
  SQLLEN len= 0;

  ok_sql(hstmt, "DROP TABLE IF EXISTS bug17857204");
  ok_sql(hstmt, "CREATE TABLE bug17857204 (id int unsigned,c char(10))");
  ok_sql(hstmt, "CREATE INDEX i ON bug17857204 (id ,c)");

  for (i= 0; i < 10; i++)
  {
    sprintf(TmpBuff,"INSERT INTO bug17857204 VALUES (%d,'%d')", i, i);
    ok_stmt(hstmt, SQLExecDirect(hstmt, (SQLCHAR*)TmpBuff, SQL_NTS));
  }
  ok_stmt(hstmt, SQLPrepare(hstmt, (SQLCHAR*)"EXPLAIN DELETE a1,a2 "\
                                             "FROM bug17857204 AS a1 "\
                                             "INNER JOIN bug17857204 AS a2 "\
                                             "WHERE a1.id=a2.id and a2.id>=?",
                                             SQL_NTS));
  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_ULONG, 
          SQL_NUMERIC, 4, 0, &uintval, 0,  &len));

  ok_stmt(hstmt, SQLExecute(hstmt));
  ok_stmt(hstmt, SQLNumResultCols(hstmt,&col_count));
  
  while (1)
  {
    SQLRETURN rc= 0;
    memset(TmpBuff,0,256);
    rc= SQLFetch(hstmt);
    if (rc != SQL_SUCCESS)
    {
      break;
    }
    
    for (i= 1; i <= col_count; i++)
    {
      ok_stmt(hstmt, SQLGetData(hstmt, i, SQL_C_CHAR, TmpBuff, 
                                sizeof(TmpBuff), &len));
    }
  }

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE bug17857204");

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


BEGIN_TESTS
  ADD_TEST(t_bug69950)
  ADD_TEST(t_bug70642)
  ADD_TEST(t_bug17358838)
  ADD_TEST(t_bug17587913)
  ADD_TEST(t_bug17857204)
  ADD_TEST(t_bug17854697)
END_TESTS

/*myoption &= ~(1 << 30);
RUN_TESTS_ONCE
myoption |= (1 << 30);
testname_suffix= "_no_i_s";*/
RUN_TESTS
