/*
  Copyright (C) 1995-2007 MySQL AB

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  There are special exceptions to the terms and conditions of the GPL
  as it is applied to this software. View the full text of the exception
  in file LICENSE.exceptions in the top-level directory of this software
  distribution.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "odbctap.h"

/**
 Simple function to do basic ops with MySQL
*/
DECLARE_TEST(t_basic)
{
    SQLRETURN rc;
    SQLINTEGER nInData= 1, nOutData;
    SQLROWCOUNT nRowCount= 0;
    char szOutData[31]={0};

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
    rc = SQLPrepare(hstmt,"insert into tmyodbc values(?,'param_insert')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT, SQL_C_LONG,SQL_INTEGER,
                          0,0,&nInData,0,NULL);
    mystmt(hstmt,rc);

    for (nInData=20 ; nInData<100; nInData=nInData+10)
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

    nInData = 10;
    while (SQLFetch(hstmt) == SQL_SUCCESS)
    {
        nRowCount++;
        printMessage("\n row %d\t: %d,%s",nRowCount, nOutData, szOutData);
        my_assert( nInData == nOutData);
        nInData += 10;
    }
    printMessage("\n total rows Found:%d\n",nRowCount);
    my_assert( nRowCount == (nInData-10)/10);

    /* FREE THE OUTPUT BUFFERS */
    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}

DECLARE_TEST(tmysql_setpos_del)
{
    SQLRETURN rc;
    SQLINTEGER nData = 500;
    SQLROWCOUNT nlen;
    char      szData[255]={0};
    SQLROWSETSIZE pcrow;
    SQLUSMALLINT rgfRowStatus;

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

    printMessage(" pcrow:%d\n",pcrow);

    printMessage(" row1:%d,%s\n",nData,szData);

    rc = SQLSetPos(hstmt,1,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLNumResultCols(hstmt,&rgfRowStatus);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);

    printMessage(" rows affected:%d\n",nlen);

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

  return OK;
}


DECLARE_TEST(tmysql_setpos_del1)
{
    SQLRETURN rc;
    long nData= 500;
    SQLLEN nlen;
    SQLCHAR szData[255]={0};
    SQLROWSETSIZE pcrow;

    tmysql_exec(hstmt,"drop table tmysql_setpos");
    rc = tmysql_exec(hstmt,"create table tmysql_setpos(col1 int, col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_setpos values(100,'MySQL1')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos values(200,'MySQL2')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos values(300,'MySQL3')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos values(400,'MySQL4')");
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

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,100,&nlen);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,3,&pcrow,NULL);
    mystmt(hstmt,rc);

    printMessage(" pcrow:%d\n",pcrow);

    printMessage(" row1:%d,%s\n",nData,szData);

    rc = SQLSetPos(hstmt,0,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);

    printMessage(" rows affected:%d\n",nlen);
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

  return OK;
}


DECLARE_TEST(tmysql_setpos_upd)
{
    SQLRETURN rc;
    long nData = 500;
    SQLLEN nlen;
    SQLCHAR szData[255]={0};
    SQLROWSETSIZE pcrow;

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

    printMessage(" pcrow:%d\n",pcrow);

    printMessage(" row1:%d,%s\n",nData,szData);

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    nData = 1000;
    strcpy((char *)szData , "updated");

    rc = SQLSetPos(hstmt,3,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt_err(hstmt,rc== SQL_ERROR,rc);

    rc = SQLSetPos(hstmt,1,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);

    printMessage(" rows affected:%d\n",nlen);

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

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);
    printMessage("\n total rows affceted:%d",nlen);
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

  return OK;
}


DECLARE_TEST(tmysql_setpos_add)
{
    SQLRETURN rc;
    long nData= 500;
    SQLLEN nlen;
    SQLCHAR szData[255]={0};
    SQLROWSETSIZE pcrow;

    tmysql_exec(hstmt,"drop table tmysql_setpos_add");
    rc = tmysql_exec(hstmt,"create table tmysql_setpos_add(col1 int, col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_setpos_add values(100,'MySQL1')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into tmysql_setpos_add values(300,'MySQL3')");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLSetCursorName(hstmt,"venu",SQL_NTS);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from tmysql_setpos_add");
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,NULL);
    mystmt(hstmt,rc);

    nData = 1000;
    strcpy((char *)szData , "insert-new1");

    rc = SQLSetPos(hstmt,3,SQL_ADD,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);

    printMessage("rows affected:%d\n",nlen);

    strcpy((char *)szData , "insert-new2");
    rc = SQLSetPos(hstmt,1,SQL_ADD,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);

    printMessage("rows affected:%d\n",nlen);

    strcpy((char *)szData , "insert-new3");
    rc = SQLSetPos(hstmt,0,SQL_ADD,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);

    printMessage("rows affected:%d\n",nlen);

    strcpy((char *)szData , "insert-new4");
    rc = SQLSetPos(hstmt,10,SQL_ADD,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);

    printMessage("rows affected:%d\n",nlen);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from tmysql_setpos_add");
    mystmt(hstmt,rc);

    myassert(6 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_sqlspecialcols)
{
    SQLRETURN rc;

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

  return OK;
}


DECLARE_TEST(t_sqltables)
{
    SQLRETURN r;

    r  = SQLTables(hstmt,NULL,0,NULL,0,NULL,0,NULL,0);
    mystmt(hstmt,r);

    myresult(hstmt);

    r = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,r);

    r  = SQLTables(hstmt,NULL,0,NULL,0,NULL,0,"'system table'",SQL_NTS);
    mystmt(hstmt,r);

    if (driver_min_version(hdbc,"03.51.07",8))
        myassert(myresult(hstmt) != 0);
    else
        myassert(0 == myresult(hstmt));

    r = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,r);

    r  = SQLTables(hstmt,NULL,0,NULL,0,NULL,0,"TABLE",SQL_NTS);
    mystmt(hstmt,r);

    myresult(hstmt);

    r = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,r);

    r  = SQLTables(hstmt,"TEST",SQL_NTS,"TEST",SQL_NTS,NULL,0,"TABLE",SQL_NTS);
    mystmt(hstmt,r);

    myresult(hstmt);

    r = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,r);

    r = SQLTables(hstmt,"%",SQL_NTS,NULL,0,NULL,0,NULL,0);
    mystmt(hstmt,r);

    myresult(hstmt);

    r = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,r);

    r = SQLTables(hstmt,NULL,0,"%",SQL_NTS,NULL,0,NULL,0);
    mystmt(hstmt,r);

    myresult(hstmt);

    r = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,r);

    r = SQLTables(hstmt,NULL,0,NULL,0,NULL,0,"%",SQL_NTS);
    mystmt(hstmt,r);

    if (driver_min_version(hdbc,"03.51.07",8))
        myassert( 2 == myresult(hstmt));
    else
        myassert( 1 == myresult(hstmt));


    r = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,r);

  return OK;
}


DECLARE_TEST(tmysql_bindcol)
{
    SQLRETURN rc;
    long      nodata;
    long      nlen, nidata = 200;
    char      szodata[20],szidata[20]="MySQL";

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
                          0,0,szidata,5,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,2,SQL_PARAM_INPUT, SQL_C_LONG,SQL_INTEGER,
                          0,0,&nidata,20,NULL);
    mystmt(hstmt,rc);

    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    printMessage(" outdata: %d, %s(%d)\n", nodata,szodata,nlen);
    my_assert(nodata == 200);

    rc = SQLFetch(hstmt);

    my_assert(rc == SQL_NO_DATA_FOUND);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"drop table tmysql_bindcol");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

  return OK;
}


