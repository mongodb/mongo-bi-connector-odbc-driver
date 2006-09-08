/* system includes */
#include <stdio.h>
#include <stdlib.h>

#include <sql.h>
#include <sqlext.h>

SQLRETURN db_odbc_test (SQLCHAR *, SQLCHAR *, SQLCHAR *);
SQLRETURN db_odbc_get_lastautoincrement_id( SQLHDBC hdbc, SQLUINTEGER *maxId);
SQLRETURN db_odbc_exec_sql_stmt ( SQLHDBC hdbc, SQLCHAR *stmtStr);
void db_odbc_error(SQLHENV henv, SQLHDBC hdbc, SQLHSTMT hstmt, const SQLCHAR* stmt);

int 
main(int argc, char **argv)
{
  SQLCHAR    *szDsn= (SQLCHAR *)"myodbc3";
  SQLCHAR    *szUser= (SQLCHAR *)"venu";
  SQLCHAR    *szPassword= (SQLCHAR *)"venu";
	SQLRETURN  nArg, iResult = SQL_ERROR;

  printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
  printf("usage: db_test [DSN] [UID] [PWD] \n\n");
  printf("       DSN    <-- data source name\n");
  printf("       UID    <-- user name\n");
  printf("       PWD    <-- password\n");
  printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

  for (nArg = 1; nArg < argc; nArg++)
  {
    if ( nArg == 1 )
      szDsn = argv[1];
    else if ( nArg == 2 )
      szUser = argv[2];
    else if ( nArg == 3 )
      szPassword = argv[3];
  }
	iResult = db_odbc_test( szDsn, szUser, szPassword );
	if ( iResult == SQL_ERROR )
		printf("\n Error!\n");
  else
		printf("\n Success\n");
  return 0;
}

void 
print_server_version( SQLHDBC hdbc )
{
  SQLCHAR server_version[65];
  SQLRETURN rc;

  rc = SQLGetInfo( hdbc, SQL_DBMS_VER, server_version, 64, NULL );
  if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
  {
    printf("\n Successfully connected to server 'mysqld-%s'",server_version);
  }
}

SQLRETURN
db_odbc_test (SQLCHAR *szDsn, SQLCHAR *szUser, SQLCHAR *szPassword)
{
   SQLHENV     henv;      
   SQLHDBC     hdbc;       
   SQLUINTEGER maxId;
   SQLCHAR     stmt[200]; 
   SQLRETURN   rc;

   rc = SQLAllocEnv (&henv);
   if ((rc == SQL_SUCCESS) || (rc == SQL_SUCCESS_WITH_INFO))
      rc = SQLAllocConnect (henv, &hdbc);

   rc = SQLConnect (hdbc, szDsn, SQL_NTS, szUser, SQL_NTS, szPassword, SQL_NTS);
   if ( (rc == SQL_SUCCESS) || (rc == SQL_SUCCESS_WITH_INFO) ) 
   {
     /* turn off auto-commit; must use SQLTransact() to commit DB mods */
     rc = SQLSetConnectOption( hdbc, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF ); 
   }
   else 
   {
     db_odbc_error( SQL_NULL_HENV, hdbc, SQL_NULL_HSTMT, "SQLConnect()" );
     return SQL_ERROR;
   }
   print_server_version( hdbc );
   
   sprintf ( stmt , "CREATE DATABASE IF NOT EXISTS test");
   rc = db_odbc_exec_sql_stmt (hdbc, stmt);
   if ( rc == SQL_ERROR)
   {
     printf ( "Error create database.\n");
     return SQL_ERROR;
   }
 
   sprintf ( stmt , "USE test");
   rc = db_odbc_exec_sql_stmt (hdbc, stmt);
   if ( rc == SQL_ERROR) 
   {
     printf ( "Error use database.\n");
     return SQL_ERROR;
   }
   
   sprintf ( stmt , "DROP TABLE test");
   db_odbc_exec_sql_stmt (hdbc, stmt); 
   
   sprintf ( stmt , "CREATE TABLE test (id int PRIMARY KEY AUTO_INCREMENT, name char(64) NOT NULL)");
   rc = db_odbc_exec_sql_stmt (hdbc, stmt); 
   if ( rc == SQL_ERROR) 
   {
	   printf ( "Error creating table.\n");
   	 return SQL_ERROR;
   }

   sprintf ( stmt , "INSERT INTO test (name) VALUES (\"Testing\")");
   rc = db_odbc_exec_sql_stmt (hdbc, stmt); 
   if ( rc == SQL_ERROR) 
   {
	   printf ( "Error Inserting row.\n");
     return SQL_ERROR;
   }

   rc = db_odbc_get_lastautoincrement_id (hdbc, &maxId);
   if ( rc == SQL_ERROR) 
   {
	   printf ( "Error Fetching last_insert_id .\n");
   	 return SQL_ERROR;
   }
   return SQL_SUCCESS;
}

