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

    rc = SQLPrepare(hstmt, (SQLCHAR *)"insert into t_decimal values (?),(?),(?),(?)",SQL_NTS);
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

    ok_sql(hstmt, "select d1 from t_decimal");

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,&str,19,NULL);
    mystmt(hstmt,rc);
    fprintf(stdout,"decimal(SQL_C_DOUBLE) : %s\n",str);
    is_str(str, "189.456700", 11);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,&str,19,NULL);
    mystmt(hstmt,rc);
    fprintf(stdout,"decimal(SQL_C_INTEGER): %s\n",str);
    is_str(str,"189.000000",11);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,&str,19,NULL);
    mystmt(hstmt,rc);
    fprintf(stdout,"decimal(SQL_C_CHAR)   : %s\n",str);
    is_str(str,"189.456700",11);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,&str,19,NULL);
    mystmt(hstmt,rc);
    fprintf(stdout,"decimal(SQL_C_LONG)   : %s\n",str);
    is_str(str, "-23.000000", 11);

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

    ok_sql(hstmt, "insert into t_enumset values('MYSQL_E2','TWO,THREE')");

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    ok_stmt(hstmt, SQLPrepare(hstmt, (SQLCHAR *)
                              "insert into t_enumset values(?,?)", SQL_NTS));

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
  is_num(len, 4);
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
  SQLINTEGER prec, scale;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_decscale");
  ok_sql(hstmt, "CREATE TABLE t_decscale (a DECIMAL(5,3))");

  ok_sql(hstmt, "SELECT * FROM t_decscale");

  ok_stmt(hstmt, SQLColAttribute(hstmt, 1, SQL_DESC_FIXED_PREC_SCALE,
                                 NULL, 0, NULL, &fixed));
  ok_stmt(hstmt, SQLColAttribute(hstmt, 1, SQL_DESC_PRECISION,
                                 NULL, 0, NULL, &prec));
  ok_stmt(hstmt, SQLColAttribute(hstmt, 1, SQL_DESC_SCALE,
                                 NULL, 0, NULL, &scale));

  is_num(fixed, SQL_FALSE);
  is_num(prec, 5);
  is_num(scale, 3);

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
  SQLUINTEGER in= 4255080020UL, out;
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


/** Test passing an SQL_C_CHAR to a SQL_WCHAR field. */
DECLARE_TEST(sqlwchar)
{
  /* Note: this is an SQLCHAR, so it is 'ANSI' data. */
  SQLCHAR data[]= "S\xe3o Paolo", buff[30];
  SQLWCHAR wbuff[30]= {0};
  wchar_t wcdata[]= L"S\x00e3o Paolo";

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_sqlwchar");
  ok_sql(hstmt, "CREATE TABLE t_sqlwchar (a VARCHAR(30)) DEFAULT CHARSET utf8");

  ok_stmt(hstmt, SQLPrepare(hstmt, (SQLCHAR *)
                            "INSERT INTO t_sqlwchar VALUES (?)", SQL_NTS));
  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR,
                                  SQL_WVARCHAR, 0, 0, data, sizeof(data),
                                  NULL));
  ok_stmt(hstmt, SQLExecute(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_RESET_PARAMS));

  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR,
                                  SQL_WVARCHAR, 0, 0, W(wcdata),
                                  sizeof(wcdata), NULL));
  ok_stmt(hstmt, SQLExecute(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_RESET_PARAMS));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "SELECT HEX(a) FROM t_sqlwchar");

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_str(my_fetch_str(hstmt, buff, 1), "53C3A36F2050616F6C6F", 20);

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_str(my_fetch_str(hstmt, buff, 1), "53C3A36F2050616F6C6F", 20);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "SELECT a FROM t_sqlwchar");

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_str(my_fetch_str(hstmt, buff, 1), data, sizeof(data));

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_str(my_fetch_str(hstmt, buff, 1), data, sizeof(data));

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "SELECT a FROM t_sqlwchar");

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_wstr(my_fetch_wstr(hstmt, wbuff, 1), wcdata, 9);

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_wstr(my_fetch_wstr(hstmt, wbuff, 1), wcdata, 9);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_sqlwchar");
  return OK;
}


