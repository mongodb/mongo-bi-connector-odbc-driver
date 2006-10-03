/***************************************************************************
                          mytest3.c  -  description
                             -------------------
    begin                : Wed Aug 8 2001
    copyright            : (C) 2003 by Venu, MySQL AB
    email                : venu@mysql.com
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
  Utility function to verify a particular column data
*/
static void verify_col_data(SQLHSTMT hstmt, const char *table, 
                            const char *col, const char *exp_data)
{
  SQLCHAR data[255], query[255];
  SQLRETURN rc;
 
  if (table && col)
  {
    sprintf(query,"SELECT %s FROM %s",col,table);
    fprintf(stdout,"\n %s", query);

    rc = SQLExecDirect(hstmt, query, SQL_NTS);
    mystmt(hstmt,rc);
  }

  rc = SQLFetch(hstmt);
  if (rc == SQL_NO_DATA)
    myassert(strcmp(exp_data,"SQL_NO_DATA") ==0 );

  rc = SQLGetData(hstmt, 1, SQL_C_CHAR, &data, 255, NULL);
  if (rc == SQL_ERROR)
  {
    fprintf(stdout,"\n *** ERROR: FAILED TO GET THE RESULT ***");
    exit(1);
  }
  fprintf(stdout,"\n obtained: `%s` (expected: `%s`)", data, exp_data);
  myassert(strcmp(data,exp_data) == 0);

  SQLFreeStmt(hstmt, SQL_UNBIND);
  SQLFreeStmt(hstmt, SQL_CLOSE);
}

/**
 Simple function to do basic ops with MySQL
*/
static void t_basic(SQLHDBC hdbc,SQLHSTMT hstmt)
{
  SQLRETURN rc;
  long nInData = 1;
  long nOutData, nRowCount=1;
  char szOutData[255];

  printMessageHeader();

    /* CREATE TABLE 'myodbc' */
    SQLExecDirect(hstmt,"drop table tmyodbc ",SQL_NTS);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLExecDirect(hstmt,"create table tmyodbc (col1 int, col2 varchar(30))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    /* DIRECT INSERT */
    rc = SQLExecDirect(hstmt,"insert into tmyodbc values(10,'direct-insert')",SQL_NTS);
    mystmt(hstmt,rc);

    /* PREPARE INSERT */
    rc = SQLPrepare(hstmt,"insert into tmyodbc values(?,'prepare-insert')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT, SQL_C_LONG,SQL_INTEGER,
                          0,0,&nInData,0,NULL);
    mystmt(hstmt,rc);
    
    for (nInData= 100 ; nInData <= 2000; nInData=nInData+50)
    {
      rc = SQLExecute(hstmt);
      mystmt(hstmt,rc);
    }

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    /* FREE THE PARAM BUFFERS */
    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    /* FETCH RESULT SET */
    rc = SQLExecDirect(hstmt,"SELECT * FROM tmyodbc",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,1, SQL_C_LONG, &nOutData,0,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2, SQL_C_CHAR, szOutData,sizeof(szOutData),NULL);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    nInData = 100;
    while(SQLFetch(hstmt) == SQL_SUCCESS)
    {
      nRowCount++;
      fprintf(stdout,"\n row %d\t: %d",nRowCount, nOutData);
      my_assert( nInData == nOutData);
      nInData+= 50;
    }
    nInData-= 50;
    fprintf(stdout,"\n Total rows Found:%d\n",nRowCount);
    my_assert( nRowCount == nInData/50);

    /* FREE THE OUTPUT BUFFERS */
    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}

static void tmysql_setpos_del(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  long nData = 500;
  SQLLEN nlen;
  SQLCHAR szData[255]={0};
  SQLUINTEGER pcrow;
  SQLSMALLINT rgfRowStatus;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table tmysql_setpos1");
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table tmysql_setpos1(col1 int, col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(100,'MySQL1')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(200,'MySQL2')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(300,'MySQL3')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(400,'MySQL4')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(300,'MySQL5')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(300,'MySQL6')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(300,'MySQL7')");
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  

    rc = SQLSetCursorName(hstmt,"venu",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = tmysql_exec(hstmt,"select * from tmysql_setpos1");
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,100,&nlen);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,5,&pcrow,NULL);
    mystmt(hstmt,rc);

    fprintf(stdout," pcrow:%d\n",pcrow);  
   
    fprintf(stdout," row1:%d,%s\n",nData,szData);    
    
    rc = SQLSetPos(hstmt,1,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLNumResultCols(hstmt,&rgfRowStatus);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);
    
    fprintf(stdout," rows affected:%d\n",nlen);    

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from tmysql_setpos1");
    mystmt(hstmt,rc);  

    my_assert( 6 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from tmysql_setpos1");
    mystmt(hstmt,rc);  

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,5,NULL,NULL);
    mystmt(hstmt,rc);  
    
    my_assert(300 == my_fetch_int(hstmt,1));
    my_assert(!strcmp((const char *)"MySQL6",
               my_fetch_str(hstmt,szData,2)));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}
static void tmysql_setpos_del1(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  long nData= 500;
  SQLLEN nlen, pccol;
  SQLCHAR szData[255]={0};
  SQLUINTEGER pcrow;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table tmysql_setpos");
    rc = tmysql_exec(hstmt,"create table tmysql_setpos(col1 int, col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_setpos values(100,'MySQL1')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos values(200,'MySQL2')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos values(300,'MySQL2')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos values(400,'MySQL2')");
    mystmt(hstmt,rc);  
 

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);     

    rc = SQLSetCursorName(hstmt,"venu",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = tmysql_exec(hstmt,"select * from tmysql_setpos");
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,&pccol);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,100,&nlen);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,2,&pcrow,NULL);
    mystmt(hstmt,rc);

    fprintf(stdout," pcrow:%d\n",pcrow);  
   
    fprintf(stdout," row1:%d,%s\n",nData,szData);    
    
    pccol = SQL_COLUMN_IGNORE;
        
    rc = SQLSetPos(hstmt,0,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);
    
    fprintf(stdout," rows affected:%d\n",nlen); 
    myassert(nlen == 1);
    
    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from tmysql_setpos");
    mystmt(hstmt,rc);  

    my_assert(3 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}
static void t_setpos_del_all(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  long nData[4];
  SQLLEN nlen;
  SQLCHAR szData[4][10];

  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_sp_del_all");
    rc = tmysql_exec(hstmt,"create table t_sp_del_all(col1 int not null primary key,\
                                                      col2 varchar(20))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_sp_del_all values(100,'MySQL1')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into t_sp_del_all values(200,'MySQL2')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into t_sp_del_all values(300,'MySQL3')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into t_sp_del_all values(400,'MySQL4')");
    mystmt(hstmt,rc); 

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);     

    rc = SQLSetCursorName(hstmt,"venu",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = SQLSetStmtOption(hstmt, SQL_ROWSET_SIZE, 4);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_sp_del_all order by col1 asc");
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,&szData[0],sizeof(szData[0]),NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_FIRST,1,NULL,NULL);
    mystmt(hstmt,rc);

    fprintf(stdout," row1 : %d, %s\n",nData[0],szData[0]);    
    fprintf(stdout," row2 : %d, %s\n",nData[1],szData[1]);    
    fprintf(stdout," row3 : %d, %s\n",nData[2],szData[2]);    
    fprintf(stdout," row4 : %d, %s\n",nData[3],szData[3]);    
    
    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt,0,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);
    
    fprintf(stdout," total rows deleted: %d\n",nlen); 
    myassert(nlen == 4);
    
    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_sp_del_all");
    mystmt(hstmt,rc);  

    my_assert(0 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
    
    rc = SQLSetStmtOption(hstmt, SQL_ROWSET_SIZE, 1);
    mystmt(hstmt,rc);

}
static void tmysql_setpos_upd(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  long nData = 500;
  SQLLEN nlen;
  SQLCHAR szData[255]={0};
  SQLUINTEGER pcrow;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table tmysql_setpos");
    rc = tmysql_exec(hstmt,"create table tmysql_setpos(col1 int, col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_setpos values(100,'MySQL1')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos values(300,'MySQL3')");
    mystmt(hstmt,rc); 
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos values(200,'MySQL2')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos values(300,'MySQL3')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos values(400,'MySQL4')");
    mystmt(hstmt,rc); 
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos values(300,'MySQL3')");
    mystmt(hstmt,rc); 

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc); 

    rc = SQLSetCursorName(hstmt,"venu",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = tmysql_exec(hstmt,"select * from tmysql_setpos");
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,3,&pcrow,NULL);
    mystmt(hstmt,rc);

    fprintf(stdout," pcrow:%d\n",pcrow);  
   
    fprintf(stdout," row1:%d,%s\n",nData,szData);    
    
    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    nData = 1000;
    strcpy((char *)szData , "updated");
    
    rc = SQLSetPos(hstmt,3,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt_err(hstmt,rc == SQL_ERROR,rc);

    rc = SQLSetPos(hstmt,1,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);
    
    fprintf(stdout," rows affected:%d\n",nlen);   
    my_assert(nlen == 1); 

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from tmysql_setpos");
    mystmt(hstmt,rc);  

    myresult(hstmt);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
    
    rc = SQLExecDirect(hstmt,"DELETE FROM tmysql_setpos WHERE col2 = 'updated'",SQL_NTS);
    mystmt(hstmt,rc);

    nlen=0;
    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n total rows affceted:%d",nlen);
    my_assert(nlen == 1);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from tmysql_setpos");
    mystmt(hstmt,rc);  

    my_assert(5 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}

static void t_setpos_upd_decimal(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN  rc;
  long rec;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_setpos_upd_decimal");
    rc = tmysql_exec(hstmt,"create table t_setpos_upd_decimal(record decimal(3,0),\
                                num1 float, num2 decimal(6,0),num3 decimal(10,3))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_setpos_upd_decimal values(001,12.3,134,0.100)");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);    

    /* MS SQL Server to work...*/
    SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER) SQL_CONCUR_ROWVER, 0);
    SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_DYNAMIC, 0);
    SQLSetStmtOption(hstmt,SQL_SIMULATE_CURSOR,SQL_SC_NON_UNIQUE);

    rc = tmysql_exec(hstmt,"select record from t_setpos_upd_decimal");
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&rec,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,NULL,NULL);
    mystmt(hstmt,rc);
    fprintf(stdout," row1: %d",rec);    
    
    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rec = 100;
    
    rc = SQLSetPos(hstmt,1,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt_r(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_setpos_upd_decimal");
    mystmt(hstmt,rc);  

    myresult(hstmt);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}
static void tmysql_specialcols(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table tmysql_specialcols");
    rc = tmysql_exec(hstmt,"create table tmysql_specialcols(col1 int primary key, col2 varchar(30), col3 int)");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"create index tmysql_ind1 on tmysql_specialcols(col1)");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_specialcols values(100,'venu',1)");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_specialcols values(200,'MySQL',2)");
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  

    rc = tmysql_exec(hstmt,"select * from tmysql_specialcols");
    mystmt(hstmt,rc);  

    myresult(hstmt);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLSpecialColumns(hstmt,
                          SQL_BEST_ROWID,
                          NULL,0,
                          NULL,0,
                          "tmysql_specialcols",SQL_NTS,
                          SQL_SCOPE_SESSION,
                          SQL_NULLABLE);
    mystmt(hstmt,rc);

    myresult(hstmt);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"drop table tmysql_specialcols");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);
}
static void tmysql_bindcol(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  long    nodata, nidata = 200;
  SQLLEN  nlen, length;
  char    szodata[20],szidata[20]="MySQL";

  printMessageHeader();

    tmysql_exec(hstmt,"drop table tmysql_bindcol");

    rc = tmysql_exec(hstmt,"create table tmysql_bindcol(col1 int primary key, col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_bindcol values(100,'venu')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_bindcol values(200,'MySQL')");
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  

    rc = tmysql_prepare(hstmt,"select * from tmysql_bindcol where col2 = ? AND col1 = ?");
    mystmt(hstmt,rc); 
   
    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nodata,0,&nlen);
    mystmt(hstmt,rc);  

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szodata,200,&nlen);
    mystmt(hstmt,rc); 
    
    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT, SQL_C_CHAR,SQL_VARCHAR,
                          0,0,szidata,0,&length);
    mystmt(hstmt,rc); 

    rc = SQLBindParameter(hstmt,2,SQL_PARAM_INPUT, SQL_C_LONG,SQL_INTEGER,
                          0,0,&nidata,20,NULL);
    mystmt(hstmt,rc); 

    length= SQL_NTS;
    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);  

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);  
    
    fprintf(stdout," outdata: %d, %s(%d)\n", nodata,szodata,nlen);
    my_assert(nodata == 200);

    rc = SQLFetch(hstmt);

    my_assert(rc == SQL_NO_DATA_FOUND);    

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"drop table tmysql_bindcol");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);
}
static void tmysql_pos_delete(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  SQLHSTMT hstmt1;
  SQLUINTEGER pcrow;
  SQLUSMALLINT rgfRowStatus;

  printMessageHeader();

    rc = SQLAllocStmt(hdbc,&hstmt1);
    mycon(hdbc,rc);    
    
    tmysql_exec(hstmt,"drop table tmysql_pos_delete");
    rc = tmysql_exec(hstmt,"create table tmysql_pos_delete(col1 int , col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_pos_delete values(100,'venu')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_pos_delete values(200,'MySQL')");
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLSetCursorName(hstmt,"venu_cur",8);
    mystmt(hstmt,rc);   

    rc = tmysql_exec(hstmt,"select * from tmysql_pos_delete");
    mystmt(hstmt,rc); 
    
    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,NULL);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt1,"   DfffELETE FROM tmysql_pos_delete WHERE CURRENT OF venu_cur",SQL_NTS);
    mystmt_r(hstmt1,rc);  

    rc = SQLExecDirect(hstmt1,"   DELETE FROM tmysql_pos_delete WHERE CURRENT OF venu_cur curs",SQL_NTS);
    mystmt_r(hstmt1,rc);  

    rc = SQLExecDirect(hstmt1,"   DELETE FROM tmysql_pos_delete WHERE ONE CURRENT OF venu_cur",SQL_NTS);
    mystmt_r(hstmt1,rc);  

    rc = SQLExecDirect(hstmt1,"   DELETE FROM tmysql_pos_delete WHERE CURRENT OF venu_cur",SQL_NTS);
    mystmt(hstmt1,rc);  

    SQLNumResultCols(hstmt1,&rgfRowStatus);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    rc = SQLFreeStmt(hstmt1,SQL_CLOSE);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from tmysql_pos_delete");
    mystmt(hstmt,rc);  

    my_assert(1 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt1,SQL_DROP);
    mystmt(hstmt1,rc);
}
static void tmysql_pos_update(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  SQLHSTMT hstmt1;
  SQLUINTEGER pcrow;
  SQLUSMALLINT rgfRowStatus;

  printMessageHeader();

    rc = SQLAllocStmt(hdbc,&hstmt1);
    mycon(hdbc,rc);

    tmysql_exec(hstmt,"drop table tmysql_pos_delete");
    rc = tmysql_exec(hstmt,"create table tmysql_pos_delete(col1 int , col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_pos_delete values(100,'venu')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_pos_delete values(200,'MySQL')");
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc); 

    rc = SQLSetCursorName(hstmt,"venu_cur",SQL_NTS);
    mystmt(hstmt,rc);
 
    rc = tmysql_exec(hstmt,"select * from tmysql_pos_delete");
    mystmt(hstmt,rc); 
    
    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,2,&pcrow,NULL);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt1,"  UPerrDATE tmysql_pos_delete SET col1= 999, col2 = 'update' WHERE CURRENT OF venu_cur",SQL_NTS);
    mystmt_r(hstmt1,rc);

    rc = SQLExecDirect(hstmt1,"  UPerrDATE tmysql_pos_delete SET col1= 999, col2 = 'update' WHERE CURRENT OF",SQL_NTS);
    mystmt_r(hstmt1,rc);

    rc = SQLExecDirect(hstmt1,"  UPDATE tmysql_pos_delete SET col1= 999, col2 = 'update' WHERE CURRENT OF venu_cur",SQL_NTS);
    mystmt(hstmt1,rc);  

    SQLNumResultCols(hstmt1,&rgfRowStatus);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    rc = SQLFreeStmt(hstmt1,SQL_CLOSE);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from tmysql_pos_delete");
    mystmt(hstmt,rc);  

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);  

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);  
    {
      SQLCHAR szData[20];
      my_assert(999 == my_fetch_int(hstmt,1));
      my_assert(!strcmp("update",my_fetch_str(hstmt,szData,2)));
    }

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt1,SQL_DROP);
    mystmt(hstmt1,rc);
}
static void tmysql_mtab_setpos_del(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  long nData= 500;
  SQLLEN nlen;
  SQLCHAR szData[255]={0};
  SQLUINTEGER pcrow;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table tmysql_t1");
    tmysql_exec(hstmt,"drop table tmysql_t2");
    rc = tmysql_exec(hstmt,"create table tmysql_t1(col1 int, col2 varchar(20))");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"create table tmysql_t2(col1 int, col2 varchar(20))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_t1 values(1,'t1_one')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into tmysql_t1 values(2,'t1_two')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into tmysql_t1 values(3,'t1_three')");
    mystmt(hstmt,rc);  
 
    rc = tmysql_exec(hstmt,"insert into tmysql_t2 values(2,'t2_one')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into tmysql_t2 values(3,'t2_two')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into tmysql_t2 values(4,'t2_three')");
    mystmt(hstmt,rc);  
 

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc); 

    rc = SQLSetCursorName(hstmt,"venu",SQL_NTS);
    mystmt(hstmt,rc);    

    /* FULL JOIN */
    rc = tmysql_exec(hstmt,"select tmysql_t1.*,tmysql_t2.* from tmysql_t1,tmysql_t2");
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,100,&nlen);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,3,&pcrow,NULL);
    mystmt(hstmt,rc);

    fprintf(stdout," pcrow:%d\n",pcrow);  
   
    fprintf(stdout," row1:%d,%s\n",nData,szData);    
    
    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    /* not yet supported..*/
    rc = SQLSetPos(hstmt,4,SQL_DELETE,SQL_LOCK_NO_CHANGE);    
    mystmt_err(hstmt,rc==SQL_ERROR,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}
static void tmysql_showkeys(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table tmysql_spk");
    
    rc = tmysql_exec(hstmt,"create table tmysql_spk(col1 int primary key)");
    mystmt(hstmt,rc);
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  
    
    rc = tmysql_exec(hstmt,"SHOW KEYS FROM tmysql_spk");
    mystmt(hstmt,rc);    

    my_assert(1 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}
static void tmysql_setpos_pkdel(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  long nData= 500;
  SQLLEN nlen;
  SQLCHAR szData[255]={0};
  SQLUINTEGER pcrow;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table tmysql_setpos1");
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table tmysql_setpos1(col1 int primary key, col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(100,'MySQL1')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(200,'MySQL2')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(300,'MySQL3')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(400,'MySQL4')");
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc); 

    rc = SQLSetCursorName(hstmt,"venu",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = tmysql_exec(hstmt,"select * from tmysql_setpos1");
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,100,&nlen);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,NULL);
    mystmt(hstmt,rc);
    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,NULL);
    mystmt(hstmt,rc);
    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,NULL);
    mystmt(hstmt,rc);
    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,NULL);
    mystmt(hstmt,rc);

    fprintf(stdout," pcrow:%d\n",pcrow);  
   
    fprintf(stdout," row1:%d,%s\n",nData,szData);    
    
    rc = SQLSetPos(hstmt,4,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt_err(hstmt,rc==SQL_ERROR,rc);

    rc = SQLSetPos(hstmt,1,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);
    
    fprintf(stdout," rows affected:%d\n",nlen);    

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from tmysql_setpos1");
    mystmt(hstmt,rc);  

    my_assert( 3 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}
static void t_alias_setpos_pkdel(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  long nData= 500;
  SQLLEN nlen;
  SQLCHAR szData[255]={0};
  SQLUINTEGER pcrow;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_alias_setpos_del");
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_alias_setpos_del(col1 int primary key, col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_alias_setpos_del values(100,'MySQL1')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into t_alias_setpos_del values(200,'MySQL2')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into t_alias_setpos_del values(300,'MySQL3')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into t_alias_setpos_del values(400,'MySQL4')");
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  

    rc = SQLSetCursorName(hstmt,"venu",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = tmysql_exec(hstmt,"select col1 as id, col2 as name from t_alias_setpos_del");
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,100,&nlen);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,4,&pcrow,NULL);
    mystmt(hstmt,rc);

    fprintf(stdout," pcrow:%d\n",pcrow);  
   
    fprintf(stdout," row1:%d,%s\n",nData,szData);    
    
    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt,1,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);
    
    fprintf(stdout," rows affected:%d\n",nlen);    

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_alias_setpos_del");
    mystmt(hstmt,rc);  

    my_assert( 3 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}
static void t_alias_setpos_del(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  long nData[4];
  SQLLEN nlen;
  SQLCHAR szData[4][10];

  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_alias_setpos_del");
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_alias_setpos_del(col1 int, col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_alias_setpos_del values(100,'MySQL1')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into t_alias_setpos_del values(200,'MySQL2')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into t_alias_setpos_del values(300,'MySQL3')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into t_alias_setpos_del values(400,'MySQL4')");
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  

    rc = SQLSetStmtOption(hstmt,SQL_ROWSET_SIZE,4);
    mystmt(hstmt,rc);

    rc = SQLSetCursorName(hstmt,"venu",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = tmysql_exec(hstmt,"select col1 as id, col2 as name from t_alias_setpos_del");
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,100,&nlen);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,NULL,NULL);
    mystmt(hstmt,rc);   

    fprintf(stdout," row1 : %d, %s\n",nData[0],szData[0]);    
    fprintf(stdout," row2 : %d, %s\n",nData[1],szData[1]);    
    fprintf(stdout," row3 : %d, %s\n",nData[2],szData[2]);    
    fprintf(stdout," row4 : %d, %s\n",nData[3],szData[3]);    
    
    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt,1,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);
    
    fprintf(stdout," rows affected:%d\n",nlen);    

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_alias_setpos_del");
    mystmt(hstmt,rc);  

    my_assert( 3 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
    
    rc = SQLSetStmtOption(hstmt,SQL_ROWSET_SIZE,1);
    mystmt(hstmt,rc);

}

static void tmysql_setpos_pkdel1(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  long nData= 500;
  SQLLEN nlen;
  SQLCHAR szData[255]={0};
  SQLUINTEGER pcrow;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table tmysql_setpos1");
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table tmysql_setpos1(col1 int primary key, col3 int,col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(100,10,'MySQL1')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(200,20,'MySQL2')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(300,20,'MySQL3')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(400,20,'MySQL4')");
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLSetCursorName(hstmt,"venu",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = tmysql_exec(hstmt,"select col2,col3 from tmysql_setpos1");
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,100,&nlen);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,4,&pcrow,NULL);
    mystmt(hstmt,rc);

    fprintf(stdout," pcrow:%d\n",pcrow);  
   
    fprintf(stdout," row1:%d,%s\n",nData,szData);    
    
    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt,1,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);
    
    fprintf(stdout," rows affected:%d\n",nlen);    

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from tmysql_setpos1");
    mystmt(hstmt,rc);  

    my_assert( 3 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}
static void tmysql_setpos_pkdel2(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  long nData= 500;
  SQLLEN nlen;
  SQLCHAR szData[255]={0};
  SQLUINTEGER pcrow;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table tmysql_setpos1");
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table tmysql_setpos1(col1 int, col3 int,col2 varchar(30) primary key)");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(100,10,'MySQL1')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(200,20,'MySQL2')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(300,20,'MySQL3')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(400,20,'MySQL4')");
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc); 

    rc = SQLSetCursorName(hstmt,"venu",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = tmysql_exec(hstmt,"select col2,col3 from tmysql_setpos1");
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,100,&nlen);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,NULL);
    mystmt(hstmt,rc);
    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,NULL);
    mystmt(hstmt,rc);
    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,NULL);
    mystmt(hstmt,rc);
    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,NULL);
    mystmt(hstmt,rc);

    fprintf(stdout," pcrow:%d\n",pcrow,NULL);  
   
    fprintf(stdout," row1:%d,%s\n",nData,szData);    
    
    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt,1,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);
    
    fprintf(stdout," rows affected:%d\n",nlen);    

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from tmysql_setpos1");
    mystmt(hstmt,rc);  

    my_assert( 3 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}
