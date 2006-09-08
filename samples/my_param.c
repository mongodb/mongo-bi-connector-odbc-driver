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
			  my_param.c  -  description
			     ---------------------
    begin		 : Wed Sep 8 2001
    copyright		 : (C) MySQL AB 1995-2002, www.mysql.com
    author		 : venu ( venu@mysql.com )
 ***************************************************************************/

/***************************************************************************
 *									   *
 *  This is a basic sample to demonstrate how to insert or delete or	   *
 *  update data in the table using parameters				   *
 *									   *
 ***************************************************************************/

#include "my_utility.h" /* MyODBC 3.51 sample utility header */

/********************************************************
 * initialize tables					*
 *********************************************************/
void my_init_table(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN   rc;

  printf("\nmy_init_table:\n");

  /* drop table 'my_demo_param' if it already exists */
  printf(" creating table 'my_demo_param'\n");

  rc = SQLExecDirect(hstmt,(SQLCHAR*) "DROP TABLE if exists my_demo_param",
		     SQL_NTS);
  mystmt(hstmt,rc);

  /* commit the transaction */
  rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
  mycon(hdbc,rc);

  /* create the table 'my_demo_param' */
  rc = SQLExecDirect(hstmt,(SQLCHAR*) "\
CREATE TABLE my_demo_param(\
id   int,\
auto int primary key auto_increment,\
name varchar(20),\
timestamp timestamp(14))",
		     SQL_NTS);
  mystmt(hstmt,rc);

  /* commit the transaction*/
  rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
  mycon(hdbc,rc);

}

/********************************************************
 * prints the statement resultset			*
 *********************************************************/
int my_print_resultset(SQLHSTMT hstmt)
{
  SQLRETURN   rc;
  SQLUINTEGER nRowCount=0, pcColDef;
  SQLCHAR     szColName[MAX_NAME_LEN];
  SQLCHAR     szData[MAX_COLUMNS][MAX_ROW_DATA_LEN]={0};
  SQLSMALLINT nIndex,ncol,pfSqlType, pcbScale, pfNullable;

  /* get total number of columns from the resultset */
  rc = SQLNumResultCols(hstmt,&ncol);
  mystmt(hstmt,rc);

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

  printf("\n -------------------------------------------\n");

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

  return(nRowCount);
}

/********************************************************
 * insert data using parameters				*
 *********************************************************/
void my_param_insert(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN   rc;
  long id;
  char	   name[50];

  printf("\nmy_param_insert:\n");

  /* prepare the insert statement with parameters */
  rc= SQLPrepare(hstmt,
		 (SQLCHAR*) "INSERT INTO my_demo_param(id,name) VALUES(?,?)",
		 SQL_NTS);
  mystmt(hstmt,rc);

  /* now supply data to parameter 1 and 2 */
  rc= SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT,
		       SQL_C_LONG, SQL_INTEGER, 0,0,
		       &id, 0, NULL);
  mystmt(hstmt,rc);

  rc= SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT,
		       SQL_C_CHAR, SQL_CHAR, 0,0,
		       name, sizeof(name), NULL);
  mystmt(hstmt,rc);

  /* now insert 10 rows of data */
  for (id= 0; id < 10; id++)
  {
    sprintf(name,"MySQL%d",id);
    rc= SQLExecute(hstmt);
    mystmt(hstmt,rc);
  }

  /* Free statement param resorces */
  rc= SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
  mystmt(hstmt,rc);

  /* Free statement cursor resorces */
  rc = SQLFreeStmt(hstmt, SQL_CLOSE);
  mystmt(hstmt,rc);

  /* commit the transaction */
  rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
  mycon(hdbc,rc);

  /* Now fetch and verify the data */
  rc = SQLExecDirect(hstmt, (SQLCHAR*) "SELECT * FROM my_demo_param", SQL_NTS);
  mystmt(hstmt,rc);

  assert(10 == my_print_resultset(hstmt));
}

/********************************************************
 * update data using parameters				*
 *********************************************************/
