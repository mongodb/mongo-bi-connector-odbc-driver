/*
  Copyright (C) 1995-2006 MySQL AB

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


DECLARE_TEST(my_ts)
{
  SQLCHAR          szTs[50];
  TIMESTAMP_STRUCT ts;
  SQLLEN           len;

  ok_sql(hstmt, "DROP TABLE IF EXISTS my_ts");
  ok_sql(hstmt, "CREATE TABLE my_ts (ts TIMESTAMP)");

  /* insert using SQL_C_CHAR to SQL_TIMESTAMP */
  strcpy((char *)szTs, "2002-01-07 10:20:49.06");
  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT,
                                  SQL_C_CHAR, SQL_TIMESTAMP,
                                  0, 0, szTs, sizeof(szTs), NULL));
  ok_sql(hstmt, "INSERT INTO my_ts (ts) VALUES (?)");

  ok_stmt(hstmt, SQLFreeStmt(hstmt,SQL_RESET_PARAMS));
  ok_stmt(hstmt, SQLFreeStmt(hstmt,SQL_CLOSE));

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE,
                                (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN, 0));

  /* insert using SQL_C_TIMESTAMP to SQL_TIMESTAMP */
  ts.year= 2002;
  ts.month= 1;
  ts.day= 7;
  ts.hour= 19;
  ts.minute= 47;
  ts.second= 59;
  ts.fraction= 4;
  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT,
                                  SQL_C_TIMESTAMP, SQL_TIMESTAMP,
                                  0, 0, &ts, sizeof(ts), NULL));

  ok_sql(hstmt, "INSERT INTO my_ts (ts) VALUES (?)");

  ok_stmt(hstmt, SQLFreeStmt(hstmt,SQL_RESET_PARAMS));
  ok_stmt(hstmt, SQLFreeStmt(hstmt,SQL_CLOSE));

  /* now fetch and verify the results .. */
  ok_sql(hstmt, "SELECT * FROM my_ts");

  /* now fetch first row */
  ok_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, 1));

  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, szTs, sizeof(szTs), &len));
  is_str(szTs, "2002-01-07 10:20:49", len);
  printf("# row1 using SQL_C_CHAR: %s (%ld)\n", szTs, len);

  ok_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, 1));

  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_TIMESTAMP, &ts, sizeof(ts), &len));
  is_num(ts.year,  2002);
  is_num(ts.month, 1);
  is_num(ts.day,   7);
  is_num(ts.hour,  10);
  is_num(ts.minute,20);
  is_num(ts.second,49);
  printf("# row1 using SQL_C_TIMESTAMP: %d-%d-%d %d:%d:%d.%d (%ld)\n",
         ts.year, ts.month,ts.day, ts.hour, ts.minute, ts.second, ts.fraction,
         len);

  ok_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, 2));

  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, szTs, sizeof(szTs), &len));
  is_str(szTs, "2002-01-07 19:47:59", len);
  printf("# row2 using SQL_C_CHAR: %s(%ld)\n", szTs, len);

  ok_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, 2));

  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_TIMESTAMP, &ts, sizeof(ts), &len));
  is_num(ts.year,  2002);
  is_num(ts.month, 1);
  is_num(ts.day,   7);
  is_num(ts.hour,  19);
  is_num(ts.minute,47);
  is_num(ts.second,59);
  printf("# row2 using SQL_C_TIMESTAMP: %d-%d-%d %d:%d:%d.%d (%ld)\n",
         ts.year, ts.month,ts.day, ts.hour, ts.minute, ts.second, ts.fraction,
         len);


  expect_stmt(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, 3),
              SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt,SQL_UNBIND));
  ok_stmt(hstmt,  SQLFreeStmt(hstmt,SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS my_ts");

  return OK;
}


DECLARE_TEST(t_tstotime)
{
    SQLRETURN rc;
    SQL_TIMESTAMP_STRUCT ts;

    ts.day    = 02;
    ts.month  = 8;
    ts.year   = 2001;
    ts.hour   = 18;
    ts.minute = 20;
    ts.second = 45;
    ts.fraction = 05;   

    tmysql_exec(hstmt,"drop table t_tstotime");

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_tstotime(col1 date ,col2 time, col3 timestamp(14))");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    /* TIMESTAMP TO DATE, TIME and TS CONVERSION */
    rc = SQLPrepare(hstmt,"insert into t_tstotime(col1,col2,col3) values(?,?,?)",SQL_NTS);
    mystmt(hstmt,rc);   

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_TIMESTAMP,
                          SQL_DATE,0,0,&ts,sizeof(ts),NULL);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,2,SQL_PARAM_INPUT,SQL_C_TIMESTAMP,
                          SQL_TIME,0,0,&ts,sizeof(ts),NULL);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,3,SQL_PARAM_INPUT,SQL_C_TIMESTAMP,
                          SQL_TIMESTAMP,0,0,&ts,sizeof(ts),NULL);
    mystmt(hstmt,rc);

    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);  

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from t_tstotime");
    mystmt(hstmt,rc);  

    my_assert( 1 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_tstotime1)
{
  SQLCHAR ts[40]= "2001-08-02 18:20:45.05";

  ok_sql(hstmt,"DROP TABLE IF EXISTS t_tstotime1");

  ok_sql(hstmt,
         "CREATE TABLE t_tstotime1(col1 DATE, col2 TIME, col3 TIMESTAMP)");

  ok_stmt(hstmt, SQLFreeStmt(hstmt,SQL_CLOSE));

  /* TIMESTAMP TO DATE, TIME and TS CONVERSION */
  ok_stmt(hstmt, SQLPrepare(hstmt,
                            "INSERT INTO t_tstotime1  VALUES (?,?,?)",
                            SQL_NTS));

  ok_stmt(hstmt, SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_CHAR,
                                  SQL_DATE,0,0,&ts,sizeof(ts),NULL));
  ok_stmt(hstmt, SQLBindParameter(hstmt,2,SQL_PARAM_INPUT,SQL_C_CHAR,
                                  SQL_TIME,0,0,&ts,sizeof(ts),NULL));
  ok_stmt(hstmt, SQLBindParameter(hstmt,3,SQL_PARAM_INPUT,SQL_C_CHAR,
                                  SQL_TIMESTAMP,0,0,&ts,sizeof(ts),NULL));

  ok_stmt(hstmt, SQLExecute(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_RESET_PARAMS));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_con(hdbc, SQLTransact(NULL,hdbc,SQL_COMMIT));

  ok_sql(hstmt, "SELECT * FROM t_tstotime1");

  my_assert(1 == myresult(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_RESET_PARAMS));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  return OK;
}


