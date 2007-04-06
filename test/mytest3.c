/*
  Copyright (C) 2003-2007 MySQL AB

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

/* Utility function to verify a particular column data */
static void verify_col_data(SQLHSTMT hstmt, const char *table,
                            const char *col, const char *exp_data)
{
  SQLCHAR data[255], query[255];
  SQLRETURN rc;

  if (table && col)
  {
    sprintf(query,"SELECT %s FROM %s",col,table);
    fprintf(stdout,"%s\n", query);

    rc = SQLExecDirect(hstmt, query, SQL_NTS);
    mystmt(hstmt,rc);
  }

  rc = SQLFetch(hstmt);
  if (rc == SQL_NO_DATA)
    myassert(strcmp(exp_data,"SQL_NO_DATA") ==0 );

  rc = SQLGetData(hstmt, 1, SQL_C_CHAR, &data, 255, NULL);
  if (rc == SQL_ERROR)
  {
    fprintf(stdout,"*** ERROR: FAILED TO GET THE RESULT ***\n");
    exit(1);
  }
  fprintf(stdout,"obtained: `%s` (expected: `%s`)\n", data, exp_data);
  myassert(strcmp(data,exp_data) == 0);

  SQLFreeStmt(hstmt, SQL_UNBIND);
  SQLFreeStmt(hstmt, SQL_CLOSE);
}


DECLARE_TEST(t_setpos_del_all)
{
  SQLRETURN rc;
  SQLINTEGER nData[4];
  SQLLEN nlen;
  SQLCHAR szData[4][10];

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

  return OK;
}


DECLARE_TEST(t_setpos_upd_decimal)
{
  SQLRETURN  rc;
  SQLINTEGER rec;

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

  return OK;
}


DECLARE_TEST(tmysql_specialcols)
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


DECLARE_TEST(t_max_select)
{
  SQLRETURN rc;
  SQLCHAR szData[255];
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

  return OK;
}


DECLARE_TEST(t_getcursor)
{
  SQLRETURN rc;
  SQLHSTMT hstmt1,hstmt2,hstmt3;
  SQLCHAR curname[50];
  SQLSMALLINT nlen;

    rc = SQLAllocHandle(SQL_HANDLE_STMT,hdbc,&hstmt1);
    mycon(hdbc, rc);
    rc = SQLAllocHandle(SQL_HANDLE_STMT,hdbc,&hstmt2);
    mycon(hdbc, rc);
    rc = SQLAllocHandle(SQL_HANDLE_STMT,hdbc,&hstmt3);
    mycon(hdbc, rc);

    rc = SQLGetCursorName(hstmt1,curname,50,&nlen);
    if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    {
      fprintf(stdout,"default cursor name  : %s(%d)\n",curname,nlen);
      myassert(nlen == 8);
      myassert(!strcmp(curname,"SQL_CUR0"));

      rc = SQLGetCursorName(hstmt3,curname,50,&nlen);
      mystmt(hstmt1,rc);
      fprintf(stdout,"default cursor name  : %s(%d)\n",curname,nlen);

      rc = SQLGetCursorName(hstmt1,curname,4,&nlen);
      mystmt_err(hstmt1,rc == SQL_SUCCESS_WITH_INFO, rc);
      fprintf(stdout,"truncated cursor name: %s(%d)\n",curname,nlen);
      myassert(nlen == 8);
      myassert(!strcmp(curname,"SQL"));

      rc = SQLGetCursorName(hstmt1,curname,0,&nlen);
      mystmt_err(hstmt1,rc == SQL_SUCCESS_WITH_INFO, rc);
      fprintf(stdout,"untouched cursor name: %s(%d)\n",curname,nlen);
      myassert(nlen == 8);

      rc = SQLGetCursorName(hstmt1,curname,8,&nlen);
      mystmt_err(hstmt1,rc == SQL_SUCCESS_WITH_INFO, rc);
      fprintf(stdout,"truncated cursor name: %s(%d)\n",curname,nlen);
      myassert(nlen == 8);
      myassert(!strcmp(curname,"SQL_CUR"));

      rc = SQLGetCursorName(hstmt1,curname,9,&nlen);
      mystmt(hstmt1,rc);
      fprintf(stdout,"full cursor name     : %s(%d)\n",curname,nlen);
      myassert(nlen == 8);
      myassert(!strcmp(curname,"SQL_CUR0"));
    }

    rc = SQLSetCursorName(hstmt1,"venucur123",7);
    mystmt(hstmt1,rc);

    rc = SQLGetCursorName(hstmt1,curname,8,&nlen);
    mystmt(hstmt1,rc);
    fprintf(stdout,"full setcursor name  : %s(%d)\n",curname,nlen);
    myassert(nlen == 7);
    myassert(!strcmp(curname,"venucur"));

    rc = SQLFreeHandle(SQL_HANDLE_STMT,hstmt1);
    mystmt(hstmt1,rc);

  return OK;
}


DECLARE_TEST(t_getcursor1)
{
  SQLRETURN rc;
  SQLHSTMT hstmt1;
  SQLCHAR curname[50];
  SQLSMALLINT nlen,index;;

  for(index=0; index < 100; index++)
  {
    rc = SQLAllocHandle(SQL_HANDLE_STMT,hdbc,&hstmt1);
    mycon(hdbc, rc);

    rc = SQLGetCursorName(hstmt1,curname,50,&nlen);
    if (rc != SQL_SUCCESS)
      break;
    fprintf(stdout,"%s(%d) \n",curname,nlen);

    rc = SQLFreeHandle(SQL_HANDLE_STMT,hstmt1);
    mystmt(hstmt1,rc);
  }

  return OK;
}


DECLARE_TEST(t_gettypeinfo)
{
  SQLRETURN rc;
  SQLSMALLINT pccol;

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLGetTypeInfo(hstmt,SQL_ALL_TYPES);
    mystmt(hstmt,rc);

    rc = SQLNumResultCols(hstmt,&pccol);
    mystmt(hstmt,rc);
    fprintf(stdout,"total columns: %d\n",pccol);
    myassert(pccol == 19);
    myresult(hstmt);

    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_getinfo)
{
  SQLRETURN rc;
  SQLCHAR   rgbValue[100];
  SQLSMALLINT pcbInfo;

    rc = SQLGetInfo(hdbc,SQL_DRIVER_ODBC_VER,rgbValue,100,&pcbInfo);
    mycon(hdbc,rc);
    fprintf(stdout,"SQL_DRIVER_ODBC_VER: %s(%d)\n",rgbValue,pcbInfo);

  return OK;
}


