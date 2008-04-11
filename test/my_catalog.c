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

DECLARE_TEST(my_columns_null)
{
    SQLRETURN   rc;

    /* initialize data */
    SQLExecDirect(hstmt,"drop table my_column_null",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table my_column_null(id int not null, name varchar(30))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLColumns(hstmt,NULL,SQL_NTS,NULL,SQL_NTS,"my_column_null",SQL_NTS,NULL,SQL_NTS);
    mystmt(hstmt,rc);

    myassert(2 == my_print_non_format_result(hstmt));

    SQLFreeStmt(hstmt, SQL_CLOSE);

  ok_sql(hstmt, "DROP TABLE IF EXISTS my_column_null");

  return OK;
}


DECLARE_TEST(my_drop_table)
{
    SQLRETURN   rc;

    /* initialize data */
    SQLExecDirect(hstmt,"drop table my_drop_table",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table my_drop_table(id int not null)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLColumns(hstmt,NULL,0,NULL,0,"my_drop_table",SQL_NTS,NULL,0);
    mystmt(hstmt,rc);

    myassert(1 == my_print_non_format_result(hstmt));

    rc = SQLExecDirect(hstmt,"drop table my_drop_table",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


#define TODBC_BIND_CHAR(n,buf) SQLBindCol(hstmt,n,SQL_C_CHAR,&buf,sizeof(buf),NULL);


DECLARE_TEST(my_table_dbs)
{
    SQLCHAR    database[100];
    SQLRETURN  rc;
    SQLINTEGER nrows;
    SQLLEN lenOrNull;

    SQLExecDirect(hstmt, "DROP DATABASE my_all_db_test1",   SQL_NTS);
    SQLExecDirect(hstmt, "DROP DATABASE my_all_db_test2",   SQL_NTS);
    SQLExecDirect(hstmt, "DROP DATABASE my_all_db_test3",   SQL_NTS);
    SQLExecDirect(hstmt, "DROP DATABASE my_all_db_test4",   SQL_NTS);

    rc = SQLTables(hstmt,"%",1,NULL,0,NULL,0,NULL,0);
    mystmt(hstmt,rc);

    nrows = my_print_non_format_result(hstmt);
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTables(hstmt,"SQL_ALL_CATALOGS",SQL_NTS,NULL,0,NULL,0,NULL,0);
    mystmt(hstmt,rc);

    is(nrows == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTables(hstmt,"test",4,NULL,0,NULL,0,NULL,0);
    mystmt(hstmt,rc);

    my_print_non_format_result(hstmt);
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTables(hstmt,"mysql",5,NULL,0,NULL,0,NULL,0);
    mystmt(hstmt,rc);

    is(my_print_non_format_result(hstmt) != 0);
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTables(hstmt,"%",1,NULL,0,NULL,0,NULL,0);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    memset(database,0,100);
    rc = SQLGetData(hstmt,1,SQL_C_CHAR,(SQLCHAR *)&database,100,NULL);
    mystmt(hstmt,rc);
    printMessage("\n catalog: %s", database);

    memset(database,0,100);
    rc = SQLGetData(hstmt,2,SQL_C_CHAR,(SQLCHAR *)&database,100,&lenOrNull);
    mystmt(hstmt,rc);
    printMessage("\n schema: %s", database); 
    myassert(lenOrNull == SQL_NULL_DATA);

    memset(database,0,100);
    rc = SQLGetData(hstmt,3,SQL_C_CHAR,(SQLCHAR *)&database,100,&lenOrNull);
    mystmt(hstmt,rc);
    printMessage("\n table: %s", database); 
    myassert(lenOrNull == SQL_NULL_DATA);

    memset(database,0,100);
    rc = SQLGetData(hstmt,4,SQL_C_CHAR,(SQLCHAR *)&database,100,&lenOrNull);
    mystmt(hstmt,rc);
    printMessage("\n type: %s", database); 
    myassert(lenOrNull == SQL_NULL_DATA);

    memset(database,0,100);
    rc = SQLGetData(hstmt,5,SQL_C_CHAR, (SQLCHAR*)&database,100,&lenOrNull);
    mystmt(hstmt,rc);
    printMessage("\n database remark: %s", database);
    myassert(lenOrNull == SQL_NULL_DATA);

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLExecDirect(hstmt, "CREATE DATABASE my_all_db_test1",    SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt, "CREATE DATABASE my_all_db_test2",    SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt, "CREATE DATABASE my_all_db_test3",    SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt, "CREATE DATABASE my_all_db_test4",    SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLTables(hstmt,"%",1,NULL,0,NULL,0,NULL,0);
    mystmt(hstmt,rc);

    nrows += 4;
    is(nrows == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTables(hstmt,"SQL_ALL_CATALOGS",SQL_NTS,NULL,0,NULL,0,NULL,0);
    mystmt(hstmt,rc);

    is(nrows == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTables(hstmt,"my_all_db_test",SQL_NTS,NULL,0,NULL,0,NULL,0);
    mystmt(hstmt,rc);

    is(0 == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTables(hstmt,"my_all_db_test%",SQL_NTS,NULL,0,NULL,0,NULL,0);
    mystmt(hstmt,rc);

    is(0 == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    /* unknown table should be empty */
    rc = SQLTables(hstmt,"my_all_db_test%",SQL_NTS,NULL,0,"xyz",SQL_NTS,NULL,0);
    mystmt(hstmt,rc);

    is(0 == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt, "DROP DATABASE my_all_db_test1",  SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt, "DROP DATABASE my_all_db_test2",  SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt, "DROP DATABASE my_all_db_test3",  SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt, "DROP DATABASE my_all_db_test4",  SQL_NTS);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(my_colpriv)
{
  ok_sql(hstmt, "DROP TABLE IF EXISTS test_colprev1");
  ok_sql(hstmt, "DROP TABLE IF EXISTS test_colprev2");
  ok_sql(hstmt, "DROP TABLE IF EXISTS test_colprev3");

  ok_sql(hstmt, "CREATE TABLE test_colprev1(a INT,b INT,c INT, d INT)");
  ok_sql(hstmt, "CREATE TABLE test_colprev2(a INT,b INT,c INT, d INT)");
  ok_sql(hstmt, "CREATE TABLE test_colprev3(a INT,b INT,c INT, d INT)");

  (void)SQLExecDirect(hstmt, (SQLCHAR *)"DROP USER my_colpriv", SQL_NTS);

  ok_sql(hstmt, "GRANT SELECT(a,b),INSERT(d),UPDATE(c) ON test_colprev1 TO my_colpriv");
  ok_sql(hstmt, "GRANT SELECT(c,a),UPDATE(a,b) ON test_colprev3 TO my_colpriv");

  ok_sql(hstmt, "FLUSH PRIVILEGES");

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLColumnPrivileges(hstmt,
                                     NULL, SQL_NTS, NULL, SQL_NTS,
                                     "test_colprev1", SQL_NTS, NULL, SQL_NTS));

  is(4 == my_print_non_format_result(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLColumnPrivileges(hstmt,
                                     NULL, SQL_NTS, NULL, SQL_NTS,
                                     "test_colprev1", SQL_NTS, "a", SQL_NTS));

  is(1 == my_print_non_format_result(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLColumnPrivileges(hstmt,
                                     NULL, SQL_NTS, NULL, SQL_NTS,
                                     "test_colprev2", SQL_NTS, NULL, SQL_NTS));
  is(0 == my_print_non_format_result(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLColumnPrivileges(hstmt,
                                     NULL, SQL_NTS, NULL, SQL_NTS,
                                     "test_colprev3", SQL_NTS, NULL, SQL_NTS));

  is(4 == my_print_non_format_result(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLColumnPrivileges(hstmt,
                                     NULL, SQL_NTS, NULL, SQL_NTS,
                                     "test_%", SQL_NTS, NULL, SQL_NTS));

  my_print_non_format_result(hstmt);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLColumnPrivileges(hstmt,
                                     NULL, SQL_NTS, NULL, SQL_NTS,
                                     "test_colprev%", SQL_NTS, NULL, SQL_NTS));

  is(8 == my_print_non_format_result(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLColumnPrivileges(hstmt,
                                     "mysql", SQL_NTS, NULL, SQL_NTS,
                                     "columns_priv", SQL_NTS, NULL, SQL_NTS));

  my_print_non_format_result(hstmt);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP USER my_colpriv");

  ok_sql(hstmt, "DROP TABLE test_colprev1, test_colprev2, test_colprev3");

  return OK;
}


DECLARE_TEST(t_sqlprocedures)
{
  if (!mysql_min_version(hdbc, "5.0", 3))
    skip("server does not support stored procedures");

  /* avoid errors in case binary log is activated */
  ok_sql(hstmt, "SET GLOBAL log_bin_trust_function_creators = 1");

  ok_sql(hstmt, "DROP FUNCTION IF EXISTS t_sqlproc_func");
  ok_sql(hstmt,
         "CREATE FUNCTION t_sqlproc_func (a INT) RETURNS INT RETURN SQRT(a)");

  ok_sql(hstmt, "DROP PROCEDURE IF EXISTS t_sqlproc_proc");
  ok_sql(hstmt,
         "CREATE PROCEDURE t_sqlproc_proc (OUT a INT) BEGIN"
         " SELECT COUNT(*) INTO a FROM t_sqlproc;"
         "END;");

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  /* Try without specifying a catalog. */
  ok_stmt(hstmt, SQLProcedures(hstmt, NULL, 0, NULL, 0, "t_sqlproc%", SQL_NTS));

  is_num(my_print_non_format_result(hstmt), 2);

  /* And try with specifying a catalog.  */
  ok_stmt(hstmt, SQLProcedures(hstmt, "test", SQL_NTS, NULL, 0,
                               "t_sqlproc%", SQL_NTS));

  is_num(my_print_non_format_result(hstmt), 2);

  ok_sql(hstmt, "DROP PROCEDURE t_sqlproc_proc");
  ok_sql(hstmt, "DROP FUNCTION t_sqlproc_func");

  return OK;
}


DECLARE_TEST(t_catalog)
{
    SQLRETURN rc;
    SQLCHAR      name[MYSQL_NAME_LEN+1];
    SQLUSMALLINT i;
    SQLSMALLINT  ncols, len;

    SQLCHAR colnames[19][20]= {
        "TABLE_CAT","TABLE_SCHEM","TABLE_NAME","COLUMN_NAME",
        "DATA_TYPE","TYPE_NAME","COLUMN_SIZE","BUFFER_LENGTH",
        "DECIMAL_DIGITS","NUM_PREC_RADIX","NULLABLE","REMARKS",
        "COLUMN_DEF","SQL_DATA_TYPE","SQL_DATETIME_SUB",
        "CHAR_OCTET_LENGTH","ORDINAL_POSITION","IS_NULLABLE"
    };
    SQLSMALLINT collengths[18]= {
        9,11,10,11,9,9,11,13,14,14,8,7,10,13,16,17,16,11
    };

    SQLExecDirect(hstmt,"drop table t_catalog",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table t_catalog(a tinyint, b char(4))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLColumns(hstmt,NULL,0,NULL,0,"t_catalog",9,NULL,0);
    mystmt(hstmt,rc);

    rc = SQLNumResultCols(hstmt, &ncols);
    mystmt(hstmt,rc);

    printMessage("\n total columns: %d", ncols);
    myassert(ncols == 18);
    myassert(myresult(hstmt) == 2);

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLColumns(hstmt,NULL,0,NULL,0,"t_catalog",9,NULL,0);
    mystmt(hstmt,rc);

    rc = SQLNumResultCols(hstmt,&ncols);
    mystmt(hstmt,rc);

    for (i= 1; i <= (SQLUINTEGER) ncols; i++)
    {
        rc = SQLDescribeCol(hstmt, i, name, MYSQL_NAME_LEN+1, &len, NULL, NULL, NULL, NULL);
        mystmt(hstmt,rc);

        printMessage("\n column %d: %s (%d)", i, name, len);
        myassert(strcmp(name,colnames[i-1]) == 0 && len == collengths[i-1]);
    }
    SQLFreeStmt(hstmt,SQL_CLOSE);

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_catalog");

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


/* To test SQLColumns misc case */
DECLARE_TEST(t_columns)
{
  SQLSMALLINT   NumPrecRadix, DataType, Nullable, DecimalDigits;
  SQLLEN        cbColumnSize, cbDecimalDigits, cbNumPrecRadix,
                cbDataType, cbNullable;
  SQLINTEGER    cbDatabaseName;
  SQLUINTEGER   ColumnSize, i;
  SQLUINTEGER   ColumnCount= 7;
  SQLCHAR       ColumnName[MAX_NAME_LEN], DatabaseName[MAX_NAME_LEN];
  SQLINTEGER    Values[7][5][2]=
  {
    { {5,2},  {5,4}, {0,2},  {10,2},  {1,2}},
    { {1,2},  {5,4},  {0,-1}, {10,-1}, {1,2}},
    { {12,2}, {20,4}, {0,-1}, {10,-1}, {0,2}},
    { {3,2},  {10,4}, {2,2},  {10,2},  {1,2}},
    { {-6,2},  {3,4}, {0,2},  {10,2},  {0,2}},
    { {4,2}, {10,4}, {0,2},  {10,2},  {0,2}},
    { {-6,2}, {3,4}, {0,2},  {10,2},  {1,2}}
  };

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_columns");

  ok_sql(hstmt,
         "CREATE TABLE t_columns (col0 SMALLINT,"
         "col1 CHAR(5), col2 VARCHAR(20) NOT NULL, col3 DECIMAL(10,2),"
         "col4 TINYINT NOT NULL, col5 INTEGER PRIMARY KEY,"
         "col6 TINYINT NOT NULL UNIQUE AUTO_INCREMENT)");

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_METADATA_ID,
                                (SQLPOINTER)SQL_FALSE, SQL_IS_UINTEGER));

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE,
                                (SQLPOINTER)SQL_CURSOR_STATIC, 0));

  ok_con(hdbc, SQLGetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG,
                                 DatabaseName, MAX_NAME_LEN,
                                 &cbDatabaseName)); /* Current Catalog */

  for (i= 0; i < ColumnCount; i++)
  {
    sprintf((char *)ColumnName, "col%d", (int)i);
    printMessage("checking column `%s`", (char *)ColumnName);

    ok_stmt(hstmt, SQLColumns(hstmt,
                              DatabaseName, (SQLSMALLINT)cbDatabaseName,
                              (SQLCHAR *)"", SQL_NTS,
                              (SQLCHAR *)"t_columns", SQL_NTS,
                              ColumnName, SQL_NTS));

    /* 5 -- Data type */
    ok_stmt(hstmt, SQLBindCol(hstmt, 5, SQL_C_SSHORT, &DataType, 0,
                              &cbDataType));

    /* 7 -- Column Size */
    ok_stmt(hstmt, SQLBindCol(hstmt, 7, SQL_C_ULONG, &ColumnSize, 0,
                              &cbColumnSize));

    /* 9 -- Decimal Digits */
    ok_stmt(hstmt, SQLBindCol(hstmt, 9, SQL_C_SSHORT, &DecimalDigits, 0,
                              &cbDecimalDigits));

    /* 10 -- Num Prec Radix */
    ok_stmt(hstmt, SQLBindCol(hstmt, 10, SQL_C_SSHORT, &NumPrecRadix, 0,
                              &cbNumPrecRadix));

    /* 11 -- Nullable */
    ok_stmt(hstmt, SQLBindCol(hstmt, 11, SQL_C_SSHORT, &Nullable, 0,
                              &cbNullable));

    ok_stmt(hstmt, SQLFetch(hstmt));

    is_num(DataType,   Values[i][0][0]);
    is_num(cbDataType, Values[i][0][1]);

    is_num(ColumnSize,   Values[i][1][0]);
    is_num(cbColumnSize, Values[i][1][1]);

    is_num(DecimalDigits,   Values[i][2][0]);
    is_num(cbDecimalDigits, Values[i][2][1]);

    is_num(NumPrecRadix,   Values[i][3][0]);
    is_num(cbNumPrecRadix, Values[i][3][1]);

    is_num(Nullable,   Values[i][4][0]);
    is_num(cbNullable, Values[i][4][1]);

    expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA);

    ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
    ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  }

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_columns");

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
  SQLULEN     pcbColDef;
  SQLCHAR     szColName[MAX_NAME_LEN];
  int is51= mysql_min_version(hdbc, "5.1", 3);

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
     /* This depends on NAME_LEN in mysql_com.h */
#if UNRELIABLE_TEST
     is(t_tables_bug_data[i-1].pcbColDef == pcbColDef ||
        t_tables_bug_data[i-1].pcbColDef == pcbColDef / 3);
#endif
     myassert(t_tables_bug_data[i-1].pibScale == pibScale);
     myassert(t_tables_bug_data[i-1].pfNullable == pfNullable);
   }
   SQLFreeStmt(hstmt,SQL_CLOSE);

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

    SQLExecDirect(hstmt, "DROP DATABASE IF EXISTS test_odbc_current", SQL_NTS);

    strcpy(cur_db, "test_odbc_current");
    rc = SQLSetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (char *)cur_db, SQL_NTS);
    mycon_r(hdbc,rc);

    rc = SQLExecDirect(hstmt, "CREATE DATABASE test_odbc_current", SQL_NTS);
    mystmt(hstmt,rc);

    strcpy(cur_db, "test_odbc_current");
    rc = SQLSetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (char *)cur_db, SQL_NTS);
    mycon(hdbc,rc);

    rc = SQLGetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (char *)db, 255, &len);
    mycon(hdbc,rc);
    fprintf(stdout,"current_catalog: %s (%ld)\n", db, len);
    myassert(strcmp(cur_db, db) == 0 || strlen(cur_db) == len);

    strcpy(cur_db, "test_odbc_current_12455");
    rc = SQLSetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (char *)cur_db, SQL_NTS);
    mycon_r(hdbc,rc);

    rc = SQLSetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (char *)cur_db, len);
    mycon(hdbc,rc);

    /* reset for further tests */
    rc = SQLSetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (SQLCHAR *)"test", SQL_NTS);
    mycon(hdbc,rc);

    rc = SQLExecDirect(hstmt, "DROP DATABASE test_odbc_current", SQL_NTS);
    mycon(hstmt,rc);

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

  ok_sql(hstmt, "DROP TABLE IF EXISTS tmysql_spk");

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

    is_num(myresult(hstmt), 0);

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

    is_num(myresult(hstmt), 3);

    r = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,r);

  return OK;
}


/**
 Bug #4518: SQLForeignKeys returns too many foreign key
 Bug #27723: SQLForeignKeys does not escape _ and % in the table name arguments

 The original test case was extended to have a table that would inadvertantly
 get included because of the poor escaping.
*/
DECLARE_TEST(t_bug4518)
{
  SQLCHAR buff[255];

  /** @todo re-enable this test when I_S based SQLForeignKeys is done. */
  if (mysql_min_version(hdbc, "5.1", 3))
    skip("can't test foreign keys with 5.1 or later yet");

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug4518_c, t_bug4518_c2, t_bug4518ac, "
                "                     t_bug4518_p");
  ok_sql(hstmt, "CREATE TABLE t_bug4518_p (id INT PRIMARY KEY) ENGINE=InnoDB");
  ok_sql(hstmt, "CREATE TABLE t_bug4518_c (id INT, parent_id INT,"
                "                          FOREIGN KEY (parent_id)"
                "                           REFERENCES"
                "                             t_bug4518_p(id)"
                "                           ON DELETE SET NULL)"
                " ENGINE=InnoDB");
  ok_sql(hstmt, "CREATE TABLE t_bug4518_c2 (id INT, parent_id INT,"
                "                           FOREIGN KEY (parent_id)"
                "                            REFERENCES"
                "                              t_bug4518_p(id)"
                "                            ON DELETE SET NULL)"
                " ENGINE=InnoDB");
  ok_sql(hstmt, "CREATE TABLE t_bug4518ac (id INT, parent_id INT,"
                "                          FOREIGN KEY (parent_id)"
                "                           REFERENCES"
                "                             t_bug4518_p(id)"
                "                           ON DELETE SET NULL)"
                " ENGINE=InnoDB");

  ok_stmt(hstmt, SQLForeignKeys(hstmt, NULL, 0, NULL, 0, NULL, 0, NULL, 0,
                                NULL, 0, (SQLCHAR *)"t_bug4518_c", SQL_NTS));

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_str(my_fetch_str(hstmt, buff, 3), "t_bug4518_p", 11);
  is_str(my_fetch_str(hstmt, buff, 4), "id", 2);
  is_str(my_fetch_str(hstmt, buff, 7), "t_bug4518_c", 11);
  is_str(my_fetch_str(hstmt, buff, 8), "parent_id", 9);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt,SQL_CLOSE));

  ok_sql(hstmt,
         "DROP TABLE t_bug4518_c, t_bug4518_c2, t_bug4518ac, t_bug4518_p");

  return OK;
}


