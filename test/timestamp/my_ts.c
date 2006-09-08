/***************************************************************************
                          my_ts.c  -  description
                             ---------------------
    begin                : Mon Jan 7, 2002
    copyright            : (C) Copyright © MySQL AB 1995-2002
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

/********************************************************
* timestamp demo..
*********************************************************/
void my_ts(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN        rc;
    SQLCHAR          szTs[50];
    TIMESTAMP_STRUCT ts;
    SQLINTEGER       pclen;

    /* drop table 'myodbc3_demo_result' if it already exists */
    SQLExecDirect(hstmt,"DROP TABLE my_ts",SQL_NTS);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* create the table 'myodbc3_demo_result' */
    rc = SQLExecDirect(hstmt,"CREATE TABLE my_ts(ts timestamp)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);    

    /* insert using SQL_C_CHAR to SQL_TIMESTAMP */    
    strcpy(szTs,"2002-01-07 10:20:49.06");
    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,
                          SQL_C_CHAR,SQL_TIMESTAMP,
                          0,0,&szTs,sizeof(szTs),NULL);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"INSERT INTO my_ts(ts) values(?)",SQL_NTS);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt,SQL_CLOSE);


    /* insert using SQL_C_TIMESTAMP to SQL_TIMESTAMP */    
    ts.year = 2002;
    ts.month=01;
    ts.day=07;
    ts.hour=19;
    ts.minute=47;
    ts.second=59;
    ts.fraction=04;
    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,
                          SQL_C_TIMESTAMP,SQL_TIMESTAMP,
                          0,0,&ts,sizeof(ts),NULL);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"INSERT INTO my_ts(ts) values(?)",SQL_NTS);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    strcpy(szTs,"");
    /* now fetch and verify the results .. */        
    rc = SQLExecDirect(hstmt,"SELECT * from my_ts",SQL_NTS);
    mystmt(hstmt,rc);

    /* now fetch first row */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_ABSOLUTE,1);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szTs,sizeof(szTs),&pclen);
    mystmt(hstmt,rc);
    printMessage("\n row1 using SQL_C_CHAR:%s(%d)\n",szTs,pclen);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_ABSOLUTE,1);
    mystmt(hstmt,rc);
    ts.fraction= 0;

    rc = SQLGetData(hstmt,1,SQL_C_TIMESTAMP,&ts,sizeof(ts),&pclen);
    mystmt(hstmt,rc);
    printMessage("\n row1 using SQL_C_TIMESTAMP:%d-%d-%d %d:%d:%d.%d(%d)\n",
           ts.year,ts.month,
           ts.day,ts.hour,ts.minute,
           ts.second,ts.fraction,pclen);


    /* now fetch second row */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_ABSOLUTE,2);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szTs,sizeof(szTs),&pclen);
    mystmt(hstmt,rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_ABSOLUTE,2);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt,1,SQL_C_TIMESTAMP,&ts,sizeof(ts),&pclen);
    mystmt(hstmt,rc);
    printMessage("\n row2 using SQL_C_TIMESTAMP:%d-%d-%d %d:%d:%d.%d(%d)\n",
           ts.year,ts.month,
           ts.day,ts.hour,ts.minute,
           ts.second,ts.fraction,pclen);


    rc = SQLFetchScroll(hstmt,SQL_FETCH_ABSOLUTE,3);
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);
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
     * simple timestamp conversion demo
    */
    my_ts(hdbc, hstmt);

    /* 
     * disconnect from the server, by freeing all resources
    */
    mydisconnect(&henv,&hdbc,&hstmt);

    printMessageFooter( 1 );

    return(0);
} 



