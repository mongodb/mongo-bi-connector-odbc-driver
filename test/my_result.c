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
    ok_sql(hstmt, "DROP TABLE if exists myodbc3_demo_result");

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);

    /* create the table 'myodbc3_demo_result' */
    ok_sql(hstmt,"CREATE TABLE myodbc3_demo_result("
           "id int primary key auto_increment,name varchar(20))");

    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);

    /* insert 2 rows of data */
    ok_sql(hstmt, "INSERT INTO myodbc3_demo_result values(1,'MySQL')");
    ok_sql(hstmt, "INSERT INTO myodbc3_demo_result values(2,'MyODBC')");

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);

    /* update second row */
    ok_sql(hstmt, "UPDATE myodbc3_demo_result set name="
           "'MyODBC 3.51' where id=2");

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);

    /* now fetch back..*/
    ok_sql(hstmt, "SELECT * from myodbc3_demo_result");

    /* get total number of columns from the resultset */
    rc = SQLNumResultCols(hstmt,&ncol);
    mystmt(hstmt,rc);

    printMessage("total columns in resultset:%d",ncol);

    /* print the column names  and do the row bind */
    for (nIndex = 1; nIndex <= ncol; nIndex++)
    {
        rc = SQLDescribeCol(hstmt,nIndex,szColName, MAX_NAME_LEN+1, NULL,
                            &pfSqlType,&pcColDef,&pcbScale,&pfNullable);
        mystmt(hstmt,rc);

        printf("%s\t",szColName);

    }
    printf("\n");

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
            printf("%s\t",szData);
        }

        printf("\n");
        rc = SQLFetch(hstmt);
    }
    SQLFreeStmt(hstmt,SQL_UNBIND);

    printMessage("total rows fetched:%d",nRowCount);

    /* free the statement row bind resources */
    rc = SQLFreeStmt(hstmt, SQL_UNBIND);
    mystmt(hstmt,rc);

    /* free the statement cursor */
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

  ok_sql(hstmt, "DROP TABLE myodbc3_demo_result");

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

    rc = SQLGetEnvAttr(henv,SQL_ATTR_ODBC_VERSION,&OdbcVersion,0,NULL);
    myenv(henv,rc);

    fprintf(stdout,"# odbc version:");
    if (OdbcVersion == SQL_OV_ODBC2)
    {
      fprintf(stdout," SQL_OV_ODBC2\n");
      DateType= SQL_DATE;
    }
    else
    {
      fprintf(stdout," SQL_OV_ODBC3\n");
      DateType= SQL_TYPE_DATE;
    }

    rc = SQLGetInfo(hdbc,SQL_DBMS_VER,DbVersion,MAX_NAME_LEN,NULL);
    mycon(hdbc,rc);

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_convert");

  ok_sql(hstmt, "CREATE TABLE t_convert(col0 int, col1 date, col2 char(10))");

  ok_sql(hstmt, "INSERT INTO t_convert VALUES(10,'2002-10-24','venu')");
  ok_sql(hstmt, "INSERT INTO t_convert VALUES(20,'2002-10-23','venu1')");
  ok_sql(hstmt, "INSERT INTO t_convert VALUES(30,'2002-10-25','venu2')");
  ok_sql(hstmt, "INSERT INTO t_convert VALUES(40,'2002-10-24','venu3')");

    ok_sql(hstmt, "SELECT MAX(col0) FROM t_convert");

    rc = SQLDescribeCol(hstmt,1,ColName,MAX_NAME_LEN,NULL,&SqlType,NULL,NULL,NULL);
    mystmt(hstmt,rc);

    printMessage("MAX(col0): %d", SqlType);
    myassert(SqlType == SQL_INTEGER);

    SQLFreeStmt(hstmt,SQL_CLOSE);

    ok_sql(hstmt, "SELECT MAX(col1) FROM t_convert");

    rc = SQLDescribeCol(hstmt,1,ColName,MAX_NAME_LEN,NULL,&SqlType,NULL,NULL,NULL);
    mystmt(hstmt,rc);

    printMessage("MAX(col1): %d", SqlType);
    myassert(SqlType == DateType);

    SQLFreeStmt(hstmt,SQL_CLOSE);

    ok_sql(hstmt, "SELECT MAX(col2) FROM t_convert");

    rc = SQLDescribeCol(hstmt,1,ColName,MAX_NAME_LEN,NULL,&SqlType,NULL,NULL,NULL);
    mystmt(hstmt,rc);

    printMessage("MAX(col0): %d", SqlType);

    SQLFreeStmt(hstmt,SQL_CLOSE);

    if (strncmp((char *)DbVersion,"4.",2) >= 0)
    {
      ok_sql(hstmt, "SELECT CAST(MAX(col1) AS DATE) AS col1 FROM t_convert");
      mystmt(hstmt,rc);

      rc = SQLDescribeCol(hstmt,1,ColName,MAX_NAME_LEN,NULL,&SqlType,NULL,NULL,NULL);
      mystmt(hstmt,rc);

      printMessage("CAST(MAX(col1) AS DATE): %d", SqlType);
      myassert(SqlType == DateType);

      SQLFreeStmt(hstmt,SQL_CLOSE);

      ok_sql(hstmt, "SELECT CONVERT(MAX(col1),DATE) AS col1 FROM t_convert");
      mystmt(hstmt,rc);

      rc = SQLDescribeCol(hstmt,1,ColName,MAX_NAME_LEN,NULL,&SqlType,NULL,NULL,NULL);
      mystmt(hstmt,rc);

      printMessage("CONVERT(MAX(col1),DATE): %d", SqlType);
      myassert(SqlType == DateType);

      SQLFreeStmt(hstmt,SQL_CLOSE);

      ok_sql(hstmt,"SELECT CAST(MAX(col1) AS CHAR) AS col1 FROM t_convert");

      rc = SQLDescribeCol(hstmt,1,(SQLCHAR *)&ColName,MAX_NAME_LEN,NULL,&SqlType,NULL,NULL,NULL);
      mystmt(hstmt,rc);

      printMessage("CAST(MAX(col1) AS CHAR): %d", SqlType);
      myassert(SqlType == SQL_VARCHAR || SqlType == SQL_LONGVARCHAR);

      SQLFreeStmt(hstmt,SQL_CLOSE);
    }

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_convert");

  return OK;
}