/**
 Tests the non-error code paths in catalog.c that return an empty set to
 make sure the resulting empty result sets at least indicate the right
 number of columns.
*/
DECLARE_TEST(empty_set)
{
  SQLSMALLINT columns;

  /* SQLTables(): no known table types. */
  ok_stmt(hstmt, SQLTables(hstmt, NULL, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS,
                           (SQLCHAR *)"UNKNOWN", SQL_NTS));
  ok_stmt(hstmt, SQLNumResultCols(hstmt, &columns));
  is_num(columns, 5);
  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  /* SQLTables(): no tables found. */
  ok_stmt(hstmt, SQLTables(hstmt, NULL, SQL_NTS, NULL, SQL_NTS,
                           (SQLCHAR *)"no_such_table", SQL_NTS, NULL, SQL_NTS));
  ok_stmt(hstmt, SQLNumResultCols(hstmt, &columns));
  is_num(columns, 5);
  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  return OK;
}


/**
 Bug #23031: SQLTables returns inaccurate catalog information on views
*/
DECLARE_TEST(t_bug23031)
{
  SQLCHAR buff[255];

  ok_sql(hstmt, "DROP VIEW IF EXISTS t_bug23031_v");
  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug23031_t");
  ok_sql(hstmt, "CREATE TABLE t_bug23031_t (a INT) COMMENT 'Whee!'");
  ok_sql(hstmt, "CREATE VIEW t_bug23031_v AS SELECT * FROM t_bug23031_t");

  /* Get both the table and view. */
  ok_stmt(hstmt, SQLTables(hstmt, NULL, SQL_NTS, NULL, SQL_NTS,
                           (SQLCHAR *)"t_bug23031%", SQL_NTS, NULL, SQL_NTS));

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_str(my_fetch_str(hstmt, buff, 3), "t_bug23031_t", 12);
  is_str(my_fetch_str(hstmt, buff, 4), "TABLE", 5);
  is_str(my_fetch_str(hstmt, buff, 5), "Whee!", 5);

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_str(my_fetch_str(hstmt, buff, 3), "t_bug23031_v", 12);
  is_str(my_fetch_str(hstmt, buff, 4), "VIEW", 4);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  /* Get just the table. */
  ok_stmt(hstmt, SQLTables(hstmt, NULL, SQL_NTS, NULL, SQL_NTS,
                           (SQLCHAR *)"t_bug23031%", SQL_NTS,
                           (SQLCHAR *)"TABLE", SQL_NTS));

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_str(my_fetch_str(hstmt, buff, 3), "t_bug23031_t", 12);
  is_str(my_fetch_str(hstmt, buff, 4), "TABLE", 5);
  is_str(my_fetch_str(hstmt, buff, 5), "Whee!", 5);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  /* Get just the view. */
  ok_stmt(hstmt, SQLTables(hstmt, NULL, SQL_NTS, NULL, SQL_NTS,
                           (SQLCHAR *)"t_bug23031%", SQL_NTS,
                           (SQLCHAR *)"'VIEW'", SQL_NTS));

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_str(my_fetch_str(hstmt, buff, 3), "t_bug23031_v", 12);
  is_str(my_fetch_str(hstmt, buff, 4), "VIEW", 4);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP VIEW IF EXISTS t_bug23031_v");
  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug23031_t");

  return OK;
}


