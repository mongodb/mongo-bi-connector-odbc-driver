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


/* initialize tables */
DECLARE_TEST(my_init_table)
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

  return OK;
}


/* perform positioned update and delete */
DECLARE_TEST(my_positioned_cursor)
{
    SQLRETURN   rc;
    SQLROWCOUNT nRowCount;
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

  return OK;
}


/* perform delete and update using SQLSetPos */
DECLARE_TEST(my_setpos_cursor)
{
  SQLLEN      nRowCount;
  SQLINTEGER  id;
  SQLCHAR     name[50];

  ok_sql(hstmt, "SELECT * FROM my_demo_cursor");

  ok_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_LONG, &id, 0, NULL));
  ok_stmt(hstmt, SQLBindCol(hstmt, 2, SQL_C_CHAR, name, sizeof(name),NULL));

  ok_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_FIRST, 1L));

  strcpy((char *)name, "first-row");

  /* now update the name field to 'first-row' using SQLSetPos */
  ok_stmt(hstmt, SQLSetPos(hstmt, 1, SQL_UPDATE, SQL_LOCK_NO_CHANGE));

  ok_stmt(hstmt, SQLRowCount(hstmt, &nRowCount));
  is_num(nRowCount, 1);

  /* position to second row and delete it ..*/
  ok_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, 2L));

  /* now delete the current, second row */
  ok_stmt(hstmt, SQLSetPos(hstmt, 1, SQL_DELETE, SQL_LOCK_NO_CHANGE));

  ok_stmt(hstmt, SQLRowCount(hstmt, &nRowCount));
  is_num(nRowCount, 1);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  /* Now fetch and verify the data */
  ok_sql(hstmt, "SELECT * FROM my_demo_cursor");

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_num(my_fetch_int(hstmt, 1), 0);
  is_str(my_fetch_str(hstmt, name, 2), "first-row", 9);

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_num(my_fetch_int(hstmt, 1), 3);
  is_str(my_fetch_str(hstmt, name, 2), "MySQL3", 6);

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_num(my_fetch_int(hstmt, 1), 4);
  is_str(my_fetch_str(hstmt, name, 2), "updated", 7);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE my_demo_cursor");

  return OK;
}


/**
 Bug #5853: Using Update with 'WHERE CURRENT OF' with binary data crashes
*/
DECLARE_TEST(t_bug5853)
{
  SQLRETURN rc;
  SQLHSTMT  hstmt_pos;
  SQLCHAR   nData[4];
  SQLLEN    nLen= SQL_DATA_AT_EXEC;
  int       i= 0;

  ok_con(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt_pos));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug5853");

  ok_sql(hstmt,"CREATE TABLE t_bug5853 (id INT AUTO_INCREMENT PRIMARY KEY, a VARCHAR(3))");

  ok_sql(hstmt,"INSERT INTO t_bug5853 (a) VALUES ('abc'),('def')");

  ok_stmt(hstmt_pos,
          SQLPrepare(hstmt_pos, (SQLCHAR *)
                     "UPDATE t_bug5853 SET a = ? WHERE CURRENT OF bug5853",
                     SQL_NTS));

  ok_stmt(hstmt_pos, SQLBindParameter(hstmt_pos, 1, SQL_PARAM_INPUT,
                                      SQL_VARCHAR, SQL_C_CHAR, 0, 0, NULL,
                                      SQL_DATA_AT_EXEC, &nLen));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE,
                                (SQLPOINTER)SQL_CURSOR_DYNAMIC,0));

  ok_stmt(hstmt, SQLSetCursorName(hstmt, (SQLCHAR *)"bug5853", SQL_NTS));

  ok_sql(hstmt, "SELECT * FROM t_bug5853");

  while ((rc= SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0)) != SQL_NO_DATA_FOUND)
  {
    char data[2][3] = { "uvw", "xyz" };

    expect_stmt(hstmt_pos, SQLExecute(hstmt_pos), SQL_NEED_DATA);
    rc= SQL_NEED_DATA;

    while (rc == SQL_NEED_DATA)
    {
      SQLPOINTER token;
      rc= SQLParamData(hstmt_pos, &token);
      if (rc == SQL_NEED_DATA)
      {
        ok_stmt(hstmt_pos, SQLPutData(hstmt_pos, data[i++ % 2],
                                      sizeof(data[0])));
      }
    }
  }

  ok_stmt(hstmt_pos, SQLFreeStmt(hstmt_pos, SQL_CLOSE));

  ok_sql(hstmt,"SELECT * FROM t_bug5853");

  ok_stmt(hstmt, SQLBindCol(hstmt, 2, SQL_C_CHAR, nData, sizeof(nData), &nLen));

  ok_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0));
  is_str(nData, "uvw", 3);

  ok_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0));
  is_str(nData, "xyz", 3);

  expect_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0),
              SQL_NO_DATA_FOUND);

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug5853");

  return OK;
}