static SQLINTEGER desc_col_check(SQLHSTMT hstmt,
                           SQLUSMALLINT icol,
                           const char *name,
                           SQLSMALLINT sql_type,
                           SQLULEN col_def,
                           SQLULEN col_def1,
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

  printMessage("Column Number'%d':", icol);

  printMessage(" Column Name    : %s", szColName);
  printMessage(" NameLengh      : %d", pcbColName);
  printMessage(" DataType       : %d", pfSqlType);
  printMessage(" ColumnSize     : %d", pcbColDef);
  printMessage(" DecimalDigits  : %d", pibScale);
  printMessage(" Nullable       : %d", pfNullable);

  is_str(szColName, name, pcbColName);
  is_num(pfSqlType, sql_type);
  is(col_def == pcbColDef || col_def1 == pcbColDef);
  is_num(pibScale, scale);
  is_num(pfNullable, nullable);

  return OK;
}


/* To test SQLDescribeCol */
DECLARE_TEST(t_desc_col)
{
  SQLSMALLINT ColumnCount;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_desc_col");

  ok_sql(hstmt, "CREATE TABLE t_desc_col("
         "c1 integer,"
         "c2 binary(2) NOT NULL,"
         "c3 char(1),"
         "c4 varchar(5),"
         "c5 decimal(10,3) NOT NULL,"
         "c6 tinyint,"
         "c7 smallint,"
         "c8 numeric(4,2),"
         "c9 real,"
         "c10 float(5),"
         "c11 bigint NOT NULL,"
         "c12 varbinary(12),"
         "c13 char(20) NOT NULL,"
         "c14 float(10,3),"
         "c15 tinytext,"
         "c16 text,"
         "c17 mediumtext,"
         "c18 longtext,"
         "c19 tinyblob,"
         "c20 blob,"
         "c21 mediumblob,"
         "c22 longblob,"
         "c23 tinyblob)");

  ok_sql(hstmt, "SELECT * FROM t_desc_col");

  ok_stmt(hstmt, SQLNumResultCols(hstmt, &ColumnCount));

  is_num(ColumnCount, 23);

  is(desc_col_check(hstmt, 1,  "c1",  SQL_INTEGER,   10, 10, 0,  SQL_NULLABLE) == OK);
  is(desc_col_check(hstmt, 2,  "c2",  SQL_BINARY,    4,  2,  0,  SQL_NO_NULLS) == OK);
  is(desc_col_check(hstmt, 3,  "c3",  SQL_CHAR,      1,  1,  0,  SQL_NULLABLE) == OK);
  is(desc_col_check(hstmt, 4,  "c4",  SQL_VARCHAR,   5,  5,  0,  SQL_NULLABLE) == OK);
  is(desc_col_check(hstmt, 5,  "c5",  SQL_DECIMAL,   10, 10, 3,  SQL_NO_NULLS) == OK);
  is(desc_col_check(hstmt, 6,  "c6",  SQL_TINYINT,   3,  4,  0,  SQL_NULLABLE) == OK);
  is(desc_col_check(hstmt, 7,  "c7",  SQL_SMALLINT,  5,  6,  0,  SQL_NULLABLE) == OK);
  is(desc_col_check(hstmt, 8,  "c8",  SQL_DECIMAL,   4,  4,  2,  SQL_NULLABLE) == OK);
  is(desc_col_check(hstmt, 9,  "c9",  SQL_DOUBLE,    15, 15, 0, SQL_NULLABLE) == OK);
  is(desc_col_check(hstmt, 10, "c10", SQL_REAL,      7,  7,  0, SQL_NULLABLE) == OK);
  is(desc_col_check(hstmt, 11, "c11", SQL_BIGINT,    19, 19, 0,  SQL_NO_NULLS) == OK);

  is(desc_col_check(hstmt, 12, "c12", SQL_VARBINARY, 12, 12, 0,  SQL_NULLABLE) == OK);

  is(desc_col_check(hstmt, 13, "c13", SQL_CHAR,      20, 20, 0,  SQL_NO_NULLS) == OK);
  is(desc_col_check(hstmt, 14, "c14", SQL_REAL,      7,  7,  0,  SQL_NULLABLE) == OK);
  is(desc_col_check(hstmt, 15, "c15", SQL_LONGVARCHAR, 255, 255, 0,  SQL_NULLABLE) == OK);
  is(desc_col_check(hstmt, 16, "c16", SQL_LONGVARCHAR, 65535, 65535, 0,  SQL_NULLABLE) == OK);
  is(desc_col_check(hstmt, 17, "c17", SQL_LONGVARCHAR, 16777215, 16777215, 0,  SQL_NULLABLE) == OK);
  is(desc_col_check(hstmt, 18, "c18", SQL_LONGVARCHAR, 4294967295 , 16777215 , 0,  SQL_NULLABLE) == OK);
  is(desc_col_check(hstmt, 19, "c19", SQL_LONGVARBINARY, 255, 255, 0,  SQL_NULLABLE) == OK);
  is(desc_col_check(hstmt, 20, "c20", SQL_LONGVARBINARY, 65535, 65535, 0,  SQL_NULLABLE) == OK);
  is(desc_col_check(hstmt, 21, "c21", SQL_LONGVARBINARY, 16777215, 16777215, 0,  SQL_NULLABLE) == OK);
  is(desc_col_check(hstmt, 22, "c22", SQL_LONGVARBINARY, 4294967295 , 16777215 , 0,  SQL_NULLABLE) == OK);
  is(desc_col_check(hstmt, 23, "c23", SQL_LONGVARBINARY, 255, 5, 0,  SQL_NULLABLE) == OK);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_desc_col");

  return OK;
}


/* Test for misc CONVERT bug #1082 */
DECLARE_TEST(t_convert)
{
  SQLRETURN  rc;
  SQLLEN     data_len;
  SQLCHAR    data[50];

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_convert");

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
    myassert(strcmp((char *)data,"record1-must be string") == 0 ||
             strcmp((char *)data,"record2-must be string") == 0);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    myassert(strcmp((char *)data,"record1-must be string") == 0 ||
             strcmp((char *)data,"record2-must be string") == 0);

    rc = SQLFetch(hstmt);
    myassert( rc == SQL_NO_DATA);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_convert");

  return OK;
}


