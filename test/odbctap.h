/*
  Copyright (C) 2007 MySQL AB

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

/**
  @file odbctap.h

  A basic interface for writing tests that produces TAP-compliant output.
*/

#ifdef HAVE_CONFIG_H
# include <myconf.h>
#endif
/* Work around iODBC header bug on Mac OS X 10.3 */
#undef HAVE_CONFIG_H

#include <stdarg.h>
#ifdef WIN32
#  include <windows.h>
#  define sleep(x) Sleep(x*1000)
#else
#  include <unistd.h>
#  include <signal.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sql.h>
#include <sqlext.h>

/* for clock() */
#include <time.h>


void printMessage(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  fprintf(stdout, "# ");
  vfprintf(stdout, fmt, ap);
  fprintf(stdout, "\n");
  va_end(ap);
}


#define MAX_NAME_LEN 255
#define MAX_COLUMNS 500
#define MAX_ROW_DATA_LEN 1000
#define MYSQL_NAME_LEN 64

SQLCHAR *mydsn= (SQLCHAR *)"test";
SQLCHAR *myuid= (SQLCHAR *)"root";
SQLCHAR *mypwd= (SQLCHAR *)"";
SQLCHAR *mysock= NULL;
int      myoption= 0;
SQLCHAR *myserver= (SQLCHAR *)"localhost";
SQLCHAR *mydb= (SQLCHAR *)"test";
SQLCHAR *test_db= (SQLCHAR *)"client_odbc_test";

int g_nCursor;
char g_szHeader[501];

#ifndef OK
# define OK 0
#endif
#ifndef FAIL
# define FAIL 1
#endif
#ifndef SKIP
# define SKIP -1
#endif
#ifndef FALSE
# define FALSE 0
#endif
#ifndef TRUE
# define TRUE 1
#endif

char *SKIP_REASON= NULL;

typedef int (*test_func)(SQLHDBC, SQLHSTMT, SQLHENV);
static void print_diag(SQLRETURN rc, SQLSMALLINT htype, SQLHANDLE handle,
		       const char *text, const char *file, int line);

/* Disable _attribute__ on non-gcc compilers. */
#if !defined(__attribute__) && !defined(__GNUC__)
# define __attribute__(arg)
#endif

/*
 The parameters may be unused. so we add the attribute to stifle warnings.
 They may also still be used, and no warning will be generated.
*/
#define DECLARE_TEST(name) \
  int (name)(SQLHDBC hdbc __attribute__((unused)), \
             SQLHSTMT hstmt __attribute__((unused)), \
             SQLHENV henv __attribute__((unused)))

typedef struct {
  char *name;
  test_func func;
  int   expect;
} my_test;

#ifdef WIN32
#define ENABLE_ALARMS
#define RUN_TESTS_SIGNAL
#define RUN_TESTS_ALARM
#else
#define ENABLE_ALARMS    int do_alarms= !getenv("DISABLE_TIMEOUT")
#define RUN_TESTS_SIGNAL (void)signal(SIGALRM, test_timeout)
#define RUN_TESTS_ALARM  if (do_alarms) alarm(30)
#endif