/*
   This is from the canonical example of retrieving a SQL_NUMERIC_STRUCT:
   http://support.microsoft.com/default.aspx/kb/222831
*/
DECLARE_TEST(t_sqlnum_msdn)
{
  SQLHANDLE ard;
  SQL_NUMERIC_STRUCT *sqlnum= malloc(sizeof(SQL_NUMERIC_STRUCT));
  SQLCHAR exp_data[SQL_MAX_NUMERIC_LEN]=
          {0x7c, 0x62, 0,0,0,0,0,0,0,0,0,0,0,0,0,0};

  sqlnum->sign= sqlnum->precision= sqlnum->scale= 128;

  ok_sql(hstmt, "select 25.212");
  ok_stmt(hstmt, SQLGetStmtAttr(hstmt, SQL_ATTR_APP_ROW_DESC, &ard, 0, NULL));

  ok_desc(ard, SQLSetDescField(ard, 1, SQL_DESC_TYPE,
                               (SQLPOINTER) SQL_C_NUMERIC, SQL_IS_INTEGER));
  ok_desc(ard, SQLSetDescField(ard, 1, SQL_DESC_SCALE,
                               (SQLPOINTER) 3, SQL_IS_INTEGER));
  ok_desc(ard, SQLSetDescField(ard, 1, SQL_DESC_DATA_PTR,
                               sqlnum, SQL_IS_POINTER));

  ok_stmt(hstmt, SQLFetch(hstmt));

  is_num(sqlnum->sign, 1);
  is_num(sqlnum->precision, 38);
  is_num(sqlnum->scale, 3);
  is(!memcmp(sqlnum->val, exp_data, SQL_MAX_NUMERIC_LEN));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  free(sqlnum);
  return OK;
}


/**
  Internal function to test retrieving a SQL_NUMERIC_STRUCT value.

  @todo Printing some additional output (sqlnum->val as hex, dec)

  @param[in]  hstmt       Statement handle
  @param[in]  numstr      String to retrieve as SQL_NUMERIC_STRUCT
  @param[in]  prec        Precision to retrieve
  @param[in]  scale       Scale to retrieve
  @param[in]  sign        Expected sign (1=+,0=-)
  @param[in]  expdata     Expected byte array value (need this or expnum)
  @param[in]  expnum      Expected numeric value (if it fits)
  @param[in]  overflow    Whether to expect a retrieval failure (22003)
  @return OK/FAIL just like a test.
*/
int sqlnum_test_from_str(SQLHANDLE hstmt,
                         const char *numstr, SQLCHAR prec, SQLSCHAR scale,
                         SQLCHAR sign, SQLCHAR *expdata, int expnum,
                         int overflow)
{
  SQL_NUMERIC_STRUCT *sqlnum= malloc(sizeof(SQL_NUMERIC_STRUCT));
  SQLCHAR buf[512];
  SQLHANDLE ard;
  unsigned long numval;

  sprintf((char *)buf, "select %s", numstr);
  /* ok_sql(hstmt, buf); */
  ok_stmt(hstmt, SQLExecDirect(hstmt, buf, SQL_NTS));

  ok_stmt(hstmt, SQLGetStmtAttr(hstmt, SQL_ATTR_APP_ROW_DESC, &ard, 0, NULL));

  ok_desc(ard, SQLSetDescField(ard, 1, SQL_DESC_TYPE,
                               (SQLPOINTER) SQL_C_NUMERIC, SQL_IS_INTEGER));
  ok_desc(ard, SQLSetDescField(ard, 1, SQL_DESC_PRECISION,
                               (SQLPOINTER)(SQLINTEGER) prec, SQL_IS_INTEGER));
  ok_desc(ard, SQLSetDescField(ard, 1, SQL_DESC_SCALE,
                               (SQLPOINTER)(SQLINTEGER) scale, SQL_IS_INTEGER));
  ok_desc(ard, SQLSetDescField(ard, 1, SQL_DESC_DATA_PTR,
                               sqlnum, SQL_IS_POINTER));

  if (overflow)
  {
    expect_stmt(hstmt, SQLFetch(hstmt), SQL_ERROR);
    return check_sqlstate(hstmt, "22003");
  }
  else
    ok_stmt(hstmt, SQLFetch(hstmt));

  is_num(sqlnum->precision, prec);
  is_num(sqlnum->scale, scale);
  is_num(sqlnum->sign, sign);
  if (expdata)
  {
    is(!memcmp(sqlnum->val, expdata, SQL_MAX_NUMERIC_LEN));
  }
  else
  {
    /* only use this for <=32bit values */
    int i;
    numval= 0;
    for (i= 0; i < 8; ++i)
      numval += sqlnum->val[7 - i] << (8 * (7 - i));
    is_num(numval, expnum);
  }

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  free(sqlnum);
  return OK;
}


