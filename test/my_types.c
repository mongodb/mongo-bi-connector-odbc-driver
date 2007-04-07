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
  SQLRETURN   rc;
  SQLINTEGER  session_id, ctn;

    tmysql_exec(hstmt,"drop table t_longlong");
    rc = tmysql_exec(hstmt,"create table t_longlong (\
                          session_id  bigint not null,\
                          ctn         bigint not null)");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER) SQL_CONCUR_ROWVER, 0);
    SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_STATIC, 0);

    rc = SQLPrepare(hstmt,"insert into t_longlong values (?,?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter( hstmt, 1, SQL_PARAM_INPUT, SQL_C_UBIGINT,
                           SQL_BIGINT, 20, 0, &session_id, 20, NULL );

    rc = SQLBindParameter( hstmt, 2, SQL_PARAM_INPUT, SQL_C_UBIGINT,
                           SQL_BIGINT, 20, 0, &ctn, 20, NULL );

    for (session_id=50; session_id < 100; session_id++)
    {
      ctn += session_id;
      rc = SQLExecute(hstmt);
      mystmt(hstmt,rc);
    }

    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from t_longlong");
    mystmt(hstmt,rc);

    my_assert( 50 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_numeric)
{
  SQLRETURN       rc;
  SQL_NUMERIC_STRUCT num;

    tmysql_exec(hstmt,"drop table t_decimal");
    rc = tmysql_exec(hstmt,"create table t_decimal(d1 decimal(10,6))");
    mystmt(hstmt,rc);
    rc = tmysql_exec(hstmt,"insert into t_decimal values(10.2)");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLPrepare(hstmt,"insert into t_decimal values (?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter( hstmt, 1, SQL_PARAM_INPUT, SQL_C_NUMERIC,
                           SQL_DECIMAL, 10, 4, &num, 0, NULL );
    mystmt_r(hstmt,rc);

    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLPrepare(hstmt,"insert into t_decimal values (?),(?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter( hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG,
                           SQL_DECIMAL, 10, 4, &rc, 0, NULL );
    mystmt(hstmt,rc);

    rc = SQLBindParameter( hstmt, 2, SQL_PARAM_INPUT, SQL_C_NUMERIC,
                           SQL_DECIMAL, 10, 4, &num, 0, NULL );
    mystmt_r(hstmt,rc);

    SQLFreeStmt(hstmt, SQL_CLOSE);
    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);

    rc = SQLBindCol( hstmt, 1, SQL_C_NUMERIC, &num, 0, NULL );
    mystmt_r(hstmt,rc);

    SQLFreeStmt(hstmt, SQL_CLOSE);
    SQLFreeStmt(hstmt, SQL_UNBIND);

    rc = SQLExecDirect(hstmt, "select * from t_decimal",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData( hstmt, 1, SQL_C_NUMERIC, &num, 0, NULL );
    mystmt_r(hstmt,rc);

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_CLOSE);

  return OK;
}


DECLARE_TEST(t_decimal)
{
  SQLCHAR         str[20],s_data[]="189.4567";
  SQLDOUBLE       d_data=189.4567;
  SQLINTEGER      i_data=189, l_data=-23;
  SQLRETURN       rc;

    tmysql_exec(hstmt,"drop table t_decimal");
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

  return OK;
}


BEGIN_TESTS
  ADD_TEST(t_longlong1)
  ADD_TEST(t_numeric)
  ADD_TEST(t_decimal)
END_TESTS


RUN_TESTS