static void t_refresh(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  
  printMessageHeader();

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,SQL_AUTOCOMMIT_OFF);
    mycon(hdbc,rc);

    tmysql_exec(hstmt,"drop table if exists t_refresh");    
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_refresh(col1 int ,col2 varchar(30)) TYPE = InnoDB");
    mystmt(hstmt,rc);   

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"insert into t_refresh values(10,'venu')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_refresh values(20,'mysql')");
    mystmt(hstmt,rc); 

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  

    rc = tmysql_exec(hstmt,"select * from t_refresh");
    mystmt(hstmt,rc);  

    my_assert( 2 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,SQL_AUTOCOMMIT_ON);
    mycon(hdbc,rc);
}
static void tmysql_setpos_pkdel3(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  long nData= 500;
  SQLLEN nlen;
  SQLCHAR szData[255]={0};
  SQLUINTEGER pcrow;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table tmysql_setpos1");
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table tmysql_setpos1(col1 int, col3 int,col2 varchar(30) primary key)");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(100,10,'MySQL1')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(200,20,'MySQL2')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(300,20,'MySQL3')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos1 values(400,20,'MySQL4')");
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc); 

    rc = SQLSetCursorName(hstmt,"venu",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = tmysql_exec(hstmt,"select col1 from tmysql_setpos1");
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,4,&pcrow,NULL);
    mystmt(hstmt,rc);

    fprintf(stdout," pcrow:%d\n",pcrow);  
   
    fprintf(stdout," row1:%d,%s\n",nData,szData);    
    
    rc = SQLSetPos(hstmt,1,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);
    
    fprintf(stdout," rows affected:%d\n",nlen);    

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from tmysql_setpos1");
    mystmt(hstmt,rc);  

    my_assert( 3 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}
static void t_mul_pkdel(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  long nData= 500;
  SQLLEN nlen;
  SQLCHAR szData[255]={0};
  SQLUINTEGER pcrow;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_mul_pkdel");
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_mul_pkdel(col1 int NOT NULL,col3 int,col2 varchar(30) NOT NULL,primary key(col1,col2))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_mul_pkdel values(100,10,'MySQL1')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into t_mul_pkdel values(200,20,'MySQL2')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into t_mul_pkdel values(300,20,'MySQL3')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into t_mul_pkdel values(400,20,'MySQL4')");
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLSetCursorName(hstmt,"venu",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = tmysql_exec(hstmt,"select col1,col2 from t_mul_pkdel");
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,NULL);
    mystmt(hstmt,rc);

    fprintf(stdout," pcrow:%d\n",pcrow);  
   
    fprintf(stdout," row1:%d,%s\n",nData,szData);    
    
    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt,1,SQL_DELETE,SQL_LOCK_UNLOCK);
    mystmt_err(hstmt,rc==SQL_ERROR,rc);

    rc = SQLSetPos(hstmt,1,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);
    
    fprintf(stdout," rows affected:%d\n",nlen);    

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_mul_pkdel");
    mystmt(hstmt,rc);  

    my_assert( 3 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}
static void t_mul_pkdel1(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  long nData= 500;
  SQLLEN nlen;
  SQLCHAR szData[255]={0};
  SQLUINTEGER pcrow;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_mul_pkdel");
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_mul_pkdel(col1 int NOT NULL,col3 int,col2 varchar(30) NOT NULL,primary key(col1,col2))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_mul_pkdel values(100,10,'MySQL1')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into t_mul_pkdel values(200,20,'MySQL2')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into t_mul_pkdel values(300,20,'MySQL3')");
    mystmt(hstmt,rc);  
    rc = tmysql_exec(hstmt,"insert into t_mul_pkdel values(400,20,'MySQL4')");
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc); 

    rc = SQLSetCursorName(hstmt,"venu",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = tmysql_exec(hstmt,"select col1 from t_mul_pkdel");
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,4,&pcrow,NULL);
    mystmt(hstmt,rc);

    fprintf(stdout," pcrow:%d\n",pcrow);  
   
    fprintf(stdout," row1:%d,%s\n",nData,szData);    
    
    rc = SQLSetPos(hstmt,4,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt_err(hstmt,rc==SQL_ERROR,rc);

    rc = SQLSetPos(hstmt,1,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);
    
    fprintf(stdout," rows affected:%d\n",nlen);    

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_mul_pkdel");
    mystmt(hstmt,rc);  

    my_assert( 3 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}
static void t_max_select(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  SQLCHAR szData[255];
  long i;
  
  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_max_select");
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_max_select(col1 int ,col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"insert into t_max_select values(?,?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_LONG,
                            SQL_INTEGER,0,0,&i,0,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,2,SQL_PARAM_INPUT,SQL_C_CHAR,
                            SQL_CHAR,0,0,szData,sizeof(szData),NULL);
    mystmt(hstmt,rc);

    fprintf(stdout," inserting 1000 rows, it will take some time\n");
    for(i = 1; i <= 1000; i++)
    {  
      fprintf(stdout," \r %d", i);
      sprintf((char *)szData,"MySQL%d",i);
      rc = SQLExecute(hstmt);
      mystmt(hstmt,rc);
    }
    fprintf(stdout,"\n");

    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);  

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from t_max_select");
    mystmt(hstmt,rc);  

    my_assert( 1000 == myrowcount(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}
static void t_tran(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  
  printMessageHeader();

    if (!server_supports_trans(hdbc))
      return;

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,SQL_AUTOCOMMIT_OFF);
    mycon(hdbc,rc);

    tmysql_exec(hstmt,"drop table if exists t_tran");    
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_tran(col1 int ,col2 varchar(30)) TYPE = InnoDB");
    mystmt(hstmt,rc);   

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"insert into t_tran values(10,'venu')");
    mystmt(hstmt,rc);
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"insert into t_tran values(20,'mysql')");
    mystmt(hstmt,rc); 

    rc = SQLTransact(NULL,hdbc,SQL_ROLLBACK);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  

    rc = tmysql_exec(hstmt,"select * from t_tran");
    mystmt(hstmt,rc);  

    my_assert( 1 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,SQL_AUTOCOMMIT_ON);
    mycon(hdbc,rc);
}
static void t_max_con(SQLHSTMT hstmt)
{
  SQLRETURN  rc;
  long       i, max_connections, used_connections, test_connections;
  SQLHENV    env;
  SQLHDBC    dbc[101];
  
  printMessageHeader();

  /* Get max_connections */
  rc = SQLExecDirect(hstmt, "SHOW VARIABLES like 'max_connections'",SQL_NTS);
  mystmt(hstmt, rc);

  rc = SQLFetch(hstmt);
  mystmt(hstmt, rc);

  rc = SQLGetData(hstmt, 2, SQL_C_LONG, &max_connections, 0, NULL);
  mystmt(hstmt,rc);

  rc = SQLFetch(hstmt);
  myassert(rc == SQL_NO_DATA_FOUND);

  SQLFreeStmt(hstmt, SQL_UNBIND);
  SQLFreeStmt(hstmt, SQL_CLOSE);
  fprintf(stdout,"\n total max connections supported: %d", max_connections);

  /* get number of connections used */
  rc = SQLExecDirect(hstmt, "SHOW STATUS like 'threads_connected'",SQL_NTS);
  mystmt(hstmt, rc);
  
  rc = SQLFetch(hstmt);
  mystmt(hstmt, rc);

  rc = SQLGetData(hstmt, 2, SQL_C_LONG, &used_connections, 0, NULL);
  mystmt(hstmt,rc);

  rc = SQLFetch(hstmt);
  myassert(rc == SQL_NO_DATA_FOUND);

  SQLFreeStmt(hstmt, SQL_UNBIND);
  SQLFreeStmt(hstmt, SQL_CLOSE);

  fprintf(stdout,"\n total connections used         : %d", used_connections);

  test_connections= max_connections-used_connections;
  fprintf(stdout,"\n total connections to be tested : %d", test_connections);

  if (test_connections >= 100)
  {
    fprintf(stdout,"\n test can't be performed due to max_connections are very high");
    return;
  }

  rc = SQLAllocEnv(&env);
  myenv(env,rc);

  rc = SQLSetEnvAttr(env,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0);
  myenv(env,rc);

  fprintf(stdout,"\n");
  
  for (i=0; i <= test_connections; i++)
  {
    rc = SQLAllocConnect(env, &dbc[i]);
    myenv(env,rc);

    fprintf(stdout," %d", i);
    rc = SQLConnect(dbc[i], mydsn, SQL_NTS, myuid, SQL_NTS,  mypwd, SQL_NTS);
    mycon(dbc[i],rc);
  }
  rc = SQLAllocConnect(env, &dbc[i]);
  myenv(env,rc);

  fprintf(stdout,"\n establishing '%d'th connection, it should fail",i);
  rc = SQLConnect(dbc[i], mydsn, SQL_NTS, myuid, SQL_NTS,  mypwd, SQL_NTS);
  myerror(rc, SQL_HANDLE_DBC, dbc[i], __FILE__, __LINE__);
  if (rc != SQL_ERROR)
    fprintf(stderr,"\n test failed");

  rc = SQLFreeHandle(SQL_HANDLE_DBC,dbc[i]);
  mycon(dbc[i],rc);

  fprintf(stdout,"\n freeing all connections\n");
  for (i=0; i<= test_connections; i++)
  {
    fprintf(stdout," %d", i);

    SQLDisconnect(dbc[i]);
    SQLFreeHandle(SQL_HANDLE_DBC,dbc[i]);
  }
  SQLFreeHandle(SQL_HANDLE_ENV,env);
  SQLFreeStmt(hstmt, SQL_UNBIND);
  SQLFreeStmt(hstmt, SQL_CLOSE);
}

static void t_tstotime(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  SQL_TIMESTAMP_STRUCT ts;
  SQLCHAR   szData[50];

  ts.day    = 02;
  ts.month  = 8;
  ts.year   = 2001;
  ts.hour   = 18;
  ts.minute = 20;
  ts.second = 45;
  ts.fraction = 05;   
  
  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_tstotime");
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_tstotime(col1 date ,col2 time, col3 timestamp(14))");
    mystmt(hstmt,rc);
        
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    /* TIMESTAMP TO DATE, TIME and TS CONVERSION */
    rc = SQLPrepare(hstmt,"insert into t_tstotime(col1,col2,col3) values(?,?,?)",SQL_NTS);
    mystmt(hstmt,rc);   
    
    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_TIMESTAMP,
                            SQL_DATE,0,0,&ts,sizeof(ts),NULL);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,2,SQL_PARAM_INPUT,SQL_C_TIMESTAMP,
                            SQL_TIME,0,0,&ts,sizeof(ts),NULL);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,3,SQL_PARAM_INPUT,SQL_C_TIMESTAMP,
                            SQL_TIMESTAMP,0,0,&ts,sizeof(ts),NULL);
    mystmt(hstmt,rc);

    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);

    ts.day++;
    ts.month++;
    ts.year++;
    ts.hour++;
    ts.minute++;
    ts.second++;
    ts.fraction++;   

    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);  

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from t_tstotime");
    mystmt(hstmt,rc);  

    my_assert( 2 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
    
    rc = tmysql_exec(hstmt,"select * from t_tstotime");
    mystmt(hstmt,rc);  

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    fprintf(stdout,"\n");

    my_assert(!strcmp("2001-08-02",my_fetch_str(hstmt,(SQLCHAR *)szData,1)));
    my_assert(!strcmp("18:20:45",my_fetch_str(hstmt,(SQLCHAR *)szData,2)));
    my_assert(!strcmp("2001-08-02 18:20:45",my_fetch_str(hstmt,(SQLCHAR *)szData,3)));

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    fprintf(stdout,"\n");

    my_assert(!strcmp("2002-09-03",my_fetch_str(hstmt,(SQLCHAR *)szData,1)));
    my_assert(!strcmp("19:21:46",my_fetch_str(hstmt,(SQLCHAR *)szData,2)));
    my_assert(!strcmp("2002-09-03 19:21:46",my_fetch_str(hstmt,(SQLCHAR *)szData,3)));

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}
static void t_tstotime1(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  SQLCHAR ts[40] = "2001-08-02 18:20:45.05", szData[50];
  
  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_tstotime1");
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_tstotime1(col1 date ,col2 time, col3 timestamp(14))");
    mystmt(hstmt,rc);
        
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    /* TIMESTAMP TO DATE, TIME and TS CONVERSION */
    rc = SQLPrepare(hstmt,"insert into t_tstotime1 values(?,?,?)",SQL_NTS);
    mystmt(hstmt,rc);
    
    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_CHAR,
                            SQL_DATE,0,0,&ts,sizeof(ts),NULL);
    mystmt(hstmt,rc);
    rc = SQLBindParameter(hstmt,2,SQL_PARAM_INPUT,SQL_C_CHAR,
                            SQL_TIME,0,0,&ts,sizeof(ts),NULL);
    mystmt(hstmt,rc);
    rc = SQLBindParameter(hstmt,3,SQL_PARAM_INPUT,SQL_C_CHAR,
                            SQL_TIMESTAMP,0,0,&ts,sizeof(ts),NULL);
    mystmt(hstmt,rc);
    
    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);  

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from t_tstotime1");
    mystmt(hstmt,rc);  

    my_assert( 1 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
    
    rc = tmysql_exec(hstmt,"select * from t_tstotime1");
    mystmt(hstmt,rc);  

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    fprintf(stdout,"\n");

    my_assert(!strcmp("2001-08-02",my_fetch_str(hstmt,(SQLCHAR *)szData,1)));
    my_assert(!strcmp("2001-08-02 18:20:45",my_fetch_str(hstmt,(SQLCHAR *)szData,3)));

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}

static void t_enumset(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  SQLCHAR szEnum[40]="MYSQL_E1";
  SQLCHAR szSet[40]="THREE,ONE,TWO";
  
  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_enumset");
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_enumset(col1 enum('MYSQL_E1','MYSQL_E2'),col2 set('ONE','TWO','THREE'))");
    mystmt(hstmt,rc);
        
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into t_enumset values('MYSQL_E2','TWO,THREE')",SQL_NTS);
    mystmt(hstmt,rc);   

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  

    rc = SQLPrepare(hstmt,"insert into t_enumset values(?,?)",SQL_NTS);
    mystmt(hstmt,rc);   

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_CHAR,SQL_CHAR,0,0,&szEnum,sizeof(szEnum),NULL);
    mystmt(hstmt,rc);   

    rc = SQLBindParameter(hstmt,2,SQL_PARAM_INPUT,SQL_C_CHAR,SQL_CHAR,0,0,&szSet,sizeof(szSet),NULL);
    mystmt(hstmt,rc);   

    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc); 

    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);  

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from t_enumset");
    mystmt(hstmt,rc);  

    my_assert( 2 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}

static void t_bigint(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc; 
  SQLCHAR id[20]="999";
  SQLLEN nlen;
  
  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_bingint");
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_bingint(id int(20) NOT NULL auto_increment,name varchar(20) default 'venu', primary key(id))");
    mystmt(hstmt,rc);
        
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    /* TIMESTAMP TO DATE, TIME and TS CONVERSION */
    rc = tmysql_prepare(hstmt,"insert into t_bingint values(?,'venuxyz')");
    mystmt(hstmt,rc);

    nlen = 4;
    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_LONG,
                            SQL_BIGINT,20,0,&id,0,&nlen);
    mystmt(hstmt,rc);

    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_bingint values(10,'mysql1')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_bingint values(20,'mysql2')");
    mystmt(hstmt,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLSpecialColumns(hstmt,SQL_ROWVER,NULL,SQL_NTS,NULL,SQL_NTS,
                          "t_bingint",SQL_NTS,SQL_SCOPE_TRANSACTION,SQL_NULLABLE);

    mycon(hdbc,rc);

    my_assert( 0 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLColumns(hstmt,NULL,SQL_NTS,NULL,SQL_NTS,"t_bingint",SQL_NTS,NULL,SQL_NTS);

    mycon(hdbc,rc);

    my_assert( 2 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLStatistics(hstmt,NULL,SQL_NTS,NULL,SQL_NTS,"t_bingint",SQL_NTS,SQL_INDEX_ALL,SQL_QUICK);

    mycon(hdbc,rc);

    my_assert( 1 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLGetTypeInfo(hstmt,SQL_BIGINT);
    mycon(hdbc,rc);

    my_assert( 4 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLGetTypeInfo(hstmt,SQL_BIGINT);
    mycon(hdbc,rc);

    my_assert( 4 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_bingint");
    mystmt(hstmt,rc);  

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc); 

    rc = SQLGetData(hstmt,1,SQL_C_DEFAULT,&id,10,&nlen);
    mystmt(hstmt,rc); 

    fprintf(stdout,"\n id:%s,nlen:%d,%d\n",id,nlen,sizeof(SQL_BIGINT));
    
    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}
static void t_odbc3_envattr()
{
  SQLRETURN rc;
  SQLHENV henv;
  SQLHDBC hdbc;
  SQLINTEGER ov_version;

  printMessageHeader();

    rc = SQLAllocEnv(&henv);
    myenv(henv,rc);

	  rc = SQLSetEnvAttr(henv,SQL_ATTR_OUTPUT_NTS,(SQLPOINTER)SQL_FALSE,0);
    myenv_err(henv,rc == SQL_ERROR,rc);

	  rc = SQLSetEnvAttr(henv,SQL_ATTR_OUTPUT_NTS,(SQLPOINTER)SQL_TRUE,0);
    myenv(henv,rc);

    rc = SQLAllocConnect(henv,&hdbc);
    myenv(henv,rc);

    rc = SQLSetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0);
    myenv_err(henv,rc == SQL_ERROR,rc);
	
    rc = SQLGetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)&ov_version,0,0);
    myenv(henv,rc);
    fprintf(stdout,"default odbc version:%d\n",ov_version);
    my_assert(ov_version == SQL_OV_ODBC2);

    rc = SQLFreeConnect(hdbc);
    mycon(hdbc,rc);

    rc = SQLSetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0);
    myenv(henv,rc);

    rc = SQLGetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)&ov_version,0,0);
    myenv(henv,rc);
    fprintf(stdout,"new odbc version:%d\n",ov_version);
    my_assert(ov_version == SQL_OV_ODBC3);

    rc = SQLFreeEnv(henv);
    myenv(henv,rc);

    rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    myenv(henv,rc);

    rc = SQLSetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0);
    myenv(henv,rc);

    rc = SQLAllocConnect(henv,&hdbc);
    myenv(henv,rc);

    rc = SQLSetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0);
    myenv_err(henv,rc == SQL_ERROR,rc);

    rc = SQLGetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,&ov_version,0,0);
    myenv(henv,rc);
    fprintf(stdout,"default odbc version:%d\n",ov_version);
    my_assert(ov_version == (SQLPOINTER)SQL_OV_ODBC3);

    rc = SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    mycon(hdbc,rc);

    rc = SQLFreeHandle(SQL_HANDLE_ENV, henv);
    myenv(henv,rc);
}
static void t_odbc3_handle()
{
  SQLRETURN rc;
  SQLHENV henv;
  SQLHDBC hdbc;
  SQLHSTMT hstmt;
  SQLINTEGER ov_version;

  printMessageHeader();

    rc = SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&henv);
    myenv(henv,rc);

#if 0
    rc = SQLAllocHandle(SQL_HANDLE_DBC,henv,&hdbc);
    myenv_err(henv,rc == SQL_ERROR,rc);

    rc = SQLGetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,&ov_version,0,0);
    myenv(henv,rc);
    fprintf(stdout,"\n default odbc version:%d",ov_version);
    my_assert(ov_version == (SQLPOINTER)SQL_OV_ODBC2);
