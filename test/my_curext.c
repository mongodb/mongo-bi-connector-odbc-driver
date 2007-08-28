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

DECLARE_TEST(my_pcbvalue)
{
    SQLRETURN   rc;
    SQLROWCOUNT nRowCount;
    SQLINTEGER  nData= 500;
    SQLLEN      int_pcbValue, pcbValue, pcbValue1, pcbValue2;
    SQLCHAR     szData[255]={0};

  ok_sql(hstmt, "DROP TABLE IF EXISTS my_pcbValue");

    rc = SQLExecDirect(hstmt,"create table my_pcbValue(id int, name varchar(30),\
                                                       name1 varchar(30),\
                                                       name2 varchar(30))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into my_pcbValue(id,name) values(100,'venu')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into my_pcbValue(id,name) values(200,'monty')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,0,&int_pcbValue);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,15,&pcbValue);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,3,SQL_C_CHAR,szData,3,&pcbValue1);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,4,SQL_C_CHAR,szData,2,&pcbValue2);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC, 0);
    mystmt(hstmt, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY ,(SQLPOINTER)SQL_CONCUR_ROWVER , 0);
    mystmt(hstmt, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE  ,(SQLPOINTER)1 , 0);
    mystmt(hstmt, rc);

    /* Open the resultset of table 'my_demo_cursor' */
    rc = SQLExecDirect(hstmt,"SELECT * FROM my_pcbValue",SQL_NTS);
    mystmt(hstmt,rc);

    /* goto the last row */
    rc = SQLFetchScroll(hstmt, SQL_FETCH_LAST, 1L);
    mystmt(hstmt,rc);

    /* Now delete the newly updated record */
    strcpy((char*)szData,"updated");
    nData = 99999;

    int_pcbValue=2;
    pcbValue=3;
    pcbValue1=9;
    pcbValue2=SQL_NTS;

    rc = SQLSetPos(hstmt,1,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt, &nRowCount);
    mystmt(hstmt, rc);

    printMessage(" total rows updated:%d\n",nRowCount);
    is(nRowCount == 1);

    /* Free statement cursor resorces */
    rc = SQLFreeStmt(hstmt, SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);

    /* Now fetch and verify the data */
    rc = SQLExecDirect(hstmt, "SELECT * FROM my_pcbValue",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt,1,SQL_C_LONG,&nData,0,NULL);
    mystmt(hstmt,rc);
    printMessage("\n nData :%d",nData);
    myassert(nData == 99999);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,50,NULL);
    mystmt(hstmt,rc);
    printMessage("\n szData:%s\n",szData);
    myassert(strcmp(szData,"upd") == 0);

    rc = SQLGetData(hstmt,3,SQL_C_CHAR,szData,50,NULL);
    mystmt(hstmt,rc);
    printMessage("\n szData:%s\n",szData);
    myassert(strcmp(szData,"updated") == 0);

    rc = SQLGetData(hstmt,4,SQL_C_CHAR,szData,50,NULL);
    mystmt(hstmt,rc);
    printMessage("\n szData:%s\n",szData);
    myassert(strcmp(szData,"updated") == 0);

    rc = SQLFetch(hstmt);
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);

  ok_sql(hstmt, "DROP TABLE IF EXISTS my_pcbValue");

  return OK;
}


