/***************************************************************************
                          my_param.c  -  description
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
 *  This is a basic sample to demonstrate how to insert or delete or       *
 *  update data in the table using parameters                              *
 *                                                                         *
 ***************************************************************************/

#include "mytest3.h" /* MyODBC 3.51 sample utility header */

/********************************************************
* initialize tables                                     *
*********************************************************/
void my_init_table(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN   rc;

    /* drop table 'my_demo_param' if it already exists */
    printMessage(" creating table 'my_demo_param'\n");

    rc = SQLExecDirect(hstmt,"DROP TABLE if exists my_demo_param",SQL_NTS);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* create the table 'my_demo_param' */
    rc = SQLExecDirect(hstmt,"CREATE TABLE my_demo_param(\
                              id   int,\
                              auto int primary key auto_increment,\
                              name varchar(20),\
                              timestamp timestamp(14))",SQL_NTS);
    mystmt(hstmt,rc);

    /* commit the transaction*/
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);    
}

/********************************************************
* insert data using parameters                          *
*********************************************************/
void my_param_insert(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN   rc;
    long        id;
    char        name[50];

    /* prepare the insert statement with parameters */
    rc = SQLPrepare(hstmt,"INSERT INTO my_demo_param(id,name) VALUES(?,?)",SQL_NTS);
    mystmt(hstmt,rc);

    /* now supply data to parameter 1 and 2 */
    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, 
                          SQL_C_LONG, SQL_INTEGER, 0,0,
                          &id, 0, NULL);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, 
                          SQL_C_CHAR, SQL_CHAR, 0,0,
                          name, sizeof(name), NULL);
    mystmt(hstmt,rc);

    /* now insert 10 rows of data */
    for (id = 0; id < 10; id++)
    {
        sprintf(name,"MySQL%d",id);

        rc = SQLExecute(hstmt);
        mystmt(hstmt,rc);
    }

    /* Free statement param resorces */
    rc = SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    mystmt(hstmt,rc);

    /* Free statement cursor resorces */
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* Now fetch and verify the data */
    rc = SQLExecDirect(hstmt, "SELECT * FROM my_demo_param",SQL_NTS);
    mystmt(hstmt,rc);

    assert(10 == myresult(hstmt));
}

/********************************************************
* update data using parameters                          *
*********************************************************/
void my_param_update(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN   rc;
    SQLLEN nRowCount;
    SQLINTEGER id=9;
    char name[]="update";

    /* prepare the insert statement with parameters */
    rc = SQLPrepare(hstmt,"UPDATE my_demo_param set name = ? WHERE id = ?",SQL_NTS);
    mystmt(hstmt,rc);

    /* now supply data to parameter 1 and 2 */
    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT,
                          SQL_C_CHAR, SQL_CHAR, 0,0,
                          name, sizeof(name), NULL);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT,
                          SQL_C_LONG, SQL_INTEGER, 0,0,
                          &id, 0, NULL);
    mystmt(hstmt,rc);

    /* now execute the update statement */
    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);

    /* check the rows affected by the update statement */
    rc = SQLRowCount(hstmt, &nRowCount);
    mystmt(hstmt,rc);
    printMessage("\n total rows updated:%d\n",nRowCount);
    assert( nRowCount == 1);

    /* Free statement param resorces */
    rc = SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    mystmt(hstmt,rc);

    /* Free statement cursor resorces */
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* Now fetch and verify the data */
    rc = SQLExecDirect(hstmt, "SELECT * FROM my_demo_param",SQL_NTS);
    mystmt(hstmt,rc);

    assert(10 == myresult(hstmt));
}

/********************************************************
* delete data using parameters                          *
*********************************************************/
void my_param_delete(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN   rc;
    SQLLEN nRowCount;
    long id;

    /* supply data to parameter 1 */    
    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, 
                          SQL_C_LONG, SQL_INTEGER, 0,0,
                          &id, 0, NULL);
    mystmt(hstmt,rc);

    /* execute the DELETE STATEMENT to delete 5th row  */
    id = 5;
    rc = SQLExecDirect(hstmt,"DELETE FROM my_demo_param WHERE id = ?",SQL_NTS);
    mystmt(hstmt,rc);

    /* check the rows affected by the update statement */
    rc = SQLRowCount(hstmt, &nRowCount);
    mystmt(hstmt,rc);
    printMessage(" total rows deleted:%d\n",nRowCount);
    assert( nRowCount == 1);

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    /* execute the DELETE STATEMENT to delete 8th row  */
    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, 
                          SQL_C_LONG, SQL_INTEGER, 0,0,
                          &id, 0, NULL);
    mystmt(hstmt,rc);

    id = 8;
    rc = SQLExecDirect(hstmt,"DELETE FROM my_demo_param WHERE id = ?",SQL_NTS);
    mystmt(hstmt,rc);

    /* check the rows affected by the update statement */
    rc = SQLRowCount(hstmt, &nRowCount);
    mystmt(hstmt,rc);
    printMessage(" total rows deleted:%d\n",nRowCount);
    assert( nRowCount == 1);

    /* Free statement param resorces */
    rc = SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    mystmt(hstmt,rc);

    /* Free statement cursor resorces */
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* Now fetch and verify the data */
    rc = SQLExecDirect(hstmt, "SELECT * FROM my_demo_param",SQL_NTS);
    mystmt(hstmt,rc);

    assert(8 == myresult(hstmt));

    /* drop the table */
    rc = SQLExecDirect(hstmt,"DROP TABLE my_demo_param",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
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
     * initialize table
    */
    my_init_table(hdbc, hstmt);

    /* 
     * insert data using parameters
    */
    my_param_insert(hdbc, hstmt);

    /* 
     * parameter update 
    */
    my_param_update(hdbc, hstmt);

    /* 
     * parameter delete
    */
    my_param_delete(hdbc, hstmt);

    /* 
     * disconnect from the server, by freeing all resources
    */
    mydisconnect(&henv,&hdbc,&hstmt);

    printMessageFooter( 1 );

    return(0);
} 



