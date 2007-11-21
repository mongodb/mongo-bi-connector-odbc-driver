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
 * Tests for descriptors.
 */

#include "odbctap.h"


/*
 * Tests for paramset:
 * SQL_ATTR_PARAMSET_SIZE (apd->array_size)
 * SQL_ATTR_PARAM_STATUS_PTR (ipd->array_status_ptr)
 * SQL_ATTR_PARAM_OPERATION_PTR (apd->array_status_ptr)
 * SQL_ATTR_PARAMS_PROCESSED_PTR (apd->rows_processed_ptr)
 */
DECLARE_TEST(t_desc_paramset)
{
  SQLUINTEGER parsetsize= 4;
  SQLUSMALLINT parstatus[4];
  SQLUSMALLINT parop[4]; /* operation */
  SQLUINTEGER pardone; /* processed */
  SQLHANDLE ipd, apd;
  SQLINTEGER params1[4];
  SQLINTEGER params2[4];

  parop[0]= SQL_PARAM_PROCEED;
  parop[1]= SQL_PARAM_IGNORE;
  parop[2]= SQL_PARAM_IGNORE;
  parop[3]= SQL_PARAM_PROCEED;
  params1[0]= 0;
  params1[1]= 1;
  params1[2]= 2;
  params1[3]= 3;
  params2[0]= 100;
  params2[1]= 101;
  params2[2]= 102;
  params2[3]= 103;

  /* get the descriptors */
  ok_stmt(hstmt, SQLGetStmtAttr(hstmt, SQL_ATTR_APP_PARAM_DESC,
                                &apd, SQL_IS_POINTER, NULL));
  ok_stmt(hstmt, SQLGetStmtAttr(hstmt, SQL_ATTR_IMP_PARAM_DESC,
                                &ipd, SQL_IS_POINTER, NULL));

  /* set the fields */
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMSET_SIZE,
                                (SQLPOINTER) parsetsize, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAM_STATUS_PTR,
                                parstatus, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAM_OPERATION_PTR,
                                parop, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMS_PROCESSED_PTR,
                                &pardone, 0));

  /* verify the fields */
  {
    SQLPOINTER x_parstatus, x_parop, x_pardone;
    SQLUINTEGER x_parsetsize;

    ok_desc(apd, SQLGetDescField(apd, 0, SQL_DESC_ARRAY_SIZE,
                                 &x_parsetsize, SQL_IS_UINTEGER, NULL));
    ok_desc(ipd, SQLGetDescField(ipd, 0, SQL_DESC_ARRAY_STATUS_PTR,
                                 &x_parstatus, SQL_IS_POINTER, NULL));
    ok_desc(apd, SQLGetDescField(apd, 0, SQL_DESC_ARRAY_STATUS_PTR,
                                 &x_parop, SQL_IS_POINTER, NULL));
    ok_desc(ipd, SQLGetDescField(ipd, 0, SQL_DESC_ROWS_PROCESSED_PTR,
                                 &x_pardone, SQL_IS_POINTER, NULL));

    is_num(x_parsetsize, parsetsize);
    is(x_parstatus == parstatus);
    is(x_parop == parop);
    is(x_pardone == &pardone);
  }

  ok_sql(hstmt, "drop table if exists t_paramset");
  ok_sql(hstmt, "create table t_paramset(x int, y int)");
  ok_stmt(hstmt, SQLPrepare(hstmt, "insert into t_paramset "
                            "values (?, ?)", SQL_NTS));

  ok_stmt(hstmt,
          SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_INTEGER, SQL_C_LONG,
                           0, 0, params1, sizeof(SQLINTEGER), NULL));
  ok_stmt(hstmt,
          SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_INTEGER, SQL_C_LONG,
                           0, 0, params2, sizeof(SQLINTEGER), NULL));
  /*
  ok_stmt(hstmt, SQLExecute(hstmt));
  */
  /* TODO, finish test and implement */

  return OK;
}


/*
 * Test for errors when setting descriptor fields.
 */