#endif

    rc = SQLSetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0);
    myenv(henv,rc);

    rc = SQLAllocHandle(SQL_HANDLE_DBC,henv,&hdbc);
    myenv(henv,rc);

    rc = SQLSetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0);
    myenv_err(henv,rc == SQL_ERROR,rc);

    rc = SQLGetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)&ov_version,0,0);
    myenv(henv,rc);
    fprintf(stdout,"\n default odbc version:%d",ov_version);
    my_assert(ov_version == SQL_OV_ODBC3);

    rc = SQLConnect(hdbc, mydsn, SQL_NTS, myuid, SQL_NTS,  mypwd, SQL_NTS);
    mycon(hdbc,rc);

    rc = SQLAllocHandle(SQL_HANDLE_STMT,hdbc,&hstmt);
    mycon(hdbc, rc);

    rc = SQLDisconnect(hdbc);
    mycon(hdbc, rc);

    rc = SQLFreeHandle(SQL_HANDLE_DBC,hdbc);
    mycon(hdbc,rc);

    rc = SQLFreeHandle(SQL_HANDLE_ENV,henv);
    myenv(henv,rc);
}
static void t_getcursor(SQLHDBC hdbc)
{
  SQLRETURN rc; 
  SQLHSTMT hstmt1,hstmt2,hstmt3;
  SQLCHAR curname[50];
  SQLSMALLINT nlen;

  printMessageHeader();
    
    rc = SQLAllocHandle(SQL_HANDLE_STMT,hdbc,&hstmt1);
    mycon(hdbc, rc);
    rc = SQLAllocHandle(SQL_HANDLE_STMT,hdbc,&hstmt2);
    mycon(hdbc, rc);
    rc = SQLAllocHandle(SQL_HANDLE_STMT,hdbc,&hstmt3);
    mycon(hdbc, rc);

    rc = SQLGetCursorName(hstmt1,curname,50,&nlen);
    if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    {
      fprintf(stdout,"\n default cursor name  : %s(%d)",curname,nlen);
      myassert(nlen == 8);
      myassert(!strcmp(curname,"SQL_CUR0"));

      rc = SQLGetCursorName(hstmt3,curname,50,&nlen);
      mystmt(hstmt1,rc);
      fprintf(stdout,"\n default cursor name  : %s(%d)",curname,nlen);

      rc = SQLGetCursorName(hstmt1,curname,4,&nlen);
      mystmt_err(hstmt1,rc == SQL_SUCCESS_WITH_INFO, rc);
      fprintf(stdout,"\n truncated cursor name: %s(%d)",curname,nlen);
      myassert(nlen == 8);
      myassert(!strcmp(curname,"SQL"));

      rc = SQLGetCursorName(hstmt1,curname,0,&nlen);
      mystmt_err(hstmt1,rc == SQL_SUCCESS_WITH_INFO, rc);
      fprintf(stdout,"\n untouched cursor name: %s(%d)",curname,nlen);
      myassert(nlen == 8);    

      rc = SQLGetCursorName(hstmt1,curname,8,&nlen);
      mystmt_err(hstmt1,rc == SQL_SUCCESS_WITH_INFO, rc);
      fprintf(stdout,"\n truncated cursor name: %s(%d)",curname,nlen);
      myassert(nlen == 8); 
      myassert(!strcmp(curname,"SQL_CUR"));

      rc = SQLGetCursorName(hstmt1,curname,9,&nlen);
      mystmt(hstmt1,rc);
      fprintf(stdout,"\n full cursor name     : %s(%d)",curname,nlen);
      myassert(nlen == 8); 
      myassert(!strcmp(curname,"SQL_CUR0"));
    }

    rc = SQLSetCursorName(hstmt1,"venucur123",7);
    mystmt(hstmt1,rc);

    rc = SQLGetCursorName(hstmt1,curname,8,&nlen);
    mystmt(hstmt1,rc);
    fprintf(stdout,"\n full setcursor name  : %s(%d)",curname,nlen);
    myassert(nlen == 7); 
    myassert(!strcmp(curname,"venucur"));

    rc = SQLFreeHandle(SQL_HANDLE_STMT,hstmt1);
    mystmt(hstmt1,rc);
}
static void t_getcursor1(SQLHDBC hdbc)
{
  SQLRETURN rc; 
  SQLHSTMT hstmt1;
  SQLCHAR curname[50];
  SQLSMALLINT nlen,index;;
  
  printMessageHeader();
    
  for(index=0; index < 100; index++)
  {
    rc = SQLAllocHandle(SQL_HANDLE_STMT,hdbc,&hstmt1);
    mycon(hdbc, rc);

    rc = SQLGetCursorName(hstmt1,curname,50,&nlen);
    if (rc != SQL_SUCCESS)
      break;
    fprintf(stdout,"\n %s(%d) ",curname,nlen);

    rc = SQLFreeHandle(SQL_HANDLE_STMT,hstmt1);
    mystmt(hstmt1,rc);
  }
}
static void t_gettypeinfo(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  SQLSMALLINT pccol;
  
  printMessageHeader();

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLGetTypeInfo(hstmt,SQL_ALL_TYPES);
    mystmt(hstmt,rc);

    rc = SQLNumResultCols(hstmt,&pccol);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n total columns: %d",pccol);
    myassert(pccol == 19);
    myresult(hstmt);

    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);    
}
static void t_getinfo(SQLHDBC hdbc)
{
  SQLRETURN rc;
  SQLCHAR   rgbValue[100];
  SQLSMALLINT pcbInfo;
  
  printMessageHeader();

    rc = SQLGetInfo(hdbc,SQL_DRIVER_ODBC_VER,rgbValue,100,&pcbInfo);
    mycon(hdbc,rc);
    fprintf(stdout,"\n SQL_DRIVER_ODBC_VER: %s(%d)",rgbValue,pcbInfo);
}
static void t_stmt_attr_status(SQLHDBC hdbc,SQLHSTMT hstmt)
{
  SQLRETURN rc;  
  SQLUSMALLINT rowStatusPtr[3];
  SQLUINTEGER rowsFetchedPtr;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_stmtstatus");
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_stmtstatus(id int, name char(20))");
    mystmt(hstmt,rc);
        
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"insert into t_stmtstatus values(10,'data1')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into t_stmtstatus values(20,'data2')");
    mystmt(hstmt,rc);
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_CURSOR_SCROLLABLE,(SQLPOINTER)SQL_NONSCROLLABLE,0);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_stmtstatus");
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROWS_FETCHED_PTR,&rowsFetchedPtr,0);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROW_STATUS_PTR,&rowStatusPtr,0);
    mystmt(hstmt,rc);    

    rc = SQLFetchScroll(hstmt,SQL_FETCH_ABSOLUTE,2);
    mystmt_err(hstmt,rc == SQL_ERROR,rc);    

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);    

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_CURSOR_SCROLLABLE,(SQLPOINTER)SQL_SCROLLABLE,0);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_stmtstatus");
    mystmt(hstmt,rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_ABSOLUTE,2);
    mystmt(hstmt,rc);

    printMessage("\n total rows fetched: %d",rowsFetchedPtr);
    printMessage("\n row 0 status      : %d",rowStatusPtr[0]);
    printMessage("\n row 1 status      : %d",rowStatusPtr[1]);
    printMessage("\n row 2 status      : %d",rowStatusPtr[2]);
    myassert(rowsFetchedPtr == 1);
    myassert(rowStatusPtr[0] == 0);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROWS_FETCHED_PTR,(SQLPOINTER)0,0);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROW_STATUS_PTR,(SQLPOINTER)0,0);
    mystmt(hstmt,rc); 
}
static void t_max_rows(SQLHDBC hdbc,SQLHSTMT hstmt)
{
  SQLRETURN rc; 
  long i;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_max_rows");
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_max_rows(id int)");
    mystmt(hstmt,rc);
        
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLPrepare(hstmt,"insert into t_max_rows values(?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_ULONG,SQL_INTEGER,0,0,&i,0,NULL);
    mystmt(hstmt,rc);

    for(i=0; i < 10; i++)
    {
      rc = SQLExecute(hstmt);
      mystmt(hstmt,rc);
    }

    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt,SQL_CLOSE);
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_MAX_ROWS,(SQLPOINTER)0,0);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select count(*) from t_max_rows");
    mystmt(hstmt,rc);
    myassert( 1 == myresult(hstmt) );
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = tmysql_exec(hstmt,"select * from t_max_rows");
    mystmt(hstmt,rc);
    myassert( 10 == myresult(hstmt) );
    SQLFreeStmt(hstmt,SQL_CLOSE);

    /* MAX rows through connection attribute */    
    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_MAX_ROWS,(SQLPOINTER)5,0);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_max_rows");
    mystmt(hstmt,rc);
    myassert( 5 == myrowcount(hstmt));

    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_MAX_ROWS,(SQLPOINTER)15,0);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_max_rows");
    mystmt(hstmt,rc);
    myassert( 10 == myrowcount(hstmt));

    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_MAX_ROWS,(SQLPOINTER)0,0);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_max_rows");
    mystmt(hstmt,rc);
    myassert( 10 == myrowcount(hstmt));

    SQLFreeStmt(hstmt,SQL_CLOSE);

    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}
static void t_prepare(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  long      nidata= 200, nodata;
  SQLLEN    nlen;
  char      szodata[20],szidata[20]="MySQL";
  short     pccol;

  printMessageHeader();  

    SQLFreeStmt(hstmt,SQL_CLOSE);

    tmysql_exec(hstmt,"drop table t_prepare");

    rc = tmysql_exec(hstmt,"create table t_prepare(col1 int primary key, col2 varchar(30), col3 set(\"one\", \"two\"))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_prepare values(100,'venu','one')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_prepare values(200,'MySQL','two')");
    mystmt(hstmt,rc);  

    rc = SQLPrepare(hstmt,"select * from t_prepare where col2 = ? AND col1 = ?",SQL_NTS);
    mystmt(hstmt,rc); 

    rc = SQLNumResultCols(hstmt,&pccol);
    mystmt(hstmt,rc); 
   
    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nodata,0,&nlen);
    mystmt(hstmt,rc);  

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szodata,200,&nlen);
    mystmt(hstmt,rc); 
    
    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT, SQL_C_CHAR,SQL_VARCHAR,
                          0,0,szidata,20,&nlen);
    mystmt(hstmt,rc); 

    rc = SQLBindParameter(hstmt,2,SQL_PARAM_INPUT, SQL_C_LONG,SQL_INTEGER,
                          0,0,&nidata,20,NULL);
    mystmt(hstmt,rc); 

    nlen= strlen(szidata);
    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);  

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);  
    
    fprintf(stdout," outdata: %d, %s(%d)\n", nodata,szodata,nlen);
    my_assert(nodata == 200);

    rc = SQLFetch(hstmt);
    my_assert(rc == SQL_NO_DATA_FOUND);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);
}
static void t_prepare1(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  long      nidata = 1000;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_prepare1");

    rc = tmysql_exec(hstmt,"create table t_prepare1(col1 int primary key, col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_prepare1 values(100,'venu')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_prepare1 values(200,'MySQL')");
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  

    rc = tmysql_prepare(hstmt,"insert into t_prepare1(col1) values(?)");
    mystmt(hstmt,rc); 

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT, SQL_C_LONG,SQL_INTEGER,
                          0,0,&nidata,0,NULL);
    mystmt(hstmt,rc); 

    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);    

    rc = SQLExecDirect(hstmt,"SELECT * FROM t_prepare1",SQL_NTS);
    mystmt(hstmt,rc);    

    myassert(3 == myresult(hstmt));/* unless prepare is supported..*/

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}
static void test_diagrec( SQLSMALLINT HandleType,SQLHANDLE Handle,
			             SQLSMALLINT RecNumber,SQLSMALLINT BufferLength,
			             SQLRETURN return_value_expected)
{
  SQLRETURN rc;
  SQLCHAR   sqlstate[6]={0};
  SQLCHAR   message[255]={0};
  SQLINTEGER native_err=0;
  SQLSMALLINT msglen=0;

  rc = SQLGetDiagRec(HandleType,Handle,RecNumber,
		     (char *)&sqlstate,&native_err,
		     (char *)&message,BufferLength,&msglen);

  fprintf(stdout,"\n %d@%s(%d)",rc,message,msglen);
  myassert(return_value_expected == rc);
}
static void t_diagrec(SQLHENV henv,SQLHDBC hdbc,SQLHSTMT hstmt)
{
  SQLRETURN rc;

  printMessageHeader();

  fprintf(stdout," ** SQL_HANDLE_STMT ** \n");
		
  rc = SQLExecDirect(hstmt,"DROP TABLE ODBC3_NON_EXISTANTi_TAB",SQL_NTS);
  myassert(rc == SQL_ERROR);

  test_diagrec(SQL_HANDLE_STMT,hstmt,2,0,SQL_NO_DATA_FOUND);      
  test_diagrec(SQL_HANDLE_STMT,hstmt,1,255,SQL_SUCCESS);
  test_diagrec(SQL_HANDLE_STMT,hstmt,1,0,SQL_SUCCESS_WITH_INFO);
  test_diagrec(SQL_HANDLE_STMT,hstmt,1,10,SQL_SUCCESS_WITH_INFO);
  test_diagrec(SQL_HANDLE_STMT,hstmt,1,-1,SQL_ERROR);			
#if 0
  fprintf(stdout," ** SQL_HANDLE_ENV **\n");

  rc = SQLFreeEnv(henv);
  myassert(rc == SQL_ERROR);

  test_diagrec(SQL_HANDLE_ENV,henv,2,0,SQL_NO_DATA_FOUND);
  test_diagrec(SQL_HANDLE_ENV,henv,1,255,SQL_SUCCESS);
  test_diagrec(SQL_HANDLE_ENV,henv,1,0,SQL_SUCCESS_WITH_INFO);
  test_diagrec(SQL_HANDLE_ENV,henv,1,10,SQL_SUCCESS_WITH_INFO);
  test_diagrec(SQL_HANDLE_ENV,henv,1,-1,SQL_ERROR);	

  fprintf(stdout," ** SQL_HANDLE_DBC **\n");
  
  rc = SQLFreeConnect(hdbc);
  myassert(rc == SQL_ERROR);
			
  test_diagrec(SQL_HANDLE_DBC,hdbc,2,0,SQL_NO_DATA_FOUND);
  test_diagrec(SQL_HANDLE_DBC,hdbc,1,255,SQL_SUCCESS);
  test_diagrec(SQL_HANDLE_DBC,hdbc,1,0,SQL_SUCCESS_WITH_INFO);
  test_diagrec(SQL_HANDLE_DBC,hdbc,1,10,SQL_SUCCESS_WITH_INFO);
  test_diagrec(SQL_HANDLE_DBC,hdbc,1,-1,SQL_ERROR);			
#endif
}
#if 0
static void t_diaglist(SQLHDBC hdbc,SQLHSTMT hstmt)
{
  SQLRETURN rc;
  SQLCHAR   sqlstate[6]={0};
  SQLCHAR   message[255]={0};
  SQLINTEGER native_err=0;
  SQLSMALLINT msglen=0;

  printMessageHeader();
		
  rc = SQLSetPos(hstmt,100,SQL_DELETE,SQL_LOCK_NO_CHANGE);
  myassert(rc == SQL_ERROR);

  rc = SQLGetDiagRec(3,hstmt,1,
           (char *)&sqlstate,&native_err,
           (char *)&message,255,&msglen);
  mystmt(hstmt,rc);
  fprintf(stdout,"\n %d@%s(%d)",rc,message,msglen);

  rc = SQLExecDirect(hstmt,"SELECT * FROM tupd",SQL_NTS);
  mystmt(hstmt,rc);

  rc = SQLSetPos(hstmt,3,10,SQL_LOCK_NO_CHANGE);
  myassert(rc == SQL_ERROR);

  rc = SQLGetDiagRec(3,hstmt,1,
	           (char *)&sqlstate,&native_err,
                   (char *)&message,255,&msglen);
  mystmt(hstmt,rc);
  fprintf(stdout,"\n %d@%s(%d)",rc,message,msglen);

  SQLFreeStmt(hstmt,SQL_CLOSE);
}
#endif

static void t_scroll(SQLHDBC hdbc,SQLHSTMT hstmt)
{
  SQLRETURN rc;
  long i;

  printMessageHeader();

  tmysql_exec(hstmt,"drop table t_scroll");

    rc = tmysql_exec(hstmt,"create table t_scroll(col1 int)");
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"insert into t_scroll values(?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT, SQL_C_ULONG,
                          SQL_INTEGER,0,0,&i,0,NULL);
    mystmt(hstmt,rc);

    for( i = 1; i <= 5; i++ )
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
    my_assert(i == 1); /* MyODBC .39 returns 2 instead of 1 */

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
static void t_acc_crash(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN   rc;
  long id;
  SQLCHAR     name[20], data[30];
  SQL_TIMESTAMP_STRUCT ts;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table if exists t_acc_crash");
    rc = tmysql_exec(hstmt,"create table t_acc_crash(id int(11) not null auto_increment,\
                                                     name char(20),\
                                                     ts date,\
                                                     primary key(id))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_acc_crash(id,name) values(1,'venu')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_acc_crash(name) values('monty')");
    mystmt(hstmt,rc);  

    rc = tmysql_exec(hstmt,"insert into t_acc_crash(name) values('mysql')");
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc); 
    
    rc = SQLSetStmtOption(hstmt, SQL_ROWSET_SIZE, 1);
    mystmt(hstmt,rc);
 
    rc = tmysql_exec(hstmt,"select * from t_acc_crash order by id asc");
    mystmt(hstmt,rc); 

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&id,0,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,(SQLCHAR *)&name,20,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,3,SQL_C_DATE,&ts,30,NULL);
    mystmt(hstmt,rc);    
    
    rc = SQLFetchScroll(hstmt,SQL_FETCH_FIRST,1);
    mystmt(hstmt,rc);

    id = 9;
    strcpy(name,"updated");
    ts.year=2010;ts.month=9;ts.day=25;

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt,1,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from t_acc_crash order by id desc");
    mystmt(hstmt,rc);  

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc); 
    
    my_assert(9 == my_fetch_int(hstmt,1));
    my_assert(!strcmp("updated", my_fetch_str(hstmt,data,2)));
    my_assert(!strcmp("2010-09-25", my_fetch_str(hstmt,data,3)));
    
    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}

static void tmysql_pcbvalue(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN   rc;
  long nodata;
  SQLLEN nlen, slen,tlen;
  SQLCHAR     szdata[20],sztdata[100];  

  printMessageHeader();

    tmysql_exec(hstmt,"drop table tmysql_pcbvalue");

    rc = tmysql_exec(hstmt,"create table tmysql_pcbvalue(col1 int primary key, col2 varchar(10),col3 text)");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_pcbvalue values(100,'venu','mysql')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_pcbvalue values(200,'monty','mysql2')");
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  

    /* MS SQL Server to work...*/
    SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER) SQL_CONCUR_ROWVER, 0);
    SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_KEYSET_DRIVEN, 0);

    rc = SQLExecDirect(hstmt,"select * from tmysql_pcbvalue",SQL_NTS);
    mystmt(hstmt,rc); 

    rc = SQLSetStmtOption(hstmt,SQL_ROWSET_SIZE,1);
    mystmt(hstmt,rc);
   
    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nodata,0,&nlen);
    mystmt(hstmt,rc);  

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szdata,20,&slen);
    mystmt(hstmt,rc); 

    rc = SQLBindCol(hstmt,3,SQL_C_CHAR,sztdata,100,&tlen);
    mystmt(hstmt,rc); 

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_FIRST,1,0,NULL);
    mystmt(hstmt,rc);      
    fprintf(stdout," row1: %d(%d), %s(%d), %s(%d)\n", nodata,nlen,szdata,slen,sztdata,tlen);    

    strcpy(szdata,"updated-one");

    rc = SQLSetPos(hstmt,1,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);  
    
    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,0,0);
    mystmt(hstmt,rc);      
    
    fprintf(stdout," row2: %d(%d), %s(%d),%s(%d)\n", nodata,nlen,szdata,slen,sztdata,tlen);    

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,0,0);
    myassert(rc == SQL_NO_DATA_FOUND);
        
    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLExecDirect(hstmt,"select * from tmysql_pcbvalue",SQL_NTS);
    mystmt(hstmt,rc); 

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc); 

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szdata,20,&slen);
    mystmt(hstmt,rc); 
    fprintf(stdout," updated data:%s(%d)",szdata,slen);
    my_assert(slen == 4);
    my_assert(strcmp(szdata,"upda")==0);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);
}
static void my_setpos_upd_pk_order(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  long nData = 500;
  SQLLEN nlen;
  SQLCHAR szData[255]={0};
  SQLUINTEGER pcrow;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table my_setpos_upd_pk_order");
    rc = tmysql_exec(hstmt,"create table my_setpos_upd_pk_order(col1 int not null, col2 varchar(30) NOT NULL, primary key(col2,col1))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into my_setpos_upd_pk_order values(100,'MySQL1')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into my_setpos_upd_pk_order values(200,'MySQL2')");
    mystmt(hstmt,rc); 

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc); 

    rc = SQLSetCursorName(hstmt,"venu",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = tmysql_exec(hstmt,"select * from my_setpos_upd_pk_order");
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,2,&pcrow,NULL);
    mystmt(hstmt,rc);

    fprintf(stdout," pcrow:%d\n",pcrow);  
   
    fprintf(stdout," row1:%d,%s\n",nData,szData);    
    
    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    nData = 1000;
    strcpy((char *)szData , "updated");

    rc = SQLSetPos(hstmt,1,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);
    
    fprintf(stdout," rows affected:%d\n",nlen);    

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from my_setpos_upd_pk_order");
    mystmt(hstmt,rc);  

    myresult(hstmt);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
    
    rc = SQLExecDirect(hstmt,"DELETE FROM my_setpos_upd_pk_order WHERE col2 = 'updated'",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n total rows affceted:%d",nlen);
    my_assert(nlen == 1);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}
