/***************************************************************************
                          my_keys.c  -  description
                             -------------------
    begin                : Sat Dec 29 2001
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

SQLCHAR *mysock= NULL;

/*
UPDATE with primary keys ... 
*/
void my_primary_keys(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN rc;
    SQLLEN rowcount;
    long nData;

    SQLExecDirect(hstmt,"drop table my_primary_keys",SQL_NTS);
    rc = SQLExecDirect(hstmt,"create table my_primary_keys(col1 int not null,\
                                                           col2 varchar(30) unique,\
                                                           col3 int unique not null,\
                                                           col4 int not null,\
                                                           primary key(col1))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into my_primary_keys values(100,'MySQL1',1,3000)",SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt,"insert into my_primary_keys values(200,'MySQL2',2,3000)",SQL_NTS);
    mystmt(hstmt,rc);  
    rc = SQLExecDirect(hstmt,"insert into my_primary_keys values(300,'MySQL3',3,3000)",SQL_NTS);
    mystmt(hstmt,rc);  
    rc = SQLExecDirect(hstmt,"insert into my_primary_keys values(400,'MySQL4',4,3000)",SQL_NTS);
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);     

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC, 0);
    mystmt(hstmt, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY ,(SQLPOINTER)SQL_CONCUR_ROWVER , 0);
    mystmt(hstmt, rc);  

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE  ,(SQLPOINTER)1 , 0);
    mystmt(hstmt, rc);  

    rc = SQLExecDirect(hstmt,"select col4 from my_primary_keys",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,2,NULL,NULL);
    mystmt(hstmt,rc);

    nData = 999;

    rc = SQLSetPos(hstmt,0,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&rowcount);
    mystmt(hstmt,rc);

    printMessage(" rows affected:%d\n",rowcount); 
    /* myassert(rowcount == 1); */

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"select * from my_primary_keys",SQL_NTS);
    mystmt(hstmt,rc);

    myassert(4 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"select * from my_primary_keys",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(3000 == my_fetch_int(hstmt,4));

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(999 == my_fetch_int(hstmt,4));

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(3000 == my_fetch_int(hstmt,4));

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(3000 == my_fetch_int(hstmt,4));

    rc = SQLFetch(hstmt);
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);   
}

/*
UPDATE with unique and notnull  keys ... 
*/
void my_unique_notnull_keys(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN rc;
    SQLLEN rowcount;
    long nData;

    SQLExecDirect(hstmt,"drop table my_unique_notnull_keys",SQL_NTS);
    rc = SQLExecDirect(hstmt,"create table my_unique_notnull_keys(col1 int not null,\
                                                           col2 varchar(30) unique,\
                                                           col3 int unique not null,\
                                                           col4 int not null)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into my_unique_notnull_keys values(100,'MySQL1',1,3000)",SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt,"insert into my_unique_notnull_keys values(200,'MySQL2',2,3000)",SQL_NTS);
    mystmt(hstmt,rc);  
    rc = SQLExecDirect(hstmt,"insert into my_unique_notnull_keys values(300,'MySQL3',3,3000)",SQL_NTS);
    mystmt(hstmt,rc);  
    rc = SQLExecDirect(hstmt,"insert into my_unique_notnull_keys values(400,'MySQL4',4,3000)",SQL_NTS);
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);     

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC, 0);
    mystmt(hstmt, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY ,(SQLPOINTER)SQL_CONCUR_ROWVER , 0);
    mystmt(hstmt, rc);  

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE  ,(SQLPOINTER)1 , 0);
    mystmt(hstmt, rc);  

    rc = SQLExecDirect(hstmt,"select col4 from my_unique_notnull_keys",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,2,NULL,NULL);
    mystmt(hstmt,rc);

    nData = 999;

    rc = SQLSetPos(hstmt,0,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&rowcount);
    mystmt(hstmt,rc);

    printMessage(" rows affected:%d\n",rowcount); 
    myassert(rowcount == 1);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"select * from my_unique_notnull_keys",SQL_NTS);
    mystmt(hstmt,rc);

    myassert(4 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"select * from my_unique_notnull_keys",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(3000 == my_fetch_int(hstmt,4));

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(999 == my_fetch_int(hstmt,4));

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(3000 == my_fetch_int(hstmt,4));

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(3000 == my_fetch_int(hstmt,4));

    rc = SQLFetch(hstmt);
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);   

}