/**
 Bug #15713: null pointer when use the table qualifier in SQLColumns()
*/
DECLARE_TEST(bug15713)
{
  HDBC hdbc1;
  HSTMT hstmt1;
  SQLCHAR   conn[256], conn_out[256];
  SQLSMALLINT conn_out_len;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug15713");
  ok_sql(hstmt, "CREATE TABLE t_bug15713 (a INT)");

  /* The connection strings must not include DATABASE. */
  sprintf((char *)conn, "DSN=%s;UID=%s;PASSWORD=%s", mydsn, myuid, mypwd);
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

  ok_env(henv, SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1));

  ok_con(hdbc1, SQLDriverConnect(hdbc1, NULL, conn, sizeof(conn), conn_out,
                                 sizeof(conn_out), &conn_out_len,
                                 SQL_DRIVER_NOPROMPT));
  ok_con(hdbc1, SQLAllocStmt(hdbc1, &hstmt1));

  ok_stmt(hstmt1, SQLColumns(hstmt1, (SQLCHAR *)"test", SQL_NTS,
                             NULL, 0, (SQLCHAR *)"t_bug15713", SQL_NTS,
                             NULL, 0));

  ok_stmt(hstmt1, SQLFetch(hstmt1));
  is_str(my_fetch_str(hstmt1, conn, 1), "test", 4);
  is_str(my_fetch_str(hstmt1, conn, 3), "t_bug15713", 10);
  is_str(my_fetch_str(hstmt1, conn, 4), "a", 1);

  expect_stmt(hstmt1, SQLFetch(hstmt1), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_DROP));
  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeHandle(SQL_HANDLE_DBC, hdbc1));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug15713");
  return OK;
}