DECLARE_TEST(t_max_rows)
{
  SQLRETURN rc;
  SQLUINTEGER i;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_max_rows");

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_max_rows(id int)");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    ok_stmt(hstmt, SQLPrepare(hstmt,
                              (SQLCHAR *)"insert into t_max_rows values(?)",
                              SQL_NTS));

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

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_max_rows");

  return OK;
}


DECLARE_TEST(t_multistep)
{
  SQLRETURN  rc;
  SQLCHAR    szData[150];
  SQLLEN     pcbValue;
  SQLINTEGER id;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_multistep");
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
    printMessage("id: %ld",id);
    myassert(id == 10);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,0,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    printMessage("length: %ld", pcbValue);
    myassert(pcbValue == 28);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,0,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    printMessage("length: %ld", pcbValue);
    myassert(pcbValue == 28);

    rc = SQLGetData(hstmt,2,SQL_C_BINARY,szData,0,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    printMessage("length: %ld", pcbValue);
    myassert(pcbValue == 28);

    rc = SQLGetData(hstmt,2,SQL_C_BINARY,szData,0,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    printMessage("length: %ld", pcbValue);
    myassert(pcbValue == 28);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,0,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    printMessage("length: %ld", pcbValue);
    is_num(pcbValue, 28);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,10,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    printMessage("data  : %s (%ld)",szData,pcbValue);
    is_num(pcbValue, 28);
    is_str(szData, "MySQL - O", 10);

    pcbValue= 0;
    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,5,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    printMessage("data  : %s (%ld)",szData,pcbValue);
    is_num(pcbValue, 19);
    is_str(szData, "pen ", 5);

    pcbValue= 0;
    szData[0]='A';
    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,0,&pcbValue);
    myassert(rc == SQL_SUCCESS_WITH_INFO);
    printMessage("data  : %s (%ld)",szData,pcbValue);
    myassert(pcbValue == 15);
    myassert(szData[0] == 'A');

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,pcbValue+1,&pcbValue);
    mystmt(hstmt,rc);
    printMessage("data  : %s (%ld)",szData,pcbValue);
    is_num(pcbValue, 15);
    is_str(szData,"Source Database", 16);

    pcbValue= 99;
    szData[0]='A';
    expect_stmt(hstmt, SQLGetData(hstmt, 2, SQL_C_CHAR, szData, 0, &pcbValue),
                SQL_NO_DATA_FOUND);
    printMessage("data  : %s (%ld)",szData,pcbValue);
    myassert(pcbValue == 99);
    myassert(szData[0] == 'A');


    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA_FOUND);

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_multistep");

  return OK;
}


DECLARE_TEST(t_zerolength)
{
  SQLRETURN  rc;
  SQLCHAR    szData[100], bData[100], bData1[100];
  SQLLEN     pcbValue,pcbValue1,pcbValue2;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_zerolength");
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
    printMessage("length: %d", pcbValue);
    myassert(pcbValue == 0);

    bData[0]=bData[1]='z';
    rc = SQLGetData(hstmt,2,SQL_C_BINARY,bData,0,&pcbValue1);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    printMessage("length: %d", pcbValue1);
    myassert(pcbValue1 == 0);
    myassert(bData[0] == 'z');
    myassert(bData[1] == 'z');

    bData1[0]=bData1[1]='z';
    rc = SQLGetData(hstmt,3,SQL_C_BINARY,bData1,0,&pcbValue2);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    printMessage("length: %d", pcbValue2);
    myassert(pcbValue2 == 0);
    myassert(bData1[0] == 'z');
    myassert(bData1[1] == 'z');

    pcbValue= pcbValue1= 99;
    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,1,&pcbValue);
    mystmt(hstmt,rc);
    printMessage("data: %s, length: %d", szData, pcbValue);
    myassert(pcbValue == 0);
    myassert(szData[0] == '\0');

    bData[0]=bData[1]='z';
    rc = SQLGetData(hstmt,2,SQL_C_BINARY,bData,1,&pcbValue1);
    mystmt(hstmt,rc);
    printMessage("data: %s, length: %d", bData, pcbValue1);
    myassert(pcbValue1 == 0);
    myassert(bData[0]== '\0');

    bData1[0]=bData1[1]='z';
    rc = SQLGetData(hstmt,3,SQL_C_CHAR,bData1,1,&pcbValue2);
    mystmt(hstmt,rc);
    printMessage("data: %s, length: %d", bData1, pcbValue2);
    myassert(pcbValue2 == 0);
    myassert(bData1[0] == '\0');
    myassert(bData1[1] == 'z');

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    pcbValue= pcbValue1= 99;
    szData[0]= bData[0]= 'z';
    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,0,&pcbValue);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    printMessage("length: %d", pcbValue);
    myassert(pcbValue == 4);
    myassert(szData[0] == 'z');

    rc = SQLGetData(hstmt,2,SQL_C_BINARY,bData,0,&pcbValue1);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    printMessage("length: %d", pcbValue1);
    myassert(pcbValue1 == 5);
    myassert(bData[0] == 'z');

    bData[0]=bData1[1]='z';
    rc = SQLGetData(hstmt,3,SQL_C_BINARY,bData1,0,&pcbValue2);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    printMessage("length: %d", pcbValue2);
    myassert(pcbValue2 == 5);

    pcbValue= pcbValue1= 99;
    szData[0]= szData[1]= bData[0]= bData[1]= 'z';
    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,1,&pcbValue);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    printMessage("data: %s, length: %d", szData,pcbValue);
    myassert(pcbValue == 4);
    myassert(szData[0] == '\0');

    rc = SQLGetData(hstmt,2,SQL_C_BINARY,bData,1,&pcbValue1);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    printMessage("data; %s, length: %d", bData, pcbValue1);
    myassert(pcbValue1 == 5);
    myassert(bData[0] == 'm');

    bData[0]=bData1[1]='z';
    rc = SQLGetData(hstmt,3,SQL_C_BINARY,bData1,1,&pcbValue2);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    printMessage("length: %d", pcbValue2);
    myassert(pcbValue2 == 5);
    myassert(bData1[0] == 'm');
    myassert(bData1[1] == 'z');

    pcbValue= pcbValue1= 99;
    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,4,&pcbValue);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    printMessage("data: %s, length: %d", szData, pcbValue);
    is_num(pcbValue, 4);
    is_str(szData,"ven", 3);

    rc = SQLGetData(hstmt,2,SQL_C_BINARY,bData,4,&pcbValue1);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    printMessage("data: %s, length: %d", bData, pcbValue1);
    is_num(pcbValue1, 5);
    is_str(bData, "mysq", 4);

    pcbValue= pcbValue1= 99;
    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,5,&pcbValue);
    mystmt(hstmt,rc);
    printMessage("data: %s, length: %d", szData, pcbValue);
    is_num(pcbValue, 4);
    is_str(szData, "venu", 4);

    rc = SQLGetData(hstmt,2,SQL_C_BINARY,bData,5,&pcbValue1);
    mystmt(hstmt,rc);
    printMessage("data: %s, length: %d", bData, pcbValue1);
    is_num(pcbValue1, 5);
    is_str(bData, "mysql", 5);

    szData[0]= 'z';
    rc = SQLGetData(hstmt,3,SQL_C_CHAR,szData,0,&pcbValue);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    printMessage("data: %s, length: %d", szData, pcbValue);
    myassert(pcbValue == 5 || pcbValue == 10);
    myassert(szData[0] == 'z');