DECLARE_TEST(t_setpos_del_all)
{
  SQLRETURN rc;
  SQLINTEGER nData[4];
  SQLLEN nlen;
  SQLCHAR szData[4][10];
  SQLUSMALLINT rgfRowStatus;

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

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,0,NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_FIRST,1,NULL,&rgfRowStatus);
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
  SQLUSMALLINT rgfRowStatus;

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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,NULL,&rgfRowStatus);
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


DECLARE_TEST(t_setpos_position)
{
  SQLRETURN rc;
  SQLINTEGER nData= 500;
  SQLLEN nlen;
  SQLCHAR szData[255]={0};
  SQLUINTEGER pcrow;
  SQLUSMALLINT rgfRowStatus;

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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,&rgfRowStatus);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,&rgfRowStatus);
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
  SQLCHAR szData[20];
  SQLINTEGER nData;
  SQLLEN  pcbValue, nlen, pcrow;
  SQLUSMALLINT rgfRowStatus;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_pos_column_ignore");
  ok_sql(hstmt,
         "CREATE TABLE t_pos_column_ignore "
         "(col1 INT NOT NULL PRIMARY KEY, col2 VARCHAR(30))");
  ok_sql(hstmt,
         "INSERT INTO t_pos_column_ignore VALUES (10,'venu'),(100,'MySQL')");

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY,
                                (SQLPOINTER)SQL_CONCUR_ROWVER, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE,
                                (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, 0));

  /* ignore all columns */
  ok_sql(hstmt, "SELECT * FROM t_pos_column_ignore ORDER BY col1 ASC");

  ok_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_LONG, &nData, 0, &pcbValue));
  ok_stmt(hstmt, SQLBindCol(hstmt, 2, SQL_C_CHAR, szData, sizeof(szData),
                            &pcbValue));

  ok_stmt(hstmt, SQLExtendedFetch(hstmt, SQL_FETCH_NEXT, 1, &pcrow,
                                  &rgfRowStatus));

  ok_stmt(hstmt, SQLSetPos(hstmt, 1, SQL_POSITION, SQL_LOCK_NO_CHANGE));

  nData= 99;
  strcpy((char *)szData , "updated");

  pcbValue= SQL_COLUMN_IGNORE;
  ok_stmt(hstmt, SQLSetPos(hstmt, 1, SQL_UPDATE, SQL_LOCK_NO_CHANGE));

  ok_stmt(hstmt, SQLRowCount(hstmt, &nlen));
  is_num(nlen, 0);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "SELECT * FROM t_pos_column_ignore ORDER BY col1 ASC");

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_num(my_fetch_int(hstmt, 1), 10);
  is_str(my_fetch_str(hstmt, szData, 2), "venu", 4);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  /* ignore only one column */
  ok_sql(hstmt, "SELECT * FROM t_pos_column_ignore ORDER BY col1 ASC");

  ok_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_LONG, &nData, 0, NULL));
  ok_stmt(hstmt, SQLBindCol(hstmt, 2, SQL_C_CHAR, szData, sizeof(szData),
                            &pcbValue));

  ok_stmt(hstmt, SQLExtendedFetch(hstmt, SQL_FETCH_NEXT, 1, &pcrow,
                                  &rgfRowStatus));

  ok_stmt(hstmt, SQLSetPos(hstmt, 1, SQL_POSITION, SQL_LOCK_NO_CHANGE));

  nData= 99;
  strcpy((char *)szData , "updated");

  pcbValue= SQL_COLUMN_IGNORE;
  ok_stmt(hstmt, SQLSetPos(hstmt, 1, SQL_UPDATE, SQL_LOCK_NO_CHANGE));

  ok_stmt(hstmt, SQLRowCount(hstmt, &nlen));
  is_num(nlen, 1);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "SELECT * FROM t_pos_column_ignore ORDER BY col1 ASC");

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_num(my_fetch_int(hstmt, 1), 99);
  is_str(my_fetch_str(hstmt, szData, 2), "venu", 4);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_pos_column_ignore");

  return OK;
}


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
  fprintf(stdout,"\n obtained: `%s` (expected: `%s`)\n", data, exp_data);
  myassert(strcmp(data,exp_data) == 0);

  SQLFreeStmt(hstmt, SQL_UNBIND);
  SQLFreeStmt(hstmt, SQL_CLOSE);
}