/**
 Bug #28316: Fatal crash with Crystal Reports and MySQL Server
*/
DECLARE_TEST(t_bug28316)
{
  ok_stmt(hstmt, SQLProcedures(hstmt, NULL, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS));

  return OK;
}


/**
 Bug #8860: Generic SQLColumns not supported?
*/
DECLARE_TEST(bug8860)
{
  SQLCHAR buff[512];

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug8860, `t_bug8860_a'b`");
  ok_sql(hstmt, "CREATE TABLE t_bug8860 (a INT)");
  ok_sql(hstmt, "CREATE TABLE `t_bug8860_a'b` (b INT)");

  /*
   Specifying nothing gets us columns from all of the tables in the
   current database.
  */
  ok_stmt(hstmt, SQLColumns(hstmt, NULL, 0, NULL, 0, NULL, 0, NULL, 0));

  /* We should have at least two rows. There may be more. */
  is(myrowcount(hstmt) >= 2);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  /* Specifying "" as the table name gets us nothing. */
  ok_stmt(hstmt, SQLColumns(hstmt, NULL, 0, NULL, 0, (SQLCHAR *)"", SQL_NTS,
                            NULL, 0));

  is_num(myrowcount(hstmt), 0);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  /* Get the info from just one table.  */
  ok_stmt(hstmt, SQLColumns(hstmt, NULL, 0, NULL, 0, "t_bug8860", SQL_NTS,
                            NULL, 0));

  ok_stmt(hstmt, SQLFetch(hstmt));

  is_str(my_fetch_str(hstmt, buff, 3), "t_bug8860", 9);
  is_str(my_fetch_str(hstmt, buff, 4), "a", 1);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  /* Get the info from just one table with a funny name.  */
  ok_stmt(hstmt, SQLColumns(hstmt, NULL, 0, NULL, 0, "t_bug8860_a'b", SQL_NTS,
                            NULL, 0));

  ok_stmt(hstmt, SQLFetch(hstmt));

  is_str(my_fetch_str(hstmt, buff, 3), "t_bug8860_a'b", 13);
  is_str(my_fetch_str(hstmt, buff, 4), "b", 1);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  ok_sql(hstmt, "DROP TABLE t_bug8860, `t_bug8860_a'b`");
  return OK;
}