DECLARE_TEST(t_stmt_attr_status)
{
  SQLRETURN rc;
  SQLUSMALLINT rowStatusPtr[3];
  SQLUINTEGER rowsFetchedPtr;

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

    printMessage("total rows fetched: %d\n",rowsFetchedPtr);
    printMessage("row 0 status      : %d\n",rowStatusPtr[0]);
    printMessage("row 1 status      : %d\n",rowStatusPtr[1]);
    printMessage("row 2 status      : %d\n",rowStatusPtr[2]);
    myassert(rowsFetchedPtr == 1);
    myassert(rowStatusPtr[0] == 0);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROWS_FETCHED_PTR,(SQLPOINTER)0,0);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROW_STATUS_PTR,(SQLPOINTER)0,0);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_max_rows)
{
  SQLRETURN rc;
  SQLUINTEGER i;

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

    /*
     This query includes leading spaces to act as a regression test
     for Bug #6609: SQL_ATTR_MAX_ROWS and leading spaces in query result in
     truncating end of query.
    */
    rc = tmysql_exec(hstmt,"  select * from t_max_rows");
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

  return OK;
}


DECLARE_TEST(t_prepare)
{
  SQLRETURN rc;
  SQLINTEGER nidata= 200, nodata;
  SQLLEN    nlen;
  char      szodata[20],szidata[20]="MySQL";
  short     pccol;

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

  return OK;
}


DECLARE_TEST(t_prepare1)
{
  SQLRETURN rc;
  SQLINTEGER nidata= 1000;

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

  return OK;
}


static void test_diagrec(SQLSMALLINT HandleType,SQLHANDLE Handle,
                         SQLSMALLINT RecNumber, SQLSMALLINT BufferLength,
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

  fprintf(stdout,"%d@%s(%d)\n",rc,message,msglen);
  myassert(return_value_expected == rc);
}


DECLARE_TEST(t_diagrec)
{
  SQLRETURN rc;

  fprintf(stdout," ** SQL_HANDLE_STMT ** \n");

  rc = SQLExecDirect(hstmt,"DROP TABLE ODBC3_NON_EXISTANTi_TAB",SQL_NTS);
  myassert(rc == SQL_ERROR);

  test_diagrec(SQL_HANDLE_STMT,hstmt,2,0,SQL_NO_DATA_FOUND);
  test_diagrec(SQL_HANDLE_STMT,hstmt,1,255,SQL_SUCCESS);
  test_diagrec(SQL_HANDLE_STMT,hstmt,1,0,SQL_SUCCESS_WITH_INFO);
  test_diagrec(SQL_HANDLE_STMT,hstmt,1,10,SQL_SUCCESS_WITH_INFO);
  test_diagrec(SQL_HANDLE_STMT,hstmt,1,-1,SQL_ERROR);

  return OK;
}


DECLARE_TEST(t_acc_crash)
{
  SQLRETURN   rc;
  SQLINTEGER  id;
  SQLCHAR     name[20], data[30];
  SQL_TIMESTAMP_STRUCT ts;

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

  return OK;
}


DECLARE_TEST(t_msdev_bug)
{
  SQLRETURN  rc;
  SQLCHAR    catalog[30];
  SQLLEN     len;

  rc = SQLGetConnectOption(hdbc,SQL_CURRENT_QUALIFIER,&catalog);
  mycon(hdbc,rc);
  fprintf(stdout," SQL_CURRENT_QUALIFIER:%s\n",catalog);

  rc = SQLGetConnectAttr(hdbc,SQL_ATTR_CURRENT_CATALOG,&catalog,30,&len);
  mycon(hdbc,rc);
  fprintf(stdout," SQL_ATTR_CURRENT_CATRALOG:%s(%d)\n",catalog,len);

  return OK;
}


DECLARE_TEST(t_setpos_position)
{
  SQLRETURN rc;
  SQLINTEGER nData= 500;
  SQLLEN nlen;
  SQLCHAR szData[255]={0};
  SQLUINTEGER pcrow;

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
    fprintf(stdout,"total rows affceted:%d\n",nlen);
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

  return OK;
}


DECLARE_TEST(t_pos_column_ignore)
{
  SQLRETURN rc;
  SQLHSTMT hstmt1;
  SQLCHAR szData[]="updated";
  SQLINTEGER nData;
  SQLLEN  pcbValue, nlen, pcrow;

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

  return OK;
}


DECLARE_TEST(t_longlong1)
{
  SQLRETURN   rc;
  SQLINTEGER  session_id, ctn;

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

  return OK;
}


DECLARE_TEST(t_time)
{
  SQLRETURN       rc;
  SQL_TIME_STRUCT tm;
  SQLCHAR         str[20];

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
    fprintf(stdout,"time:%s\n",str);
    my_assert(strcmp(str,"20:59:45")==0);

    rc = SQLFetch(hstmt);
    my_assert(rc == SQL_NO_DATA_FOUND);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_numeric)
{
  SQLRETURN       rc;
  SQL_NUMERIC_STRUCT num;

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

  return OK;
}


