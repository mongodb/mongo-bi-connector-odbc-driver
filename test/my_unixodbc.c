/*
  Copyright (c) 2003, 2007, 2008 MySQL AB, 2010 Sun Microsystems, Inc.
  Use is subject to license terms.

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

DECLARE_TEST(t_odbc3_envattr)
{
    SQLRETURN rc;
    SQLHENV henv1;
    SQLHDBC hdbc1;
    SQLINTEGER ov_version;

    rc = SQLAllocEnv(&henv1);
    myenv(henv1,rc);

    rc = SQLGetEnvAttr(henv1,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)&ov_version,0,0);
    myenv(henv1,rc);
    printMessage("default odbc version:%d\n",ov_version);
    is_num(ov_version, SQL_OV_ODBC2);

    rc = SQLSetEnvAttr(henv1,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0);
    myenv(henv1,rc);

    rc = SQLAllocConnect(henv1,&hdbc1);
    myenv(henv1,rc);

    rc = SQLFreeConnect(hdbc1);
    mycon(hdbc1,rc);

    rc = SQLSetEnvAttr(henv1,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0);
    myenv(henv1,rc);

    rc = SQLGetEnvAttr(henv1,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)&ov_version,0,0);
    myenv(henv1,rc);
    printMessage("new odbc version:%d\n",ov_version);
    my_assert(ov_version == SQL_OV_ODBC3);

    rc = SQLFreeEnv(henv1);
    myenv(henv1,rc);

  return OK;
}


DECLARE_TEST(t_odbc3_handle)
{
    SQLRETURN rc;
    SQLHENV henv1;
    SQLHDBC hdbc1;
    SQLHSTMT hstmt1;
    SQLINTEGER ov_version;

    rc = SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&henv1);
    myenv(henv1,rc);

    /*
      Verify that we get an error trying to allocate a connection handle
      before we've set SQL_ATTR_ODBC_VERSION.
    */
    rc = SQLAllocHandle(SQL_HANDLE_DBC,henv1,&hdbc1);
    myenv_err(henv1,rc == SQL_ERROR,rc);

    rc = SQLSetEnvAttr(henv1,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0);
    myenv(henv1,rc);

    rc = SQLGetEnvAttr(henv1,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)&ov_version,0,0);
    myenv(henv1,rc);
    my_assert(ov_version == SQL_OV_ODBC3);

    rc = SQLAllocHandle(SQL_HANDLE_DBC,henv1,&hdbc1);
    myenv(henv1,rc);

    rc = SQLConnect(hdbc1, mydsn, SQL_NTS, myuid, SQL_NTS,  mypwd, SQL_NTS);
    mycon(hdbc1,rc);

    rc = SQLAllocHandle(SQL_HANDLE_STMT,hdbc1,&hstmt1);
    mycon(hdbc1, rc);

    rc = SQLDisconnect(hdbc1);
    mycon(hdbc1, rc);

    rc = SQLFreeHandle(SQL_HANDLE_DBC,hdbc1);
    mycon(hdbc1,rc);

    rc = SQLFreeHandle(SQL_HANDLE_ENV,henv1);
    myenv(henv1,rc);

  return OK;
}


DECLARE_TEST(t_driver_connect)
{
  DECLARE_BASIC_HANDLES(henv1, hdbc1, hstmt1);

  is(OK == alloc_basic_handles_with_opt(&henv1, &hdbc1, &hstmt1, NULL,
                                        NULL, NULL, NULL, 
                                        "OPTION=3;STMT=use mysql"));
  
  free_basic_handles(&henv1, &hdbc1, &hstmt1);

  return OK;
}


BEGIN_TESTS
  ADD_TEST(t_odbc3_envattr)
#ifndef NO_DRIVERMANAGER
  ADD_TEST(t_odbc3_handle)
#endif
  ADD_TEST(t_driver_connect)
END_TESTS


RUN_TESTS