DECLARE_TEST(tmysql_pcbvalue)
{
    SQLRETURN  rc;
    long       nodata;
    SQLLEN     nlen, slen,tlen;
    char       szdata[20],sztdata[100];
    SQLUSMALLINT rgfRowStatus[20];

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
#if 0
    SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER) SQL_CONCUR_ROWVER, 0);
    SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_KEYSET_DRIVEN, 0);
#endif

    rc = SQLExecDirect(hstmt,"select * from tmysql_pcbvalue",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nodata,0,&nlen);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szdata,20,&slen);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,3,SQL_C_CHAR,sztdata,101,&tlen);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_FIRST,1,0,rgfRowStatus);
    mystmt(hstmt,rc);
    printMessage("row1: %d(%d), %s(%d),%s(%d)\n", nodata,nlen,szdata,slen,sztdata,tlen);

    strcpy(szdata,"updated-one");

    rc = SQLSetPos(hstmt,1,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,0,0);
    mystmt(hstmt,rc);

    printMessage("row2: %d(%d), %s(%d),%s(%d)\n", nodata,nlen,szdata,slen,sztdata,tlen);

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
    printMessage("updated data:%s(%d)",szdata,slen);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);

    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

  return OK;
}


DECLARE_TEST(tmysql_bindparam)
{
    SQLRETURN rc;
    long      nodata, nidata= 200;
    SQLLEN    nlen;
    char      szodata[20],szidata[20]="MySQL";
    short     pccol;

    tmysql_exec(hstmt,"drop table tmysql_bindparam");

    rc = tmysql_exec(hstmt,"create table tmysql_bindparam(col1 int primary key, col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_bindparam values(100,'venu')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_bindparam values(200,'MySQL')");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_prepare(hstmt,"select * from tmysql_bindparam where col2 = ? AND col1 = ?");
    mystmt(hstmt,rc);

    rc = SQLNumResultCols(hstmt,&pccol);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nodata,0,&nlen);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szodata,200,&nlen);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT, SQL_C_CHAR,SQL_VARCHAR,
                          0,0,szidata,5,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,2,SQL_PARAM_INPUT, SQL_C_LONG,SQL_INTEGER,
                          0,0,&nidata,20,NULL);
    mystmt(hstmt,rc);

    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    printMessage(" outdata: %d, %s(%d)\n", nodata,szodata,nlen);
    my_assert(nodata == 200);

    rc = SQLFetch(hstmt);

    my_assert(rc == SQL_NO_DATA_FOUND);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"drop table tmysql_bindparam");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

  return OK;
}


DECLARE_TEST(tmysql_fix)
{
    SQLRETURN rc;

    /* dump based */
    printMessage("table structure for 'shop'..\n");
    tmysql_exec(hstmt,"drop table if exists shop");

    rc = tmysql_exec(hstmt,"CREATE TABLE shop (\
                valor varchar(20) NOT NULL default ''\
                ) TYPE=MyISAM;");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"LOCK TABLES shop WRITE");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"UNLOCK TABLES");
    mystmt(hstmt,rc);

    printMessage("table structure for 'sqlerr'..\n");
    tmysql_exec(hstmt,"drop table if exists sqlerr");

    rc = tmysql_exec(hstmt,"CREATE TABLE sqlerr (\
                  td date NOT NULL default '0000-00-00',\
                  node varchar(8) NOT NULL default '',\
                  tag varchar(10) NOT NULL default '',\
                  sqlname varchar(8) default NULL,\
                  fix_err varchar(100) default NULL,\
                  sql_err varchar(255) default NULL,\
                  prog_err varchar(100) default NULL\
                ) TYPE=MyISAM");
    mystmt(hstmt,rc);

    printMessage("dump data for table 'sqlerr'..\n");
    rc = tmysql_exec(hstmt,"LOCK TABLES sqlerr WRITE");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"INSERT INTO sqlerr VALUES\
                  ('0000-00-00','0','0','0','0','0','0'),\
                  ('2001-08-29','FIX','SQLT2','ins1',\
                  NULL,NULL, 'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2',\
                  'ins1',NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2',\
                  'ins1',NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2','ins1',\
                  NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2',\
                  'ins1',NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2',\
                  'ins1',NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2',\
                  'ins1',NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000!-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2',\
                  'ins1',NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2',\
                  'ins1',NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.')");

    rc = tmysql_exec(hstmt,"UNLOCK TABLES");
    mystmt(hstmt,rc);

    printMessage("table structure for 'sqllib'..\n");
    tmysql_exec(hstmt,"drop table if exists sqllib");

    rc = tmysql_exec(hstmt,"CREATE TABLE sqllib (\
                  sqlname varchar(8) NOT NULL default '',\
                  sqlcmd varchar(150) NOT NULL default '',\
                  PRIMARY KEY  (sqlname)\
                ) TYPE=MyISAM");
    mystmt(hstmt,rc);

    printMessage("dump data for 'sqllib'..\n");
    rc = tmysql_exec(hstmt,"LOCK TABLES sqllib WRITE");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"INSERT INTO sqllib VALUES ('ins1','insert into shop (valor) values(?)')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"UNLOCK TABLES");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    /* trace based */
    {
        SQLSMALLINT pcpar,pccol,pfSqlType,pibScale,pfNullable;
        SQLSMALLINT index;
        SQLCHAR     td[30]="20010830163225";
        SQLCHAR     node[30]="FIX";
        SQLCHAR     tag[30]="SQLT2";
        SQLCHAR     sqlname[30]="ins1";
        SQLCHAR     sqlerr[30]="error";
        SQLCHAR     fixerr[30]= "fixerr";
        SQLCHAR     progerr[30]="progerr";
        SQLULEN     pcbParamDef;

        SQLFreeStmt(hstmt,SQL_CLOSE);
        rc = SQLPrepare(hstmt,"insert into sqlerr (TD, NODE, TAG, SQLNAME, SQL_ERR, FIX_ERR, PROG_ERR)\
                         values (?, ?, ?, ?, ?, ?, ?)",200);
        mystmt(hstmt,rc);

        rc = SQLNumParams(hstmt,&pcpar);
        mystmt(hstmt,rc);

        rc = SQLNumResultCols(hstmt,&pccol);
        mystmt(hstmt,rc);

        for (index=1; index <= pcpar; index++)
        {
            rc = SQLDescribeParam(hstmt,index,&pfSqlType,&pcbParamDef,&pibScale,&pfNullable);
            mystmt(hstmt,rc);

            printMessage("descparam[%d]:%d,%d,%d,%d\n",index,pfSqlType,pcbParamDef,pibScale,pfNullable);
        }

        rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,11,12,0,0,td,100,0);
        mystmt(hstmt,rc);

        rc = SQLBindParameter(hstmt,2,SQL_PARAM_INPUT,1,12,0,0,node,100,0);
        mystmt(hstmt,rc);

        rc = SQLBindParameter(hstmt,3,SQL_PARAM_INPUT,1,12,0,0,tag,100,0);
        mystmt(hstmt,rc);
        rc = SQLBindParameter(hstmt,4,SQL_PARAM_INPUT,1,12,0,0,sqlname,100,0);
        mystmt(hstmt,rc);
        rc = SQLBindParameter(hstmt,6,SQL_PARAM_INPUT,1,12,0,0,sqlerr,0,0);
        mystmt(hstmt,rc);
        rc = SQLBindParameter(hstmt,7,SQL_PARAM_INPUT,1,12,0,0,fixerr,0,0);
        mystmt(hstmt,rc);
        rc = SQLBindParameter(hstmt,8,SQL_PARAM_INPUT,1,12,0,0,progerr,0,0);
        mystmt(hstmt,rc);

        rc = SQLExecute(hstmt);
        mystmt(hstmt,rc);
    }

  return OK;
}