DECLARE_TEST(t_decimal)
{
  SQLCHAR         str[20],s_data[]="189.4567";
  SQLDOUBLE       d_data=189.4567;
  SQLINTEGER      i_data=189, l_data=-23;
  SQLRETURN       rc;

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
    fprintf(stdout,"decimal(SQL_C_DOUBLE) : %s\n",str);
    my_assert(strncmp(str,"189.4567",8)==0);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,&str,19,NULL);
    mystmt(hstmt,rc);
    fprintf(stdout,"decimal(SQL_C_INTEGER): %s\n",str);
    my_assert(strncmp(str,"189.0000",5)==0);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,&str,19,NULL);
    mystmt(hstmt,rc);
    fprintf(stdout,"decimal(SQL_C_CHAR)   : %s\n",str);
    my_assert(strncmp(str,"189.4567",8)==0);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,&str,19,NULL);
    mystmt(hstmt,rc);
    fprintf(stdout,"decimal(SQL_C_LONG)   : %s\n",str);
    my_assert(strncmp(str,"-23.00",6)==0);

    rc = SQLFetch(hstmt);
    my_assert(rc == SQL_NO_DATA_FOUND);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_warning)
{
  SQLRETURN rc;
  SQLCHAR szData[20];
  SQLINTEGER pcbValue;

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
    fprintf(stdout,"data: %s(%d)\n",szData,pcbValue);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,4,&pcbValue);
    mystmt_err(hstmt, rc == SQL_SUCCESS_WITH_INFO, rc);
    fprintf(stdout,"data: %s(%d)\n",szData,pcbValue);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,4,&pcbValue);
    mystmt_err(hstmt, rc == SQL_SUCCESS_WITH_INFO, rc);
    fprintf(stdout,"data: %s(%d)\n",szData,pcbValue);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,4,&pcbValue);
    mystmt(hstmt,rc);
    fprintf(stdout,"data: %s(%d)\n",szData,pcbValue);

    rc = SQLFetch(hstmt);
    mystmt_err(hstmt, rc == SQL_NO_DATA_FOUND, rc);

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);

  return OK;
}


DECLARE_TEST(t_multistep)
{
  SQLRETURN  rc;
  SQLCHAR    szData[150];
  SQLLEN     pcbValue;
  SQLINTEGER id;

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
    fprintf(stdout,"id: %ld\n",id);
    myassert(id == 10);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,0,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    fprintf(stdout,"length: %ld\n", pcbValue);
    myassert(pcbValue == 28);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,0,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    fprintf(stdout,"length: %ld\n", pcbValue);
    myassert(pcbValue == 28);

    rc = SQLGetData(hstmt,2,SQL_C_BINARY,szData,0,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    fprintf(stdout,"length: %ld\n", pcbValue);
    myassert(pcbValue == 28);

    rc = SQLGetData(hstmt,2,SQL_C_BINARY,szData,0,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    fprintf(stdout,"length: %ld\n", pcbValue);
    myassert(pcbValue == 28);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,0,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    fprintf(stdout,"length: %ld\n", pcbValue);
    myassert(pcbValue == 28);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,10,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    fprintf(stdout,"data  : %s (%ld)\n",szData,pcbValue);
    myassert(pcbValue == 28);
    myassert(strcmp(szData,"MySQL - O") == 0);

    pcbValue= 0;
    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,5,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    fprintf(stdout,"data  : %s (%ld)\n",szData,pcbValue);
    myassert(pcbValue == 19);
    myassert(strcmp(szData,"pen ") == 0);

    pcbValue= 0;
    szData[0]='A';
    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,0,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    fprintf(stdout,"data  : %s (%ld)\n",szData,pcbValue);
    myassert(pcbValue == 15);
    myassert(szData[0] == 'A');

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,pcbValue+1,&pcbValue);
    mystmt(hstmt,rc);
    fprintf(stdout,"data  : %s (%ld)\n",szData,pcbValue);
    myassert(pcbValue == 15);
    myassert(strcmp(szData,"Source Database") == 0);

    pcbValue= 99;
    szData[0]='A';
    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,0,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    fprintf(stdout,"data  : %s (%ld)\n",szData,pcbValue);
    myassert(pcbValue == 0);
    myassert(szData[0] == 'A');


    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA_FOUND);

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);
}


DECLARE_TEST(t_zerolength)
{
  SQLRETURN  rc;
  SQLCHAR    szData[100], bData[100], bData1[100];
  SQLLEN     pcbValue,pcbValue1,pcbValue2;

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
    fprintf(stdout,"length: %d\n", pcbValue);
    myassert(pcbValue == 0);

    bData[0]=bData[1]='z';
    rc = SQLGetData(hstmt,2,SQL_C_BINARY,bData,0,&pcbValue1);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"length: %d\n", pcbValue1);
    myassert(pcbValue1 == 0);
    myassert(bData[0] == 'z');
    myassert(bData[1] == 'z');

    bData1[0]=bData1[1]='z';
    rc = SQLGetData(hstmt,3,SQL_C_BINARY,bData1,0,&pcbValue2);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"length: %d\n", pcbValue2);
    myassert(pcbValue2 == 0);
    myassert(bData1[0] == 'z');
    myassert(bData1[1] == 'z');

    pcbValue= pcbValue1= 99;
    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,1,&pcbValue);
    mystmt(hstmt,rc);
    fprintf(stdout,"data: %s, length: %d\n", szData, pcbValue);
    myassert(pcbValue == 0);
    myassert(szData[0] == '\0');

    bData[0]=bData[1]='z';
    rc = SQLGetData(hstmt,2,SQL_C_BINARY,bData,1,&pcbValue1);
    mystmt(hstmt,rc);
    fprintf(stdout,"data: %s, length: %d\n", bData, pcbValue1);
    myassert(pcbValue1 == 0);
    myassert(bData[0]== '\0');

    bData1[0]=bData1[1]='z';
    rc = SQLGetData(hstmt,3,SQL_C_CHAR,bData1,1,&pcbValue2);
    mystmt(hstmt,rc);
    fprintf(stdout,"data: %s, length: %d\n", bData1, pcbValue2);
    myassert(pcbValue2 == 0);
    myassert(bData1[0] == '\0');
    myassert(bData1[1] == 'z');

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    pcbValue= pcbValue1= 99;
    szData[0]= bData[0]= 'z';
    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,0,&pcbValue);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"length: %d\n", pcbValue);
    myassert(pcbValue == 4);
    myassert(szData[0] == 'z');

    rc = SQLGetData(hstmt,2,SQL_C_BINARY,bData,0,&pcbValue1);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"length: %d\n", pcbValue1);
    myassert(pcbValue1 == 5);
    myassert(bData[0] == 'z');

    bData[0]=bData1[1]='z';
    rc = SQLGetData(hstmt,3,SQL_C_BINARY,bData1,0,&pcbValue2);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"length: %d\n", pcbValue2);
    myassert(pcbValue2 == 5);

    pcbValue= pcbValue1= 99;
    szData[0]= szData[1]= bData[0]= bData[1]= 'z';
    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,1,&pcbValue);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"data: %s, length: %d\n", szData,pcbValue);
    myassert(pcbValue == 4);
    myassert(szData[0] == '\0');

    rc = SQLGetData(hstmt,2,SQL_C_BINARY,bData,1,&pcbValue1);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"data; %s, length: %d\n", bData, pcbValue1);
    myassert(pcbValue1 == 5);
    myassert(bData[0] == 'm');

    bData[0]=bData1[1]='z';
    rc = SQLGetData(hstmt,3,SQL_C_BINARY,bData1,1,&pcbValue2);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"length: %d\n", pcbValue2);
    myassert(pcbValue2 == 5);
    myassert(bData1[0] == 'm');
    myassert(bData1[1] == 'z');

    pcbValue= pcbValue1= 99;
    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,4,&pcbValue);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"data: %s, length: %d\n", szData, pcbValue);
    myassert(pcbValue == 4);
    myassert(strcmp(szData,"ven")==0);

    rc = SQLGetData(hstmt,2,SQL_C_BINARY,bData,4,&pcbValue1);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"data: %s, length: %d\n", bData, pcbValue1);
    myassert(pcbValue1 == 5);
    myassert(strncmp(bData,"mysq",4)==0);

    pcbValue= pcbValue1= 99;
    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,5,&pcbValue);
    mystmt(hstmt,rc);
    fprintf(stdout,"data: %s, length: %d\n", szData, pcbValue);
    myassert(pcbValue == 4);
    myassert(strcmp(szData,"venu")==0);

    rc = SQLGetData(hstmt,2,SQL_C_BINARY,bData,5,&pcbValue1);
    mystmt(hstmt,rc);
    fprintf(stdout,"data: %s, length: %d\n", bData, pcbValue1);
    myassert(pcbValue1 == 5);
    myassert(strncmp(bData,"mysql",5)==0);

    szData[0]= 'z';
    rc = SQLGetData(hstmt,3,SQL_C_CHAR,szData,0,&pcbValue);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"data: %s, length: %d\n", szData, pcbValue);
    myassert(pcbValue == 5 || pcbValue == 10);
    myassert(szData[0] == 'z');

