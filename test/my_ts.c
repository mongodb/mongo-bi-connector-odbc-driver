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


BEGIN_TESTS
  ADD_TEST(my_ts)
  ADD_TEST(t_tstotime)
  ADD_TEST(t_tstotime1)
END_TESTS


RUN_TESTS