SQLRETURN 
db_odbc_get_lastautoincrement_id(SQLHDBC hdbc, SQLUINTEGER *maxId)
{
   SQLRETURN   rc;
   SQLHSTMT    hstmt;
   SQLCHAR     stmtStr[100];
   SQLUINTEGER valMaxId;
   SQLUINTEGER InsertedId;

   /* Initialize maxId */
   *maxId = 0;

   /* Build the SQL statement */
   sprintf(stmtStr, "SELECT LAST_INSERT_ID()");

   /* Allocate ODBC statement structure */
   rc = SQLAllocStmt( hdbc, &hstmt );
   if ( (rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO) )
   {
      db_odbc_error( SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, "GetLastInsertId: SQLAllocStmt" );
      return SQL_ERROR;
   }

   /* Bind return value */
   rc = SQLBindCol( hstmt, 1, SQL_C_LONG, &InsertedId,
                    (SDWORD)sizeof(SDWORD), &valMaxId);

   /* Execute statement */
   rc = SQLExecDirect( hstmt, stmtStr, SQL_NTS );
   if ( (rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO) )
   {
      db_odbc_error( SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, "GetLastInsertId: SQLExecDirect" );
      rc = SQLFreeStmt( hstmt, SQL_DROP );
      return SQL_ERROR;
   }

   /* Get return value */
   rc = SQLFetch( hstmt );
   if ( (rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO)
        && (rc != SQL_NO_DATA_FOUND) )
   {
      db_odbc_error( SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, "GetLastInsertId: SQLFetch" );
      return SQL_ERROR;
   }
   *maxId = InsertedId;
   printf("\n Last Inserted id: %ld", InsertedId);
   rc = SQLFreeStmt( hstmt, SQL_DROP );
   return SQL_SUCCESS;
}

SQLRETURN
db_odbc_exec_sql_stmt (SQLHDBC  hdbc, SQLCHAR  *stmtStr)
{
   SQLRETURN rc;
   SQLHSTMT	 hstmt;

   /* Allocate a SQLHSTMT to communicate with ODBC DB Driver.  */
   rc = SQLAllocStmt( hdbc, &hstmt );
   if ( (rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO) )
   {
     db_odbc_error( SQL_NULL_HENV, hdbc, SQL_NULL_HSTMT, NULL );
     return SQL_ERROR;
   }

   /* Execute statement */
   rc = SQLExecDirect( hstmt, stmtStr, SQL_NTS );
   if ( (rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO) )
   {
      db_odbc_error( SQL_NULL_HENV, SQL_NULL_HDBC, hstmt, "ExecSQLStmt: SQLExecDirect" );
      return SQL_ERROR;
   }
   return  SQL_SUCCESS;
}

void db_odbc_error(SQLHENV henv, SQLHDBC hdbc, SQLHSTMT hstmt, const SQLCHAR* stmt)
{
   SQLCHAR      sqlstate[6];
   SQLCHAR      errmsg[SQL_MAX_MESSAGE_LENGTH];
   SQLINTEGER   nativeerr;
   SQLUSMALLINT actualmsglen = 0;
   SQLRETURN    rc;

   rc = SQLError(henv, hdbc, hstmt,
                 sqlstate, &nativeerr, errmsg,
                 SQL_MAX_MESSAGE_LENGTH - 1, &actualmsglen);
   
   if (rc == SQL_ERROR)
      return;
   
   if (rc != SQL_NO_DATA_FOUND)
      errmsg[actualmsglen] = '\0';
   
   if (stmt)
     printf("\n Error occured in %s\n", stmt);
   printf("[%s][%ld:%s]\n",sqlstate,nativeerr,errmsg);
}

