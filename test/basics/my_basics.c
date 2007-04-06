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


BEGIN_TESTS
  ADD_TEST(my_basics)
END_TESTS


RUN_TESTS
