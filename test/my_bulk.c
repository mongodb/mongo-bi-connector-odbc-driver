/*
  Copyright (C) 1997-2007 MySQL AB

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

#define MAX_INSERT_COUNT 800
#define MAX_TXT_LENGTH 10

DECLARE_TEST(t_bulk_check)
{
    SQLRETURN  rc;
    SQLCHAR    ltxt[MAX_TXT_LENGTH];

    SQLExecDirect(hstmt, "DROP TABLE t_bulk_check", SQL_NTS);

    rc = SQLExecDirect(hstmt, "CREATE TABLE t_bulk_check(id int PRIMARY KEY, ltxt text)", SQL_NTS);
    mystmt(hstmt, rc);

    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLPrepare(hstmt, "INSERT INTO t_bulk_check(id,ltxt) values(1,?)", SQL_NTS);
    mystmt(hstmt, rc);

    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
                          0,0,ltxt,MAX_TXT_LENGTH,NULL);
    mystmt(hstmt, rc);

    memset(ltxt, 'E', MAX_TXT_LENGTH);
    ltxt[MAX_TXT_LENGTH] = '\0';

    rc = SQLExecute(hstmt);
    mystmt(hstmt, rc);

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_CLOSE);    

    rc = SQLExecDirect(hstmt, "SELECT * FROM t_bulk_check", SQL_NTS);
    mystmt(hstmt, rc);

    my_assert(1 == my_print_non_format_result(hstmt));

    SQLFreeStmt(hstmt, SQL_CLOSE);

    SQLExecDirect(hstmt, "DROP TABLE t_bulk_check", SQL_NTS);
    SQLFreeStmt(hstmt, SQL_CLOSE);

  return OK;
}


DECLARE_TEST(t_bulk_insert)
{
    SQLRETURN rc;
    SQLINTEGER i,id[MAX_INSERT_COUNT+1];
    char    name[MAX_INSERT_COUNT][40],txt[MAX_INSERT_COUNT][60],ltxt[MAX_INSERT_COUNT][70];
    clock_t start, end;
    double duration,dbl[MAX_INSERT_COUNT];
    double dt;

    SQLExecDirect(hstmt, "DROP TABLE my_bulk", SQL_NTS);

    rc = SQLExecDirect(hstmt, "CREATE TABLE my_bulk ( id   int,          " \
                              "                       v    varchar(100), " \
                              "                       txt  text,         " \
                              "                       ft   float(10),    " \
                              "                       ltxt long varchar )", SQL_NTS);
    mystmt(hstmt, rc);

    SQLFreeStmt(hstmt, SQL_CLOSE);

    dt = 0.23456;

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_STATIC, 0);
    mystmt(hstmt, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)MAX_INSERT_COUNT, 0);
    mystmt(hstmt, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY , (SQLPOINTER)SQL_CONCUR_ROWVER , 0);
    mystmt(hstmt, rc);

    rc = SQLBindCol(hstmt, 1, SQL_C_LONG, &id[0], 0, NULL);
    mystmt(hstmt, rc);

    rc = SQLBindCol(hstmt, 2, SQL_C_CHAR, &name[0], 30 /* sizeof(name[0]) */, NULL);
    mystmt(hstmt, rc);

    rc = SQLBindCol(hstmt, 3, SQL_C_CHAR, &txt[0], sizeof(txt[0]), NULL);
    mystmt(hstmt, rc);

    rc = SQLBindCol(hstmt, 4, SQL_C_DOUBLE, &dbl[0], sizeof(dbl[0]), NULL);
    mystmt(hstmt, rc);

    rc = SQLBindCol(hstmt, 5, SQL_C_CHAR, &ltxt[0], sizeof(ltxt[0]), NULL);
    mystmt(hstmt, rc);

    rc = SQLExecDirect(hstmt, "SELECT id, v, txt, ft, ltxt FROM my_bulk", SQL_NTS);
    mystmt(hstmt, rc);

    rc = SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0);
    mystmt_err(hstmt, rc == SQL_NO_DATA_FOUND, rc);

    for (i= 0; i < MAX_INSERT_COUNT; i++)
    {
        id[i]=i;
        dbl[i] = i+dt;      
        sprintf( name[i], "Varchar%d", i );      
        sprintf( txt[i], "Text%d", i );      
        sprintf( ltxt[i], "LongText, id row:%d", i );
    }    

    printMessage( "\n total bulk adds   : %d", MAX_INSERT_COUNT*2);
    start = clock();    

    rc = SQLBulkOperations(hstmt, SQL_ADD);    
    mystmt(hstmt, rc);

    rc = SQLBulkOperations(hstmt, SQL_ADD);    
    mystmt(hstmt, rc);

    end = clock();

    duration = difftime(end,start)/CLOCKS_PER_SEC;
    printMessage( " (in '%lf' secs)", duration);

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);    

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)1, 0);
    mystmt(hstmt, rc);    

    start= clock();
    rc = SQLExecDirect(hstmt, "SELECT * FROM my_bulk", SQL_NTS);
    mystmt(hstmt, rc);
    my_assert(MAX_INSERT_COUNT*2 == myrowcount(hstmt));
    end= clock();

    duration = difftime(end,start)/CLOCKS_PER_SEC;
    printMessage(" (in '%lf' secs)\n", duration);

    SQLExecDirect(hstmt, "DROP TABLE my_bulk", SQL_NTS);
    SQLFreeStmt(hstmt, SQL_CLOSE);

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