DECLARE_TEST(tmysql_pos_delete)
{
    SQLRETURN rc;
    SQLHSTMT hstmt1;
    SQLROWSETSIZE pcrow;
    SQLUSMALLINT rgfRowStatus;

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

  return OK;
}


DECLARE_TEST(tmysql_pos_update)
{
    SQLRETURN rc;
    SQLHSTMT hstmt1;
    SQLROWSETSIZE pcrow;
    SQLUSMALLINT rgfRowStatus;

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

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
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

  return OK;
}


DECLARE_TEST(tmysql_pos_update_ex)
{
    SQLRETURN rc;
    SQLHSTMT hstmt1;
    SQLROWSETSIZE pcrow;
    SQLUSMALLINT rgfRowStatus;
    SQLCHAR cursor[30],sql[100],data[]="updated";

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

    SQLFreeStmt(hstmt1,SQL_RESET_PARAMS);
    rc = SQLFreeStmt(hstmt1,SQL_DROP);
    mystmt(hstmt1,rc);

  return OK;
}


DECLARE_TEST(tmysql_pos_update_ex1)
{
    SQLRETURN rc;
    SQLHSTMT hstmt1;
    SQLROWSETSIZE pcrow;
    SQLUSMALLINT rgfRowStatus;
    SQLCHAR cursor[30],sql[100],data[]="updated";


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

    SQLFreeStmt(hstmt1,SQL_UNBIND);
    SQLFreeStmt(hstmt1,SQL_RESET_PARAMS);
    rc = SQLFreeStmt(hstmt1,SQL_DROP);
    mystmt(hstmt1,rc);

  return OK;
}


DECLARE_TEST(tmysql_pos_update_ex2)
{
    SQLRETURN rc;
    SQLHSTMT hstmt1;
    SQLROWSETSIZE pcrow;
    SQLUSMALLINT rgfRowStatus;
    SQLCHAR cursor[30],sql[100],data[]="updated";

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

    SQLFreeStmt(hstmt1,SQL_UNBIND);
    SQLFreeStmt(hstmt1,SQL_RESET_PARAMS);
    rc = SQLFreeStmt(hstmt1,SQL_DROP);
    mystmt(hstmt1,rc);

  return OK;
}

DECLARE_TEST(tmysql_pos_update_ex3)
{
    SQLRETURN rc;
    SQLHSTMT hstmt1;
    SQLROWSETSIZE pcrow;
    SQLCHAR cursor[30],sql[100];

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

    SQLFreeStmt(hstmt1,SQL_UNBIND);
    SQLFreeStmt(hstmt1,SQL_RESET_PARAMS);
    rc = SQLFreeStmt(hstmt1,SQL_DROP);
    mystmt(hstmt1,rc);

  return OK;
}


DECLARE_TEST(tmysql_pos_update_ex4)
{
    SQLRETURN rc;
    SQLHSTMT hstmt1;
    SQLROWSETSIZE pcrow;
    SQLCHAR cursor[30],sql[100],data[]="venu";

    rc = SQLAllocStmt(hdbc,&hstmt1);
    mycon(hdbc,rc);

    tmysql_exec(hstmt,"drop table t_pos_updex4");
    rc = tmysql_exec(hstmt,"create table t_pos_updex4( name varchar(20) not null, surname varchar(20) not null,  addresss varchar(50), primary key(surname ))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_pos_updex4(name, surname) values('Bill', 'Gates')");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_pos_updex4");
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,1,&pcrow,NULL);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,1,SQL_C_CHAR,data,20,NULL);
    mystmt(hstmt,rc);

    rc = SQLGetCursorName(hstmt,cursor,30,NULL);
    mystmt(hstmt,rc);

    sprintf(sql,"UPDATE t_pos_updex4 SET name = 'venu' WHERE CURRENT OF %s",cursor);

    rc = SQLExecDirect(hstmt1,sql,SQL_NTS);
    mystmt(hstmt1,rc);

    /*rc = SQLSetPos(hstmt,1,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
      mystmt(hstmt,rc);*/

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    rc = SQLFreeStmt(hstmt1,SQL_CLOSE);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select name from t_pos_updex4");
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    {
        SQLCHAR szData[20];
        my_assert(!strcmp("venu",my_fetch_str(hstmt,szData,1)));
    }

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt1,SQL_UNBIND);
    SQLFreeStmt(hstmt1,SQL_RESET_PARAMS);
    rc = SQLFreeStmt(hstmt1,SQL_DROP);
    mystmt(hstmt1,rc);

  return OK;
}


DECLARE_TEST(tmysql_pos_dyncursor)
{
    SQLRETURN rc;
    SQLHSTMT hstmt1;
    SQLROWSETSIZE pcrow;
    SQLUSMALLINT rgfRowStatus;
    SQLCHAR  szCursor[20],buff[100];

    rc = SQLAllocStmt(hdbc,&hstmt1);
    mycon(hdbc,rc);

    tmysql_exec(hstmt,"drop table tmysql_pos_dyncursor");
    rc = tmysql_exec(hstmt,"create table tmysql_pos_dyncursor(col1 int , col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_pos_dyncursor values(100,'venu')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_pos_dyncursor values(200,'MySQL')");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLSetCursorName(hstmt,"venu_cur",SQL_NTS);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from tmysql_pos_dyncursor");
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,NULL);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLGetCursorName(hstmt,szCursor,20,NULL);
    mystmt(hstmt,rc);

    sprintf(buff,"UPDATE tmysql_pos_dyncursor SET col1= 999, col2 = 'update' WHERE CURRENT OF %s",szCursor);

    rc = SQLExecDirect(hstmt1,buff,SQL_NTS);
    mystmt(hstmt1,rc);

    SQLNumResultCols(hstmt1,&rgfRowStatus);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    rc = SQLFreeStmt(hstmt1,SQL_CLOSE);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from tmysql_pos_dyncursor");
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

  return OK;
}


DECLARE_TEST(tmysql_mtab_setpos_del)
{
    SQLRETURN rc;
    long nData= 500;
    SQLLEN nlen;
    SQLCHAR szData[255]={0};
    SQLROWSETSIZE pcrow;

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

    printMessage(" pcrow:%d\n",pcrow);

    printMessage(" row1:%d,%s\n",nData,szData);

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    /* not yet supported..*/
    rc = SQLSetPos(hstmt,2,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt_r(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(tmysql_showkeys)
{
    SQLRETURN rc;

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

  return OK;
}


DECLARE_TEST(tmysql_setpos_pkdel)
{
    SQLRETURN rc;
    long nData= 500;
    SQLLEN nlen;
    SQLCHAR szData[255]={0};
    SQLROWSETSIZE pcrow;

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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,4,&pcrow,NULL);
    mystmt(hstmt,rc);

    printMessage(" pcrow:%d\n",pcrow);

    printMessage(" row1:%d,%s\n",nData,szData);

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt,1,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);

    printMessage(" rows affected:%d\n",nlen);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from tmysql_setpos1");
    mystmt(hstmt,rc);

    my_assert( 3 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_alias_setpos_pkdel)
{
    SQLRETURN rc;
    long nData= 500;
    SQLLEN nlen;
    SQLCHAR szData[255]={0};
    SQLROWSETSIZE pcrow;

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

    printMessage(" pcrow:%d\n",pcrow);

    printMessage(" row1:%d,%s\n",nData,szData);

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt,1,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);

    printMessage(" rows affected:%d\n",nlen);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_alias_setpos_del");
    mystmt(hstmt,rc);

    my_assert( 3 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_alias_setpos_del)
{
    SQLRETURN rc;
    long nData= 500;
    SQLLEN nlen;
    SQLCHAR szData[255]={0};
    SQLROWSETSIZE pcrow;
    SQLUSMALLINT rgfRowStatus;

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

    rc = SQLSetCursorName(hstmt,"venu",SQL_NTS);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select col1 as id, col2 as name from t_alias_setpos_del");
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,100,&nlen);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,1,&pcrow,&rgfRowStatus);
    mystmt(hstmt,rc);

    printMessage(" pcrow:%d,rgfRowStatus:%d\n",pcrow,rgfRowStatus);

    printMessage(" row1:%d,%s\n",nData,szData);

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt,1,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);

    printMessage(" rows affected:%d\n",nlen);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_alias_setpos_del");
    mystmt(hstmt,rc);

    my_assert( 3 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}

