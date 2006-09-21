/***************************************************************************
                          my_error.c  -  description
                             -------------------
    begin                : Wed Aug 8 2001
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

void t_odbc3_error()
{
    SQLRETURN rc; 
    SQLHENV henv;
    SQLHDBC hdbc;
    SQLHSTMT hstmt;
    SQLINTEGER ov_version;

    rc = SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&henv);
    myenv(henv,rc);

    rc = SQLSetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0);
    myenv(henv,rc);

    rc = SQLAllocHandle(SQL_HANDLE_DBC,henv,&hdbc);
    myenv(henv,rc);

    rc = SQLGetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)&ov_version,0,0);
    myenv(henv,rc);
    printMessage("\n default odbc version:%d",ov_version);
    my_assert(ov_version == SQL_OV_ODBC3);

    rc = SQLConnect(hdbc, mydsn, SQL_NTS, myuid, SQL_NTS,  mypwd, SQL_NTS);
    mycon(hdbc,rc);

    rc = SQLAllocHandle(SQL_HANDLE_STMT,hdbc,&hstmt);
    mycon(hdbc, rc);

    rc = SQLExecDirect(hstmt,"CREATE DATABASE IF NOT EXISTS client_odbc_test",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"use client_odbc_test",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"select * from iNON_EXITING_TABLE",SQL_NTS);
    myassert(rc == SQL_ERROR);
    check_sqlstate(hdbc, hstmt,"42S02");

    SQLExecDirect(hstmt,"DROP TABLE test_error",SQL_NTS);
    rc = SQLExecDirect(hstmt,"CREATE TABLE test_error(id int)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"CREATE TABLE test_error(id int)",SQL_NTS);
    myassert(rc == SQL_ERROR);
    check_sqlstate(hdbc, hstmt,"42S01");

    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_FETCH_BOOKMARK_PTR,(SQLPOINTER)NULL,0);
    myassert(rc == SQL_ERROR);
    check_sqlstate(hdbc, hstmt,"HYC00");

    rc = SQLFreeStmt(hstmt,SQL_DROP);
    mystmt(hstmt,rc);

    rc = SQLDisconnect(hdbc);
    mycon(hdbc, rc);

    rc = SQLFreeHandle(SQL_HANDLE_DBC,hdbc);
    mycon(hdbc,rc);

    rc = SQLFreeHandle(SQL_HANDLE_ENV,henv);
    myenv(henv,rc);
}
void t_odbc2_error()
{
    SQLRETURN rc;
    SQLHENV henv;
    SQLHDBC hdbc;
    SQLHSTMT hstmt;
    SQLINTEGER ov_version;

    rc = SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&henv);
    myenv(henv,rc);

    rc = SQLSetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC2,0);
    myenv(henv,rc);

    rc = SQLAllocHandle(SQL_HANDLE_DBC,henv,&hdbc);
    myenv(henv,rc);

    rc = SQLGetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)&ov_version,0,0);
    myenv(henv,rc);
    printMessage("\ndefault odbc version:%d",ov_version);
    my_assert(ov_version == SQL_OV_ODBC2);

    rc = SQLConnect(hdbc, mydsn, SQL_NTS, myuid, SQL_NTS,  mypwd, SQL_NTS);
    mycon(hdbc,rc);

    rc = SQLAllocHandle(SQL_HANDLE_STMT,hdbc,&hstmt);
    mycon(hdbc, rc);

    rc = SQLExecDirect(hstmt,"CREATE DATABASE IF NOT EXISTS client_odbc_test",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"use client_odbc_test",SQL_NTS);
    mystmt(hstmt,rc);


    rc = SQLExecDirect(hstmt,"select * from iNON_EXITING_TABLE",SQL_NTS);
    myassert(rc == SQL_ERROR);
    check_sqlstate(hdbc, hstmt,"S0002");

    SQLExecDirect(hstmt,"DROP TABLE test_error",SQL_NTS);
    rc = SQLExecDirect(hstmt,"CREATE TABLE test_error(id int)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"CREATE TABLE test_error(id int)",SQL_NTS);
    myassert(rc == SQL_ERROR);
    check_sqlstate(hdbc, hstmt,"S0001");

    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_FETCH_BOOKMARK_PTR,(SQLPOINTER)NULL,0);
    myassert(rc == SQL_ERROR);
    check_sqlstate(hdbc, hstmt,"S1C00");

    rc = SQLFreeStmt(hstmt,SQL_DROP);
    mystmt(hstmt,rc);

    rc = SQLDisconnect(hdbc);
    mycon(hdbc, rc);

    rc = SQLFreeHandle(SQL_HANDLE_DBC,hdbc);
    mycon(hdbc,rc);

    rc = SQLFreeHandle(SQL_HANDLE_ENV,henv);
    myenv(henv,rc);
}
/*---------------------------------------------------------------------
@ type       : MyODBC3 test main function
@ name       : main
@ purpose    : to control all tests
@ limitation : none
*---------------------------------------------------------------------*/
int main(int argc, char *argv[])
{  
    SQLINTEGER narg;      

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

    }   

    t_odbc2_error();
    t_odbc3_error();
    t_odbc2_error();

    printMessageFooter( 1 );

    return(0);
} 



