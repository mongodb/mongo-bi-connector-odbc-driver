/*
  Copyright (C) 1997-2007 MySQL AB

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
    SQLRETURN rc;
    SQLHENV henv1;
    SQLHDBC hdbc1;
    SQLCHAR conn_in[255], conn_out[255];
    SQLSMALLINT conn_out_len;

    rc = SQLAllocEnv(&henv1);
    myenv(henv1,rc);

    rc = SQLAllocConnect(henv1,&hdbc1);
    myenv(henv1,rc);

    sprintf(conn_in,"DRIVER=%s;USER=%s;PASSWORD=%s;"
                    "DATABASE=%s;SERVER=%s;OPTION=3;STMT=use mysql",
            mydriver, myuid, mypwd, mydb, myserver);
    if (mysock != NULL)
    {
      strcat(conn_in, ";SOCKET=");
      strcat(conn_in, mysock);
    }
    if (myport)
    {
      char pbuff[20];
      sprintf(pbuff, ";PORT=%d", myport);
      strcat((char *)conn_in, pbuff);
    }
    rc = SQLDriverConnect(hdbc1, (SQLHWND)0, conn_in, sizeof(conn_in),
                          conn_out, sizeof(conn_out), &conn_out_len,
                          SQL_DRIVER_NOPROMPT);

    if (rc == SQL_SUCCESS)
    {
        mycon(hdbc1,rc);
        printMessage( "output string: `%s`\n", conn_out);

        rc = SQLDisconnect(hdbc1);
        mycon(hdbc1,rc);
    }

    rc = SQLFreeConnect(hdbc1);
    mycon(hdbc1,rc);

    rc = SQLFreeEnv(henv1);
    myenv(henv1,rc);

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
