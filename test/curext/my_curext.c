/***************************************************************************
                          my_curext.c  -  description
                             -------------------
    begin                : Tue Feb 05 2002
    copyright            : (C) MySQL AB 1995-2002
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


/**
* to test the pcbValue on cursor ops
**/
void my_pcbValue(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN   rc;
    SQLROWCOUNT nRowCount;
    SQLINTEGER  nData = 500, int_pcbValue, pcbValue, pcbValue1, pcbValue2;
    SQLCHAR     szData[255]={0};

    /* initialize data */
    SQLExecDirect(hstmt,"drop table my_pcbValue",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table my_pcbValue(id int, name varchar(30),\
                                                       name1 varchar(30),\
                                                       name2 varchar(30))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into my_pcbValue(id,name) values(100,'venu')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into my_pcbValue(id,name) values(200,'monty')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc); 

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,0,&int_pcbValue);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,15,&pcbValue);
    mystmt(hstmt,rc);        

    rc = SQLBindCol(hstmt,3,SQL_C_CHAR,szData,3,&pcbValue1);
    mystmt(hstmt,rc);        

    rc = SQLBindCol(hstmt,4,SQL_C_CHAR,szData,2,&pcbValue2);
    mystmt(hstmt,rc);    

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC, 0);
    mystmt(hstmt, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY ,(SQLPOINTER)SQL_CONCUR_ROWVER , 0);
    mystmt(hstmt, rc);  

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE  ,(SQLPOINTER)1 , 0);
    mystmt(hstmt, rc);  

    /* Open the resultset of table 'my_demo_cursor' */
    rc = SQLExecDirect(hstmt,"SELECT * FROM my_pcbValue",SQL_NTS);
    mystmt(hstmt,rc);

    /* goto the last row */
    rc = SQLFetchScroll(hstmt, SQL_FETCH_LAST, 1L);
    mystmt(hstmt,rc);

    /* Now delete the newly updated record */
    strcpy((char*)szData,"updated");
    nData = 99999;

    int_pcbValue=2;
    pcbValue=3;
    pcbValue1=9;
    pcbValue2=SQL_NTS;

    rc = SQLSetPos(hstmt,1,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt, &nRowCount);
    mystmt(hstmt, rc);

    printMessage(" total rows updated:%d\n",nRowCount);
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
    rc = SQLExecDirect(hstmt, "SELECT * FROM my_pcbValue",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt,1,SQL_C_LONG,&nData,0,NULL);
    mystmt(hstmt,rc);
    printMessage("\n nData :%d",nData);
    myassert(nData == 99999);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,50,NULL);
    mystmt(hstmt,rc);
    printMessage("\n szData:%s\n",szData);
    myassert(strcmp(szData,"upd") == 0);    

    rc = SQLGetData(hstmt,3,SQL_C_CHAR,szData,50,NULL);
    mystmt(hstmt,rc);
    printMessage("\n szData:%s\n",szData);
    myassert(strcmp(szData,"updated") == 0);

    rc = SQLGetData(hstmt,4,SQL_C_CHAR,szData,50,NULL);
    mystmt(hstmt,rc);
    printMessage("\n szData:%s\n",szData);
    myassert(strcmp(szData,"updated") == 0);

    rc = SQLFetch(hstmt);
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);    

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);
}

