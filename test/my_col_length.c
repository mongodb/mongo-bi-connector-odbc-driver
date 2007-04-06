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

/* TESTING FOR TRUE LENGTH */
DECLARE_TEST(t_true_length)
{
    SQLRETURN rc;
    SQLHDBC hdbc1;
    SQLHSTMT hstmt1;
    char     conn[256];
    char     data1[25],data2[25];
    SQLINTEGER len1,len2,desc_len;

    rc = SQLAllocConnect(henv,&hdbc1);
    myenv(henv,rc);

    sprintf(conn,"DSN=%s;UID=%s;PWD=%s;OPTION=0",mydsn,myuid,mypwd); 
    if (mysock != NULL)
    {
      strcat(conn, ";SOCKET=");
      strcat(conn, mysock);
    }
    rc = SQLDriverConnect(hdbc1,NULL,conn,sizeof(conn),NULL,0,NULL,SQL_DRIVER_NOPROMPT);
    mycon(hdbc1,rc);

    rc = SQLAllocStmt(hdbc1,&hstmt1);
    mycon(hdbc1,rc);
    rc = SQLExecDirect(hstmt1,"CREATE DATABASE IF NOT EXISTS client_odbc_test",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLExecDirect(hstmt1,"use client_odbc_test",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLExecDirect(hstmt1,"DROP TABLE IF EXISTS t_true_length",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLExecDirect(hstmt1,"create table t_true_length(col1 char(20),col2 varchar(15))",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLExecDirect(hstmt1,"insert into t_true_length values('venu','mysql')",SQL_NTS);
    mystmt(hstmt1,rc);

    SQLFreeStmt(hstmt1,SQL_CLOSE);

    rc = SQLEndTran(SQL_HANDLE_DBC,hdbc1,SQL_COMMIT);
    mycon(hdbc1,rc);

    rc = SQLExecDirect(hstmt1,"select * from t_true_length",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLDescribeCol(hstmt1,1,NULL,40,NULL,NULL,&desc_len,NULL,NULL);
    mystmt(hstmt1,rc);
    printMessage("desc-col1-length:%d\n",desc_len);

    rc = SQLDescribeCol(hstmt1,2,NULL,40,NULL,NULL,&desc_len,NULL,NULL);
    mystmt(hstmt1,rc);
    printMessage("desc-col2-length:%d\n",desc_len);

    rc = SQLFetch(hstmt1);
    mystmt(hstmt1,rc);

    rc = SQLGetData(hstmt1,1,SQL_C_CHAR,&data1,20,&len1);
    mystmt(hstmt1,rc);
    printMessage("fetch-col1:%s(%d)\n",data1,len1);

    rc = SQLGetData(hstmt1,2,SQL_C_CHAR,&data2,20,&len2);
    mystmt(hstmt1,rc);
    printMessage("fetch-col2:%s(%d)\n",data2,len2);

    rc = SQLFetch(hstmt1);
    mystmt_err(hstmt1,rc == SQL_NO_DATA_FOUND, rc);     

    SQLFreeStmt(hstmt1,SQL_UNBIND);    
    SQLFreeStmt(hstmt1,SQL_CLOSE);

    rc = SQLDisconnect(hdbc1);
    mycon(hdbc1,rc);

    rc = SQLFreeConnect(hdbc1);
    mycon(hdbc1,rc);
}


/* TESTING FOR MAX LENGTH */
DECLARE_TEST(t_max_length)
{
    SQLRETURN rc;
    SQLHDBC hdbc1;
    SQLHSTMT hstmt1;
    SQLCHAR data1[25],data2[25];
    SQLULEN desc_len;
    SQLINTEGER len1,len2;
    SQLCHAR  conn[256];

    rc = SQLAllocConnect(henv,&hdbc1);
    myenv(henv,rc);

    sprintf(conn,"DSN=%s;UID=%s;PWD=%s;OPTION=1",mydsn,myuid,mypwd); 
    if (mysock != NULL)
    {
      strcat(conn, ";SOCKET=");
      strcat(conn, mysock);
    }
    rc = SQLDriverConnect(hdbc1,NULL,conn,sizeof(conn),NULL,0,NULL,SQL_DRIVER_NOPROMPT);
    mycon(hdbc1,rc);

    rc = SQLAllocStmt(hdbc1,&hstmt1);
    mycon(hdbc1,rc);
    sprintf(conn,"DSN=%s;UID=%s;PWD=%s;OPTION=0",mydsn,myuid,mypwd); 

    rc = SQLExecDirect(hstmt1,"CREATE DATABASE IF NOT EXISTS client_odbc_test",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLExecDirect(hstmt1,"use client_odbc_test",SQL_NTS);
    mystmt(hstmt1,rc);

    SQLExecDirect(hstmt1,"drop table t_max_length",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLExecDirect(hstmt1,"create table t_max_length(col1 char(20),col2 varchar(15))",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLExecDirect(hstmt1,"insert into t_max_length values('venu','mysql')",SQL_NTS);
    mystmt(hstmt1,rc);

    SQLFreeStmt(hstmt1,SQL_CLOSE);

    rc = SQLEndTran(SQL_HANDLE_DBC,hdbc1,SQL_COMMIT);
    mycon(hdbc1,rc);

    rc = SQLExecDirect(hstmt1,"select * from t_max_length",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLDescribeCol(hstmt1,1,NULL,40,NULL,NULL,&desc_len,NULL,NULL);
    mystmt(hstmt1,rc);
    printMessage("desc-col1-length:%d\n",desc_len);

    rc = SQLDescribeCol(hstmt1,2,NULL,40,NULL,NULL,&desc_len,NULL,NULL);
    mystmt(hstmt1,rc);
    printMessage("desc-col2-length:%d\n",desc_len);

    rc = SQLFetch(hstmt1);
    mystmt(hstmt1,rc);

    rc = SQLGetData(hstmt1,1,SQL_C_CHAR,&data1,20,&len1);
    mystmt(hstmt1,rc);
    printMessage("fetch-col1:%s(%d)\n",data1,len1);

    rc = SQLGetData(hstmt1,2,SQL_C_CHAR,&data2,20,&len2);
    mystmt(hstmt1,rc);
    printMessage("fetch-col2:%s(%d)\n",data2,len2);

    rc = SQLFetch(hstmt1);
    mystmt_err(hstmt1,rc == SQL_NO_DATA_FOUND, rc);     

    SQLFreeStmt(hstmt1,SQL_UNBIND);    
    SQLFreeStmt(hstmt1,SQL_CLOSE);

    rc = SQLDisconnect(hdbc1);
    mycon(hdbc1,rc);

    rc = SQLFreeConnect(hdbc1);
    mycon(hdbc1,rc);
}


BEGIN_TESTS
  ADD_TEST(t_true_length)
  ADD_TEST(t_max_length)
END_TESTS


RUN_TESTS