#if TO_BE_FIXED_IN_DRIVER
    szData[0]=szData[1]='z';
    rc = SQLGetData(hstmt,3,SQL_C_CHAR,szData,1,&pcbValue);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"data: %s, length: %d\n", szData, pcbValue);
    myassert(pcbValue == 10);
    myassert(szData[0] == 'm');
    myassert(szData[1] == 'z');

    rc = SQLGetData(hstmt,3,SQL_C_CHAR,szData,4,&pcbValue);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    fprintf(stdout,"data: %s, length: %d\n", szData, pcbValue);
    myassert(pcbValue == 10);
    myassert(strncmp(szData,"mont",4) == 0);

    rc = SQLGetData(hstmt,3,SQL_C_CHAR,szData,5,&pcbValue);
    mystmt(hstmt,rc);
    fprintf(stdout,"data: %s, length: %d\n", szData, pcbValue);
    myassert(pcbValue == 10);
    myassert(strncmp(szData,"monty",5) == 0);
#endif

    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA_FOUND);

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);

  return OK;
}


DECLARE_TEST(t_pos_datetime_delete)
{
  SQLRETURN rc;
  SQLHSTMT  hstmt1;
  SQLINTEGER int_data;
  SQLLEN    row_count, cur_type;

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
    fprintf(stdout,"current_row: %d\n", int_data);
    myassert(int_data == 1);

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt1,"DELETE FROM t_pos_delete WHERE CURRENT OF venu_cur",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLRowCount(hstmt1,&row_count);
    mystmt(hstmt1,rc);
    fprintf(stdout, "rows affected: %d\n", row_count);
    myassert(row_count == 1);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,NULL,NULL);
    mystmt(hstmt,rc);
    fprintf(stdout,"current_row: %d\n", int_data);
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
    fprintf(stdout, "rows affected: %d\n", row_count);
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

  return OK;
}


DECLARE_TEST(t_pos_datetime_delete1)
{
  SQLRETURN rc;
  SQLHSTMT hstmt1;
  SQLINTEGER int_data;
  SQLLEN row_count, cur_type;

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
    fprintf(stdout,"current_row: %d\n", int_data);
    myassert(int_data == 2);

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt1,"DELETE FROM t_pos_delete WHERE CURRENT OF venu_cur",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLRowCount(hstmt1,&row_count);
    mystmt(hstmt1,rc);
    fprintf(stdout, "rows affected: %d\n", row_count);
    myassert(row_count == 1);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,NULL,NULL);
    mystmt(hstmt,rc);
    fprintf(stdout,"current_row: %d\n", int_data);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,NULL,NULL);
    mystmt(hstmt,rc);
    fprintf(stdout,"current_row: %d\n", int_data);

    /*rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,NULL,NULL);
    mystmt(hstmt,rc);*/

    rc = SQLSetPos(hstmt,1,SQL_POSITION,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt1,"DELETE FROM t_pos_delete WHERE CURRENT OF venu_cur",SQL_NTS);
    mystmt(hstmt1,rc);

    rc = SQLRowCount(hstmt1,&row_count);
    mystmt(hstmt1,rc);
    fprintf(stdout, "rows affected: %d\n", row_count);
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

  return OK;
}


#define TEST_ODBC_TEXT_LEN 3000
DECLARE_TEST(t_text_fetch)
{
  SQLRETURN  rc;
  SQLINTEGER i;
  SQLLEN     row_count, length;
  SQLCHAR    data[TEST_ODBC_TEXT_LEN+1];

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
       fprintf(stdout,"row '%d' (lengths: \n", row_count);
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
    fprintf(stdout,"total rows: %d\n", row_count);
    myassert(row_count == i);

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"DROP TABLE t_text_fetch",SQL_NTS);
    mystmt(hstmt,rc);

  return OK;
}