static void my_setpos_upd_pk_order1(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  long nData = 500;
  SQLCHAR szData[255]={0};
  SQLUINTEGER pcrow;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table my_setpos_upd_pk_order");
    rc = tmysql_exec(hstmt,"create table my_setpos_upd_pk_order(col1 int not null, col2 varchar(30) NOT NULL, col3 int not null, primary key(col2,col1,col3))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into my_setpos_upd_pk_order values(100,'MySQL1',1)");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into my_setpos_upd_pk_order values(200,'MySQL2',2)");
    mystmt(hstmt,rc); 

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc); 

    rc = SQLSetCursorName(hstmt,"venu",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = tmysql_exec(hstmt,"select * from my_setpos_upd_pk_order");
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,100,NULL);
    mystmt(hstmt,rc);  

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,2,&pcrow,NULL);
    mystmt(hstmt,rc);

    fprintf(stdout," pcrow:%d\n",pcrow);  
   
    fprintf(stdout," row1:%d,%s\n",nData,szData);    
    
    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    nData = 1000;
    strcpy((char *)szData , "updated");

    rc = SQLSetPos(hstmt,1,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt_err(hstmt,rc == SQL_ERROR, rc);    

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}
static void tmysql_pos_update_ex(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  SQLHSTMT hstmt1;
  SQLUINTEGER pcrow;
  SQLUSMALLINT rgfRowStatus;
  SQLCHAR cursor[30],sql[100],data[]="updated";

  printMessageHeader();

    rc = SQLAllocStmt(hdbc,&hstmt1);
    mycon(hdbc,rc);

    tmysql_exec(hstmt,"drop table t_pos_updex");
    rc = tmysql_exec(hstmt,"create table t_pos_updex(col1 int NOT NULL primary key, col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_pos_updex values(100,'venu')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_pos_updex values(200,'MySQL')");
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc); 

    rc = tmysql_exec(hstmt,"select * from t_pos_updex");
    mystmt(hstmt,rc); 
    
    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,2,&pcrow,NULL);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt1,1,SQL_PARAM_INPUT,SQL_C_CHAR,SQL_CHAR,0,0,data,20,NULL);
    mystmt(hstmt1,rc);

    rc = SQLGetCursorName(hstmt,cursor,30,NULL);
    mystmt(hstmt,rc); 

    sprintf(sql,"UPDATE t_pos_updex SET col1= 999, col2 = ? WHERE CURRENT OF %s",cursor);

    rc = SQLExecDirect(hstmt1,sql,SQL_NTS);
    mystmt(hstmt1,rc);  

    SQLNumResultCols(hstmt1,&rgfRowStatus);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    rc = SQLFreeStmt(hstmt1,SQL_CLOSE);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from t_pos_updex");
    mystmt(hstmt,rc);  

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);  

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);  
    {
      SQLCHAR szData[20];
      my_assert(999 == my_fetch_int(hstmt,1));
      my_assert(!strcmp("updated",my_fetch_str(hstmt,szData,2)));
    }

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt1,SQL_DROP);
    mystmt(hstmt1,rc);
}

static void tmysql_pos_update_ex1(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  SQLHSTMT hstmt1;
  SQLUINTEGER pcrow;
  SQLUSMALLINT rgfRowStatus;
  SQLCHAR cursor[30],sql[100],data[]="updated";

  printMessageHeader();

    rc = SQLAllocStmt(hdbc,&hstmt1);
    mycon(hdbc,rc);

    tmysql_exec(hstmt,"drop table t_pos_updex");
    rc = tmysql_exec(hstmt,"create table t_pos_updex(col1 int, col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_pos_updex values(100,'venu')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_pos_updex values(200,'MySQL')");
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc); 

    rc = tmysql_exec(hstmt,"select * from t_pos_updex");
    mystmt(hstmt,rc); 
    
    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,2,&pcrow,NULL);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt1,1,SQL_PARAM_INPUT,SQL_C_CHAR,SQL_CHAR,0,0,data,20,NULL);
    mystmt(hstmt1,rc);

    rc = SQLGetCursorName(hstmt,cursor,30,NULL);
    mystmt(hstmt,rc); 

    sprintf(sql,"UPDATE t_pos_updex SET col1= 999, col2 = ? WHERE CURRENT OF %s",cursor);

    rc = SQLExecDirect(hstmt1,sql,SQL_NTS);
    mystmt(hstmt1,rc);  

    SQLNumResultCols(hstmt1,&rgfRowStatus);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    rc = SQLFreeStmt(hstmt1,SQL_CLOSE);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from t_pos_updex");
    mystmt(hstmt,rc);  

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);  

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);  
    {
      SQLCHAR szData[20];
      my_assert(999 == my_fetch_int(hstmt,1));
      my_assert(!strcmp("updated",my_fetch_str(hstmt,szData,2)));
    }

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt1,SQL_DROP);
    mystmt(hstmt1,rc);
}

static void tmysql_pos_update_ex2(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  SQLHSTMT hstmt1;
  SQLUINTEGER pcrow;
  SQLUSMALLINT rgfRowStatus;
  SQLCHAR cursor[30],sql[100],data[]="updated";

  printMessageHeader();

    rc = SQLAllocStmt(hdbc,&hstmt1);
    mycon(hdbc,rc);

    tmysql_exec(hstmt,"drop table t_pos_updex");
    rc = tmysql_exec(hstmt,"create table t_pos_updex(col1 int NOT NULL, col2 varchar(30), col3 int NOT NULL,primary key(col1,col3))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_pos_updex values(100,'venu',1)");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_pos_updex values(200,'MySQL',2)");
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc); 

    rc = tmysql_exec(hstmt,"select col1,col2 from t_pos_updex");
    mystmt(hstmt,rc); 
    
    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,2,&pcrow,NULL);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt1,1,SQL_PARAM_INPUT,SQL_C_CHAR,SQL_CHAR,0,0,data,20,NULL);
    mystmt(hstmt1,rc);

    rc = SQLGetCursorName(hstmt,cursor,30,NULL);
    mystmt(hstmt,rc); 

    sprintf(sql,"UPDATE t_pos_updex SET col1= 999, col2 = ? WHERE CURRENT OF %s",cursor);

    rc = SQLExecDirect(hstmt1,sql,SQL_NTS);
    mystmt(hstmt1,rc);  

    SQLNumResultCols(hstmt1,&rgfRowStatus);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    rc = SQLFreeStmt(hstmt1,SQL_CLOSE);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from t_pos_updex");
    mystmt(hstmt,rc);  

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);  

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);  
    {
      SQLCHAR szData[20];
      my_assert(999 == my_fetch_int(hstmt,1));
      my_assert(!strcmp("updated",my_fetch_str(hstmt,szData,2)));
    }

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt1,SQL_DROP);
    mystmt(hstmt1,rc);
}

static void tmysql_pos_update_ex3(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  SQLHSTMT hstmt1;
  SQLUINTEGER pcrow;
  SQLCHAR cursor[30],sql[100];

  printMessageHeader();

    rc = SQLAllocStmt(hdbc,&hstmt1);
    mycon(hdbc,rc);

    tmysql_exec(hstmt,"drop table t_pos_updex");
    rc = tmysql_exec(hstmt,"create table t_pos_updex(col1 int NOT NULL primary key, col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_pos_updex values(100,'venu')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_pos_updex values(200,'MySQL')");
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc); 

    rc = tmysql_exec(hstmt,"select * from t_pos_updex");
    mystmt(hstmt,rc); 
    
    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,2,&pcrow,NULL);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLGetCursorName(hstmt,cursor,30,NULL);
    mystmt(hstmt,rc); 

    sprintf(sql,"UPDATE t_pos_updex SET col1= 999, col2 = ? WHERE CURRENT OF %s",cursor);

    rc = SQLExecDirect(hstmt1,sql,SQL_NTS);
    mystmt_err(hstmt1,rc == SQL_ERROR,rc);  
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt1,SQL_DROP);
    mystmt(hstmt1,rc);
}


static void t_msdev_bug(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN  rc;
  SQLCHAR    catalog[30];
  SQLLEN     len;

  printMessageHeader();

   rc = SQLGetConnectOption(hdbc,SQL_CURRENT_QUALIFIER,&catalog);
   mycon(hdbc,rc);
   fprintf(stdout," SQL_CURRENT_QUALIFIER:%s\n",catalog);

   rc = SQLGetConnectAttr(hdbc,SQL_ATTR_CURRENT_CATALOG,&catalog,30,&len);
   mycon(hdbc,rc);
   fprintf(stdout," SQL_ATTR_CURRENT_CATRALOG:%s(%d)\n",catalog,len);
}

static void t_setpos_position(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  long nData= 500;
  SQLLEN nlen;
  SQLCHAR szData[255]={0};
  SQLUINTEGER pcrow;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_setpos_position");
    rc = tmysql_exec(hstmt,"create table t_setpos_position(col1 int, col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_setpos_position values(100,'MySQL1')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into t_setpos_position values(200,'MySQL2')");
    mystmt(hstmt,rc); 
    rc = tmysql_exec(hstmt,"insert into t_setpos_position values(300,'MySQL3')");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc); 

    rc = SQLSetCursorName(hstmt,"venu",SQL_NTS);
    mystmt(hstmt,rc);     
    
    SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER) SQL_CONCUR_ROWVER, 0);
    SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_DYNAMIC, 0);
    SQLSetStmtOption(hstmt,SQL_SIMULATE_CURSOR,SQL_SC_NON_UNIQUE);

    rc = tmysql_exec(hstmt,"select * from t_setpos_position");
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,NULL);
    mystmt(hstmt,rc);

    fprintf(stdout," pcrow:%d\n",pcrow);     
    fprintf(stdout," row1:%d,%s\n",nData,szData);    
    
    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    nData = 1000;
    strcpy((char *)szData , "updated");
    
    rc = SQLSetPos(hstmt,3,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt_err(hstmt,rc == SQL_ERROR,rc);

    rc = SQLSetPos(hstmt,2,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt_err(hstmt,rc == SQL_ERROR,rc);

    rc = SQLSetPos(hstmt,1,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);
    
    fprintf(stdout," rows affected:%d\n",nlen);    

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_setpos_position");
    mystmt(hstmt,rc);  

    myresult(hstmt);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
    
    rc = SQLExecDirect(hstmt,"DELETE FROM t_setpos_position WHERE col2 = 'updated'",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n total rows affceted:%d",nlen);
    my_assert(nlen == 1);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from t_setpos_position");
    mystmt(hstmt,rc);  

    my_assert(2 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}

static void t_error(SQLHDBC hdbc,SQLHSTMT hstmt)
{
  SQLRETURN rc;
  SQLCHAR   state[6], errmsg[300];
  SQLINTEGER native;
  SQLSMALLINT pclen;
  SQLHENV     henvl;

  printMessageHeader();

    if (!server_is_mysql(hdbc))
      return;

    rc = SQLExecDirect(hstmt,"drop table NON_EXISTANT_TABLE_t_error",SQL_NTS);
    myassert(rc == SQL_ERROR);

    rc = SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, (SQLCHAR *)&state, 
                       &native, (SQLCHAR *)&errmsg,255,&pclen);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n state  : %s\n native : %d\n error  : %s\n errlen : %d\n",
              state, native, errmsg, pclen);
    myassert(strcmp(state,"42S02") == 0);    
    myassert(native == 1051);

    rc = SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, (SQLCHAR *)&state, 
                       &native, (SQLCHAR *)&errmsg,255,&pclen);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n state  : %s\n native : %d\n error  : %s\n errlen : %d\n",
              state, native, errmsg, pclen);
    myassert(strcmp(state,"42S02") == 0);    
    myassert(native == 1051);

    rc = SQLError(NULL,NULL,hstmt,(SQLCHAR *)&state,&native,
                  (SQLCHAR *)&errmsg,255,&pclen);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n state  : %s\n native : %d\n error  : %s\n errlen : %d\n",
              state, native, errmsg, pclen);
    myassert(strcmp(state,"42S02") == 0);    
    myassert(native == 1051);

    rc = SQLError(NULL,NULL,hstmt,(SQLCHAR *)&state,&native,
                  (SQLCHAR *)&errmsg,255,&pclen);
    myassert(rc == SQL_NO_DATA_FOUND);

    SQLFreeStmt(hstmt, SQL_CLOSE);

    /* env and dbc related */
    rc = SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&henvl);
    myassert(rc == SQL_SUCCESS);

    rc = SQLSetEnvAttr(henvl, SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0);
    myassert(rc == SQL_SUCCESS);

    rc = SQLSetEnvAttr(henvl,SQL_ATTR_CP_MATCH, (SQLPOINTER)100,0);
    myassert(rc == SQL_ERROR);

    rc = SQLGetDiagRec(SQL_HANDLE_ENV, henvl, 1, (SQLCHAR *)&state, 
                       &native, (SQLCHAR *)&errmsg,255,&pclen);
    myassert(rc == SQL_SUCCESS);
    fprintf(stdout,"\n state  : %s\n native : %d\n error  : %s\n errlen : %d\n",
              state, native, errmsg, pclen);
    myassert(strcmp(state,"HY024") == 0 || strcmp(state,"HYC00")==0);    

    rc = SQLGetDiagRec(SQL_HANDLE_ENV, henvl, 1, (SQLCHAR *)&state, 
                       &native, (SQLCHAR *)&errmsg,255,&pclen);
    myassert(rc == SQL_SUCCESS);
    fprintf(stdout,"\n state  : %s\n native : %d\n error  : %s\n errlen : %d\n",
              state, native, errmsg, pclen);
    myassert(strcmp(state,"HY024") == 0 || strcmp(state,"HYC00")==0);    

    rc = SQLError(henvl,NULL,NULL,(SQLCHAR *)&state,&native,
                  (SQLCHAR *)&errmsg,255,&pclen);
    myassert(rc == SQL_SUCCESS);
    fprintf(stdout,"\n state  : %s\n native : %d\n error  : %s\n errlen : %d\n",
              state, native, errmsg, pclen);
    myassert(strcmp(state,"HY024") == 0 || strcmp(state,"HYC00")==0);    

    rc = SQLError(NULL,NULL,hstmt,(SQLCHAR *)&state,&native,
                  (SQLCHAR *)&errmsg,255,&pclen);
    myassert(rc == SQL_NO_DATA_FOUND);
    SQLFreeEnv(henvl);
}

static void t_pos_column_ignore(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  SQLHSTMT hstmt1;
  SQLCHAR szData[]="updated";
  long nData;
  SQLLEN  pcbValue, nlen, pcrow;

  printMessageHeader();

    rc = SQLAllocStmt(hdbc,&hstmt1);
    mycon(hdbc,rc);

    tmysql_exec(hstmt,"drop table t_pos_column_ignore");
    rc = tmysql_exec(hstmt,"create table t_pos_column_ignore(col1 int NOT NULL primary key, col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_pos_column_ignore values(10,'venu')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_pos_column_ignore values(100,'MySQL')");
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc); 
    
    SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER) SQL_CONCUR_ROWVER, 0);
    SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_KEYSET_DRIVEN, 0);

    /* ignore all columns */
    rc = tmysql_exec(hstmt,"select * from t_pos_column_ignore order by col1 asc");
    mystmt(hstmt,rc);        

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,&pcbValue);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,100,&pcbValue);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,NULL);
    mystmt(hstmt,rc);
    
    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    nData = 99;
    strcpy((char *)szData , "updated");
    
    pcbValue = SQL_COLUMN_IGNORE;
    rc = SQLSetPos(hstmt,1,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);
    
    fprintf(stdout," rows affected:%d\n",nlen);    
    myassert(nlen == 0);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt1,SQL_CLOSE);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from t_pos_column_ignore order by col1 asc");
    mystmt(hstmt,rc);  

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);  
    {
      SQLCHAR szData[20];
      my_assert(10 == my_fetch_int(hstmt,1));
      my_assert(!strcmp("venu",my_fetch_str(hstmt,szData,2)));
    }

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    /* ignore only one column */   

    rc = tmysql_exec(hstmt,"select * from t_pos_column_ignore order by col1 asc");
    mystmt(hstmt,rc);        

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,100,&pcbValue);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,NULL);
    mystmt(hstmt,rc);
    
    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    nData = 99;
    strcpy((char *)szData , "updated");
    
    pcbValue = SQL_COLUMN_IGNORE;
    rc = SQLSetPos(hstmt,1,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);
    
    fprintf(stdout," rows affected:%d\n",nlen);    

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt1,SQL_CLOSE);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from t_pos_column_ignore order by col1 asc");
    mystmt(hstmt,rc);  

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);  
    {
      SQLCHAR szData[20];
      my_assert(99 == my_fetch_int(hstmt,1));
      my_assert(!strcmp("venu",my_fetch_str(hstmt,szData,2)));
    }

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt1,SQL_DROP);
    mystmt(hstmt1,rc);
}


static void t_longlong1(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN   rc;
  SQLINTEGER  session_id, ctn;
  
  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_longlong");
    rc = tmysql_exec(hstmt,"create table t_longlong (\
                          session_id  bigint not null,\
                          ctn         bigint not null)");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);
    
    SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER) SQL_CONCUR_ROWVER, 0);
    SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_STATIC, 0);

    rc = SQLPrepare(hstmt,"insert into t_longlong values (?,?)",SQL_NTS);
    mystmt(hstmt,rc);   
        
    rc = SQLBindParameter( hstmt, 1, SQL_PARAM_INPUT, SQL_C_UBIGINT, 
                           SQL_BIGINT, 20, 0, &session_id, 20, NULL );

    rc = SQLBindParameter( hstmt, 2, SQL_PARAM_INPUT, SQL_C_UBIGINT, 
                           SQL_BIGINT, 20, 0, &ctn, 20, NULL );

    for (session_id=50; session_id < 100; session_id++)
    {
      ctn += session_id;
      rc = SQLExecute(hstmt);
      mystmt(hstmt,rc);
    }

    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);  

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from t_longlong");
    mystmt(hstmt,rc);  

    my_assert( 50 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}


static void t_time(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN       rc;
  SQL_TIME_STRUCT tm;
  SQLCHAR         str[20];

  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_time");
    rc = tmysql_exec(hstmt,"create table t_time(tm time, ts timestamp(14))");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLPrepare(hstmt,"insert into t_time values (?,?)",SQL_NTS);
    mystmt(hstmt,rc);   
        
    rc = SQLBindParameter( hstmt, 1, SQL_PARAM_INPUT, SQL_C_TIME, 
                           SQL_TIME, 0, 0, &tm, 0, NULL );
    mystmt(hstmt,rc);   
        
    rc = SQLBindParameter( hstmt, 2, SQL_PARAM_INPUT, SQL_C_TIME, 
                           SQL_TIMESTAMP, 0, 0, &tm, 15, NULL );
    mystmt(hstmt,rc);   

    tm.hour = 20;
    tm.minute = 59;
    tm.second = 45;

    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);   

    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);  

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLExecDirect(hstmt,"select tm from t_time",SQL_NTS);
    mystmt(hstmt,rc);  

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);   

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,&str,100,NULL);
    mystmt(hstmt,rc);   
    fprintf(stdout,"\n time:%s",str);
    my_assert(strcmp(str,"20:59:45")==0);

    rc = SQLFetch(hstmt);
    my_assert(rc == SQL_NO_DATA_FOUND);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}

static void t_numeric(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN       rc;
  SQL_NUMERIC_STRUCT num;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_decimal");
    rc = tmysql_exec(hstmt,"create table t_decimal(d1 decimal(10,6))");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into t_decimal values(10.2)");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLPrepare(hstmt,"insert into t_decimal values (?)",SQL_NTS);
    mystmt(hstmt,rc);   
        
    rc = SQLBindParameter( hstmt, 1, SQL_PARAM_INPUT, SQL_C_NUMERIC, 
                           SQL_DECIMAL, 10, 4, &num, 0, NULL );
    mystmt_r(hstmt,rc);

    SQLFreeStmt(hstmt, SQL_CLOSE);
    
    rc = SQLPrepare(hstmt,"insert into t_decimal values (?),(?)",SQL_NTS);
    mystmt(hstmt,rc);   
        
    rc = SQLBindParameter( hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, 
                           SQL_DECIMAL, 10, 4, &rc, 0, NULL );
    mystmt(hstmt,rc);   
    
    rc = SQLBindParameter( hstmt, 2, SQL_PARAM_INPUT, SQL_C_NUMERIC, 
                           SQL_DECIMAL, 10, 4, &num, 0, NULL );
    mystmt_r(hstmt,rc);

    SQLFreeStmt(hstmt, SQL_CLOSE);
    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);

    rc = SQLBindCol( hstmt, 1, SQL_C_NUMERIC, &num, 0, NULL );
    mystmt_r(hstmt,rc);

    SQLFreeStmt(hstmt, SQL_CLOSE);
    SQLFreeStmt(hstmt, SQL_UNBIND);
    
    rc = SQLExecDirect(hstmt, "select * from t_decimal",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    
    rc = SQLGetData( hstmt, 1, SQL_C_NUMERIC, &num, 0, NULL );
    mystmt_r(hstmt,rc);   
    
    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_CLOSE);
}

static void t_decimal(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLCHAR         str[20],s_data[]="189.4567";
  SQLDOUBLE       d_data=189.4567;
  long            i_data=189, l_data=-23;
  SQLRETURN       rc;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_decimal");
    rc = tmysql_exec(hstmt,"create table t_decimal(d1 decimal(10,6))");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLPrepare(hstmt,"insert into t_decimal values (?),(?),(?),(?)",SQL_NTS);
    mystmt(hstmt,rc);   
        
    rc = SQLBindParameter( hstmt, 1, SQL_PARAM_INPUT, SQL_C_DOUBLE, 
                           SQL_DECIMAL, 10, 4, &d_data, 0, NULL );
    mystmt(hstmt,rc);   
        
    rc = SQLBindParameter( hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, 
                           SQL_DECIMAL, 10, 4, &i_data, 0, NULL );
    mystmt(hstmt,rc);   
        
    rc = SQLBindParameter( hstmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, 
                           SQL_DECIMAL, 10, 4, &s_data, 9, NULL );
    mystmt(hstmt,rc);   
    
    rc = SQLBindParameter( hstmt, 4, SQL_PARAM_INPUT, SQL_C_LONG, 
                           SQL_DECIMAL, 10, 4, &l_data, 0, NULL );
    mystmt(hstmt,rc);   
    
    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);   

    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);  

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLExecDirect(hstmt,"select d1 from t_decimal",SQL_NTS);
    mystmt(hstmt,rc);  

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);   

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,&str,19,NULL);
    mystmt(hstmt,rc);   
    fprintf(stdout,"\n decimal(SQL_C_DOUBLE) : %s",str);
    my_assert(strncmp(str,"189.4567",8)==0);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);   

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,&str,19,NULL);
    mystmt(hstmt,rc);   
    fprintf(stdout,"\n decimal(SQL_C_INTEGER): %s",str);
    my_assert(strncmp(str,"189.0000",5)==0);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);   

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,&str,19,NULL);
    mystmt(hstmt,rc);   
    fprintf(stdout,"\n decimal(SQL_C_CHAR)   : %s",str);
    my_assert(strncmp(str,"189.4567",8)==0);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);   

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,&str,19,NULL);
    mystmt(hstmt,rc);   
    fprintf(stdout,"\n decimal(SQL_C_LONG)   : %s",str);
    my_assert(strncmp(str,"-23.00",6)==0);

    rc = SQLFetch(hstmt);
    my_assert(rc == SQL_NO_DATA_FOUND);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}