DECLARE_TEST(t_pos_datetime_delete)
{
  SQLRETURN rc;
  SQLHSTMT  hstmt1;
  SQLINTEGER int_data;
  SQLLEN    row_count, cur_type;
  SQLUSMALLINT rgfRowStatus;

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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,NULL,&rgfRowStatus);
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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,NULL,&rgfRowStatus);
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
  SQLUSMALLINT rgfRowStatus;

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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,3,NULL,&rgfRowStatus);
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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,NULL,&rgfRowStatus);
    mystmt(hstmt,rc);
    fprintf(stdout,"current_row: %d\n", int_data);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,NULL,&rgfRowStatus);
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
  SQLSMALLINT nlen,index;

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

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,name,20,NULL);
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


DECLARE_TEST(tmysql_setpos_del)
{
    SQLRETURN rc;
    SQLINTEGER nData = 500;
    SQLUINTEGER nlen;
    SQLCHAR szData[255]={0};
    SQLUINTEGER pcrow;
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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,5,&pcrow,&rgfRowStatus);
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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,5,NULL,&rgfRowStatus);
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
    SQLINTEGER nData= 500;
    SQLLEN nlen;
    SQLCHAR szData[255]={0};
    SQLUINTEGER pcrow;
    SQLUSMALLINT rgfRowStatus;

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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,3,&pcrow,&rgfRowStatus);
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
    SQLINTEGER nData = 500;
    SQLLEN nlen;
    SQLCHAR szData[255]={0};
    SQLUINTEGER pcrow;
    SQLUSMALLINT rgfRowStatus;

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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,3,&pcrow,&rgfRowStatus);
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
    SQLINTEGER nData= 500;
    SQLLEN nlen;
    SQLCHAR szData[255]={0};
    SQLUINTEGER pcrow;
    SQLUSMALLINT rgfRowStatus;

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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,&rgfRowStatus);
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


DECLARE_TEST(tmysql_pos_delete)
{
    SQLRETURN rc;
    SQLHSTMT hstmt1;
    SQLUINTEGER pcrow;
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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,&rgfRowStatus);
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


DECLARE_TEST(t_pos_update)
{
  SQLHSTMT hstmt1;
  SQLCHAR  szData[10];

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_pos_update");
  ok_sql(hstmt, "CREATE TABLE t_pos_update (col1 INT, col2 VARCHAR(30))");
  ok_sql(hstmt, "INSERT INTO t_pos_update VALUES (100,'venu'),(200,'MySQL')");

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLSetCursorName(hstmt, (SQLCHAR *)"venu_cur", SQL_NTS));

  ok_sql(hstmt, "SELECT * FROM t_pos_update");

  ok_stmt(hstmt, SQLExtendedFetch(hstmt, SQL_FETCH_ABSOLUTE, 2, NULL, NULL));

  ok_stmt(hstmt, SQLSetPos(hstmt, 1, SQL_POSITION, SQL_LOCK_NO_CHANGE));

  ok_con(hdbc, SQLAllocStmt(hdbc, &hstmt1));

  expect_sql(hstmt1,
             "  UPerrDATE t_pos_update SET col1 = 999, col2 = 'update' "
             "WHERE CURRENT OF venu_cur",
             SQL_ERROR);

  expect_sql(hstmt1,
             "  UPDATE t_pos_update SET col1 = 999, col2 = 'update' "
             "WHERE CURRENT OF",
             SQL_ERROR);

  ok_sql(hstmt1,
         "  UPDATE t_pos_update SET col1 = 999, col2 = 'update' "
         "WHERE CURRENT OF venu_cur");

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));
  ok_stmt(hstmt1, SQLFreeHandle(SQL_HANDLE_STMT, hstmt1));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "SELECT * FROM t_pos_update");

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_num(my_fetch_int(hstmt, 1), 100);
  is_str(my_fetch_str(hstmt, szData, 2), "venu", 4);

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_num(my_fetch_int(hstmt, 1), 999);
  is_str(my_fetch_str(hstmt, szData, 2), "update", 5);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_pos_update");

  return OK;
}


