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

SQLINTEGER my_max_rows = 1000;
clock_t t_start, t_end;
SQLDOUBLE my_time;

/* Clean data */
DECLARE_TEST(t_clean_data)
{
    SQLRETURN rc;

    rc = SQLExecDirect(hstmt,"drop database client_odbc_test1",SQL_NTS);
    mystmt(hstmt,rc);

  return OK;
}


/* Initialize data */
DECLARE_TEST(t_init_data)
{
    SQLRETURN rc;
    SQLINTEGER i;
    SQLCHAR    ch[]="MySQL AB";

    SQLExecDirect(hstmt,"drop database if exists client_odbc_test1",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create database client_odbc_test1",SQL_NTS);
    mystmt(hstmt,rc);

    SQLExecDirect(hstmt,"drop table client_odbc_test1.t_use_result",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table client_odbc_test1.t_use_result(id int,name char(10))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"insert into client_odbc_test1.t_use_result values(?,?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT, SQL_C_ULONG,
                          SQL_INTEGER,0,0,&i,0,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,2,SQL_PARAM_INPUT, SQL_C_CHAR,
                          SQL_CHAR,0,0,&ch,10,NULL);
    mystmt(hstmt,rc);

    printMessage("\n inserting '%d' rows (it will take few minutes)\n",my_max_rows);
    t_start = clock();

    for ( i = 1; i <= my_max_rows; i++ )
    {
        rc = SQLExecute(hstmt);
        mystmt(hstmt,rc);
        if (!(i % 1000)) printMessage(" \r %d",i);
    }
    t_end = clock();

    my_time = difftime(t_end,t_start)/CLOCKS_PER_SEC;   
    printMessage("\n total of '%d' rows inserted in %5.3f SECS",my_max_rows,my_time);

    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLEndTran(SQL_HANDLE_DBC,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

  return OK;
}


/* Fetch and count the time */
SQLINTEGER t_fetch_data(SQLHDBC hdbc,SQLHSTMT hstmt)
{
    SQLRETURN   rc;
    SQLINTEGER  row_count=0;

    /* set row size as 1 */
    rc = SQLSetStmtAttr(hstmt,SQL_ATTR_ROW_ARRAY_SIZE,(SQLPOINTER)1,0);
    mystmt(hstmt,rc);

    SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER) SQL_CONCUR_ROWVER, 0);
    SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_FORWARD_ONLY, 0);

    t_start = clock();
    rc = SQLExecDirect(hstmt,"select * from client_odbc_test1.t_use_result",SQL_NTS);
    mystmt(hstmt,rc);
    t_end = clock();

    my_time = difftime(t_end,t_start)/CLOCKS_PER_SEC;   
    printMessage("\n SELECT statement executed in %5.3f SECS",my_time);

    t_start = clock();
    rc = SQLFetch(hstmt);
    while (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    {
        row_count++;
        rc = SQLFetch(hstmt);
    }
    t_end = clock();  

    my_time = difftime(t_end,t_start)/CLOCKS_PER_SEC; 
    printMessage("\n total '%d' rows fetched in %5.3f SECS",row_count,my_time);

    SQLFreeStmt(hstmt,SQL_UNBIND);    
    SQLFreeStmt(hstmt,SQL_CLOSE);  
    return(row_count);
}


/* making use of mysql_use_result */
DECLARE_TEST(t_use_result)
{
    SQLHENV   henv1;
    SQLHDBC   hdbc1;
    SQLHSTMT  hstmt1;
    SQLCHAR   conn[255];
    SQLINTEGER option = 131072L * 8;

    sprintf(conn,"DSN=%s;UID=%s;PWD=%s;OPTION=%d",
            mydsn,myuid,mypwd,option);
    if (mysock != NULL)
    {
      strcat(conn, ";SOCKET=");
      strcat(conn, mysock);
    }
    mydrvconnect(&henv1,&hdbc1,&hstmt1,conn);
    my_assert(t_fetch_data(hdbc1,hstmt1) == my_max_rows);
    mydisconnect(&henv1,&hdbc1,&hstmt1);

  return OK;
}

/* making use of mysql_store_result */
DECLARE_TEST(t_store_result)
{
    SQLHENV   henv1;
    SQLHDBC   hdbc1;
    SQLHSTMT  hstmt1;
    SQLCHAR   conn[255];
    SQLINTEGER option = 3;

    sprintf(conn,"DSN=%s;UID=%s;PWD=%s;OPTION=%d",
            mydsn,myuid,mypwd,option);
    if (mysock != NULL)
    {
      strcat(conn, ";SOCKET=");
      strcat(conn, mysock);
    }
    mydrvconnect(&henv1,&hdbc1,&hstmt1,conn);
    my_assert(t_fetch_data(hdbc1,hstmt1) == my_max_rows);
    mydisconnect(&henv1,&hdbc1,&hstmt1);

  return OK;
}


BEGIN_TESTS
  ADD_TEST(t_init_data)
  ADD_TEST(t_use_result)
  ADD_TEST(t_store_result)
  ADD_TEST(t_clean_data)
END_TESTS


RUN_TESTS
