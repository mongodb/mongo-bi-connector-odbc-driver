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

/* result set demo */
DECLARE_TEST(my_resultset)
{
    SQLRETURN   rc;
    SQLUINTEGER nRowCount=0;
    SQLULEN     pcColDef;
    SQLCHAR     szColName[MAX_NAME_LEN];
    SQLCHAR     szData[MAX_ROW_DATA_LEN+1];
    SQLSMALLINT nIndex,ncol,pfSqlType, pcbScale, pfNullable;

    /* drop table 'myodbc3_demo_result' if it already exists */
    rc = SQLExecDirect(hstmt,"DROP TABLE if exists myodbc3_demo_result",SQL_NTS);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* create the table 'myodbc3_demo_result' */
    rc = SQLExecDirect(hstmt,"CREATE TABLE myodbc3_demo_result(\
                              id int primary key auto_increment,\
                              name varchar(20))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);    

    /* insert 2 rows of data */    
    rc = SQLExecDirect(hstmt,"INSERT INTO myodbc3_demo_result values(\
                              1,'MySQL')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"INSERT INTO myodbc3_demo_result values(\
                              2,'MyODBC')",SQL_NTS);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* update second row */    
    rc = SQLExecDirect(hstmt,"UPDATE myodbc3_demo_result set name=\
                              'MyODBC 3.51' where id=2",SQL_NTS);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* now fetch back..*/   
    rc = SQLExecDirect(hstmt,"SELECT * from myodbc3_demo_result",SQL_NTS);
    mystmt(hstmt,rc);

    /* get total number of columns from the resultset */
    rc = SQLNumResultCols(hstmt,&ncol);
    mystmt(hstmt,rc);

    printMessage(" total columns in resultset:%d\n\n",ncol);

    /* print the column names  and do the row bind */
    for (nIndex = 1; nIndex <= ncol; nIndex++)
    {
        rc = SQLDescribeCol(hstmt,nIndex,szColName, MAX_NAME_LEN+1, NULL,
                            &pfSqlType,&pcColDef,&pcbScale,&pfNullable);
        mystmt(hstmt,rc);

        printMessage(" %s\t",szColName);

    }

    printMessage("\n--------------------\n");

    /* now fetch row by row */
    rc = SQLFetch(hstmt);
    while (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    {
        nRowCount++;
        for (nIndex=1; nIndex<= ncol; nIndex++)
        {
            rc = SQLGetData(hstmt,nIndex, SQL_C_CHAR, szData,
                            MAX_ROW_DATA_LEN,NULL);
            mystmt(hstmt,rc);
            printMessage(" %s\t",szData);
        }

        printMessage("\n");
        rc = SQLFetch(hstmt);
    }
    SQLFreeStmt(hstmt,SQL_UNBIND);

    printMessage("\n total rows fetched:%d\n",nRowCount);

    /* free the statement row bind resources */
    rc = SQLFreeStmt(hstmt, SQL_UNBIND);
    mystmt(hstmt,rc);

    /* free the statement cursor */
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt, "DROP TABLE myodbc3_demo_result", SQL_NTS);
    mystmt(hstmt,rc);

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
  SQLULEN     pcbColDef;
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

    /*
    SQL_VARBINARY is temporarily disabled due to the problems with metadata 
    returned by the server.

    desc_col_check(hstmt, 12, "c12", SQL_VARBINARY, 12, 12, 0,  SQL_NULLABLE);
    */

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

  return OK;
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

    ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
    ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE,
                                  (SQLPOINTER)SQL_CURSOR_STATIC, 0));

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


static int colattr(SQLHSTMT hstmt, SQLUSMALLINT cno, SQLUSMALLINT attribute,
                   SQLCHAR *sptr, SQLSMALLINT slen, SQLINTEGER nptr)
{
  SQLCHAR     lsptr[40];
  SQLSMALLINT lslen;
  SQLLEN      lnptr;

  ok_stmt(hstmt, SQLFreeStmt(hstmt,SQL_CLOSE));
  ok_sql(hstmt, "SELECT * FROM t_colattr");

  ok_stmt(hstmt, SQLColAttribute(hstmt, cno, attribute, lsptr, sizeof(lsptr),
                                 &lslen, &lnptr));
  if (sptr)
  {
    is_num(lslen, slen);
    is_str(lsptr, sptr, lslen);
  }
  else
  {
    is_num(lnptr, nptr);
  }

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  return OK;
}


DECLARE_TEST(t_colattributes)
{
  ok_sql(hstmt, "DROP TABLE IF EXISTS t_colattr");

  ok_sql(hstmt,
         "CREATE TABLE t_colattr ("
         "t1 TINYINT NOT NULL AUTO_INCREMENT PRIMARY KEY,"
         "t2 TINYINT(10),"
         "t3 TINYINT UNSIGNED,"
         "s1 SMALLINT,"
         "s2 SMALLINT(10),"
         "s3 SMALLINT UNSIGNED,"
         "m1 MEDIUMINT,"
         "m2 MEDIUMINT(10),"
         "m3 MEDIUMINT UNSIGNED,"
         "i1 INT,"
         "i2 INT(10) NOT NULL,"
         "i3 INT UNSIGNED,"
         "i4 INT ZEROFILL,"
         "b1 BIGINT,"
         "b2 BIGINT(10),"
         "b3 BIGINT UNSIGNED,"
         "f1 FLOAT,"
         "f2 FLOAT(10),"
         "f3 FLOAT(24) ZEROFILL,"
         "f4 FLOAT(10,4),"
         "d1 DOUBLE,"
         "d2 DOUBLE(30,3),"
         "d3 DOUBLE PRECISION,"
         "d4 DOUBLE PRECISION(30,3),"
         "r1 REAL,"
         "r2 REAL(30,3),"
         "dc1 DECIMAL,"
         "dc2 DECIMAL(10),"
         "dc3 DECIMAL(10,3),"
         "n1 NUMERIC,"
         "n2 NUMERIC(10,3),"
         "dt DATE,"
         "dtime DATETIME,"
         "ts1 TIMESTAMP(8),"
         "ts2 TIMESTAMP(14),"
         "ti  TIME,"
         "yr1 YEAR,"
         "yr2 YEAR(2),"
         "yr3 YEAR(4),"
         "c1 CHAR(10),"
         "c2 CHAR(10) BINARY,"
         "c3 NATIONAL CHAR(10),"
         "v1 VARCHAR(10),"
         "v2 VARCHAR(10) BINARY,"
         "v3 NATIONAL VARCHAR(10),"
         "bl1 TINYBLOB,"
         "bl2 BLOB,"
         "bl3 MEDIUMBLOB,"
         "bl4 LONGBLOB,"
         "txt1 TINYTEXT,"
         "txt2 TEXT,"
         "txt3 MEDIUMTEXT,"
         "txt4 LONGTEXT,"
         "en ENUM('v1','v2'),"
         "st SET('1','2','3'))");

  if (colattr(hstmt, 1, SQL_COLUMN_COUNT, NULL, 0, 55) != OK)
    return FAIL;
  if (colattr(hstmt, 1, SQL_COLUMN_AUTO_INCREMENT, NULL, 0, SQL_TRUE) != OK)
    return FAIL;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_colattr");

  return OK;
}


DECLARE_TEST(t_exfetch)
{
    SQLRETURN rc;
    SQLUINTEGER i;

    ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE,
                                  (SQLPOINTER)SQL_CURSOR_STATIC, 0));

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