/*
   Testing of retrieving SQL_NUMERIC_STRUCT

   Make sure to use little-endian in SQLCHAR arrays.
*/
DECLARE_TEST(t_sqlnum_from_str)
{
  char *num1= "25.212";
  char *num2= "-101234.0010";
  char *num3= "-101230.0010";

  /* some basic tests including min-precision and scale changes */
  is(sqlnum_test_from_str(hstmt, num1, 5, 3, 1, NULL, 25212, 0) == OK);
  is(sqlnum_test_from_str(hstmt, num1, 4, 3, 1, NULL, 0, 1) == OK);
  is(sqlnum_test_from_str(hstmt, num1, 4, 2, 1, NULL, 2521, 0) == OK);
  is(sqlnum_test_from_str(hstmt, num1, 2, 0, 1, NULL, 25, 0) == OK);
  is(sqlnum_test_from_str(hstmt, num1, 2, -1, 1, NULL, 0, 1) == OK);

  /* more comprehensive testing of scale and precision */
  {SQLCHAR expdata[]= {0x2a, 0x15, 0x57, 0x3c, 0,0,0,0,0,0,0,0,0,0,0,0};
   is(sqlnum_test_from_str(hstmt, num2, 9, 4, 0, expdata, 0, 0) == OK);}
  is(sqlnum_test_from_str(hstmt, num2, 9, 4, 0, NULL, 1012340010, 0) == OK);
  is(sqlnum_test_from_str(hstmt, num2, 9, 3, 0, NULL, 101234001, 0) == OK);
  is(sqlnum_test_from_str(hstmt, num2, 8, 3, 0, NULL, 0, 1) == OK);
  is(sqlnum_test_from_str(hstmt, num2, 8, 2, 0, NULL, 10123400, 0) == OK);
  is(sqlnum_test_from_str(hstmt, num2, 7, 2, 0, NULL, 10123400, 0) == OK);
  is(sqlnum_test_from_str(hstmt, num2, 6, 2, 0, NULL, 10123400, 0) == OK);
  is(sqlnum_test_from_str(hstmt, num2, 6, 1, 0, NULL, 1012340, 0) == OK);
  is(sqlnum_test_from_str(hstmt, num2, 6, 0, 0, NULL, 101234, 0) == OK);
  is(sqlnum_test_from_str(hstmt, num2, 6, -1, 0, NULL, 0, 1) == OK);

  is(sqlnum_test_from_str(hstmt, num3, 6, -1, 0, NULL, 10123, 0) == OK);
  is(sqlnum_test_from_str(hstmt, num3, 5, -1, 0, NULL, 10123, 0) == OK);

  /* Bug#35920 */
  is(sqlnum_test_from_str(hstmt, "8000.00", 30, 2, 1, NULL, 800000, 0) == OK);
  is(sqlnum_test_from_str(hstmt, "1234567.00", 30, 2, 1, NULL, 123456700, 0) == OK);

  /* some larger numbers */
  {SQLCHAR expdata[SQL_MAX_NUMERIC_LEN]= {0xD5, 0x50, 0x94, 0x49, 0,0,0,0,0,0,0,0,0,0,0,0};
   is(OK == sqlnum_test_from_str(hstmt, "1234456789", 10, 0, 1, expdata, 0, 0));}
  {SQLCHAR expdata[SQL_MAX_NUMERIC_LEN]= {0x7c, 0x62, 0,0, 0,0,0,0,0,0,0,0,0,0,0,0};
   is(OK == sqlnum_test_from_str(hstmt, "25.212", 5, 3, 1, expdata, 0, 0));}
  {SQLCHAR expdata[SQL_MAX_NUMERIC_LEN]= {0xaa, 0x86, 0x1, 0, 0,0,0,0,0,0,0,0,0,0,0,0};
   is(OK == sqlnum_test_from_str(hstmt, "10.0010", 6, 4, 1, expdata, 0, 0));}
  {SQLCHAR expdata[SQL_MAX_NUMERIC_LEN]= {0x2a, 0x15, 0x57, 0x3c, 0,0,0,0,0,0,0,0,0,0,0,0};
   is(OK == sqlnum_test_from_str(hstmt, "-101234.0010", 10, 4, 0, expdata, 0, 0));}
  {SQLCHAR expdata[SQL_MAX_NUMERIC_LEN]= {0x72, 0x8b, 0x1, 0, 0,0,0,0,0,0,0,0,0,0,0,0};
   is(OK == sqlnum_test_from_str(hstmt, "101234", 6, 0, 1, expdata, 0, 0));}
  {SQLCHAR expdata[SQL_MAX_NUMERIC_LEN]= {0x97, 0x03, 0x7C, 0xE3, 0x76, 0x5E, 0xF0, 0x00, 0x24, 0x1A, 0,0,0,0,0,0};
   is(OK == sqlnum_test_from_str(hstmt, "123445678999123445678999", 24, 0, 1, expdata, 0, 0));}
  {SQLCHAR expdata[SQL_MAX_NUMERIC_LEN]= {0x95, 0xFA, 0x0B, 0xF1, 0xED, 0x3C, 0x7C, 0xE4, 0x1B, 0x5F, 0x80, 0x1A, 0x16, 0x06, 0,0};
   is(OK == sqlnum_test_from_str(hstmt, "123445678999123445678999543216789", 33, 0, 1, expdata, 0, 0));}
  {SQLCHAR expdata[SQL_MAX_NUMERIC_LEN]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   is(OK == sqlnum_test_from_str(hstmt, "12344567899912344567899954321678909876543212", 44, 0, 1, expdata, 0, 1));}
  /* overflow with dec pt after the overflow */
  {SQLCHAR expdata[SQL_MAX_NUMERIC_LEN]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   is(OK == sqlnum_test_from_str(hstmt, "1234456789991234456789995432167890987654321.2", 44, 1, 1, expdata, 0, 1));}
  {SQLCHAR expdata[SQL_MAX_NUMERIC_LEN]= {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
   is(OK == sqlnum_test_from_str(hstmt, "340282366920938463463374607431768211455", 39, 0, 1, expdata, 0, 0)); /* MAX */}
  {SQLCHAR expdata[SQL_MAX_NUMERIC_LEN]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   is(OK == sqlnum_test_from_str(hstmt, "340282366920938463463374607431768211456", 39, 0, 1, expdata, 0, 1)); /* MAX+1 */}
  {SQLCHAR expdata[SQL_MAX_NUMERIC_LEN]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   is(OK == sqlnum_test_from_str(hstmt, "0", 1, 0, 1, expdata, 0, 0));}

  return OK;
}


/*
   Basic test of binding a SQL_NUMERIC_STRUCT as a query parameter
*/
DECLARE_TEST(t_bindsqlnum_basic)
{
  SQL_NUMERIC_STRUCT *sqlnum= malloc(sizeof(SQL_NUMERIC_STRUCT));
  SQLCHAR outstr[20];
  memset(sqlnum, 0, sizeof(SQL_NUMERIC_STRUCT));

  sqlnum->sign= 1;
  sqlnum->val[0]= 0x7c;
  sqlnum->val[1]= 0x62;

  ok_stmt(hstmt, SQLPrepare(hstmt, (SQLCHAR *)"select ?", SQL_NTS));

  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_NUMERIC,
                                  SQL_DECIMAL, 5, 3,
                                  sqlnum, 0, NULL));

  ok_stmt(hstmt, SQLExecute(hstmt));
  ok_stmt(hstmt, SQLFetch(hstmt));
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, outstr, 20, NULL));
  is_str(outstr, "25.212", 6);
  is_num(sqlnum->sign, 1);
  is_num(sqlnum->precision, 5);
  is_num(sqlnum->scale, 3);

  return OK;
}


