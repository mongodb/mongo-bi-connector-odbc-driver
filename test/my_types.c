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


DECLARE_TEST(t_longlong1)
{
#if SQLBIGINT_MADE_PORTABLE || defined(_WIN32)
  SQLBIGINT session_id, ctn;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_longlong");
  ok_sql(hstmt, "CREATE TABLE t_longlong (a BIGINT, b BIGINT)");

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY,
                                (SQLPOINTER)SQL_CONCUR_ROWVER, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY,
                                (SQLPOINTER)SQL_CURSOR_STATIC, 0));

  ok_stmt(hstmt, SQLPrepare(hstmt,
                            (SQLCHAR *)"INSERT INTO t_longlong VALUES (?,?)",
                            SQL_NTS));

  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_UBIGINT,
                                  SQL_BIGINT, 20, 0, &session_id, 20, NULL));
  ok_stmt(hstmt, SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_UBIGINT,
                                  SQL_BIGINT, 20, 0, &ctn, 20, NULL));

  for (session_id= 50, ctn= 0; session_id < 100; session_id++)
  {
    ctn+= session_id;
    ok_stmt(hstmt, SQLExecute(hstmt));
  }

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_RESET_PARAMS));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "SELECT * FROM t_longlong");

  my_assert(50 == myresult(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_longlong");
#endif /* SQLBIGINT_MADE_PORTABLE || defined(_WIN32) */

  return OK;
}


/* This just proves we don't support SQL_C_NUMERIC. */
DECLARE_TEST(t_numeric)
{
  SQL_NUMERIC_STRUCT num;
  SQLINTEGER dummy;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_decimal");
  ok_sql(hstmt, "CREATE TABLE t_decimal (d1 DECIMAL(10,6))");
  ok_sql(hstmt, "INSERT INTO t_decimal VALUES (10.2)");

  ok_stmt(hstmt, SQLPrepare(hstmt,
                            (SQLCHAR *)"INSERT INTO t_decimal VALUES (?)",
                            SQL_NTS));

  expect_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_NUMERIC,
                                      SQL_DECIMAL, 10, 4, &num, 0, NULL),
              SQL_ERROR);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLPrepare(hstmt,
                            (SQLCHAR *)"INSERT INTO t_decimal VALUES (?),(?)",
                            SQL_NTS));
  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG,
                                  SQL_DECIMAL, 10, 4, &dummy, 0, NULL));

  expect_stmt(hstmt, SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_NUMERIC,
                                      SQL_DECIMAL, 10, 4, &num, 0, NULL),
              SQL_ERROR);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_RESET_PARAMS));

  expect_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_NUMERIC, &num, 0, NULL),
              SQL_ERROR);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));

  ok_sql(hstmt, "SELECT * FROM t_decimal");

  ok_stmt(hstmt, SQLFetch(hstmt));

  expect_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_NUMERIC, &num, 0, NULL),
              SQL_ERROR);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_RESET_PARAMS));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_decimal");

  return OK;
}


DECLARE_TEST(t_decimal)
{
  SQLCHAR         str[20],s_data[]="189.4567";
  SQLDOUBLE       d_data=189.4567;
  SQLINTEGER      i_data=189, l_data=-23;
  SQLRETURN       rc;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_decimal");
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

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_decimal");

  return OK;
}


DECLARE_TEST(t_bigint)
{
#if SQLBIGINT_MADE_PORTABLE || defined(_WIN32)
    SQLRETURN rc;
    SQLLEN nlen = 4;
    union {                    /* An union to get 4 byte alignment */
      SQLCHAR buf[20];
      SQLINTEGER dummy;
    } id = {"99998888"};       /* Just to get a binary pattern for some 64 bit big int */

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

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_LONG,
                          SQL_BIGINT,0,0,&id.buf,sizeof(id.buf),&nlen);
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

    rc = SQLGetData(hstmt,1,SQL_C_DEFAULT,&id.buf,sizeof(id.buf),&nlen);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