DECLARE_TEST(t_desc_set_error)
{
  SQLHANDLE ird, ard;
  SQLPOINTER array_status_ptr= (SQLPOINTER) 0xc0c0;

  ok_stmt(hstmt, SQLGetStmtAttr(hstmt, SQL_ATTR_IMP_ROW_DESC,
                                &ird, SQL_IS_POINTER, NULL));
  ok_stmt(hstmt, SQLGetStmtAttr(hstmt, SQL_ATTR_APP_ROW_DESC,
                                &ard, SQL_IS_POINTER, NULL));

  /* Test bad header field permissions */
  expect_desc(ard, SQLSetDescField(ard, 0, SQL_DESC_ROWS_PROCESSED_PTR,
                                   NULL, SQL_IS_POINTER), SQL_ERROR);
  is(check_sqlstate_ex(ard, SQL_HANDLE_DESC, "HY091") == OK);

  /* Test the HY016 error received when setting any field on an IRD
   * besides SQL_DESC_ARRAY_STATUS_PTR or SQL_DESC_ROWS_PROCESSED_PTR.
   *
   * Windows intercepts this and returns HY091
   */
  ok_desc(ird, SQLSetDescField(ird, 0, SQL_DESC_ARRAY_STATUS_PTR,
                               array_status_ptr, SQL_IS_POINTER));
  expect_desc(ird, SQLSetDescField(ird, 0, SQL_DESC_AUTO_UNIQUE_VALUE,
                                   (SQLPOINTER) 1, SQL_IS_INTEGER),
              SQL_ERROR);
#ifdef _WIN32
  is(check_sqlstate_ex(ird, SQL_HANDLE_DESC, "HY091") == OK);
#else
  is(check_sqlstate_ex(ird, SQL_HANDLE_DESC, "HY016") == OK);
#endif

  /* Test invalid field identifier (will be HY016 on ird, HY091 on others) */
  expect_desc(ard, SQLSetDescField(ard, 0, 999, NULL, SQL_IS_POINTER),
              SQL_ERROR);
  is(check_sqlstate_ex(ard, SQL_HANDLE_DESC, "HY091") == OK);

  /* Test bad data type (SQLINTEGER cant be assigned to SQLPOINTER) */
  expect_desc(ard, SQLSetDescField(ard, 0, SQL_DESC_BIND_OFFSET_PTR,
                                   NULL, SQL_IS_INTEGER), SQL_ERROR);
  is(check_sqlstate_ex(ard, SQL_HANDLE_DESC, "HY015") == OK);

  return OK;
}


/*
   Implicit Resetting of COUNT Field with SQLBindCol()
*/
DECLARE_TEST(t_sqlbindcol_count_reset)
{
  SQLHANDLE ard;
  SQLINTEGER count;
  SQLCHAR *buf[10];

  ok_stmt(hstmt, SQLGetStmtAttr(hstmt, SQL_ATTR_APP_ROW_DESC,
                                &ard, SQL_IS_POINTER, NULL));

  ok_desc(ard, SQLGetDescField(ard, 0, SQL_DESC_COUNT, &count,
                               SQL_IS_INTEGER, NULL));
  is_num(count, 0);

  ok_sql(hstmt, "select 1,2,3,4,5");

  ok_desc(ard, SQLGetDescField(ard, 0, SQL_DESC_COUNT, &count,
                               SQL_IS_INTEGER, NULL));
  is_num(count, 0);

  /* bind column 3 -> expand to count = 3 */
  ok_stmt(hstmt, SQLBindCol(hstmt, 3, SQL_C_CHAR, buf, 10, NULL));

  ok_desc(ard, SQLGetDescField(ard, 0, SQL_DESC_COUNT, &count,
                               SQL_IS_INTEGER, NULL));
  is_num(count, 3);

  /* unbind column 3 -> contract to count = 0 */
  ok_stmt(hstmt, SQLBindCol(hstmt, 3, 0, NULL, 0, NULL));

  ok_desc(ard, SQLGetDescField(ard, 0, SQL_DESC_COUNT, &count,
                               SQL_IS_INTEGER, NULL));
  is_num(count, 0);

  /* bind column 2 -> expand to count = 2 */
  ok_stmt(hstmt, SQLBindCol(hstmt, 2, SQL_C_CHAR, buf, 10, NULL));

  ok_desc(ard, SQLGetDescField(ard, 0, SQL_DESC_COUNT, &count,
                               SQL_IS_INTEGER, NULL));
  is_num(count, 2);

  /* bind column 3 -> expand to count = 3 */
  ok_stmt(hstmt, SQLBindCol(hstmt, 3, SQL_C_CHAR, buf, 10, NULL));

  ok_desc(ard, SQLGetDescField(ard, 0, SQL_DESC_COUNT, &count,
                               SQL_IS_INTEGER, NULL));
  is_num(count, 3);

  /* unbind column 3 -> contract to count = 2 */
  ok_stmt(hstmt, SQLBindCol(hstmt, 3, 0, NULL, 0, NULL));

  ok_desc(ard, SQLGetDescField(ard, 0, SQL_DESC_COUNT, &count,
                               SQL_IS_INTEGER, NULL));
  is_num(count, 2);

  return OK;
}


BEGIN_TESTS
  ADD_TEST(t_desc_paramset)
  ADD_TEST(t_desc_set_error)
  ADD_TEST(t_sqlbindcol_count_reset)
END_TESTS


RUN_TESTS