DECLARE_TEST(t_bug25846)
{
  SQLSMALLINT          column_count;
  SQLLEN               my_time_cb;
  SQLLEN               my_date_cb;
  SQL_TIMESTAMP_STRUCT my_time_ts;
  SQL_TIMESTAMP_STRUCT my_date_ts;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug25846");
  ok_sql(hstmt, "CREATE TABLE t_bug25846 (a TIME, b DATE)");
  ok_sql(hstmt, "INSERT INTO t_bug25846 VALUES ('02:56:30', '1969-07-21')");

  ok_sql(hstmt, "SELECT * FROM t_bug25846");

  ok_stmt(hstmt, SQLNumResultCols(hstmt, &column_count));
  is_num(column_count, 2);

  /* Bind the TIMESTAMP buffer for TIME column */
  ok_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_TIMESTAMP, &my_time_ts,
                            sizeof(my_time_ts), &my_time_cb));

  /* Bind the TIMESTAMP buffer for DATE column */
  ok_stmt(hstmt, SQLBindCol(hstmt, 2, SQL_C_TIMESTAMP, &my_date_ts,
                            sizeof(my_date_ts), &my_date_cb));

  ok_stmt(hstmt, SQLFetch(hstmt));

  is_num(my_time_ts.hour,   2);
  is_num(my_time_ts.minute, 56);
  is_num(my_time_ts.second, 30);

  is_num(my_date_ts.year,   1969);
  is_num(my_date_ts.month,  7);
  is_num(my_date_ts.day,    21);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug25846");

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


