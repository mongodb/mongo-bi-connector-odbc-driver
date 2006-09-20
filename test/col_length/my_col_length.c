/***************************************************************************
                          my_col_length.c  -  description
                             -------------------
    begin                : Tue Oct 16 2001
    copyright            : (C) MySQL AB 1997-2001
    author               : venu ( venu@mysql.com )
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "mytest3.h"

SQLCHAR *mysock= NULL;

/**
TESTING FOR TRUE LENGTH
*/
void t_true_length(SQLHENV henv)
{
    SQLRETURN rc;
    SQLHDBC hdbc;
    SQLHSTMT hstmt;
    char     conn[256];
    char     data1[25],data2[25];
    SQLINTEGER len1,len2,desc_len;

    rc = SQLAllocConnect(henv,&hdbc);
    myenv(henv,rc);

    sprintf(conn,"DRIVER=MyODBC;DSN=%s;UID=%s;PWD=%s;OPTION=0",mydsn,myuid,mypwd); 
    if (mysock != NULL)
    {
      strcat(conn, ";SOCKET=");
      strcat(conn, mysock);
    }
    rc = SQLDriverConnect(hdbc,NULL,conn,sizeof(conn),NULL,0,NULL,SQL_DRIVER_NOPROMPT);
    mycon(hdbc,rc);

    rc = SQLAllocStmt(hdbc,&hstmt);
    mycon(hdbc,rc);
    /* PMG 2004.05.04 Added this in order for the test to pass. */
    sprintf(conn,"DRIVER=MyODBC;DSN=%s;UID=%s;PWD=%s;OPTION=0",mydsn,myuid,mypwd); 
    rc = SQLExecDirect(hstmt,"CREATE DATABASE IF NOT EXISTS client_odbc_test",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"use client_odbc_test",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"DROP TABLE IF EXISTS t_true_length",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"create table t_true_length(col1 char(20),col2 varchar(15))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into t_true_length values('venu','mysql')",SQL_NTS);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLEndTran(SQL_HANDLE_DBC,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLExecDirect(hstmt,"select * from t_true_length",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLDescribeCol(hstmt,1,NULL,40,NULL,NULL,&desc_len,NULL,NULL);
    mystmt(hstmt,rc);
    printMessage("desc-col1-length:%d\n",desc_len);

    rc = SQLDescribeCol(hstmt,2,NULL,40,NULL,NULL,&desc_len,NULL,NULL);
    mystmt(hstmt,rc);
    printMessage("desc-col2-length:%d\n",desc_len);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,&data1,20,&len1);
    mystmt(hstmt,rc);
    printMessage("fetch-col1:%s(%d)\n",data1,len1);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,&data2,20,&len2);
    mystmt(hstmt,rc);
    printMessage("fetch-col2:%s(%d)\n",data2,len2);

    rc = SQLFetch(hstmt);
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);     

    SQLFreeStmt(hstmt,SQL_UNBIND);    
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLDisconnect(hdbc);
    mycon(hdbc,rc);

    rc = SQLFreeConnect(hdbc);
    mycon(hdbc,rc);
}
/**
TESTING FOR MAX LENGTH
*/
void t_max_length(SQLHENV henv)
{
    SQLRETURN rc;
    SQLHDBC hdbc;
    SQLHSTMT hstmt;
    SQLCHAR data1[25],data2[25];
    SQLULEN desc_len;
    SQLINTEGER len1,len2;
    SQLCHAR  conn[256];

    rc = SQLAllocConnect(henv,&hdbc);
    myenv(henv,rc);

    sprintf(conn,"DRIVER=MyODBC;DSN=%s;UID=%s;PWD=%s;OPTION=1",mydsn,myuid,mypwd); 
    if (mysock != NULL)
    {
      strcat(conn, ";SOCKET=");
      strcat(conn, mysock);
    }
    rc = SQLDriverConnect(hdbc,NULL,conn,sizeof(conn),NULL,0,NULL,SQL_DRIVER_NOPROMPT);
    mycon(hdbc,rc);

    rc = SQLAllocStmt(hdbc,&hstmt);
    mycon(hdbc,rc);
    sprintf(conn,"DRIVER=MyODBC;DSN=%s;UID=%s;PWD=%s;OPTION=0",mydsn,myuid,mypwd); 

    rc = SQLExecDirect(hstmt,"CREATE DATABASE IF NOT EXISTS client_odbc_test",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"use client_odbc_test",SQL_NTS);
    mystmt(hstmt,rc);

    SQLExecDirect(hstmt,"drop table t_max_length",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"create table t_max_length(col1 char(20),col2 varchar(15))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into t_max_length values('venu','mysql')",SQL_NTS);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLEndTran(SQL_HANDLE_DBC,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLExecDirect(hstmt,"select * from t_max_length",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLDescribeCol(hstmt,1,NULL,40,NULL,NULL,&desc_len,NULL,NULL);
    mystmt(hstmt,rc);
    printMessage("desc-col1-length:%d\n",desc_len);

    rc = SQLDescribeCol(hstmt,2,NULL,40,NULL,NULL,&desc_len,NULL,NULL);
    mystmt(hstmt,rc);
    printMessage("desc-col2-length:%d\n",desc_len);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,&data1,20,&len1);
    mystmt(hstmt,rc);
    printMessage("fetch-col1:%s(%d)\n",data1,len1);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,&data2,20,&len2);
    mystmt(hstmt,rc);
    printMessage("fetch-col2:%s(%d)\n",data2,len2);

    rc = SQLFetch(hstmt);
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);     

    SQLFreeStmt(hstmt,SQL_UNBIND);    
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLDisconnect(hdbc);
    mycon(hdbc,rc);

    rc = SQLFreeConnect(hdbc);
    mycon(hdbc,rc);
}
/**
MAIN ROUTINE...
*/
int main(int argc, char *argv[])
{
    SQLHENV   henv;
    SQLINTEGER narg;
    SQLRETURN rc;      

    printMessageHeader();

    /*
     * if connection string supplied through arguments, overrite
     * the default one..
    */
    for (narg = 1; narg < argc; narg++)
    {
        if ( narg == 1 )
            mydsn = argv[1];
        else if ( narg == 2 )
            myuid = argv[2];
        else if ( narg == 3 )
            mypwd = argv[3];
        else if ( narg == 4 )
            mysock= argv[4];
    }    

    rc = SQLAllocEnv(&henv);
    myenv(henv,rc);    

    t_true_length(henv);     
    t_max_length(henv); 

    rc = SQLFreeEnv(henv);
    myenv(henv,rc);    

    printMessageFooter( 1 );

    return(0);
}

