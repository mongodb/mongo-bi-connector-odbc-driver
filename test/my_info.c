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


DECLARE_TEST(t_gettypeinfo)
{
  SQLRETURN rc;
  SQLSMALLINT pccol;

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLGetTypeInfo(hstmt,SQL_ALL_TYPES);
    mystmt(hstmt,rc);

    rc = SQLNumResultCols(hstmt,&pccol);
    mystmt(hstmt,rc);
    fprintf(stdout,"total columns: %d\n",pccol);
    myassert(pccol == 19);
    myresult(hstmt);

    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_getinfo)
{
  SQLRETURN rc;
  SQLCHAR   rgbValue[100];
  SQLSMALLINT pcbInfo;

    rc = SQLGetInfo(hdbc,SQL_DRIVER_ODBC_VER,rgbValue,100,&pcbInfo);
    mycon(hdbc,rc);
    fprintf(stdout,"SQL_DRIVER_ODBC_VER: %s(%d)\n",rgbValue,pcbInfo);

  return OK;
}


DECLARE_TEST(t_stmt_attr_status)
{
  SQLRETURN rc;
  SQLUSMALLINT rowStatusPtr[3];
  SQLUINTEGER rowsFetchedPtr;

    tmysql_exec(hstmt,"drop table t_stmtstatus");

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_stmtstatus(id int, name char(20))");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"insert into t_stmtstatus values(10,'data1')");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into t_stmtstatus values(20,'data2')");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_CURSOR_SCROLLABLE,(SQLPOINTER)SQL_NONSCROLLABLE,0);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_stmtstatus");
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROWS_FETCHED_PTR,&rowsFetchedPtr,0);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROW_STATUS_PTR,&rowStatusPtr,0);
    mystmt(hstmt,rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_ABSOLUTE,2);
    mystmt_err(hstmt,rc == SQL_ERROR,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_CURSOR_SCROLLABLE,(SQLPOINTER)SQL_SCROLLABLE,0);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from t_stmtstatus");
    mystmt(hstmt,rc);

    rc = SQLFetchScroll(hstmt,SQL_FETCH_ABSOLUTE,2);
    mystmt(hstmt,rc);

    printMessage("total rows fetched: %d\n",rowsFetchedPtr);
    printMessage("row 0 status      : %d\n",rowStatusPtr[0]);
    printMessage("row 1 status      : %d\n",rowStatusPtr[1]);
    printMessage("row 2 status      : %d\n",rowStatusPtr[2]);
    myassert(rowsFetchedPtr == 1);
    myassert(rowStatusPtr[0] == 0);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROWS_FETCHED_PTR,(SQLPOINTER)0,0);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROW_STATUS_PTR,(SQLPOINTER)0,0);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_msdev_bug)
{
  SQLRETURN  rc;
  SQLCHAR    catalog[30];
  SQLLEN     len;

  rc = SQLGetConnectOption(hdbc,SQL_CURRENT_QUALIFIER,&catalog);
  mycon(hdbc,rc);
  fprintf(stdout," SQL_CURRENT_QUALIFIER:%s\n",catalog);

  rc = SQLGetConnectAttr(hdbc,SQL_ATTR_CURRENT_CATALOG,&catalog,30,&len);
  mycon(hdbc,rc);
  fprintf(stdout," SQL_ATTR_CURRENT_CATRALOG:%s(%d)\n",catalog,len);

  return OK;
}


BEGIN_TESTS
  ADD_TEST(t_gettypeinfo)
  ADD_TEST(t_getinfo)
  ADD_TEST(t_stmt_attr_status)
  ADD_TEST(t_msdev_bug)
END_TESTS


RUN_TESTS