/**
 Bug #12520: DATETIME Default Value 0000-00-00 00:00:00 not returning
 correct thru ODBC
*/
DECLARE_TEST(t_bug12520)
{
  SQL_TIMESTAMP_STRUCT my_time_ts;
  SQLLEN len, my_time_cb;
  SQLCHAR datetime[50];

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug12520");
  ok_sql(hstmt,
         "CREATE TABLE t_bug12520 (a DATETIME DEFAULT '0000-00-00 00:00',"
         "b DATETIME DEFAULT '0000-00-00 00:00', c INT)");

  ok_sql(hstmt, "INSERT INTO t_bug12520 (c) VALUES (1)");

  ok_sql(hstmt, "SELECT a, b FROM t_bug12520");

  ok_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_CHAR, datetime, sizeof(datetime),
                            &len));
  ok_stmt(hstmt, SQLBindCol(hstmt, 2, SQL_C_TIMESTAMP, &my_time_ts, 0,
                            &my_time_cb));

  ok_stmt(hstmt, SQLFetch(hstmt));

  is_str(datetime, "0000-00-00 00:00:00", 19);
  is_num(my_time_cb, SQL_NULL_DATA);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug12520");

  return OK;
}

/**
 Bug #15773: Wrong between results
*/
DECLARE_TEST(t_bug15773)
{
  SQL_DATE_STRUCT a,b,c,d;
  SQLLEN len1;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug15773");
  ok_sql(hstmt, "CREATE TABLE t_bug15773("
         "`a` varchar(255) NOT NULL default '',"
         "`b` datetime NOT NULL default '0000-00-00 00:00:00',"
         "`c` datetime NOT NULL default '0000-00-00 00:00:00'"
         ") ENGINE=InnoDB DEFAULT CHARSET=latin1");
  ok_sql(hstmt, "INSERT INTO t_bug15773 VALUES ('a', '2005-12-24 00:00:00', '2008-05-12 00:00:00')");
  ok_sql(hstmt, "INSERT INTO t_bug15773 VALUES ('b', '2004-01-01 00:00:00', '2005-01-01 00:00:00')");
  ok_sql(hstmt, "INSERT INTO t_bug15773 VALUES ('c', '2004-12-12 00:00:00', '2005-12-12 00:00:00')");

  ok_stmt(hstmt, SQLPrepare(hstmt, (SQLCHAR *)"SELECT * FROM t_bug15773"
                           " WHERE (?) BETWEEN b AND c", SQL_NTS));

  d.day= 15;
  d.month= 12;
  d.year = 2005;

  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_TYPE_DATE,
                                  SQL_TYPE_DATE, 0, 0, &d, 0, NULL));
  ok_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_CHAR, &a, 255, &len1));
  ok_stmt(hstmt, SQLBindCol(hstmt, 2, SQL_C_TYPE_DATE, &b, 0, &len1));
  ok_stmt(hstmt, SQLBindCol(hstmt, 3, SQL_C_TYPE_DATE, &c, 0, &len1));

  ok_stmt(hstmt, SQLExecute(hstmt));

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_UNBIND));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug15773");
  return OK;
}


BEGIN_TESTS
  ADD_TEST(my_ts)
  ADD_TEST(t_tstotime)
  ADD_TEST(t_tstotime1)
  ADD_TEST(t_bug25846)
  ADD_TEST(t_time)
  ADD_TEST(t_time1)
  ADD_TEST(t_bug12520)
  ADD_TEST(t_bug15773)
END_TESTS


RUN_TESTS
