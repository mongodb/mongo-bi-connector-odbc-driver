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
/*
 * Tests for Bug #13766 - transforming dates to/from zero/invalid dates.
 * This is a separate file so the DSN OPTION doesn't affect other tests.
 */

#include "odbctap.h"

/*
 * Test the FLAG_ZERO_DATE_TO_MIN transforms the date correctly.
 * NULL test is in my_datetime.c:t_bug12520()
 */
DECLARE_TEST(bug13766_result)
{
  int i;
  SQL_DATE_STRUCT xdate[6];
  SQL_TIMESTAMP_STRUCT xts[6];
  SQLLEN isNull[12];

  ok_sql(hstmt, "select cast('0000-00-00' as date), "
      "cast('0000-10-00' as date), "
      "cast('0000-00-10' as date), "
      "cast('2007-00-00' as date), "
      "cast('2007-10-00' as date), "
      "cast('2007-00-10' as date), "
      "cast('0000-00-00 00:00:00' as datetime), "
      "cast('0000-10-00 00:00:00' as datetime), "
      "cast('0000-00-10 00:00:00' as datetime), "
      "cast('2007-00-00 00:00:00' as datetime), "
      "cast('2007-10-00 00:00:00' as datetime), "
      "cast('2007-00-10 00:00:00' as datetime) ");
  ok_stmt(hstmt, SQLFetch(hstmt));
  for (i= 0; i < 6; ++i)
  {
    ok_stmt(hstmt, SQLGetData(hstmt, i+1, SQL_C_TYPE_DATE,
                              &xdate[i], 0, &isNull[i]));
  }
  for (i= 0; i < 6; ++i)
  {
    ok_stmt(hstmt, SQLGetData(hstmt, 6+i+1, SQL_C_TYPE_TIMESTAMP,
                              &xts[i], 0, &isNull[6+i]));
  }

  i= 0;
  /* 0000-00-00 */
  is_num(xdate[i].year, 0);
  is_num(xdate[i].month, 1);
  is_num(xdate[i].day, 1);
  is_num(xts[i].year, 0);
  is_num(xts[i].month, 1);
  is_num(xts[i].day, 1);
  i++;

  /*
    the server is not consistent in how it handles 0000-xx-xx, it changed
    within the 5.0 and 5.1 series
  */
  if (isNull[i] == SQL_NULL_DATA)
  {
    is_num(isNull[i], SQL_NULL_DATA);
    is_num(isNull[6+i], SQL_NULL_DATA);
    i++;
    is_num(isNull[i], SQL_NULL_DATA);
    is_num(isNull[6+i], SQL_NULL_DATA);
    i++;
  }
  else
  {
    /* 0000-10-00 */
    is_num(xdate[i].year, 0);
    is_num(xdate[i].month, 10);
    is_num(xdate[i].day, 1);
    is_num(xts[i].year, 0);
    is_num(xts[i].month, 10);
    is_num(xts[i].day, 1);
    i++;
    /* 0000-00-10 */
    is_num(xdate[i].year, 0);
    is_num(xdate[i].month, 1);
    is_num(xdate[i].day, 10);
    is_num(xts[i].year, 0);
    is_num(xts[i].month, 1);
    is_num(xts[i].day, 10);
    i++;
  }

  /* 2007-00-00 */
  is_num(xdate[i].year, 2007);
  is_num(xdate[i].month, 1);
  is_num(xdate[i].day, 1);
  is_num(xts[i].year, 2007);
  is_num(xts[i].month, 1);
  is_num(xts[i].day, 1);
  i++;
  /* 2007-10-00 */
  is_num(xdate[i].year, 2007);
  is_num(xdate[i].month, 10);
  is_num(xdate[i].day, 1);
  is_num(xts[i].year, 2007);
  is_num(xts[i].month, 10);
  is_num(xts[i].day, 1);
  i++;
  /* 2007-00-10 */
  is_num(xdate[i].year, 2007);
  is_num(xdate[i].month, 1);
  is_num(xdate[i].day, 10);
  is_num(xts[i].year, 2007);
  is_num(xts[i].month, 1);
  is_num(xts[i].day, 10);

  return OK;
}