void my_param_update(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN   rc;
  SQLINTEGER  id=9, nRowCount;
  SQLCHAR     name[]="update";

  printf("\nmy_param_update:\n");

  /* prepare the insert statement with parameters */
  rc= SQLPrepare(hstmt,
		 (SQLCHAR*) "UPDATE my_demo_param set name = ? WHERE id = ?",
		 SQL_NTS);
  mystmt(hstmt,rc);

  /* now supply data to parameter 1 and 2 */
  rc= SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT,
		       SQL_C_CHAR, SQL_CHAR, 0,0,
		       name, sizeof(name), NULL);
  mystmt(hstmt,rc);

  rc= SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT,
		       SQL_C_LONG, SQL_INTEGER, 0,0,
		       &id, 0, NULL);
  mystmt(hstmt,rc);

  /* now execute the update statement */
  rc= SQLExecute(hstmt);
  mystmt(hstmt,rc);

  /* check the rows affected by the update statement */
  rc= SQLRowCount(hstmt, &nRowCount);
  mystmt(hstmt,rc);
  printf("\n total rows updated:%d\n",nRowCount);
  assert(nRowCount == 1);

  /* Free statement param resorces */
  rc= SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
  mystmt(hstmt,rc);

  /* Free statement cursor resorces */
  rc= SQLFreeStmt(hstmt, SQL_CLOSE);
  mystmt(hstmt,rc);

  /* commit the transaction */
  rc= SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
  mycon(hdbc,rc);

  /* Now fetch and verify the data */
  rc= SQLExecDirect(hstmt, (SQLCHAR*) "SELECT * FROM my_demo_param",SQL_NTS);
  mystmt(hstmt,rc);

  assert(10 == my_print_resultset(hstmt));
}


/********************************************************
 * delete data using parameters				*
 *********************************************************/
void my_param_delete(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN   rc;
  long id, nRowCount;

  printf("\nmy_param_delete:\n");

  /* supply data to parameter 1 */
  rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT,
			SQL_C_LONG, SQL_INTEGER, 0,0,
			&id, 0, NULL);
  mystmt(hstmt,rc);

  /* execute the DELETE STATEMENT to delete 5th row  */
  id = 5;
  rc = SQLExecDirect(hstmt,
		     (SQLCHAR*) "DELETE FROM my_demo_param WHERE id = ?",
		     SQL_NTS);
  mystmt(hstmt,rc);

  /* check the rows affected by the update statement */
  rc = SQLRowCount(hstmt, &nRowCount);
  mystmt(hstmt,rc);
  printf(" total rows deleted:%d\n",nRowCount);
  assert( nRowCount == 1);

  /* execute the DELETE STATEMENT to delete 8th row  */
  id = 8;
  rc = SQLExecDirect(hstmt,
		     (SQLCHAR*) "DELETE FROM my_demo_param WHERE id = ?",
		     SQL_NTS);
  mystmt(hstmt,rc);

  /* check the rows affected by the update statement */
  rc = SQLRowCount(hstmt, &nRowCount);
  mystmt(hstmt,rc);
  printf(" total rows deleted:%d\n",nRowCount);
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
  rc = SQLExecDirect(hstmt, (SQLCHAR*) "SELECT * FROM my_demo_param",
		     SQL_NTS);
  mystmt(hstmt,rc);

  assert(8 == my_print_resultset(hstmt));
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
  int rc;

  /*
   * if connection string supplied through arguments, overrite
   * the default one..
   */
  for (narg= 1; narg < argc; narg++)
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
   * initialize table
   */
  my_init_table(hdbc, hstmt);

  /*
   * insert data using parameters
   */
  my_param_insert(hdbc, hstmt);

  /*
   * parameter update
   */
  my_param_update(hdbc, hstmt);

  /*
   * parameter delete
   */
  my_param_delete(hdbc, hstmt);

  /* Clean up after oursleves. */
  rc = SQLExecDirect(hstmt,(SQLCHAR*) "DROP TABLE if exists my_demo_param",
		     SQL_NTS);
  mystmt(hstmt,rc);

  /*
   * disconnect from the server, by freeing all resources
   */
  mydisconnect(henv,hdbc,hstmt);

  return(0);
}