/**
 Bug #26934: SQLTables behavior has changed
*/
DECLARE_TEST(t_bug26934)
{
  HENV henv1;
  HDBC hdbc1;
  HSTMT hstmt1;

  alloc_basic_handles(&henv1, &hdbc1, &hstmt1);

  ok_sql(hstmt1, "SET @@wait_timeout = 1");
  sleep(2);
  expect_stmt(hstmt1, SQLTables(hstmt1, "%", 1, NULL, SQL_NTS, NULL, SQL_NTS,
                                NULL, SQL_NTS), SQL_ERROR);
  if (check_sqlstate(hstmt1, "08S01") != OK)
    return FAIL;

  free_basic_handles(&henv1, &hdbc1, &hstmt1);

  return OK;
}


/**
 Bug #29888: Crystal wizard throws error on including tables
*/
DECLARE_TEST(t_bug29888)
{
  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug29888");
  ok_sql(hstmt, "CREATE TABLE t_bug29888 (a INT, b INT)");

  ok_stmt(hstmt, SQLColumns(hstmt, mydb, SQL_NTS, NULL, SQL_NTS,
                            (SQLCHAR *)"t_bug29888", SQL_NTS,
                            (SQLCHAR *)"%", SQL_NTS));

  is_num(myrowcount(hstmt), 2);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug29888");

  return OK;
}


