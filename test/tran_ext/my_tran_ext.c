/***************************************************************************
                          my_tran_ext.c  -  description
                             -------------------
    begin                : Sat Sep 28 2002
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
 *  This is a basic sample to demonstrate the transaction support in       *
 *  MySQL using  MySQL ODBC 3.51 driver                                    *
 *                                                                         *
 ***************************************************************************/

#include "mytest3.h" /* MyODBC 3.51 sample utility header */

SQLCHAR *mysock= NULL;

/********************************************************
* Transactional behaviour before and the connection     *
*********************************************************/
void test_tran_ext(SQLHDBC hdbc)
{
    SQLRETURN  rc;
    SQLCHAR    conn[255];
    SQLINTEGER option = 131072L *2 + 4;/* No Transactions */

    /* set AUTOCOMMIT to OFF */
    rc = SQLSetConnectAttr(hdbc,SQL_ATTR_AUTOCOMMIT,(SQLPOINTER)SQL_AUTOCOMMIT_OFF,0);
    mycon(hdbc,rc);

    /* TXN_ISOLATION */
    rc = SQLSetConnectAttr(hdbc,SQL_TXN_ISOLATION,(SQLPOINTER)SQL_TXN_REPEATABLE_READ,0);
    mycon(hdbc,rc);    

    sprintf(conn,"DSN=%s;UID=%s;PWD=%s;OPTION=%d",
            mydsn,myuid,mypwd,option);
    if (mysock != NULL)
    {
      strcat(conn, ";SOCKET=");
      strcat(conn, mysock);
    }
    rc = SQLDriverConnect(hdbc,NULL,conn,255,
                          NULL,0,NULL,SQL_DRIVER_NOPROMPT);
    mycon(hdbc,rc);

    rc = SQLDisconnect(hdbc);
    mycon(hdbc,rc);    

    option = 1 + 4;
    sprintf(conn,"DSN=%s;UID=%s;PWD=%s;OPTION=%d",
            mydsn,myuid,mypwd,option);
    if (mysock != NULL)
    {
      strcat(conn, ";SOCKET=");
      strcat(conn, mysock);
    }
    rc = SQLDriverConnect(hdbc,NULL,conn,255,
                          NULL,0,NULL,SQL_DRIVER_NOPROMPT);
    mycon(hdbc,rc);

    rc = SQLDisconnect(hdbc);
    mycon(hdbc,rc);    

    printf(" success!!\n");
}
/********************************************************
* main routine                                          *
*********************************************************/
int main(int argc, char *argv[])
{
    SQLHENV   henv;
    SQLHDBC   hdbc;
    SQLINTEGER narg;      
    SQLRETURN  rc;

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
    rc = SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&henv);
    myenv(henv,rc);   

    rc = SQLSetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0);
    myenv(henv,rc);   

    rc = SQLAllocHandle(SQL_HANDLE_DBC,henv, &hdbc);
    myenv(henv,rc);    

    test_tran_ext(hdbc);

    SQLFreeHandle(SQL_HANDLE_DBC,hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV,henv);

    printMessageFooter( 1 );

    return(0);
} 