static void t_warning(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  SQLCHAR szData[20];
  SQLINTEGER pcbValue;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_warning");
    rc = tmysql_exec(hstmt,"create table t_warning(col2 char(20))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_warning values('venu anuganti')");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc); 
    
    SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER) SQL_CONCUR_ROWVER, 0);
    SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_KEYSET_DRIVEN, 0);

    /* ignore all columns */
    rc = tmysql_exec(hstmt,"select * from t_warning");
    mystmt(hstmt,rc);  
    
    rc = SQLFetch(hstmt);
    mystmt(hstmt, rc);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,4,&pcbValue);
    mystmt_err(hstmt, rc == SQL_SUCCESS_WITH_INFO, rc);
    fprintf(stdout,"\n data: %s(%d)",szData,pcbValue);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,4,&pcbValue);
    mystmt_err(hstmt, rc == SQL_SUCCESS_WITH_INFO, rc);
    fprintf(stdout,"\n data: %s(%d)",szData,pcbValue);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,4,&pcbValue);
    mystmt_err(hstmt, rc == SQL_SUCCESS_WITH_INFO, rc);
    fprintf(stdout,"\n data: %s(%d)",szData,pcbValue);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,4,&pcbValue);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n data: %s(%d)",szData,pcbValue);

    rc = SQLFetch(hstmt);
    mystmt_err(hstmt, rc == SQL_NO_DATA_FOUND, rc);

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);
}

static void t_multistep(SQLHDBC hdbc, SQLHSTMT hstmt)
{
#ifdef DBUG_OFF
  SQLRETURN  rc;
  SQLCHAR    szData[150];
  SQLINTEGER pcbValue,id;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_multistep");
    rc = tmysql_exec(hstmt,"create table t_multistep(col1 int,col2 varchar(200))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_multistep values(10,'MySQL - Open Source Database')");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc); 
    
    SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER) SQL_CONCUR_ROWVER, 0);
    SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_KEYSET_DRIVEN, 0);

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&id,0,NULL);
    mystmt(hstmt,rc);
    
    rc = tmysql_exec(hstmt,"select * from t_multistep");
    mystmt(hstmt,rc);
    
    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n id: %ld",id);
    myassert(id == 10);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,0,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    fprintf(stdout,"\n length: %ld", pcbValue);
    myassert(pcbValue == 28);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,0,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    fprintf(stdout,"\n length: %ld", pcbValue);
    myassert(pcbValue == 28);

    rc = SQLGetData(hstmt,2,SQL_C_BINARY,szData,0,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    fprintf(stdout,"\n length: %ld", pcbValue);
    myassert(pcbValue == 28);

    rc = SQLGetData(hstmt,2,SQL_C_BINARY,szData,0,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    fprintf(stdout,"\n length: %ld", pcbValue);
    myassert(pcbValue == 28);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,0,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    fprintf(stdout,"\n length: %ld", pcbValue);
    myassert(pcbValue == 28);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,10,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    fprintf(stdout,"\n data  : %s (%ld)",szData,pcbValue);
    myassert(pcbValue == 28);
    myassert(strcmp(szData,"MySQL - O") == 0);

    pcbValue= 0;
    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,5,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    fprintf(stdout,"\n data  : %s (%ld)",szData,pcbValue);
    myassert(pcbValue == 19);
    myassert(strcmp(szData,"pen ") == 0);

    pcbValue= 0;
    szData[0]='A';
    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,0,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    fprintf(stdout,"\n data  : %s (%ld)",szData,pcbValue);
    myassert(pcbValue == 15);
    myassert(szData[0] == 'A');

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,pcbValue+1,&pcbValue);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n data  : %s (%ld)",szData,pcbValue);    
    myassert(pcbValue == 15);
    myassert(strcmp(szData,"Source Database") == 0);

    pcbValue= 99;
    szData[0]='A';
    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,0,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    fprintf(stdout,"\n data  : %s (%ld)",szData,pcbValue);
    myassert(pcbValue == 0);
    myassert(szData[0] == 'A');


    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA_FOUND);

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);
#endif
}

static void t_zerolength(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN  rc;
  SQLCHAR    szData[100], bData[100], bData1[100];
  SQLLEN     pcbValue,pcbValue1,pcbValue2;

  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_zerolength");
    rc = tmysql_exec(hstmt,"create table t_zerolength(str varchar(20), bin varbinary(20), blb blob)");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_zerolength values('','','')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_zerolength values('venu','mysql','monty')");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc); 
    
    SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER) SQL_CONCUR_ROWVER, 0);
    SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_KEYSET_DRIVEN, 0);

    rc = tmysql_exec(hstmt,"select * from t_zerolength");
    mystmt(hstmt,rc);
    
    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    pcbValue= pcbValue1= 99;
    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,0,&pcbValue);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"\n length: %d", pcbValue);
    myassert(pcbValue == 0);

    bData[0]=bData[1]='z';
    rc = SQLGetData(hstmt,2,SQL_C_BINARY,bData,0,&pcbValue1);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"\n length: %d", pcbValue1);
    myassert(pcbValue1 == 0);    
    myassert(bData[0] == 'z');
    myassert(bData[1] == 'z');

    bData1[0]=bData1[1]='z';
    rc = SQLGetData(hstmt,3,SQL_C_BINARY,bData1,0,&pcbValue2);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"\n length: %d", pcbValue2);
    myassert(pcbValue2 == 0);    
    myassert(bData1[0] == 'z');
    myassert(bData1[1] == 'z');

    pcbValue= pcbValue1= 99;
    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,1,&pcbValue);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n data: %s, length: %d", szData, pcbValue);
    myassert(pcbValue == 0);
    myassert(szData[0] == '\0'); 

    bData[0]=bData[1]='z';
    rc = SQLGetData(hstmt,2,SQL_C_BINARY,bData,1,&pcbValue1);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n data: %s, length: %d", bData, pcbValue1);
    myassert(pcbValue1 == 0);
    myassert(bData[0]== '\0');

    bData1[0]=bData1[1]='z';
    rc = SQLGetData(hstmt,3,SQL_C_CHAR,bData1,1,&pcbValue2);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n data: %s, length: %d", bData1, pcbValue2);
    myassert(pcbValue2 == 0);
    myassert(bData1[0] == '\0');
    myassert(bData1[1] == 'z');

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    pcbValue= pcbValue1= 99;
    szData[0]= bData[0]= 'z';
    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,0,&pcbValue);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"\n length: %d", pcbValue);
    myassert(pcbValue == 4);
    myassert(szData[0] == 'z');
    
    rc = SQLGetData(hstmt,2,SQL_C_BINARY,bData,0,&pcbValue1);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"\n length: %d", pcbValue1);
    myassert(pcbValue1 == 5);  
    myassert(bData[0] == 'z');  
    
    bData[0]=bData1[1]='z';
    rc = SQLGetData(hstmt,3,SQL_C_BINARY,bData1,0,&pcbValue2);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"\n length: %d", pcbValue2);
    myassert(pcbValue2 == 5);  

    pcbValue= pcbValue1= 99;
    szData[0]= szData[1]= bData[0]= bData[1]= 'z';
    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,1,&pcbValue);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"\n data: %s, length: %d", szData,pcbValue);
    myassert(pcbValue == 4);
    myassert(szData[0] == '\0');

    rc = SQLGetData(hstmt,2,SQL_C_BINARY,bData,1,&pcbValue1);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"\n data; %s, length: %d", bData, pcbValue1);
    myassert(pcbValue1 == 5);    
    myassert(bData[0] == 'm');
    
    bData[0]=bData1[1]='z';
    rc = SQLGetData(hstmt,3,SQL_C_BINARY,bData1,1,&pcbValue2);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"\n length: %d", pcbValue2);
    myassert(pcbValue2 == 5);  
    myassert(bData1[0] == 'm'); 
    myassert(bData1[1] == 'z'); 

    pcbValue= pcbValue1= 99;
    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,4,&pcbValue);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"\n data: %s, length: %d", szData, pcbValue);
    myassert(pcbValue == 4);
    myassert(strcmp(szData,"ven")==0);

    rc = SQLGetData(hstmt,2,SQL_C_BINARY,bData,4,&pcbValue1);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"\n data: %s, length: %d", bData, pcbValue1);
    myassert(pcbValue1 == 5);
    myassert(strncmp(bData,"mysq",4)==0);

    pcbValue= pcbValue1= 99;
    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,5,&pcbValue);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n data: %s, length: %d", szData, pcbValue);
    myassert(pcbValue == 4);
    myassert(strcmp(szData,"venu")==0);

    rc = SQLGetData(hstmt,2,SQL_C_BINARY,bData,5,&pcbValue1);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n data: %s, length: %d", bData, pcbValue1);
    myassert(pcbValue1 == 5);
    myassert(strncmp(bData,"mysql",5)==0);

    szData[0]= 'z';
    rc = SQLGetData(hstmt,3,SQL_C_CHAR,szData,0,&pcbValue);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"\n data: %s, length: %d", szData, pcbValue);
    myassert(pcbValue == 5 || pcbValue == 10);
    myassert(szData[0] == 'z');

#if TO_BE_FIXED_IN_DRIVER    
    szData[0]=szData[1]='z';
    rc = SQLGetData(hstmt,3,SQL_C_CHAR,szData,1,&pcbValue);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"\n data: %s, length: %d", szData, pcbValue);
    myassert(pcbValue == 10);
    myassert(szData[0] == 'm');
    myassert(szData[1] == 'z');
    
    rc = SQLGetData(hstmt,3,SQL_C_CHAR,szData,4,&pcbValue);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"\n data: %s, length: %d", szData, pcbValue);
    myassert(pcbValue == 10);
    myassert(strncmp(szData,"mont",4) == 0);
    
    rc = SQLGetData(hstmt,3,SQL_C_CHAR,szData,5,&pcbValue);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n data: %s, length: %d", szData, pcbValue);
    myassert(pcbValue == 10);
    myassert(strncmp(szData,"monty",5) == 0);
#endif

    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA_FOUND);

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);
}


static void t_pos_datetime_delete(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  SQLHSTMT  hstmt1;
  long      int_data;
  SQLLEN    row_count, cur_type;

  printMessageHeader();

    rc = SQLAllocStmt(hdbc,&hstmt1);
    mycon(hdbc,rc);    
    
    tmysql_exec(hstmt,"drop table t_pos_delete");
    rc = tmysql_exec(hstmt,"create table t_pos_delete(id int not null default '0',\
                                                      name varchar(20) NOT NULL default '',\
                                                      created datetime NOT NULL default '2000-01-01')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_pos_delete values(1,'venu','2003-02-10 14:45:39')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_pos_delete(name) values('')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_pos_delete(id) values(2)");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_pos_delete");
    mystmt(hstmt,rc);  

    my_assert(3 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER) SQL_CONCUR_ROWVER, 0);
    SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_DYNAMIC, 0);
    SQLSetStmtOption(hstmt,SQL_SIMULATE_CURSOR,SQL_SC_TRY_UNIQUE);

    SQLSetStmtAttr(hstmt1, SQL_ATTR_CONCURRENCY, (SQLPOINTER) SQL_CONCUR_ROWVER, 0);
    SQLSetStmtAttr(hstmt1, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_DYNAMIC, 0);
    SQLSetStmtOption(hstmt1,SQL_SIMULATE_CURSOR,SQL_SC_TRY_UNIQUE);

    rc = SQLSetCursorName(hstmt,"venu_cur",8);
    mystmt(hstmt,rc);   

    rc = SQLGetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, &cur_type, 0, NULL);
    mystmt(hstmt,rc);   

    rc = tmysql_exec(hstmt,"select * from t_pos_delete");
    mystmt(hstmt,rc); 

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&int_data,0,NULL);
    mystmt(hstmt,rc);   
    
    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,NULL,NULL);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n current_row: %d", int_data);
    myassert(int_data == 1);

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt1,"DELETE FROM t_pos_delete WHERE CURRENT OF venu_cur",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLRowCount(hstmt1,&row_count);
    mystmt(hstmt1,rc);
    fprintf(stdout, "\n rows affected: %d", row_count);
    myassert(row_count == 1); 
    
    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,NULL,NULL);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n current_row: %d", int_data);
    if (cur_type == SQL_CURSOR_DYNAMIC)
      myassert(int_data == 2);
    else
      myassert(int_data == 0);
    
    /*rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,NULL,NULL);
    mystmt(hstmt,rc);*/

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt1,"DELETE FROM t_pos_delete WHERE CURRENT OF venu_cur",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLRowCount(hstmt1,&row_count);
    mystmt(hstmt1,rc);
    fprintf(stdout, "\n rows affected: %d", row_count);
    myassert(row_count == 1); 

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);
    SQLFreeStmt(hstmt1,SQL_CLOSE);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from t_pos_delete");
    mystmt(hstmt,rc);  

    my_assert(1 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    if (cur_type == SQL_CURSOR_DYNAMIC)
      verify_col_data(hstmt,"t_pos_delete","id","0");
    else
      verify_col_data(hstmt,"t_pos_delete","id","2");

    rc = SQLFreeStmt(hstmt1,SQL_DROP);
    mystmt(hstmt1,rc);
}


static void t_pos_datetime_delete1(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  SQLHSTMT hstmt1;
  long int_data;
  SQLLEN row_count, cur_type;

  printMessageHeader();

    rc = SQLAllocStmt(hdbc,&hstmt1);
    mycon(hdbc,rc);    
    
    tmysql_exec(hstmt,"drop table t_pos_delete");
    rc = tmysql_exec(hstmt,"create table t_pos_delete(id int not null default '0',\
                                                      name varchar(20) NOT NULL default '',\
                                                      created datetime NOT NULL default '2000-01-01')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_pos_delete values(1,'venu','2003-02-10 14:45:39')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_pos_delete(name) values('')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_pos_delete(id) values(2)");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_pos_delete(id) values(3)");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_pos_delete(id) values(4)");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_pos_delete(id) values(5)");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_pos_delete");
    mystmt(hstmt,rc);  

    my_assert(6 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER) SQL_CONCUR_ROWVER, 0);
    SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_DYNAMIC, 0);
    SQLSetStmtOption(hstmt,SQL_SIMULATE_CURSOR,SQL_SC_NON_UNIQUE);

    SQLSetStmtAttr(hstmt1, SQL_ATTR_CONCURRENCY, (SQLPOINTER) SQL_CONCUR_ROWVER, 0);
    SQLSetStmtAttr(hstmt1, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_DYNAMIC, 0);
    SQLSetStmtOption(hstmt1,SQL_SIMULATE_CURSOR,SQL_SC_NON_UNIQUE);

    rc = SQLSetCursorName(hstmt,"venu_cur",8);
    mystmt(hstmt,rc);    

    rc = SQLGetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, &cur_type, 0, NULL);
    mystmt(hstmt,rc);   

    rc = tmysql_exec(hstmt,"select * from t_pos_delete");
    mystmt(hstmt,rc); 

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&int_data,0,NULL);
    mystmt(hstmt,rc);   
    
    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,3,NULL,NULL);
    mystmt(hstmt,rc);  
    fprintf(stdout,"\n current_row: %d", int_data);
    myassert(int_data == 2);

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt1,"DELETE FROM t_pos_delete WHERE CURRENT OF venu_cur",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLRowCount(hstmt1,&row_count);
    mystmt(hstmt1,rc);
    fprintf(stdout, "\n rows affected: %d", row_count);
    myassert(row_count == 1); 
    
    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,NULL,NULL);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n current_row: %d", int_data);
    
    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,NULL,NULL);
    mystmt(hstmt,rc);  
    fprintf(stdout,"\n current_row: %d", int_data);
    
    /*rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,NULL,NULL);
    mystmt(hstmt,rc);*/

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt1,"DELETE FROM t_pos_delete WHERE CURRENT OF venu_cur",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLRowCount(hstmt1,&row_count);
    mystmt(hstmt1,rc);
    fprintf(stdout, "\n rows affected: %d", row_count);
    myassert(row_count == 1); 

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);
    SQLFreeStmt(hstmt1,SQL_CLOSE);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from t_pos_delete");
    mystmt(hstmt,rc);  

    my_assert(4 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt1,SQL_DROP);
    mystmt(hstmt1,rc);
}

#define TEST_ODBC_TEXT_LEN 3000
static void t_text_fetch(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN  rc;
  long       i;
  SQLLEN     row_count, length;
  SQLCHAR    data[TEST_ODBC_TEXT_LEN+1];

  printMessageHeader();

    SQLExecDirect(hstmt,"drop table t_text_fetch",SQL_NTS);
    rc = SQLExecDirect(hstmt,"create table t_text_fetch(t1 tinytext, \
                                                      t2 text, \
                                                      t3 mediumtext, \
                                                      t4 longtext)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"insert into t_text_fetch values(?,?,?,?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR,
                          0,0,(char *)data, TEST_ODBC_TEXT_LEN/3, NULL);
    mystmt(hstmt,rc);
    
    rc = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR,
                          0,0,(char *)data, TEST_ODBC_TEXT_LEN/2, NULL);
    mystmt(hstmt,rc);
    
    rc = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR,
                          0,0,(char *)data, 
                          (SQLINTEGER)(TEST_ODBC_TEXT_LEN/1.5), NULL);
    mystmt(hstmt,rc);
    
    rc = SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR,
                          0,0,(char *)data, TEST_ODBC_TEXT_LEN-1, NULL);
    mystmt(hstmt,rc);

    memset(data,'A',TEST_ODBC_TEXT_LEN);
    data[TEST_ODBC_TEXT_LEN]='\0';

    for (i=0; i < 10; i++)
    {
      rc = SQLExecute(hstmt);
      mystmt(hstmt,rc);
    }

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"SELECT * FROM t_text_fetch",SQL_NTS);
    mystmt(hstmt,rc);

    row_count= 0;
    rc = SQLFetch(hstmt);
    while (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    {
       fprintf(stdout,"\n row '%d' (lengths: ", row_count);
       rc = SQLGetData(hstmt,1,SQL_C_CHAR,(char *)data,TEST_ODBC_TEXT_LEN,&length);
       mystmt(hstmt,rc);
       fprintf(stdout,"%d", length);
       myassert(length == 255);
       
       rc = SQLGetData(hstmt,2,SQL_C_CHAR,(char *)data,TEST_ODBC_TEXT_LEN,&length);
       mystmt(hstmt,rc);
       fprintf(stdout,",%d", length);
       myassert(length == TEST_ODBC_TEXT_LEN/2);
       
       rc = SQLGetData(hstmt,3,SQL_C_CHAR,(char *)data,TEST_ODBC_TEXT_LEN,&length);
       mystmt(hstmt,rc);
       fprintf(stdout,",%d", length);
       myassert(length == (SQLINTEGER)(TEST_ODBC_TEXT_LEN/1.5));
       
       rc = SQLGetData(hstmt,4,SQL_C_CHAR,(char *)data,TEST_ODBC_TEXT_LEN,&length);
       mystmt(hstmt,rc);
       fprintf(stdout,",%d)", length);
       myassert(length == TEST_ODBC_TEXT_LEN-1);
       row_count++;

       rc = SQLFetch(hstmt);
    }
    fprintf(stdout,"\n total rows: %d", row_count);
    myassert(row_count == i);
    
    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);
    
    rc = SQLExecDirect(hstmt,"DROP TABLE t_text_fetch",SQL_NTS);
    mystmt(hstmt,rc);
}