#endif /* SQLBIGINT_MADE_PORTABLE || defined(_WIN32) */
  return OK;
}


DECLARE_TEST(t_enumset)
{
    SQLRETURN rc;
    SQLCHAR szEnum[40]="MYSQL_E1";
    SQLCHAR szSet[40]="THREE,ONE,TWO";

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_enumset");

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

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_enumset");

  return OK;
}


/**
 Bug #16917: MyODBC doesn't return ASCII 0 characters for TEXT columns
*/
DECLARE_TEST(t_bug16917)
{
  SQLCHAR buff[255];
  SQLLEN  len;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug16917");
  ok_sql(hstmt, "CREATE TABLE t_bug16917 (a TEXT)");
  ok_sql(hstmt, "INSERT INTO t_bug16917 VALUES ('a\\0b')");

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "SELECT a FROM t_bug16917");

  ok_stmt(hstmt, SQLFetch(hstmt));

  /* This SQLSetPos() causes the field lengths to get lost. */
  ok_stmt(hstmt, SQLSetPos(hstmt, 1, SQL_POSITION, SQL_LOCK_NO_CHANGE));

  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, buff, 0, &len));
  is_num(len, 3);

  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, buff, sizeof(buff), &len));
  is_num(buff[0], 'a');
  is_num(buff[1], 0);
  is_num(buff[2], 'b');
  is_num(len, 3);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug16917");
  return OK;
}


/**
 Bug #16235: ODBC driver doesn't format parameters correctly
*/
DECLARE_TEST(t_bug16235)
{
  SQLCHAR varchar[]= "a'b", text[]= "c'd", buff[10];
  SQLLEN varchar_len= SQL_NTS, text_len= SQL_NTS;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug16235");
  ok_sql(hstmt, "CREATE TABLE t_bug16235 (a NVARCHAR(20), b TEXT)");

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLPrepare(hstmt, (SQLCHAR *)
                            "INSERT INTO t_bug16235 VALUES (?,?)", SQL_NTS));
  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR,
                                  SQL_WVARCHAR, 0, 0, varchar, sizeof(varchar),
                                  &varchar_len));
  ok_stmt(hstmt, SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR,
                                  SQL_WLONGVARCHAR, 0, 0, text, sizeof(text),
                                  &text_len));

  ok_stmt(hstmt, SQLExecute(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "SELECT * FROM t_bug16235");

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_str(my_fetch_str(hstmt, buff, 1), "a'b", 3);
  is_str(my_fetch_str(hstmt, buff, 2), "c'd", 3);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug16235");

  return OK;
}


/**
 Bug #27862: Function return incorrect SQL_COLUMN_SIZE
*/
DECLARE_TEST(t_bug27862_1)
{
  SQLLEN len;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug27862");
  ok_sql(hstmt, "CREATE TABLE t_bug27862 (a VARCHAR(2), b VARCHAR(2))");
  ok_sql(hstmt, "INSERT INTO t_bug27862 VALUES ('a','b')");

  ok_sql(hstmt, "SELECT CONCAT(a,b) FROM t_bug27862");

  ok_stmt(hstmt, SQLColAttribute(hstmt, 1, SQL_DESC_DISPLAY_SIZE, NULL, 0,
                                 NULL, &len));
  is_num(len, 4);
  ok_stmt(hstmt, SQLColAttribute(hstmt, 1, SQL_DESC_LENGTH, NULL, 0,
                                 NULL, &len));
  is_num(len, 2);
  ok_stmt(hstmt, SQLColAttribute(hstmt, 1, SQL_DESC_OCTET_LENGTH, NULL, 0,
                                 NULL, &len));
  is_num(len, 5);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug27862");

  return OK;
}