/**
  Bug #24306: SQLBulkOperations always uses indicator varables' values from
  the first record
*/
DECLARE_TEST(t_bulk_insert_indicator)
{
  SQLINTEGER id[4], nData;
  SQLLEN     indicator[4], nLen;

  ok_sql(hstmt, "DROP TABLE IF EXISTS my_bulk");
  ok_sql(hstmt, "CREATE TABLE my_bulk (id int default 5)");

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE,
                                (SQLPOINTER)SQL_CURSOR_STATIC, 0));

  ok_stmt(hstmt,
          SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)3, 0));

  ok_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_LONG, id, 0, indicator));

  ok_sql(hstmt, "SELECT id FROM my_bulk");

  expect_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0),
              SQL_NO_DATA_FOUND);

  id[0]= 1; indicator[0]= SQL_COLUMN_IGNORE;
  id[1]= 2; indicator[1]= SQL_NULL_DATA;
  id[2]= 3; indicator[2]= 0;

  ok_stmt(hstmt, SQLBulkOperations(hstmt, SQL_ADD));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE,
                                (SQLPOINTER)1, 0));

  ok_sql(hstmt, "SELECT id FROM my_bulk");

  ok_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_LONG, &nData, 0, &nLen));

  ok_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0));
  is_num(nData, 5);
  my_assert(nLen != SQL_NULL_DATA);

  ok_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0));
  is_num(nLen, SQL_NULL_DATA);

  ok_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0));
  is_num(nData, 3);
  my_assert(nLen != SQL_NULL_DATA);

  expect_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0),
              SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS my_bulk");

  return OK;
}


/**
  Simple structure for a row (just one element) plus an indicator column.
*/
typedef struct {
  SQLINTEGER val;
  SQLLEN     ind;
} row;


/**
  This is related to the fix for Bug #24306 -- handling of row-wise binding,
  plus handling of SQL_ATTR_ROW_BIND_OFFSET_PTR, within the context of
  SQLBulkOperations(hstmt, SQL_ADD).
*/
DECLARE_TEST(t_bulk_insert_rows)
{
  row        rows[3];
  SQLINTEGER nData, offset;
  SQLLEN     nLen;

  ok_sql(hstmt, "DROP TABLE IF EXISTS my_bulk");
  ok_sql(hstmt, "CREATE TABLE my_bulk (id int default 5)");

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE,
                                (SQLPOINTER)SQL_CURSOR_STATIC, 0));

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE,
                                (SQLPOINTER)3, 0));

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_BIND_TYPE,
                                (SQLPOINTER)sizeof(row), 0));

  ok_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_LONG, &rows[0].val, 0,
                            &rows[0].ind));

  ok_sql(hstmt, "SELECT id FROM my_bulk");

  expect_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0),
              SQL_NO_DATA_FOUND);

  rows[0].val= 1; rows[0].ind= SQL_COLUMN_IGNORE;
  rows[1].val= 2; rows[1].ind= SQL_NULL_DATA;
  rows[2].val= 3; rows[2].ind= 0;

  ok_stmt(hstmt, SQLBulkOperations(hstmt, SQL_ADD));

  /* Now re-insert the last row using SQL_ATTR_ROW_BIND_OFFSET_PTR */
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE,
                                (SQLPOINTER)1, 0));

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_BIND_OFFSET_PTR,
                                (SQLPOINTER)&offset, 0));

  offset= 2 * sizeof(row);

  ok_stmt(hstmt, SQLBulkOperations(hstmt, SQL_ADD));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "SELECT id FROM my_bulk");

  ok_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_LONG, &nData, 0, &nLen));

  ok_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0));
  is_num(nData, 5);
  my_assert(nLen != SQL_NULL_DATA);

  ok_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0));
  is_num(nLen, SQL_NULL_DATA);

  ok_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0));
  is_num(nData, 3);
  my_assert(nLen != SQL_NULL_DATA);

  ok_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0));
  is_num(nData, 3);
  my_assert(nLen != SQL_NULL_DATA);

  expect_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0),
              SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS my_bulk");

  return OK;
}


BEGIN_TESTS
  ADD_TEST(t_bulk_check)
  ADD_TEST(t_bulk_insert)
  ADD_TEST(t_mul_pkdel)
  ADD_TEST(t_mul_pkdel1)
  ADD_TEST(t_bulk_insert_indicator)
  ADD_TEST(t_bulk_insert_rows)
END_TESTS


RUN_TESTS