DECLARE_TEST(tmysql_setpos_pkdel1)
{
    SQLRETURN rc;
    long nData= 500;
    SQLLEN nlen;
    SQLCHAR szData[255]={0};
    SQLROWSETSIZE pcrow;

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

    printMessage(" pcrow:%d\n",pcrow);

    printMessage(" row1:%d,%s\n",nData,szData);

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt,1,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);

    printMessage(" rows affected:%d\n",nlen);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from tmysql_setpos1");
    mystmt(hstmt,rc);

    my_assert( 3 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(tmysql_setpos_pkdel2)
{
    SQLRETURN rc;
    long nData= 500;
    SQLLEN nlen;
    SQLCHAR szData[255]={0};
    SQLROWSETSIZE pcrow;

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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,4,&pcrow,NULL);
    mystmt(hstmt,rc);
    printMessage(" pcrow:%d\n",pcrow);

    printMessage(" row1:%d,%s\n",nData,szData);

    rc = SQLSetPos(hstmt,1,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);

    printMessage(" rows affected:%d\n",nlen);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from tmysql_setpos1");
    mystmt(hstmt,rc);

    my_assert( 3 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(tmysql_rowstatus)
{
    SQLRETURN rc;
    SQLHSTMT hstmt1;
    SQLROWSETSIZE pcrow[4];
    SQLUSMALLINT rgfRowStatus[6];
    long nData = 555;
    SQLCHAR szData[255] = "setpos-update";

    rc = SQLAllocStmt(hdbc,&hstmt1);
    mycon(hdbc,rc);

    rc = SQLSetCursorName(hstmt,"venu_cur",SQL_NTS);
    mystmt(hstmt,rc);

    tmysql_exec(hstmt,"drop table tmysql_rowstatus");
    rc = tmysql_exec(hstmt,"create table tmysql_rowstatus(col1 int , col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_rowstatus values(100,'venu')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_rowstatus values(200,'MySQL')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_rowstatus values(300,'MySQL3')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_rowstatus values(400,'MySQL3')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_rowstatus values(500,'MySQL3')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_rowstatus values(600,'MySQL3')");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE  ,(SQLPOINTER)1 , 0);
    mystmt(hstmt, rc);

    rc = tmysql_exec(hstmt,"select * from tmysql_rowstatus");
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,0,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,sizeof(szData),NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,pcrow,(SQLUSMALLINT *)&rgfRowStatus);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,pcrow,(SQLUSMALLINT *)&rgfRowStatus);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt1,"UPDATE tmysql_rowstatus SET col1= 999, col2 = 'pos-update' WHERE CURRENT OF venu_cur",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_LAST,1,NULL,NULL);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt,1,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    printMessage("\nrgfRowStatus[0]:%d",rgfRowStatus[0]);

    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLFreeStmt(hstmt1,SQL_DROP);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);


    rc = tmysql_exec(hstmt,"select * from tmysql_rowstatus");
    mystmt(hstmt,rc);

    myassert(5 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_refresh)
{
    SQLRETURN rc;

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,SQL_AUTOCOMMIT_OFF);
    mycon(hdbc,rc);

    tmysql_exec(hstmt,"drop table if exists t_refresh");

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_refresh(col1 int ,col2 varchar(30)) TYPE = BDB");
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

  return OK;
}