/**
 Because integers are given the charset 63 (binary) when they are
 used as strings, functions like CONCAT() return a binary string.
 This is a server bug that we do not try to work around.
*/
DECLARE_TEST(t_bug27862_2)
{
  SQLLEN len;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug27862");
  ok_sql(hstmt, "CREATE TABLE t_bug27862 (c DATE, d INT)");
  ok_sql(hstmt, "INSERT INTO t_bug27862 VALUES ('2007-01-13',5)");

  ok_sql(hstmt, "SELECT CONCAT_WS(' - ', DATE_FORMAT(c, '%b-%d-%y'), d) "
         "FROM t_bug27862");

  ok_stmt(hstmt, SQLColAttribute(hstmt, 1, SQL_DESC_DISPLAY_SIZE, NULL, 0,
                                 NULL, &len));
  is_num(len, 104);
  ok_stmt(hstmt, SQLColAttribute(hstmt, 1, SQL_DESC_LENGTH, NULL, 0,
                                 NULL, &len));
  is_num(len, 26);
  ok_stmt(hstmt, SQLColAttribute(hstmt, 1, SQL_DESC_OCTET_LENGTH, NULL, 0,
                                 NULL, &len));
  is_num(len, 27);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug27862");

  return OK;
}


/**
  SQL_DESC_FIXED_PREC_SCALE was wrong for new DECIMAL types.
*/
DECLARE_TEST(decimal_scale)
{
  SQLINTEGER fixed= SQL_FALSE;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_decscale");
  ok_sql(hstmt, "CREATE TABLE t_decscale (a DECIMAL(5,3))");

  ok_sql(hstmt, "SELECT * FROM t_decscale");

  ok_stmt(hstmt, SQLColAttribute(hstmt, 1, SQL_DESC_FIXED_PREC_SCALE,
                                 NULL, 0, NULL, &fixed));

  is_num(fixed, SQL_FALSE);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_decscale");
  return OK;
}


/**
  Wrong value returned for SQL_DESC_LITERAL_SUFFIX for binary field.
*/
DECLARE_TEST(binary_suffix)
{
  SQLCHAR suffix[10];
  SQLSMALLINT len;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_binarysuffix");
  ok_sql(hstmt, "CREATE TABLE t_binarysuffix (a BINARY(10))");

  ok_sql(hstmt, "SELECT * FROM t_binarysuffix");

  ok_stmt(hstmt, SQLColAttribute(hstmt, 1, SQL_DESC_LITERAL_SUFFIX,
                                 suffix, 10, &len, NULL));

  is_num(len, 0);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_binarysuffix");
  return OK;
}


/**
  Wrong value returned for SQL_DESC_SCALE for float and double.
*/
DECLARE_TEST(float_scale)
{
  SQLINTEGER scale;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_floatscale");
  ok_sql(hstmt, "CREATE TABLE t_floatscale(a FLOAT, b DOUBLE, c DECIMAL(3,2))");

  ok_sql(hstmt, "SELECT * FROM t_floatscale");

  ok_stmt(hstmt, SQLColAttribute(hstmt, 1, SQL_DESC_SCALE,
                                 NULL, 0, NULL, &scale));

  is_num(scale, 0);

  ok_stmt(hstmt, SQLColAttribute(hstmt, 2, SQL_DESC_SCALE,
                                 NULL, 0, NULL, &scale));

  is_num(scale, 0);

  ok_stmt(hstmt, SQLColAttribute(hstmt, 3, SQL_DESC_SCALE,
                                 NULL, 0, NULL, &scale));

  is_num(scale, 2);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_floatscale");
  return OK;
}


