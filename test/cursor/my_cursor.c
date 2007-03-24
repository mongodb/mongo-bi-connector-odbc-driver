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

#include "mytest3.h" /* MyODBC 3.51 sample utility header */

/********************************************************
* initialize tables                                     *
*********************************************************/
void my_init_table(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN   rc;
    long        id;
    char        name[50];

    /* drop table 'my_demo_param' if it already exists */
    printMessage(" creating table 'my_demo_cursor'\n");

    rc = SQLExecDirect(hstmt,"DROP TABLE if exists my_demo_cursor",SQL_NTS);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* create the table 'my_demo_param' */
    rc = SQLExecDirect(hstmt,"CREATE TABLE my_demo_cursor(\
                              id int, name varchar(20))",SQL_NTS);
    mystmt(hstmt,rc);

    /* commit the transaction*/
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);    

    /* prepare the insert statement with parameters */
    rc = SQLPrepare(hstmt,"INSERT INTO my_demo_cursor VALUES(?,?)",SQL_NTS);
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

    /* now insert 5 rows of data */
    for (id = 0; id < 5; id++)
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
    rc = SQLExecDirect(hstmt, "SELECT * FROM my_demo_cursor",SQL_NTS);
    mystmt(hstmt,rc);

    assert(5 == myresult(hstmt));
}

/********************************************************
* perform positioned update and delete                  *
*********************************************************/
void my_positioned_cursor(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN   rc;
    SQLROWCOUNT nRowCount;
    SQLHSTMT    hstmt_pos;

    /* create new statement handle */
    rc = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt_pos);
    mycon(hdbc, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE,
                        (SQLPOINTER)SQL_CURSOR_DYNAMIC,0);
    mystmt(hstmt, rc);

    /* set the cursor name as 'mysqlcur' on hstmt */
    rc = SQLSetCursorName(hstmt, "mysqlcur", SQL_NTS);
    mystmt(hstmt, rc);

    /* Open the resultset of table 'my_demo_cursor' */
    rc = SQLExecDirect(hstmt,"SELECT * FROM my_demo_cursor",SQL_NTS);
    mystmt(hstmt,rc);

    /* goto the last row */
    rc = SQLFetchScroll(hstmt, SQL_FETCH_LAST, 1L);
    mystmt(hstmt,rc);

    /* now update the name field to 'update' using positioned cursor */
    rc = SQLExecDirect(hstmt_pos, "UPDATE my_demo_cursor SET name='updated' WHERE CURRENT OF mysqlcur", SQL_NTS);
    mystmt(hstmt_pos, rc);

    rc = SQLRowCount(hstmt_pos, &nRowCount);
    mystmt(hstmt_pos, rc);

    printMessage(" total rows updated:%d\n",nRowCount);
    assert(nRowCount == 1);

    /* Free statement cursor resorces */
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt_pos, SQL_CLOSE);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* Now delete 2nd row */
    rc = SQLExecDirect(hstmt,"SELECT * FROM my_demo_cursor",SQL_NTS);
    mystmt(hstmt,rc);

    /* goto the second row row */
    rc = SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, 2L);
    mystmt(hstmt,rc);

    /* now delete the current row */
    rc = SQLExecDirect(hstmt_pos, "DELETE FROM my_demo_cursor WHERE CURRENT OF mysqlcur", SQL_NTS);
    mystmt(hstmt_pos, rc);

    rc = SQLRowCount(hstmt_pos, &nRowCount);
    mystmt(hstmt_pos, rc);

    printMessage(" total rows deleted:%d\n",nRowCount);
    assert(nRowCount == 1);    

    /* free the statement cursor */
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    /* Free the statement 'hstmt_pos' */
    rc = SQLFreeHandle(SQL_HANDLE_STMT, hstmt_pos);
    mystmt(hstmt_pos,rc);

    /* Now fetch and verify the data */
    rc = SQLExecDirect(hstmt, "SELECT * FROM my_demo_cursor",SQL_NTS);
    mystmt(hstmt,rc);

    assert(4 == myresult(hstmt));
}

/********************************************************
* perform delete and update using SQLSetPos             *
*********************************************************/
void my_setpos_cursor(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN   rc;
    SQLROWCOUNT nRowCount;
    long        id;
    char        name[50];

    printMessage("\nmy_setpos_cursor:\n");

    /* Open the resultset of table 'my_demo_cursor' */
    rc = SQLExecDirect(hstmt,"SELECT * FROM my_demo_cursor",SQL_NTS);
    mystmt(hstmt,rc);

    /* bind row data buffers */
    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&id,0,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,name,sizeof(name),NULL);
    mystmt(hstmt,rc);

    /* goto the first row */
    rc = SQLFetchScroll(hstmt, SQL_FETCH_FIRST, 1L);
    mystmt(hstmt,rc);

    strcpy(name,"first-row");

    /* now update the name field to 'first-row' using SQLSetPos */    
    rc = SQLSetPos(hstmt, 1, SQL_UPDATE, SQL_LOCK_NO_CHANGE);
    mystmt(hstmt, rc);

    rc = SQLRowCount(hstmt, &nRowCount);
    mystmt(hstmt, rc);

    printMessage(" total rows updated:%d\n",nRowCount);
    assert(nRowCount == 1);

    /* position to second row and delete it ..*/    
    rc = SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, 2L);
    mystmt(hstmt,rc);

    /* now delete the current, second row */
    rc = SQLSetPos(hstmt, 1, SQL_DELETE, SQL_LOCK_NO_CHANGE);
    mystmt(hstmt, rc);

    rc = SQLRowCount(hstmt, &nRowCount);
    mystmt(hstmt, rc);

    printMessage(" total rows deleted:%d\n",nRowCount);
    assert(nRowCount == 1);

    /* Free statement cursor resorces */
    rc = SQLFreeStmt(hstmt, SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    /* Now fetch and verify the data */
    rc = SQLExecDirect(hstmt, "SELECT * FROM my_demo_cursor",SQL_NTS);
    mystmt(hstmt,rc);

    assert(3 == myresult(hstmt));

    /* drop the table */
    rc = SQLExecDirect(hstmt,"DROP TABLE my_demo_cursor",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT); 
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);  
}