#if TO_BE_FIXED_IN_DRIVER
    szData[0]=szData[1]='z';
    rc = SQLGetData(hstmt,3,SQL_C_CHAR,szData,1,&pcbValue);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    printMessage("data: %s, length: %d", szData, pcbValue);
    myassert(pcbValue == 10);
    myassert(szData[0] == 'm');
    myassert(szData[1] == 'z');

    rc = SQLGetData(hstmt,3,SQL_C_CHAR,szData,4,&pcbValue);
    mystmt_err(hstmt,rc == SQL_SUCCESS_WITH_INFO,rc);
    printMessage("data: %s, length: %d", szData, pcbValue);
    myassert(pcbValue == 10);
    myassert(strncmp(szData,"mont",4) == 0);

    rc = SQLGetData(hstmt,3,SQL_C_CHAR,szData,5,&pcbValue);
    mystmt(hstmt,rc);
    printMessage("data: %s, length: %d", szData, pcbValue);
    myassert(pcbValue == 10);
    myassert(strncmp(szData,"monty",5) == 0);
#endif

    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA_FOUND);

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_zerolength");

  return OK;
}


/* Test the bug when two stmts are used with the don't cache results */
DECLARE_TEST(t_cache_bug)
{
  SQLHENV    henv1;
  SQLHDBC    hdbc1;
  SQLHSTMT   hstmt1, hstmt2;
  SQLCHAR    conn[MAX_NAME_LEN];

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_cache");
  ok_sql(hstmt, "CREATE TABLE t_cache (id INT)");
  ok_sql(hstmt, "INSERT INTO t_cache VALUES (1),(2),(3),(4),(5)");

  sprintf((char *)conn, "DSN=%s;USER=%s;PASSWORD=%s;DATABASE=%s;OPTION=1048579",
          mydsn, myuid, mypwd, mydb);
  if (mysock != NULL)
  {
    strcat((char *)conn, ";SOCKET=");
    strcat((char *)conn, (char *)mysock);
  }
  if (myport)
  {
    char pbuff[20];
    sprintf(pbuff, ";PORT=%d", myport);
    strcat((char *)conn, pbuff);
  }
  is(mydrvconnect(&henv1, &hdbc1, &hstmt1, conn) == OK);

  ok_stmt(hstmt1, SQLSetStmtAttr(hstmt1, SQL_ATTR_CURSOR_TYPE,
                                 (SQLPOINTER)SQL_CURSOR_STATIC, 0));

  ok_sql(hstmt1, "SELECT * FROM t_cache");

  ok_stmt(hstmt1, SQLFetch(hstmt1));

  ok_stmt(hstmt1, SQLFetch(hstmt1));

  ok_con(hdbc1, SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt2));

  ok_stmt(hstmt2, SQLColumns(hstmt2, NULL, 0, NULL, 0,
                             (SQLCHAR *)"t_cache", SQL_NTS, NULL, 0));

  ok_stmt(hstmt2, SQLFetch(hstmt2));

  ok_stmt(hstmt1, SQLFetch(hstmt1));

  expect_stmt(hstmt2, SQLFetch(hstmt2), SQL_NO_DATA);

  ok_stmt(hstmt1, SQLFetch(hstmt1));

  ok_stmt(hstmt2, SQLFreeHandle(SQL_HANDLE_STMT, hstmt2));

  ok_stmt(hstmt1, SQLFetch(hstmt1));

  expect_stmt(hstmt1, SQLFetch(hstmt1), SQL_NO_DATA);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));

  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));
  ok_env(henv1, SQLFreeEnv(henv1));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_cache");

  return OK;
}


/* Test the bug when two stmts are used with the don't cache results */
DECLARE_TEST(t_non_cache_bug)
{
  SQLHENV    henv1;
  SQLHDBC    hdbc1;
  SQLHSTMT   hstmt1, hstmt2;
  SQLCHAR    conn[MAX_NAME_LEN];

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_cache");
  ok_sql(hstmt, "CREATE TABLE t_cache (id INT)");
  ok_sql(hstmt, "INSERT INTO t_cache VALUES (1),(2),(3),(4),(5)");

  sprintf((char *)conn, "DSN=%s;USER=%s;PASSWORD=%s;DATABASE=%s;OPTION=3",
          mydsn, myuid, mypwd, mydb);
  if (mysock != NULL)
  {
    strcat((char *)conn, ";SOCKET=");
    strcat((char *)conn, (char *)mysock);
  }
  is(mydrvconnect(&henv1, &hdbc1, &hstmt1, conn) == OK);

  ok_stmt(hstmt1, SQLSetStmtAttr(hstmt1, SQL_ATTR_CURSOR_TYPE,
                                 (SQLPOINTER)SQL_CURSOR_STATIC, 0));

  ok_sql(hstmt1, "SELECT * FROM t_cache");

  ok_stmt(hstmt1, SQLFetch(hstmt1));

  ok_stmt(hstmt1, SQLFetch(hstmt1));

  ok_con(hdbc1, SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt2));

  ok_stmt(hstmt2, SQLColumns(hstmt2, NULL, 0, NULL, 0,
                             (SQLCHAR *)"t_cache", SQL_NTS, NULL, 0));

  ok_stmt(hstmt2, SQLFetch(hstmt2));

  ok_stmt(hstmt1, SQLFetch(hstmt1));

  expect_stmt(hstmt2, SQLFetch(hstmt2), SQL_NO_DATA);

  ok_stmt(hstmt1, SQLFetch(hstmt1));

  ok_stmt(hstmt2, SQLFreeHandle(SQL_HANDLE_STMT, hstmt2));

  ok_stmt(hstmt1, SQLFetch(hstmt1));

  expect_stmt(hstmt1, SQLFetch(hstmt1), SQL_NO_DATA);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));

  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));
  ok_env(henv1, SQLFreeEnv(henv1));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_cache");

  return OK;
}