/**
  Bug #14407: SQLColumns gives wrong information of not nulls
*/
DECLARE_TEST(t_bug14407)
{
  SQLCHAR col[10];
  SQLSMALLINT nullable;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug14407");
  ok_sql(hstmt,
         "CREATE TABLE t_bug14407(a INT NOT NULL AUTO_INCREMENT PRIMARY KEY)");

  ok_stmt(hstmt, SQLColumns(hstmt, NULL, 0, NULL, 0,
                            (SQLCHAR *)"t_bug14407", SQL_NTS, NULL, 0));

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_str(my_fetch_str(hstmt, col, 4), "a", 1);
  is_num(my_fetch_int(hstmt, 11), SQL_NULLABLE);
  is_str(my_fetch_str(hstmt, col, 18), "YES", 3);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  /**
    Bug #26108  MyODBC ADO field attributes reporting adFldMayBeNull for
    not null columns
  */
  ok_sql(hstmt, "SELECT * FROM t_bug14407");

  ok_stmt(hstmt, SQLDescribeCol(hstmt, 1, NULL, 0, NULL, NULL, NULL, NULL,
                                &nullable));
  is_num(nullable, SQL_NULLABLE);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug14407");
  return OK;
}


/*
  Bug #32864 MyODBC /Crystal Reports truncated table names,
             lost fields when names > 21chars
*/
DECLARE_TEST(t_bug32864)
{
  SQLLEN dispsize= 0;
  SQLLEN colsize= 0;

  ok_stmt(hstmt, SQLTables(hstmt, "%", SQL_NTS, NULL, 0, NULL, 0, NULL, 0));
  ok_stmt(hstmt, SQLColAttribute(hstmt, 3, SQL_COLUMN_DISPLAY_SIZE, NULL, 0,
                                 NULL, &dispsize));

  is_num(dispsize, 64);

  ok_stmt(hstmt, SQLDescribeCol(hstmt, 3, NULL, 0, NULL, NULL,
                                &colsize, NULL, NULL));

  is_num(colsize, 64);

  return OK;
}