/* To test SQLColumns misc case */
static void t_columns(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLUSMALLINT  NumPrecRadix, DataType, Nullable;
  SQLUINTEGER   cbColumnSize, cbDecimalDigits, cbNumPrecRadix, 
                cbDatabaseName, cbDataType, cbNullable;
  SQLRETURN     rc;
  long          ColumnSize, i;
  SQLUINTEGER   ColumnCount= 7;
  SQLUSMALLINT  DecimalDigits;
  SQLCHAR       ColumnName[MAX_NAME_LEN], DatabaseName[MAX_NAME_LEN];
  SQLUINTEGER   Values[7][5][2]=
  {
    { {5,2},  {6,4}, {0,2},  {10,2},  {1,2}},
    { {12,2},  {5,4},  {0,-1}, {10,-1}, {1,2}},
    { {12,2}, {20,4}, {0,-1}, {10,-1}, {0,2}},
    { {3,2},  {10,4}, {2,2},  {10,2},  {1,2}},
    { {65530,2},  {4,4}, {0,2},  {10,2},  {0,2}},
    { {4,2}, {11,4}, {0,2},  {10,2},  {0,2}},
    { {65530,2}, {4,4}, {0,2},  {10,2},  {1,2}}
  };
  
  printMessageHeader();
  
    SQLFreeStmt(hstmt, SQL_CLOSE);
    SQLExecDirect(hstmt,"DROP TABLE test_column",SQL_NTS);
    
    rc = SQLExecDirect(hstmt,"CREATE TABLE test_column(col0 smallint, \
                                                       col1 char(5),\
                                                       col2 varchar(20) not null,\
                                                       col3 decimal(10,2),\
                                                       col4 tinyint not null,\
                                                       col5 integer primary key,\
                                                       col6 tinyint not null unique auto_increment)",SQL_NTS);
    mystmt(hstmt,rc);

    mystmt(hstmt,rc);

    rc= SQLSetStmtAttr(hstmt, SQL_ATTR_METADATA_ID,
                      (SQLPOINTER)SQL_FALSE, SQL_IS_UINTEGER);
    mystmt(hstmt,rc);

    SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_STATIC, 0);

    rc= SQLGetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG,(SQLCHAR *)DatabaseName,
                          MAX_NAME_LEN, &cbDatabaseName);/* Current Catalog */
    mycon(hdbc,rc);
    
    for (i=0; i< ColumnCount; i++)
    {
      sprintf(ColumnName,"col%d",i);
      
      rc= SQLColumns(hstmt, 
                     (SQLCHAR *)DatabaseName, (SQLUSMALLINT)cbDatabaseName,
                     SQL_NULL_HANDLE, 0,
                     (SQLCHAR *)"test_column", SQL_NTS,
                     (SQLCHAR *)ColumnName, SQL_NTS);
      mystmt(hstmt,rc);
    
      /* 5 -- Data type */
      rc=  SQLBindCol(hstmt, 5, SQL_C_SSHORT, &DataType, 0, &cbDataType);
      mystmt(hstmt,rc);   
    
      /* 7 -- Column Size */
      rc=  SQLBindCol(hstmt, 7, SQL_C_ULONG, &ColumnSize, 0, &cbColumnSize);
      mystmt(hstmt,rc);    
      
      /* 9 -- Decimal Digits */
      rc= SQLBindCol(hstmt, 9, SQL_C_SSHORT, &DecimalDigits, 0, &cbDecimalDigits);
      mystmt(hstmt,rc);    
      
      /* 10 -- Num Prec Radix */
      rc= SQLBindCol(hstmt, 10, SQL_C_SSHORT, &NumPrecRadix, 0, &cbNumPrecRadix);
      mystmt(hstmt,rc);    
      
      /* 11 -- Nullable */
      rc= SQLBindCol(hstmt, 11, SQL_C_SSHORT, &Nullable, 0, &cbNullable);
      mystmt(hstmt,rc);

      rc= SQLFetch(hstmt);
      mystmt(hstmt,rc);
    
      fprintf(stdout,"\n Column %s:", ColumnName);
      fprintf(stdout,"\n\t DataType     = %d(%d)", DataType, cbDataType);
      fprintf(stdout,"\n\t ColumnSize   = %d(%d)", ColumnSize, cbColumnSize);
      fprintf(stdout,"\n\t DecimalDigits= %d(%d)", DecimalDigits, cbDecimalDigits);
      fprintf(stdout,"\n\t NumPrecRadix = %d(%d)", NumPrecRadix, cbNumPrecRadix);
      fprintf(stdout,"\n\t Nullable     = %s(%d)\n", 
                      Nullable == SQL_NO_NULLS ? "NO": "YES", cbNullable); 
    
      myassert(DataType == Values[i][0][0]);
      myassert(cbDataType == Values[i][0][1]);

      myassert(ColumnSize == Values[i][1][0]);
      myassert(cbColumnSize == Values[i][1][1]);

      myassert(DecimalDigits == Values[i][2][0]);
      myassert(cbDecimalDigits == Values[i][2][1]);
      
      myassert(NumPrecRadix == Values[i][3][0]);
      myassert(cbNumPrecRadix == Values[i][3][1]);
      
      myassert(Nullable == Values[i][4][0]);
      myassert(cbNullable == Values[i][4][1]);

      rc= SQLFetch(hstmt);
      myassert(rc == SQL_NO_DATA);

      SQLFreeStmt(hstmt,SQL_UNBIND);
      SQLFreeStmt(hstmt,SQL_CLOSE);
    }

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);
    
    rc = SQLExecDirect(hstmt,"DROP TABLE test_column",SQL_NTS);
    mystmt(hstmt,rc);
    SQLFreeStmt(hstmt,SQL_CLOSE);
}


/* To test a convertion type */
static void t_convert_type(SQLHENV henv, SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN   rc;
  SQLSMALLINT SqlType, DateType;
  SQLCHAR     ColName[MAX_NAME_LEN];
  SQLCHAR     DbVersion[MAX_NAME_LEN];
  SQLINTEGER  OdbcVersion;
  
  printMessageHeader();
  
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLGetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,&OdbcVersion,0,NULL);
    myenv(henv,rc);

    fprintf(stdout,"\n odbc version:");
    if (OdbcVersion == SQL_OV_ODBC2)
    {
      fprintf(stdout," SQL_OV_ODBC2");
      DateType= SQL_DATE;
    }
    else
    {
      fprintf(stdout," SQL_OV_ODBC3");
      DateType= SQL_TYPE_DATE;
    }

    rc = SQLGetInfo(hdbc,SQL_DBMS_VER,(SQLCHAR *)&DbVersion,MAX_NAME_LEN,NULL);
    mycon(hdbc,rc);
   
    SQLExecDirect(hstmt,"DROP TABLE t_convert",SQL_NTS);
    
    rc = SQLExecDirect(hstmt,"CREATE TABLE t_convert(col0 integer, \
                                                     col1 date,\
                                                     col2 char(10))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"INSERT INTO t_convert VALUES(10,'2002-10-24','venu')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"INSERT INTO t_convert VALUES(20,'2002-10-23','venu1')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"INSERT INTO t_convert VALUES(30,'2002-10-25','venu2')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"INSERT INTO t_convert VALUES(40,'2002-10-24','venu3')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"SELECT MAX(col0) FROM t_convert",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLDescribeCol(hstmt,1,(SQLCHAR *)&ColName,MAX_NAME_LEN,NULL,&SqlType,NULL,NULL,NULL);
    mystmt(hstmt,rc);

    fprintf(stdout,"\n MAX(col0): %d", SqlType);
    myassert(SqlType == SQL_INTEGER);

    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"SELECT MAX(col1) FROM t_convert",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLDescribeCol(hstmt,1,(SQLCHAR *)&ColName,MAX_NAME_LEN,NULL,&SqlType,NULL,NULL,NULL);
    mystmt(hstmt,rc);

    fprintf(stdout,"\n MAX(col1): %d", SqlType);
    myassert(SqlType == DateType);

    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"SELECT MAX(col2) FROM t_convert",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLDescribeCol(hstmt,1,(SQLCHAR *)&ColName,MAX_NAME_LEN,NULL,&SqlType,NULL,NULL,NULL);
    mystmt(hstmt,rc);

    fprintf(stdout,"\n MAX(col0): %d", SqlType);

    SQLFreeStmt(hstmt,SQL_CLOSE);

    if (strncmp(DbVersion,"4.",2) >= 0)
    {
      rc = SQLExecDirect(hstmt,"SELECT CAST(MAX(col1) AS DATE) AS col1 FROM t_convert",SQL_NTS);
      mystmt(hstmt,rc);

      rc = SQLDescribeCol(hstmt,1,(SQLCHAR *)&ColName,MAX_NAME_LEN,NULL,&SqlType,NULL,NULL,NULL);
      mystmt(hstmt,rc);

      fprintf(stdout,"\n CAST(MAX(col1) AS DATE): %d", SqlType);
      myassert(SqlType == DateType);

      SQLFreeStmt(hstmt,SQL_CLOSE);

      rc = SQLExecDirect(hstmt,"SELECT CONVERT(MAX(col1),DATE) AS col1 FROM t_convert",SQL_NTS);
      mystmt(hstmt,rc);

      rc = SQLDescribeCol(hstmt,1,(SQLCHAR *)&ColName,MAX_NAME_LEN,NULL,&SqlType,NULL,NULL,NULL);
      mystmt(hstmt,rc);

      fprintf(stdout,"\n CONVERT(MAX(col1),DATE): %d", SqlType);
      myassert(SqlType == DateType);

      SQLFreeStmt(hstmt,SQL_CLOSE);

      rc = SQLExecDirect(hstmt,"SELECT CAST(MAX(col1) AS CHAR) AS col1 FROM t_convert",SQL_NTS);
      mystmt(hstmt,rc);

      rc = SQLDescribeCol(hstmt,1,(SQLCHAR *)&ColName,MAX_NAME_LEN,NULL,&SqlType,NULL,NULL,NULL);
      mystmt(hstmt,rc);

      fprintf(stdout,"\n CAST(MAX(col1) AS CHAR): %d", SqlType);
      myassert(SqlType == SQL_VARCHAR);

      SQLFreeStmt(hstmt,SQL_CLOSE);
    }
    
    rc = SQLExecDirect(hstmt,"DROP TABLE t_convert",SQL_NTS);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt,SQL_CLOSE);
}

/*
  Test the bug when two stmts are used with the don't cache results
*/
static void t_cache_bug()
{
  SQLRETURN  rc;
  SQLHENV    henv;
  SQLHDBC    hdbc;
  SQLHSTMT   hstmt1, hstmt2;
  SQLCHAR    conn[MAX_NAME_LEN];

  printMessageHeader();
    
    sprintf(conn,"DRIVER=MyODBC;DSN=%s;USER=%s;PASSWORD=%s;OPTION=1048579",
            mydsn,myuid,mypwd);
    if (mysock != NULL)
    {
      strcat(conn, ";SOCKET=");
      strcat(conn, mysock);
    }
    mydrvconnect(&henv,&hdbc,&hstmt1,conn);  
    
    tmysql_exec(hstmt1,"drop table t_cache");
    rc = tmysql_exec(hstmt1,"create table t_cache(id int)");
    mystmt(hstmt1,rc);

    rc = tmysql_exec(hstmt1,"insert into t_cache values(1)");
    mystmt(hstmt1,rc);

    rc = tmysql_exec(hstmt1,"insert into t_cache values(2)");
    mystmt(hstmt1,rc);

    rc = tmysql_exec(hstmt1,"insert into t_cache values(3)");
    mystmt(hstmt1,rc);

    rc = tmysql_exec(hstmt1,"insert into t_cache values(4)");
    mystmt(hstmt1,rc);

    rc = tmysql_exec(hstmt1,"insert into t_cache values(5)");
    mystmt(hstmt1,rc);

    rc = SQLExecDirect(hstmt1,"select * from t_cache",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLFetch(hstmt1);
    mystmt(hstmt1,rc);

    rc = SQLFetch(hstmt1);
    mystmt(hstmt1,rc);

    rc = SQLAllocHandle(SQL_HANDLE_STMT,hdbc,&hstmt2);
    mycon(hdbc,rc);   
    
    rc = SQLColumns(hstmt2,test_db,SQL_NTS,
                    NULL,0,"t_cache",SQL_NTS,
                    NULL,0);
    mystmt(hstmt2,rc);

    rc = SQLFetch(hstmt2);
    mystmt(hstmt2,rc);

    rc = SQLFetch(hstmt1);
    mystmt(hstmt1,rc);

    rc = SQLFetch(hstmt2);
    myassert(rc == SQL_NO_DATA);

    rc = SQLFetch(hstmt1);
    mystmt(hstmt2,rc);

    rc = SQLFreeHandle(SQL_HANDLE_STMT,hstmt2);
    mystmt(hstmt2,rc);

    rc = SQLFetch(hstmt1);
    mystmt(hstmt2,rc);

    rc = SQLFetch(hstmt1);
    myassert(rc == SQL_NO_DATA);

    rc = SQLFreeStmt(hstmt1, SQL_DROP);
    mystmt(hstmt1,rc);

    rc = SQLDisconnect(hdbc);
    mycon(hdbc,rc);

    rc = SQLFreeConnect(hdbc);
    mycon(hdbc,rc);

    rc = SQLFreeEnv(henv);
    myenv(henv,rc);   
}

/*
  Test the bug when two stmts are used with the don't cache results
*/
static void t_non_cache_bug()
{
  SQLRETURN  rc;
  SQLHENV    henv;
  SQLHDBC    hdbc;
  SQLHSTMT   hstmt1, hstmt2;
  SQLCHAR    conn[MAX_NAME_LEN];

  printMessageHeader();
    
    sprintf(conn,"DRIVER=MyODBC;DSN=%s;USER=%s;PASSWORD=%s;OPTION=3",
            mydsn,myuid,mypwd);
    if (mysock != NULL)
    {
      strcat(conn, ";SOCKET=");
      strcat(conn, mysock);
    }
    mydrvconnect(&henv,&hdbc,&hstmt1,conn);  
    
    tmysql_exec(hstmt1,"drop table t_cache");
    rc = tmysql_exec(hstmt1,"create table t_cache(id int)");
    mystmt(hstmt1,rc);

    rc = tmysql_exec(hstmt1,"insert into t_cache values(1)");
    mystmt(hstmt1,rc);

    rc = tmysql_exec(hstmt1,"insert into t_cache values(2)");
    mystmt(hstmt1,rc);

    rc = tmysql_exec(hstmt1,"insert into t_cache values(3)");
    mystmt(hstmt1,rc);

    rc = tmysql_exec(hstmt1,"insert into t_cache values(4)");
    mystmt(hstmt1,rc);

    rc = tmysql_exec(hstmt1,"insert into t_cache values(5)");
    mystmt(hstmt1,rc);

    rc = SQLExecDirect(hstmt1,"select * from t_cache",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLFetch(hstmt1);
    mystmt(hstmt1,rc);

    rc = SQLFetch(hstmt1);
    mystmt(hstmt1,rc);

    rc = SQLAllocHandle(SQL_HANDLE_STMT,hdbc,&hstmt2);
    mycon(hdbc,rc);   
    
    rc = SQLColumns(hstmt2,test_db,SQL_NTS,
                    NULL,0,"t_cache",SQL_NTS,
                    NULL,0);
    mystmt(hstmt2,rc);

    rc = SQLFetch(hstmt2);
    mystmt(hstmt2,rc);

    rc = SQLFetch(hstmt1);
    mystmt(hstmt1,rc);

    rc = SQLFetch(hstmt2);
    myassert(rc == SQL_NO_DATA);

    rc = SQLFetch(hstmt1);
    mystmt(hstmt2,rc);

    rc = SQLFreeHandle(SQL_HANDLE_STMT,hstmt2);
    mystmt(hstmt2,rc);

    rc = SQLFetch(hstmt1);
    mystmt(hstmt2,rc);

    rc = SQLFetch(hstmt1);
    myassert(rc == SQL_NO_DATA);

    rc = SQLFreeStmt(hstmt1, SQL_DROP);
    mystmt(hstmt1,rc);

    rc = SQLDisconnect(hdbc);
    mycon(hdbc,rc);

    rc = SQLFreeConnect(hdbc);
    mycon(hdbc,rc);

    rc = SQLFreeEnv(henv);
    myenv(henv,rc);   
}

/*
  Test the bug when blob size > 8k
*/
static void t_blob_bug(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN  rc;
  SQLCHAR    *data;
  long       i;
  SQLLEN     length;
  const SQLINTEGER max_blob_size=1024*100;

  printMessageHeader();
        
    SQLExecDirect(hstmt,"drop table t_blob",SQL_NTS);
    rc = SQLExecDirect(hstmt,"create table t_blob(blb long varbinary)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"insert into t_blob values(?)",SQL_NTS);
    mystmt(hstmt,rc);

    if (!(data = (SQLCHAR *)calloc(max_blob_size,sizeof(SQLCHAR))))
    {     
      SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
      SQLFreeStmt(hstmt,SQL_CLOSE);
      return;
    }

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_CHAR,SQL_VARBINARY,
                          0,0,data,0,&length);
    mystmt(hstmt,rc);

    memset(data,'X',max_blob_size);

    fprintf(stdout,"\n inserting %d rows\n", max_blob_size / 1024);
    for (length=1024; length <= max_blob_size; length+= 1024)
    {
      fprintf(stdout,"\r %d", length/1024);
      rc = SQLExecute(hstmt);
      mystmt(hstmt,rc);
    }

    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"SELECT length(blb) FROM t_blob",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&length,0,NULL);
    mystmt(hstmt,rc); 

    for (i= 1; i <= max_blob_size/1024; i++)
    {
      rc = SQLFetch(hstmt);
      mystmt(hstmt,rc);
    
      fprintf(stdout,"\n row %d length: %d", i, length);
      myassert(length == i * 1024);
    }
    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA);

    free(data);

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);
}

/*
  Test the bug SQLTables
*/

typedef struct t_table_bug 
{
  SQLCHAR     szColName[MAX_NAME_LEN];
  SQLSMALLINT pcbColName;
  SQLSMALLINT pfSqlType;
  SQLUINTEGER pcbColDef;
  SQLSMALLINT pibScale;
  SQLSMALLINT pfNullable;
} t_describe_col;


t_describe_col t_tables_bug_data[5] = 
{
  {"TABLE_CAT",   9, SQL_VARCHAR, MYSQL_NAME_LEN, 0, SQL_NULLABLE},
  {"TABLE_SCHEM",11, SQL_VARCHAR, MYSQL_NAME_LEN, 0, SQL_NULLABLE},
  {"TABLE_NAME", 10, SQL_VARCHAR, MYSQL_NAME_LEN, 0, SQL_NULLABLE},
  {"TABLE_TYPE", 10, SQL_VARCHAR, MYSQL_NAME_LEN, 0, SQL_NULLABLE},
  {"REMARKS",     7, SQL_VARCHAR, MYSQL_NAME_LEN, 0, SQL_NULLABLE},
};

static void t_tables_bug(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN   rc;
  SQLSMALLINT i, ColumnCount, pcbColName, pfSqlType, pibScale, pfNullable;
  SQLUINTEGER pcbColDef;
  SQLCHAR     szColName[MAX_NAME_LEN];  

  printMessageHeader();

   SQLFreeStmt(hstmt, SQL_CLOSE);

   rc = SQLTables(hstmt,NULL,0,NULL,0,NULL,0,"'TABLE'",SQL_NTS);
   mystmt(hstmt,rc);

   rc = SQLNumResultCols(hstmt,&ColumnCount);
   mystmt(hstmt,rc);   
   
   fprintf(stdout, "\n total columns in SQLTables: %d", ColumnCount);
   myassert(ColumnCount == 5);

   for (i= 1; i <= ColumnCount; i++)
   {
     rc = SQLDescribeCol(hstmt, (SQLUSMALLINT)i, 
                         szColName,MAX_NAME_LEN,&pcbColName,
                         &pfSqlType,&pcbColDef,&pibScale,&pfNullable);
     mystmt(hstmt,rc);

     fprintf(stdout, "\n Column Number'%d':", i);
     fprintf(stdout, "\n\t Column Name    : %s", szColName);
     fprintf(stdout, "\n\t NameLengh      : %d", pcbColName);
     fprintf(stdout, "\n\t DataType       : %d", pfSqlType);
     fprintf(stdout, "\n\t ColumnSize     : %d", pcbColDef);
     fprintf(stdout, "\n\t DecimalDigits  : %d", pibScale);
     fprintf(stdout, "\n\t Nullable       : %d", pfNullable);

     myassert(strcmp(t_tables_bug_data[i-1].szColName,szColName) == 0);
     myassert(t_tables_bug_data[i-1].pcbColName == pcbColName);
     myassert(t_tables_bug_data[i-1].pfSqlType == pfSqlType);
     myassert(t_tables_bug_data[i-1].pcbColDef == pcbColDef);
     myassert(t_tables_bug_data[i-1].pibScale == pibScale);
     myassert(t_tables_bug_data[i-1].pfNullable == pfNullable);
   }
   SQLFreeStmt(hstmt,SQL_CLOSE);
}

/*
  Test for a simple SQLPutData and SQLParamData handling for longtext
*/
static void t_putdata(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN  rc;
  SQLLEN     pcbLength;
  long       c1;
  SQLCHAR    data[255];
  SQLPOINTER token;

  printMessageHeader();
    

    SQLExecDirect(hstmt,"drop table t_putdata",SQL_NTS);
    rc = SQLExecDirect(hstmt,"create table t_putdata(c1 int, c2 long varchar)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"insert into t_putdata values(?,?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_LONG,
                          SQL_INTEGER,0,0,&c1,0,NULL); 

    rc = SQLBindParameter(hstmt,2,SQL_PARAM_INPUT,SQL_C_CHAR,
                          SQL_LONGVARCHAR,0,0,
                          (SQLPOINTER)1,0,&pcbLength); 

    pcbLength =  SQL_LEN_DATA_AT_EXEC(0);    

    c1 = 10;
    rc = SQLExecute(hstmt);
    myassert(rc == SQL_NEED_DATA);
    
    rc = SQLParamData(hstmt, &token);
    myassert(rc == SQL_NEED_DATA);
     
    strcpy(data,"mysql ab");
    rc = SQLPutData(hstmt,data,6);
    mystmt(hstmt,rc);

    strcpy(data,"- the open source database company");
    rc = SQLPutData(hstmt,data,strlen(data));
    mystmt(hstmt,rc);

    rc = SQLParamData(hstmt, &token);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"select c2 from t_putdata where c1= 10",SQL_NTS);
    mystmt(hstmt,rc); 

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc); 

    pcbLength= 0;
    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &pcbLength);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n data: %s(%d)", data, pcbLength);
    myassert(strcmp(data,"mysql - the open source database company")==0);
    myassert(pcbLength == 40);

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);
}

