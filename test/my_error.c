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


void check_sqlstate(SQLHDBC hdbc, SQLHSTMT hstmt,SQLCHAR *sqlstate)
{
    SQLCHAR     sql_state[6];
    SQLINTEGER  err_code=0;
    SQLCHAR     err_msg[SQL_MAX_MESSAGE_LENGTH]={0};
    SQLSMALLINT err_len=0;

    memset(err_msg,'C',SQL_MAX_MESSAGE_LENGTH);
    SQLGetDiagRec(SQL_HANDLE_STMT,hstmt,1,
                  (SQLCHAR *)&sql_state,(SQLINTEGER *)&err_code,
                  (SQLCHAR*)&err_msg, SQL_MAX_MESSAGE_LENGTH-1,
                  (SQLSMALLINT *)&err_len);

    printMessage("\n");
    printMessage("\n SQLSTATE (expected:%s, obtained:%s)",sqlstate,sql_state);
    printMessage("\n ERROR: %s",err_msg);
    if (!driver_min_version(hdbc,"03.52",5))
        myassert(strcmp(sql_state,sqlstate)==0);
}


DECLARE_TEST(t_odbc3_error)
{
    SQLRETURN rc; 
    SQLHENV henv1;
    SQLHDBC hdbc1;
    SQLHSTMT hstmt1;
    SQLINTEGER ov_version;

    rc = SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&henv1);
    myenv(henv1,rc);

    rc = SQLSetEnvAttr(henv1,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0);
    myenv(henv1,rc);

    rc = SQLAllocHandle(SQL_HANDLE_DBC,henv1,&hdbc1);
    myenv(henv1,rc);

    rc = SQLGetEnvAttr(henv1,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)&ov_version,0,0);
    myenv(henv1,rc);
    printMessage("\n default odbc version:%d",ov_version);
    my_assert(ov_version == SQL_OV_ODBC3);

    rc = SQLConnect(hdbc1, mydsn, SQL_NTS, myuid, SQL_NTS,  mypwd, SQL_NTS);
    mycon(hdbc1,rc);

    rc = SQLAllocHandle(SQL_HANDLE_STMT,hdbc1,&hstmt1);
    mycon(hdbc1, rc);

    rc = SQLExecDirect(hstmt1,"CREATE DATABASE IF NOT EXISTS client_odbc_test",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLExecDirect(hstmt1,"use client_odbc_test",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLExecDirect(hstmt1,"select * from iNON_EXITING_TABLE",SQL_NTS);
    myassert(rc == SQL_ERROR);
    check_sqlstate(hdbc1, hstmt1,"42S02");

    SQLExecDirect(hstmt1,"DROP TABLE test_error",SQL_NTS);
    rc = SQLExecDirect(hstmt1,"CREATE TABLE test_error(id int)",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLExecDirect(hstmt1,"CREATE TABLE test_error(id int)",SQL_NTS);
    myassert(rc == SQL_ERROR);
    check_sqlstate(hdbc1, hstmt1,"42S01");

    SQLFreeStmt(hstmt1,SQL_CLOSE);

    rc = SQLSetStmtAttr(hstmt1,SQL_ATTR_FETCH_BOOKMARK_PTR,(SQLPOINTER)NULL,0);
    myassert(rc == SQL_ERROR);
    check_sqlstate(hdbc1, hstmt1,"HYC00");

    rc = SQLFreeStmt(hstmt1,SQL_DROP);
    mystmt(hstmt1,rc);

    rc = SQLDisconnect(hdbc1);
    mycon(hdbc1, rc);

    rc = SQLFreeHandle(SQL_HANDLE_DBC,hdbc1);
    mycon(hdbc1,rc);

    rc = SQLFreeHandle(SQL_HANDLE_ENV,henv1);
    myenv(henv1,rc);
}


DECLARE_TEST(t_odbc2_error)
{
    SQLRETURN rc;
    SQLHENV henv1;
    SQLHDBC hdbc1;
    SQLHSTMT hstmt1;
    SQLINTEGER ov_version;

    rc = SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&henv1);
    myenv(henv1,rc);

    rc = SQLSetEnvAttr(henv1,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC2,0);
    myenv(henv1,rc);

    rc = SQLAllocHandle(SQL_HANDLE_DBC,henv1,&hdbc1);
    myenv(henv1,rc);

    rc = SQLGetEnvAttr(henv1,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)&ov_version,0,0);
    myenv(henv1,rc);
    printMessage("\ndefault odbc version:%d",ov_version);
    my_assert(ov_version == SQL_OV_ODBC2);

    rc = SQLConnect(hdbc1, mydsn, SQL_NTS, myuid, SQL_NTS,  mypwd, SQL_NTS);
    mycon(hdbc1,rc);

    rc = SQLAllocHandle(SQL_HANDLE_STMT,hdbc1,&hstmt1);
    mycon(hdbc1, rc);

    rc = SQLExecDirect(hstmt1,"CREATE DATABASE IF NOT EXISTS client_odbc_test",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLExecDirect(hstmt1,"use client_odbc_test",SQL_NTS);
    mystmt(hstmt1,rc);


    rc = SQLExecDirect(hstmt1,"select * from iNON_EXITING_TABLE",SQL_NTS);
    myassert(rc == SQL_ERROR);
    check_sqlstate(hdbc1, hstmt1,"S0002");

    SQLExecDirect(hstmt1,"DROP TABLE test_error",SQL_NTS);
    rc = SQLExecDirect(hstmt1,"CREATE TABLE test_error(id int)",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLExecDirect(hstmt1,"CREATE TABLE test_error(id int)",SQL_NTS);
    myassert(rc == SQL_ERROR);
    check_sqlstate(hdbc1, hstmt1,"S0001");

    SQLFreeStmt(hstmt1,SQL_CLOSE);

    rc = SQLSetStmtAttr(hstmt1,SQL_ATTR_FETCH_BOOKMARK_PTR,(SQLPOINTER)NULL,0);
    myassert(rc == SQL_ERROR);
    check_sqlstate(hdbc1, hstmt1,"S1C00");

    rc = SQLFreeStmt(hstmt1,SQL_DROP);
    mystmt(hstmt1,rc);

    rc = SQLDisconnect(hdbc1);
    mycon(hdbc1, rc);

    rc = SQLFreeHandle(SQL_HANDLE_DBC,hdbc1);
    mycon(hdbc1,rc);

    rc = SQLFreeHandle(SQL_HANDLE_ENV,henv1);
    myenv(henv1,rc);
}

BEGIN_TESTS
  ADD_TEST(t_odbc2_error)
  ADD_TEST(t_odbc3_error)
  /* Run twice to test the driver's handling of switching  */
  ADD_TEST(t_odbc2_error)
END_TESTS

RUN_TESTS
