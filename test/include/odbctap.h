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

/** @todo Remove dependency on this crufty old header. */
#define DEBUG_LEVEL 2
#include "mytest3.h"
#include <stdarg.h>
#undef printMessage
void printMessage(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  fprintf(stdout, "# ");
  vfprintf(stdout, fmt, ap);
  fprintf(stdout, "\n");
  va_end(ap);
}

#ifndef OK
# define OK 0
#endif
#ifndef FAIL
# define FAIL 1
#endif
#ifndef SKIP
# define SKIP -1
#endif

typedef int (*test_func)(SQLHDBC, SQLHSTMT, SQLHENV);

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

#define BEGIN_TESTS my_test tests[]= {
#define ADD_TEST(name) { #name, name, OK   },
#define ADD_TODO(name) { #name, name, FAIL },
#define END_TESTS }; \
int main(int argc, char **argv) \
{ \
  SQLHENV  henv; \
  SQLHDBC  hdbc; \
  SQLHSTMT hstmt; \
  int      i, num_tests; \
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
  num_tests= sizeof(tests) / sizeof(tests[0]); \
  printf("1..%d\n", num_tests); \
\
  alloc_basic_handles(&henv,&hdbc,&hstmt); \
\
  for (i= 0; i < num_tests; i++ ) \
  { \
    int rc= tests[i].func(hdbc, hstmt, henv); \
    printf("%s %d %s %s\n", rc == OK ? "ok" : "not ok", i + 1, \
           tests[i].expect == FAIL ? "# TODO" : "-", \
           tests[i].name); \
  } \
\
  free_basic_handles(&henv,&hdbc,&hstmt); \
\
  exit(0); \
}

/**
  Execute an SQL statement and bail out if the execution does not return
  SQL_SUCCESS or SQL_SUCCESS_WITH_INFO.

  @param hstmt Handle for statement object
  @param query The query to execute
*/
#define ok_sql(hstmt, query) \
do { \
  SQLRETURN rc= SQLExecDirect(hstmt, (SQLCHAR *)query, SQL_NTS); \
  print_diag(rc, SQL_HANDLE_STMT, hstmt, \
             "SQLExecDirect(hstmt, \"" query "\", SQL_NTS)",\
             __FILE__, __LINE__); \
  if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) \
    return FAIL; \
} while (0)

/**
  Verify that the results of an ODBC function call on a statement handle was
  SQL_SUCCESS or SQL_SUCCESS_WITH_INFO.

  @param hstmt Handle for statement object
  @param call  The function call
*/
#define ok_stmt(hstmt, call) \
do { \
  SQLRETURN rc= call; \
  print_diag(rc, SQL_HANDLE_STMT, hstmt, #call, __FILE__, __LINE__); \
  if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) \
    return FAIL; \
} while (0)


/**
  Verify that the result of an ODBC function call matches an expected
  result, such as SQL_NO_DATA_FOUND.

  @param hstmt  Handle for statement object
  @param call   The function call
  @param expect The expected result
*/
#define expect_stmt(hstmt, call, expect) \
do { \
  SQLRETURN rc= call; \
  if (rc != expect) \
  { \
    print_diag(rc, SQL_HANDLE_STMT, hstmt, #call, __FILE__, __LINE__); \
    printf("# Expected %d, but got %d in %s on line %d\n", expect, rc, \
           __FILE__, __LINE__); \
    return FAIL; \
  } \
} while (0)


/**
  Verify that the results of an ODBC function call on a connection handle
  was SQL_SUCCESS or SQL_SUCCESS_WITH_INFO.

  @param hstmt Handle for statement object
  @param call  The function call
*/
#define ok_con(hdbc, call) \
do { \
  SQLRETURN rc= call; \
  print_diag(rc, SQL_HANDLE_DBC, hdbc, #call, __FILE__, __LINE__); \
  if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) \
    return FAIL; \
} while (0)


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


/**
*/
void print_diag(SQLRETURN rc, SQLSMALLINT htype, SQLHANDLE handle,
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
      printf("# Did not get expected diagnostics from SQLGetDiagRec()"
             " in file %s on line %d\n", file, line);
  }
}


void alloc_basic_handles(SQLHENV *henv,SQLHDBC *hdbc, SQLHSTMT *hstmt)
{
  SQLRETURN rc;
  SQLCHAR   connIn[MAX_NAME_LEN], connOut[MAX_NAME_LEN];

  rc= SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,henv);
  myenv(*henv,rc);

  rc= SQLSetEnvAttr(*henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0);
  myenv(*henv,rc);

  rc= SQLAllocHandle(SQL_HANDLE_DBC,*henv, hdbc);
  myenv(*henv,rc);

  sprintf((char *)connIn, "DSN=%s;USER=%s;PASSWORD=%s;DATABASE=test;OPTION=%d",
          (char *)mydsn, (char *)myuid, (char *)mypwd, myoption);
  if (mysock && mysock[0])
  {
    strcat((char *)connIn, ";SOCKET=");
    strcat((char *)connIn, (char *)mysock);
  }

  rc= SQLDriverConnect(*hdbc, NULL, connIn, MAX_NAME_LEN, connOut, MAX_NAME_LEN,
                       NULL, SQL_DRIVER_NOPROMPT);
  mycon(*hdbc,rc);

  rc= SQLSetConnectAttr(*hdbc,SQL_ATTR_AUTOCOMMIT,
                         (SQLPOINTER)SQL_AUTOCOMMIT_ON,0);
  mycon(*hdbc,rc);

  rc= SQLAllocHandle(SQL_HANDLE_STMT,*hdbc,hstmt);
  mycon(*hdbc,rc);

  SQLSetStmtAttr(*hstmt,SQL_ATTR_CURSOR_TYPE,(SQLPOINTER)SQL_CURSOR_STATIC,0);
  SQLSetStmtOption(*hstmt,SQL_SIMULATE_CURSOR,SQL_SC_NON_UNIQUE);
  SQLSetStmtOption(*hstmt, SQL_CURSOR_TYPE, SQL_CURSOR_KEYSET_DRIVEN);
}


void free_basic_handles(SQLHENV *henv,SQLHDBC *hdbc, SQLHSTMT *hstmt)
{
  SQLRETURN rc;

  rc= SQLEndTran(SQL_HANDLE_DBC, *hdbc, SQL_COMMIT);
  mycon(*hdbc, rc);

  rc= SQLFreeStmt(*hstmt, SQL_DROP);
  mystmt(*hstmt,rc);

  rc= SQLDisconnect(*hdbc);
  mycon(*hdbc,rc);

  rc= SQLFreeConnect(*hdbc);
  mycon(*hdbc,rc);

  rc= SQLFreeEnv(*henv);
  myenv(*henv,rc);
}
