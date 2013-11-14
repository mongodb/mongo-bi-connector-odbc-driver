/*
  Copyright (c) 2013, Oracle and/or its affiliates. All rights reserved.

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


/* Since atm it does not look like we can make a reliable test for automated testing, this is just
   a helper program to test manually putting/reusing (of) connection to/from the pool. The pooling in
   the DM you use should be turned on for MySQL driver */

DECLARE_TEST(t_reset_connection)
{
  DECLARE_BASIC_HANDLES(henv1, hdbc1, hstmt1);
  int i;
  SQLINTEGER len;
  const int rows= 10;
  char dbase[32];

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_reset_connection");
  ok_sql(hstmt, "DROP DATABASE IF EXISTS t_reset_connection");
  ok_sql(hstmt, "CREATE DATABASE t_reset_connection");
  ok_sql(hstmt, "CREATE TABLE t_reset_connection(a int(10) unsigned NOT NULL PRIMARY KEY AUTO_INCREMENT)");

  for (i= 0; i < rows ; ++i)
  {
    ok_sql(hstmt, "INSERT INTO t_reset_connection values()");
  }

  ok_env(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1));
  ok_con(hdbc1, SQLConnect(hdbc1, mydsn, strlen(mydsn), myuid, strlen(myuid), mypwd, strlen(mypwd)));

  ok_con(hdbc1, SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1));

  ok_stmt(hstmt1, SQLSetStmtAttr(hstmt1,SQL_ATTR_MAX_ROWS,(SQLPOINTER)5,0));

  ok_sql(hstmt1, "select a from t_reset_connection");
  is_num(myrowcount(hstmt1), 5);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));

  ok_con(hdbc1, SQLSetConnectAttr(hdbc1, SQL_ATTR_CURRENT_CATALOG,
                                  "t_reset_connection", strlen("t_reset_connection")));
  ok_con(hdbc1, SQLGetConnectAttr(hdbc1, SQL_ATTR_CURRENT_CATALOG,
                                  dbase, sizeof(dbase), &len));

  is_str(dbase, "t_reset_connection", len);
  SQLDisconnect(hdbc1);
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  hdbc1= NULL;
  hstmt1= NULL;

  sleep(2);

  ok_env(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1));

  /* Here the connection is supposed to be taken from the pool */
  ok_con(hdbc1, SQLConnect(hdbc1, mydsn, strlen(mydsn), myuid, strlen(myuid), mypwd, strlen(mypwd)));

  ok_con(hdbc1, SQLGetConnectAttr(hdbc1, SQL_ATTR_CURRENT_CATALOG,
                                  dbase, sizeof(dbase), &len));
  is_str(dbase, mydb, len);

  ok_con(hdbc1, SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1));

  ok_sql(hstmt1, "select a from t_reset_connection");
  is_num(myrowcount(hstmt1), rows);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));
  SQLDisconnect(hdbc1);
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_reset_connection");
  ok_sql(hstmt, "DROP DATABASE IF EXISTS t_reset_connection");

  return OK;
}


BEGIN_TESTS
  ADD_TEST(t_reset_connection)
END_TESTS

myenable_pooling= 1;

RUN_TESTS
