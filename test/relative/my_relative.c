/***************************************************************************
                          my_relative.c  -  description
                             -------------------
    begin                : Fri Sep 29 2001
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

/**
Testing basic scrolling feature
*/
void t_scroll(SQLHDBC hdbc,SQLHSTMT hstmt)
{
    SQLRETURN rc;
    long      i;

    SQLExecDirect(hstmt,"drop table t_scroll",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table t_scroll(col1 int)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"insert into t_scroll values(?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT, SQL_C_ULONG,
                          SQL_INTEGER,0,0,&i,0,NULL);
    mystmt(hstmt,rc);

    for ( i = 1; i <= 5; i++ )
    {
        rc = SQLExecute(hstmt);
        mystmt(hstmt,rc);
    }

    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLEndTran(SQL_HANDLE_DBC,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLExecDirect(hstmt,"select * from t_scroll",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&i,0,NULL);
    mystmt(hstmt,rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_LAST,0);/* 5 */
    mystmt(hstmt,rc);
    my_assert(i == 5);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_PREV,0);/* 4 */
    mystmt(hstmt,rc);
    my_assert(i == 4);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-3);/* 1 */
    mystmt(hstmt,rc);
    my_assert(i == 1);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-1);/* 0 */
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_PREV,1); /* 0 */
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_FIRST,-1);/* 0 */
    mystmt(hstmt,rc);
    my_assert(i == 1);    

    rc = SQLFetchScroll(hstmt,SQL_FETCH_ABSOLUTE,4);/* 4 */
    mystmt(hstmt,rc);
    my_assert(i == 4);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,2);/* 4 */
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_PREV,2);/* last */
    mystmt(hstmt,rc);
    my_assert(i == 5);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_NEXT,2);/* last+1 */
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_ABSOLUTE,-7);/* 0 */
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);     

    rc = SQLFetchScroll(hstmt,SQL_FETCH_FIRST,2);/* 1 */
    mystmt(hstmt,rc);
    my_assert(i == 1);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_PREV,2);/* 0 */
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_NEXT,0);/* 1*/
    mystmt(hstmt,rc);
    my_assert(i == 1);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_PREV,0);/* 0 */
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-1); /* 0 */
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,1); /* 1 */
    mystmt(hstmt,rc);
    my_assert(i == 1);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-1);/* 0 */
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,1);/* 1 */
    mystmt(hstmt,rc);
    my_assert(i == 1);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,1);/* 2 */
    mystmt(hstmt,rc);
    my_assert(i == 2);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-2);/* 0 */
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,6);/* last+1 */
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_PREV,6);/* last+1 */
    mystmt(hstmt, rc);
    my_assert(i == 5);

    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt,SQL_UNBIND);    
    SQLFreeStmt(hstmt,SQL_CLOSE);
}
/**
Testing SQL_FETCH_RELATIVE with row_set_size as 10
*/
void t_relative(SQLHDBC hdbc,SQLHSTMT hstmt)
{
    SQLRETURN rc;
    SQLINTEGER nrows, iarray[15];
    SQLUINTEGER i;
    long      index;
    char      name[21];

    SQLExecDirect(hstmt,"drop table t_relative",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table t_relative(id int,name char(20))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"insert into t_relative values(?,?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT, SQL_C_ULONG,
                          SQL_INTEGER,0,0,&i,0,NULL);
    mystmt(hstmt,rc);
    rc = SQLBindParameter(hstmt,2,SQL_PARAM_INPUT, SQL_C_CHAR,
                          SQL_CHAR,20,0,name,20,NULL);
    mystmt(hstmt,rc);

    for ( i = 1; i <= 50; i++ )
    {
        sprintf(name,"my%d",i);
        rc = SQLExecute(hstmt);
        mystmt(hstmt,rc);
    }

    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLEndTran(SQL_HANDLE_DBC,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    /* set row size as 10 */
    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROW_ARRAY_SIZE,(SQLPOINTER)10,0);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROWS_FETCHED_PTR,&nrows,0);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"select * from t_relative",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&iarray,0,NULL);
    mystmt(hstmt,rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_NEXT,0);/* 1-10 */
    mystmt(hstmt,rc);

    printMessage("1-10, total rows:%ld\n",nrows);
    myassert(nrows == 10);

    for (index=1; index<=nrows; index++)
    {
        printMessage("\n %d",iarray[index-1]);
        myassert(iarray[index-1] == index);
    }    

    rc = SQLFetchScroll(hstmt,SQL_FETCH_NEXT,0);/* 10-20 */
    mystmt(hstmt,rc);

    printMessage("\n10-20, total rows:%d\n",nrows);

    for (index=1; index<=nrows; index++)
    {
        printMessage(" %d ",iarray[index-1]);
        myassert(iarray[index-1] == index+10);
    }    

    rc = SQLFetchScroll(hstmt,SQL_FETCH_PREV,0);/* 1-10 */
    mystmt(hstmt,rc);

    printMessage("\n1-10, total rows:%d\n",nrows);

    for (index=1; index<=nrows; index++)
    {
        printMessage(" %d ",iarray[index-1]);
        myassert(iarray[index-1] == index);
    }    

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,1);/* 2-11 */
    mystmt(hstmt,rc);

    printMessage("\n2-12, total rows:%d\n",nrows);

    for (index=1; index<=nrows; index++)
    {
        printMessage(" %d ",iarray[index-1]);
        myassert(iarray[index-1] == index+1);
    } 

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-1);/* 1-10 */
    mystmt(hstmt,rc);

    printMessage("\n1-10, total rows:%d\n",nrows);

    for (index=1; index<=nrows; index++)
    {
        printMessage(" %d",iarray[index-1]);
        myassert(iarray[index-1] == index);
    }       

    rc = SQLFetchScroll(hstmt,SQL_FETCH_FIRST,0);/* 1-10 */
    mystmt(hstmt,rc);

    printMessage("\n1-10, total rows:%d\n",nrows);

    for (index=1; index<=nrows; index++)
    {
        printMessage(" %d",iarray[index-1]);
        myassert(iarray[index-1] == index);
    }      

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-1);/* BOF */
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,1);/* 1-10 */
    mystmt(hstmt,rc);

    printMessage("\n1-10, total rows:%d\n",nrows);

    for (index=1; index<=nrows; index++)
    {
        printMessage(" %d",iarray[index-1]);
        myassert(iarray[index-1] == index);
    } 

    SQLFreeStmt(hstmt,SQL_UNBIND);    
    SQLFreeStmt(hstmt,SQL_CLOSE);          

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROW_ARRAY_SIZE,(SQLPOINTER)1,0);
    mystmt(hstmt,rc);
}
/**
Testing SQL_FETCH_RELATIVE with row_set_size as 1
*/
void t_relative1(SQLHDBC hdbc,SQLHSTMT hstmt)
{
    SQLRETURN rc;
    SQLINTEGER nrows;
    long i;
    const int max_rows=10;

    SQLExecDirect(hstmt,"drop table t_relative1",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table t_relative1(id int)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"insert into t_relative1 values(?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT, SQL_C_ULONG,
                          SQL_INTEGER,0,0,&i,0,NULL);
    mystmt(hstmt,rc);

    for ( i = 1; i <= max_rows; i++ )
    {
        rc = SQLExecute(hstmt);
        mystmt(hstmt,rc);
    }

    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLEndTran(SQL_HANDLE_DBC,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    /* set row_size as 1 */
    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROW_ARRAY_SIZE,(SQLPOINTER)1,0);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROWS_FETCHED_PTR,&nrows,0);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"select * from t_relative1",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&i,0,NULL);
    mystmt(hstmt,rc);

    /* row 1 */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_NEXT,0);/* 1 */
    mystmt(hstmt,rc);
    my_assert(i==1);

    /* Before start */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-1);/* before start */
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    /* jump to last row */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,max_rows);/* last row */
    mystmt(hstmt,rc);
    my_assert(i==max_rows);

    /* jump to last row+1 */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,1);/* after last */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    /* goto first row */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_FIRST,1);/* 1 */    
    mystmt(hstmt,rc);
    my_assert(i==1);

    /* before start */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-1);/* before start */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    /* goto fifth  row */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,5);/* 5 */    
    mystmt(hstmt,rc);
    my_assert(i==5);

    /* goto after end */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,max_rows);/* after last */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    /* 
       the scenarios from ODBC spec     
    */    

    /* CASE 1 */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_FIRST,1);/* 1 */    
    mystmt(hstmt,rc);
    my_assert(i==1);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-1);/* before start */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    /* BeforeStart AND FetchOffset <= 0 */    
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-20);/* before start */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-1);/* before start */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,0);/* before start */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    /* case 1: Before start AND FetchOffset > 0 */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,1);/* 1 */    
    mystmt(hstmt,rc);
    my_assert(i==1);

    /* CASE 2 */    
    rc = SQLFetchScroll(hstmt,SQL_FETCH_LAST,1);/* last row */    
    mystmt(hstmt,rc);
    my_assert(i==max_rows);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,1);/* after end */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    /* After end AND FetchOffset >= 0 */    
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,10);/* after end */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,20);/* after end */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,1);/* after end */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,0);/* after end */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    /* After end AND FetchOffset < 0 */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-1);/* last row */    
    mystmt(hstmt,rc);
    my_assert(i==max_rows);


    /* CASE 3 */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_FIRST,1);/* first row */    
    mystmt(hstmt,rc);
    my_assert(i==1);

    /* CurrRowsetStart = 1 AND FetchOffset < 0 */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,0);/* first row */    
    mystmt(hstmt,rc);
    my_assert(i==1);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-1);/* before start */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    /* CASE 4 */
    /* CurrRowsetStart > 1 AND CurrRowsetStart + FetchOffset < 1 AND
       | FetchOffset | > RowsetSize
    */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_FIRST,1);/* first row */    
    mystmt(hstmt,rc);
    my_assert(i==1);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,3);/* fourth row */    
    mystmt(hstmt,rc);
    my_assert(i==4);

    /* the following call satisfies 4 > 1 AND (3-4) < 1 AND |-4| > 1 */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-4);/* before start */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    /* CASE 5 */
    /* 1 <= CurrRowsetStart + FetchOffset <= LastResultRow */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_FIRST,1);/* first row */    
    mystmt(hstmt,rc);
    my_assert(i==1);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,5);/* sixth row */    
    mystmt(hstmt,rc);
    my_assert(i==6);

    /* 1 <= 6-2 <= 10 */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-2);/* fourth row */    
    mystmt(hstmt,rc);
    my_assert(i==4);

    /* CASE 6 */
    /*  CurrRowsetStart > 1 AND CurrRowsetStart + FetchOffset < 1 AND
        | FetchOffset | <= RowsetSize
     */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_FIRST,1);/* first row */    
    mystmt(hstmt,rc);
    my_assert(i==1);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,3);/* fourth row */    
    mystmt(hstmt,rc);
    my_assert(i==4);

    /* 4 >1 AND 4-4 <1 AND |-4| <=10 */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-4);/* before start */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);


    SQLFreeStmt(hstmt,SQL_UNBIND);    
    SQLFreeStmt(hstmt,SQL_CLOSE);          

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROW_ARRAY_SIZE,(SQLPOINTER)1,0);
    mystmt(hstmt,rc);
}
/**
Testing SQL_FETCH_RELATIVE with row_set_size as 2
*/
void t_relative2(SQLHDBC hdbc,SQLHSTMT hstmt)
{
    SQLRETURN rc;
    SQLINTEGER nrows,iarray[15];
    SQLUINTEGER i;
    const int max_rows=10;

    SQLExecDirect(hstmt,"drop table t_relative2",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table t_relative2(id int)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"insert into t_relative2 values(?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT, SQL_C_ULONG,
                          SQL_INTEGER,0,0,&i,0,NULL);
    mystmt(hstmt,rc);

    for ( i = 1; i <= max_rows; i++ )
    {
        rc = SQLExecute(hstmt);
        mystmt(hstmt,rc);
    }

    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLEndTran(SQL_HANDLE_DBC,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    /* set row_size as 2 */
    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROW_ARRAY_SIZE,(SQLPOINTER)2,0);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROWS_FETCHED_PTR,&nrows,0);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"select * from t_relative2",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&iarray,0,NULL);
    mystmt(hstmt,rc);

    /* row 1 */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_NEXT,0);/* 1 */
    mystmt(hstmt,rc);
    my_assert(nrows == 2);
    my_assert(iarray[0]==1);
    my_assert(iarray[1]==2);


    /* Before start */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-1);/* before start */
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    /* jump to last row */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,max_rows);/* last row */
    mystmt(hstmt,rc);        
    my_assert(nrows == 1);
    my_assert(iarray[0]==max_rows);

    /* jump to last row+1 */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,1);/* after last */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    /* goto first row */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_FIRST,1);/* 1 */    
    mystmt(hstmt,rc);
    my_assert(nrows == 2);
    my_assert(iarray[0]==1);
    my_assert(iarray[1]==2);

    /* before start */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-1);/* before start */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    /* goto fifth  row */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,5);/* 5 */    
    mystmt(hstmt,rc);    
    my_assert(nrows == 2);
    my_assert(iarray[0]==5);
    my_assert(iarray[1]==6);

    /* goto after end */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,max_rows);/* after last */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    /* 
       the scenarios from ODBC spec     
    */    

    /* CASE 1 */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_FIRST,1);/* 1 */    
    mystmt(hstmt,rc);
    my_assert(nrows == 2);
    my_assert(iarray[0]==1);
    my_assert(iarray[1]==2);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-1);/* before start */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    /* BeforeStart AND FetchOffset <= 0 */    
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-20);/* before start */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-1);/* before start */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,0);/* before start */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    /* case 1: Before start AND FetchOffset > 0 */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,1);/* 1 */    
    mystmt(hstmt,rc);
    my_assert(nrows == 2);
    my_assert(iarray[0]==1);
    my_assert(iarray[1]==2);

    /* CASE 2 */    
    rc = SQLFetchScroll(hstmt,SQL_FETCH_LAST,1);/* last row */    
    mystmt(hstmt,rc);
    my_assert(nrows == 2);
    my_assert(iarray[0]==max_rows-1);    
    my_assert(iarray[1]==max_rows);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,1);/* last row */    
    mystmt(hstmt,rc);
    my_assert(nrows == 1);
    my_assert(iarray[0]==max_rows);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,1);/* after last row */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    /* After end AND FetchOffset >= 0 */    
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,10);/* after end */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,20);/* after end */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,1);/* after end */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,0);/* after end */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    /* After end AND FetchOffset < 0 */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-1);/* last row */    
    mystmt(hstmt,rc);
    my_assert(nrows == 1);
    my_assert(iarray[0]==max_rows);    


    /* CASE 3 */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_FIRST,1);/* first row */    
    mystmt(hstmt,rc);
    my_assert(nrows == 2);
    my_assert(iarray[0]==1);
    my_assert(iarray[1]==2);

    /* CurrRowsetStart = 1 AND FetchOffset < 0 */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,0);/* first row */    
    mystmt(hstmt,rc);
    my_assert(nrows == 2);
    my_assert(iarray[0]==1);
    my_assert(iarray[1]==2);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-1);/* before start */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    /* CASE 4 */
    /* CurrRowsetStart > 1 AND CurrRowsetStart + FetchOffset < 1 AND
       | FetchOffset | > RowsetSize
    */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_FIRST,1);/* first row */    
    mystmt(hstmt,rc);
    my_assert(nrows == 2);
    my_assert(iarray[0]==1);
    my_assert(iarray[1]==2);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,3);/* fourth row */    
    mystmt(hstmt,rc);
    my_assert(nrows == 2);
    my_assert(iarray[0]==4);
    my_assert(iarray[1]==5);

    /* the following call satisfies 4 > 1 AND (3-4) < 1 AND |-4| > 1 */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-4);/* before start */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    /* CASE 5 */
    /* 1 <= CurrRowsetStart + FetchOffset <= LastResultRow */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_FIRST,1);/* first row */    
    mystmt(hstmt,rc);
    my_assert(nrows == 2);
    my_assert(iarray[0]==1);
    my_assert(iarray[1]==2);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,5);/* sixth row */    
    mystmt(hstmt,rc);
    my_assert(nrows == 2);
    my_assert(iarray[0]==6);
    my_assert(iarray[1]==7);

    /* 1 <= 6-2 <= 10 */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-2);/* fourth row */    
    mystmt(hstmt,rc);
    my_assert(nrows == 2);
    my_assert(iarray[0]==4);
    my_assert(iarray[1]==5);

    /* CASE 6 */
    /*  CurrRowsetStart > 1 AND CurrRowsetStart + FetchOffset < 1 AND
        | FetchOffset | <= RowsetSize
     */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_FIRST,1);/* first row */    
    mystmt(hstmt,rc);
    my_assert(nrows == 2);
    my_assert(iarray[0]==1);
    my_assert(iarray[1]==2);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,3);/* fourth row */    
    mystmt(hstmt,rc);
    my_assert(nrows == 2);
    my_assert(iarray[0]==4);
    my_assert(iarray[1]==5);

    /* 4 >1 AND 4-4 <1 AND |-4| <=10 */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_RELATIVE,-4);/* before start */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);


    SQLFreeStmt(hstmt,SQL_UNBIND);    
    SQLFreeStmt(hstmt,SQL_CLOSE);          

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROW_ARRAY_SIZE,(SQLPOINTER)1,0);
    mystmt(hstmt,rc);
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
    t_scroll(hdbc,hstmt);
    t_relative(hdbc,hstmt);
    t_relative1(hdbc,hstmt);
    t_relative2(hdbc,hstmt);
    mydisconnect(&henv,&hdbc,&hstmt);

    printMessageFooter( 1 );

    return(0);
}
