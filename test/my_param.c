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

/********************************************************
* initialize tables                                     *
*********************************************************/
DECLARE_TEST(my_init_table)
{
    SQLRETURN   rc;

    /* drop table 'my_demo_param' if it already exists */
    printMessage(" creating table 'my_demo_param'\n");

    rc = SQLExecDirect(hstmt,"DROP TABLE if exists my_demo_param",SQL_NTS);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);

    /* create the table 'my_demo_param' */
    rc = SQLExecDirect(hstmt,"CREATE TABLE my_demo_param(\
                              id   int,\
                              auto int primary key auto_increment,\
                              name varchar(20),\
                              timestamp timestamp(14))",SQL_NTS);
    mystmt(hstmt,rc);

    /* commit the transaction*/
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);

  return OK;
}


DECLARE_TEST(my_param_insert)
{
    SQLRETURN   rc;
    SQLINTEGER  id;
    char        name[50];

    /* prepare the insert statement with parameters */
    rc = SQLPrepare(hstmt,"INSERT INTO my_demo_param(id,name) VALUES(?,?)",SQL_NTS);
    mystmt(hstmt,rc);

    /* now supply data to parameter 1 and 2 */
    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT,
                          SQL_C_LONG, SQL_INTEGER, 0,0,
                          &id, 0, NULL);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT,
                          SQL_C_CHAR, SQL_CHAR, 0,0,
                          name, sizeof(name), NULL);
    mystmt(hstmt,rc);

    /* now insert 10 rows of data */
    for (id = 0; id < 10; id++)
    {
        sprintf(name,"MySQL%d",id);

        rc = SQLExecute(hstmt);
        mystmt(hstmt,rc);
    }

    /* Free statement param resorces */
    rc = SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    mystmt(hstmt,rc);

    /* Free statement cursor resorces */
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);

    /* Now fetch and verify the data */
    rc = SQLExecDirect(hstmt, "SELECT * FROM my_demo_param",SQL_NTS);
    mystmt(hstmt,rc);

    assert(10 == myresult(hstmt));

  return OK;
}


DECLARE_TEST(my_param_update)
{
    SQLRETURN  rc;
    SQLROWCOUNT nRowCount;
    SQLINTEGER id=9;
    char name[]="update";

    /* prepare the insert statement with parameters */
    rc = SQLPrepare(hstmt,"UPDATE my_demo_param set name = ? WHERE id = ?",SQL_NTS);
    mystmt(hstmt,rc);

    /* now supply data to parameter 1 and 2 */
    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT,
                          SQL_C_CHAR, SQL_CHAR, 0,0,
                          name, sizeof(name), NULL);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT,
                          SQL_C_LONG, SQL_INTEGER, 0,0,
                          &id, 0, NULL);
    mystmt(hstmt,rc);

    /* now execute the update statement */
    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);

    /* check the rows affected by the update statement */
    rc = SQLRowCount(hstmt, &nRowCount);
    mystmt(hstmt,rc);
    printMessage("\n total rows updated:%d\n",nRowCount);
    assert( nRowCount == 1);

    /* Free statement param resorces */
    rc = SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    mystmt(hstmt,rc);

    /* Free statement cursor resorces */
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);

    /* Now fetch and verify the data */
    rc = SQLExecDirect(hstmt, "SELECT * FROM my_demo_param",SQL_NTS);
    mystmt(hstmt,rc);

    assert(10 == myresult(hstmt));

  return OK;
}


DECLARE_TEST(my_param_delete)
{
    SQLRETURN  rc;
    SQLINTEGER id;
    SQLROWCOUNT nRowCount;

    /* supply data to parameter 1 */
    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT,
                          SQL_C_LONG, SQL_INTEGER, 0,0,
                          &id, 0, NULL);
    mystmt(hstmt,rc);

    /* execute the DELETE STATEMENT to delete 5th row  */
    id = 5;
    rc = SQLExecDirect(hstmt,"DELETE FROM my_demo_param WHERE id = ?",SQL_NTS);
    mystmt(hstmt,rc);

    /* check the rows affected by the update statement */
    rc = SQLRowCount(hstmt, &nRowCount);
    mystmt(hstmt,rc);
    printMessage(" total rows deleted:%d\n",nRowCount);
    assert( nRowCount == 1);

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    /* execute the DELETE STATEMENT to delete 8th row  */
    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT,
                          SQL_C_LONG, SQL_INTEGER, 0,0,
                          &id, 0, NULL);
    mystmt(hstmt,rc);

    id = 8;
    rc = SQLExecDirect(hstmt,"DELETE FROM my_demo_param WHERE id = ?",SQL_NTS);
    mystmt(hstmt,rc);

    /* check the rows affected by the update statement */
    rc = SQLRowCount(hstmt, &nRowCount);
    mystmt(hstmt,rc);
    printMessage(" total rows deleted:%d\n",nRowCount);
    assert( nRowCount == 1);

    /* Free statement param resorces */
    rc = SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    mystmt(hstmt,rc);

    /* Free statement cursor resorces */
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);

    /* Now fetch and verify the data */
    rc = SQLExecDirect(hstmt, "SELECT * FROM my_demo_param",SQL_NTS);
    mystmt(hstmt,rc);

    assert(8 == myresult(hstmt));

    /* drop the table */
    rc = SQLExecDirect(hstmt,"DROP TABLE my_demo_param",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


BEGIN_TESTS
  ADD_TEST(my_init_table)
  ADD_TEST(my_param_insert)
  ADD_TEST(my_param_update)
  ADD_TEST(my_param_delete)
END_TESTS


RUN_TESTS