DECLARE_TEST(t_empty_str_bug)
{
  SQLRETURN    rc;
  SQLINTEGER   id;
  SQLLEN       name_len, desc_len;
  SQLCHAR      name[20], desc[20];

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_empty_str_bug");

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

    ok_stmt(hstmt, SQLSetCursorName(hstmt, (SQLCHAR *)"venu", SQL_NTS));

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
    strcpy((char *)name,"MySQL AB");
    name_len= SQL_NTS;
    strcpy((char *)desc,"");
    desc_len= SQL_COLUMN_IGNORE;

    rc = SQLSetPos(hstmt,1,SQL_ADD,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt,&name_len);
    mystmt(hstmt,rc);

    printMessage("rows affected: %d",name_len);
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

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_empty_str_bug");

  return OK;
}


DECLARE_TEST(t_desccol)
{
    SQLRETURN rc;
    SQLCHAR colname[20];
    SQLSMALLINT collen,datatype,decptr,nullable;
    SQLULEN colsize;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_desccol");
    tmysql_exec(hstmt,"drop table t_desccol");

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_desccol(col1 int, col2 varchar(10), col3 text)");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    ok_sql(hstmt,"insert into t_desccol values(10,'venu','mysql')");

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from t_desccol");
    mystmt(hstmt,rc);

    rc = SQLDescribeCol(hstmt,1,colname,20,&collen,&datatype,&colsize,&decptr,&nullable);
    mystmt(hstmt,rc);
    printMessage("1: %s,%d,%d,%d,%d,%d",colname,collen,datatype,colsize,decptr,nullable);;

    rc = SQLDescribeCol(hstmt,2,colname,20,&collen,&datatype,&colsize,&decptr,&nullable);
    mystmt(hstmt,rc);
    printMessage("2: %s,%d,%d,%d,%d,%d",colname,collen,datatype,colsize,decptr,nullable);;

    rc = SQLDescribeCol(hstmt,3,colname,20,&collen,&datatype,&colsize,&decptr,&nullable);
    mystmt(hstmt,rc);
    printMessage("3: %s,%d,%d,%d,%d,%d",colname,collen,datatype,colsize,decptr,nullable);;

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_desccol");

  return OK;
}


int desccol(SQLHSTMT hstmt, char *cname, SQLSMALLINT clen,
            SQLSMALLINT sqltype, SQLULEN size,
            SQLSMALLINT scale, SQLSMALLINT isNull)
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

    sprintf((char *)select,"select %s from t_desccolext",cname);
    printMessage("%s",select);

    rc = SQLExecDirect(hstmt,select,SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLDescribeCol( hstmt,1,lcname,  sizeof(lcname),&lclen,
                         &lsqltype,&lsize,&lscale,&lisNull);
    mystmt(hstmt,rc);

    printMessage("name: %s (%d)",lcname,lclen);
    printMessage(" sqltype: %d, size: %d, scale: %d, null: %d\n",lsqltype,lsize,lscale,lisNull);

    is_str(lcname, cname, clen);
    myassert(lclen == clen);
    myassert(lsqltype == sqltype);
    myassert(lsize == size);
    myassert(lscale == scale);
    myassert(lisNull == isNull);

    SQLFreeStmt(hstmt,SQL_CLOSE);

    return OK;
}


DECLARE_TEST(t_desccolext)
{
    SQLRETURN rc;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_desccolext");

  ok_sql(hstmt, "create table t_desccolext\
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
        st set('1','2','3'))");

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    is(desccol(hstmt,"t1",2,SQL_TINYINT,3,0,SQL_NULLABLE) == OK);
    is(desccol(hstmt,"t2",2,SQL_TINYINT,3,0,SQL_NULLABLE) == OK);
    is(desccol(hstmt,"t3",2,SQL_TINYINT,3,0,SQL_NULLABLE) == OK);

    is(desccol(hstmt,"s1",2,SQL_SMALLINT,5,0,SQL_NULLABLE) == OK);
    is(desccol(hstmt,"s2",2,SQL_SMALLINT,5,0,SQL_NULLABLE) == OK);
    is(desccol(hstmt,"s3",2,SQL_SMALLINT,5,0,SQL_NULLABLE) == OK);

    is(desccol(hstmt,"m1",2,SQL_INTEGER,8,0,SQL_NULLABLE) == OK);
    is(desccol(hstmt,"m2",2,SQL_INTEGER,8,0,SQL_NULLABLE) == OK);
    is(desccol(hstmt,"m3",2,SQL_INTEGER,8,0,SQL_NULLABLE) == OK);

    is(desccol(hstmt,"i1",2,SQL_INTEGER,10,0,SQL_NULLABLE) == OK);
    is(desccol(hstmt,"i2",2,SQL_INTEGER,10,0,SQL_NO_NULLS) == OK);
    is(desccol(hstmt,"i3",2,SQL_INTEGER,10,0,SQL_NULLABLE) == OK);
    is(desccol(hstmt,"i4",2,SQL_INTEGER,10,0,SQL_NULLABLE) == OK);

    is(desccol(hstmt,"b1",2,SQL_BIGINT,19,0,SQL_NULLABLE) == OK);
    is(desccol(hstmt,"b2",2,SQL_BIGINT,19,0,SQL_NULLABLE) == OK);
    is(desccol(hstmt,"b3",2,SQL_BIGINT,20,0,SQL_NULLABLE) == OK);

    is(desccol(hstmt,"f1",2,SQL_REAL,7,0,SQL_NULLABLE) == OK);
    is(desccol(hstmt,"f2",2,SQL_REAL,7,0,SQL_NULLABLE) == OK);
    is(desccol(hstmt,"f3",2,SQL_REAL,7,0,SQL_NULLABLE) == OK);
    is(desccol(hstmt,"f4",2,SQL_REAL,7,0,SQL_NULLABLE) == OK);

    is(desccol(hstmt,"d1",2,SQL_DOUBLE,15,0,SQL_NULLABLE) == OK);
    is(desccol(hstmt,"d2",2,SQL_DOUBLE,15,0,SQL_NULLABLE) == OK);
    is(desccol(hstmt,"d3",2,SQL_DOUBLE,15,0,SQL_NULLABLE) == OK);
    is(desccol(hstmt,"d4",2,SQL_DOUBLE,15,0,SQL_NULLABLE) == OK);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_desccolext");

  return OK;
}