/*
  Test for a simple SQLPutData and SQLParamData handling for longtext
*/
static void t_putdata1(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN  rc;
  SQLLEN     pcbLength;
  long       c1;
  SQLCHAR    data[255];
  SQLPOINTER token;

  printMessageHeader();
    

    SQLExecDirect(hstmt,"drop table t_putdata",SQL_NTS);
    rc = SQLExecDirect(hstmt,"create table t_putdata(c1 int, c2 long varchar)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into t_putdata values(10,'venu')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"update t_putdata set c2= ? where c1 = ?",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_CHAR,
                          SQL_LONGVARCHAR,0,0,
                          (SQLPOINTER)1,0,&pcbLength); 

    rc = SQLBindParameter(hstmt,2,SQL_PARAM_INPUT,SQL_C_LONG,
                          SQL_INTEGER,0,0,&c1,0,NULL); 

    pcbLength =  SQL_LEN_DATA_AT_EXEC(0);    

    c1 = 10;
    rc = SQLExecute(hstmt);
    myassert(rc == SQL_NEED_DATA);
    
    rc = SQLParamData(hstmt, &token);
    myassert(rc == SQL_NEED_DATA);
     
    strcpy(data,"mysql ab");
    rc = SQLPutData(hstmt,data,6);
    mystmt(hstmt,rc);

    strcpy(data,"- the open source database company");
    rc = SQLPutData(hstmt,data,strlen(data));
    mystmt(hstmt,rc);

    rc = SQLParamData(hstmt, &token);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"select c2 from t_putdata where c1= 10",SQL_NTS);
    mystmt(hstmt,rc); 

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc); 

    pcbLength= 0;
    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &pcbLength);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n data: %s(%d)", data, pcbLength);
    myassert(strcmp(data,"mysql - the open source database company")==0);
    myassert(pcbLength == 40);

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);
}
/*
  Test for a simple SQLPutData and SQLParamData handling for longtext
*/
static void t_putdata2(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN  rc;
  SQLLEN     pcbLength;
  long       c1;
  SQLCHAR    data[255];
  SQLPOINTER token;

  printMessageHeader();    

    SQLExecDirect(hstmt,"drop table t_putdata",SQL_NTS);
    rc = SQLExecDirect(hstmt,"create table t_putdata(c1 int, c2 long varchar, c3 long varchar)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"insert into t_putdata values(?,?,?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_LONG,
                          SQL_INTEGER,0,0,&c1,0,NULL); 

    rc = SQLBindParameter(hstmt,2,SQL_PARAM_INPUT,SQL_C_CHAR,
                          SQL_LONGVARCHAR,0,0,
                          (SQLPOINTER)1,0,&pcbLength); 

    rc = SQLBindParameter(hstmt,3,SQL_PARAM_INPUT,SQL_C_CHAR,
                          SQL_LONGVARCHAR,0,0,
                          (SQLPOINTER)1,0,&pcbLength); 

    pcbLength =  SQL_LEN_DATA_AT_EXEC(0);    

    c1 = 10;
    rc = SQLExecute(hstmt);
    myassert(rc == SQL_NEED_DATA);
    
    rc = SQLParamData(hstmt, &token);
    myassert(rc == SQL_NEED_DATA);
     
    strcpy(data,"mysql ab");
    rc = SQLPutData(hstmt,data,6);
    mystmt(hstmt,rc);

    strcpy(data,"- the open source database company");
    rc = SQLPutData(hstmt,data,strlen(data));
    mystmt(hstmt,rc);

    rc = SQLParamData(hstmt, &token);
    myassert(rc == SQL_NEED_DATA);

    strcpy(data,"MySQL AB");
    rc = SQLPutData(hstmt,data, 8);
    mystmt(hstmt,rc);

    rc = SQLParamData(hstmt, &token);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"select c2,c3 from t_putdata where c1= 10",SQL_NTS);
    mystmt(hstmt,rc); 

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc); 

    pcbLength= 0;
    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &pcbLength);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n data: %s(%d)", data, pcbLength);
    myassert(strcmp(data,"mysql - the open source database company")==0);
    myassert(pcbLength == 40);

    pcbLength= 0;
    rc = SQLGetData(hstmt, 2, SQL_C_CHAR, data, sizeof(data), &pcbLength);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n data: %s(%d)", data, pcbLength);
    myassert(strcmp(data,"MySQL AB")==0);
    myassert(pcbLength == 8);

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);
}
/*
  Test for a simple time struct
*/
static void t_time1(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN       rc;
  SQL_TIME_STRUCT tt;
  SQLCHAR         data[30];
  SQLLEN          length;

  printMessageHeader();    

    SQLExecDirect(hstmt,"drop table t_time",SQL_NTS);
    rc = SQLExecDirect(hstmt,"create table t_time(t time, t1 timestamp, t2 datetime, t3 date)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"insert into t_time(t) values(?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_TYPE_TIME,
                          SQL_TIME,0,0,&tt,0,NULL);

    
    tt.hour= 00;
    tt.minute= 00;
    tt.second= 03;

    rc = SQLExecute(hstmt);
    mystmt(hstmt, rc);

    tt.hour= 01;
    tt.minute= 00;
    tt.second= 00;

    rc = SQLExecute(hstmt);
    mystmt(hstmt, rc);

    tt.hour= 19;
    tt.minute= 00;
    tt.second= 00;

    rc = SQLExecute(hstmt);
    mystmt(hstmt, rc);

    tt.hour= 01;
    tt.minute= 01;
    tt.second= 00;

    rc = SQLExecute(hstmt);
    mystmt(hstmt, rc);

    tt.hour= 01;
    tt.minute= 00;
    tt.second= 01;

    rc = SQLExecute(hstmt);
    mystmt(hstmt, rc);

    tt.hour= 00;
    tt.minute= 01;
    tt.second= 00;

    rc = SQLExecute(hstmt);
    mystmt(hstmt, rc);

    tt.hour= 00;
    tt.minute= 11;
    tt.second= 12;

    rc = SQLExecute(hstmt);
    mystmt(hstmt, rc);

    tt.hour= 01;
    tt.minute= 01;
    tt.second= 01;

    rc = SQLExecute(hstmt);
    mystmt(hstmt, rc);

    tt.hour= 00;
    tt.minute= 00;
    tt.second= 00;

    rc = SQLExecute(hstmt);
    mystmt(hstmt, rc);

    tt.hour= 10;
    tt.minute= 11;
    tt.second= 12;

    rc = SQLExecute(hstmt);
    mystmt(hstmt, rc);

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"select t from t_time",SQL_NTS);
    mystmt(hstmt,rc); 

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc); 

    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &length);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n time: %s(%d)", data, length);
    myassert(strcmp(data,"00:00:03")==0);
    myassert(length == 8); 

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc); 

    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &length);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n time: %s(%d)", data, length);
    myassert(strcmp(data,"01:00:00")==0);
    myassert(length == 8);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc); 

    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &length);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n time: %s(%d)", data, length);
    myassert(strcmp(data,"19:00:00")==0);
    myassert(length == 8);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc); 

    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &length);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n time: %s(%d)", data, length);
    myassert(strcmp(data,"01:01:00")==0);
    myassert(length == 8);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc); 

    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &length);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n time: %s(%d)", data, length);
    myassert(strcmp(data,"01:00:01")==0);
    myassert(length == 8);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc); 

    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &length);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n time: %s(%d)", data, length);
    myassert(strcmp(data,"00:01:00")==0);
    myassert(length == 8);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc); 

    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &length);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n time: %s(%d)", data, length);
    myassert(strcmp(data,"00:11:12")==0);
    myassert(length == 8);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc); 

    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &length);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n time: %s(%d)", data, length);
    myassert(strcmp(data,"01:01:01")==0);
    myassert(length == 8);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc); 

    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &length);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n time: %s(%d)", data, length);

    myassert(strcmp(data,"00:00:00")==0);
    myassert(length == 8);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc); 

    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &length);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n time: %s(%d)", data, length);
    myassert(strcmp(data,"10:11:12")==0);
    myassert(length == 8);

    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA);

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"delete from t_time",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into t_time(t1) values('2003-05-12 10:11:12')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"select t1 from t_time", SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc); 

    rc = SQLGetData(hstmt, 1, SQL_C_TIME, &tt, sizeof(tt), &length);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n time: %d:%d:%d(%d)", tt.hour, tt.minute, tt.second, length);

    myassert(tt.hour == 10 && tt.minute == 11 && tt.second == 12);
    myassert(length == sizeof(SQL_TIME_STRUCT));

    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA);

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"delete from t_time",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into t_time(t2) values('03-12-28 05:59:59')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"select t2 from t_time", SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc); 

    rc = SQLGetData(hstmt, 1, SQL_C_TIME, &tt, sizeof(tt), &length);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n time: %d:%d:%d(%d)", tt.hour, tt.minute, tt.second, length);
    myassert(tt.hour == 05 && tt.minute == 59 && tt.second == 59);
    myassert(length == sizeof(SQL_TIME_STRUCT));

    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA);

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"delete from t_time",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into t_time(t3) values('2003-05-12 10:11:12')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"select t3 from t_time", SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc); 

    rc = SQLGetData(hstmt, 1, SQL_C_TIME, &tt, sizeof(tt), &length);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n time: %d:%d:%d(%d)", tt.hour, tt.minute, tt.second, length);
    myassert(tt.hour == 00 || tt.minute == 00 || tt.second == 00);
    myassert(length == sizeof(SQL_TIME_STRUCT));

    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA);

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);
}

/*
  test for SQL_ATTR_ROW_ARRAY_SIZE
*/
void t_row_array_size(SQLHDBC hdbc,SQLHSTMT hstmt)
{
  SQLRETURN rc;
  long      i, iarray[15];
  SQLUINTEGER nrows;
  const int max_rows=9;

  printMessageHeader();

    SQLExecDirect(hstmt,"drop table t_row_array_size",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table t_row_array_size(id int)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"insert into t_row_array_size values(?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT, SQL_C_ULONG,
                          SQL_INTEGER,0,0,&i,0,NULL);
    mystmt(hstmt,rc);

    for( i = 1; i <= max_rows; i++ )
    {
      rc = SQLExecute(hstmt);
      mystmt(hstmt,rc);
    }

    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLEndTran(SQL_HANDLE_DBC,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    /* set row_size as 2 */
    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROW_ARRAY_SIZE,(SQLPOINTER)2,SQL_IS_UINTEGER);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROWS_FETCHED_PTR,&nrows,SQL_IS_POINTER);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"select * from t_row_array_size",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&iarray,0,NULL);
    mystmt(hstmt,rc);

    /* row 1-2 */
    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    my_assert(nrows == 2);
    my_assert(iarray[0]==1);
    my_assert(iarray[1]==2);

    /* row 3-4 */
    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    my_assert(nrows == 2);
    my_assert(iarray[0]==3);
    my_assert(iarray[1]==4);
    
    /* row 5-6 */
    rc = SQLFetchScroll(hstmt,SQL_FETCH_NEXT,1);/* 1 */    
    mystmt(hstmt,rc);
    my_assert(nrows == 2);
    my_assert(iarray[0]==5);
    my_assert(iarray[1]==6);

    /* row 7-8 */
    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    my_assert(nrows == 2);
    my_assert(iarray[0]==7);
    my_assert(iarray[1]==8);

    /* row 9 */
    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    my_assert(nrows == 1);
    my_assert(iarray[0]==9);

    rc = SQLFetch(hstmt);/* end */    
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    SQLFreeStmt(hstmt,SQL_UNBIND);    
    SQLFreeStmt(hstmt,SQL_CLOSE); 

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROW_ARRAY_SIZE,(SQLPOINTER)1,0);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROWS_FETCHED_PTR,(SQLPOINTER)0,SQL_IS_POINTER);
    mystmt(hstmt,rc);
}

/*
  Test for SQL_ATTR_ROWS_FETCHED_PTR
*/
static void t_rows_fetched_ptr(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLCHAR     name[255];
  SQLSMALLINT pccol;
  SQLRETURN   rc;
  SQLLEN      rows_fetched, pcb_value[4];
  long        i, data[4];
  SQLUSMALLINT row_status[4];

  printMessageHeader(); 
  
    SQLExecDirect(hstmt,"drop table t_rows_fetched_ptr",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table t_rows_fetched_ptr(id int)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"insert into t_rows_fetched_ptr values(?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT, SQL_C_ULONG,
                          SQL_INTEGER,0,0,&i,0,NULL);
    mystmt(hstmt,rc);

    for( i = 1; i <= 3; i++ )
    {
      rc = SQLExecute(hstmt);
      mystmt(hstmt,rc);
    }

    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLEndTran(SQL_HANDLE_DBC,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);   

    rc = SQLExecDirect(hstmt,"select * from t_rows_fetched_ptr",SQL_NTS);
    mystmt(hstmt, rc);

    rc = SQLNumResultCols(hstmt, &pccol);                                                                     
    mystmt(hstmt, rc);
    fprintf(stdout,"\n total columns: %d", pccol);
    myassert(pccol == 1);

    pccol= 0;
    rc = SQLColAttribute(hstmt, 1, SQL_DESC_COUNT, 0, 0, 0, &pccol);                                          
    mystmt(hstmt, rc);
    fprintf(stdout,"\n desc count: %d", pccol);
    myassert(pccol == 1);

    rc = SQLColAttribute(hstmt, 1, SQL_DESC_NAME, &name, 255, 0, &pccol);                                          
    mystmt(hstmt, rc);
    fprintf(stdout,"\n desc name: %s", name);
    assert(strcmp(name,"id") ==0 || strcmp(name, "ID") == 0);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROWS_FETCHED_PTR, &rows_fetched, SQL_IS_POINTER);
    mystmt(hstmt, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)2, SQL_IS_UINTEGER);
    mystmt(hstmt, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_STATUS_PTR, &row_status, SQL_IS_POINTER);
    mystmt(hstmt, rc);

    rc = SQLBindCol(hstmt, 1, SQL_C_LONG, (SQLPOINTER)data, 0, (SQLINTEGER *)pcb_value);
    mystmt(hstmt, rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt, rc);

    fprintf(stdout,"\n fetched ptr: %d", rows_fetched);
    myassert(rows_fetched == 2);
    
    for( i = 0; i < rows_fetched; i++ )
    {   
      fprintf(stdout,"\n row[%d]:", i);
      fprintf(stdout,"\n\t value : %d (%d)", data[i], pcb_value[i]);
      fprintf(stdout,"\n\t status: %d", row_status[i]);      
      myassert(row_status[i] == SQL_ROW_SUCCESS); 
    }

    rc = SQLFetch(hstmt);
    mystmt(hstmt, rc);

    fprintf(stdout,"\n fetched ptr: %d", rows_fetched);
    myassert(rows_fetched == 1);
    
    for( i = 0; i < rows_fetched; i++ )
    {   
      fprintf(stdout,"\n row[%d]:", i);
      fprintf(stdout,"\n\t value : %d (%d)", data[i], pcb_value[i]);
      fprintf(stdout,"\n\t status: %d", row_status[0]);    
      myassert(row_status[i] == SQL_ROW_SUCCESS); 
    }
    myassert(row_status[1] == SQL_ROW_NOROW); 

    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA);

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROW_ARRAY_SIZE,(SQLPOINTER)1,0);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROWS_FETCHED_PTR,(SQLPOINTER)0,SQL_IS_POINTER);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROW_STATUS_PTR,(SQLPOINTER)0,0);
    mystmt(hstmt,rc); 
}

static void t_empty_str_bug(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN    rc;
  long         id;
  SQLLEN       name_len, desc_len;
  SQLCHAR      name[20], desc[20];

  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_empty_str_bug");
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"CREATE TABLE t_empty_str_bug(Id int NOT NULL,\
                                                        Name varchar(10) default NULL, \
                                                        Description varchar(10) default NULL, \
                                                        PRIMARY KEY  (Id))");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  

    rc = SQLSetCursorName(hstmt,"venu",SQL_NTS);
    mystmt(hstmt,rc);    

    rc = tmysql_exec(hstmt,"select * from t_empty_str_bug");
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&id,0,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,&name,100,&name_len);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,3,SQL_C_CHAR,&desc,100,&desc_len);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,NULL,NULL);
    myassert(rc == SQL_NO_DATA_FOUND);

    id= 10;
    strcpy(name,"MySQL AB");name_len= SQL_NTS;
    strcpy(desc,"");desc_len= SQL_COLUMN_IGNORE;

    rc = SQLSetPos(hstmt,1,SQL_ADD,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&name_len);
    mystmt(hstmt,rc);
    
    fprintf(stdout," rows affected:%d\n",name_len);    
    myassert(name_len == 1);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_empty_str_bug");
    mystmt(hstmt,rc);  

    my_assert( 1 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_empty_str_bug");
    mystmt(hstmt,rc);  

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,1,NULL,NULL);
    mystmt(hstmt,rc);  
    
    name[0]='\0';
    my_assert(10 == my_fetch_int(hstmt,1));
    my_assert(!strcmp((const char *)"MySQL AB",my_fetch_str(hstmt,name,2)));
    my_assert(!strcmp((const char *)"MySQL AB",my_fetch_str(hstmt,name,3))); /* NULL */

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}

static void t_current_catalog(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLCHAR     cur_db[255], db[255];
  SQLRETURN   rc;
  SQLUINTEGER len;

  printMessageHeader();

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLGetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (char *)db, 255, &len);
    mycon(hdbc,rc);
    fprintf(stdout,"\n current_catalog: %s (%ld)", db, len);
    myassert(strcmp(db, test_db) == 0 || strlen(test_db) == len);

    rc = SQLSetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (char *)db, SQL_NTS);
    mycon(hdbc,rc);

    SQLExecDirect(hstmt, "DROP DATABASE t_odbc_test_cur_catalog", SQL_NTS);

    strcpy(cur_db, "t_odbc_test_cur_catalog");
    rc = SQLSetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (char *)cur_db, SQL_NTS);
    mycon_r(hdbc,rc);

    rc = SQLExecDirect(hstmt, "CREATE DATABASE t_odbc_test_cur_catalog", SQL_NTS);
    mystmt(hstmt,rc);

    strcpy(cur_db, "t_odbc_test_cur_catalog");
    rc = SQLSetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (char *)cur_db, SQL_NTS);
    mycon(hdbc,rc);

    rc = SQLGetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (char *)db, 255, &len);
    mycon(hdbc,rc);
    fprintf(stdout,"\n current_catalog: %s (%ld)", db, len);
    myassert(strcmp(cur_db, db) == 0 || strlen(cur_db) == len);

    strcpy(cur_db, "t_odbc_test_cur_catalog-test-12455");
    rc = SQLSetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (char *)cur_db, SQL_NTS);
    mycon_r(hdbc,rc);

    rc = SQLSetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (char *)cur_db, len);
    mycon(hdbc,rc);

    /* reset for further tests */
    rc = SQLSetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (char *)test_db, SQL_NTS);
    mycon(hdbc,rc);

    rc = SQLExecDirect(hstmt, "DROP DATABASE t_odbc_test_cur_catalog", SQL_NTS);
    mycon(hstmt,rc);    
}

#if 0 /* to be enabled only for 3.52 which has true prepared statements */
static void desc_param_check(SQLHSTMT     hstmt, 
                             SQLUSMALLINT pno,
                             SQLSMALLINT  dtype,
                             SQLUINTEGER  psize,
                             SQLSMALLINT  ddigits,
                             SQLSMALLINT  lnullable)
{
  SQLRETURN   rc;
  SQLUINTEGER ParamSize;
  SQLSMALLINT ParamType, DecDigits, Nullable;

    rc = SQLDescribeParam(hstmt, pno, &ParamType, &ParamSize, &DecDigits, &Nullable);
    mystmt(hstmt,rc);

    fprintf(stdout,"\n\n parameter %d:", pno);
    fprintf(stdout,"\n\t type    : %d", ParamType);
    fprintf(stdout,"\n\t size    : %ld", ParamSize);
    fprintf(stdout,"\n\t decimals: %d", DecDigits);
    fprintf(stdout,"\n\t nullable: %s", Nullable ? "SQL_NULLABLE": "SQL_NO_NULLS");

    myassert(dtype == ParamType);
    myassert(psize == ParamSize);
    myassert(ddigits == DecDigits);
    myassert(lnullable == Nullable);
}


/* To test SQLDescribeParam */
static void t_desc_param(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN   rc;
  SQLSMALLINT ParamCount;
  
  printMessageHeader();
  
    SQLFreeStmt(hstmt, SQL_CLOSE);
    
    SQLExecDirect(hstmt,"DROP TABLE t_desc_param",SQL_NTS);
    
    rc = SQLExecDirect(hstmt,"CREATE TABLE t_desc_param(c1  integer, \
                                                        c2  binary(2) NOT NULL,\
                                                        c3  char(10), \
                                                        c4  varchar(5),\
                                                        c5  decimal(10,3) NOT NULL,\
                                                        c6  tinyint,\
                                                        c7  smallint,\
                                                        c8  numeric(4,2),\
                                                        c9  real,\
                                                        c10 float(5),\
                                                        c11 bigint NOT NULL,\
                                                        c12 varbinary(12))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"INSERT INTO t_desc_param VALUES(?,?,?,?,?,?,?,?,?,?,?,?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLNumParams(hstmt, &ParamCount);
    mystmt(hstmt,rc);
    fprintf(stdout,"\n total parameters: %d", ParamCount);
    my_assert(ParamCount == 12);

    desc_param_check(hstmt, 1,  SQL_INTEGER,      10, 0, SQL_NULLABLE);
    desc_param_check(hstmt, 2,  SQL_BINARY,       2,  0, SQL_NO_NULLS);
    desc_param_check(hstmt, 3,  SQL_CHAR,         10, 0, SQL_NULLABLE);
    desc_param_check(hstmt, 4,  SQL_VARCHAR,      5,  0, SQL_NULLABLE);
    desc_param_check(hstmt, 5,  SQL_DECIMAL,      10, 3, SQL_NO_NULLS);
    desc_param_check(hstmt, 6,  SQL_TINYINT,      3,  0, SQL_NULLABLE);
    desc_param_check(hstmt, 7,  SQL_SMALLINT,     5,  0, SQL_NULLABLE);
    desc_param_check(hstmt, 8,  SQL_NUMERIC,      4,  2, SQL_NULLABLE);
    desc_param_check(hstmt, 9,  SQL_REAL,         24, 0, SQL_NULLABLE);
    desc_param_check(hstmt, 10, SQL_REAL,         24, 0, SQL_NULLABLE);
    desc_param_check(hstmt, 11, SQL_BIGINT,       19, 0, SQL_NO_NULLS);
    desc_param_check(hstmt, 12, SQL_VARBINARY,    12, 0, SQL_NULLABLE);

    SQLFreeStmt(hstmt,SQL_CLOSE);
}
#endif

static void t_rows_fetched_ptr1(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN   rc;
  SQLLEN      rowsFetched, rowsSize;
  long        i;
  
  printMessageHeader();
    
    SQLExecDirect(hstmt,"drop table t_rows_fetched_ptr",SQL_NTS);
    
    rc = SQLExecDirect(hstmt,"create table t_rows_fetched_ptr(a int)",SQL_NTS);
    mystmt(hstmt,rc);
    
    rc = SQLExecDirect(hstmt,"insert into t_rows_fetched_ptr values(0)",SQL_NTS);
    mystmt(hstmt,rc);
    
    rc = SQLExecDirect(hstmt,"insert into t_rows_fetched_ptr values(1)",SQL_NTS);
    mystmt(hstmt,rc);
    
    rc = SQLExecDirect(hstmt,"insert into t_rows_fetched_ptr values(2)",SQL_NTS);
    mystmt(hstmt,rc);
    
    rc = SQLExecDirect(hstmt,"insert into t_rows_fetched_ptr values(3)",SQL_NTS);
    mystmt(hstmt,rc);
    
    rc = SQLExecDirect(hstmt,"insert into t_rows_fetched_ptr values(4)",SQL_NTS);
    mystmt(hstmt,rc);
    
    rc = SQLExecDirect(hstmt,"insert into t_rows_fetched_ptr values(5)",SQL_NTS);
    mystmt(hstmt,rc);

    rowsSize= 1;
    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)rowsSize, 0);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROWS_FETCHED_PTR, &rowsFetched, 0);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt, "SELECT * FROM t_rows_fetched_ptr",SQL_NTS);
    mystmt(hstmt,rc);

    i= 0;
    rc = SQLFetchScroll(hstmt,SQL_FETCH_NEXT,0);
    while (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    {
      fprintf(stdout,"\n total rows fetched: %ld", rowsFetched);
      myassert(rowsFetched == rowsSize);
      i++; rowsFetched= 0;
      rc = SQLFetch(hstmt);
    }
    myassert( i == 6);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rowsSize= 2;
    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)rowsSize, 0);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROWS_FETCHED_PTR, &rowsFetched, 0);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt, "SELECT * FROM t_rows_fetched_ptr",SQL_NTS);
    mystmt(hstmt,rc);

    i= 0;
    rc = SQLFetchScroll(hstmt,SQL_FETCH_NEXT,0);
    while (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    {
      fprintf(stdout,"\n total rows fetched: %ld", rowsFetched);
      myassert(rowsFetched == rowsSize);
      i++;rowsFetched= 0;
      rc = SQLFetch(hstmt);
    }
    myassert( i == 3);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rowsSize= 3;
    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)rowsSize, 0);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROWS_FETCHED_PTR, &rowsFetched, 0);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt, "SELECT * FROM t_rows_fetched_ptr",SQL_NTS);
    mystmt(hstmt,rc);

    i= 0;
    rc = SQLFetchScroll(hstmt,SQL_FETCH_NEXT,0);
    while (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    {
      printMessage("\n total rows fetched: %ld", rowsFetched);
      myassert(rowsFetched == rowsSize);
      i++;rowsFetched= 0;
      rc = SQLFetch(hstmt);
    }
    myassert( i == 2);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rowsSize= 4;
    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)rowsSize, 0);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROWS_FETCHED_PTR, &rowsFetched, 0);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt, "SELECT * FROM t_rows_fetched_ptr",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    printMessage("\n total rows fetched: %ld", rowsFetched);
    myassert(rowsFetched == rowsSize);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    printMessage("\n total rows fetched: %ld", rowsFetched);
    myassert(rowsFetched == 2);

    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA);
    
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)1, 0);/* reset */
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROWS_FETCHED_PTR, NULL, 0);
    mystmt(hstmt,rc);
}

