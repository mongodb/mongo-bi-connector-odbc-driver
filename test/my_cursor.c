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
    long        id;
    char        name[50];

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
    SQLRETURN   rc;
    SQLROWCOUNT nRowCount;
    long        id;
    char        name[50];

    /* Open the resultset of table 'my_demo_cursor' */
    rc = SQLExecDirect(hstmt,"SELECT * FROM my_demo_cursor",SQL_NTS);
    mystmt(hstmt,rc);

    /* bind row data buffers */
    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&id,0,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,name,sizeof(name),NULL);
    mystmt(hstmt,rc);

    /* goto the first row */
    rc = SQLFetchScroll(hstmt, SQL_FETCH_FIRST, 1L);
    mystmt(hstmt,rc);

    strcpy(name,"first-row");

    /* now update the name field to 'first-row' using SQLSetPos */
    rc = SQLSetPos(hstmt, 1, SQL_UPDATE, SQL_LOCK_NO_CHANGE);
    mystmt(hstmt, rc);

    rc = SQLRowCount(hstmt, &nRowCount);
    mystmt(hstmt, rc);

    printMessage(" total rows updated:%d\n",nRowCount);
    assert(nRowCount == 1);

    /* position to second row and delete it ..*/
    rc = SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, 2L);
    mystmt(hstmt,rc);

    /* now delete the current, second row */
    rc = SQLSetPos(hstmt, 1, SQL_DELETE, SQL_LOCK_NO_CHANGE);
    mystmt(hstmt, rc);

    rc = SQLRowCount(hstmt, &nRowCount);
    mystmt(hstmt, rc);

    printMessage(" total rows deleted:%d\n",nRowCount);
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
    rc = SQLExecDirect(hstmt, "SELECT * FROM my_demo_cursor",SQL_NTS);
    mystmt(hstmt,rc);

    assert(3 == myresult(hstmt));

    /* drop the table */
    rc = SQLExecDirect(hstmt,"DROP TABLE my_demo_cursor",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


/**
 Bug #5853: Using Update with 'WHERE CURRENT OF' with binary data crashes
*/
DECLARE_TEST(t_bug5853)
{
  SQLRETURN rc;
  SQLHSTMT  hstmt_pos;
  SQLCHAR   nData[3];
  SQLLEN    nLen= SQL_DATA_AT_EXEC;
  int       i= 0;

  rc= SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt_pos);
  mycon(hdbc, rc);

  rc= SQLExecDirect(hstmt,"DROP TABLE IF EXISTS t_bug5853",SQL_NTS);
  mystmt(hstmt,rc);

  rc= SQLExecDirect(hstmt,"CREATE TABLE t_bug5853 (id INT AUTO_INCREMENT PRIMARY KEY, a VARCHAR(3))",SQL_NTS);
  mystmt(hstmt,rc);

  rc= SQLExecDirect(hstmt,"INSERT INTO t_bug5853 (a) VALUES ('abc'),('def')",SQL_NTS);
  mystmt(hstmt,rc);

  rc= SQLPrepare(hstmt,"INSERT INTO t_bug5853 VALUES(?)",SQL_NTS);
  mystmt(hstmt,rc);

  rc= SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE,
                     (SQLPOINTER)SQL_CURSOR_DYNAMIC,0);
  mystmt(hstmt, rc);

  rc= SQLSetCursorName(hstmt, "bug5853", SQL_NTS);
  mystmt(hstmt, rc);

  rc= SQLExecDirect(hstmt,"SELECT * FROM t_bug5853",SQL_NTS);
  mystmt(hstmt,rc);

  rc= SQLPrepare(hstmt_pos,
                 "UPDATE t_bug5853 SET a = ? WHERE CURRENT OF bug5853",
                 SQL_NTS);
  mystmt(hstmt_pos, rc);

  rc= SQLBindParameter(hstmt_pos, 1, SQL_PARAM_INPUT, SQL_VARCHAR, SQL_C_CHAR,
                       0, 0, NULL, 0, &nLen);
  mystmt(hstmt_pos,rc);

  while ((rc= SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0)) != SQL_NO_DATA_FOUND)
  {
    char data[2][3] = { "uvw", "xyz" };

    mystmt(hstmt,rc);

    rc= SQLExecute(hstmt_pos);
    mystmt_err(hstmt_pos, rc == SQL_NEED_DATA, rc);

    while (rc == SQL_NEED_DATA)
    {
      SQLPOINTER token;
      rc= SQLParamData(hstmt_pos, &token);
      if (rc == SQL_NEED_DATA)
      {
        SQLRETURN rc2;
        rc2= SQLPutData(hstmt_pos, data[i++ % 2], sizeof(data[0]));
        mystmt(hstmt_pos,rc2);
      }
    }
    mystmt(hstmt_pos,rc);
  }

  rc= SQLFreeStmt(hstmt_pos, SQL_CLOSE);
  mystmt(hstmt,rc);

  rc= SQLExecDirect(hstmt,"SELECT * FROM t_bug5853",SQL_NTS);
  mystmt(hstmt,rc);

  rc= SQLBindCol(hstmt, 2, SQL_C_CHAR, nData, 3, &nLen);
  mystmt(hstmt,rc);

  rc= SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0);
  mystmt(hstmt,rc);
  my_assert(strncmp((char *)nData, "uvw", 3));

  rc= SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0);
  mystmt(hstmt,rc);
  my_assert(strncmp((char *)nData, "xyz", 3));

  rc= SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0);
  mystmt_err(hstmt, rc == SQL_NO_DATA_FOUND, rc);

  rc= SQLExecDirect(hstmt,"DROP TABLE IF EXISTS t_bug5853",SQL_NTS);
  mystmt(hstmt,rc);

  return OK;
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
END_TESTS


RUN_TESTS