/* To test SQLColumns misc case */
DECLARE_TEST(t_columns)
{
  SQLSMALLINT   NumPrecRadix, DataType, Nullable, DecimalDigits;
  SQLLEN        cbColumnSize, cbDecimalDigits, cbNumPrecRadix,
                cbDatabaseName, cbDataType, cbNullable;
  SQLRETURN     rc;
  SQLUINTEGER   ColumnSize, i;
  SQLUINTEGER   ColumnCount= 7;
  SQLCHAR       ColumnName[MAX_NAME_LEN], DatabaseName[MAX_NAME_LEN];
  SQLINTEGER    Values[7][5][2]=
  {
    { {5,2},  {6,4}, {0,2},  {10,2},  {1,2}},
    { {1,2},  {5,4},  {0,-1}, {10,-1}, {1,2}},
    { {12,2}, {20,4}, {0,-1}, {10,-1}, {0,2}},
    { {3,2},  {10,4}, {2,2},  {10,2},  {1,2}},
    { {-6,2},  {4,4}, {0,2},  {10,2},  {0,2}},
    { {4,2}, {11,4}, {0,2},  {10,2},  {0,2}},
    { {-6,2}, {4,4}, {0,2},  {10,2},  {0,2}}
  };

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

      fprintf(stdout,"Column %s:\n", ColumnName);
      fprintf(stdout,"\t DataType     = %d(%d)\n", DataType, cbDataType);
      fprintf(stdout,"\t ColumnSize   = %d(%d)\n", ColumnSize, cbColumnSize);
      fprintf(stdout,"\t DecimalDigits= %d(%d)\n", DecimalDigits, cbDecimalDigits);
      fprintf(stdout,"\t NumPrecRadix = %d(%d)\n", NumPrecRadix, cbNumPrecRadix);
      fprintf(stdout,"\t Nullable     = %s(%d)\n\n",
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

  return OK;
}


/* To test a convertion type */
DECLARE_TEST(t_convert_type)
{
  SQLRETURN   rc;
  SQLSMALLINT SqlType, DateType;
  SQLCHAR     ColName[MAX_NAME_LEN];
  SQLCHAR     DbVersion[MAX_NAME_LEN];
  SQLINTEGER  OdbcVersion;

    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLGetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,&OdbcVersion,0,NULL);
    myenv(henv,rc);

    fprintf(stdout,"odbc version:\n");
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

    fprintf(stdout,"MAX(col0): %d\n", SqlType);
    myassert(SqlType == SQL_INTEGER);

    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"SELECT MAX(col1) FROM t_convert",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLDescribeCol(hstmt,1,(SQLCHAR *)&ColName,MAX_NAME_LEN,NULL,&SqlType,NULL,NULL,NULL);
    mystmt(hstmt,rc);

    fprintf(stdout,"MAX(col1): %d\n", SqlType);
    myassert(SqlType == DateType);

    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"SELECT MAX(col2) FROM t_convert",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLDescribeCol(hstmt,1,(SQLCHAR *)&ColName,MAX_NAME_LEN,NULL,&SqlType,NULL,NULL,NULL);
    mystmt(hstmt,rc);

    fprintf(stdout,"MAX(col0): %d\n", SqlType);

    SQLFreeStmt(hstmt,SQL_CLOSE);

    if (strncmp(DbVersion,"4.",2) >= 0)
    {
      rc = SQLExecDirect(hstmt,"SELECT CAST(MAX(col1) AS DATE) AS col1 FROM t_convert",SQL_NTS);
      mystmt(hstmt,rc);

      rc = SQLDescribeCol(hstmt,1,(SQLCHAR *)&ColName,MAX_NAME_LEN,NULL,&SqlType,NULL,NULL,NULL);
      mystmt(hstmt,rc);

      fprintf(stdout,"CAST(MAX(col1) AS DATE): %d\n", SqlType);
      myassert(SqlType == DateType);

      SQLFreeStmt(hstmt,SQL_CLOSE);

      rc = SQLExecDirect(hstmt,"SELECT CONVERT(MAX(col1),DATE) AS col1 FROM t_convert",SQL_NTS);
      mystmt(hstmt,rc);

      rc = SQLDescribeCol(hstmt,1,(SQLCHAR *)&ColName,MAX_NAME_LEN,NULL,&SqlType,NULL,NULL,NULL);
      mystmt(hstmt,rc);

      fprintf(stdout,"CONVERT(MAX(col1),DATE): %d\n", SqlType);
      myassert(SqlType == DateType);

      SQLFreeStmt(hstmt,SQL_CLOSE);

      rc = SQLExecDirect(hstmt,"SELECT CAST(MAX(col1) AS CHAR) AS col1 FROM t_convert",SQL_NTS);
      mystmt(hstmt,rc);

      rc = SQLDescribeCol(hstmt,1,(SQLCHAR *)&ColName,MAX_NAME_LEN,NULL,&SqlType,NULL,NULL,NULL);
      mystmt(hstmt,rc);

      fprintf(stdout,"CAST(MAX(col1) AS CHAR): %d\n", SqlType);
      myassert(SqlType == SQL_VARCHAR || SqlType == SQL_LONGVARCHAR);

      SQLFreeStmt(hstmt,SQL_CLOSE);
    }

    rc = SQLExecDirect(hstmt,"DROP TABLE t_convert",SQL_NTS);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt,SQL_CLOSE);

  return OK;
}

