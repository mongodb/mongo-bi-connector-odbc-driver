#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include <sql.h>
#include <sqlext.h>

typedef int Bool;
#define  TRUE  1
#define  FALSE 0

#define  ODBC_Error(code) ((SQL_SUCCESS != (code) && SQL_SUCCESS_WITH_INFO != (code)))

void print_error(SQLUSMALLINT handle_type, SQLHANDLE handle)
{
  SQLCHAR      szSqlState[6],szErrorMsg[SQL_MAX_MESSAGE_LENGTH]; 
  SQLINTEGER   pfNativeError; 
  SQLSMALLINT  pcbErrorMsg; 
                                                      
  if (SQLGetDiagRec(handle_type, handle,1, 
                    (SQLCHAR *)&szSqlState, 
                    (SQLINTEGER *)&pfNativeError, 
                    (SQLCHAR *)&szErrorMsg, 
                    SQL_MAX_MESSAGE_LENGTH-1, 
                    (SQLSMALLINT *)&pcbErrorMsg) == SQL_SUCCESS) 
    printf("\n[%s]%s\n",szSqlState,szErrorMsg); 
}

struct db {
   /* Handles **************************************************/
   SQLHENV*    pHenv;                           /* used environment handle */
   SQLHENV     henv;                            /* environment             */
   SQLHDBC     hdbc;                            /* connection              */
};

static Bool connect(SQLHENV*  pHenv,
                    SQLHDBC*  pHdbc)
{
   SQLRETURN   ret;
   char* dsn = "myodbc3";
   char* uid = "root";
   char* pwd = "";

   if(*pHenv == SQL_NULL_HENV) {
      ret = SQLAllocEnv(pHenv);
      if (ODBC_Error(ret))
      {
	 printf("SQLAllocEnv failed, ODBC error: %d", ret);
         return FALSE;
      }
   }

   if(*pHdbc == SQL_NULL_HDBC) {
      ret = SQLAllocConnect(*pHenv,pHdbc);
      if (ODBC_Error(ret))
      {        
	 printf("ODBC error: %d", ret);
         print_error(SQL_HANDLE_ENV, *pHenv);
         return FALSE;
      }
   }

   ret = SQLSetConnectOption(*pHdbc,SQL_LOGIN_TIMEOUT,10);
   if (ODBC_Error(ret))
   {
     printf("ODBC error: %d", ret);
     print_error(SQL_HANDLE_DBC, *pHdbc);
     return FALSE;
   }

   ret = SQLConnect(*pHdbc,(UCHAR*)dsn,SQL_NTS,(UCHAR*)uid,SQL_NTS,(UCHAR*)pwd,SQL_NTS);
   if (ODBC_Error(ret))
   {
     printf("ODBC error: %d", ret);
     print_error(SQL_HANDLE_DBC, *pHdbc);
     return FALSE;
   }

   ret = SQLSetConnectOption(*pHdbc,SQL_AUTOCOMMIT,SQL_AUTOCOMMIT_OFF);
   if (ODBC_Error(ret))
   {
     printf("ODBC error: %d", ret);
     print_error(SQL_HANDLE_DBC, *pHdbc);
     return FALSE;
   }

   return TRUE;
}

static Bool disconnect(SQLHENV*  pHenv,
                       SQLHDBC*  pHdbc,
                       Bool      freeEnv)
{
   SQLRETURN   ret;
   Bool        ok = TRUE;

   if(SQL_NULL_HDBC != *pHdbc) {

      ret = SQLDisconnect(*pHdbc);
      if (ODBC_Error(ret))
      {
	 printf("ODBC error: %d", ret);
         print_error(SQL_HANDLE_DBC, *pHdbc);
         ok = FALSE;
      }

      ret = SQLFreeConnect(*pHdbc);
      if (ODBC_Error(ret))
      {
	 printf("ODBC error: %d", ret);
         print_error(SQL_HANDLE_DBC, *pHdbc);
         ok = FALSE;
      }
      else {
         *pHdbc = SQL_NULL_HDBC;
      }
   }

   if(freeEnv && SQL_NULL_HENV != *pHenv) {

      ret = SQLFreeEnv(*pHenv);
      if (ODBC_Error(ret))
      {
	 printf("ODBC error: %d", ret);
         print_error(SQL_HANDLE_ENV, *pHenv);
         ok = FALSE;
      }
      else {
         *pHenv = SQL_NULL_HENV;
      }
   }

   return ok;

}

Bool prepareDb(struct db** db)
{
   /* Allocate memory for DB handles, parameters and results - i.e. struct db.
    */
   if(NULL == (*db = malloc(sizeof **db))) {
      fprintf(stderr,"prepareDb: can't allocate %u bytes\n",sizeof **db);
      return FALSE;
   }

   /* Initialise the ODBC handles to NULL.
    */
	(*db)->pHenv = &((*db)->henv);
   	(*db)->henv  = SQL_NULL_HENV;
   	(*db)->hdbc  = SQL_NULL_HDBC;

   	/* Try to make a connection
     */
   	if(connect((*db)->pHenv,&((*db)->hdbc)))
		return TRUE;
	return FALSE;

	/*
	 *
	 *	Here we would also prepare statements if the connection was created..
	 *
	 */
}

void unprepareDb(struct db** db)
{
   if(NULL != *db) {

      /* Disconnect
       */
      if(!disconnect((*db)->pHenv,&((*db)->hdbc),TRUE))
         fprintf(stderr,"unprepareDb: can't disconnect\n");

      /* Free the memory allocated for db structure
       */
      free(*db);
      *db = NULL;
   }
}

static void* eval_thread(void* arg)
{
	Bool ok = TRUE;
	struct db* thread_db = NULL;

	printf("Thread %u started\n",*(unsigned int*)(arg));

	ok = prepareDb(&thread_db);

	/*
		while(ok)
		{
			executing lots of transactions through prepared statements....
		}
	*/

	unprepareDb(&thread_db);

	return (ok ? arg : NULL);
}


int main(int argc,char* argv[])
{
	unsigned int i, nThreads;
	pthread_t*     thread_id = NULL;
	void**         thread_result = NULL;
	Bool           ok = TRUE;

	nThreads = 10;

	if(NULL == (thread_id = (pthread_t*)malloc((i= nThreads * sizeof(pthread_t))))) 
      	  fprintf(stderr,"can't allocate %u bytes for thread id's\n",i);
   	else if(NULL == (thread_result = (void**)malloc((i = nThreads * sizeof(void*)))))
	  fprintf(stderr,"can't allocate %u bytes for thread results\n",i);

printf("Memory for threads allocated, starting threads.\n");

	/* Start the threads
	 */
	for(i = 0; ok && i < nThreads; ++i) {
	  if(0 != pthread_create(thread_id+i,NULL,eval_thread,&i)) 
		 fprintf(stderr,"run_threads, not created: thread #%u\n",i);
	}

printf("Threads started.\n");

	/* Release and join the started threads.
 	 */
	for(i = 0; ok && i < nThreads; ++i) {
	 if(0 != pthread_join(thread_id[i],thread_result+i)) 
		fprintf(stderr,"run_threads, not joined: thread #%u\n",i);
	}

	if(NULL != thread_id) free(thread_id);

	return 0;
}