/**
  Internal function to test sending a SQL_NUMERIC_STRUCT value.

  @todo Printing some additional output (sqlnum->val as hex, dec)

  @param[in]  hstmt       Statement handle
  @param[in]  numdata     Numeric data
  @param[in]  prec        Precision to send
  @param[in]  scale       Scale to send
  @param[in]  sign        Sign (1=+,0=-)
  @param[in]  outstr      Expected result string
  @param[in]  exptrunc    Expected truncation failure
  @return OK/FAIL just like a test.
*/
int sqlnum_test_to_str(SQLHANDLE hstmt, SQLCHAR *numdata, SQLCHAR prec,
                       SQLSCHAR scale, SQLCHAR sign, char *outstr,
                       char *exptrunc)
{
  SQL_NUMERIC_STRUCT *sqlnum= malloc(sizeof(SQL_NUMERIC_STRUCT));
  SQLCHAR obuf[30];
  SQLRETURN exprc= SQL_SUCCESS;

  /* TODO until sqlnum errors are supported */
  /*
  if (!strcmp("01S07", exptrunc))
    exprc= SQL_SUCCESS_WITH_INFO;
  else if (!strcmp("22003", exptrunc))
    exprc= SQL_ERROR;
  */

  sqlnum->sign= sign;
  memcpy(sqlnum->val, numdata, SQL_MAX_NUMERIC_LEN);

  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_NUMERIC,
                                  SQL_DECIMAL, prec, scale, sqlnum, 0, NULL));

  ok_sql(hstmt, "select ?");

  expect_stmt(hstmt, SQLFetch(hstmt), exprc);
  if (exprc != SQL_SUCCESS)
  {
    is(check_sqlstate(hstmt, (char *)exptrunc) == OK);
  }
  if (exprc == SQL_ERROR)
    return OK;
  is_num(sqlnum->precision, prec);
  is_num(sqlnum->scale, scale);
  is_num(sqlnum->sign, sign);
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, obuf, sizeof(obuf), NULL));
  is_str(obuf, outstr, strlen((char *)outstr));
  is(!memcmp(sqlnum->val, numdata, SQL_MAX_NUMERIC_LEN));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  free(sqlnum);
  return OK;
}