DECLARE_TEST(tmysql_rowstatus)
{
    SQLRETURN rc;
    SQLHSTMT hstmt1;
    SQLROWSETSIZE pcrow[4];
    SQLUSMALLINT rgfRowStatus[6];
    SQLINTEGER nData= 555;
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

    ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
    ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE,
                                  (SQLPOINTER)SQL_CURSOR_STATIC, 0));

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


/* TESTING FOR TRUE LENGTH */
DECLARE_TEST(t_true_length)
{
  SQLCHAR data1[25],data2[25];
  SQLLEN len1,len2;
  SQLULEN desc_len;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_true_length");
  ok_sql(hstmt, "CREATE TABLE t_true_length (a CHAR(20), b VARCHAR(15))");
  ok_sql(hstmt, "INSERT INTO t_true_length VALUES ('venu','mysql')");

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "SELECT * FROM t_true_length");

  ok_stmt(hstmt, SQLDescribeCol(hstmt, 1, NULL, 0, NULL, NULL,
                                &desc_len, NULL, NULL));
  is_num(desc_len, 20);

  ok_stmt(hstmt, SQLDescribeCol(hstmt, 2, NULL, 0, NULL, NULL,
                                &desc_len, NULL, NULL));
  is_num(desc_len, 15);

  ok_stmt(hstmt, SQLFetch(hstmt));

  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, data1, sizeof(data1),
                            &len1));
  is_str(data1, "venu", 4);
  is_num(len1, 4);

  ok_stmt(hstmt, SQLGetData(hstmt, 2, SQL_C_CHAR, data2, sizeof(data2),
                            &len2));
  is_str(data2, "mysql", 5);
  is_num(len2, 5);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_true_length");

  return OK;
}