/* Test the bug when two stmts are used with the don't cache results */
DECLARE_TEST(t_cache_bug)
{
  SQLRETURN  rc;
  SQLHENV    henv1;
  SQLHDBC    hdbc1;
  SQLHSTMT   hstmt11, hstmt12;
  SQLCHAR    conn[MAX_NAME_LEN];

    sprintf(conn,"DSN=%s;USER=%s;PASSWORD=%s;OPTION=1048579",
            mydsn,myuid,mypwd);
    if (mysock != NULL)
    {
      strcat(conn, ";SOCKET=");
      strcat(conn, mysock);
    }
    mydrvconnect(&henv1,&hdbc1,&hstmt11,conn);

    tmysql_exec(hstmt11,"drop table t_cache");
    rc = tmysql_exec(hstmt11,"create table t_cache(id int)");
    mystmt(hstmt11,rc);

    rc = tmysql_exec(hstmt11,"insert into t_cache values(1)");
    mystmt(hstmt11,rc);

    rc = tmysql_exec(hstmt11,"insert into t_cache values(2)");
    mystmt(hstmt11,rc);

    rc = tmysql_exec(hstmt11,"insert into t_cache values(3)");
    mystmt(hstmt11,rc);

    rc = tmysql_exec(hstmt11,"insert into t_cache values(4)");
    mystmt(hstmt11,rc);

    rc = tmysql_exec(hstmt11,"insert into t_cache values(5)");
    mystmt(hstmt11,rc);

    rc = SQLExecDirect(hstmt11,"select * from t_cache",SQL_NTS);
    mystmt(hstmt11,rc);

    rc = SQLFetch(hstmt11);
    mystmt(hstmt11,rc);

    rc = SQLFetch(hstmt11);
    mystmt(hstmt11,rc);

    rc = SQLAllocHandle(SQL_HANDLE_STMT,hdbc1,&hstmt12);
    mycon(hdbc1,rc);

    rc = SQLColumns(hstmt12,test_db,SQL_NTS,
                    NULL,0,"t_cache",SQL_NTS,
                    NULL,0);
    mystmt(hstmt12,rc);

    rc = SQLFetch(hstmt12);
    mystmt(hstmt12,rc);

    rc = SQLFetch(hstmt11);
    mystmt(hstmt11,rc);

    rc = SQLFetch(hstmt12);
    myassert(rc == SQL_NO_DATA);

    rc = SQLFetch(hstmt11);
    mystmt(hstmt12,rc);

    rc = SQLFreeHandle(SQL_HANDLE_STMT,hstmt12);
    mystmt(hstmt12,rc);

    rc = SQLFetch(hstmt11);
    mystmt(hstmt12,rc);

    rc = SQLFetch(hstmt11);
    myassert(rc == SQL_NO_DATA);

    rc = SQLFreeStmt(hstmt11, SQL_DROP);
    mystmt(hstmt11,rc);

    rc = SQLDisconnect(hdbc1);
    mycon(hdbc1,rc);

    rc = SQLFreeConnect(hdbc1);
    mycon(hdbc1,rc);

    rc = SQLFreeEnv(henv1);
    myenv(henv1,rc);

  return OK;
}


/* Test the bug when two stmts are used with the don't cache results */
DECLARE_TEST(t_non_cache_bug)
{
  SQLRETURN  rc;
  SQLHENV    henv1;
  SQLHDBC    hdbc1;
  SQLHSTMT   hstmt11, hstmt12;
  SQLCHAR    conn[MAX_NAME_LEN];

    sprintf(conn,"DSN=%s;USER=%s;PASSWORD=%s;OPTION=3",
            mydsn,myuid,mypwd);
    if (mysock != NULL)
    {
      strcat(conn, ";SOCKET=");
      strcat(conn, mysock);
    }
    mydrvconnect(&henv1,&hdbc1,&hstmt11,conn);

    tmysql_exec(hstmt11,"drop table t_cache");
    rc = tmysql_exec(hstmt11,"create table t_cache(id int)");
    mystmt(hstmt11,rc);

    rc = tmysql_exec(hstmt11,"insert into t_cache values(1)");
    mystmt(hstmt11,rc);

    rc = tmysql_exec(hstmt11,"insert into t_cache values(2)");
    mystmt(hstmt11,rc);

    rc = tmysql_exec(hstmt11,"insert into t_cache values(3)");
    mystmt(hstmt11,rc);

    rc = tmysql_exec(hstmt11,"insert into t_cache values(4)");
    mystmt(hstmt11,rc);

    rc = tmysql_exec(hstmt11,"insert into t_cache values(5)");
    mystmt(hstmt11,rc);

    rc = SQLExecDirect(hstmt11,"select * from t_cache",SQL_NTS);
    mystmt(hstmt11,rc);

    rc = SQLFetch(hstmt11);
    mystmt(hstmt11,rc);

    rc = SQLFetch(hstmt11);
    mystmt(hstmt11,rc);

    rc = SQLAllocHandle(SQL_HANDLE_STMT,hdbc1,&hstmt12);
    mycon(hdbc1,rc);

    rc = SQLColumns(hstmt12,test_db,SQL_NTS,
                    NULL,0,"t_cache",SQL_NTS,
                    NULL,0);
    mystmt(hstmt12,rc);

    rc = SQLFetch(hstmt12);
    mystmt(hstmt12,rc);

    rc = SQLFetch(hstmt11);
    mystmt(hstmt11,rc);

    rc = SQLFetch(hstmt12);
    myassert(rc == SQL_NO_DATA);

    rc = SQLFetch(hstmt11);
    mystmt(hstmt12,rc);

    rc = SQLFreeHandle(SQL_HANDLE_STMT,hstmt12);
    mystmt(hstmt12,rc);

    rc = SQLFetch(hstmt11);
    mystmt(hstmt12,rc);

    rc = SQLFetch(hstmt11);
    myassert(rc == SQL_NO_DATA);

    rc = SQLFreeStmt(hstmt11, SQL_DROP);
    mystmt(hstmt11,rc);

    rc = SQLDisconnect(hdbc1);
    mycon(hdbc1,rc);

    rc = SQLFreeConnect(hdbc1);
    mycon(hdbc1,rc);

    rc = SQLFreeEnv(henv1);
    myenv(henv1,rc);

  return OK;
}


/* Test the bug when blob size > 8k */
DECLARE_TEST(t_blob_bug)
{
  SQLRETURN  rc;
  SQLCHAR    *data;
  SQLINTEGER i;
  SQLLEN     length;
  const SQLINTEGER max_blob_size=1024*100;

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

    fprintf(stdout,"inserting %d rows\n\n", max_blob_size / 1024);
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

      fprintf(stdout,"row %d length: %d\n", i, length);
      myassert(length == i * 1024);
    }
    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA);

    free(data);

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);

  return OK;
}


/* Test the bug SQLTables */
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