/*
   Testing of passing SQL_NUMERIC_STRUCT as query parameters
*/
DECLARE_TEST(t_sqlnum_to_str)
{
  {SQLCHAR numdata[]= {0xD5, 0x50, 0x94, 0x49, 0,0,0,0,0,0,0,0,0,0,0,0};
   is(OK == sqlnum_test_to_str(hstmt, numdata, 10, 4, 1, "123445.6789", ""));}

  /* fractional truncation */
  {SQLCHAR numdata[]= {0xD5, 0x50, 0x94, 0x49, 0,0,0,0,0,0,0,0,0,0,0,0};
   is(OK == sqlnum_test_to_str(hstmt, numdata, 9, 2, 1, "12344567.8", "01S07"));}
  {SQLCHAR numdata[]= {0xD5, 0x50, 0x94, 0x49, 0,0,0,0,0,0,0,0,0,0,0,0};
   is(OK == sqlnum_test_to_str(hstmt, numdata, 8, 2, 1, "12344567", "01S07"));}

  /* whole number truncation - error */
  /* TODO need err handling for this test
  {SQLCHAR numdata[]= {0xD5, 0x50, 0x94, 0x49, 0,0,0,0,0,0,0,0,0,0,0,0};
   is(OK == sqlnum_test_to_str(hstmt, numdata, 7, 2, 1, "1234456", "22003"));}
  */

  /* negative scale */
  {SQLCHAR numdata[]= {0xD5, 0x50, 0x94, 0x49, 0,0,0,0,0,0,0,0,0,0,0,0};
   is(OK == sqlnum_test_to_str(hstmt, numdata, 10, -2, 1, "123445678900", ""));}
  {SQLCHAR numdata[]= {0xD5, 0x50, 0x94, 0x49, 0,0,0,0,0,0,0,0,0,0,0,0};
   is(OK == sqlnum_test_to_str(hstmt, numdata, 10, -2, 0, "-123445678900", ""));}

  /* scale > prec */
  {SQLCHAR numdata[]= {0xD5, 0x50, 0x94, 0x49, 0,0,0,0,0,0,0,0,0,0,0,0};
   is(OK == sqlnum_test_to_str(hstmt, numdata, 10, 11, 1, "0.01234456789", ""));}
  {SQLCHAR numdata[]= {0xD5, 0x50, 0x94, 0x49, 0,0,0,0,0,0,0,0,0,0,0,0};
   is(OK == sqlnum_test_to_str(hstmt, numdata, 10, 11, 0, "-0.01234456789", ""));}
  {SQLCHAR numdata[]= {0xD5, 0x50, 0x94, 0x49, 0,0,0,0,0,0,0,0,0,0,0,0};
   is(OK == sqlnum_test_to_str(hstmt, numdata, 10, 20, 1, "0.00000000001234456789", ""));}

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


/**
  Bug #29402: field type charset 63 problem
*/
DECLARE_TEST(t_bug29402)
{
  SQLSMALLINT name_length, data_type, decimal_digits, nullable;
  SQLCHAR column_name[SQL_MAX_COLUMN_NAME_LEN];
  SQLULEN column_size;
  SQLHENV    henv1;
  SQLHDBC    hdbc1;
  SQLHSTMT   hstmt1;

  /* First check how the option FLAG_NO_BINARY_RESULT works */
  SET_DSN_OPTION(1 << 28);

  alloc_basic_handles(&henv1, &hdbc1, &hstmt1);

  ok_sql(hstmt1, "SELECT CONCAT('ABCDEFG', 20) concated");

  ok_stmt(hstmt1, SQLDescribeCol(hstmt1, 1, column_name, sizeof(column_name),
                                &name_length, &data_type, &column_size,
                                &decimal_digits, &nullable));

  is(data_type == SQL_VARCHAR || data_type == SQL_WVARCHAR);
  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));
  free_basic_handles(&henv1, &hdbc1, &hstmt1);

  /* Check without FLAG_NO_BINARY_RESULT */
  ok_sql(hstmt, "SELECT CONCAT('ABCDEFG', 20) concated");

  ok_stmt(hstmt, SQLDescribeCol(hstmt, 1, column_name, sizeof(column_name),
                                &name_length, &data_type, &column_size,
                                &decimal_digits, &nullable));

  is_num(data_type, SQL_VARBINARY);
  return OK;
}


BEGIN_TESTS
  ADD_TEST(t_longlong1)
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
  ADD_TEST(sqlwchar)
  ADD_TEST(t_sqlnum_msdn)
  ADD_TEST(t_sqlnum_from_str)
  ADD_TEST(t_bindsqlnum_basic)
  ADD_TEST(t_sqlnum_to_str)
  ADD_TEST(t_bug31220)
  ADD_TEST(t_bug29402)
END_TESTS


RUN_TESTS