/**
 Bug #5853: Using Update with 'WHERE CURRENT OF' with binary data crashes
*/
void t_bug5853(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;
  SQLHSTMT  hstmt_pos;
  SQLCHAR   nData[3];
  SQLLEN    nLen= SQL_DATA_AT_EXEC;
  int       i= 0;

  rc= SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt_pos);
  mycon(hdbc, rc);

  rc= SQLExecDirect(hstmt,"DROP TABLE IF EXISTS t_bug5853",SQL_NTS);
  mystmt(hstmt,rc);

  rc= SQLExecDirect(hstmt,"CREATE TABLE t_bug5853 (id INT AUTO_INCREMENT PRIMARY KEY, a VARCHAR(3))",SQL_NTS);
  mystmt(hstmt,rc);

  rc= SQLExecDirect(hstmt,"INSERT INTO t_bug5853 (a) VALUES ('abc'),('def')",SQL_NTS);
  mystmt(hstmt,rc);

  rc= SQLPrepare(hstmt,"INSERT INTO t_bug5853 VALUES(?)",SQL_NTS);
  mystmt(hstmt,rc);

  rc= SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE,
                     (SQLPOINTER)SQL_CURSOR_DYNAMIC,0);
  mystmt(hstmt, rc);

  rc= SQLSetCursorName(hstmt, "bug5853", SQL_NTS);
  mystmt(hstmt, rc);

  rc= SQLExecDirect(hstmt,"SELECT * FROM t_bug5853",SQL_NTS);
  mystmt(hstmt,rc);

  rc= SQLPrepare(hstmt_pos,
                 "UPDATE t_bug5853 SET a = ? WHERE CURRENT OF bug5853",
                 SQL_NTS);
  mystmt(hstmt_pos, rc);

  rc= SQLBindParameter(hstmt_pos, 1, SQL_PARAM_INPUT, SQL_VARCHAR, SQL_C_CHAR,
                       0, 0, NULL, 0, &nLen);
  mystmt(hstmt_pos,rc);

  while ((rc= SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0)) != SQL_NO_DATA_FOUND)
  {
    char data[2][3] = { "uvw", "xyz" };

    mystmt(hstmt,rc);

    rc= SQLExecute(hstmt_pos);
    mystmt_err(hstmt_pos, rc == SQL_NEED_DATA, rc);

    while (rc == SQL_NEED_DATA)
    {
      SQLPOINTER token;
      rc= SQLParamData(hstmt_pos, &token);
      if (rc == SQL_NEED_DATA)
      {
        SQLRETURN rc2;
        rc2= SQLPutData(hstmt_pos, data[i++ % 2], sizeof(data[0]));
        mystmt(hstmt_pos,rc2);
      }
    }
    mystmt(hstmt_pos,rc);
  }

  rc= SQLFreeStmt(hstmt_pos, SQL_CLOSE);
  mystmt(hstmt,rc);

  rc= SQLExecDirect(hstmt,"SELECT * FROM t_bug5853",SQL_NTS);
  mystmt(hstmt,rc);

  rc= SQLBindCol(hstmt, 2, SQL_C_CHAR, nData, 3, &nLen);
  mystmt(hstmt,rc);

  rc= SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0);
  mystmt(hstmt,rc);
  my_assert(strncmp((char *)nData, "uvw", 3));

  rc= SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0);
  mystmt(hstmt,rc);
  my_assert(strncmp((char *)nData, "xyz", 3));

  rc= SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0);
  mystmt_err(hstmt, rc == SQL_NO_DATA_FOUND, rc);

  rc= SQLExecDirect(hstmt,"DROP TABLE IF EXISTS t_bug5853",SQL_NTS);
  mystmt(hstmt,rc);

  /* return OK; */
}

/********************************************************
* main routine                                          *
*********************************************************/
int main(int argc, char *argv[])
{
    SQLHENV    henv;
    SQLHDBC    hdbc;
    SQLHSTMT   hstmt;
    SQLINTEGER narg;

    printMessageHeader();

    /*
     * if connection string supplied through arguments, overrite
     * the default one..
    */
    for (narg = 1; narg < argc; narg++)
    {
        if ( narg == 1 )
            mydsn = argv[1];
        else if ( narg == 2 )
            myuid = argv[2];
        else if ( narg == 3 )
            mypwd = argv[3];
    }

    myconnect(&henv,&hdbc,&hstmt);

    if (driver_supports_positioned_ops(hdbc))
    {
        my_init_table(hdbc, hstmt);
        my_positioned_cursor(hdbc, hstmt);
        my_setpos_cursor(hdbc, hstmt);
        t_bug5853(hdbc, hstmt);
    }

    mydisconnect(&henv,&hdbc,&hstmt);

    printMessageFooter( 1 );

    return(0);
}
