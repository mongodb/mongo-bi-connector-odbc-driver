/***************************************************************************
                          my_tran.c  -  description
                             -------------------
    begin                : Wed Aug 8 2001
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

#include "../include/mytest3.h" /* MyODBC 3.51 sample utility header */


/********************************************************
* Transactional behaviour using BDB/InnoDB table type   *
*********************************************************/
void my_transaction(SQLHDBC hdbc, SQLHSTMT hstmt, SQLHENV henv, SQLHDBC hdbc2, SQLHSTMT hstmt2, SQLHENV henv2)
{
    SQLRETURN rc, rc2;

    if (!server_supports_trans(hdbc))
        return;

    /* set AUTOCOMMIT to OFF */
    rc = SQLSetConnectAttr(hdbc,SQL_ATTR_AUTOCOMMIT,(SQLPOINTER)SQL_AUTOCOMMIT_OFF,0);
    mycon(hdbc,rc);

    rc = SQLExecDirect(hstmt,"DROP TABLE IF EXISTS my_demo_transaction",SQL_NTS);    
    mystmt(hstmt,rc);   

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    /* create the table 'mytran_demo' of type BDB' or 'InnoDB' */
    rc = SQLExecDirect(hstmt,"CREATE TABLE my_demo_transaction(col1 int ,col2 varchar(30)) TYPE = InnoDB",SQL_NTS);
    mystmt(hstmt,rc);   

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    /* insert a row and commit the transaction */
    rc = SQLExecDirect(hstmt,"INSERT INTO my_demo_transaction VALUES(10,'venu')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    /* now insert the second row, and rollback the transaction */
    rc = SQLExecDirect(hstmt,"INSERT INTO my_demo_transaction VALUES(20,'mysql')",SQL_NTS);
    mystmt(hstmt,rc); 

    rc = SQLTransact(NULL,hdbc,SQL_ROLLBACK);
    mycon(hdbc,rc);

    /* delete first row, and rollback it */
    rc = SQLExecDirect(hstmt,"DELETE FROM my_demo_transaction WHERE col1 = 10",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_ROLLBACK);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  

    /* test the results now, only one row should exists */
    rc = SQLExecDirect(hstmt,"SELECT * FROM my_demo_transaction",SQL_NTS);
    mystmt(hstmt,rc);  

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);  

    rc = SQLFetch(hstmt);
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND,rc);  

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);


    /* now insert some more records to check SQLEndTran */
    rc = SQLExecDirect(hstmt,"INSERT INTO my_demo_transaction VALUES(30,'test'),(40,'transaction')",SQL_NTS);
    mystmt(hstmt,rc); 

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

	rc = SQLExecDirect(hstmt,"DELETE FROM my_demo_transaction WHERE col1 = 30",SQL_NTS);
    mystmt(hstmt,rc); 

	/* Commit the transaction using DBC handler */
	rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);

    /* test the results now, select should not find any data */
    rc2 = SQLExecDirect(hstmt2,"SELECT * FROM my_demo_transaction WHERE col1 = 30",SQL_NTS);
    mystmt(hstmt2,rc2);  

    rc2 = SQLFetch(hstmt2);
    mystmt_err(hstmt2,rc2 == SQL_NO_DATA_FOUND,rc2);  

    rc2 = SQLFreeStmt(hstmt2,SQL_CLOSE);
    mystmt(hstmt2,rc2);

	/* Delete a row to check  */
	rc = SQLExecDirect(hstmt,"DELETE FROM my_demo_transaction WHERE col1 = 40",SQL_NTS);
    mystmt(hstmt,rc); 

	/* Commit the transaction using DBC handler */
	rc = SQLEndTran(SQL_HANDLE_ENV, henv, SQL_COMMIT);
    mycon(hdbc,rc);

    /* test the results now, select should not find any data */
    rc2 = SQLExecDirect(hstmt2,"SELECT * FROM my_demo_transaction WHERE col1 = 40",SQL_NTS);
    mystmt(hstmt2,rc2);  

    rc2 = SQLFetch(hstmt2);
    mystmt_err(hstmt2,rc2 == SQL_NO_DATA_FOUND,rc2);  

    rc2 = SQLFreeStmt(hstmt2,SQL_CLOSE);
    mystmt(hstmt2,rc2);

	/* drop the table */
    rc = SQLExecDirect(hstmt,"DROP TABLE my_demo_transaction",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_ROLLBACK);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  

}

/********************************************************
* main routine                                          *
*********************************************************/
int main(int argc, char *argv[])
{
    SQLHENV   henv, henv2;
    SQLHDBC   hdbc, hdbc2;
    SQLHSTMT  hstmt, hstmt2;
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

    /* 
     * connect to MySQL server
    */
    myconnect(&henv,&hdbc,&hstmt); 

	// create another connection to check whether transactions were committed/rolled back
    myconnect(&henv2,&hdbc2,&hstmt2); 

    /* 
     * simple transaction test
    */
	my_transaction(hdbc, hstmt, henv, hdbc2, hstmt2, henv2);

    /* 
     * disconnect from the server, by freeing all resources
    */
    mydisconnect(&henv,&hdbc,&hstmt);

    printMessageFooter( 1 );

    return(0);
} 