DECLARE_TEST(t_tables_bug)
{
  SQLRETURN   rc;
  SQLSMALLINT i, ColumnCount, pcbColName, pfSqlType, pibScale, pfNullable;
  SQLUINTEGER pcbColDef;
  SQLCHAR     szColName[MAX_NAME_LEN];

   SQLFreeStmt(hstmt, SQL_CLOSE);

   rc = SQLTables(hstmt,NULL,0,NULL,0,NULL,0,"'TABLE'",SQL_NTS);
   mystmt(hstmt,rc);

   rc = SQLNumResultCols(hstmt,&ColumnCount);
   mystmt(hstmt,rc);

   fprintf(stdout, "total columns in SQLTables: %d\n", ColumnCount);
   myassert(ColumnCount == 5);

   for (i= 1; i <= ColumnCount; i++)
   {
     rc = SQLDescribeCol(hstmt, (SQLUSMALLINT)i,
                         szColName,MAX_NAME_LEN,&pcbColName,
                         &pfSqlType,&pcbColDef,&pibScale,&pfNullable);
     mystmt(hstmt,rc);

     fprintf(stdout, "Column Number'%d':\n", i);
     fprintf(stdout, "\t Column Name    : %s\n", szColName);
     fprintf(stdout, "\t NameLengh      : %d\n", pcbColName);
     fprintf(stdout, "\t DataType       : %d\n", pfSqlType);
     fprintf(stdout, "\t ColumnSize     : %d\n", pcbColDef);
     fprintf(stdout, "\t DecimalDigits  : %d\n", pibScale);
     fprintf(stdout, "\t Nullable       : %d\n", pfNullable);

     myassert(strcmp(t_tables_bug_data[i-1].szColName,szColName) == 0);
     myassert(t_tables_bug_data[i-1].pcbColName == pcbColName);
     myassert(t_tables_bug_data[i-1].pfSqlType == pfSqlType);
     myassert(t_tables_bug_data[i-1].pcbColDef == pcbColDef);
     myassert(t_tables_bug_data[i-1].pibScale == pibScale);
     myassert(t_tables_bug_data[i-1].pfNullable == pfNullable);
   }
   SQLFreeStmt(hstmt,SQL_CLOSE);

  return OK;
}


/* Test for a simple SQLPutData and SQLParamData handling for longtext */
DECLARE_TEST(t_putdata)
{
  SQLRETURN  rc;
  SQLLEN     pcbLength;
  SQLINTEGER c1;
  SQLCHAR    data[255];
  SQLPOINTER token;

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
    fprintf(stdout,"data: %s(%d)\n", data, pcbLength);
    myassert(strcmp(data,"mysql - the open source database company")==0);
    myassert(pcbLength == 40);

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);

  return OK;
}

/* Test for a simple SQLPutData and SQLParamData handling for longtext */
DECLARE_TEST(t_putdata1)
{
  SQLRETURN  rc;
  SQLLEN     pcbLength;
  SQLINTEGER c1;
  SQLCHAR    data[255];
  SQLPOINTER token;

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
    fprintf(stdout,"data: %s(%d)\n", data, pcbLength);
    myassert(strcmp(data,"mysql - the open source database company")==0);
    myassert(pcbLength == 40);

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);

  return OK;
}


/* Test for a simple SQLPutData and SQLParamData handling for longtext */
DECLARE_TEST(t_putdata2)
{
  SQLRETURN  rc;
  SQLLEN     pcbLength;
  SQLINTEGER c1;
  SQLCHAR    data[255];
  SQLPOINTER token;

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
    fprintf(stdout,"data: %s(%d)\n", data, pcbLength);
    myassert(strcmp(data,"mysql - the open source database company")==0);
    myassert(pcbLength == 40);

    pcbLength= 0;
    rc = SQLGetData(hstmt, 2, SQL_C_CHAR, data, sizeof(data), &pcbLength);
    mystmt(hstmt,rc);
    fprintf(stdout,"data: %s(%d)\n", data, pcbLength);
    myassert(strcmp(data,"MySQL AB")==0);
    myassert(pcbLength == 8);

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);

  return OK;
}


/* Test for a simple time struct */
DECLARE_TEST(t_time1)
{
  SQLRETURN       rc;
  SQL_TIME_STRUCT tt;
  SQLCHAR         data[30];
  SQLLEN          length;

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
    fprintf(stdout,"time: %s(%d)\n", data, length);
    myassert(strcmp(data,"00:00:03")==0);
    myassert(length == 8);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &length);
    mystmt(hstmt,rc);
    fprintf(stdout,"time: %s(%d)\n", data, length);
    myassert(strcmp(data,"01:00:00")==0);
    myassert(length == 8);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &length);
    mystmt(hstmt,rc);
    fprintf(stdout,"time: %s(%d)\n", data, length);
    myassert(strcmp(data,"19:00:00")==0);
    myassert(length == 8);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &length);
    mystmt(hstmt,rc);
    fprintf(stdout,"time: %s(%d)\n", data, length);
    myassert(strcmp(data,"01:01:00")==0);
    myassert(length == 8);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &length);
    mystmt(hstmt,rc);
    fprintf(stdout,"time: %s(%d)\n", data, length);
    myassert(strcmp(data,"01:00:01")==0);
    myassert(length == 8);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &length);
    mystmt(hstmt,rc);
    fprintf(stdout,"time: %s(%d)\n", data, length);
    myassert(strcmp(data,"00:01:00")==0);
    myassert(length == 8);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &length);
    mystmt(hstmt,rc);
    fprintf(stdout,"time: %s(%d)\n", data, length);
    myassert(strcmp(data,"00:11:12")==0);
    myassert(length == 8);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &length);
    mystmt(hstmt,rc);
    fprintf(stdout,"time: %s(%d)\n", data, length);
    myassert(strcmp(data,"01:01:01")==0);
    myassert(length == 8);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &length);
    mystmt(hstmt,rc);
    fprintf(stdout,"time: %s(%d)\n", data, length);

    myassert(strcmp(data,"00:00:00")==0);
    myassert(length == 8);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &length);
    mystmt(hstmt,rc);
    fprintf(stdout,"time: %s(%d)\n", data, length);
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
    fprintf(stdout,"time: %d:%d:%d(%d)\n", tt.hour, tt.minute, tt.second, length);

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
    fprintf(stdout,"time: %d:%d:%d(%d)\n", tt.hour, tt.minute, tt.second, length);
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
    fprintf(stdout,"time: %d:%d:%d(%d)\n", tt.hour, tt.minute, tt.second, length);
    myassert(tt.hour == 00 || tt.minute == 00 || tt.second == 00);
    myassert(length == sizeof(SQL_TIME_STRUCT));

    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA);

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);

  return OK;
}