DECLARE_TEST(t_desccol1)
{
    SQLRETURN rc;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_desccol1");
    rc = SQLExecDirect(hstmt,(SQLCHAR *)"create table t_desccol1\
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
        printMessage("total columns:%d",pccol);

        printMessage("Name   nlen type    size decs null");
        for ( index = 1; index <= pccol; index++)
        {
            rc = SQLDescribeCol(hstmt, index, ColumnName,
                                sizeof(ColumnName),
                                &ColumnNameSize, &ColumnSQLDataType,
                                &ColumnSize,
                                &ColumnDecimals, &ColumnNullable);
            mystmt(hstmt,rc);

            printMessage("%-6s %4d %4d %7ld %4d %4d", ColumnName,
                         ColumnNameSize, ColumnSQLDataType, ColumnSize,
                         ColumnDecimals, ColumnNullable);
        }
    }

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_desccol1");

  return OK;
}


DECLARE_TEST(t_colattributes)
{
  SQLLEN count;
  SQLINTEGER isauto;

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

  ok_stmt(hstmt, SQLFreeStmt(hstmt,SQL_CLOSE));
  ok_sql(hstmt, "SELECT * FROM t_colattr");

  ok_stmt(hstmt, SQLColAttribute(hstmt, 1, SQL_COLUMN_COUNT, NULL, 0, NULL,
                                 &count));
  is(count == 55);

  ok_stmt(hstmt, SQLColAttribute(hstmt, 1, SQL_COLUMN_AUTO_INCREMENT, NULL, 0,
                                 NULL, &isauto));
  is_num(isauto, SQL_TRUE);

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_colattr");

  return OK;
}


DECLARE_TEST(t_exfetch)
{
    SQLRETURN rc;
    SQLUINTEGER i;

    ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE,
                                  (SQLPOINTER)SQL_CURSOR_STATIC, 0));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_exfetch");

    rc = tmysql_exec(hstmt,"create table t_exfetch(col1 int)");
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt, (SQLCHAR *)"insert into t_exfetch values (?)",
                    SQL_NTS);
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

    rc = SQLExecDirect(hstmt, (SQLCHAR *)"select * from t_exfetch",SQL_NTS);
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

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_exfetch");

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

  ok_con(hdbc, SQLAllocStmt(hdbc, &hstmt1));

  ok_stmt(hstmt, SQLSetCursorName(hstmt, (SQLCHAR *)"venu_cur", SQL_NTS));

  ok_sql(hstmt, "DROP TABLE IF EXISTS tmysql_rowstatus");
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

    ok_sql(hstmt1,
           "UPDATE tmysql_rowstatus SET col1 = 999,"
           "col2 = 'pos-update' WHERE CURRENT OF venu_cur");

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_LAST,1,NULL,NULL);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt,1,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    printMessage("rgfRowStatus[0]:%d",rgfRowStatus[0]);

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

  ok_sql(hstmt, "DROP TABLE IF EXISTS tmysql_rowstatus");

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


/**
Bug #16817: ODBC doesn't return multiple resultsets
*/
DECLARE_TEST(t_bug16817)
{
  SQLCHAR name[30];
  SQLSMALLINT ncol;

  ok_sql(hstmt, "DROP PROCEDURE IF EXISTS p_bug16817");
  ok_sql(hstmt, "CREATE PROCEDURE p_bug16817 () "
                "BEGIN "
                "  SELECT 'Marten' FROM DUAL; "
                "  SELECT 'Zack' FROM DUAL; "
               "END");

  ok_sql(hstmt, "CALL p_bug16817()");

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_str(my_fetch_str(hstmt, name, 1), "Marten", 6);
  ok_stmt(hstmt, SQLMoreResults(hstmt));

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_str(my_fetch_str(hstmt, name, 1), "Zack", 4);
  ok_stmt(hstmt, SQLMoreResults(hstmt));

  ok_stmt(hstmt, SQLNumResultCols(hstmt,&ncol));
  is_num(ncol, 0);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP PROCEDURE p_bug16817");
  return OK;
}


DECLARE_TEST(t_binary_collation)
{
  SQLSMALLINT name_length, data_type, decimal_digits, nullable;
  SQLCHAR column_name[SQL_MAX_COLUMN_NAME_LEN];
  SQLULEN column_size;
  SQLCHAR server_version[MYSQL_NAME_LEN+1];

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_binary_collation");
  ok_sql(hstmt, "CREATE TABLE t_binary_collation (id INT)");
  ok_sql(hstmt, "SHOW CREATE TABLE t_binary_collation");

  ok_con(hdbc, SQLGetInfo(hdbc, SQL_DBMS_VER, server_version,
                          MYSQL_NAME_LEN, NULL));

  ok_stmt(hstmt, SQLDescribeCol(hstmt, 1, column_name, sizeof(column_name),
                                &name_length, &data_type, &column_size,
                                &decimal_digits, &nullable));
  if (mysql_min_version(hdbc, "5.2", 3) ||
      /* 5.0.46 or later in 5.0 series */
      (!strncmp("5.0", server_version, 3) &&
        mysql_min_version(hdbc, "5.0.46", 6)) ||
      /* 5.1.22 or later in 5.1 series */
      (!strncmp("5.1", server_version, 3) &&
        mysql_min_version(hdbc, "5.1.22", 6)))
  {
    is_num(data_type, SQL_VARCHAR);
  }
  else
  {
    is_num(data_type, SQL_VARBINARY);
  }
  ok_sql(hstmt, "DROP TABLE IF EXISTS t_binary_collation");
  return OK;
}


/*
 * Bug 29239 - Prepare before insert returns wrong result
 */
