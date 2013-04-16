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

/**
  Bug #66548: Driver use the char ';' as separator in attributes string 
              instead of the '\0'
*/
DECLARE_TEST(t_bug66548)
{
  /* TODO: remove #ifdef _WIN32 when Linux and MacOS setup is released */
#ifdef _WIN32
  SQLCHAR attrs[8192];
  SQLCHAR drv[128];
  SQLCHAR conn_out[512];
  SQLSMALLINT conn_out_len;
  int i, len;
  HDBC hdbc1;
  
  /* 
    Use ';' as separator because sprintf doesn't work after '\0'
    The last attribute in the list must end with ';'
  */

  sprintf((char*)attrs, "DSN=bug66548dsn;SERVER=%s;USER=%s;PASSWORD=%s;"
                          "DATABASE=%s;", myserver, myuid, mypwd, mydb);

  len= strlen(attrs);
  
  /* replacing ';' by '\0' */
  for (i= 0; i < len; ++i)
  {
    if (attrs[i] == ';')
      attrs[i]= '\0';
  }

  /* Adding the extra string termination to get \0\0 */
  attrs[i]= '\0';

  if (mydriver[0] == '{')
  {
    /* We need to remove {} in the driver name or it will not register */
    len= strlen(mydriver);
    memcpy(drv, mydriver+1, sizeof(SQLCHAR)*(len-2));
    drv[len-2]= '\0';
  }

  /* 
    Trying to remove the DSN if it is left from the previous run, 
    no need to check the result
  */
  SQLConfigDataSource(NULL, ODBC_REMOVE_SYS_DSN, drv, "DSN=bug66548dsn\0\0");

  /* Create the DSN */
  ok_install(SQLConfigDataSource(NULL, ODBC_ADD_SYS_DSN, drv, attrs));

  ok_env(henv, SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1));

  /* Try connecting using the newly created DSN */
  ok_con(hdbc1, SQLDriverConnect(hdbc1, NULL, "DSN=bug66548dsn", SQL_NTS, conn_out,
                                 sizeof(conn_out), &conn_out_len,
                                 SQL_DRIVER_NOPROMPT));
  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeHandle(SQL_HANDLE_DBC, hdbc1));

  ok_install(SQLConfigDataSource(NULL, ODBC_REMOVE_SYS_DSN, drv, "DSN=bug66548dsn\0\0"));
#endif

  return OK;
}


BEGIN_TESTS
  ADD_TEST(t_bug66548)
END_TESTS


RUN_TESTS

