/***************************************************************************
                          my_basics.c  -  description
                             ---------------------
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
 *  This is a basic sample to demonstrate the basic execution of SQL       *
 *  statements using  MySQL ODBC 3.51 driver                               *
 *                                                                         *
 ***************************************************************************/

#include "mytest3.h" /* MyODBC 3.51 sample utility header */

/********************************************************
* Execution of BASIC SQL statements                     *
*********************************************************/
void my_basics(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN  rc;
    SQLINTEGER nRowCount;

    /* drop table 'myodbc3_demo_basic' if it already exists */
    rc = SQLExecDirect(hstmt,"DROP TABLE if exists myodbc3_demo_basic",SQL_NTS);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* create the table 'myodbc3_demo_result' */
    rc = SQLExecDirect(hstmt,"CREATE TABLE myodbc3_demo_basic(\
                              id int primary key auto_increment,\
                              name varchar(20))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);    

    /* insert 3 rows of data */    
    rc = SQLExecDirect(hstmt,"INSERT INTO myodbc3_demo_basic values(\
                              1,'MySQL')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"INSERT INTO myodbc3_demo_basic values(\
                              2,'MyODBC')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"INSERT INTO myodbc3_demo_basic values(\
                              3,'monty')",SQL_NTS);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* update second row */    
    rc = SQLExecDirect(hstmt,"UPDATE myodbc3_demo_basic set name=\
                              'MyODBC 3.51' where id=2",SQL_NTS);
    mystmt(hstmt,rc);

    /* get the rows affected by update statement */
    rc = SQLRowCount(hstmt,&nRowCount);
    mystmt(hstmt,rc);
    printMessage( " total rows updated:%d", nRowCount );

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* delete third column */
    rc = SQLExecDirect(hstmt,"DELETE FROM myodbc3_demo_basic where id = 3",SQL_NTS);
    mystmt(hstmt,rc);

    /* get the rows affected by delete statement */
    rc = SQLRowCount(hstmt,&nRowCount);
    mystmt(hstmt,rc);
    printMessage(" total rows deleted:%d",nRowCount);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* alter the table 'myodbc3_demo_basic' to 'myodbc3_new_name' */
    rc = SQLExecDirect(hstmt,"ALTER TABLE myodbc3_demo_basic RENAME myodbc3_new_name",SQL_NTS);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* drop the table with the original table name, and it should
       return error saying 'table not found'
    */
    rc = SQLExecDirect(hstmt,"DROP TABLE myodbc3_demo_basic",SQL_NTS);
    /* mystmt_err(hstmt, rc == SQL_ERROR, rc); */

    /* now drop the table, which is altered..*/   
    rc = SQLExecDirect(hstmt,"DROP TABLE myodbc3_new_name",SQL_NTS);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* free the statement cursor */
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);
}
/********************************************************
* main routine                                          *
*********************************************************/
int main(int argc, char *argv[])
{
    SQLHENV    henv;
    SQLHDBC    hdbc; 
    SQLHSTMT   hstmt;
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

    /* 
     * simple execution of SQL statements
    */
    my_basics(hdbc, hstmt);

    /* 
     * disconnect from the server, by freeing all resources
    */
    mydisconnect(&henv,&hdbc,&hstmt);

    printMessageFooter( 1 );

    return(0);
} 