/* to test the pcbValue on cursor ops **/
DECLARE_TEST(my_pcbvalue_add)
{
    SQLRETURN   rc;
    SQLROWCOUNT nRowCount;
    SQLINTEGER  nData= 500;
    SQLLEN      int_pcbValue, pcbValue, pcbValue1, pcbValue2;
    SQLCHAR     szData[255]={0};

  ok_sql(hstmt, "DROP TABLE IF EXISTS my_pcbValue_add");

    rc = SQLExecDirect(hstmt,"create table my_pcbValue_add(id int, name varchar(30),\
                                                       name1 varchar(30),\
                                                       name2 varchar(30))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into my_pcbValue_add(id,name) values(100,'venu')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into my_pcbValue_add(id,name) values(200,'monty')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,0,&int_pcbValue);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,szData,15,&pcbValue);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,3,SQL_C_CHAR,szData,3,&pcbValue1);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,4,SQL_C_CHAR,szData,2,&pcbValue2);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC, 0);
    mystmt(hstmt, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY ,(SQLPOINTER)SQL_CONCUR_ROWVER , 0);
    mystmt(hstmt, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE  ,(SQLPOINTER)1 , 0);
    mystmt(hstmt, rc);

    /* Open the resultset of table 'my_pcbValue_add' */
    rc = SQLExecDirect(hstmt,"SELECT * FROM my_pcbValue_add",SQL_NTS);
    mystmt(hstmt,rc);

    /* goto the last row */
    rc = SQLFetchScroll(hstmt, SQL_FETCH_LAST, 1L);
    mystmt(hstmt,rc);

    /* Now delete the newly updated record */
    strcpy((char*)szData,"inserted");
    nData = 99999;

    int_pcbValue=2;
    pcbValue=3;
    pcbValue1=6;
    pcbValue2=SQL_NTS;

    rc = SQLSetPos(hstmt,1,SQL_ADD,SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt, &nRowCount);
    mystmt(hstmt, rc);

    printMessage(" total rows updated:%d\n",nRowCount);
    is(nRowCount == 1);

    /* Free statement cursor resorces */
    rc = SQLFreeStmt(hstmt, SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);

    /* Now fetch and verify the data */
    rc = SQLExecDirect(hstmt, "SELECT * FROM my_pcbValue_add",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt,1,SQL_C_LONG,&nData,0,NULL);
    mystmt(hstmt,rc);
    printMessage("\n nData :%d",nData);
    myassert(nData == 99999);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,szData,50,NULL);
    mystmt(hstmt,rc);
    printMessage("\n szData:%s\n",szData);
    myassert(strcmp(szData,"ins") == 0);

    rc = SQLGetData(hstmt,3,SQL_C_CHAR,szData,50,NULL);
    mystmt(hstmt,rc);
    printMessage("\n szData:%s\n",szData);
    myassert(strcmp(szData,"insert") == 0);

    rc = SQLGetData(hstmt,4,SQL_C_CHAR,szData,50,NULL);
    mystmt(hstmt,rc);
    printMessage("\n szData:%s\n",szData);
    myassert(strcmp(szData,"inserted") == 0);

    rc = SQLFetch(hstmt);
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);

  ok_sql(hstmt, "DROP TABLE IF EXISTS my_pcbValue_add");

  return OK;
}


/* spaces in column names */
DECLARE_TEST(my_columnspace)
{
    SQLRETURN   rc;

  ok_sql(hstmt, "DROP TABLE IF EXISTS TestColNames");

    rc = SQLExecDirect(hstmt,"CREATE TABLE `TestColNames`(`Value One` text,\
                                                           `Value Two` text,\
                                                           `Value Three` text)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLExecDirect(hstmt,"INSERT INTO TestColNames VALUES ('venu','anuganti','mysql ab')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"INSERT INTO TestColNames VALUES ('monty','widenius','mysql ab')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"SELECT * FROM `TestColNames`",SQL_NTS);
    mystmt(hstmt,rc);

    myassert(2 == my_print_non_format_result(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"SELECT `Value One`,`Value Two`,`Value Three`  FROM `TestColNames`",SQL_NTS);
    mystmt(hstmt,rc);

    myassert(2 == my_print_non_format_result(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  ok_sql(hstmt, "DROP TABLE IF EXISTS TestColNames");

  return OK;
}


/* to test the empty string returning NO_DATA */
DECLARE_TEST(my_empty_string)
{
    SQLRETURN   rc;
    SQLLEN      pcbValue;
    SQLCHAR     szData[255]={0};

  ok_sql(hstmt, "DROP TABLE IF EXISTS my_empty_string");

    rc = SQLExecDirect(hstmt,"create table my_empty_string(name varchar(30))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into my_empty_string values('')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    /* Now fetch and verify the data */
    rc = SQLExecDirect(hstmt, "SELECT * FROM my_empty_string",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,szData,50,&pcbValue);
    mystmt(hstmt,rc);
    printMessage("\n szData:%s(%d)\n",szData,pcbValue);

    rc = SQLFetch(hstmt);
    mystmt_err(hstmt,rc==SQL_NO_DATA_FOUND,rc);

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);

  ok_sql(hstmt, "DROP TABLE IF EXISTS my_empty_string");

  return OK;
}


BEGIN_TESTS
  ADD_TEST(my_pcbvalue)
  ADD_TEST(my_pcbvalue_add)
  ADD_TEST(my_columnspace)
  ADD_TEST(my_empty_string)
END_TESTS


RUN_TESTS