/*
  Bug #32989 - Crystal Reports fails if a field has a single quote
*/
DECLARE_TEST(t_bug32989)
{
  SQLCHAR name[20];
  SQLLEN len;

  ok_sql(hstmt, "drop table if exists t_bug32989");
  ok_sql(hstmt, "create table t_bug32989 (`doesn't work` int)");

  ok_stmt(hstmt, SQLColumns(hstmt, "test", SQL_NTS, NULL, 0,
                            "t_bug32989", SQL_NTS, NULL, 0));
  ok_stmt(hstmt, SQLFetch(hstmt));

  ok_stmt(hstmt, SQLGetData(hstmt, 4, SQL_C_CHAR, name, 20, &len));
  is_num(len, 12);
  is_str(name, "doesn't work", 13);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "drop table if exists t_bug32989");
  return OK;
}


/**
 Bug #33298: ADO returns wrong parameter count in the query (always 0)
*/
DECLARE_TEST(t_bug33298)
{
  SQLRETURN rc= 0;
  expect_stmt(hstmt, SQLProcedureColumns(hstmt, 0, NULL, 0, NULL,
                                         0, NULL, 0, NULL),
                                         SQL_SUCCESS_WITH_INFO);
  rc= SQLFetch(hstmt);
  /**
    SQL_NO_DATA is ok for the current implementation,
    SQL_SUCCESS should be ok for when SQLProcedureColumns is implemented
  */
  if (rc == SQL_ERROR)
    return FAIL;

  return OK;
}