DECLARE_TEST(tmysql_setpos_pkdel3)
{
    SQLRETURN rc;
    long nData= 500;
    SQLLEN nlen;
    SQLCHAR szData[255]={0};
    SQLROWSETSIZE pcrow;

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

    printMessage(" pcrow:%d\n",pcrow);

    printMessage(" row1:%d,%s\n",nData,szData);

    rc = SQLSetPos(hstmt,1,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);

    printMessage(" rows affected:%d\n",nlen);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from tmysql_setpos1");
    mystmt(hstmt,rc);

    my_assert( 3 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_mul_pkdel)
{
    SQLRETURN rc;
    long nData= 500;
    SQLLEN nlen;
    SQLCHAR szData[255]={0};
    SQLROWSETSIZE pcrow;

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
    printMessage(" row1:%d,%s\n",nData,szData);

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt,1,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);

    printMessage(" rows affected:%d\n",nlen);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_mul_pkdel");
    mystmt(hstmt,rc);

    my_assert( 3 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_mul_pkdel1)
{
    SQLRETURN rc;
    long nData= 500;
    SQLLEN nlen;
    SQLCHAR szData[255]={0};
    SQLROWSETSIZE pcrow;

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

    printMessage(" row1:%d,%s\n",nData,szData);

    rc = SQLSetPos(hstmt,1,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);

    printMessage(" rows affected:%d\n",nlen);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_mul_pkdel");
    mystmt(hstmt,rc);

    my_assert( 3 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_max_select)
{
    SQLRETURN rc;
    SQLCHAR szData[255]={0};
    SQLINTEGER i;

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

    for (i = 1; i <= 1000; i++)
    {
        sprintf((char *)szData,"MySQL%d",i);

        rc = SQLExecute(hstmt);
        mystmt(hstmt,rc);
    }

    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from t_max_select");
    mystmt(hstmt,rc);

    my_assert( 1000 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_tran)
{
    SQLRETURN rc;

    if (!server_supports_trans(hdbc))
        return SKIP;

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

  return OK;
}


DECLARE_TEST(t_tran_ddl)
{
    SQLRETURN rc;
    SQLSMALLINT rgbValue,len;

    if (!server_supports_trans(hdbc))
        return SKIP;

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,SQL_AUTOCOMMIT_OFF);
    mycon(hdbc,rc);

    rc = SQLGetInfo(hdbc,SQL_TXN_CAPABLE,&rgbValue,0,&len);
    mycon(hdbc,rc);
    my_assert(rgbValue == SQL_TC_DDL_COMMIT);
    my_assert(len == 2);

    tmysql_exec(hstmt,"drop table if exists t_tran1");

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_tran1(col1 int ,col2 varchar(30)) TYPE = BDB");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_ROLLBACK);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"drop table if exists t_tran1");
    mystmt(hstmt,rc);

    rc = SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,SQL_AUTOCOMMIT_ON);
    mycon(hdbc,rc);

  return OK;
}


DECLARE_TEST(t_max_con)
{
    SQLRETURN rc;
    long i;
    SQLHDBC hdbc1;

    for (i= 0; i < 200; i++)
    {
        rc = SQLAllocConnect(henv, &hdbc1);
        myenv(henv,rc);

        rc = SQLConnect(hdbc1, mydsn, SQL_NTS, myuid, SQL_NTS,  mypwd, SQL_NTS);
        mycon(hdbc1,rc);

        rc = SQLDisconnect(hdbc1);
        mycon(hdbc1,rc);

        rc = SQLFreeConnect(hdbc1);
        mycon(hdbc1,rc);
    }

  return OK;
}


DECLARE_TEST(t_enumset)
{
    SQLRETURN rc;
    SQLCHAR szEnum[40]="MYSQL_E1";
    SQLCHAR szSet[40]="THREE,ONE,TWO";

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

  return OK;
}


DECLARE_TEST(t_desccol)
{
    SQLRETURN rc;
    SQLCHAR colname[20];
    SQLSMALLINT collen,datatype,decptr,nullable;
    SQLULEN colsize;

    tmysql_exec(hstmt,"drop table t_desccol");

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_desccol(col1 int, col2 varchar(10), col3 text)");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into t_desccol values(10,'venu','mysql')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from t_desccol");
    mystmt(hstmt,rc);

    rc = SQLDescribeCol(hstmt,1,(char *)colname,20,&collen,&datatype,&colsize,&decptr,&nullable);
    mystmt(hstmt,rc);
    printMessage("1: %s,%d,%d,%d,%d,%d\n",colname,collen,datatype,colsize,decptr,nullable);;

    rc = SQLDescribeCol(hstmt,2,(char *)colname,20,&collen,&datatype,&colsize,&decptr,&nullable);
    mystmt(hstmt,rc);
    printMessage("2: %s,%d,%d,%d,%d,%d\n",colname,collen,datatype,colsize,decptr,nullable);;

    rc = SQLDescribeCol(hstmt,3,(char *)colname,20,&collen,&datatype,&colsize,&decptr,&nullable);
    mystmt(hstmt,rc);
    printMessage("3: %s,%d,%d,%d,%d,%d\n",colname,collen,datatype,colsize,decptr,nullable);;

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


void desccol(SQLHSTMT hstmt, SQLCHAR    *cname,  SQLSMALLINT clen,
             SQLSMALLINT sqltype,SQLUINTEGER size,
             SQLSMALLINT scale,SQLSMALLINT isNull)
{
    SQLRETURN   rc =0;
    SQLCHAR     lcname[254];
    SQLSMALLINT lclen;
    SQLSMALLINT lsqltype;
    SQLULEN     lsize;
    SQLSMALLINT lscale;
    SQLSMALLINT lisNull;
    SQLCHAR     select[255];

    SQLFreeStmt(hstmt,SQL_CLOSE);

    sprintf(select,"select %s from t_desccolext",cname);
    printMessage("\n%s",select);

    rc = SQLExecDirect(hstmt,select,SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLDescribeCol( hstmt,1,lcname,  sizeof(lcname),&lclen,
                         &lsqltype,&lsize,&lscale,&lisNull);
    mystmt(hstmt,rc);

    printMessage("\n name	: %s (%d)",lcname,lclen);
    printMessage("\n sqltype: %d, size: %d, scale: %d, null: %d\n",lsqltype,lsize,lscale,lisNull);

    myassert(strcmp(lcname,cname)==0);
    myassert(lclen == clen);
    myassert(lsqltype == sqltype);
    myassert(lsize == size);
    myassert(lscale == scale);
    myassert(lisNull == isNull);

    SQLFreeStmt(hstmt,SQL_CLOSE);
}


DECLARE_TEST(t_desccolext)
{
    SQLRETURN rc;
    SQLCHAR     *sql;

    tmysql_exec(hstmt,"drop table t_desccolext");

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    sql= "create table t_desccolext\
      ( t1 tinyint,\
        t2 tinyint(10),\
        t3 tinyint unsigned,\
        s1 smallint,\
        s2 smallint(10),\
        s3 smallint unsigned,\
        m1 mediumint,\
        m2 mediumint(10),\
        m3 mediumint unsigned,\
        i1 int,\
        i2 int(10) not null,\
        i3 int unsigned,\
        i4 int zerofill,\
        b1 bigint,\
        b2 bigint(10),\
        b3 bigint unsigned,\
        f1 float,\
        f2 float(10),\
        f3 float(24) zerofill,\
        f4 float(10,4),\
        d1 double,\
        d2 double(30,3),\
        d3 double precision,\
        d4 double precision(30,3),\
        r1 real,\
        r2 real(30,3),\
        dc1 decimal,\
        dc2 decimal(10),\
        dc3 decimal(10,3),\
        n1 numeric,\
        n2 numeric(10,3),\
        dt date,\
        dtime datetime,\
        ts1 timestamp(8),\
        ts2 timestamp(14),\
        ti  time,\
        yr1 year,\
        yr2 year(2),\
        yr3 year(4),\
        c1 char(10),\
        c2 char(10) binary,\
        c3 national char(10),\
        v1 varchar(10),\
        v2 varchar(10) binary,\
        v3 national varchar(10),\
        bl1 tinyblob,\
        bl2 blob,\
        bl3 mediumblob,\
        bl4 longblob,\
        txt1 tinytext,\
        txt2 text,\
        txt3 mediumtext,\
        txt4 longtext,\
        en enum('v1','v2'),\
        st set('1','2','3'))";

    rc = tmysql_exec(hstmt,sql);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    desccol(hstmt,"t1",2,SQL_TINYINT,4,0,SQL_NULLABLE);
    desccol(hstmt,"t2",2,SQL_TINYINT,10,0,SQL_NULLABLE);
    desccol(hstmt,"t3",2,SQL_TINYINT,3,0,SQL_NULLABLE);

    desccol(hstmt,"s1",2,SQL_SMALLINT,6,0,SQL_NULLABLE);
    desccol(hstmt,"s2",2,SQL_SMALLINT,10,0,SQL_NULLABLE);
    desccol(hstmt,"s3",2,SQL_SMALLINT,5,0,SQL_NULLABLE);

    desccol(hstmt,"m1",2,SQL_INTEGER,9,0,SQL_NULLABLE);
    desccol(hstmt,"m2",2,SQL_INTEGER,10,0,SQL_NULLABLE);
    desccol(hstmt,"m3",2,SQL_INTEGER,8,0,SQL_NULLABLE);

    desccol(hstmt,"i1",2,SQL_INTEGER,11,0,SQL_NULLABLE);
    desccol(hstmt,"i2",2,SQL_INTEGER,10,0,SQL_NO_NULLS);
    desccol(hstmt,"i3",2,SQL_INTEGER,10,0,SQL_NULLABLE);
    desccol(hstmt,"i4",2,SQL_INTEGER,10,0,SQL_NULLABLE);

    desccol(hstmt,"b1",2,SQL_BIGINT,19,0,SQL_NULLABLE);
    desccol(hstmt,"b2",2,SQL_BIGINT,19,0,SQL_NULLABLE);
    desccol(hstmt,"b3",2,SQL_BIGINT,20,0,SQL_NULLABLE);

    desccol(hstmt,"f1",2,SQL_REAL,12,31,SQL_NULLABLE);
    desccol(hstmt,"f2",2,SQL_REAL,12,31,SQL_NULLABLE);
    desccol(hstmt,"f3",2,SQL_REAL,12,31,SQL_NULLABLE);
    desccol(hstmt,"f4",2,SQL_REAL,10,4,SQL_NULLABLE);

    desccol(hstmt,"d1",2,SQL_DOUBLE,22,31,SQL_NULLABLE);
    desccol(hstmt,"d2",2,SQL_DOUBLE,30,3,SQL_NULLABLE);
    desccol(hstmt,"d3",2,SQL_DOUBLE,22,31,SQL_NULLABLE);
    desccol(hstmt,"d4",2,SQL_DOUBLE,30,3,SQL_NULLABLE);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


void colattr(SQLHSTMT hstmt, SQLUSMALLINT cno,
             SQLUSMALLINT attribute, SQLCHAR *sptr,
             SQLSMALLINT slen, SQLINTEGER nptr)
{
    SQLRETURN   rc =0;
    SQLCHAR     lsptr[40];
    SQLLEN      lnptr;
    SQLSMALLINT lslen;

    SQLFreeStmt(hstmt,SQL_CLOSE);
    rc = SQLExecDirect(hstmt,"select * from t_colattr",SQL_NTS);
    mystmt(hstmt,rc);

    printMessage("col %d, attribute %d\t: ",cno, attribute);
    rc = SQLColAttributes(hstmt,cno,attribute,lsptr,100,&lslen,&lnptr);
    mystmt(hstmt,rc);

    if (sptr)
    {
        printMessage("%s(%ld)\n",lsptr,lslen);
        myassert(!strcmp(lsptr,sptr)==0);
        myassert(lslen == slen);
    }
    else
    {
        printMessage("%ld(%d)\n",lnptr,lslen);
        myassert(lnptr == nptr);
    }

    SQLFreeStmt(hstmt,SQL_CLOSE);
}

DECLARE_TEST(t_colattributes)
{
    SQLRETURN rc;
    SQLCHAR     *sql;

    tmysql_exec(hstmt,"drop table t_colattr");

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    sql= "create table t_colattr\
      ( t1 tinyint not null auto_increment primary key,\
        t2 tinyint(10),\
        t3 tinyint unsigned,\
        s1 smallint,\
        s2 smallint(10),\
        s3 smallint unsigned,\
        m1 mediumint,\
        m2 mediumint(10),\
        m3 mediumint unsigned,\
        i1 int,\
        i2 int(10) not null,\
        i3 int unsigned,\
        i4 int zerofill,\
        b1 bigint,\
        b2 bigint(10),\
        b3 bigint unsigned,\
        f1 float,\
        f2 float(10),\
        f3 float(24) zerofill,\
        f4 float(10,4),\
        d1 double,\
        d2 double(30,3),\
        d3 double precision,\
        d4 double precision(30,3),\
        r1 real,\
        r2 real(30,3),\
        dc1 decimal,\
        dc2 decimal(10),\
        dc3 decimal(10,3),\
        n1 numeric,\
        n2 numeric(10,3),\
        dt date,\
        dtime datetime,\
        ts1 timestamp(8),\
        ts2 timestamp(14),\
        ti  time,\
        yr1 year,\
        yr2 year(2),\
        yr3 year(4),\
        c1 char(10),\
        c2 char(10) binary,\
        c3 national char(10),\
        v1 varchar(10),\
        v2 varchar(10) binary,\
        v3 national varchar(10),\
        bl1 tinyblob,\
        bl2 blob,\
        bl3 mediumblob,\
        bl4 longblob,\
        txt1 tinytext,\
        txt2 text,\
        txt3 mediumtext,\
        txt4 longtext,\
        en enum('v1','v2'),\
        st set('1','2','3'))";

    rc = tmysql_exec(hstmt,sql);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    colattr(hstmt,1,SQL_COLUMN_COUNT,NULL,0,55);
    colattr(hstmt,1,SQL_COLUMN_AUTO_INCREMENT,NULL,0,SQL_TRUE);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_bigint)
{
    SQLRETURN rc;
    SQLCHAR id[20]="999";
    SQLLEN nlen;

    tmysql_exec(hstmt,"drop table t_bigint");

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_bigint(id int(20) NOT NULL auto_increment,name varchar(20), primary key(id))");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    /* TIMESTAMP TO DATE, TIME and TS CONVERSION */
    rc = tmysql_prepare(hstmt,"insert into t_bigint values(?,'venuxyz')");
    mystmt(hstmt,rc);

    nlen = 4;
    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_LONG,
                          SQL_BIGINT,0,0,&id,sizeof(id),&nlen);
    mystmt(hstmt,rc);

    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_bigint values(10,'mysql1')");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_bigint values(20,'mysql2')");
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLSpecialColumns(hstmt,SQL_ROWVER,NULL,SQL_NTS,NULL,SQL_NTS,
                           "t_bigint",SQL_NTS,SQL_SCOPE_TRANSACTION,SQL_NULLABLE);

    mycon(hdbc,rc);

    my_assert( 0 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLColumns(hstmt,NULL,SQL_NTS,NULL,SQL_NTS,"t_bigint",SQL_NTS,NULL,SQL_NTS);

    mycon(hdbc,rc);

    my_assert( 2 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLStatistics(hstmt,NULL,SQL_NTS,NULL,SQL_NTS,"t_bigint",SQL_NTS,SQL_INDEX_ALL,SQL_QUICK);

    mycon(hdbc,rc);

    my_assert( 1 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

#if CATALOG_FUNCTIONS_FIXED
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
#endif

    rc = tmysql_exec(hstmt,"select * from t_bigint");
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt,1,SQL_C_DEFAULT,&id,sizeof(id),&nlen);
    mystmt(hstmt,rc);

    printMessage("\n id:%s,nlen:%d,%d\n",id,nlen,sizeof(SQL_BIGINT));

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_setpos_upd_bug1)
{
    SQLRETURN rc;
    long id;
    SQLLEN len,id_len,f_len,l_len,ts_len;
    SQLCHAR fname[21],lname[21],ts[17],szTable[256];
    SQLUSMALLINT pccol;

    tmysql_exec(hstmt,"drop table if exists t_setpos_upd_bug1");
    rc = tmysql_exec(hstmt,"create table t_setpos_upd_bug1(id int(11) NOT NULL auto_increment,\
                                                           fname char(20) NOT NULL default '',\
                                                           lname char(20) NOT NULL default '',\
                                                           last_modi timestamp(14),\
                                                           PRIMARY KEY(id)) TYPE=MyISAM");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_setpos_upd_bug1(fname,lname) values('joshua','kugler')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into t_setpos_upd_bug1(fname,lname) values('monty','widenius')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into t_setpos_upd_bug1(fname,lname) values('mr.','venu')");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_setpos_upd_bug1 order by id asc");
    mystmt(hstmt,rc);

    rc = SQLNumResultCols(hstmt,(SQLSMALLINT *)&pccol);
    mystmt(hstmt,rc);

    printMessage(" total columns:%d\n",pccol);

    rc = SQLBindCol(hstmt,1,SQL_C_SLONG,&id,4,&id_len);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,fname,6,&f_len);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,3,SQL_C_CHAR,lname,20,&l_len);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,4,SQL_C_TIMESTAMP,ts,21,&ts_len);
    mystmt(hstmt,rc);

    rc = SQLColAttributes(hstmt,1,SQL_COLUMN_TABLE_NAME,szTable,256,NULL,NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_FIRST,0,NULL,NULL);
    mystmt(hstmt,rc);

    rc = SQLSetStmtOption(hstmt,SQL_QUERY_TIMEOUT,30);
    mystmt(hstmt,rc);

    strcpy((char *)fname , "updated");
    strcpy((char *)lname , "updated01234567890");

    rc = SQLSetPos(hstmt,1,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&len);
    mystmt(hstmt,rc);

    printMessage(" rows affected:%d\n",len);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_setpos_upd_bug1");
    mystmt(hstmt,rc);

    myresult(hstmt);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"DELETE FROM t_setpos_upd_bug1 WHERE fname = 'update'",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&len);
    mystmt(hstmt,rc);
    printMessage("\n total rows affceted:%d",len);
    my_assert(len == 1);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from t_setpos_upd_bug1");
    mystmt(hstmt,rc);

    my_assert(2 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_acc_update)
{
    SQLRETURN rc;
    long id,id1;
    SQLLEN pcrow;
    SQLHSTMT hstmt1;

    tmysql_exec(hstmt,"drop table t_acc_update");
    rc = tmysql_exec(hstmt,"create table t_acc_update(id int)");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into t_acc_update values(1)");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into t_acc_update values(2)");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);

    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"select id from t_acc_update where id = ?",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_DEFAULT,SQL_INTEGER,11,0,&id,0,NULL);
    mystmt(hstmt,rc);

    id = 2;
    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt,1,SQL_C_LONG,&id1,512,NULL);
    mystmt(hstmt,rc);
    printMessage("outdata:%d\n",id1);

    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt,SQL_UNBIND);
    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);


    rc = SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,0L);
    mycon(hdbc,rc);

    rc = SQLAllocStmt(hdbc,&hstmt1);
    mycon(hdbc,rc);

    id = 2;
    id1=2;
    rc = SQLBindParameter(hstmt1,1,SQL_PARAM_INPUT,SQL_C_LONG,SQL_INTEGER,10,0,&id,0,NULL);
    mystmt(hstmt1,rc);

    rc = SQLBindParameter(hstmt1,2,SQL_PARAM_INPUT,SQL_C_DEFAULT,SQL_INTEGER,11,0,&id1,0,NULL);
    mystmt(hstmt1,rc);

    rc = SQLExecDirect(hstmt1,"UPDATE t_acc_update SET id = ?  WHERE id = ?",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLRowCount(hstmt1,&pcrow);
    mystmt(hstmt1,rc);
    printMessage("rows affected:%d\n",pcrow);

    SQLFreeStmt(hstmt1,SQL_RESET_PARAMS);
    rc = SQLFreeStmt(hstmt1,SQL_DROP);
    mystmt(hstmt1,rc);

    rc = SQLTransact(NULL,hdbc,0);
    mycon(hdbc,rc);

    rc = SQLSetConnectOption(hdbc,SQL_AUTOCOMMIT,1L);
    mycon(hdbc,rc);

  return OK;
}


