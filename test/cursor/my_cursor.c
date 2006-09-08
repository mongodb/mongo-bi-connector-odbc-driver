/***************************************************************************
                          my_cursor.c  -  description
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
 *  This is a basic sample to demonstrate how to perform positioned        *
 *  update and deletes using cursors                                       *
 *                                                                         *
 ***************************************************************************/

#include "mytest3.h" /* MyODBC 3.51 sample utility header */

/********************************************************
* initialize tables                                     *
*********************************************************/
void my_init_table(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN   rc;
    SQLINTEGER  id;
    SQLCHAR     name[50];

    /* drop table 'my_demo_param' if it already exists */
    printMessage(" creating table 'my_demo_cursor'\n");

    rc = SQLExecDirect(hstmt,"DROP TABLE if exists my_demo_cursor",SQL_NTS);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* create the table 'my_demo_param' */
    rc = SQLExecDirect(hstmt,"CREATE TABLE my_demo_cursor(\
                              id int, name varchar(20))",SQL_NTS);
    mystmt(hstmt,rc);

    /* commit the transaction*/
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);    

    /* prepare the insert statement with parameters */
    rc = SQLPrepare(hstmt,"INSERT INTO my_demo_cursor VALUES(?,?)",SQL_NTS);
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

    /* now insert 5 rows of data */
    for (id = 0; id < 5; id++)
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
    rc = SQLExecDirect(hstmt, "SELECT * FROM my_demo_cursor",SQL_NTS);
    mystmt(hstmt,rc);

    assert(5 == myresult(hstmt));
}

/********************************************************
* perform positioned update and delete                  *
*********************************************************/
void my_positioned_cursor(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN   rc;
    SQLINTEGER  nRowCount;
    SQLHSTMT    hstmt_pos;

    /* create new statement handle */
    rc = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt_pos);
    mycon(hdbc, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE,
                        (SQLPOINTER)SQL_CURSOR_DYNAMIC,0);
    mystmt(hstmt, rc);

    /* set the cursor name as 'mysqlcur' on hstmt */
    rc = SQLSetCursorName(hstmt, "mysqlcur", SQL_NTS);
    mystmt(hstmt, rc);

    /* Open the resultset of table 'my_demo_cursor' */
    rc = SQLExecDirect(hstmt,"SELECT * FROM my_demo_cursor",SQL_NTS);
    mystmt(hstmt,rc);

    /* goto the last row */
    rc = SQLFetchScroll(hstmt, SQL_FETCH_LAST, 1L);
    mystmt(hstmt,rc);

    /* now update the name field to 'update' using positioned cursor */
    rc = SQLExecDirect(hstmt_pos, "UPDATE my_demo_cursor SET name='updated' WHERE CURRENT OF mysqlcur", SQL_NTS);
    mystmt(hstmt_pos, rc);

    rc = SQLRowCount(hstmt_pos, &nRowCount);
    mystmt(hstmt_pos, rc);

    printMessage(" total rows updated:%d\n",nRowCount);
    assert(nRowCount == 1);

    /* Free statement cursor resorces */
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt_pos, SQL_CLOSE);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* Now delete 2nd row */
    rc = SQLExecDirect(hstmt,"SELECT * FROM my_demo_cursor",SQL_NTS);
    mystmt(hstmt,rc);

    /* goto the second row row */
    rc = SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, 2L);
    mystmt(hstmt,rc);

    /* now delete the current row */
    rc = SQLExecDirect(hstmt_pos, "DELETE FROM my_demo_cursor WHERE CURRENT OF mysqlcur", SQL_NTS);
    mystmt(hstmt_pos, rc);

    rc = SQLRowCount(hstmt_pos, &nRowCount);
    mystmt(hstmt_pos, rc);

    printMessage(" total rows deleted:%d\n",nRowCount);
    assert(nRowCount == 1);    

    /* free the statement cursor */
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    /* Free the statement 'hstmt_pos' */
    rc = SQLFreeHandle(SQL_HANDLE_STMT, hstmt_pos);
    mystmt(hstmt_pos,rc);

    /* Now fetch and verify the data */
    rc = SQLExecDirect(hstmt, "SELECT * FROM my_demo_cursor",SQL_NTS);
    mystmt(hstmt,rc);

    assert(4 == myresult(hstmt));
}

/********************************************************
* perform delete and update using SQLSetPos             *
*********************************************************/
void my_setpos_cursor(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN   rc;
    SQLINTEGER  id, nRowCount;
    SQLCHAR     name[50];

    printMessage("\nmy_setpos_cursor:\n");

    /* Open the resultset of table 'my_demo_cursor' */
    rc = SQLExecDirect(hstmt,"SELECT * FROM my_demo_cursor",SQL_NTS);
    mystmt(hstmt,rc);

    /* bind row data buffers */
    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&id,0,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,name,sizeof(name),NULL);
    mystmt(hstmt,rc);

    /* goto the first row */
    rc = SQLFetchScroll(hstmt, SQL_FETCH_FIRST, 1L);
    mystmt(hstmt,rc);

    strcpy(name,"first-row");

    /* now update the name field to 'first-row' using SQLSetPos */    
    rc = SQLSetPos(hstmt, 1, SQL_UPDATE, SQL_LOCK_NO_CHANGE);
    mystmt(hstmt, rc);

    rc = SQLRowCount(hstmt, &nRowCount);
    mystmt(hstmt, rc);

    printMessage(" total rows updated:%d\n",nRowCount);
    assert(nRowCount == 1);

    /* position to second row and delete it ..*/    
    rc = SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, 2L);
    mystmt(hstmt,rc);

    /* now delete the current, second row */
    rc = SQLSetPos(hstmt, 1, SQL_DELETE, SQL_LOCK_NO_CHANGE);
    mystmt(hstmt, rc);

    rc = SQLRowCount(hstmt, &nRowCount);
    mystmt(hstmt, rc);

    printMessage(" total rows deleted:%d\n",nRowCount);
    assert(nRowCount == 1);

    /* Free statement cursor resorces */
    rc = SQLFreeStmt(hstmt, SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* Now fetch and verify the data */
    rc = SQLExecDirect(hstmt, "SELECT * FROM my_demo_cursor",SQL_NTS);
    mystmt(hstmt,rc);

    assert(3 == myresult(hstmt));

    /* drop the table */
    rc = SQLExecDirect(hstmt,"DROP TABLE my_demo_cursor",SQL_NTS);
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

    if (driver_supports_positioned_ops(hdbc))
    {

        /* 
         * initialize table
        */
        my_init_table(hdbc, hstmt);

        /* 
         * positioned cursor update and delete
        */
        my_positioned_cursor(hdbc, hstmt);

        /* 
         * Update and Delete using SQLSetPos
        */
        my_setpos_cursor(hdbc, hstmt);
    }

    /* 
     * disconnect from the server, by freeing all resources
    */
    mydisconnect(&henv,&hdbc,&hstmt);

    printMessageFooter( 1 );

    return(0);
} 