/**
* to test the pcbValue on cursor ops
**/
void my_pcbValue_add(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN   rc;
    SQLROWCOUNT nRowCount;
    SQLINTEGER  nData = 500, int_pcbValue, pcbValue, pcbValue1, pcbValue2;
    SQLCHAR     szData[255]={0};

    /* initialize data */
    SQLExecDirect(hstmt,"drop table my_pcbValue_add",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table my_pcbValue_add(id int, name varchar(30),\
                                                       name1 varchar(30),\
                                                       name2 varchar(30))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into my_pcbValue_add(id,name) values(100,'venu')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into my_pcbValue_add(id,name) values(200,'monty')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc); 

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,0,&int_pcbValue);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,15,&pcbValue);
    mystmt(hstmt,rc);        

    rc = SQLBindCol(hstmt,3,SQL_C_CHAR,szData,3,&pcbValue1);
    mystmt(hstmt,rc);        

    rc = SQLBindCol(hstmt,4,SQL_C_CHAR,szData,2,&pcbValue2);
    mystmt(hstmt,rc);    

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC, 0);
    mystmt(hstmt, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY ,(SQLPOINTER)SQL_CONCUR_ROWVER , 0);
    mystmt(hstmt, rc);  

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE  ,(SQLPOINTER)1 , 0);
    mystmt(hstmt, rc);  

    /* Open the resultset of table 'my_pcbValue_add' */
    rc = SQLExecDirect(hstmt,"SELECT * FROM my_pcbValue_add",SQL_NTS);
    mystmt(hstmt,rc);

    /* goto the last row */
    rc = SQLFetchScroll(hstmt, SQL_FETCH_LAST, 1L);
    mystmt(hstmt,rc);

    /* Now delete the newly updated record */
    strcpy((char*)szData,"inserted");
    nData = 99999;

    int_pcbValue=2;
    pcbValue=3;
    pcbValue1=6;
    pcbValue2=SQL_NTS;

    rc = SQLSetPos(hstmt,1,SQL_ADD,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt, &nRowCount);
    mystmt(hstmt, rc);

    printMessage(" total rows updated:%d\n",nRowCount);
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
    rc = SQLExecDirect(hstmt, "SELECT * FROM my_pcbValue_add",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);    

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt,1,SQL_C_LONG,&nData,0,NULL);
    mystmt(hstmt,rc);
    printMessage("\n nData :%d",nData);
    myassert(nData == 99999);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,50,NULL);
    mystmt(hstmt,rc);
    printMessage("\n szData:%s\n",szData);
    myassert(strcmp(szData,"ins") == 0);    

    rc = SQLGetData(hstmt,3,SQL_C_CHAR,szData,50,NULL);
    mystmt(hstmt,rc);
    printMessage("\n szData:%s\n",szData);
    myassert(strcmp(szData,"insert") == 0);

    rc = SQLGetData(hstmt,4,SQL_C_CHAR,szData,50,NULL);
    mystmt(hstmt,rc);
    printMessage("\n szData:%s\n",szData);
    myassert(strcmp(szData,"inserted") == 0);

    rc = SQLFetch(hstmt);
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);    

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);
}


/**
* spaces in column names
**/
void my_columnspace(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN   rc;

    /* initialize data */
    rc = SQLExecDirect(hstmt,"DROP TABLE IF EXISTS TestColNames",SQL_NTS);   
    mystmt(hstmt,rc);   

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLExecDirect(hstmt,"CREATE TABLE `TestColNames`(`Value One` text,\
                                                           `Value Two` text,\
                                                           `Value Three` text)",SQL_NTS);
    mystmt(hstmt,rc);   

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLExecDirect(hstmt,"INSERT INTO TestColNames VALUES ('venu','anuganti','mysql ab')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"INSERT INTO TestColNames VALUES ('monty','widenius','mysql ab')",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);    

    rc = SQLExecDirect(hstmt,"SELECT * FROM `TestColNames`",SQL_NTS);
    mystmt(hstmt,rc);

    myassert(2 == my_print_non_format_result(hstmt));    

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);    

    rc = SQLExecDirect(hstmt,"SELECT `Value One`,`Value Two`,`Value Three`  FROM `TestColNames`",SQL_NTS);
    mystmt(hstmt,rc);

    myassert(2 == my_print_non_format_result(hstmt));    

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);    
}


/**
* to test the empty string returning NO_DATA
**/
void my_empty_string(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN   rc;
    SQLINTEGER  pcbValue;
    SQLCHAR     szData[255]={0};

    /* initialize data */
    SQLExecDirect(hstmt,"drop table my_empty_string",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table my_empty_string(name varchar(30))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into my_empty_string values('')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    /* Now fetch and verify the data */
    rc = SQLExecDirect(hstmt, "SELECT * FROM my_empty_string",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,50,&pcbValue);
    mystmt(hstmt,rc);
    printMessage("\n szData:%s(%d)\n",szData,pcbValue);

    rc = SQLFetch(hstmt);
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);    

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);
}


/**
MAIN ROUTINE...
*/
int main(int argc, char *argv[])
{
    SQLHENV   henv;
    SQLHDBC   hdbc;
    SQLHSTMT  hstmt;
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

    myconnect(&henv,&hdbc,&hstmt);  

    if (driver_supports_setpos(hdbc))
    {
        my_pcbValue(hdbc,hstmt);
        my_pcbValue_add(hdbc,hstmt);
    }
    my_columnspace(hdbc,hstmt);
    my_empty_string(hdbc,hstmt);

    mydisconnect(&henv,&hdbc,&hstmt);


    printMessageFooter( 1 );

    return(0);
}