/* test for SQL_ATTR_ROW_ARRAY_SIZE */
DECLARE_TEST(t_row_array_size)
{
  SQLRETURN rc;
  SQLINTEGER i, iarray[15];
  SQLLEN nrows;
  const int max_rows=9;

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
    rc = SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0);
    mystmt(hstmt,rc);
    my_assert(nrows == 2);
    my_assert(iarray[0]==1);
    my_assert(iarray[1]==2);

    /* row 3-4 */
    rc = SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0);
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
    rc = SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0);
    mystmt(hstmt,rc);
    my_assert(nrows == 2);
    my_assert(iarray[0]==7);
    my_assert(iarray[1]==8);

    /* row 9 */
    rc = SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0);
    mystmt(hstmt,rc);
    my_assert(nrows == 1);
    my_assert(iarray[0]==9);

    rc = SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0);
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROW_ARRAY_SIZE,(SQLPOINTER)1,0);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROWS_FETCHED_PTR,(SQLPOINTER)0,SQL_IS_POINTER);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_empty_str_bug)
{
  SQLRETURN    rc;
  SQLINTEGER   id;
  SQLLEN       name_len, desc_len;
  SQLCHAR      name[20], desc[20];

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

  return OK;
}


DECLARE_TEST(t_current_catalog)
{
  SQLCHAR     cur_db[255], db[255];
  SQLRETURN   rc;
  SQLUINTEGER len;

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLGetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (char *)db, 255, &len);
    mycon(hdbc,rc);
    fprintf(stdout,"current_catalog: %s (%ld)\n", db, len);
    myassert(strcmp(db, "test") == 0 || strlen("test") == len);

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
    fprintf(stdout,"current_catalog: %s (%ld)\n", db, len);
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

  return OK;
}


DECLARE_TEST(t_rows_fetched_ptr1)
{
  SQLRETURN   rc;
  SQLLEN      rowsFetched, rowsSize;
  SQLINTEGER  i;

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
      fprintf(stdout,"total rows fetched: %ld\n", rowsFetched);
      myassert(rowsFetched == rowsSize);
      i++; rowsFetched= 0;
      rc = SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0);
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
      fprintf(stdout,"total rows fetched: %ld\n", rowsFetched);
      myassert(rowsFetched == rowsSize);
      i++;rowsFetched= 0;
      rc = SQLFetchScroll(hstmt,SQL_FETCH_NEXT,0);
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
      printMessage("total rows fetched: %ld\n", rowsFetched);
      myassert(rowsFetched == rowsSize);
      i++;rowsFetched= 0;
      rc = SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0);
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

    printMessage("total rows fetched: %ld\n", rowsFetched);
    myassert(rowsFetched == rowsSize);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    printMessage("total rows fetched: %ld\n", rowsFetched);
    myassert(rowsFetched == 2);

    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA);

    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)1, 0);/* reset */
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROWS_FETCHED_PTR, NULL, 0);
    mystmt(hstmt,rc);

  return OK;
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

  fprintf(stdout, "\t Column Name    : %s\n", szColName);
  fprintf(stdout, "\t NameLengh      : %d\n", pcbColName);
  fprintf(stdout, "\t DataType       : %d\n", pfSqlType);
  fprintf(stdout, "\t ColumnSize     : %d\n", pcbColDef);
  fprintf(stdout, "\t DecimalDigits  : %d\n", pibScale);
  fprintf(stdout, "\t Nullable       : %d\n", pfNullable);

  myassert(strcmp(name,szColName) == 0);
  myassert(sql_type == pfSqlType);
  myassert(col_def == pcbColDef || col_def1 == pcbColDef);
  myassert(scale == pibScale);
  myassert(nullable == pfNullable);
}


/* To test SQLDescribeCol */
DECLARE_TEST(t_desc_col)
{
  SQLRETURN   rc;
  SQLSMALLINT ColumnCount;

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

    fprintf(stdout,"total columns: %d\n", ColumnCount);
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


/* Test for a simple SQLPutData and SQLParamData handling bug #1316 */
DECLARE_TEST(t_putdata3)
{
  char buffer[]= "MySQL - The worlds's most popular open source database";
  SQLRETURN  rc;
  const int MAX_PART_SIZE = 5;

  char *pdata= 0, data[50];
  int dynData;
  int commonLen= 20;

  SQLINTEGER  id, id1, id2, id3, resId;
  SQLINTEGER  resUTimeSec;
  SQLINTEGER  resUTimeMSec;
  SQLINTEGER  resDataLen;
  SQLLEN      resData;

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

  return OK;
}


/* Test for misc CONVERT bug #1082 */
DECLARE_TEST(t_convert)
{
  SQLRETURN  rc;
  SQLLEN     data_len;
  SQLCHAR    data[50];

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

  return OK;
}


BEGIN_TESTS
  ADD_TEST(t_setpos_del_all)
  ADD_TEST(t_setpos_upd_decimal)
  ADD_TEST(tmysql_specialcols)
  ADD_TEST(t_max_select)
  ADD_TEST(t_getcursor)
  ADD_TEST(t_getcursor1)
  ADD_TEST(t_gettypeinfo)
  ADD_TEST(t_getinfo)
  ADD_TEST(t_stmt_attr_status)
  ADD_TEST(t_max_rows)
  ADD_TEST(t_prepare)
  ADD_TEST(t_prepare1)
  ADD_TEST(t_diagrec)
  ADD_TEST(t_acc_crash)
  ADD_TEST(t_msdev_bug)
  ADD_TEST(t_setpos_position)
  ADD_TEST(t_pos_column_ignore)
  ADD_TEST(t_longlong1)
  ADD_TEST(t_time)
  ADD_TEST(t_numeric)
  ADD_TEST(t_decimal)
  ADD_TEST(t_warning)
  ADD_TEST(t_multistep)
  ADD_TEST(t_zerolength)
  ADD_TEST(t_pos_datetime_delete)
  ADD_TEST(t_pos_datetime_delete1)
  ADD_TEST(t_text_fetch)
  ADD_TEST(t_columns)
  ADD_TEST(t_convert_type)
  ADD_TEST(t_cache_bug)
  ADD_TEST(t_non_cache_bug)
  ADD_TEST(t_blob_bug)
  ADD_TEST(t_tables_bug)
  ADD_TEST(t_putdata)
  ADD_TEST(t_putdata1)
  ADD_TEST(t_putdata2)
  ADD_TEST(t_time1)
  ADD_TEST(t_row_array_size)
  ADD_TEST(t_empty_str_bug)
  ADD_TEST(t_current_catalog)
  ADD_TEST(t_rows_fetched_ptr1)
  ADD_TEST(t_desc_col)
  ADD_TEST(t_putdata3)
  ADD_TEST(t_convert)
END_TESTS


RUN_TESTS
