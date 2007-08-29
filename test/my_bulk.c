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

DECLARE_TEST(t_bulk_insert)
{
  SQLINTEGER i, id[MAX_INSERT_COUNT+1];
  SQLCHAR    name[MAX_INSERT_COUNT][40],
             txt[MAX_INSERT_COUNT][60],
             ltxt[MAX_INSERT_COUNT][70];
  SQLDOUBLE  dt, dbl[MAX_INSERT_COUNT];

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bulk_insert");
  ok_sql(hstmt, "CREATE TABLE t_bulk_insert (id INT, v VARCHAR(100),"
         "txt TEXT, ft FLOAT(10), ltxt LONG VARCHAR)");

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  dt= 0.23456;

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE,
                                (SQLPOINTER)SQL_CURSOR_STATIC, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE,
                                (SQLPOINTER)MAX_INSERT_COUNT, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY,
                                (SQLPOINTER)SQL_CONCUR_ROWVER, 0));

  ok_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_LONG, id, 0, NULL));
  ok_stmt(hstmt, SQLBindCol(hstmt, 2, SQL_C_CHAR, name, sizeof(name[0]), NULL));
  ok_stmt(hstmt, SQLBindCol(hstmt, 3, SQL_C_CHAR, txt, sizeof(txt[0]), NULL));
  ok_stmt(hstmt, SQLBindCol(hstmt, 4, SQL_C_DOUBLE, dbl, 0, NULL));
  ok_stmt(hstmt, SQLBindCol(hstmt, 5, SQL_C_CHAR, ltxt, sizeof(ltxt[0]), NULL));

  ok_sql(hstmt, "SELECT id, v, txt, ft, ltxt FROM t_bulk_insert");

  expect_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0),
              SQL_NO_DATA_FOUND);

  for (i= 0; i < MAX_INSERT_COUNT; i++)
  {
    id[i]= i;
    dbl[i]= i + dt;
    sprintf((char *)name[i], "Varchar%d", i);
    sprintf((char *)txt[i],  "Text%d", i);
    sprintf((char *)ltxt[i], "LongText, id row:%d", i);
  }

  ok_stmt(hstmt, SQLBulkOperations(hstmt, SQL_ADD));
  ok_stmt(hstmt, SQLBulkOperations(hstmt, SQL_ADD));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE,
                                (SQLPOINTER)1, 0));

  ok_sql(hstmt, "SELECT * FROM t_bulk_insert");
  is_num(myrowcount(hstmt), MAX_INSERT_COUNT * 2);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bulk_insert");

  return OK;
}


DECLARE_TEST(t_mul_pkdel)
{
  SQLINTEGER nData;
  SQLLEN nlen;
  SQLROWSETSIZE pcrow;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_mul_pkdel");
  ok_sql(hstmt, "CREATE TABLE t_mul_pkdel (a INT NOT NULL, b INT,"
         "c VARCHAR(30) NOT NULL, PRIMARY KEY(a, c))");
  ok_sql(hstmt, "INSERT INTO t_mul_pkdel VALUES (100,10,'MySQL1'),"
         "(200,20,'MySQL2'),(300,20,'MySQL3'),(400,20,'MySQL4')");

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLSetCursorName(hstmt, (SQLCHAR *)"venu", SQL_NTS));

  ok_sql(hstmt, "SELECT a, c FROM t_mul_pkdel");

  ok_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_LONG, &nData, 0, NULL));

  ok_stmt(hstmt, SQLExtendedFetch(hstmt, SQL_FETCH_NEXT, 1, &pcrow, NULL));
  ok_stmt(hstmt, SQLSetPos(hstmt, 1, SQL_POSITION, SQL_LOCK_NO_CHANGE));

  ok_stmt(hstmt, SQLSetPos(hstmt, 1, SQL_DELETE, SQL_LOCK_NO_CHANGE));
  ok_stmt(hstmt, SQLRowCount(hstmt, &nlen));
  is_num(nlen, 1);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "SELECT * FROM t_mul_pkdel");

  is_num(myrowcount(hstmt), 3);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_mul_pkdel");

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
  SQLINTEGER nData;
  SQLLEN     nLen, offset;

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

  /* Remove SQL_ATTR_ROW_BIND_OFFSET_PTR */
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_BIND_OFFSET_PTR,
                                NULL, 0));

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
  ADD_TEST(t_bulk_insert)
  ADD_TEST(t_mul_pkdel)
  ADD_TEST(t_bulk_insert_indicator)
  ADD_TEST(t_bulk_insert_rows)
END_TESTS


RUN_TESTS