void tmy_cursor(SQLHSTMT hstmt,SQLCHAR *setCurName,SQLCHAR *getCurName,SQLSMALLINT setLen)
{
    SQLRETURN rc;
    SQLSMALLINT getLen;

    printMessage("\nsetcursor:%s(%d)",setCurName,setLen);
    rc = SQLSetCursorName(hstmt,setCurName,setLen);
    mystmt(hstmt,rc);

    rc = SQLGetCursorName(hstmt,getCurName,20,&getLen);
    mystmt(hstmt,rc);

    printMessage("\ngetcursor:%s(%d)\n",getCurName,strlen(getCurName));
}

DECLARE_TEST(tmycursor_1)
{
    SQLCHAR getCurName[20];

    tmy_cursor(hstmt,"MYSQL",getCurName,5);
    myassert(strcmp(getCurName,"MYSQL")==0);

    tmy_cursor(hstmt,"MYSQL",getCurName,10);
    myassert(strcmp(getCurName,"MYSQL")==0);

    tmy_cursor(hstmt,"MYSQL",getCurName,2);
    myassert(strcmp(getCurName,"MY")==0);

  return OK;
}


DECLARE_TEST(tmy_cursor2)
{
    SQLRETURN   rc;
    SQLCHAR     getCursor[50];
    SQLSMALLINT getLen;

    rc = SQLSetCursorName(hstmt,"MYODBC",6);
    mystmt(hstmt,rc);

    memset(getCursor,0,50);
    getLen = -1;

    rc = SQLGetCursorName(hstmt,getCursor,0,&getLen);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    myassert(strcmp(getCursor,"")==0);
    myassert(getLen == 6);

    memset(getCursor,0,50);
    getLen = -1;

    rc = SQLGetCursorName(hstmt,getCursor,-1,&getLen);
    mystmt_err(hstmt,rc == SQL_ERROR,rc);

    memset(getCursor,0,50);
    getLen = -1;

    rc = SQLGetCursorName(hstmt,getCursor,4,&getLen);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    printMessage("\ntmy_cursor2:%s(%d)",getCursor,getLen);
    myassert(strcmp(getCursor,"MYO")==0);
    myassert(getLen == 6);

    rc = SQLGetCursorName(hstmt,getCursor,6,&getLen);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    printMessage("\ntmy_cursor2:%s(%d)",getCursor,getLen);
    myassert(strcmp(getCursor,"MYODB")==0);
    myassert(getLen == 6);

    rc = SQLGetCursorName(hstmt,getCursor,7,&getLen);
    mystmt(hstmt,rc);
    printMessage("\ntmy_cursor2:%s(%d)",getCursor,getLen);
    myassert(strcmp(getCursor,"MYODBC")==0);
    myassert(getLen == 6);

  return OK;
}