/**
 Bug #12805: ADO failed to retrieve the length of LONGBLOB columns
*/
DECLARE_TEST(t_bug12805)
{
  SQLHENV    henv1;
  SQLHDBC    hdbc1;
  SQLHSTMT   hstmt1;
  SQLULEN    length;  

  SET_DSN_OPTION(1 << 27);

  alloc_basic_handles(&henv1, &hdbc1, &hstmt1);

  ok_sql(hstmt1, "DROP TABLE IF EXISTS bug12805");
  ok_sql(hstmt1, "CREATE TABLE bug12805("\
                 "id INT PRIMARY KEY auto_increment,"\
                 "longimagedata LONGBLOB NULL)");

  ok_stmt(hstmt1, SQLColumns(hstmt1, NULL, 0, NULL, 0,
                             (SQLCHAR *)"bug12805", SQL_NTS,
                             (SQLCHAR *)"longimagedata", SQL_NTS));

  ok_stmt(hstmt1, SQLFetch(hstmt1));
  ok_stmt(hstmt1, SQLGetData(hstmt1, 7, SQL_C_ULONG, &length,
                             sizeof(SQLULEN), NULL));
  is_num(length, 2147483647);
  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));

  length= 0;
  ok_sql(hstmt1, "SELECT * FROM bug12805");
  ok_stmt(hstmt1, SQLDescribeCol(hstmt1, 2, NULL, NULL, NULL, NULL,
                                 &length, NULL, NULL));
  is_num(length, 2147483647);

  length= 0;
  ok_stmt(hstmt1, SQLColAttribute(hstmt1, 2, SQL_DESC_PRECISION, NULL, 0,
                                 NULL, &length));
  
  is_num(length, 2147483647);
  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));
  free_basic_handles(&henv1, &hdbc1, &hstmt1);

  /* Check without the 32-bit signed flag */
  ok_stmt(hstmt, SQLColumns(hstmt, NULL, 0, NULL, 0,
                            (SQLCHAR *)"bug12805", SQL_NTS,
                            (SQLCHAR *)"longimagedata", SQL_NTS));

  ok_stmt(hstmt, SQLFetch(hstmt));
  ok_stmt(hstmt, SQLGetData(hstmt, 7, SQL_C_ULONG, &length,
                             sizeof(SQLULEN), NULL));
  is_num(length, 4294967295);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  length= 0;
  ok_sql(hstmt, "SELECT * FROM bug12805");
  ok_stmt(hstmt, SQLDescribeCol(hstmt, 2, NULL, NULL, NULL, NULL,
                                 &length, NULL, NULL));
  is_num(length, 4294967295);

  length= 0;
  ok_stmt(hstmt, SQLColAttribute(hstmt, 2, SQL_DESC_PRECISION, NULL, 0,
                                 NULL, &length));
  
  /* This length is always 2G */
  is_num(length, 2147483647);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  ok_sql(hstmt, "DROP TABLE bug12805");

  SET_DSN_OPTION(0);
  return OK;
}


BEGIN_TESTS
  ADD_TEST(my_columns_null)
  ADD_TEST(my_drop_table)
  ADD_TEST(my_table_dbs)
  ADD_TEST(my_colpriv)
  ADD_TEST(t_sqlprocedures)
  ADD_TEST(t_catalog)
  ADD_TEST(tmysql_specialcols)
  ADD_TEST(t_columns)
  ADD_TEST(t_tables_bug)
  ADD_TEST(t_current_catalog)
  ADD_TEST(tmysql_showkeys)
  ADD_TEST(t_sqltables)
  ADD_TEST(t_bug4518)
  ADD_TEST(empty_set)
  ADD_TEST(t_bug23031)
  ADD_TEST(bug15713)
  ADD_TEST(t_bug28316)
  ADD_TEST(bug8860)
  ADD_TEST(t_bug26934)
  ADD_TEST(t_bug29888)
  ADD_TEST(t_bug14407)
  ADD_TEST(t_bug32864)
  ADD_TEST(t_bug32989)
  ADD_TEST(t_bug33298)
  ADD_TEST(t_bug12805)
END_TESTS


RUN_TESTS