/**
  Test the BIT type, which has different behavior for BIT(1) and BIT(n > 1).
*/
DECLARE_TEST(bit)
{
  SQLCHAR col[10];
  SQLINTEGER type;
  SQLLEN len;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bit");
  ok_sql(hstmt, "CREATE TABLE t_bit (a BIT(1), b BIT(17))");

  ok_stmt(hstmt, SQLColumns(hstmt, NULL, 0, NULL, 0,
                            (SQLCHAR *)"t_bit", SQL_NTS, NULL, 0));

  ok_stmt(hstmt, SQLFetch(hstmt));

  is_str(my_fetch_str(hstmt, col, 4), "a", 1);
  is_num(my_fetch_int(hstmt, 5), SQL_BIT); /* DATA_TYPE */
  is_num(my_fetch_int(hstmt, 7), 1); /* COLUMN_SIZE */
  is_num(my_fetch_int(hstmt, 8), 1); /* BUFFER_LENGTH */
  ok_stmt(hstmt, SQLGetData(hstmt, 16, SQL_C_LONG, &type, 0, &len));
  is_num(len, SQL_NULL_DATA); /* CHAR_OCTET_LENGTH */

  ok_stmt(hstmt, SQLFetch(hstmt));

  is_str(my_fetch_str(hstmt, col, 4), "b", 1);
  is_num(my_fetch_int(hstmt, 5), SQL_BINARY); /* DATA_TYPE */
  is_num(my_fetch_int(hstmt, 7), 3); /* COLUMN_SIZE */
  is_num(my_fetch_int(hstmt, 8), 3); /* BUFFER_LENGTH */
  is_num(my_fetch_int(hstmt, 16), 3); /* CHAR_OCTET_LENGTH */

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "SELECT * FROM t_bit");

  ok_stmt(hstmt, SQLColAttribute(hstmt, 1, SQL_DESC_TYPE, NULL, 0, NULL,
                                 &type));
  is_num(type, SQL_BIT);

  ok_stmt(hstmt, SQLColAttribute(hstmt, 2, SQL_DESC_TYPE, NULL, 0, NULL,
                                 &type));
  is_num(type, SQL_BINARY);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bit");
  return OK;
}


/**
 Bug #32171: ODBC Driver incorrectly parses large Unsigned Integers
*/
DECLARE_TEST(t_bug32171)
{
  SQLUINTEGER in= 4255080020, out;
  SQLCHAR buff[128];

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug32171");
  ok_sql(hstmt, "CREATE TABLE t_bug32171 (a INT UNSIGNED)");

  sprintf((char *)buff, "INSERT INTO t_bug32171 VALUES ('%u')", in);
  ok_stmt(hstmt, SQLExecDirect(hstmt, buff, SQL_NTS));

  ok_sql(hstmt, "SELECT * FROM t_bug32171");

  ok_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_ULONG, &out, 0, NULL));
  ok_stmt(hstmt, SQLFetch(hstmt));

  is_num(out, in);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug32171");

  return OK;
}


/*
  Bug #31220 - SQLFetch or SQLFetchScroll returns negative data length
               when using SQL_C_WCHAR
*/
DECLARE_TEST(t_bug31220)
{
  SQLLEN outlen= 999;
  SQLWCHAR outbuf[5];

  /* the call sequence of this test is not allowed under a driver manager */
  if (using_dm(hdbc))
    return OK;

  ok_sql(hstmt, "select 1");
  ok_stmt(hstmt, SQLBindCol(hstmt, 1, 999 /* unknown type */,
                            outbuf, 5, &outlen));
  expect_stmt(hstmt, SQLFetch(hstmt), SQL_ERROR);
  is(check_sqlstate(hstmt, "07006") == OK);
  is_num(outlen, 999);
  return OK;  
}


BEGIN_TESTS
  ADD_TEST(t_longlong1)
  ADD_TEST(t_numeric)
  ADD_TEST(t_decimal)
  ADD_TEST(t_bigint)
  ADD_TEST(t_enumset)
  ADD_TEST(t_bug16917)
  ADD_TEST(t_bug16235)
  ADD_TEST(t_bug27862_1)
  ADD_TODO(t_bug27862_2)
  ADD_TEST(decimal_scale)
  ADD_TEST(binary_suffix)
  ADD_TEST(float_scale)
  ADD_TEST(bit)
  ADD_TEST(t_bug32171)
  ADD_TEST(t_bug31220)
END_TESTS


RUN_TESTS
