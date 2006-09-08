/* Copyright (C) 1995-2003 MySQL AB

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/***************************************************************************
			  my_result.c  -  description
			     ---------------------
    begin		 : Wed Sep 8 2001
    copyright		 : (C) MySQL AB 1995-2002, www.mysql.com
    author		 : venu ( venu@mysql.com )
 ***************************************************************************/

/***************************************************************************
 *									   *
 *  This is a basic sample to demonstrate how to get the resultset	   *
 *  using  MySQL ODBC 3.51 driver					   *
 *									   *
 ***************************************************************************/

#include "my_utility.h" /* MyODBC 3.51 sample utility header */

/********************************************************
 * result set demo					*
 *********************************************************/
void my_resultset(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN   rc;
  SQLUINTEGER nRowCount=0, pcColDef;
  SQLCHAR     szColName[MAX_NAME_LEN];
  SQLCHAR     szData[MAX_COLUMNS][MAX_ROW_DATA_LEN]={0};
  SQLSMALLINT nIndex,ncol,pfSqlType, pcbScale, pfNullable;

  printf("\nmy_resultset:\n");

  /* drop table 'myodbc3_demo_result' if it already exists */
  rc = SQLExecDirect(hstmt,
		     (SQLCHAR*) "DROP TABLE if exists myodbc3_demo_result",
		     SQL_NTS);
  mystmt(hstmt,rc);

  /* commit the transaction */
  rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
  mycon(hdbc,rc);

  /* create the table 'myodbc3_demo_result' */
  rc = SQLExecDirect(hstmt, (SQLCHAR*) "\
CREATE TABLE myodbc3_demo_result(\
id int primary key auto_increment,\
name varchar(20))",
		     SQL_NTS);
  mystmt(hstmt,rc);

  rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
  mycon(hdbc,rc);

  /* insert 2 rows of data */
  rc = SQLExecDirect(hstmt,(SQLCHAR*) "\
INSERT INTO myodbc3_demo_result values(1,'MySQL')",
		     SQL_NTS);
  mystmt(hstmt,rc);

  rc = SQLExecDirect(hstmt,(SQLCHAR*) "\
INSERT INTO myodbc3_demo_result values(2,'MyODBC')",
		     SQL_NTS);
  mystmt(hstmt,rc);

  /* commit the transaction */
  rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
  mycon(hdbc,rc);

  /* update second row */
  rc = SQLExecDirect(hstmt,(SQLCHAR*) "\
UPDATE myodbc3_demo_result set name='MyODBC 3.51' where id=2",
		     SQL_NTS);
  mystmt(hstmt,rc);

  /* commit the transaction */
  rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
  mycon(hdbc,rc);

  /* now fetch back..*/
  rc = SQLExecDirect(hstmt, (SQLCHAR*) "SELECT * from myodbc3_demo_result",
		     SQL_NTS);
  mystmt(hstmt,rc);

  /* get total number of columns from the resultset */
  rc = SQLNumResultCols(hstmt,&ncol);
  mystmt(hstmt,rc);

  printf(" total columns in resultset:%d\n\n",ncol);

  /* print the column names  and do the row bind */
  for(nIndex = 1; nIndex <= ncol; nIndex++)
  {
    rc = SQLDescribeCol(hstmt,nIndex,szColName, MAX_NAME_LEN+1, NULL,
			&pfSqlType,&pcColDef,&pcbScale,&pfNullable);
    mystmt(hstmt,rc);

    printf(" %s\t",szColName);

    rc = SQLBindCol(hstmt,nIndex, SQL_C_CHAR, szData[nIndex-1],
		    MAX_ROW_DATA_LEN+1,NULL);
    mystmt(hstmt,rc);
  }

  printf("\n--------------------\n");

  /* now fetch row by row */
  rc = SQLFetch(hstmt);
  while(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
  {
    nRowCount++;
    for(nIndex=0; nIndex< ncol; nIndex++)
      printf(" %s\t",szData[nIndex]);

    printf("\n");
    rc = SQLFetch(hstmt);
  }
  SQLFreeStmt(hstmt,SQL_UNBIND);

  printf("\n total rows fetched:%d\n",nRowCount);

  /* free the statement row bind resources */
  rc = SQLFreeStmt(hstmt, SQL_UNBIND);
  mystmt(hstmt,rc);

  /* free the statement cursor */
  rc = SQLFreeStmt(hstmt, SQL_CLOSE);
  mystmt(hstmt,rc);
  printf(" success!!\n");
}

/********************************************************
 * main routine						*
 *********************************************************/
int main(int argc, char *argv[])
{
  SQLHENV    henv;
  SQLHDBC    hdbc;
  SQLHSTMT   hstmt;
  SQLINTEGER narg;

  /*
   *	show the usage string when the user asks for this
   */
  printf("***********************************************\n");
  printf("usage: my_result [DSN] [UID] [PWD] \n");
  printf("***********************************************\n");

  /*
   * if connection string supplied through arguments, overrite
   * the default one..
   */
  for(narg = 1; narg < argc; narg++)
  {
    if ( narg == 1 )
      mydsn = argv[1];
    else if ( narg == 2 )
      myuid = argv[2];
    else if ( narg == 3 )
      mypwd = argv[3];
  }

  /*
   * connect to MySQL server
   */
  myconnect(&henv,&hdbc,&hstmt);

  /*
   * simple resultset demo
   */
  my_resultset(hdbc, hstmt);

  /*
   * disconnect from the server, by freeing all resources
   */
  mydisconnect(henv,hdbc,hstmt);

  return(0);
}
