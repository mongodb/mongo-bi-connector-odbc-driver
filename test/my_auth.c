/*
  Copyright (c) 2011, 2012, Oracle and/or its affiliates. All rights reserved.

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
#include "mysql_version.h"

/* 
  Test for setting plugin-dir and default-auth mysql options 
  using Connector/ODBC connection parameter.

*/
DECLARE_TEST(t_plugin_auth)
{
#if MYSQL_VERSION_ID >= 50507
  SQLCHAR   conn[512], conn_out[512];
  SQLSMALLINT conn_out_len;
  SQLCHAR *tplugin_dir= (SQLCHAR *)"/tmp/test_new_directory/";
  SQLCHAR *tdefault_auth= (SQLCHAR *)"auth_test_plugin";
  HDBC hdbc1;
  HSTMT hstmt1;
  SQLCHAR buf[255];
  SQLLEN buflen;
  SQLRETURN rc;
  SQLUSMALLINT plugin_status= FALSE;

  ok_sql(hstmt, "SELECT PLUGIN_NAME FROM INFORMATION_SCHEMA.PLUGINS " 
                "WHERE PLUGIN_NAME='test_plugin_server';");
  SQLFetch(hstmt);
  SQLGetData(hstmt, 1, SQL_CHAR, buf, sizeof(buf), &buflen);

  /* Check whether plugin already exist, if not install it. */
  if(strcmp(buf, "test_plugin_server")!=0)
  {
    plugin_status= TRUE;
#ifdef _WIN32
    ok_sql(hstmt, "INSTALL PLUGIN test_plugin_server "
                    "SONAME 'auth_test_plugin'");
#else
    ok_sql(hstmt, "INSTALL PLUGIN test_plugin_server "
                    "SONAME 'auth_test_plugin.so'");
#endif
  }

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  /* Can FAIL if user does not exists */
  SQLExecDirect(hstmt, "DROP USER `plug_13070711`@`localhost`", SQL_NTS);
  SQLExecDirect(hstmt, "DROP USER `plug_13070711`@`%`", SQL_NTS);

  ok_sql(hstmt, "CREATE USER `plug_13070711`@`%` "
                  "IDENTIFIED BY 'plug_dest_passwd';");
  ok_sql(hstmt, "GRANT ALL PRIVILEGES ON test.* "
                  "TO `plug_13070711`@`%`;");

  ok_sql(hstmt, "CREATE USER `plug_13070711`@`localhost` "
                  "IDENTIFIED BY 'plug_dest_passwd';");
  ok_sql(hstmt, "GRANT ALL PRIVILEGES ON test.* "
                  "TO `plug_13070711`@`localhost`;");

  ok_env(henv, SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1));

  /*
    Test verifies that setting PLUGIN_DIR coonection parameter 
    properly works by setting wrong plugin-dir path using 
    PLUGIN_DIR connection paramater, which fails connection as 
    it won't find required library needed for authentication.
  */
  sprintf((char *)conn, "DSN=%s;UID=plug_13070711;PWD=plug_dest_passwd;"
                        "DATABASE=test;DEFAULT_AUTH=%s;PLUGIN_DIR=%s",
            (char *)mydsn, (char *) tdefault_auth, (char *) tplugin_dir);
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

  expect_dbc(hdbc1, SQLDriverConnect(hdbc1, NULL, conn, SQL_NTS, conn_out,
                              sizeof(conn_out), &conn_out_len,
                              SQL_DRIVER_NOPROMPT), SQL_ERROR);

  /*
    Test verifies that setting DEFAULT_AUTH coonection parameter 
    properly works by setting wrong default-auth library name 
    using DEFAULT_AUTH connection paramater, which fails connection as 
    it won't find required library needed for authentication.
  */
  sprintf((char *)conn, "DSN=%s;UID=plug_13070711;PWD=plug_dest_passwd;"
                        "DATABASE=test;DEFAULT_AUTH=%s_test",
            (char *)mydsn, (char *) tdefault_auth);
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

  expect_dbc(hdbc1, SQLDriverConnect(hdbc1, NULL, conn, SQL_NTS, conn_out,
                              sizeof(conn_out), &conn_out_len,
                              SQL_DRIVER_NOPROMPT), SQL_ERROR);

  /*
    Tests for successfull connection using DEFAULT_AUTH and TEST_PLUGINDIR
  */
  sprintf((char *)conn, "DSN=%s;UID=plug_13070711;PWD=plug_dest_passwd;"
                        "DATABASE=test;DEFAULT_AUTH=auth_test_plugin", 
  (char *)mydsn);
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
  if (myauth && myauth[0])
  {
    strcat((char *)conn, ";DEFAULT_AUTH=");
    strcat((char *)conn, (char *)myauth);
  }
  if (myplugindir && myplugindir[0])
  {
    strcat((char *)conn, ";PLUGIN_DIR=");
    strcat((char *)conn, (char *)myplugindir);
  }

  rc= SQLDriverConnect(hdbc1, NULL, conn, SQL_NTS, conn_out,
                        sizeof(conn_out), &conn_out_len,
                        SQL_DRIVER_NOPROMPT);

  /*
    If authetication plugin library 'auth_test_plugin' not found
    then either TEST_PLUGINDIR is not set and/or default path 
    does not contain this library.
  */
  if (rc != SQL_SUCCESS)
  {
    ok_sql(hstmt, "DROP USER `plug_13070711`@`localhost`");
    ok_sql(hstmt, "DROP USER `plug_13070711`@`%`");
    ok_sql(hstmt, "UNINSTALL PLUGIN test_plugin_server");
    printf("# Set TEST_PLUGINDIR environment variables\n");
    return FAIL;
  }

  ok_con(hdbc1, SQLAllocStmt(hdbc1, &hstmt1));

  ok_sql(hstmt1, "select CURRENT_USER()");
  ok_stmt(hstmt1,SQLFetch(hstmt1));

  is_str(my_fetch_str(hstmt1, buf, 1), "plug_13070711@localhost", 23);

  ok_sql(hstmt, "DROP USER `plug_13070711`@`localhost`");
  ok_sql(hstmt, "DROP USER `plug_13070711`@`%`");

  if(plugin_status)
  {
    ok_sql(hstmt, "UNINSTALL PLUGIN test_plugin_server");
  }

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));
  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeHandle(SQL_HANDLE_DBC, hdbc1));
#endif

  return OK;
}


DECLARE_TEST(t_dummy_test)
{
  return OK;
}


BEGIN_TESTS
  // ADD_TEST(t_plugin_auth) TODO: Fix
  ADD_TEST(t_dummy_test)
  END_TESTS

RUN_TESTS