static void desc_col_check(SQLHSTMT hstmt, 
                           SQLUSMALLINT icol,
                           const char *name,
                           SQLSMALLINT sql_type,
                           SQLUINTEGER col_def,
                           SQLUINTEGER col_def1,
                           SQLSMALLINT scale,
                           SQLSMALLINT nullable)
{
  SQLRETURN   rc;
  SQLSMALLINT pcbColName, pfSqlType, pibScale, pfNullable;
  SQLUINTEGER pcbColDef;
  SQLCHAR     szColName[MAX_NAME_LEN];  

  rc = SQLDescribeCol(hstmt, icol, 
                      szColName,MAX_NAME_LEN,&pcbColName,
                      &pfSqlType,&pcbColDef,&pibScale,&pfNullable);
  mystmt(hstmt,rc);

  fprintf(stdout, "\n\n Column Number'%d':", icol);

  fprintf(stdout, "\n\t Column Name    : %s", szColName);
  fprintf(stdout, "\n\t NameLengh      : %d", pcbColName);
  fprintf(stdout, "\n\t DataType       : %d", pfSqlType);
  fprintf(stdout, "\n\t ColumnSize     : %d", pcbColDef);
  fprintf(stdout, "\n\t DecimalDigits  : %d", pibScale);
  fprintf(stdout, "\n\t Nullable       : %d", pfNullable);

  myassert(strcmp(name,szColName) == 0);
  myassert(sql_type == pfSqlType);
  myassert(col_def == pcbColDef || col_def1 == pcbColDef);
  myassert(scale == pibScale);
  myassert(nullable == pfNullable);
}

/* To test SQLDescribeCol */
static void t_desc_col(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN   rc;
  SQLSMALLINT ColumnCount;
  
  printMessageHeader();
  
    SQLFreeStmt(hstmt, SQL_CLOSE);
    
    SQLExecDirect(hstmt,"DROP TABLE t_desc_col",SQL_NTS);
    
    rc = SQLExecDirect(hstmt,"CREATE TABLE t_desc_col(c1  integer, \
                                                      c2  binary(2) NOT NULL,\
                                                      c3  char(1), \
                                                      c4  varchar(5),\
                                                      c5  decimal(10,3) NOT NULL,\
                                                      c6  tinyint,\
                                                      c7  smallint,\
                                                      c8  numeric(4,2),\
                                                      c9  real,\
                                                      c10 float(5),\
                                                      c11 bigint NOT NULL,\
                                                      c12 varbinary(12),\
                                                      c13 char(20) NOT NULL,\
                                                      c14 float(10,3),\
                                                      c15 tinytext,\
                                                      c16 text,\
                                                      c17 mediumtext,\
                                                      c18 longtext,\
                                                      c19 tinyblob,\
                                                      c20 blob,\
                                                      c21 mediumblob,\
                                                      c22 longblob,\
                                                      c23 tinyblob)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"SELECT * FROM t_desc_col",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLNumResultCols(hstmt, &ColumnCount);
    mystmt(hstmt,rc);

    fprintf(stdout,"\n total columns: %d", ColumnCount);
    my_assert(ColumnCount == 23);

    desc_col_check(hstmt, 1,  "c1",  SQL_INTEGER,   10, 11, 0,  SQL_NULLABLE);
    desc_col_check(hstmt, 2,  "c2",  SQL_BINARY,    2,  2,  0,  SQL_NO_NULLS);
    desc_col_check(hstmt, 3,  "c3",  SQL_CHAR,      1,  1,  0,  SQL_NULLABLE);
    desc_col_check(hstmt, 4,  "c4",  SQL_VARCHAR,   5,  5,  0,  SQL_NULLABLE);
    desc_col_check(hstmt, 5,  "c5",  SQL_DECIMAL,   10, 10, 3,  SQL_NO_NULLS);
    desc_col_check(hstmt, 6,  "c6",  SQL_TINYINT,   3,  4,  0,  SQL_NULLABLE);
    desc_col_check(hstmt, 7,  "c7",  SQL_SMALLINT,  5,  6,  0,  SQL_NULLABLE);
    desc_col_check(hstmt, 8,  "c8",  SQL_DECIMAL,   4,  4,  2,  SQL_NULLABLE);
    desc_col_check(hstmt, 9,  "c9",  SQL_DOUBLE,    22, 24, 31, SQL_NULLABLE);
    desc_col_check(hstmt, 10, "c10", SQL_REAL,      12, 24, 31, SQL_NULLABLE);
    desc_col_check(hstmt, 11, "c11", SQL_BIGINT,    19, 19, 0,  SQL_NO_NULLS);
    desc_col_check(hstmt, 12, "c12", SQL_VARBINARY, 12, 12, 0,  SQL_NULLABLE);
    desc_col_check(hstmt, 13, "c13", SQL_CHAR,      20, 20, 0,  SQL_NO_NULLS);
    desc_col_check(hstmt, 14, "c14", SQL_REAL,      10, 24, 3,  SQL_NULLABLE);
    desc_col_check(hstmt, 15, "c15", SQL_LONGVARCHAR, 255, 255, 0,  SQL_NULLABLE);
    desc_col_check(hstmt, 16, "c16", SQL_LONGVARCHAR, 65535, 65535, 0,  SQL_NULLABLE);
    desc_col_check(hstmt, 17, "c17", SQL_LONGVARCHAR, 16777215, 16777215, 0,  SQL_NULLABLE);
    desc_col_check(hstmt, 18, "c18", SQL_LONGVARCHAR, 4294967295 , 16777215 , 0,  SQL_NULLABLE);
    desc_col_check(hstmt, 19, "c19", SQL_LONGVARBINARY, 255, 255, 0,  SQL_NULLABLE);
    desc_col_check(hstmt, 20, "c20", SQL_LONGVARBINARY, 65535, 65535, 0,  SQL_NULLABLE);
    desc_col_check(hstmt, 21, "c21", SQL_LONGVARBINARY, 16777215, 16777215, 0,  SQL_NULLABLE);
    desc_col_check(hstmt, 22, "c22", SQL_LONGVARBINARY, 4294967295 , 16777215 , 0,  SQL_NULLABLE);
    desc_col_check(hstmt, 23, "c23", SQL_LONGVARBINARY, 255, 5, 0,  SQL_NULLABLE);

    SQLFreeStmt(hstmt,SQL_CLOSE);
}

/*
  Test for a simple SQLPutData and SQLParamData handling 
  bug #1316
*/

static void t_putdata3(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  char buffer[]= "MySQL - The worlds's most popular open source database";
  SQLRETURN  rc;  
  const int MAX_PART_SIZE = 5;

  char *pdata= 0, data[50];
  int dynData;
  int commonLen= 20;

  long        id, id1, id2, id3, resId;
  long        resUTimeSec;
  long        resUTimeMSec;
  long        resDataLen;
  long        resData;


  printMessageHeader();
    

    SQLExecDirect(hstmt,"drop table t_putdata3",SQL_NTS);
    rc = SQLExecDirect(hstmt,"CREATE TABLE t_putdata3 ( id INT, id1  INT, \
                     id2 INT, id3  INT, pdata blob);",SQL_NTS);
    mystmt(hstmt,rc);

    dynData = 1; 

    rc = SQLPrepare(hstmt, "INSERT INTO t_putdata3 VALUES ( ?, ?, ?, ?, ? )", SQL_NTS);
    mystmt(hstmt,rc);

    id= 1, id1= 2, id2= 3, id3= 4;
    resId = 0;
    resUTimeSec = 0;
    resUTimeMSec = 0;
    resDataLen = 0;
    resData = SQL_LEN_DATA_AT_EXEC(0);

    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG,
                          SQL_INTEGER, 0, 0, &id, 0, &resId);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG,
                          SQL_INTEGER, 0, 0, &id1, 0, &resUTimeSec);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_SLONG,
                          SQL_INTEGER, 0, 0, &id2, 0, &resUTimeMSec);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_SLONG,
                          SQL_INTEGER, 0, 0, &id3, 0, 
                          &resDataLen);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT,
                          SQL_C_BINARY, SQL_LONGVARBINARY, 10, 10, 
                          dynData ? (SQLPOINTER)5 :
                          pdata, 0, &resData);
    mystmt(hstmt,rc);

    rc = SQLExecute(hstmt);
    if (rc == SQL_NEED_DATA)
    {
      int parameter;
      if (SQLParamData(hstmt,(void**)&parameter) == SQL_NEED_DATA && parameter == 5)
      {
        int len = 0;
        int partsize;

        /* storing long data by parts */
        while (len < commonLen)
        {
          partsize = commonLen - len;
          if (partsize > MAX_PART_SIZE) 
            partsize = MAX_PART_SIZE;
          
          rc = SQLPutData(hstmt, buffer+len, partsize);
          mystmt(hstmt,rc);
          len += partsize;
        }
        if (SQLParamData(hstmt,(void**)&parameter) == SQL_ERROR)
        {
           
        }
      }
    } /* end if (rc == SQL_NEED_DATA) */

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    if (mysql_min_version(hdbc, "4.0", 3))
    {
      rc = tmysql_exec(hstmt,"select id, id1, id2, id3,  convert(pdata,char) from t_putdata3");
      mystmt(hstmt,rc);  

      rc = SQLFetch(hstmt);

      my_assert(1 == my_fetch_int(hstmt,1));
      my_assert(2 == my_fetch_int(hstmt,2));
      my_assert(3 == my_fetch_int(hstmt,3));
      my_assert(4 == my_fetch_int(hstmt,4));

      my_assert(strncmp(buffer, my_fetch_str(hstmt,data,5), commonLen) == 0);
    }
    else
    {
      rc = tmysql_exec(hstmt,"select id, id1, id2, id3,  pdata from t_putdata3");
      mystmt(hstmt,rc);  

      rc = SQLFetch(hstmt);

      my_assert(1 == my_fetch_int(hstmt,1));
      my_assert(2 == my_fetch_int(hstmt,2));
      my_assert(3 == my_fetch_int(hstmt,3));
      my_assert(4 == my_fetch_int(hstmt,4));
      my_assert(strncmp("4D7953514C202D2054686520776F726C64732773", 
                my_fetch_str(hstmt,data,5), commonLen) == 0);
    }

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    /*
     output:

      ######################################
      t_putdata3
      ######################################
       my_fetch_int: 1
       my_fetch_int: 2
       my_fetch_int: 3
       my_fetch_int: 4
       my_fetch_str: MySQL - The worlds's(20)
    */
}

/*
  Test for misc CONVERT
  bug #1082
*/
static void t_convert(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN  rc;
  SQLLEN     data_len;
  SQLCHAR    data[50];

  printMessageHeader();

    tmysql_exec(hstmt,"drop table t_convert");
    
    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"CREATE TABLE t_convert(testing tinytext)");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"INSERT INTO t_convert VALUES('record1')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"INSERT INTO t_convert VALUES('record2')");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"SELECT CONCAT(testing, '-must be string') FROM t_convert ORDER BY RAND()");
    mystmt(hstmt,rc); 

    rc = SQLBindCol(hstmt,1,SQL_C_CHAR, &data, 100, &data_len);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(strcmp(data,"record1-must be string") == 0 || 
             strcmp(data,"record2-must be string") == 0);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(strcmp(data,"record1-must be string") == 0 || 
             strcmp(data,"record2-must be string") == 0);

    rc = SQLFetch(hstmt);
    myassert( rc == SQL_NO_DATA);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);
    
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}

/**
  mytest rountine to control individual tests
*/
static void mytest(int tno, SQLHENV henv, SQLHDBC hdbc, SQLHSTMT hstmt)
{  
  if( tno == 1 || tno == -1) /* basic and general */
  {
    t_basic(hdbc, hstmt);
    t_error(hdbc,hstmt);
  }

  if( (tno == 2 || tno == -1) && driver_supports_setpos(hdbc))
  {       
     /* sqlsetpos */
    t_pos_datetime_delete(hdbc,hstmt);
    t_pos_datetime_delete1(hdbc,hstmt);
    /* t_setpos_upd_decimal(hdbc,hstmt); */
    t_pos_column_ignore(hdbc,hstmt);
    /* my_setpos_upd_pk_order(hdbc,hstmt); */
    /* my_setpos_upd_pk_order1(hdbc,hstmt); */
    t_mul_pkdel(hdbc,hstmt);
    t_mul_pkdel1(hdbc,hstmt);
#if 0
    tmysql_setpos_pkdel(hdbc,hstmt);
    tmysql_setpos_pkdel1(hdbc,hstmt);
    tmysql_setpos_pkdel2(hdbc,hstmt);
    tmysql_setpos_pkdel3(hdbc,hstmt);
    tmysql_setpos_del(hdbc,hstmt);
    tmysql_setpos_del1(hdbc,hstmt);
    tmysql_setpos_upd(hdbc,hstmt);
    tmysql_mtab_setpos_del(hdbc,hstmt); 
    t_alias_setpos_pkdel(hdbc,hstmt);    
    t_setpos_position(hdbc,hstmt);
    t_pos_column_ignore(hdbc,hstmt);
    t_setpos_del_all(hdbc,hstmt);
#endif
    t_refresh(hdbc,hstmt);
    t_empty_str_bug(hdbc, hstmt);

#if DONT_WORK
    t_alias_setpos_del(hdbc,hstmt); /* can't work until 4.1 supports aliases in FIELDS */
#endif    
  }
  if( (tno == 3 || tno == -1) && driver_supports_setpos(hdbc))    
  {
    /* positioned updates and deletes */
    tmysql_pos_delete(hdbc,hstmt);
    tmysql_pos_update(hdbc,hstmt); /* it doesn't work with 4.1 */
    tmysql_pos_update_ex(hdbc,hstmt);
    tmysql_pos_update_ex1(hdbc,hstmt);
    tmysql_pos_update_ex2(hdbc,hstmt);
    tmysql_pos_update_ex3(hdbc,hstmt);    
    t_acc_crash(hdbc,hstmt);
    tmysql_pcbvalue(hdbc,hstmt);
  }
  if( tno == 4 || tno == -1)    /* catalogs */
  {     
    tmysql_specialcols(hdbc, hstmt);
  }
  if( tno == 5 || tno == -1)    /* transaction */
  {      
    t_tran(hdbc,hstmt); 
  }
  if( tno == 6 || tno == -1)    /* keys */
  {     
    tmysql_showkeys(hdbc,hstmt);    
  }
  if( tno == 7 || tno == -1)    /* param binding */
  {  
#if 0
    t_desc_param(hdbc,hstmt);
#endif    
  }
  if( tno == 8 || tno == -1)    /* row binding */
  {     
    tmysql_bindcol(hdbc,hstmt);   
  }
  if( tno == 9 || tno == -1)    /* conversion */
  {     
    t_convert(hdbc, hstmt);
    t_tstotime(hdbc,hstmt);
    t_tstotime1(hdbc,hstmt);
  }
  if( tno == 10 || tno == -1)   /* type checking */
  {    
    t_longlong1(hdbc,hstmt);
    t_bigint(hdbc,hstmt);
    t_enumset(hdbc,hstmt);
    t_gettypeinfo(hdbc,hstmt);
    t_getinfo(hdbc);
  }
  if( tno == 12 || tno == -1)   /* myodbc3 tests */
  {
    t_putdata3(hdbc, hstmt);
    t_time1(hdbc,hstmt);
#if 0
    t_blob_bug(hdbc,hstmt); /* TO BE FIXED IN DEBUG MODE */
#endif
    t_putdata(hdbc,hstmt);
    t_putdata1(hdbc,hstmt);
    t_putdata2(hdbc,hstmt);
    t_tables_bug(hdbc,hstmt); /* To be fixed in 3.52 */
    t_convert_type(henv,hdbc,hstmt);
#if SQLColumns_NOT_BROKEN
    t_columns(hdbc,hstmt);
#endif
    t_multistep(hdbc,hstmt);
    t_warning(hdbc,hstmt);
    t_scroll(hdbc,hstmt);
    t_getcursor(hdbc);
    t_getcursor1(hdbc);
    t_prepare(hdbc,hstmt);
    t_prepare1(hdbc,hstmt);
    t_msdev_bug(hdbc,hstmt);
    t_time(hdbc,hstmt);
    t_diagrec(henv,hdbc,hstmt);
    t_decimal(hdbc,hstmt);
    t_numeric(hdbc,hstmt);
    t_zerolength(hdbc,hstmt);
    t_odbc3_envattr();
    t_odbc3_handle();
    t_text_fetch(hdbc,hstmt);
    t_non_cache_bug(hdbc,hstmt);
    t_cache_bug();
    
  } 
  if( tno == 13 || tno == -1)   /* stmt attributes */
  {       
    t_desc_col(hdbc, hstmt);
    t_max_rows(hdbc,hstmt);
    t_stmt_attr_status(hdbc,hstmt);
    t_rows_fetched_ptr(hdbc, hstmt);
    t_rows_fetched_ptr1(hdbc, hstmt);
    t_row_array_size(hdbc, hstmt);
    t_current_catalog(hdbc, hstmt);
  } 
  if( tno == 11 || tno == -1)   /* limit tests */
  {     
    t_max_select(hdbc,hstmt);
    t_max_con(hstmt);
  } 
}


/**
  main routine to control all tests
*/
int main(int argc, char *argv[])
{
  SQLHENV   henv;
  SQLHDBC   hdbc;
  SQLHSTMT  hstmt;
  SQLINTEGER narg, tno = -1;      
  
    /*
     *  show the usage string when the user asks for this
    */
    if (argc < 2 || ( argc == 2 && ((!strcmp (argv[1], "-?")  || 
                                     !strcmp (argv[1], "--?") || 
                                     !strcmp (argv[1], "--h") ||
                                     !strcmp (argv[1], "--help")
                                   ))
                    ) || argc > 6
       )
    {
      fprintf(stdout,"------------------------------------------\n");
      fprintf(stdout,"usage: mytest3 testno [DSN] [UID] [PWD] [SOCK]\n\n");      
      fprintf(stdout,"       testno <-- test number\n");
      fprintf(stdout,"       DSN    <-- data source name\n");
      fprintf(stdout,"       UID    <-- user name\n");
      fprintf(stdout,"       PWD    <-- password\n");
      fprintf(stdout,"       SOCK   <-- socket path\n");
      
      fprintf(stdout,"\ntestno:\n");            
      fprintf(stdout,"   -1 : all\n");            
      fprintf(stdout,"    1 : basic, general\n");            
      fprintf(stdout,"    2 : sqlsetpos\n");            
      fprintf(stdout,"    3 : positioned update/delete\n");            
      fprintf(stdout,"    4 : catalog\n");            
      fprintf(stdout,"    5 : transaction\n");            
      fprintf(stdout,"    6 : keys\n");            
      fprintf(stdout,"    7 : param binding\n");            
      fprintf(stdout,"    8 : row binding\n");            
      fprintf(stdout,"    9 : type conversion\n");            
      fprintf(stdout,"   10 : data types\n");            
      fprintf(stdout,"   11 : limit \n");            
      fprintf(stdout,"   12 : myodbc3 \n");            
      fprintf(stdout,"   13 : stmt attributes \n");            
      fprintf(stdout,"------------------------------------------\n");
      exit(0);
    }   

    /*
     * if connection string supplied through arguments, overrite
     * the default one..
    */
    for(narg = 1; narg < argc; narg++)
    {
      if ( narg == 1 )
        tno = atoi(argv[1]);
      else if ( narg == 2 )
        mydsn = argv[2];
      else if ( narg == 3 )
        myuid = argv[3];
      else if ( narg == 4 )
        mypwd = argv[4];
      else if ( narg == 5 )
        mysock= argv[5];
    }

    myconnect(&henv,&hdbc,&hstmt);
    mytest(tno,henv,hdbc,hstmt);
    mydisconnect(&henv,&hdbc,&hstmt);

  fprintf(stdout,"\n\n-- test-end --\n");
  return(0);
} 