DECLARE_TEST(tmysql_pos_update_ex)
{
    SQLRETURN rc;
    SQLHSTMT hstmt1;
    SQLUINTEGER pcrow;
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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,2,&pcrow,&rgfRowStatus);
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
    SQLUINTEGER pcrow;
    SQLSMALLINT rgfRowStatus;
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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,2,&pcrow,&rgfRowStatus);
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
    SQLUINTEGER pcrow;
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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,2,&pcrow,&rgfRowStatus);
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
    SQLUINTEGER pcrow;
    SQLCHAR cursor[30],sql[100];
    SQLUSMALLINT rgfRowStatus;

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


    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,2,&pcrow,&rgfRowStatus);
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
    SQLUINTEGER pcrow;
    SQLCHAR cursor[30],sql[100],data[]="venu";
    SQLUSMALLINT rgfRowStatus;

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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,1,&pcrow,&rgfRowStatus);
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
    SQLUINTEGER pcrow;
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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,&rgfRowStatus);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,&rgfRowStatus);
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
    SQLINTEGER nData= 500;
    SQLLEN nlen;
    SQLCHAR szData[255]={0};
    SQLUINTEGER pcrow;
    SQLUSMALLINT rgfRowStatus;

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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,3,&pcrow,&rgfRowStatus);
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