DECLARE_TEST(t_bug29239)
{
  SQLHANDLE hstmt2;
  SQLINTEGER xval;

  ok_sql(hstmt, "drop table if exists bug29239");
  ok_sql(hstmt, "create table bug29239 ( x int )");

  /* prepare & bind */
  ok_stmt(hstmt, SQLPrepare(hstmt, (SQLCHAR *)"select x from bug29239",
                            SQL_NTS));
  ok_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_LONG, &xval, 0, NULL));

  /* insert before execute, with a new stmt handle */
  ok_con(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt2));
  ok_sql(hstmt2, "insert into bug29239 values (44)");
  ok_stmt(hstmt2, SQLFreeHandle(SQL_HANDLE_STMT, hstmt2));

  /* now execute */
  ok_stmt(hstmt, SQLExecute(hstmt));
  ok_stmt(hstmt, SQLFetch(hstmt));
  is_num(xval, 44);
  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA);

  ok_sql(hstmt, "drop table if exists bug29239");
  return OK;
}


/*
   Bug 30958, blank "text" fields are not accessible through ADO.
   This is a result of us not handle SQLGetData() w/a zero-len
   buffer correctly.
*/
DECLARE_TEST(t_bug30958)
{
  SQLCHAR outbuf[20]= "bug";
  SQLLEN outlen;
  SQLINTEGER outmax= 0;

  ok_sql(hstmt, "drop table if exists bug30958");
  ok_sql(hstmt, "CREATE TABLE bug30958 (tt_textfield TEXT NOT NULL)");
  ok_sql(hstmt, "INSERT INTO bug30958 (tt_textfield) VALUES ('')");

  ok_sql(hstmt, "select tt_textfield from bug30958");
  ok_stmt(hstmt, SQLFetch(hstmt));

  /*
   check first that we get truncation, with zero bytes
   available in out buffer, outbuffer should be untouched
  */
  outlen= 99;
  expect_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, outbuf, outmax,
                                &outlen), SQL_SUCCESS_WITH_INFO);
  is_str(outbuf, "bug", 3);
  is_num(outlen, 0);
  is(check_sqlstate(hstmt, "01004") == OK);

  /* expect the same result, and not SQL_NO_DATA */
  outlen= 99;
  expect_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, outbuf, outmax,
                                &outlen), SQL_SUCCESS_WITH_INFO);
  is_str(outbuf, "bug", 3);
  is_num(outlen, 0);
  is(check_sqlstate(hstmt, "01004") == OK);

  /* now provide a space to read the data */
  outmax= 1;
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, outbuf, outmax, &outlen));
  is_num(outbuf[0], 0);
  is_num(outlen, 0);

  /* only now is it unavailable (test with empty and non-empty out buffer) */
  outmax= 0;
  expect_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, outbuf, outmax,
                                &outlen), SQL_NO_DATA);
  outmax= 1;
  expect_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, outbuf, outmax,
                                &outlen), SQL_NO_DATA);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "drop table if exists bug30958");

  return OK;
}


/**
 Bug #31246: Opening rowset with extra fields leads to incorrect SQL INSERT
*/
DECLARE_TEST(t_bug31246)
{
  SQLSMALLINT ncol;
  SQLCHAR     *buf= (SQLCHAR *)"Key1";
  SQLCHAR     field1[20];
  SQLINTEGER  field2;
  SQLCHAR     field3[20];
  SQLRETURN   rc;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug31246");
  ok_sql(hstmt, "CREATE TABLE t_bug31246 ("
                "field1 VARCHAR(50) NOT NULL PRIMARY KEY, "
                "field2 int DEFAULT 10, "
                "field3 VARCHAR(50) DEFAULT \"Default Text\")");

  /* No need to insert any rows in the table, so do SELECT */
  ok_sql(hstmt, "SELECT * FROM t_bug31246");
  ok_stmt(hstmt, SQLNumResultCols(hstmt,&ncol));
  is_num(ncol, 3);

  /* Bind only one column instead of three ones */
  ok_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_CHAR,
                            buf, strlen((char *)buf), NULL));

  /* Expect SQL_NO_DATA_FOUND result from the empty table */
  rc= SQLExtendedFetch(hstmt, SQL_FETCH_NEXT, 1, NULL, NULL);
  is_num(rc, SQL_NO_DATA_FOUND);

  /* Here was the problem */
  ok_stmt(hstmt, SQLSetPos(hstmt, 1, SQL_ADD, SQL_LOCK_NO_CHANGE));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  /* Check whether the row was inserted with the default values*/
  ok_sql(hstmt, "SELECT * FROM t_bug31246 WHERE field1=\"Key1\"");
  ok_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_CHAR, field1,
          sizeof(field1), NULL));
  ok_stmt(hstmt, SQLBindCol(hstmt, 2, SQL_C_LONG, &field2,
          sizeof(SQLINTEGER), NULL));
  ok_stmt(hstmt, SQLBindCol(hstmt, 3, SQL_C_CHAR, field3,
          sizeof(field3), NULL));
  ok_stmt(hstmt, SQLFetch(hstmt));

  /* Clean-up */
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  ok_sql(hstmt, "DROP TABLE t_bug31246");
  return OK;
}


/**
  Bug #13776: Invalid string or buffer length error
*/
DECLARE_TEST(t_bug13776)
{
  SQLHENV  henv1;
  SQLHDBC  hdbc1;
  SQLHSTMT hstmt1;

  SQLULEN     pcColSz;
  SQLCHAR     szColName[MAX_NAME_LEN];
  SQLSMALLINT pfSqlType, pcbScale, pfNullable;

  SET_DSN_OPTION(1 << 27);

  /* Establish the new connection */
  alloc_basic_handles(&henv1, &hdbc1, &hstmt1);

  ok_sql(hstmt1, "DROP TABLE IF EXISTS t_bug13776");
  ok_sql(hstmt1, "CREATE TABLE t_bug13776(ltext LONGTEXT)");
  ok_sql(hstmt1, "INSERT INTO t_bug13776 VALUES ('long text test')");
  ok_sql(hstmt1, "SELECT * FROM t_bug13776");
  ok_stmt(hstmt1, SQLDescribeCol(hstmt1, 1, szColName, MAX_NAME_LEN+1, NULL,
                                 &pfSqlType, &pcColSz, &pcbScale, &pfNullable));

  /* Size of LONGTEXT should have been capped to 1 << 31. */
  is_num(pcColSz, 2147483647L);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  ok_sql(hstmt1, "DROP TABLE IF EXISTS t_bug13776");

  free_basic_handles(&henv1, &hdbc1, &hstmt1);

  SET_DSN_OPTION(0);

  return OK;
}