/**
 Bug #27544: Calling stored procedure causes "Out of sync" and "Lost
connection" errors
*/
DECLARE_TEST(t_bug27544)
{
  ok_sql(hstmt, "DROP TABLE IF EXISTS t1");
  ok_sql(hstmt, "CREATE TABLE t1(a int)");
  ok_sql(hstmt, "INSERT INTO t1 VALUES (1)");

  ok_sql(hstmt, "DROP PROCEDURE IF EXISTS p1");
  ok_sql(hstmt, "CREATE PROCEDURE p1() BEGIN"
                "   SELECT a FROM t1; "
                "END;");

  ok_sql(hstmt,"CALL p1()");
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP PROCEDURE p1");
  ok_sql(hstmt, "DROP TABLE t1");

  return OK;
}


/**
 Bug #6157: BUG in the alias use with ADO's Object
*/
DECLARE_TEST(bug6157)
{
  SQLCHAR name[30];
  SQLSMALLINT len;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug6157");
  ok_sql(hstmt, "CREATE TABLE t_bug6157 (a INT)");
  ok_sql(hstmt, "INSERT INTO t_bug6157 VALUES (1)");

  ok_sql(hstmt, "SELECT a AS b FROM t_bug6157 AS c");

  ok_stmt(hstmt, SQLFetch(hstmt));

  ok_stmt(hstmt, SQLColAttribute(hstmt, 1, SQL_DESC_NAME,
                                 name, sizeof(name), &len, NULL));
  is_str(name, "b", 1);

  ok_stmt(hstmt, SQLColAttribute(hstmt, 1, SQL_DESC_BASE_COLUMN_NAME,
                                 name, sizeof(name), &len, NULL));
  is_str(name, "a", 1);

  ok_stmt(hstmt, SQLColAttribute(hstmt, 1, SQL_DESC_TABLE_NAME,
                                 name, sizeof(name), &len, NULL));
  is_str(name, "c", 1);

  ok_stmt(hstmt, SQLColAttribute(hstmt, 1, SQL_DESC_BASE_TABLE_NAME,
                                 name, sizeof(name), &len, NULL));
  is_str(name, "t_bug6157", 9);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug6157");
  return OK;
}


BEGIN_TESTS
  ADD_TEST(my_resultset)
  ADD_TEST(t_convert_type)
  ADD_TEST(t_desc_col)
  ADD_TEST(t_convert)
  ADD_TEST(t_max_rows)
  ADD_TEST(t_multistep)
  ADD_TEST(t_zerolength)
  ADD_TEST(t_cache_bug)
  ADD_TEST(t_non_cache_bug)
  ADD_TEST(t_empty_str_bug)
  ADD_TEST(t_desccol)
  ADD_TEST(t_desccolext)
  ADD_TEST(t_desccol1)
  ADD_TEST(t_colattributes)
  ADD_TEST(t_exfetch)
  ADD_TEST(tmysql_rowstatus)
  ADD_TEST(t_true_length)
  ADD_TEST(t_bug27544)
  ADD_TEST(bug6157)
END_TESTS


RUN_TESTS