DECLARE_TEST(tmy_cursor3)
{
    SQLRETURN   rc;
    SQLCHAR     setCursor[50];
    SQLCHAR     getCursor[50];
    SQLSMALLINT getLen;
    SQLHSTMT    hstmt1;

    memset(getCursor,0,50);
    getLen = -1;

    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLGetCursorName(hstmt,getCursor,20,&getLen);
    mystmt(hstmt,rc);

    strcpy(setCursor,"MYSQLODBC");
    rc = SQLSetCursorName(hstmt,setCursor,9);
    mystmt(hstmt,rc);

    memset(getCursor,0,50);
    getLen = -1;

    rc = SQLAllocStmt(hdbc,&hstmt1);
    mycon(hdbc,rc);

    rc = SQLGetCursorName(hstmt1,getCursor,20,&getLen);
    mystmt(hstmt1,rc);

    rc = SQLFreeStmt(hstmt1,SQL_DROP);
    mystmt(hstmt1,rc);

  return OK;
}


DECLARE_TEST(t_nativesql)
{
    SQLRETURN rc;
    SQLCHAR    Statement_in[3000];
    SQLCHAR    Statement_out[4000];
    SQLUINTEGER StmtLen;

    strcpy(Statement_in, "select * from venu");

    rc = SQLNativeSql(hdbc, Statement_in,
                      SQL_NTS,Statement_out,
                      sizeof(Statement_out), &StmtLen);
    mycon(hdbc,rc);
    printMessage("outstr:%s(%d)\n",Statement_out,StmtLen);
    myassert(StmtLen == strlen(Statement_in));

  return OK;
}


