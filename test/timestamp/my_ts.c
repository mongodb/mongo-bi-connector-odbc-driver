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


BEGIN_TESTS
  ADD_TEST(my_ts)
END_TESTS


RUN_TESTS
