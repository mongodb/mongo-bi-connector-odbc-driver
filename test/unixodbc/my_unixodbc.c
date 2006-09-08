/***************************************************************************
                          my_unixodbc.c  -  description
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

void t_odbc3_envattr()
{
    SQLRETURN rc; 
    SQLHENV henv;
    SQLHDBC hdbc;
    SQLPOINTER ov_version;

    rc = SQLAllocEnv(&henv);
    myenv(henv,rc);

    rc = SQLGetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,&ov_version,0,0);
    myenv(henv,rc);
    printMessage("\ndefault odbc version:%d",ov_version);

    rc = SQLSetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0);
    myenv(henv,rc);
    my_assert(ov_version == (SQLPOINTER)SQL_OV_ODBC2);

    rc = SQLAllocConnect(henv,&hdbc);
    myenv(henv,rc);

    rc = SQLFreeConnect(hdbc);
    mycon(hdbc,rc);

    rc = SQLSetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0);
    myenv(henv,rc);

    rc = SQLGetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,&ov_version,0,0);
    myenv(henv,rc);
    printMessage("\nnew odbc version:%d",ov_version);
    my_assert(ov_version == (SQLPOINTER)SQL_OV_ODBC3);

    rc = SQLFreeEnv(henv);
    myenv(henv,rc);
}
void t_odbc3_handle()
{
    SQLRETURN rc; 
    SQLHENV henv;
    SQLHDBC hdbc;
    SQLHSTMT hstmt;
    SQLPOINTER ov_version;

    rc = SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&henv);
    myenv(henv,rc);

#if 0
    rc = SQLAllocHandle(SQL_HANDLE_DBC,henv,&hdbc);
    myenv_err(henv,rc == SQL_ERROR,rc);

    rc = SQLGetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,&ov_version,0,0);
    myenv(henv,rc);
    printMessage("\ndefault odbc version:%d",ov_version);
    my_assert(ov_version == (SQLPOINTER)SQL_OV_ODBC2);
#endif

    rc = SQLSetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0);
    myenv(henv,rc);

    rc = SQLGetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,&ov_version,0,0);
    myenv(henv,rc);
    my_assert(ov_version == (SQLPOINTER)SQL_OV_ODBC3);

    rc = SQLAllocHandle(SQL_HANDLE_DBC,henv,&hdbc);
    myenv(henv,rc);

    rc = SQLConnect(hdbc, mydsn, SQL_NTS, myuid, SQL_NTS,  mypwd, SQL_NTS);
    mycon(hdbc,rc);

    rc = SQLAllocHandle(SQL_HANDLE_STMT,hdbc,&hstmt);
    mycon(hdbc, rc);

    rc = SQLDisconnect(hdbc);
    mycon(hdbc, rc);

    rc = SQLFreeHandle(SQL_HANDLE_DBC,hdbc);
    mycon(hdbc,rc);

    rc = SQLFreeHandle(SQL_HANDLE_ENV,henv);
    myenv(henv,rc);
}
void t_driver_connect()
{
    SQLRETURN rc; 
    SQLHENV henv;
    SQLHDBC hdbc;
    SQLCHAR conn_in[255], conn_out[255];

    rc = SQLAllocEnv(&henv);
    myenv(henv,rc);

    rc = SQLAllocConnect(henv,&hdbc);
    myenv(henv,rc);

    sprintf(conn_in,"DRIVER={MySQL ODBC 3.51 Driver};USER=%s;PASSWORD=%s;DATABASE=%s;SERVER=%s;OPTION=3;SOCKET=/tmp/mysql.sock;STMT=use mysql;",
            myuid, mypwd, mydb, myserver);

    rc = SQLDriverConnect(hdbc, (SQLHWND)0, (SQLCHAR *)conn_in, sizeof(conn_in),
                          (SQLCHAR *)conn_out, sizeof(conn_out), 0, 
                          SQL_DRIVER_NOPROMPT);

    if (rc == SQL_SUCCESS)
    {
        mycon(hdbc,rc);
        printMessage( "output string: `%s`", conn_out); 

        rc = SQLDisconnect(hdbc);
        mycon(hdbc,rc);
    }

    rc = SQLFreeConnect(hdbc);
    mycon(hdbc,rc);

    rc = SQLFreeEnv(henv);
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

    t_odbc3_envattr();
    t_odbc3_handle();
    t_driver_connect();

    printMessageFooter( 1 );

    return(0);
} 



