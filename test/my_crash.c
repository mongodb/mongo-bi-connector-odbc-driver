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
  SQLCHAR    buff[512];
  SQLCHAR    colName[512];
  SQLSMALLINT maxColLen;
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


BEGIN_TESTS
  ADD_TEST(t_bug69950)
  ADD_TEST(t_bug70642)
  ADD_TEST(t_bug17358838)
END_TESTS

/*myoption &= ~(1 << 30);
RUN_TESTS_ONCE
myoption |= (1 << 30);
testname_suffix= "_no_i_s";*/
RUN_TESTS