DECLARE_TEST(tmysql_setpos_pkdel)
{
    SQLRETURN rc;
    SQLINTEGER nData= 500;
    SQLLEN nlen;
    SQLCHAR szData[255]={0};
    SQLUINTEGER pcrow;
    SQLUSMALLINT rgfRowStatus;

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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,4,&pcrow,&rgfRowStatus);
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
  SQLINTEGER nData= 500;
  SQLLEN nlen;
  SQLCHAR szData[255]= {0};
  SQLUINTEGER pcrow;
  SQLUSMALLINT rgfRowStatus;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_alias_setpos_pkdel");

  ok_con(hdbc, SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT));

  ok_sql(hstmt,
         "CREATE TABLE t_alias_setpos_pkdel (col1 INT PRIMARY KEY,"
        " col2 VARCHAR(30))");

  ok_sql(hstmt, "INSERT INTO t_alias_setpos_pkdel VALUES (100, 'MySQL1')");
  ok_sql(hstmt, "INSERT INTO t_alias_setpos_pkdel VALUES (200, 'MySQL2')");
  ok_sql(hstmt, "INSERT INTO t_alias_setpos_pkdel VALUES (300, 'MySQL3')");
  ok_sql(hstmt, "INSERT INTO t_alias_setpos_pkdel VALUES (400, 'MySQL4')");

  ok_con(hdbc, SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLSetCursorName(hstmt, "venu", SQL_NTS));

  ok_sql(hstmt,"SELECT col1 AS id, col2 AS name FROM t_alias_setpos_pkdel");

  ok_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_LONG, &nData, 0, NULL));
  ok_stmt(hstmt, SQLBindCol(hstmt, 2, SQL_C_CHAR, szData, sizeof(szData),
                            &nlen));

  ok_stmt(hstmt, SQLExtendedFetch(hstmt, SQL_FETCH_ABSOLUTE, 1,
                                  &pcrow, &rgfRowStatus));

  printMessage("pcrow:%d, rgfRowStatus:%d", pcrow, rgfRowStatus);
  printMessage(" row1:%d, %s", nData, szData);

  ok_stmt(hstmt, SQLSetPos(hstmt, 1, SQL_POSITION, SQL_LOCK_NO_CHANGE));
  ok_stmt(hstmt, SQLSetPos(hstmt, 1, SQL_DELETE, SQL_LOCK_NO_CHANGE));

  ok_stmt(hstmt, SQLRowCount(hstmt, &nlen));

  printMessage("rows affected:%d",nlen);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "SELECT * FROM t_alias_setpos_pkdel");

  my_assert(3 == myresult(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_alias_setpos_pkdel");

  return OK;
}


DECLARE_TEST(t_alias_setpos_del)
{
  SQLINTEGER nData= 500;
  SQLLEN nlen;
  SQLCHAR szData[255]= {0};
  SQLUINTEGER pcrow;
  SQLUSMALLINT rgfRowStatus;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_alias_setpos_del");

  ok_con(hdbc, SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT));

  ok_sql(hstmt, "CREATE TABLE t_alias_setpos_del (col1 INT, col2 VARCHAR(30))");

  ok_sql(hstmt, "INSERT INTO t_alias_setpos_del VALUES (100, 'MySQL1')");
  ok_sql(hstmt, "INSERT INTO t_alias_setpos_del VALUES (200, 'MySQL2')");
  ok_sql(hstmt, "INSERT INTO t_alias_setpos_del VALUES (300, 'MySQL3')");
  ok_sql(hstmt, "INSERT INTO t_alias_setpos_del VALUES (400, 'MySQL4')");

  ok_con(hdbc, SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLSetCursorName(hstmt, "venu", SQL_NTS));

  ok_sql(hstmt,"SELECT col1 AS id, col2 AS name FROM t_alias_setpos_del");

  ok_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_LONG, &nData, 0, NULL));
  ok_stmt(hstmt, SQLBindCol(hstmt, 2, SQL_C_CHAR, szData, sizeof(szData),
                            &nlen));

  ok_stmt(hstmt, SQLExtendedFetch(hstmt, SQL_FETCH_ABSOLUTE, 1,
                                  &pcrow, &rgfRowStatus));

  printMessage("pcrow:%d, rgfRowStatus:%d", pcrow, rgfRowStatus);
  printMessage(" row1:%d, %s", nData, szData);

  ok_stmt(hstmt, SQLSetPos(hstmt, 1, SQL_POSITION, SQL_LOCK_NO_CHANGE));
  ok_stmt(hstmt, SQLSetPos(hstmt, 1, SQL_DELETE, SQL_LOCK_NO_CHANGE));

  ok_stmt(hstmt, SQLRowCount(hstmt, &nlen));

  printMessage("rows affected:%d",nlen);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "SELECT * FROM t_alias_setpos_del");

  my_assert(3 == myresult(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_alias_setpos_del");

  return OK;
}


DECLARE_TEST(tmysql_setpos_pkdel1)
{
    SQLRETURN rc;
    SQLINTEGER nData= 500;
    SQLLEN nlen;
    SQLCHAR szData[255]={0};
    SQLUINTEGER pcrow;
    SQLUSMALLINT rgfRowStatus;

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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,4,&pcrow,&rgfRowStatus);
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
    SQLINTEGER nData= 500;
    SQLLEN nlen;
    SQLCHAR szData[255]={0};
    SQLUINTEGER pcrow;
    SQLUSMALLINT rgfRowStatus;

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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,4,&pcrow,&rgfRowStatus);
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


DECLARE_TEST(tmysql_setpos_pkdel3)
{
    SQLRETURN rc;
    SQLINTEGER nData= 500;
    SQLLEN nlen;
    SQLCHAR szData[255]={0};
    SQLUINTEGER pcrow;
    SQLUSMALLINT rgfRowStatus;

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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,4,&pcrow,&rgfRowStatus);
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


DECLARE_TEST(t_setpos_upd_bug1)
{
    SQLRETURN rc;
    SQLINTEGER id;
    SQLLEN len,id_len,f_len,l_len,ts_len;
    SQLCHAR fname[21],lname[21],ts[17],szTable[256];
    SQLSMALLINT pccol;
    SQLUSMALLINT rgfRowStatus;

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

    rc = SQLNumResultCols(hstmt,&pccol);
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

    rc = SQLColAttributes(hstmt,1,SQL_COLUMN_TABLE_NAME,szTable,sizeof(szTable),NULL,NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_FIRST,0,NULL,&rgfRowStatus);
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


DECLARE_TEST(my_setpos_upd_pk_order)
{
    SQLRETURN rc;
    SQLINTEGER nData= 500;
    SQLLEN nlen;
    SQLCHAR szData[255]={0};
    SQLUINTEGER pcrow;
    SQLUSMALLINT rgfRowStatus;

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

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,0,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,sizeof(szData),NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,2,&pcrow,&rgfRowStatus);
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
    SQLINTEGER nData= 500;
    SQLCHAR szData[255]={0};
    SQLUINTEGER pcrow;
    SQLUSMALLINT rgfRowStatus;

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

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,&rgfRowStatus);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_NEXT,1,&pcrow,&rgfRowStatus);
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