/*
UPDATE with unique keys ... 
*/
void my_unique_keys(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN rc;
    SQLLEN rowcount;
    long nData;

    SQLExecDirect(hstmt,"drop table my_unique_keys",SQL_NTS);
    rc = SQLExecDirect(hstmt,"create table my_unique_keys(col1 int not null,\
                                                           col2 varchar(30) unique,\
                                                           col3 int unique,\
                                                           col4 int not null)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into my_unique_keys values(100,'MySQL1',1,3000)",SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt,"insert into my_unique_keys values(200,'MySQL2',2,3000)",SQL_NTS);
    mystmt(hstmt,rc);  
    rc = SQLExecDirect(hstmt,"insert into my_unique_keys values(300,'MySQL3',3,3000)",SQL_NTS);
    mystmt(hstmt,rc);  
    rc = SQLExecDirect(hstmt,"insert into my_unique_keys values(400,'MySQL4',4,3000)",SQL_NTS);
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);     

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC, 0);
    mystmt(hstmt, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY ,(SQLPOINTER)SQL_CONCUR_ROWVER , 0);
    mystmt(hstmt, rc);  

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE  ,(SQLPOINTER)1 , 0);
    mystmt(hstmt, rc);  

    rc = SQLExecDirect(hstmt,"select col4 from my_unique_keys",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,2,NULL,NULL);
    mystmt(hstmt,rc);

    nData = 999;

    rc = SQLSetPos(hstmt,0,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&rowcount);
    mystmt(hstmt,rc);

    printMessage(" rows affected:%d\n",rowcount); 
    myassert(rowcount == 1);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"select * from my_unique_keys",SQL_NTS);
    mystmt(hstmt,rc);

    myassert(4 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"select * from my_unique_keys",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(3000 == my_fetch_int(hstmt,4));

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(999 == my_fetch_int(hstmt,4));

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(3000 == my_fetch_int(hstmt,4));

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(3000 == my_fetch_int(hstmt,4));

    rc = SQLFetch(hstmt);
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);   
}