#define BEGIN_TESTS my_test tests[]= {
#define ADD_TEST(name) { #name, name, OK   },
#define ADD_TODO(name) { #name, name, FAIL },
#define END_TESTS }; \
void test_timeout(int signum __attribute__((unused))) \
{ \
  printf("Bail out! Timeout.\n"); \
  exit(1); \
} \
\
int main(int argc, char **argv) \
{ \
  SQLHENV  henv; \
  SQLHDBC  hdbc; \
  SQLHSTMT hstmt; \
  int      i, num_tests, failcnt= 0; \
  ENABLE_ALARMS; \
\
  /* Set from environment, possibly overrided by command line */ \
  if (getenv("TEST_DSN")) \
    mydsn=  (SQLCHAR *)getenv("TEST_DSN"); \
  if (getenv("TEST_UID")) \
    myuid=  (SQLCHAR *)getenv("TEST_UID"); \
  if (getenv("TEST_PASSWORD")) \
    mypwd=  (SQLCHAR *)getenv("TEST_PASSWORD"); \
  if (getenv("TEST_SOCKET")) \
    mysock= (SQLCHAR *)getenv("TEST_SOCKET"); \
\
  if (argc > 1) \
    mydsn= (SQLCHAR *)argv[1]; \
  if (argc > 2) \
    myuid= (SQLCHAR *)argv[2]; \
  if (argc > 3) \
    mypwd= (SQLCHAR *)argv[3]; \
  if (argc > 4) \
    mysock= (SQLCHAR *)argv[4];

#define SET_DSN_OPTION(x) \
  myoption= (x);

#define RUN_TESTS \
  setbuf(stdout, NULL); \
  num_tests= sizeof(tests) / sizeof(tests[0]); \
  printf("1..%d\n", num_tests); \
\
  RUN_TESTS_SIGNAL; \
\
  if (alloc_basic_handles(&henv, &hdbc, &hstmt) != OK) \
    exit(1); \
\
  for (i= 0; i < num_tests; i++ ) \
  { \
    int rc; \
    RUN_TESTS_ALARM; \
    rc= tests[i].func(hdbc, hstmt, henv); \
    printf("%s %d - %s %s%s\n", \
           (rc == OK || rc == SKIP) ? "ok" : "not ok", \
           i + 1, \
           tests[i].name, \
           (tests[i].expect == FAIL ? "# TODO" : \
            rc == SKIP ? "# SKIP " : ""), \
           SKIP_REASON ? SKIP_REASON : ""); \
    if (rc == FAIL != tests[i].expect) \
      failcnt++; \
    SKIP_REASON= NULL; /* Reset SKIP_REASON */ \
    /* Re-allocate statement to reset all its properties. */ \
    SQLFreeStmt(hstmt, SQL_DROP); \
    SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt); \
  } \
\
  (void)free_basic_handles(&henv, &hdbc, &hstmt); \
\
  exit(failcnt); \
}


/**
 Skip a test, giving a reason.
*/
#define skip(reason) \
  do { \
    SKIP_REASON= reason; \
    return SKIP; \
  } while (0)


/**
  Execute an SQL statement and bail out if the execution does not return
  SQL_SUCCESS or SQL_SUCCESS_WITH_INFO.

  @param statement Handle for statement object
  @param query     The query to execute
*/
#define ok_sql(statement, query) \
do { \
  SQLRETURN rc= SQLExecDirect((statement), (SQLCHAR *)(query), SQL_NTS); \
  print_diag(rc, SQL_HANDLE_STMT, (statement), \
             "SQLExecDirect(" #statement ", \"" query "\", SQL_NTS)",\
             __FILE__, __LINE__); \
  if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) \
    return FAIL; \
} while (0)


/**
  Verify that the result of an SQL statement call matches an expected
  result, such as SQL_ERROR.

  @param statement Handle for statement object
  @param query     The query to execute
  @param expect    The expected result
*/
#define expect_sql(statement, query, expect) \
do { \
  SQLRETURN rc= SQLExecDirect((statement), (SQLCHAR *)(query), SQL_NTS); \
  if (rc != (expect)) \
  { \
    print_diag(rc, SQL_HANDLE_STMT, (statement), \
               "SQLExecDirect(" #statement ", \"" query "\", SQL_NTS)",\
               __FILE__, __LINE__); \
    printf("# Expected %d, but got %d in %s on line %d\n", expect, rc, \
           __FILE__, __LINE__); \
    return FAIL; \
  } \
} while (0)


/**
  Verify that the results of an ODBC function call on a statement handle was
  SQL_SUCCESS or SQL_SUCCESS_WITH_INFO.

  @param statement Handle for statement object
  @param call      The function call
*/
#define ok_stmt(statement, call) \
do { \
  SQLRETURN rc= (call); \
  print_diag(rc, SQL_HANDLE_STMT, (statement), #call, __FILE__, __LINE__); \
  if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) \
    return FAIL; \
} while (0)