static int tmy_cursor(SQLHSTMT hstmt, char *setCurName,
                      SQLCHAR *getCurName, SQLSMALLINT setLen)
{
  SQLSMALLINT getLen;

  ok_stmt(hstmt, SQLSetCursorName(hstmt, (SQLCHAR *)setCurName, setLen));
  ok_stmt(hstmt, SQLGetCursorName(hstmt, getCurName, 20, &getLen));

  return OK;
}


DECLARE_TEST(tmy_cursor1)
{
  SQLCHAR getCurName[20];

  nok_pass_on(tmy_cursor(hstmt,"MYSQL", getCurName, 5));
  is_str(getCurName, "MYSQL", 5);

  nok_pass_on(tmy_cursor(hstmt,"MYSQL", getCurName, 10));
  is_str(getCurName, "MYSQL", 5);

  nok_pass_on(tmy_cursor(hstmt,"MYSQL", getCurName, 2));
  is_str(getCurName, "MY", 2);

  return OK;
}


DECLARE_TEST(tmy_cursor2)
{
  SQLCHAR     getCursor[50]= {0};
  SQLSMALLINT getLen;

  ok_stmt(hstmt, SQLSetCursorName(hstmt, (SQLCHAR *)"MYODBC", 6));

  expect_stmt(hstmt, SQLGetCursorName(hstmt, getCursor, 0, &getLen),
              SQL_SUCCESS_WITH_INFO);
  is_str(getCursor, "", 1);
  is_num(getLen, 6);

  expect_stmt(hstmt, SQLGetCursorName(hstmt, getCursor, -1, &getLen),
              SQL_ERROR);

  expect_stmt(hstmt, SQLGetCursorName(hstmt, getCursor, 4, &getLen),
              SQL_SUCCESS_WITH_INFO);
  is_str(getCursor, "MYO", 4);
  is_num(getLen, 6);

  expect_stmt(hstmt, SQLGetCursorName(hstmt, getCursor, 6, &getLen),
              SQL_SUCCESS_WITH_INFO);
  is_str(getCursor, "MYODB", 6);
  is_num(getLen, 6);

  ok_stmt(hstmt, SQLGetCursorName(hstmt, getCursor, 7, &getLen));
  is_str(getCursor, "MYODBC", 7);
  is_num(getLen, 6);

  return OK;
}


DECLARE_TEST(tmy_cursor3)
{
  SQLCHAR     getCursor[50];
  SQLSMALLINT getLen= -1;
  SQLHSTMT    hstmt1;

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLSetCursorName(hstmt, (SQLCHAR *)"MYSQLODBC", 9));

  /* New statement should get its own (generated) cursor name. */
  ok_con(hdbc, SQLAllocStmt(hdbc, &hstmt1));
  ok_stmt(hstmt1, SQLGetCursorName(hstmt1, getCursor, 20, &getLen));
  is_str(getCursor, "SQL_CUR", 7);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));

  return OK;
}


DECLARE_TEST(tmysql_pcbvalue)
{
  SQLCHAR    szdata[20], sztdata[100];
  SQLINTEGER nodata;
  SQLLEN     nlen, slen, tlen;
  SQLUSMALLINT rgfRowStatus[20];

  ok_sql(hstmt, "DROP TABLE IF EXISTS tmysql_pcbvalue");

  ok_sql(hstmt,
         "CREATE TABLE tmysql_pcbvalue (col1 INT PRIMARY KEY,"
         "                              col2 VARCHAR(1), col3 TEXT)");
  ok_sql(hstmt, "INSERT INTO tmysql_pcbvalue VALUES (100,'venu','mysql')");
  ok_sql(hstmt, "INSERT INTO tmysql_pcbvalue VALUES (200,'monty','mysql')");

  ok_con(hdbc, SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT));

  ok_stmt(hstmt, SQLFreeStmt(hstmt,SQL_CLOSE));