/*
UPDATE with not null keys ... 
*/
void my_notnull_keys(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN rc;
    SQLLEN rowcount;
    long nData;

    SQLExecDirect(hstmt,"drop table my_unique_keys",SQL_NTS);
    rc = SQLExecDirect(hstmt,"create table my_notnull_keys(col1 int,\
                                                           col2 varchar(30),\
                                                           col3 int not null,\
                                                           col4 int not null)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into my_notnull_keys values(100,'MySQL1',1,3000)",SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt,"insert into my_notnull_keys values(200,'MySQL2',2,3000)",SQL_NTS);
    mystmt(hstmt,rc);  
    rc = SQLExecDirect(hstmt,"insert into my_notnull_keys values(300,'MySQL3',3,3000)",SQL_NTS);
    mystmt(hstmt,rc);  
    rc = SQLExecDirect(hstmt,"insert into my_notnull_keys values(400,'MySQL4',4,3000)",SQL_NTS);
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);     

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC, 0);
    mystmt(hstmt, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY ,(SQLPOINTER)SQL_CONCUR_ROWVER , 0);
    mystmt(hstmt, rc);  

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE  ,(SQLPOINTER)1 , 0);
    mystmt(hstmt, rc);  

    rc = SQLExecDirect(hstmt,"select col4 from my_notnull_keys",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,2,NULL,NULL);
    mystmt(hstmt,rc);

    nData = 999;

    rc = SQLSetPos(hstmt,0,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&rowcount);
    mystmt(hstmt,rc);

    printMessage(" rows affected:%d\n",rowcount); 
    myassert(rowcount == 1);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"select * from my_notnull_keys",SQL_NTS);
    mystmt(hstmt,rc);

    myassert(4 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"select * from my_notnull_keys",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(3000 == my_fetch_int(hstmt,4));

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(999 == my_fetch_int(hstmt,4));

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(3000 == my_fetch_int(hstmt,4));

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(3000 == my_fetch_int(hstmt,4));

    rc = SQLFetch(hstmt);
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);   

}
/*
UPDATE with no keys ... 
*/
void my_no_keys(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN rc;
    long nData;
    SQLLEN rowcount;

    /* INIT */
    SQLExecDirect(hstmt,"drop table my_no_keys",SQL_NTS);
    rc = SQLExecDirect(hstmt,"create table my_no_keys(col1 int,\
                                                      col2 varchar(30),\
                                                      col3 int,\
                                                      col4 int)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into my_no_keys values(100,'MySQL1',1,3000)",SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt,"insert into my_no_keys values(200,'MySQL2',2,3000)",SQL_NTS);
    mystmt(hstmt,rc);  
    rc = SQLExecDirect(hstmt,"insert into my_no_keys values(300,'MySQL3',3,3000)",SQL_NTS);
    mystmt(hstmt,rc);  
    rc = SQLExecDirect(hstmt,"insert into my_no_keys values(400,'MySQL4',4,3000)",SQL_NTS);
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);     

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC, 0);
    mystmt(hstmt, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY ,(SQLPOINTER)SQL_CONCUR_ROWVER , 0);
    mystmt(hstmt, rc);  

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE  ,(SQLPOINTER)1 , 0);
    mystmt(hstmt, rc);  

    /* UPDATE ROW[2]COL[4] */
    rc = SQLExecDirect(hstmt,"select col4 from my_no_keys",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,2,NULL,NULL);
    mystmt(hstmt,rc);

    nData = 999;

    /* TO BE FIXED LATER
	
	rc = SQLSetPos(hstmt,2,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&rowcount);
    mystmt(hstmt,rc);

    printMessage(" rows affected:%d\n",rowcount); 
    myassert(rowcount == 1); */

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"select * from my_no_keys",SQL_NTS);
    mystmt(hstmt,rc);

    myassert(4 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"select * from my_no_keys",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(3000 == my_fetch_int(hstmt,4));

    /*  TO BE FIXED LATER (SEE ABOVE)
	
	rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(999 == my_fetch_int(hstmt,4)); */

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(3000 == my_fetch_int(hstmt,4));

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(3000 == my_fetch_int(hstmt,4));

    /* TO BE FIXED LATER (SEE ABOVE)
	
	rc = SQLFetch(hstmt);
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc); */

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);
}
/*
UPDATE with no keys with all duplicate columns... 
*/
void my_no_keys_all_dups(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN rc;
    SQLLEN rowcount;
    long nData;

    SQLExecDirect(hstmt,"drop table my_no_keys_all_dups",SQL_NTS);
    rc = SQLExecDirect(hstmt,"create table my_no_keys_all_dups (col1 int,\
                                                      col2 int,\
                                                      col3 int)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into my_no_keys_all_dups values(100,100,100)",SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt,"insert into my_no_keys_all_dups values(100,100,100)",SQL_NTS);
    mystmt(hstmt,rc);  
    rc = SQLExecDirect(hstmt,"insert into my_no_keys_all_dups values(100,100,100)",SQL_NTS);
    mystmt(hstmt,rc);  
    rc = SQLExecDirect(hstmt,"insert into my_no_keys_all_dups values(100,100,100)",SQL_NTS);
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);     

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC, 0);
    mystmt(hstmt, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY ,(SQLPOINTER)SQL_CONCUR_ROWVER , 0);
    mystmt(hstmt, rc);  

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE  ,(SQLPOINTER)1 , 0);
    mystmt(hstmt, rc);  

    rc = SQLExecDirect(hstmt,"select col3 from my_no_keys_all_dups",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,2,NULL,NULL);
    mystmt(hstmt,rc);

    nData = 999;

    rc = SQLSetPos(hstmt,0,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&rowcount);
    mystmt(hstmt,rc);

    printMessage(" rows affected:%d\n",rowcount); 
    myassert(rowcount == 1);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"select * from my_no_keys_all_dups",SQL_NTS);
    mystmt(hstmt,rc);

    myassert(4 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"select * from my_no_keys_all_dups",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(999 == my_fetch_int(hstmt,3));

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(100 == my_fetch_int(hstmt,3));

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(100 == my_fetch_int(hstmt,3));

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(100 == my_fetch_int(hstmt,3));

    rc = SQLFetch(hstmt);
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);
}
/*
Initialize the foreignkey tables (MySQL specific)
*/
static void my_init_mysql_fkey(SQLHDBC hdbc,SQLHSTMT hstmt)
{
    SQLRETURN rc;     

    SQLExecDirect(hstmt,"DROP DATABASE test_odbc_fk",SQL_NTS);
    SQLExecDirect(hstmt,"CREATE DATABASE test_odbc_fk",SQL_NTS);
    SQLExecDirect(hstmt,"use test_odbc_fk",SQL_NTS);

    rc = SQLExecDirect(hstmt,"DROP TABLE test_fkey_c1",SQL_NTS);
    rc = SQLExecDirect(hstmt,"DROP TABLE test_fkey3",SQL_NTS);    
    rc = SQLExecDirect(hstmt,"DROP TABLE test_fkey2",SQL_NTS);
    rc = SQLExecDirect(hstmt,"DROP TABLE test_fkey1",SQL_NTS);
    rc = SQLExecDirect(hstmt,"DROP TABLE test_fkey_p1",SQL_NTS);
    rc = SQLExecDirect(hstmt,"DROP TABLE test_fkey_comment_f",SQL_NTS);
    rc = SQLExecDirect(hstmt,"DROP TABLE test_fkey_comment_p",SQL_NTS);

    rc = SQLExecDirect(hstmt,
                       "CREATE TABLE test_fkey1(\
                 A INTEGER NOT NULL,B INTEGER NOT NULL,C INTEGER NOT NULL,\
                 D INTEGER,PRIMARY KEY (C,B,A))TYPE=InnoDB;",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,
                       "CREATE TABLE test_fkey_p1(\
                 A INTEGER NOT NULL,B INTEGER NOT NULL,C INTEGER NOT NULL,\
                 D INTEGER NOT NULL,E INTEGER NOT NULL,F INTEGER NOT NULL,\
                 G INTEGER NOT NULL,H INTEGER NOT NULL,I INTEGER NOT NULL,\
                 J INTEGER NOT NULL,K INTEGER NOT NULL,L INTEGER NOT NULL,\
                 M INTEGER NOT NULL,N INTEGER NOT NULL,O INTEGER NOT NULL,\
                 P INTEGER NOT NULL,Q INTEGER NOT NULL,R INTEGER NOT NULL,\
                 PRIMARY KEY (D,F,G,H,I,J,K,L,M,N,O))TYPE=InnoDB;",SQL_NTS);
    mystmt(hstmt,rc);
    rc  = SQLExecDirect(hstmt,
                        "CREATE TABLE test_fkey2 (\
                 E INTEGER NOT NULL,C INTEGER NOT NULL,B INTEGER NOT NULL,\
                 A INTEGER NOT NULL,PRIMARY KEY (E),\
                 INDEX test_fkey2_ind(C,B,A),\
                 FOREIGN KEY (C,B,A) REFERENCES test_fkey1(C,B,A))TYPE=InnoDB;",SQL_NTS);
    mystmt(hstmt,rc);

    rc  = SQLExecDirect(hstmt,
                        "CREATE TABLE test_fkey3 (\
                 F INTEGER NOT NULL,C INTEGER NOT NULL,E INTEGER NOT NULL,\
                 G INTEGER, A INTEGER NOT NULL, B INTEGER NOT NULL,\
                 PRIMARY KEY (F),\
                 INDEX test_fkey3_ind(C,B,A),\
                 INDEX test_fkey3_ind3(E),\
                 FOREIGN KEY (C,B,A) REFERENCES test_fkey1(C,B,A),\
                 FOREIGN KEY (E) REFERENCES test_fkey2(E))TYPE=InnoDB;", SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,
                       "CREATE TABLE test_fkey_c1(\
                 A INTEGER NOT NULL,B INTEGER NOT NULL,C INTEGER NOT NULL,\
                 D INTEGER NOT NULL,E INTEGER NOT NULL,F INTEGER NOT NULL,\
                 G INTEGER NOT NULL,H INTEGER NOT NULL,I INTEGER NOT NULL,\
                 J INTEGER NOT NULL,K INTEGER NOT NULL,L INTEGER NOT NULL,\
                 M INTEGER NOT NULL,N INTEGER NOT NULL,O INTEGER NOT NULL,\
                 P INTEGER NOT NULL,Q INTEGER NOT NULL,R INTEGER NOT NULL,\
                 PRIMARY KEY (A,B,C,D,E,F,G,H,I,J,K,L,M,N,O),\
                INDEX test_fkey_c1_ind1(D,F,G,H,I,J,K,L,M,N,O),\
                INDEX test_fkey_c1_ind2(C,B,A),\
                INDEX test_fkey_c1_ind3(E),\
                 FOREIGN KEY (D,F,G,H,I,J,K,L,M,N,O) REFERENCES \
                   test_fkey_p1(D,F,G,H,I,J,K,L,M,N,O),\
                 FOREIGN KEY (C,B,A) REFERENCES test_fkey1(C,B,A),\
                 FOREIGN KEY (E) REFERENCES test_fkey2(E))TYPE=InnoDB;",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,    
                       "CREATE TABLE test_fkey_comment_p ( \
                ISP_ID SMALLINT NOT NULL, \
	              CUSTOMER_ID VARCHAR(10), \
	              ABBREVIATION VARCHAR(20) NOT NULL, \
	              NAME VARCHAR(40) NOT NULL, \
	              SEQUENCE INT NOT NULL, \
	              PRIMARY KEY PL_ISP_PK (ISP_ID), \
	              UNIQUE INDEX PL_ISP_CUSTOMER_ID_UK (CUSTOMER_ID), \
	              UNIQUE INDEX PL_ISP_ABBR_UK (ABBREVIATION) \
              ) TYPE = InnoDB COMMENT ='Holds the information of (customers)'",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,    
                       "CREATE TABLE test_fkey_comment_f ( \
	              CAMPAIGN_ID INT NOT NULL , \
	              ISP_ID SMALLINT NOT NULL, \
	              NAME  VARCHAR(40) NOT NULL, \
	              DISPLAY_NAME VARCHAR(255) NOT NULL, \
	              BILLING_TYPE SMALLINT NOT NULL, \
	              BROADBAND_OK BOOL, \
	              EMAIL_OK BOOL, \
	              PRIMARY KEY PL_ISP_CAMPAIGN_PK (CAMPAIGN_ID), \
	              UNIQUE INDEX PL_ISP_CAMPAIGN_BT_UK (BILLING_TYPE), \
	              INDEX PL_ISP_CAMPAIGN_ISP_ID_IX (ISP_ID), \
	              FOREIGN KEY (ISP_ID) REFERENCES test_fkey_comment_p(ISP_ID) \
              ) TYPE = InnoDB COMMENT ='List of campaigns (test comment)'",SQL_NTS);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt,SQL_CLOSE);
}
/*
Initialize the foreignkey tables (std)
*/
static void my_init_fkey(SQLHDBC hdbc,SQLHSTMT hstmt)
{
    SQLRETURN rc;     

    SQLExecDirect(hstmt,"DROP DATABASE test_odbc_fk",SQL_NTS);
    SQLExecDirect(hstmt,"CREATE DATABASE test_odbc_fk",SQL_NTS);
    SQLExecDirect(hstmt,"use test_odbc_fk",SQL_NTS);

    rc = SQLExecDirect(hstmt,"DROP TABLE test_fkey_c1",SQL_NTS);
    rc = SQLExecDirect(hstmt,"DROP TABLE test_fkey3",SQL_NTS);    
    rc = SQLExecDirect(hstmt,"DROP TABLE test_fkey2",SQL_NTS);
    rc = SQLExecDirect(hstmt,"DROP TABLE test_fkey1",SQL_NTS);
    rc = SQLExecDirect(hstmt,"DROP TABLE test_fkey_p1",SQL_NTS);
    rc = SQLExecDirect(hstmt,"DROP TABLE test_fkey_comment_f",SQL_NTS);
    rc = SQLExecDirect(hstmt,"DROP TABLE test_fkey_comment_p",SQL_NTS);

    rc = SQLExecDirect(hstmt,
                       "CREATE TABLE test_fkey1(\
                 A INTEGER NOT NULL,B INTEGER NOT NULL,C INTEGER NOT NULL,\
                 D INTEGER,PRIMARY KEY (C,B,A))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,
                       "CREATE TABLE test_fkey_p1(\
                 A INTEGER NOT NULL,B INTEGER NOT NULL,C INTEGER NOT NULL,\
                 D INTEGER NOT NULL,E INTEGER NOT NULL,F INTEGER NOT NULL,\
                 G INTEGER NOT NULL,H INTEGER NOT NULL,I INTEGER NOT NULL,\
                 J INTEGER NOT NULL,K INTEGER NOT NULL,L INTEGER NOT NULL,\
                 M INTEGER NOT NULL,N INTEGER NOT NULL,O INTEGER NOT NULL,\
                 P INTEGER NOT NULL,Q INTEGER NOT NULL,R INTEGER NOT NULL,\
                 PRIMARY KEY (D,F,G,H,I,J,K,L,M,N,O))",SQL_NTS);
    mystmt(hstmt,rc);
    rc  = SQLExecDirect(hstmt,
                        "CREATE TABLE test_fkey2 (\
                 E INTEGER NOT NULL,C INTEGER NOT NULL,B INTEGER NOT NULL,\
                 A INTEGER NOT NULL,PRIMARY KEY (E),\
                 FOREIGN KEY (C,B,A) REFERENCES test_fkey1(C,B,A))",SQL_NTS);
    mystmt(hstmt,rc);

    rc  = SQLExecDirect(hstmt,
                        "CREATE TABLE test_fkey3 (\
                 F INTEGER NOT NULL,C INTEGER NOT NULL,E INTEGER NOT NULL,\
                 G INTEGER, A INTEGER NOT NULL, B INTEGER NOT NULL,\
                 PRIMARY KEY (F),\
                 FOREIGN KEY (C,B,A) REFERENCES test_fkey1(C,B,A),\
                 FOREIGN KEY (E) REFERENCES test_fkey2(E))", SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,
                       "CREATE TABLE test_fkey_c1(\
                 A INTEGER NOT NULL,B INTEGER NOT NULL,C INTEGER NOT NULL,\
                 D INTEGER NOT NULL,E INTEGER NOT NULL,F INTEGER NOT NULL,\
                 G INTEGER NOT NULL,H INTEGER NOT NULL,I INTEGER NOT NULL,\
                 J INTEGER NOT NULL,K INTEGER NOT NULL,L INTEGER NOT NULL,\
                 M INTEGER NOT NULL,N INTEGER NOT NULL,O INTEGER NOT NULL,\
                 P INTEGER NOT NULL,Q INTEGER NOT NULL,R INTEGER NOT NULL,\
                 PRIMARY KEY (A,B,C,D,E,F,G,H,I,J,K,L,M,N,O),\
                 FOREIGN KEY (D,F,G,H,I,J,K,L,M,N,O) REFERENCES \
                   test_fkey_p1(D,F,G,H,I,J,K,L,M,N,O),\
                 FOREIGN KEY (C,B,A) REFERENCES test_fkey1(C,B,A),\
                 FOREIGN KEY (E) REFERENCES test_fkey2(E))",SQL_NTS);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt,SQL_CLOSE);
}
void my_foreign_keys(SQLHDBC hdbc,SQLHSTMT hstmt)
{
    SQLRETURN   rc=0;
    char        dbc[255];

    server_is_mysql(hdbc) ? my_init_mysql_fkey(hdbc,hstmt) : my_init_fkey(hdbc,hstmt);

    strcpy(dbc, "test_odbc_fk");

    printMessage("\n WITH ONLY PK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,               /*PK CATALOG*/
                        NULL, SQL_NTS,                /*PK SCHEMA*/
                        "test_fkey1", SQL_NTS,  /*PK TABLE*/
                        dbc, SQL_NTS,               /*FK CATALOG*/
                        NULL, SQL_NTS,                /*FK SCHEMA*/
                        NULL, SQL_NTS);               /*FK TABLE*/
    mystmt(hstmt,rc);
/*    myassert(9 == myresult(hstmt)); */

    printMessage("\n WITH ONLY FK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        NULL, SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey1", SQL_NTS);
    mystmt(hstmt,rc);
    myassert(0 == myresult(hstmt));

    printMessage("\n WITH ONLY FK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        NULL, SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey_c1", SQL_NTS);
    mystmt(hstmt,rc);
    /* myassert(15 == myresult(hstmt)); */

    printMessage("\n WITH ONLY FK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        NULL, SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey2", SQL_NTS);
    mystmt(hstmt,rc);
    /* myassert(3 == myresult(hstmt)); */

    printMessage("\n WITH ONLY PK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey_p1", SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        NULL, SQL_NTS);
    mystmt(hstmt,rc);
    /* myassert(11 == myresult(hstmt)); */

    printMessage("\n WITH ONLY PK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey3", SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        NULL, SQL_NTS);
    mystmt(hstmt,rc);
    myassert(0 == myresult(hstmt));

    printMessage("\n WITH ONLY PK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey2", SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        NULL, SQL_NTS);
    mystmt(hstmt,rc);
    /* myassert(2 == myresult(hstmt)); */

    printMessage("\n WITH ONLY PK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey1", SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        NULL, SQL_NTS);
    mystmt(hstmt,rc);
    /* myassert(9 == myresult(hstmt)); */

    printMessage("\n WITH ONLY FK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        NULL, SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey3", SQL_NTS);
    mystmt(hstmt,rc);
    /* myassert(4 == myresult(hstmt)); */

    printMessage("\n WITH BOTH PK and FK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey1",SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey3", SQL_NTS);
    mystmt(hstmt,rc);
    /* myassert(3 == myresult(hstmt)); */

    printMessage("\n WITH BOTH PK and FK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey_p1",SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey_c1", SQL_NTS);
    mystmt(hstmt,rc);
    /* myassert(11 == myresult(hstmt)); */

    printMessage("\n WITH BOTH PK and FK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey1",SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey2", SQL_NTS);
    mystmt(hstmt,rc);
    /* myassert(3 == myresult(hstmt)); */

    printMessage("\n WITH BOTH PK and FK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey_p1",SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey2", SQL_NTS);
    mystmt(hstmt,rc);
    myassert(0 == myresult(hstmt));

    printMessage("\n WITH BOTH PK and FK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey3", SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey1", SQL_NTS);
    mystmt(hstmt,rc);
    myassert(0 == myresult(hstmt));

    printMessage("\n WITH BOTH PK and FK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey2", SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey1", SQL_NTS);
    mystmt(hstmt,rc);
    myassert(0 == myresult(hstmt));


    printMessage("\n WITH ACTUAL LENGTH INSTEAD OF SQL_NTS");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey1",10,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey2",10);
    mystmt(hstmt,rc);
    /* assert(3 == myresult(hstmt)); */
    SQLFreeStmt(hstmt,SQL_CLOSE);

    printMessage("\n WITH NON-EXISTANT TABLES");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey_junk", SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey_junk", SQL_NTS);
    mystmt(hstmt,rc);
    myassert(0 == myresult(hstmt));  
    SQLFreeStmt(hstmt,SQL_CLOSE);

    printMessage("\n WITH COMMENT FIELD");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey_comment_p", SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        "test_fkey_comment_f", SQL_NTS);
    mystmt(hstmt,rc);
    /* myassert(1 == myresult(hstmt)); */

    {
        char buff[255];
        sprintf(buff,"use %s",mydb);
        rc = SQLExecDirect(hstmt, "DROP DATABASE test_odbc_fk", SQL_NTS);
        mystmt(hstmt, rc);
        SQLFreeStmt(hstmt, SQL_CLOSE);
        SQLExecDirect(hstmt, buff, SQL_NTS);
        SQLFreeStmt(hstmt, SQL_CLOSE);
    }
}

void t_strstr()
{
    char    *string = "'TABLE','VIEW','SYSTEM TABLE'";
    char    *str=",";
    char    *type;

    type = strstr((const char *)string,(const char *)str);
    while (type++)
    {
        int len = type - string;
        printMessage("\n Found '%s' at position '%d[%s]", str, len, type);
        type = strstr(type,str);
    }
}

/*
Initialize the foreignkey tables
*/
static void my_tables(SQLHENV henv,SQLHDBC hdbc,SQLHSTMT hstmt)
{
    SQLHDBC   hdbc1;
    SQLHSTMT  hstmt1;
    SQLRETURN rc;
    char      conn[256];

    SQLExecDirect(hstmt,"DROP DATABASE my_tables_test_db",SQL_NTS);
    rc = SQLExecDirect(hstmt,"CREATE DATABASE my_tables_test_db",SQL_NTS);
    mystmt(hstmt,rc);

    sprintf(conn,"DRIVER={MySQL ODBC 3.51 Driver};DSN=%s;UID=%s;PASSWORD=%s;DATABASE=%s",
            mydsn,myuid,mypwd,"my_tables_test_db");
    if (mysock != NULL)
    {
      strcat(conn, ";SOCKET=");
      strcat(conn, mysock);
    }
    rc = SQLAllocConnect(henv,&hdbc1);
    myenv(henv,rc);

    rc = SQLDriverConnect(hdbc1,NULL,conn,255,
                          NULL,0,NULL,SQL_DRIVER_COMPLETE);
    mycon(hdbc1,rc);

    rc = SQLAllocStmt(hdbc1,&hstmt1);
    mycon(hdbc1,rc);

    rc = SQLExecDirect(hstmt1,
                       "CREATE TABLE my_tables_test1(\
                 A INTEGER NOT NULL,B INTEGER NOT NULL)",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLExecDirect(hstmt1,
                       "CREATE TABLE my_tables_test2(\
                 A INTEGER NOT NULL,B INTEGER NOT NULL)",SQL_NTS);
    mystmt(hstmt1,rc);

    SQLEndTran(SQL_HANDLE_DBC,hdbc1,SQL_COMMIT);

    rc = SQLTables(hstmt1,"non_existing_junk",SQL_NTS,
                   NULL,0,
                   "my_tables_test1",SQL_NTS,
                   "TABLE",SQL_NTS);
    mystmt(hstmt1,rc);
    myassert(0 == my_print_non_format_result(hstmt1));  
    SQLFreeStmt(hstmt1,SQL_CLOSE);

    rc = SQLTables(hstmt1,"my_tables_test_db",SQL_NTS,
                   NULL,0,
                   "my_tables_test1",SQL_NTS,
                   "TABLE",SQL_NTS);
    mystmt(hstmt1,rc);
    myassert(1 == my_print_non_format_result(hstmt1));  
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLTables(hstmt1,"my_tables_test_db",SQL_NTS,
                   NULL,0,
                   "my_tables_test1",15,
                   "TABLE",SQL_NTS);
    mystmt(hstmt1,rc);
    myassert(1 == my_print_non_format_result(hstmt1));  
    SQLFreeStmt(hstmt1,SQL_CLOSE);

    rc = SQLTables(hstmt1,"my_tables_test_db",SQL_NTS,
                   NULL,0,
                   NULL,0,
                   "TABLE",SQL_NTS);
    mystmt(hstmt1,rc);
    myassert(2 == my_print_non_format_result(hstmt1));  
    SQLFreeStmt(hstmt1,SQL_CLOSE);

    rc = SQLTables(hstmt1,"my_tables_test_db",SQL_NTS,
                   NULL,0,
                   NULL,0,
                   "`TABLE`",SQL_NTS);
    mystmt(hstmt1,rc);
    if (!driver_min_version(hdbc,"03.51.07",8))
        myassert(0 == my_print_non_format_result(hstmt1));
    else
        myassert(0 != my_print_non_format_result(hstmt1));  
    SQLFreeStmt(hstmt1,SQL_CLOSE);

    rc = SQLTables(hstmt1,NULL,0,
                   NULL,0,
                   NULL,0,
                   "SYSTEM TABLE",SQL_NTS);
    mystmt(hstmt1,rc);
    if (!driver_min_version(hdbc,"03.51.07",8))
        myassert(0 == my_print_non_format_result(hstmt1));
    else
        myassert(my_print_non_format_result(hstmt1) != 0);  
    SQLFreeStmt(hstmt1,SQL_CLOSE);

    rc = SQLTables(hstmt1,NULL,0,
                   NULL,0,
                   NULL,0,
                   "TABLE,SYSTEM TABLE",SQL_NTS);
    mystmt(hstmt1,rc);
    if (!driver_min_version(hdbc,"03.51.07",8))
        myassert(2 == my_print_non_format_result(hstmt1));
    else
        myassert(my_print_non_format_result(hstmt1) > 2);  
    SQLFreeStmt(hstmt1,SQL_CLOSE);    

    rc = SQLTables(hstmt1,NULL,0,
                   NULL,0,
                   NULL,0,
                   "TABLE,VIEW,SYSTEM TABLE",SQL_NTS);
    mystmt(hstmt1,rc);
    if (!driver_min_version(hdbc,"03.51.07",8))
        myassert(2 == my_print_non_format_result(hstmt1));
    else
        myassert(my_print_non_format_result(hstmt1) > 2);  
    SQLFreeStmt(hstmt1,SQL_CLOSE);        

    rc = SQLTables(hstmt1,NULL,0,
                   NULL,0,
                   NULL,0,
                   "'TABLE','VIEW','SYSTEM TABLE'",SQL_NTS);
    mystmt(hstmt1,rc);
    if (!driver_min_version(hdbc,"03.51.07",8))
        myassert(2 == my_print_non_format_result(hstmt1));
    else
        myassert(my_print_non_format_result(hstmt1) > 2);  
    SQLFreeStmt(hstmt1,SQL_CLOSE);        

    rc = SQLTables(hstmt1,NULL,0,
                   NULL,0,
                   NULL,0,
                   "'TABLE',",SQL_NTS);
    mystmt(hstmt1,rc);
    myassert(2 == my_print_non_format_result(hstmt1));  
    SQLFreeStmt(hstmt1,SQL_CLOSE);

    SQLExecDirect(hstmt1,"DROP DATABASE my_tables_test_db",SQL_NTS);
    SQLEndTran(SQL_HANDLE_DBC,hdbc1,SQL_COMMIT);

    SQLFreeStmt(hstmt1,SQL_CLOSE);
    SQLDisconnect(hdbc1);
    SQLFreeConnect(hdbc1);
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
        else if ( narg == 4 )
            mysock= argv[4];
    }    

    myconnect(&henv,&hdbc,&hstmt); 
    my_tables(henv,hdbc,hstmt);
    my_foreign_keys(hdbc,hstmt);   
    if (driver_supports_setpos(hdbc))
    {
        my_primary_keys(hdbc,hstmt);
        my_unique_notnull_keys(hdbc,hstmt);
        my_unique_keys(hdbc,hstmt);
        my_no_keys(hdbc,hstmt);
        my_no_keys_all_dups(hdbc,hstmt);
    }
    mydisconnect(&henv,&hdbc,&hstmt);    

    printMessageFooter( 1 );

    return(0);
}