/**
  Test that FLAG_COLUMN_SIZE_S32 is automatically enabled when ADO library
  is loaded.
*/
DECLARE_TEST(t_bug13776_auto)
{
#ifdef WIN32
  SQLHENV  henv1;
  SQLHDBC  hdbc1;
  SQLHSTMT hstmt1;
  HMODULE  ado_dll;

  SQLULEN     pcColSz;
  SQLCHAR     szColName[MAX_NAME_LEN];
  SQLSMALLINT pfSqlType, pcbScale, pfNullable;
  SQLCHAR     *env_path= NULL;
  SQLCHAR     szFileToLoad[255];

  /** @todo get the full path to the library using getenv */
  env_path= getenv("CommonProgramFiles(x86)");
  if (!env_path)
    env_path= getenv("CommonProgramFiles");

  if (!env_path)
  {
    printf("# No path for CommonProgramFiles in %s on line %d\n",
           __FILE__, __LINE__);
    return FAIL;
  }

  sprintf(szFileToLoad, "%s\\System\\ado\\msado15.dll", env_path);

  ado_dll= LoadLibrary(szFileToLoad);
  if (!ado_dll)
  {
    printf("# Could not load %s in %s on line %d\n",
           szFileToLoad, __FILE__, __LINE__);
    return FAIL;
  }

  /* Establish the new connection */
  alloc_basic_handles(&henv1, &hdbc1, &hstmt1);

  ok_sql(hstmt1, "DROP TABLE IF EXISTS t_bug13776");
  ok_sql(hstmt1, "CREATE TABLE t_bug13776(ltext LONGTEXT)");
  ok_sql(hstmt1, "INSERT INTO t_bug13776 VALUES ('long text test')");
  ok_sql(hstmt1, "SELECT * FROM t_bug13776");
  ok_stmt(hstmt1, SQLDescribeCol(hstmt1, 1, szColName, MAX_NAME_LEN+1, NULL,
                                 &pfSqlType, &pcColSz, &pcbScale, &pfNullable));

  /*
    IF adodb15.dll is loaded SQLDescribeCol should return the length of
    LONGTEXT columns as 2G instead of 4G
  */
  is_num(pcColSz, 2147483647L);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  ok_sql(hstmt1, "DROP TABLE IF EXISTS t_bug13776");

  free_basic_handles(&henv1, &hdbc1, &hstmt1);

  FreeLibrary(ado_dll);
#endif

  return OK;
}


/*
  Bug #32420 - Don't cache results and SQLExtendedFetch work badly together
*/
DECLARE_TEST(t_bug32420)
{
  SQLHANDLE henv1, hdbc1, hstmt1;
  SQLINTEGER nData[4];
  SQLCHAR szData[4][16];
  SQLUSMALLINT rgfRowStatus[4];

  SET_DSN_OPTION(1048576);

  alloc_basic_handles(&henv1, &hdbc1, &hstmt1);

  ok_sql(hstmt1, "drop table if exists bug32420");
  ok_sql(hstmt1, "CREATE TABLE bug32420 ("\
                "tt_int INT PRIMARY KEY auto_increment,"\
                "tt_varchar VARCHAR(128) NOT NULL)");
  ok_sql(hstmt1, "INSERT INTO bug32420 VALUES "\
                "(100, 'string 1'),"\
                "(200, 'string 2'),"\
                "(300, 'string 3'),"\
                "(400, 'string 4')");

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  ok_stmt(hstmt1, SQLSetStmtOption(hstmt1, SQL_ROWSET_SIZE, 4));

  ok_sql(hstmt1, "select * from bug32420 order by 1");
  ok_stmt(hstmt1, SQLBindCol(hstmt1, 1, SQL_C_LONG, nData, 0, NULL));
  ok_stmt(hstmt1, SQLBindCol(hstmt1, 2, SQL_C_CHAR, szData, sizeof(szData[0]),
                            NULL));
  ok_stmt(hstmt1, SQLExtendedFetch(hstmt1, SQL_FETCH_NEXT, 0, NULL, 
                                   rgfRowStatus));

  is_num(nData[0], 100);
  is_str(szData[0], "string 1", 8);
  is_num(nData[1], 200);
  is_str(szData[1], "string 2", 8);
  is_num(nData[2], 300);
  is_str(szData[2], "string 3", 8);
  is_num(nData[3], 400);
  is_str(szData[3], "string 4", 8);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));
  ok_sql(hstmt1, "drop table if exists bug32420");

  free_basic_handles(&henv1, &hdbc1, &hstmt1);

  SET_DSN_OPTION(1048576);

  return OK;
}


/**
 Bug #34575: SQL_C_CHAR value type and numeric parameter type causes trouble
*/
DECLARE_TEST(t_bug34575)
{
  SQLCHAR buff[10];
  SQLLEN len= 0;
  SQLSMALLINT namelen, type, digits, nullable;

  ok_stmt(hstmt, SQLPrepare(hstmt, (SQLCHAR *) "SELECT ?", SQL_NTS));
  strcpy((char *)buff, "2.0");
  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR,
                                  SQL_DECIMAL, 10, 0, buff, sizeof(buff),
                                  &len));

  /* Note: buff has '2.0', but len is still 0! */
  ok_stmt(hstmt, SQLExecute(hstmt));

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_str(my_fetch_str(hstmt, buff, 1), "", 1);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA);

  strcpy((char *)buff, "2.0");
  len= 3;

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLExecute(hstmt));

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_str(my_fetch_str(hstmt, buff, 1), "2.0", 4);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

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
  ADD_TEST(t_bug16817)
  ADD_TEST(bug6157)
  ADD_TEST(t_binary_collation)
  ADD_TODO(t_bug29239)
  ADD_TEST(t_bug30958)
  ADD_TEST(t_bug31246)
  ADD_TEST(t_bug13776)
  ADD_TEST(t_bug13776_auto)
  ADD_TEST(t_bug32420)
  ADD_TEST(t_bug34575)
END_TESTS


RUN_TESTS
