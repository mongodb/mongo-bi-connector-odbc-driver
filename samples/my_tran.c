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
			  my_tran.c  -	description
			     -------------------
    begin		 : Wed Aug 8 2001
    copyright		 : (C) MySQL AB 1995-2002, www.mysql.com
    author		 : venu ( venu@mysql.com )
 ***************************************************************************/

/***************************************************************************
 *									   *
 *  This is a basic sample to demonstrate the transaction support in	   *
 *  MySQL using  MySQL ODBC 3.51 driver					   *
 *									   *
 ***************************************************************************/

#include "my_utility.h" /* MyODBC 3.51 sample utility header */


/********************************************************
 * Transactional behaviour using BDB/InnoDB table type	*
 *********************************************************/
void my_transaction(SQLHDBC hdbc, SQLHSTMT hstmt)
{
  SQLRETURN rc;

  printf("\nmy_transaction:\n");

  /* set AUTOCOMMIT to OFF */
  rc = SQLSetConnectAttr(hdbc,SQL_ATTR_AUTOCOMMIT,(SQLPOINTER)SQL_AUTOCOMMIT_OFF,0);
  mycon(hdbc,rc);

  rc = SQLExecDirect(hstmt,
		     (SQLCHAR*) "DROP TABLE IF EXISTS my_demo_transaction",
		     SQL_NTS);
  mystmt(hstmt,rc);

  rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
  mycon(hdbc,rc);

  /* create the table 'mytran_demo' of type BDB' or 'InnoDB' */
  rc= SQLExecDirect(hstmt,(SQLCHAR*) "\
CREATE TABLE my_demo_transaction(col1 int ,col2 varchar(30)) TYPE = BDB",
		    SQL_NTS);
  mystmt(hstmt,rc);

  rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
  mycon(hdbc,rc);

  /* insert a row and commit the transaction */
  rc = SQLExecDirect(hstmt,(SQLCHAR*) "\
INSERT INTO my_demo_transaction VALUES(10,'venu')",
		     SQL_NTS);
  mystmt(hstmt,rc);

  rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
  mycon(hdbc,rc);

  /* now insert the second row, and rollback the transaction */
  rc = SQLExecDirect(hstmt,(SQLCHAR*) "\
INSERT INTO my_demo_transaction VALUES(20,'mysql')",
		     SQL_NTS);
  mystmt(hstmt,rc);

  rc = SQLTransact(NULL,hdbc,SQL_ROLLBACK);
  mycon(hdbc,rc);

  /* delete first row, and rollback it */
  rc = SQLExecDirect(hstmt,(SQLCHAR*) "\
DELETE FROM my_demo_transaction WHERE col1 = 10",
		     SQL_NTS);
  mystmt(hstmt,rc);

  rc = SQLTransact(NULL,hdbc,SQL_ROLLBACK);
  mycon(hdbc,rc);

  rc = SQLFreeStmt(hstmt,SQL_CLOSE);
  mystmt(hstmt,rc);

  /* test the results now, only one row should exists */
  rc = SQLExecDirect(hstmt,
		     (SQLCHAR*) "SELECT * FROM my_demo_transaction",
		     SQL_NTS);
  mystmt(hstmt,rc);

  rc = SQLFetch(hstmt);
  mystmt(hstmt,rc);

  rc = SQLFetch(hstmt);
  mystmt_err(hstmt,rc == SQL_NO_DATA_FOUND,rc);

  rc = SQLFreeStmt(hstmt,SQL_CLOSE);
  mystmt(hstmt,rc);

  printf(" success!!\n");
}

/********************************************************
 * main routine						*
 *********************************************************/
int main(int argc, char *argv[])
{
  SQLHENV   henv;
  SQLHDBC   hdbc;
  SQLHSTMT  hstmt;
  SQLINTEGER narg;

  /*
   *	show the usage string when the user asks for this
   */
  printf("***********************************************\n");
  printf("usage: my_tran [DSN] [UID] [PWD] \n");
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
   * simple transaction test
   */
  my_transaction(hdbc, hstmt);

  /*
   * disconnect from the server, by freeing all resources
   */
  mydisconnect(henv,hdbc,hstmt);

  return(0);
}
