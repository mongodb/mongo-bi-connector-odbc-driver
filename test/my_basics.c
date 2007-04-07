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

DECLARE_TEST(my_basics)
{
    SQLRETURN  rc;
    SQLROWCOUNT nRowCount;

    /* drop table 'myodbc3_demo_basic' if it already exists */
    rc = SQLExecDirect(hstmt,"DROP TABLE if exists myodbc3_demo_basic",SQL_NTS);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* create the table 'myodbc3_demo_result' */
    rc = SQLExecDirect(hstmt,"CREATE TABLE myodbc3_demo_basic(\
                              id int primary key auto_increment,\
                              name varchar(20))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);    

    /* insert 3 rows of data */    
    rc = SQLExecDirect(hstmt,"INSERT INTO myodbc3_demo_basic values(\
                              1,'MySQL')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"INSERT INTO myodbc3_demo_basic values(\
                              2,'MyODBC')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"INSERT INTO myodbc3_demo_basic values(\
                              3,'monty')",SQL_NTS);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* update second row */    
    rc = SQLExecDirect(hstmt,"UPDATE myodbc3_demo_basic set name=\
                              'MyODBC 3.51' where id=2",SQL_NTS);
    mystmt(hstmt,rc);

    /* get the rows affected by update statement */
    rc = SQLRowCount(hstmt,&nRowCount);
    mystmt(hstmt,rc);
    printMessage( " total rows updated:%d", nRowCount );

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* delete third column */
    rc = SQLExecDirect(hstmt,"DELETE FROM myodbc3_demo_basic where id = 3",SQL_NTS);
    mystmt(hstmt,rc);

    /* get the rows affected by delete statement */
    rc = SQLRowCount(hstmt,&nRowCount);
    mystmt(hstmt,rc);
    printMessage(" total rows deleted:%d",nRowCount);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* alter the table 'myodbc3_demo_basic' to 'myodbc3_new_name' */
    rc = SQLExecDirect(hstmt,"ALTER TABLE myodbc3_demo_basic RENAME myodbc3_new_name",SQL_NTS);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* drop the table with the original table name, and it should
       return error saying 'table not found'
    */
    rc = SQLExecDirect(hstmt,"DROP TABLE myodbc3_demo_basic",SQL_NTS);
    /* mystmt_err(hstmt, rc == SQL_ERROR, rc); */

    /* now drop the table, which is altered..*/   
    rc = SQLExecDirect(hstmt,"DROP TABLE myodbc3_new_name",SQL_NTS);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* free the statement cursor */
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_max_select)
{
  SQLRETURN rc;
  SQLCHAR szData[255];
  SQLINTEGER i;

    tmysql_exec(hstmt,"drop table t_max_select");

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"create table t_max_select(col1 int ,col2 varchar(30))");
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"insert into t_max_select values(?,?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_LONG,
                            SQL_INTEGER,0,0,&i,0,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,2,SQL_PARAM_INPUT,SQL_C_CHAR,
                            SQL_CHAR,0,0,szData,sizeof(szData),NULL);
    mystmt(hstmt,rc);

    fprintf(stdout," inserting 1000 rows, it will take some time\n");
    for(i = 1; i <= 1000; i++)
    {
      fprintf(stdout," \r %d", i);
      sprintf((char *)szData,"MySQL%d",i);
      rc = SQLExecute(hstmt);
      mystmt(hstmt,rc);
    }
    fprintf(stdout,"\n");

    rc = SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = tmysql_exec(hstmt,"select * from t_max_select");
    mystmt(hstmt,rc);

    my_assert( 1000 == myrowcount(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


BEGIN_TESTS
  ADD_TEST(my_basics)
  ADD_TEST(t_max_select)
END_TESTS


RUN_TESTS