/*
 * Bug #13766 - Test the FLAG_MIN_DATE_TO_ZERO transforms the date
 * correctly.
 */
DECLARE_TEST(bug13766_query)
{
  SQL_DATE_STRUCT xdate;
  SQL_TIMESTAMP_STRUCT xts;
  char result[50];

  ok_stmt(hstmt, SQLPrepare(hstmt, (SQLCHAR *)"select ?", SQL_NTS));
  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_TYPE_DATE,
                                  SQL_TYPE_DATE, 0, 0, &xdate, 0, NULL));

  /* Test that we fix min date -> zero date */
  xdate.year= 0;
  xdate.month= 1;
  xdate.day= 1;
  ok_stmt(hstmt, SQLExecute(hstmt));
  ok_stmt(hstmt, SQLFetch(hstmt));
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, result, 50, NULL));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  is_str(result, "0000-00-00", 10);

  /* we dont touch dates that are not 0000-01-01 */

  xdate.year= 0;
  xdate.month= 0;
  xdate.day= 0;
  ok_stmt(hstmt, SQLExecute(hstmt));
  ok_stmt(hstmt, SQLFetch(hstmt));
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, result, 50, NULL));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  is_str(result, "0000-00-00", 10);

  xdate.year= 1;
  xdate.month= 0;
  xdate.day= 0;
  ok_stmt(hstmt, SQLExecute(hstmt));
  ok_stmt(hstmt, SQLFetch(hstmt));
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, result, 50, NULL));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  is_str(result, "0001-00-00", 10);

  xdate.year= 0;
  xdate.month= 1;
  xdate.day= 0;
  ok_stmt(hstmt, SQLExecute(hstmt));
  ok_stmt(hstmt, SQLFetch(hstmt));
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, result, 50, NULL));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  is_str(result, "0000-01-00", 10);

  xdate.year= 0;
  xdate.month= 0;
  xdate.day= 1;
  ok_stmt(hstmt, SQLExecute(hstmt));
  ok_stmt(hstmt, SQLFetch(hstmt));
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, result, 50, NULL));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  is_str(result, "0000-00-01", 10);

  /* same stuff for timestamps */

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_RESET_PARAMS));
  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP,
                                  SQL_TYPE_TIMESTAMP, 0, 0, &xts, 0, NULL));
  xts.hour = 19;
  xts.minute = 22;
  xts.second = 25;

  xts.year= 0;
  xts.month= 1;
  xts.day= 1;
  ok_stmt(hstmt, SQLExecute(hstmt));
  ok_stmt(hstmt, SQLFetch(hstmt));
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, result, 50, NULL));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  is_str(result, "0000-00-00 19:22:25", 19);

  xts.year= 0;
  xts.month= 0;
  xts.day= 0;
  ok_stmt(hstmt, SQLExecute(hstmt));
  ok_stmt(hstmt, SQLFetch(hstmt));
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, result, 50, NULL));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  is_str(result, "0000-00-00 19:22:25", 19);

  xts.year= 1;
  xts.month= 0;
  xts.day= 0;
  ok_stmt(hstmt, SQLExecute(hstmt));
  ok_stmt(hstmt, SQLFetch(hstmt));
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, result, 50, NULL));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  is_str(result, "0001-00-00 19:22:25", 19);

  xts.year= 0;
  xts.month= 1;
  xts.day= 0;
  ok_stmt(hstmt, SQLExecute(hstmt));
  ok_stmt(hstmt, SQLFetch(hstmt));
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, result, 50, NULL));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  is_str(result, "0000-01-00 19:22:25", 19);

  xts.year= 0;
  xts.month= 0;
  xts.day= 1;
  ok_stmt(hstmt, SQLExecute(hstmt));
  ok_stmt(hstmt, SQLFetch(hstmt));
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, result, 50, NULL));
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  is_str(result, "0000-00-01 19:22:25", 19);

  return OK;
}


BEGIN_TESTS
  ADD_TEST(bug13766_result)
  ADD_TEST(bug13766_query)
END_TESTS


/* FLAG_ZERO_DATE_TO_MIN & FLAG_MIN_DATE_TO_ZERO */
SET_DSN_OPTION((1 << 24) | (1 << 25));


RUN_TESTS

