/***************************************************************************
                          my_connect.c  -  description
                             -------------------
    begin                : Wed Sep 8 2001
    copyright            : (C) MySQL AB 1995-2002, www.mysql.com
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

/***************************************************************************
 *                                                                         *
 *  This is a basic sample to demonstrate how to connect to MySQL server   *
 *  using MySQL ODBC 3.51 driver                                           *
 *                                                                         *
 ***************************************************************************/

#include "mytest3.h" /* MyODBC 3.51 sample utility header */

/********************************************************
* main routine                                          *
*********************************************************/
int main(int argc, char *argv[])
{
    SQLHENV    henv;
    SQLHDBC    hdbc; 
    SQLCHAR    server_name[30];  
    SQLRETURN  rc;
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

    printMessage( "[%s][%d]\n", __FILE__, __LINE__ );
    rc = SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&henv);
    myenv(henv, rc);

    printMessage( "[%s][%d]\n", __FILE__, __LINE__ );
    rc = SQLSetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0);
    myenv(henv, rc);

    printMessage( "[%s][%d]\n", __FILE__, __LINE__ );
    rc = SQLAllocHandle(SQL_HANDLE_DBC,henv, &hdbc);   
    myenv(henv, rc);

    printMessage( "[%s][%d]\n", __FILE__, __LINE__ );
    rc = SQLConnect(hdbc, mydsn, SQL_NTS, myuid, SQL_NTS,  mypwd, SQL_NTS);
    mycon(hdbc, rc);

    printMessage( "[%s][%d]\n", __FILE__, __LINE__ );
    rc = SQLGetInfo(hdbc,SQL_DBMS_NAME,&server_name,40,NULL);
    mycon(hdbc, rc);

    printMessage( "[%s][%d] SQL_DBMS_NAME=(%s)\n", __FILE__, __LINE__, server_name );
    rc = SQLDisconnect(hdbc); 
    mycon(hdbc, rc);

    rc = SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    mycon(hdbc, rc);

    rc = SQLFreeHandle(SQL_HANDLE_ENV, henv);
    myenv(henv, rc);

    printMessageFooter( 1 );

    return(0);  
} 