#if 0
  /* MS SQL Server to work...*/
  SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY,
                 (SQLPOINTER)SQL_CONCUR_ROWVER, 0);
  SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE,
                 (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, 0);
#endif

  ok_sql(hstmt,"SELECT * FROM tmysql_pcbvalue");

  ok_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_LONG, &nodata, 0, &nlen));
  ok_stmt(hstmt, SQLBindCol(hstmt, 2, SQL_C_CHAR, szdata, sizeof(szdata),
                            &slen));
  ok_stmt(hstmt, SQLBindCol(hstmt, 3, SQL_C_CHAR, sztdata, sizeof(sztdata),
                            &tlen));

  ok_stmt(hstmt, SQLExtendedFetch(hstmt, SQL_FETCH_FIRST, 1, NULL,
                                  rgfRowStatus));

  printMessage("row1: %d(%d), %s(%d),%s(%d)\n",
               nodata, nlen, szdata, slen, sztdata, tlen);

  strcpy((char *)szdata, "updated-one");

  ok_stmt(hstmt, SQLSetPos(hstmt, 1, SQL_UPDATE, SQL_LOCK_NO_CHANGE));

  ok_stmt(hstmt, SQLExtendedFetch(hstmt, SQL_FETCH_NEXT, 1, NULL, rgfRowStatus));

  printMessage("row2: %d(%d), %s(%d),%s(%d)\n",
               nodata, nlen, szdata, slen, sztdata, tlen);

  expect_stmt(hstmt, SQLExtendedFetch(hstmt, SQL_FETCH_NEXT, 1, NULL,
                                      rgfRowStatus),
              SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_con(hdbc, SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT));

  ok_sql(hstmt, "SELECT * FROM tmysql_pcbvalue");

  ok_stmt(hstmt, SQLFetch(hstmt));

  ok_stmt(hstmt, SQLGetData(hstmt, 2, SQL_C_CHAR, szdata, sizeof(szdata),
                            &slen));

  printMessage("updated data:%s(%d)\n",szdata,slen);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS tmysql_pcbvalue");

  ok_con(hdbc, SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT));

  return OK;
}


BEGIN_TESTS
  ADD_TEST(my_init_table)
  ADD_TEST(my_positioned_cursor)
  ADD_TEST(my_setpos_cursor)
  ADD_TEST(t_bug5853)
  ADD_TEST(t_setpos_del_all)
  ADD_TEST(t_setpos_upd_decimal)
  ADD_TEST(t_setpos_position)
  ADD_TEST(t_pos_column_ignore)
  ADD_TEST(t_pos_datetime_delete)
  ADD_TEST(t_pos_datetime_delete1)
  ADD_TEST(t_getcursor)
  ADD_TEST(t_getcursor1)
  ADD_TEST(t_acc_crash)
  ADD_TEST(tmysql_setpos_del)
  ADD_TEST(tmysql_setpos_del1)
  ADD_TEST(tmysql_setpos_upd)
  ADD_TEST(tmysql_setpos_add)
  ADD_TEST(tmysql_pos_delete)
  ADD_TEST(t_pos_update)
  ADD_TEST(tmysql_pos_update_ex)
  ADD_TEST(tmysql_pos_update_ex1)
  ADD_TEST(tmysql_pos_update_ex2)
  ADD_TEST(tmysql_pos_update_ex3)
  ADD_TEST(tmysql_pos_update_ex4)
  ADD_TEST(tmysql_pos_dyncursor)
  ADD_TEST(tmysql_mtab_setpos_del)
  ADD_TEST(tmysql_setpos_pkdel)
  ADD_TEST(tmysql_setpos_pkdel1)
  ADD_TEST(tmysql_setpos_pkdel2)
  ADD_TEST(tmysql_setpos_pkdel3)
  ADD_TEST(t_alias_setpos_pkdel)
  ADD_TEST(t_alias_setpos_del)
  ADD_TEST(t_setpos_upd_bug1)
  ADD_TEST(my_setpos_upd_pk_order)
  ADD_TEST(my_setpos_upd_pk_order1)
  ADD_TEST(tmy_cursor1)
  ADD_TEST(tmy_cursor2)
  ADD_TEST(tmy_cursor3)
  ADD_TEST(tmysql_pcbvalue)
END_TESTS


RUN_TESTS
