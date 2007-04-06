/*
  Copyright (C) 1997-2007 MySQL AB

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

#define MAX_INSERT_COUNT 800
#define MAX_TXT_LENGTH 10

DECLARE_TEST(t_bulk_check)
{
    SQLRETURN  rc;
    SQLCHAR    ltxt[MAX_TXT_LENGTH];

    SQLExecDirect(hstmt, "DROP TABLE t_bulk_check", SQL_NTS);

    rc = SQLExecDirect(hstmt, "CREATE TABLE t_bulk_check(id int PRIMARY KEY, ltxt text)", SQL_NTS);
    mystmt(hstmt, rc);

    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLPrepare(hstmt, "INSERT INTO t_bulk_check(id,ltxt) values(1,?)", SQL_NTS);
    mystmt(hstmt, rc);

    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
                          0,0,ltxt,MAX_TXT_LENGTH,NULL);
    mystmt(hstmt, rc);

    memset(ltxt, 'E', MAX_TXT_LENGTH);
    ltxt[MAX_TXT_LENGTH] = '\0';

    rc = SQLExecute(hstmt);
    mystmt(hstmt, rc);

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_CLOSE);    

    rc = SQLExecDirect(hstmt, "SELECT * FROM t_bulk_check", SQL_NTS);
    mystmt(hstmt, rc);

    my_assert(1 == my_print_non_format_result(hstmt));

    SQLFreeStmt(hstmt, SQL_CLOSE);

    SQLExecDirect(hstmt, "DROP TABLE t_bulk_check", SQL_NTS);
    SQLFreeStmt(hstmt, SQL_CLOSE);

  return OK;
}


DECLARE_TEST(t_bulk_insert)
{
    SQLRETURN rc;
    SQLINTEGER i,id[MAX_INSERT_COUNT+1];
    char    name[MAX_INSERT_COUNT][40],txt[MAX_INSERT_COUNT][60],ltxt[MAX_INSERT_COUNT][70];
    clock_t start, end;
    double duration,dbl[MAX_INSERT_COUNT];
    double dt;

    SQLExecDirect(hstmt, "DROP TABLE my_bulk", SQL_NTS);

    rc = SQLExecDirect(hstmt, "CREATE TABLE my_bulk ( id   int,          " \
                              "                       v    varchar(100), " \
                              "                       txt  text,         " \
                              "                       ft   float(10),    " \
                              "                       ltxt long varchar )", SQL_NTS);
    mystmt(hstmt, rc);

    SQLFreeStmt(hstmt, SQL_CLOSE);

    dt = 0.23456;

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_STATIC, 0);
    mystmt(hstmt, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)MAX_INSERT_COUNT, 0);
    mystmt(hstmt, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY , (SQLPOINTER)SQL_CONCUR_ROWVER , 0);
    mystmt(hstmt, rc);

    rc = SQLBindCol(hstmt, 1, SQL_C_LONG, &id[0], 0, NULL);
    mystmt(hstmt, rc);

    rc = SQLBindCol(hstmt, 2, SQL_C_CHAR, &name[0], 30 /* sizeof(name[0]) */, NULL);
    mystmt(hstmt, rc);

    rc = SQLBindCol(hstmt, 3, SQL_C_CHAR, &txt[0], sizeof(txt[0]), NULL);
    mystmt(hstmt, rc);

    rc = SQLBindCol(hstmt, 4, SQL_C_DOUBLE, &dbl[0], sizeof(dbl[0]), NULL);
    mystmt(hstmt, rc);

    rc = SQLBindCol(hstmt, 5, SQL_C_CHAR, &ltxt[0], sizeof(ltxt[0]), NULL);
    mystmt(hstmt, rc);

    rc = SQLExecDirect(hstmt, "SELECT id, v, txt, ft, ltxt FROM my_bulk", SQL_NTS);
    mystmt(hstmt, rc);

    rc = SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0);
    mystmt_err(hstmt, rc == SQL_NO_DATA_FOUND, rc);

    for (i= 0; i < MAX_INSERT_COUNT; i++)
    {
        id[i]=i;
        dbl[i] = i+dt;      
        sprintf( name[i], "Varchar%d", i );      
        sprintf( txt[i], "Text%d", i );      
        sprintf( ltxt[i], "LongText, id row:%d", i );
    }    

    printMessage( "\n total bulk adds   : %d", MAX_INSERT_COUNT*2);
    start = clock();    

    rc = SQLBulkOperations(hstmt, SQL_ADD);    
    mystmt(hstmt, rc);

    rc = SQLBulkOperations(hstmt, SQL_ADD);    
    mystmt(hstmt, rc);

    end = clock();

    duration = difftime(end,start)/CLOCKS_PER_SEC;
    printMessage( " (in '%lf' secs)", duration);

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);    

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)1, 0);
    mystmt(hstmt, rc);    

    start= clock();
    rc = SQLExecDirect(hstmt, "SELECT * FROM my_bulk", SQL_NTS);
    mystmt(hstmt, rc);
    my_assert(MAX_INSERT_COUNT*2 == myrowcount(hstmt));
    end= clock();

    duration = difftime(end,start)/CLOCKS_PER_SEC;
    printMessage(" (in '%lf' secs)\n", duration);

    SQLExecDirect(hstmt, "DROP TABLE my_bulk", SQL_NTS);
    SQLFreeStmt(hstmt, SQL_CLOSE);

  return OK;
}


BEGIN_TESTS
  ADD_TEST(t_bulk_check)
  ADD_TEST(t_bulk_insert)
END_TESTS


RUN_TESTS