DECLARE_TEST(t_desccol1)
{
    SQLRETURN rc;

    tmysql_exec(hstmt,"drop table if exists t_desccol1");
    rc = SQLExecDirect(hstmt,"create table t_desccol1\
                 ( record decimal(8,0),\
                   title varchar(250),\
                   num1 float,\
                   num2 decimal(7,0),\
                   num3 decimal(12,3),\
                   code char(3),\
                   sdate date,\
                   stime time,\
                   numer numeric(7,0),\
                   muner1 numeric(12,5))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_desccol1");
    mystmt(hstmt,rc);

    {
        SQLCHAR      ColumnName[255];
        SQLSMALLINT  ColumnNameSize;
        SQLSMALLINT  ColumnSQLDataType;
        SQLULEN      ColumnSize;
        SQLSMALLINT  ColumnDecimals;
        SQLSMALLINT  ColumnNullable;
        SQLSMALLINT  index, pccol;

        rc = SQLNumResultCols(hstmt,(SQLSMALLINT *)&pccol);
        mystmt(hstmt,rc);
        printMessage("total columns:%d\n",pccol);

        printMessage("Name   nlen type    size decs null\n");
        for ( index = 1; index <= pccol; index++)
        {
            rc = SQLDescribeCol(hstmt, index, ColumnName,
                                sizeof(ColumnName),
                                &ColumnNameSize, &ColumnSQLDataType,
                                &ColumnSize,
                                &ColumnDecimals, &ColumnNullable);
            mystmt(hstmt,rc);

            printMessage("%-6s %4d %4d %7ld %4d %4d\n", ColumnName,
                         ColumnNameSize, ColumnSQLDataType, ColumnSize,
                         ColumnDecimals, ColumnNullable);
        }
    }

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_exfetch)
{
    SQLRETURN rc;
    long i;

    tmysql_exec(hstmt,"drop table t_exfetch");

    rc = tmysql_exec(hstmt,"create table t_exfetch(col1 int)");
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"insert into t_exfetch values(?)",SQL_NTS);
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

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLExecDirect(hstmt,"select * from t_exfetch",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&i,0,NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_LAST,0,NULL,NULL);/* 5 */
    mystmt(hstmt,rc);
    my_assert(i == 5);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_PREV,0,NULL,NULL);/* 4 */
    mystmt(hstmt,rc);
    my_assert(i == 4);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_RELATIVE,-3,NULL,NULL);/* 1 */
    mystmt(hstmt,rc);
    my_assert(i == 1);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_RELATIVE,-1,NULL,NULL);/* 0 */
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_PREV,1,NULL,NULL); /* 0 */
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_FIRST,-1,NULL,NULL);/* 0 */
    mystmt(hstmt,rc);
    my_assert(i == 1);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,4,NULL,NULL);/* 4 */
    mystmt(hstmt,rc);
    my_assert(i == 4);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_RELATIVE,2,NULL,NULL);/* 4 */
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_PREV,2,NULL,NULL);/* last */
    mystmt(hstmt,rc);
    my_assert(i == 5);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,2,NULL,NULL);/* last+1 */
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,-7,NULL,NULL);/* 0 */
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_FIRST,2,NULL,NULL);/* 1 */
    mystmt(hstmt,rc);
    my_assert(i == 1);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_PREV,2,NULL,NULL);/* 0 */
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,0,NULL,NULL);/* 1*/
    mystmt(hstmt,rc);
    my_assert(i == 1);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_PREV,0,NULL,NULL);/* 0 */
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_RELATIVE,-1,NULL,NULL); /* 0 */
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_RELATIVE,1,NULL,NULL); /* 1 */
    mystmt(hstmt,rc);
    my_assert(i == 1); /* MyODBC .39 returns 2 instead of 1 */

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_RELATIVE,-1,NULL,NULL);/* 0 */
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_RELATIVE,1,NULL,NULL);/* 1 */
    mystmt(hstmt,rc);
    my_assert(i == 1);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_RELATIVE,1,NULL,NULL);/* 2 */
    mystmt(hstmt,rc);
    my_assert(i == 2);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_RELATIVE,-2,NULL,NULL);/* 0 */
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_RELATIVE,6,NULL,NULL);/* last+1 */
    mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND, rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_PREV,6,NULL,NULL);/* last+1 */
    mystmt(hstmt, rc);
    my_assert(i == 5);

    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);

  return OK;
}


DECLARE_TEST(my_setpos_upd_pk_order)
{
    SQLRETURN rc;
    long nData= 500;
    SQLLEN nlen;
    SQLCHAR szData[255]={0};
    SQLROWSETSIZE pcrow;

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

    printMessage(" row1:%d,%s\n",nData,szData);

    nData = 1000;
    strcpy((char *)szData , "updated");

    rc = SQLSetPos(hstmt,1,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&nlen);
    mystmt(hstmt,rc);

    printMessage(" rows affected:%d\n",nlen);

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
    printMessage("\n total rows affceted:%d",nlen);
    my_assert(nlen == 1);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(my_setpos_upd_pk_order1)
{
    SQLRETURN rc;
    long nData= 500;
    SQLCHAR szData[255]={0};
    SQLROWSETSIZE pcrow;

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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,NULL);
    mystmt(hstmt,rc);

    printMessage(" row1:%d,%s\n",nData,szData);

    nData = 1000;
    strcpy((char *)szData , "updated");

    rc = SQLSetPos(hstmt,1,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt_err(hstmt,rc == SQL_ERROR, rc);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    return OK;
}

BEGIN_TESTS
  ADD_TEST(t_basic)
  ADD_TEST(tmysql_setpos_del)
  ADD_TEST(tmysql_setpos_del1)
  ADD_TEST(tmysql_setpos_upd)
  ADD_TEST(tmysql_setpos_add)
  ADD_TEST(t_sqlspecialcols)
  ADD_TEST(t_sqltables)
  ADD_TEST(tmysql_bindcol)
  ADD_TEST(tmysql_pcbvalue)
  ADD_TEST(tmysql_bindparam)
  ADD_TEST(tmysql_fix)
  ADD_TEST(tmysql_pos_delete)
  ADD_TEST(tmysql_pos_update)
  ADD_TEST(tmysql_pos_update_ex)
  ADD_TEST(tmysql_pos_update_ex1)
  ADD_TEST(tmysql_pos_update_ex2)
  ADD_TEST(tmysql_pos_update_ex3)
  ADD_TEST(tmysql_pos_update_ex4)
  ADD_TEST(tmysql_pos_dyncursor)
  ADD_TEST(tmysql_mtab_setpos_del)
  ADD_TEST(tmysql_showkeys)
  ADD_TEST(tmysql_setpos_pkdel)
  ADD_TEST(t_alias_setpos_pkdel)
  ADD_TEST(t_alias_setpos_del)
  ADD_TEST(tmysql_setpos_pkdel1)
  ADD_TEST(tmysql_setpos_pkdel2)
  ADD_TEST(tmysql_rowstatus)
  ADD_TEST(t_refresh)
  ADD_TEST(tmysql_setpos_pkdel3)
  ADD_TEST(t_mul_pkdel)
  ADD_TEST(t_mul_pkdel1)
  ADD_TEST(t_max_select)
  ADD_TEST(t_tran)
  ADD_TEST(t_tran_ddl)
  ADD_TEST(t_max_con)
  ADD_TEST(t_enumset)
  ADD_TEST(t_desccol)
  ADD_TEST(t_desccolext)
  ADD_TEST(t_colattributes)
  ADD_TEST(t_bigint)
  ADD_TEST(t_setpos_upd_bug1)
  ADD_TEST(t_acc_update)
  ADD_TEST(tmycursor_1)
  ADD_TEST(tmy_cursor2)
  ADD_TEST(tmy_cursor3)
  ADD_TEST(t_nativesql)
  ADD_TEST(t_desccol1)
  ADD_TEST(t_exfetch)
  ADD_TEST(my_setpos_upd_pk_order)
  ADD_TEST(my_setpos_upd_pk_order1)
END_TESTS

RUN_TESTS