/**
  If calling a help function that doesn't return OK,
  terminate as we would have using ok_stmt()

  @param call  The function call
*/
#define nok_pass_on(call) \
do { \
  int rc= (call); \
  if (rc != OK) \
    return rc; \
} while (0)


/**
  Verify that the result of an ODBC function call matches an expected
  result, such as SQL_NO_DATA_FOUND.

  @param statement Handle for statement object
  @param call      The function call
  @param expect    The expected result
*/
#define expect_stmt(statement, call, expect) \
do { \
  SQLRETURN rc= (call); \
  if (rc != (expect)) \
  { \
    print_diag(rc, SQL_HANDLE_STMT, (statement), #call, __FILE__, __LINE__); \
    printf("# Expected %d, but got %d in %s on line %d\n", (expect), rc, \
           __FILE__, __LINE__); \
    return FAIL; \
  } \
} while (0)


/**
  Verify that the results of an ODBC function call on an environment handle
  was SQL_SUCCESS or SQL_SUCCESS_WITH_INFO.

  @param environ Handle for environment
  @param call    The function call
*/
#define ok_env(environ, call) \
do { \
  SQLRETURN rc= (call); \
  print_diag(rc, SQL_HANDLE_ENV, (environ), #call, __FILE__, __LINE__); \
  if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) \
    return FAIL; \
} while (0)


/**
  Verify that the results of an ODBC function call on a connection handle
  was SQL_SUCCESS or SQL_SUCCESS_WITH_INFO.

  @param con   Handle for database connection
  @param call  The function call
*/
#define ok_con(con, call) \
do { \
  SQLRETURN rc= (call); \
  print_diag(rc, SQL_HANDLE_DBC, (con), #call, __FILE__, __LINE__); \
  if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) \
    return FAIL; \
} while (0)


/**
  Verify that a Boolean expression is true.

  @param a     The expression to check
*/
#define is(a) \
do { \
  if (!(a)) { \
    printf("# !(%s) in %s on line %d\n", \
           #a, __FILE__, __LINE__); \
    return FAIL; \
  } \
} while (0);


#define myassert(a) is(a)
#define my_assert(a) is(a)


/**
  Verify that a string (char *) matches an expected value.

  @param a     The string to compare
  @param b     The string to compare against
  @param c     The number of characters to compare
*/
#define is_str(a, b, c) \
do { \
  char *val_a= (char *)(a), *val_b= (char *)(b); \
  int val_len= (int)(c); \
  if (strncmp(val_a, val_b, val_len) != 0) { \
    printf("# %s ('%*s') != '%*s' in %s on line %d\n", \
           #a, val_len, val_a, val_len, val_b, __FILE__, __LINE__); \
    return FAIL; \
  } \
} while (0);


/**
  Verify that a number (long integer) matches an expected value.

  @param a     The number to compare
  @param b     The number to compare against
*/
#define is_num(a, b) \
do { \
  if (a != b) { \
    printf("# %s (%ld) != %ld in %s on line %d\n", \
           #a, (long)a, (long)b, __FILE__, __LINE__); \
    return FAIL; \
  } \
} while (0);


int check_sqlstate(SQLHSTMT hstmt, char *sqlstate)
{
  SQLCHAR     sql_state[6];
  SQLINTEGER  err_code= 0;
  SQLCHAR     err_msg[SQL_MAX_MESSAGE_LENGTH]= {0};
  SQLSMALLINT err_len= 0;

  memset(err_msg, 'C', SQL_MAX_MESSAGE_LENGTH);
  SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sql_state, &err_code, err_msg,
                SQL_MAX_MESSAGE_LENGTH - 1, &err_len);

  is_str(sql_state, (SQLCHAR *)sqlstate, 5);

  return OK;
}


/**
*/
static void print_diag(SQLRETURN rc, SQLSMALLINT htype, SQLHANDLE handle,
		       const char *text, const char *file, int line)
{
  if (!SQL_SUCCEEDED(rc))
  {
    SQLCHAR     sqlstate[6], message[SQL_MAX_MESSAGE_LENGTH];
    SQLINTEGER  native_error;
    SQLSMALLINT length;
    SQLRETURN   drc;

    /** @todo map rc to SQL_SUCCESS_WITH_INFO, etc */
    printf("# %s = %d\n", text, rc);

    /** @todo Handle multiple diagnostic records. */
    drc= SQLGetDiagRec(htype, handle, 1, sqlstate, &native_error,
                       message, SQL_MAX_MESSAGE_LENGTH - 1, &length);

    if (SQL_SUCCEEDED(drc))
      printf("# [%6s] %*s in %s on line %d\n",
             sqlstate, length, message, file, line);
    else
      printf("# Did not get expected diagnostics from SQLGetDiagRec() = %d"
             " in file %s on line %d\n", drc, file, line);
  }
}


/* PROTOTYPE */
void myerror( SQLRETURN rc, SQLSMALLINT htype, SQLHANDLE handle, const char *szFile, int nLine );

#define my_error() fprintf(stdout," ERROR occured at %d@%s\n",__LINE__,__FILE__)

SQLUINTEGER myresult(SQLHSTMT hstmt);

/* UTILITY MACROS */
#define myenv(henv,r)  \
        if ( ((r) != SQL_SUCCESS) ) \
            myerror( r, 1, henv,  __FILE__, __LINE__ ); \
        my_assert( ((r) == SQL_SUCCESS) || ((r) == SQL_SUCCESS_WITH_INFO) )

#define myenv_r(henv,r)  \
        if ( r == SQL_ERROR ) \
            myerror( r, 1, henv,  __FILE__, __LINE__ ); \
        my_assert( r == SQL_ERROR )

#define myenv_err(henv,r,rc)  \
        if ( rc == SQL_ERROR || rc == SQL_SUCCESS_WITH_INFO ) \
            myerror( rc, 1, henv,  __FILE__, __LINE__ ); \
        my_assert( r )

#define mycon(hdbc,r)  \
        if ( ((r) != SQL_SUCCESS) ) \
            myerror( r, 2, hdbc,  __FILE__, __LINE__ ); \
        my_assert( ((r) == SQL_SUCCESS) || ((r) == SQL_SUCCESS_WITH_INFO) )

#define mycon_r(hdbc,r)  \
        if ( r == SQL_ERROR ) \
            myerror( r, 2, hdbc,  __FILE__, __LINE__ ); \
        my_assert(r==SQL_ERROR)

#define mycon_err(hdbc,r,rc)  \
        if ( rc == SQL_ERROR || rc == SQL_SUCCESS_WITH_INFO ) \
            myerror( rc, 2, hdbc, __FILE__, __LINE__ ); \
        my_assert( r )

#define mystmt(hstmt,r)  \
        if ( ((r) != SQL_SUCCESS) ) \
            myerror( r, 3, hstmt,  __FILE__, __LINE__ ); \
        my_assert( ((r) == SQL_SUCCESS) || ((r) == SQL_SUCCESS_WITH_INFO) )

#define mystmt_r(hstmt,r)  \
        if ( r == SQL_ERROR ) \
            myerror( r, 3, hstmt,  __FILE__, __LINE__ ); \
        my_assert( r == SQL_ERROR )

#define mystmt_err(hstmt,r,rc)  \
        if ( rc == SQL_ERROR || rc == SQL_SUCCESS_WITH_INFO ) \
            myerror( rc, 3, hstmt, __FILE__, __LINE__ ); \
        my_assert( r )

#if defined(USE_C99_FUNC_MACRO)
#define printMessageHeader()\
    {\
        g_nCursor = sprintf( g_szHeader, "[%s][%s][%d]\n", __FILE__, __func__, __LINE__ );\
        fprintf( stdout, g_szHeader );\
    }
#elif defined(USE_GNU_FUNC_MACRO)
#define printMessageHeader()\
    {\
        g_nCursor = sprintf( g_szHeader, "[%s][%s][%d]\n", __FILE__, __FUNCTION__, __LINE__ );\
        fprintf( stdout, g_szHeader );\
    }
#else
#define printMessageHeader()\
    {\
        g_nCursor = sprintf( g_szHeader, "[%s][%d]\n", __FILE__, __LINE__ );\
        fprintf( stdout, g_szHeader );\
    }
#endif

#define printMessageFooter( A )\
    {\
        int nDot = g_nCursor;\
        if ( g_nCursor < 1 )\
            fprintf( stdout, "\n" );\
        for ( ; nDot < 71; nDot++ )\
        {\
            fprintf( stdout, "." );\
        }\
        if ( A )\
            fprintf( stdout, "[  OK  ]\n" );\
        else\
            fprintf( stdout, "[FAILED]\n" );\
    }


/**
ERROR HANDLER
*/
void myerror( SQLRETURN rc, SQLSMALLINT htype, SQLHANDLE handle, const char *szFile, int nLine )
{
    RETCODE lrc;

    if ( !SQL_SUCCEEDED( rc ) )
    {
        SQLCHAR      szSqlState[6],szErrorMsg[SQL_MAX_MESSAGE_LENGTH];
        SQLINTEGER   pfNativeError;
        SQLSMALLINT  pcbErrorMsg;

        lrc = SQLGetDiagRec(htype, handle,1,    
                            (SQLCHAR *)&szSqlState,
                            (SQLINTEGER *)&pfNativeError,
                            (SQLCHAR *)&szErrorMsg,
                            SQL_MAX_MESSAGE_LENGTH-1,
                            (SQLSMALLINT *)&pcbErrorMsg);
        if ( SQL_SUCCEEDED( lrc ) )
            fprintf( stdout, "\n\n[%s][%d][%s]%s\n\n", szFile, nLine, szSqlState, szErrorMsg );
        else
            fprintf( stdout, "\n\n[%s][%d] SQLGetDiagRec returned :%d, but rc = %d\n\n", szFile, nLine, lrc, rc );

        g_nCursor = 0;
    }
}

/**
  Print resultset dashes
*/
#if DEBUG_LEVEL > 1
static void my_print_dashes(SQLHSTMT hstmt, SQLSMALLINT nCol)
{
    SQLRETURN  rc;
    SQLLEN     disp_size, nullable;
    SQLCHAR    ColName[MAX_NAME_LEN+1];
    SQLUSMALLINT field_count, i, j;
    SQLSMALLINT  col_len;

    field_count= (SQLUSMALLINT)nCol;
    printMessage( "\t" );
    printMessage( "+" );

    for (i=1; i<= field_count; i++)
    {
        nullable = 0;
        rc = SQLColAttribute(hstmt,i,SQL_DESC_BASE_COLUMN_NAME,&ColName,
                             MAX_NAME_LEN,&col_len,NULL);
        mystmt(hstmt,rc);
        rc = SQLColAttribute(hstmt,i,SQL_DESC_DISPLAY_SIZE,NULL,0,
                             NULL,&disp_size);
        mystmt(hstmt,rc);
        rc = SQLColAttribute(hstmt,i,SQL_DESC_NULLABLE,NULL,0,
                             NULL,&nullable);
        mystmt(hstmt,rc);

        if (disp_size < col_len)
            disp_size = col_len;
        if (disp_size < 4 && nullable)
            disp_size = 4;
        for (j=1; j < disp_size+3; j++)
            printMessage( "-" );
        printMessage( "+" );
    }
    printMessage( "\n" );
}
#else
# define my_print_dashes(a, b)
#endif


#if DEBUG_LEVEL > 1
static void my_print_data(SQLHSTMT hstmt, SQLUSMALLINT index,
                          SQLCHAR *data, SQLINTEGER length)
{
    SQLRETURN  rc;
    SQLLEN     disp_size, nullable;
    SQLCHAR    ColName[MAX_NAME_LEN+1];
    SQLSMALLINT col_len;

    nullable = 0;
    rc = SQLColAttribute(hstmt,index,SQL_DESC_BASE_COLUMN_NAME,&ColName,
                         MAX_NAME_LEN,&col_len,NULL);
    mystmt(hstmt,rc);
    rc = SQLColAttribute(hstmt,index,SQL_DESC_DISPLAY_SIZE,NULL,0,
                         NULL,&disp_size);
    mystmt(hstmt,rc);
    rc = SQLColAttribute(hstmt,index,SQL_DESC_NULLABLE,NULL,0,
                         NULL,&nullable);
    mystmt(hstmt,rc);

    if (disp_size < length)
        disp_size = length;
    if (disp_size < col_len)
        disp_size = col_len;
    if (disp_size < 4 && nullable)
        disp_size = 4;
    if (length == SQL_NULL_DATA)
        printMessage( "%-*s  |",(int)disp_size, "NULL");
    else
        printMessage( "%-*s  |",(int)disp_size,data);
}
#else
# define my_print_data(a, b, c, d)
#endif


/**
RESULT SET
*/
int my_print_non_format_result(SQLHSTMT hstmt)
{
    SQLRETURN   rc;
    SQLUINTEGER nRowCount=0;
    SQLULEN     pcColDef;
    SQLCHAR     szColName[MAX_NAME_LEN];
    SQLCHAR     szData[MAX_COLUMNS][MAX_ROW_DATA_LEN]={{0}};
    SQLSMALLINT nIndex,ncol,pfSqlType, pcbScale, pfNullable;

    rc = SQLNumResultCols(hstmt,&ncol);
    mystmt(hstmt,rc);

    printMessage("\n");

    for (nIndex = 1; nIndex <= ncol; nIndex++)
    {
        rc = SQLDescribeCol(hstmt,nIndex,szColName, MAX_NAME_LEN+1, NULL,
                            &pfSqlType,&pcColDef,&pcbScale,&pfNullable);
        mystmt(hstmt,rc);

        printMessage("%s\t",szColName);

        rc = SQLBindCol(hstmt,nIndex, SQL_C_CHAR, szData[nIndex-1],
                        MAX_ROW_DATA_LEN+1,NULL);
        mystmt(hstmt,rc);
    }

    printMessage("\n------------------------------------------------------\n");

    rc = SQLFetch(hstmt);
    while (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    {
        nRowCount++;
        for (nIndex=0; nIndex< ncol; nIndex++)
            printMessage("%s\t",szData[nIndex]);

        printMessage("\n");
        rc = SQLFetch(hstmt);
    }

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    printMessage("Total rows fetched: %d\n",(int)nRowCount);
    return(nRowCount);
}

/**
  RESULT SET
*/
SQLUINTEGER myresult(SQLHSTMT hstmt)
{
    SQLRETURN   rc;
    SQLUINTEGER nRowCount;
    SQLCHAR     ColName[MAX_NAME_LEN+1];
    SQLCHAR     Data[MAX_ROW_DATA_LEN+1];
    SQLLEN      pcbLength, size;
    SQLUSMALLINT nIndex;
    SQLSMALLINT  ncol;

    rc = SQLNumResultCols(hstmt,&ncol);
    mystmt(hstmt,rc);

    if (ncol > 10)
        return my_print_non_format_result(hstmt);

    printMessage( "\n" );
    my_print_dashes(hstmt, ncol);
    printMessage( "\t" );
    printMessage( "|" );

    for (nIndex = 1; nIndex <= ncol; nIndex++)
    {
        rc = SQLColAttribute(hstmt,nIndex,SQL_DESC_BASE_COLUMN_NAME,ColName,MAX_NAME_LEN,
                             NULL,NULL);
        mystmt(hstmt,rc);
        my_print_data(hstmt,nIndex,ColName,0);
    }
    printMessage( "\n" );
    my_print_dashes(hstmt, ncol);

    nRowCount= 0;

    rc = SQLFetch(hstmt);
    while (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    {
        nRowCount++;
        printMessage( "\t" );
        printMessage( "|" );

        for (nIndex=1; nIndex<= ncol; nIndex++)
        {
            rc = SQLColAttribute(hstmt,nIndex,SQL_DESC_DISPLAY_SIZE,NULL,0,
                                 NULL,&size);
            mystmt(hstmt,rc);
            rc = SQLGetData(hstmt, nIndex, SQL_C_CHAR, Data,
                            MAX_ROW_DATA_LEN,&pcbLength);
            mystmt(hstmt,rc);
            my_print_data(hstmt, nIndex, Data, pcbLength);
        }
        printMessage( "\t" );
        printMessage( "\n" );
        rc = SQLFetch(hstmt);
    }
    my_print_dashes(hstmt,ncol);
    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    printMessage("Total rows fetched: %d\n",(int)nRowCount);
    return nRowCount;
}

/**
  ROWCOUNT
*/
SQLUINTEGER myrowcount(SQLHSTMT hstmt)
{
    SQLRETURN   rc;
    SQLUINTEGER nRowCount= 0;

    rc = SQLFetch(hstmt);
    while (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    {
        nRowCount++;
        rc = SQLFetch(hstmt);
    }
    SQLFreeStmt(hstmt,SQL_UNBIND);
    printMessage("Total rows fetched: %d\n",(int)nRowCount);
    return(nRowCount);
}
/**
  SQLExecDirect
*/
SQLRETURN tmysql_exec(SQLHSTMT hstmt, char *sql_stmt)
{
    return(SQLExecDirect(hstmt,(SQLCHAR *)sql_stmt,SQL_NTS));
}
/**
  SQLPrepare
*/
SQLRETURN tmysql_prepare(SQLHSTMT hstmt, char *sql_stmt)
{
    return(SQLPrepare(hstmt, (SQLCHAR *)sql_stmt, SQL_NTS));
}
/**
  return integer data by fetching it
*/
SQLINTEGER my_fetch_int(SQLHSTMT hstmt, SQLUSMALLINT irow)
{
    SQLINTEGER nData;
    SQLLEN len;

    SQLGetData(hstmt, irow, SQL_INTEGER, &nData, 0, &len);
    printMessage(" my_fetch_int: %ld (%ld)\n", (long int)nData, len);
    return (len != SQL_NULL_DATA) ? nData : 0;
}
/**
  return string data, by fetching it
*/
const char *my_fetch_str(SQLHSTMT hstmt, SQLCHAR *szData,SQLUSMALLINT irow)
{
    SQLLEN nLen;

    SQLGetData(hstmt,irow,SQL_CHAR,szData,MAX_ROW_DATA_LEN+1,&nLen);
    printMessage( " my_fetch_str: %s(%ld)\n",szData,nLen);
    return((const char *)szData);
}

/*
  Check if server running is MySQL
*/
int server_is_mysql(SQLHDBC hdbc)
{
    char driver_name[MYSQL_NAME_LEN];

    SQLGetInfo(hdbc,SQL_DRIVER_NAME,driver_name,MYSQL_NAME_LEN,NULL);

    if (strncmp(driver_name,"myodbc",6) >= 0 || strncmp(driver_name,"libmyodbc",9) >= 0)
        return TRUE;

    return FALSE;
}

/*
  Check if driver supports SQLSetPos
*/
int driver_supports_setpos(SQLHDBC hdbc)
{
    SQLRETURN    rc;
    SQLUSMALLINT status= TRUE;

    rc = SQLGetFunctions(hdbc, SQL_API_SQLSETPOS, &status);
    mycon(hdbc,rc);

    if (!status)
        return FALSE;
    return TRUE;
}

/*
  Check for minimal MySQL version
*/
int mysql_min_version(SQLHDBC hdbc, char *min_version, unsigned int length)
{
    SQLCHAR server_version[MYSQL_NAME_LEN];
    SQLRETURN rc;

    if (!server_is_mysql(hdbc))
        return TRUE;

    rc = SQLGetInfo(hdbc,SQL_DBMS_VER,server_version,MYSQL_NAME_LEN,NULL);
    mycon(hdbc, rc);

    if (strncmp((char *)server_version, min_version, length) >= 0)
        return TRUE;

    return FALSE;
}

/*
  Check if server supports transactions
*/
int server_supports_trans(SQLHDBC hdbc)
{
    SQLSMALLINT trans;
    SQLRETURN   rc;

    rc = SQLGetInfo(hdbc,SQL_TXN_CAPABLE,&trans,0,NULL);
    mycon(hdbc,rc);

    if (trans != SQL_TC_NONE)
        return TRUE;
    return FALSE;
}


/**
  DRV CONNECTION
*/
int mydrvconnect(SQLHENV *henv, SQLHDBC *hdbc, SQLHSTMT *hstmt, SQLCHAR *connIn)
{
  SQLCHAR   connOut[MAX_NAME_LEN];
  SQLSMALLINT len;

  ok_env(*henv, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, henv));

  ok_env(*henv, SQLSetEnvAttr(*henv, SQL_ATTR_ODBC_VERSION,
                              (SQLPOINTER)SQL_OV_ODBC3, 0));

  ok_env(*henv, SQLAllocHandle(SQL_HANDLE_DBC, *henv,  hdbc));

  ok_con(*hdbc, SQLDriverConnect(*hdbc, NULL, connIn, MAX_NAME_LEN, connOut,
                                 MAX_NAME_LEN, &len, SQL_DRIVER_NOPROMPT));

  ok_con(*hdbc, SQLSetConnectAttr(*hdbc, SQL_ATTR_AUTOCOMMIT,
                                  (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0));

  ok_con(*hdbc, SQLAllocHandle(SQL_HANDLE_STMT, *hdbc, hstmt));

  return OK;
}


int alloc_basic_handles(SQLHENV *henv, SQLHDBC *hdbc, SQLHSTMT *hstmt)
{
  SQLCHAR   connIn[MAX_NAME_LEN], connOut[MAX_NAME_LEN];
  SQLSMALLINT len;

  ok_env(*henv, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, henv));

  ok_env(*henv, SQLSetEnvAttr(*henv, SQL_ATTR_ODBC_VERSION,
                              (SQLPOINTER)SQL_OV_ODBC3, 0));

  ok_env(*henv, SQLAllocHandle(SQL_HANDLE_DBC, *henv, hdbc));

  sprintf((char *)connIn, "DSN=%s;USER=%s;PASSWORD=%s;DATABASE=test;OPTION=%d",
          (char *)mydsn, (char *)myuid, (char *)mypwd, myoption);
  if (mysock && mysock[0])
  {
    strcat((char *)connIn, ";SOCKET=");
    strcat((char *)connIn, (char *)mysock);
  }

  ok_con(*hdbc, SQLDriverConnect(*hdbc, NULL, connIn, MAX_NAME_LEN, connOut,
                                 MAX_NAME_LEN, &len, SQL_DRIVER_NOPROMPT));

  ok_con(*hdbc, SQLSetConnectAttr(*hdbc, SQL_ATTR_AUTOCOMMIT,
                                  (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0));

  ok_con(*hdbc, SQLAllocHandle(SQL_HANDLE_STMT, *hdbc, hstmt));

  return OK;
}


int free_basic_handles(SQLHENV *henv,SQLHDBC *hdbc, SQLHSTMT *hstmt)
{
  /* We don't care if this succeeds, the connection may have gone away. */
  (void)SQLEndTran(SQL_HANDLE_DBC, *hdbc, SQL_COMMIT);

  ok_stmt(*hstmt, SQLFreeStmt(*hstmt, SQL_DROP));

  ok_con(*hdbc, SQLDisconnect(*hdbc));

  ok_con(*hdbc, SQLFreeConnect(*hdbc));

  ok_env(*henv, SQLFreeEnv(*henv));

  return OK;
}

