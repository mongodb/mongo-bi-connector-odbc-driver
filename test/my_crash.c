/*
  Copyright (c) 2013, 2014, Oracle and/or its affiliates. All rights reserved.

  The MySQL Connector/ODBC is licensed under the terms of the GPLv2
  <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
  MySQL Connectors. There are special exceptions to the terms and
  conditions of the GPLv2 as it is applied to this software, see the
  FLOSS License Exception
  <http://www.mysql.com/about/legal/licensing/foss-exception.html>.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.
  
  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "odbctap.h"
#include "../VersionInfo.h"

/*
  Bug #69950 Visual Studio 2010 crashes when reading rows from any 
  table in Server Explorer
*/
DECLARE_TEST(t_bug69950)
{
  /* Make sure such table does not exist! */
  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug69950");

  /* Create an EMPTY fake result set */
  ok_stmt(hstmt, SQLTables(hstmt, mydb, SQL_NTS, NULL, 0, 
                           "t_bug69950", SQL_NTS, "TABLE", SQL_NTS));

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA_FOUND);
  expect_stmt(hstmt, SQLMoreResults(hstmt), SQL_NO_DATA_FOUND);

  /* CRASH! */
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  return OK;
}


/*
  Bug #17640929/70642 "Bad memory access when get out params"
  If IN param went last in the list of SP parameters containing any (IN)OUT parameter,
  the driver would access not allocated memory.
  In fact the testcase doesn't reliably hit the problem
*/
DECLARE_TEST(t_bug70642)
{
  SQLSMALLINT i;
  SQLINTEGER  par[]= {10, 20, 1, 0, 1};
  SQLSMALLINT type[]= { SQL_PARAM_OUTPUT, SQL_PARAM_INPUT, SQL_PARAM_INPUT, SQL_PARAM_INPUT,
                        SQL_PARAM_INPUT};

  ok_sql(hstmt, "DROP PROCEDURE IF EXISTS t_bug70642");
  ok_sql(hstmt, "CREATE PROCEDURE t_bug70642("
                "  OUT p_out INT, "
                "  IN p_in INT, IN d1 INT, IN d2 INT, IN d3 INT) "
                "BEGIN "
                "  SET p_in = p_in*10, p_out = p_in*10; "
                "END");

  for (i=0; i < sizeof(par)/sizeof(SQLINTEGER); ++i)
  {
    ok_stmt(hstmt, SQLBindParameter(hstmt, i+1, type[i], SQL_C_LONG, SQL_INTEGER, 0,
      0, &par[i], 0, NULL));
  }

  for (i=0; i < 10; ++i)
  {
    ok_sql(hstmt, "CALL t_bug70642(?, ?, ?, ?, ?)");

    ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  }

  ok_sql(hstmt, "DROP PROCEDURE IF EXISTS t_bug70642");

  return OK;
}


/**
  Bug#31067: SEGMENTATION FAULT IN SQLCOLUMNS IF COLUMN/TABLE NAME IS INVALID
  If column name length is larger then 129 and table name larger then 256 result in 
  segementation fault.
*/
DECLARE_TEST(t_bug17358838)
{
  SQLCHAR    colName[512];
  SQLCHAR message[SQL_MAX_MESSAGE_LENGTH + 1];
  SQLCHAR sqlstate[SQL_SQLSTATE_SIZE + 1];
  SQLINTEGER error;
  SQLSMALLINT len;

  memset(colName, '\0', 512);
  memset(colName, 'a', 500);

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug17358838");
  ok_sql(hstmt, "CREATE TABLE t_bug17358838 (a INT)");

  expect_stmt(hstmt, SQLColumns(hstmt, mydb, SQL_NTS, NULL, SQL_NTS,
    (SQLCHAR *)"t_bug17358838", SQL_NTS, colName, SQL_NTS), SQL_ERROR);

  ok_stmt(hstmt, SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, 1, sqlstate, &error,
                               message, sizeof(message), &len));

  is_str(sqlstate, "HY090", 5);
  is(strstr((char *)message, "One or more parameters exceed the maximum allowed name length"));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE t_bug17358838");

  return OK;
}


/**
  Bug 17587913 Connect crash if the catalog name given to SQLSetConnectAttr 
  IS INVALID
*/
DECLARE_TEST(t_bug17587913)
{
  SQLHDBC hdbc1;
  SQLCHAR str[1024]={0};
  SQLLEN len= 0;
  SQLCHAR *DatabaseName = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  /* 
    We are not going to use alloc_basic_handles() for a special purpose:
    SQLSetConnectAttr() is to be called before the connection is made
  */  
  ok_env(henv, SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1));

  /* getting error here */
  get_connection(&hdbc1, NULL, NULL, NULL, DatabaseName, NULL);

  ok_con(hdbc1, SQLSetConnectAttr(hdbc1, SQL_ATTR_CURRENT_CATALOG,
                                  DatabaseName, strlen(DatabaseName)));

  /* Expecting error here */
  SQLGetConnectAttr(hdbc1, SQL_ATTR_CURRENT_CATALOG, str, 100, &len);

  /* The driver crashes here on getting connected */
  ok_con(hdbc1, SQLConnect(hdbc1, mydsn, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS));  
  ok_con(hdbc1, SQLGetConnectAttr(hdbc1, SQL_ATTR_CURRENT_CATALOG, str, 100, &len));

  ok_con(hdbc1, SQLDisconnect(hdbc1));
  ok_con(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


/**
  Bug #17857204 SQLFETCH() CRASHING WHEN EXECUTE USING 
  UNIXODBC 2.3.2 VERSION
*/
DECLARE_TEST(t_bug17857204)
{
  int i;
  char TmpBuff[256] = {0};
  SQLSMALLINT col_count= 0;
  SQLUINTEGER uintval= 0;
  SQLLEN len= 0;

  ok_sql(hstmt, "DROP TABLE IF EXISTS bug17857204");
  ok_sql(hstmt, "CREATE TABLE bug17857204 (id int unsigned,c char(10))");
  ok_sql(hstmt, "CREATE INDEX i ON bug17857204 (id ,c)");

  for (i= 0; i < 10; i++)
  {
    sprintf(TmpBuff,"INSERT INTO bug17857204 VALUES (%d,'%d')", i, i);
    ok_stmt(hstmt, SQLExecDirect(hstmt, (SQLCHAR*)TmpBuff, SQL_NTS));
  }
  ok_stmt(hstmt, SQLPrepare(hstmt, (SQLCHAR*)"EXPLAIN DELETE a1,a2 "\
                                             "FROM bug17857204 AS a1 "\
                                             "INNER JOIN bug17857204 AS a2 "\
                                             "WHERE a1.id=a2.id and a2.id>=?",
                                             SQL_NTS));
  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_ULONG, 
          SQL_NUMERIC, 4, 0, &uintval, 0,  &len));

  ok_stmt(hstmt, SQLExecute(hstmt));
  ok_stmt(hstmt, SQLNumResultCols(hstmt,&col_count));
  
  while (1)
  {
    SQLRETURN rc= 0;
    memset(TmpBuff,0,256);
    rc= SQLFetch(hstmt);
    if (rc != SQL_SUCCESS)
    {
      break;
    }
    
    for (i= 1; i <= col_count; i++)
    {
      ok_stmt(hstmt, SQLGetData(hstmt, i, SQL_C_CHAR, TmpBuff, 
                                sizeof(TmpBuff), &len));
    }
  }

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE bug17857204");

  return OK;
}


/**
  Bug #17854697 SEGMENTATION FAULT IN SQLSPECIALCOLUMNS IF TABLE NAME IS 
  INVALID
*/
DECLARE_TEST(t_bug17854697)
{
  SQLCHAR *any_name = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"\
                      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  SQLCHAR buf[1024]= {0};

  int len= strlen(any_name);

  /* lets check all catalog functions */
  expect_stmt(hstmt, SQLColumnPrivileges(hstmt, any_name, SQL_NTS, NULL, 0,
                                         any_name, SQL_NTS, any_name,
                                         SQL_NTS), SQL_ERROR);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  expect_stmt(hstmt, SQLColumns(hstmt, any_name, SQL_NTS, NULL, 0,
                                any_name, SQL_NTS, any_name,
                                SQL_NTS), SQL_ERROR);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  expect_stmt(hstmt, SQLForeignKeys(hstmt, any_name, SQL_NTS, NULL, 0,
                                any_name, SQL_NTS, any_name, SQL_NTS,
                                any_name, SQL_NTS, any_name, SQL_NTS), 
                     SQL_ERROR);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  expect_stmt(hstmt, SQLPrimaryKeys(hstmt, any_name, SQL_NTS, NULL, 0,
                                    any_name, SQL_NTS), SQL_ERROR);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  expect_stmt(hstmt, SQLProcedureColumns(hstmt, any_name, SQL_NTS, NULL, 0,
                                any_name, SQL_NTS, any_name, SQL_NTS), 
                     SQL_ERROR);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  expect_stmt(hstmt, SQLProcedures(hstmt, any_name, SQL_NTS, NULL, 0,
                                any_name, SQL_NTS), SQL_ERROR);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  expect_stmt(hstmt, SQLSpecialColumns(hstmt, SQL_BEST_ROWID, any_name, SQL_NTS, 
                                NULL, 0, any_name, SQL_NTS, SQL_SCOPE_SESSION, 
                                SQL_NULLABLE), SQL_ERROR);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  expect_stmt(hstmt, SQLStatistics(hstmt, any_name, SQL_NTS, NULL, 0,
                                any_name, SQL_NTS, SQL_INDEX_ALL, SQL_QUICK),
                     SQL_ERROR);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  expect_stmt(hstmt, SQLTablePrivileges(hstmt, any_name, SQL_NTS, NULL, 0,
                                any_name, SQL_NTS), SQL_ERROR);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  expect_stmt(hstmt, SQLTables(hstmt, any_name, SQL_NTS, NULL, 0,
                                any_name, SQL_NTS, any_name, SQL_NTS),
                     SQL_ERROR);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  return OK;
}


/* Bug #17999659 CRASH IN ODBC DRIVER WITH CHARSET=WRONGCHARSET  */
DECLARE_TEST(t_bug17999659)
{
  SQLHDBC hdbc1;

  ok_env(henv, SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1));

  /* getting error here */
  expect_dbc(hdbc1, get_connection(&hdbc1, NULL, NULL, NULL, NULL, 
             "CHARSET=wrongcharset"), SQL_ERROR);

  ok_con(hdbc1, SQLFreeConnect(hdbc1));
  return OK;
}

/*
  Bug #17966018 DRIVER AND MYODBC-INSTALLER CRASH WITH LONG OPTION 
  NAMES (>100) AND VALUES (>255)
*/
DECLARE_TEST(t_bug17966018)
{
  char opt_buff[2048];
  int result_connect;

  DECLARE_BASIC_HANDLES(henv1, hdbc1, hstmt1);

  /* This makes a really long SETUP=000000...0;OPTION_0000000000...0=1 */
  sprintf(opt_buff, "OPTION_%0*d=1", 1000, 0);

  result_connect= alloc_basic_handles_with_opt(&henv1, &hdbc1, &hstmt1, NULL, 
                                               NULL, NULL, NULL, opt_buff);
  is_num(result_connect, FAIL);

  free_basic_handles(&henv1, &hdbc1, &hstmt1);
  return OK;
}


/**
  Bug #17841121 Valgrind reported invalid read/write error in my_SQLFreeDesc
*/
DECLARE_TEST(t_bug17841121)
{
  SQLHSTMT hstmt1;
  SQLHANDLE expard;
  SQLINTEGER imp_result= 0, exp_result= 0;
  int i= 0, *num, attempts_left= 20;

  do
  {
    ok_con(hdbc, SQLAllocHandle(SQL_HANDLE_DESC, hdbc, &expard));

    ok_con(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt1));

    ok_stmt(hstmt1, SQLSetStmtAttr(hstmt1, SQL_ATTR_APP_ROW_DESC, expard, 0));

    /* this affects the expard */
    ok_stmt(hstmt1, SQLBindCol(hstmt1, 1, SQL_C_LONG, &exp_result, 0, NULL));

    /* set it to null, getting rid of the expard */
    ok_stmt(hstmt1, SQLSetStmtAttr(hstmt1, SQL_ATTR_APP_ROW_DESC, 
                                  SQL_NULL_HANDLE, 0));

    ok_stmt(hstmt1, SQLExecDirect(hstmt1, (SQLCHAR*)"select 1", SQL_NTS));


    ok_stmt(hstmt1, SQLFetch(hstmt1));

    ok_stmt(hstmt1, SQLFreeHandle(SQL_HANDLE_STMT, hstmt1));

    /* after STMT was freed we want to overwrite the contents of freed memory */
    num= (int*)gc_alloc(2500*sizeof(int));

    for(i= 0; i< 2500; i++)
    {
      /* Lets assign each element its number in the array */
      num[i]= i;
    }
    
    /* this might crash, but no guarantee */
    ok_desc(expard, SQLFreeHandle(SQL_HANDLE_DESC, expard));

    for(i= 0; i< 2500; i++)
    {
      /* Check if any array elements changed */
      is_num(num[i], i);
    }
  }while (--attempts_left);

  return OK;
}


/**
  Segmentation in SQLBulkOperations for SQL_UPDATE_BY_BOOKMARK operation
  when 0 records gets fetched and provided.
*/
DECLARE_TEST(t_bookmark_update_zero_rec)
{
  SQLLEN len= 0;
  SQLUSMALLINT rowStatus[4];
  SQLUINTEGER numRowsFetched;
  SQLINTEGER nData[4], i;
  SQLCHAR szData[4][16];
  SQLCHAR bData[4][10];
  SQLLEN nRowCount;

  ok_sql(hstmt, "drop table if exists t_bookmark");
  ok_sql(hstmt, "CREATE TABLE t_bookmark ("\
                "tt_int INT PRIMARY KEY auto_increment,"\
                "tt_varchar VARCHAR(128) NOT NULL)");
  ok_sql(hstmt, "INSERT INTO t_bookmark VALUES "\
                "(1, 'string 1'),"\
                "(2, 'string 2'),"\
                "(3, 'string 3'),"\
                "(4, 'string 4'),"\
                "(5, 'string 5')");

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_USE_BOOKMARKS,
                                (SQLPOINTER) SQL_UB_VARIABLE, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_STATUS_PTR,
                                (SQLPOINTER)rowStatus, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_ROWS_FETCHED_PTR,
                                &numRowsFetched, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE,
                                (SQLPOINTER)SQL_CURSOR_STATIC, 0));
  ok_stmt(hstmt, SQLSetStmtOption(hstmt, SQL_ROWSET_SIZE, 4));

  ok_sql(hstmt, "select * from t_bookmark where tt_int = 50 order by 1");
  ok_stmt(hstmt, SQLBindCol(hstmt, 0, SQL_C_VARBOOKMARK, bData,
                         sizeof(bData[0]), NULL));
  ok_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_LONG, nData, 0, NULL));
  ok_stmt(hstmt, SQLBindCol(hstmt, 2, SQL_C_CHAR, szData, sizeof(szData[0]),
                            NULL));

  SQLFetchScroll(hstmt, SQL_FETCH_BOOKMARK, 0);

  ok_stmt(hstmt, SQLBulkOperations(hstmt, SQL_UPDATE_BY_BOOKMARK));
  ok_stmt(hstmt, SQLRowCount(hstmt, &nRowCount));
  is_num(nRowCount, 0);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  ok_sql(hstmt, "drop table if exists t_bookmark");

  return OK;
}


/* 
  Bug#17085344: SEGMENTATION FAULT IN MYODBC_CASECMP WHEN QUERY IS EMPTY
*/
DECLARE_TEST(t_bug17085344)
{
  expect_stmt(hstmt, SQLExecDirect(hstmt, "", SQL_NTS), SQL_ERROR);
  expect_stmt(hstmt, SQLExecDirect(hstmt, "  ", SQL_NTS), SQL_ERROR);

  return OK;
}


/* 
  Bug#18165197: SQLNUMRESULTCOLS() WITH NULL PARAMETER RESULTS IN
  SEGMENTATION FAULT
*/
DECLARE_TEST(t_bug18165197)
{
  ok_sql(hstmt, "SELECT 1");
  expect_stmt(hstmt, SQLNumResultCols(hstmt, NULL), SQL_ERROR);

  return OK;
}


/*
  Bug #18325878 SEGMENTATION FAULT IN SQLPARAMOPTIONS() IN SOLARIS PLATFORM
*/
DECLARE_TEST(t_bug18325878)
{
#ifdef RCNT
#undef RCNT
#endif
#define RCNT 8
  SQLRETURN rc;
  char TmpBuff[1024] = {0};
  SQLUINTEGER j = 0;
  SQLUINTEGER k = 0;
  SQLUINTEGER uintval[RCNT] = {0};
#ifdef USE_SQLPARAMOPTIONS_SQLULEN_PTR
  SQLULEN lval[RCNT] = {0};
# define PARAMTYPE SQLULEN
#else
  SQLUINTEGER lval[RCNT] = {0};
# define PARAMTYPE SQLUINTEGER
#endif

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug18325878");
  ok_sql(hstmt, "CREATE TABLE t_bug18325878 (id int)");

  ok_stmt(hstmt, SQLParamOptions(hstmt, RCNT, NULL));

  ok_stmt(hstmt, SQLPrepare(hstmt, (SQLCHAR*)"INSERT INTO t_bug18325878 VALUES (?+1)", SQL_NTS));

  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_ULONG,
                                  SQL_NUMERIC, sizeof(SQLUINTEGER), 0, 
                                  &uintval[0], 0, NULL));

  /* Set values for our parameters */
  for (j= 0; j<RCNT; j++)
  {
    uintval[j]= j;
  }

  ok_stmt(hstmt, SQLExecute(hstmt));
  ok_sql(hstmt, "SELECT * FROM t_bug18325878");

  while (SQLFetch(hstmt) == SQL_SUCCESS)
  {
    is_num(my_fetch_int(hstmt, 1), ++k);
  }

  is_num(k, RCNT);
  return OK;
}


/**
  Bug #18286366: SEG FAULT IN SQLFOREIGNKEYS() WHEN NUMBER OF COLUMNS IN 
                 THE TABLE IS MORE
*/
DECLARE_TEST(t_bug18286366)
{
  DECLARE_BASIC_HANDLES(henv1, hdbc1, hstmt1);
  SQLCHAR buff[2048], tmp_buff[50];
  int i, len= 0;

  is(OK == alloc_basic_handles_with_opt(&henv1, &hdbc1, &hstmt1, NULL, NULL, 
                                        NULL, NULL, "NO_I_S=1"));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug18286366b, t_bug18286366a");
  len= sprintf(buff, "CREATE TABLE t_bug18286366a ( ");
  for (i= 0; i < 20; i++)
  {
    len+= sprintf(buff + len, "`id%d` INT, UNIQUE(`id%d`),", i, i);
  }
  len= sprintf(buff + len - 1, ")");
  ok_stmt(hstmt, SQLExecDirect(hstmt, (SQLCHAR*)buff, SQL_NTS));

  len= sprintf(buff, "CREATE TABLE t_bug18286366b ( ");
  for (i= 0; i < 20; i++)
  {
    len+= sprintf(buff + len, "`id%d` INT, "
                              "CONSTRAINT `cons%d` FOREIGN KEY "
                              "(`id%d`) REFERENCES `t_bug18286366a` (`id%d`),", 
                              i, i, i, i);
  }
  len= sprintf(buff + len - 1, ")");
  ok_stmt(hstmt, SQLExecDirect(hstmt, (SQLCHAR*)buff, SQL_NTS));


  ok_stmt(hstmt1, SQLForeignKeys(hstmt1, NULL, 0, NULL, 0, 
                                (SQLCHAR *)"t_bug18286366a", SQL_NTS, NULL, 0,
                                NULL, 0, (SQLCHAR *)"t_bug18286366b", SQL_NTS));

  for (i= 0; i < 20; i++)
  {
    len= sprintf(tmp_buff, "id%d", i);
    ok_stmt(hstmt1, SQLFetch(hstmt1));
    is_str(my_fetch_str(hstmt1, buff, 3), "t_bug18286366a", 14);
    is_str(my_fetch_str(hstmt1, buff, 4), tmp_buff, len);
    is_str(my_fetch_str(hstmt1, buff, 7), "t_bug18286366b", 14);
    is_str(my_fetch_str(hstmt1, buff, 8), tmp_buff, len);
    len= sprintf(tmp_buff, "cons%d", i);
    is_str(my_fetch_str(hstmt1, buff, 12), tmp_buff, len);
  }

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug16920750b, t_bug16920750a");
  free_basic_handles(&henv1, &hdbc1, &hstmt1);
  return OK;
}


/**
  Bug #18286366_2: Segmentation fault in SQLForeignKeys() when number of 
                   columns in the table is more then 64
*/
#define MAX_18286366_KEYS 50
DECLARE_TEST(t_bug18286366_2)
{
  DECLARE_BASIC_HANDLES(henv1, hdbc1, hstmt1);
  SQLCHAR buff[8192], tmp_buff[100];
  int i, len= 0;

  is(OK == alloc_basic_handles_with_opt(&henv1, &hdbc1, &hstmt1, NULL, NULL, 
                                        NULL, NULL, "NO_I_S=1"));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug182863662c,  t_bug182863662b, t_bug182863662a");
  len= sprintf(buff, "CREATE TABLE t_bug182863662a ( ");
  for (i= 0; i < MAX_18286366_KEYS; i++)
  {
    len+= sprintf(buff + len, "`id%d` INT, UNIQUE(`id%d`),", i, i);
  }
  len= sprintf(buff + len - 1, ")");
  ok_stmt(hstmt, SQLExecDirect(hstmt, (SQLCHAR*)buff, SQL_NTS));

  len= sprintf(buff, "CREATE TABLE t_bug182863662b ( ");
  for (i= 0; i < MAX_18286366_KEYS; i++)
  {
    len+= sprintf(buff + len, "`id%d` INT, "
                              "CONSTRAINT `consb%d` FOREIGN KEY "
                              "(`id%d`) REFERENCES `t_bug182863662a` (`id%d`),", 
                              i, i, i, i);
  }
  len= sprintf(buff + len - 1, ")");
  ok_stmt(hstmt, SQLExecDirect(hstmt, (SQLCHAR*)buff, SQL_NTS));

  len= sprintf(buff, "CREATE TABLE t_bug182863662c ( ");
  for (i= 0; i < MAX_18286366_KEYS; i++)
  {
    len+= sprintf(buff + len, "`id%d` INT, "
                              "CONSTRAINT `consc%d` FOREIGN KEY "
                              "(`id%d`) REFERENCES `t_bug182863662a` (`id%d`),", 
                              i, i, i, i);
  }
  len= sprintf(buff + len - 1, ")");
  ok_stmt(hstmt, SQLExecDirect(hstmt, (SQLCHAR*)buff, SQL_NTS));

  ok_stmt(hstmt1, SQLForeignKeys(hstmt1, NULL, 0, NULL, 0, 
                                (SQLCHAR *)"t_bug182863662a", SQL_NTS, NULL, 0,
                                NULL, 0, (SQLCHAR *)"", SQL_NTS));

  for (i= 0; i < MAX_18286366_KEYS; i++)
  {
    ok_stmt(hstmt1, SQLFetch(hstmt1));
    is_str(my_fetch_str(hstmt1, buff, 3), "t_bug182863662a", 14);
    is_str(my_fetch_str(hstmt1, buff, 4), "id", 2);
    is_str(my_fetch_str(hstmt1, buff, 7), "t_bug182863662b", 14);
    is_str(my_fetch_str(hstmt1, buff, 8), "id", 2);
    is_str(my_fetch_str(hstmt1, buff, 12), "consb", 5);
  }

  for (i= 0; i < MAX_18286366_KEYS; i++)
  {
    ok_stmt(hstmt1, SQLFetch(hstmt1));
    is_str(my_fetch_str(hstmt1, buff, 3), "t_bug182863662a", 14);
    is_str(my_fetch_str(hstmt1, buff, 4), "id", 2);
    is_str(my_fetch_str(hstmt1, buff, 7), "t_bug182863662c", 14);
    is_str(my_fetch_str(hstmt1, buff, 8), "id", 2);
    is_str(my_fetch_str(hstmt1, buff, 12), "consc", 5);
  }
  
  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));
  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug169207502c, t_bug169207502b, t_bug169207502a");
  free_basic_handles(&henv1, &hdbc1, &hstmt1);
  return OK;
}


/*
  Bug #18286118 SEG FAULT IN SQLFOREIGNKEYS() WHEN COLUMN NAME CONTAINS
  SPECIAL CHARACTERS
*/
DECLARE_TEST(t_bug18286118)
{
  DECLARE_BASIC_HANDLES(henv1, hdbc1, hstmt1);

  char *tabname1= (char*)"t1";
  char *tabname2= (char*)"t2";
  char *colname1= (char*)"col1";
  char *colname2= (char*)") Specialname (";
  char tmpBuff[2048]= {0};
  SQLSMALLINT col_count= 0;
  SQLLEN nLen= 0;
  SQLRETURN sqlrc= SQL_SUCCESS;
  int i= 0;
  is_num(OK, alloc_basic_handles_with_opt(&henv1, &hdbc1, &hstmt1, NULL, 
                                      NULL, NULL, NULL,
                                      (SQLCHAR*)"NO_I_S=1"));

  sprintf(tmpBuff, "DROP TABLE IF EXISTS %s,%s CASCADE", tabname2,tabname1);
  ok_stmt(hstmt1, SQLExecDirect(hstmt1, (SQLCHAR*)tmpBuff, SQL_NTS));

  sprintf(tmpBuff, "CREATE TABLE %s (id int, %s bigint, primary key(%s,id)) "\
                   "COMMENT  \"  Comment1 \"", tabname1, colname1, colname1);
  ok_stmt(hstmt1, SQLExecDirect(hstmt1, (SQLCHAR*)tmpBuff, SQL_NTS));

  sprintf(tmpBuff, "CREATE TABLE %s (id  int, `%s` bigint, bd1 double, "\
                   "FOREIGN KEY (`%s`) REFERENCES %s(%s) ON DELETE CASCADE) "\
                   "COMMENT  \" Comment 2\"", tabname2, colname2, colname2,
                   tabname1, colname1);
  ok_stmt(hstmt1, SQLExecDirect(hstmt1, (SQLCHAR*)tmpBuff, SQL_NTS));

  ok_stmt(hstmt1, SQLForeignKeys(hstmt1, NULL, 0, NULL, 0, (SQLCHAR*)tabname1,
          SQL_NTS, NULL, 0, NULL, 0, (SQLCHAR*)tabname2, SQL_NTS));
  
  ok_stmt(hstmt1, SQLNumResultCols(hstmt1, &col_count));

  while (((sqlrc= SQLFetch(hstmt1)) == SQL_SUCCESS) || 
         (sqlrc == SQL_SUCCESS_WITH_INFO))
  {
    memset(tmpBuff, 0, sizeof(tmpBuff));
    /* PKTABLE_CAT */
    is_str(my_fetch_str(hstmt1, tmpBuff, 1), mydb, strlen(mydb));

    /* PKTABLE_NAME */
    is_str(my_fetch_str(hstmt1, tmpBuff, 3), tabname1, strlen(tabname1));

    /* PKCOLUMN_NAME */
    is_str(my_fetch_str(hstmt1, tmpBuff, 4), colname1, strlen(colname1));

    /* FKTABLE_CAT */
    is_str(my_fetch_str(hstmt1, tmpBuff, 5), mydb, strlen(mydb));

    /* FKTABLE_NAME */
    is_str(my_fetch_str(hstmt1, tmpBuff, 7), tabname2, strlen(tabname2));

    /* FKCOLUMN_NAME */
    is_str(my_fetch_str(hstmt1, tmpBuff, 8), colname2, strlen(tabname2));
  }
  ok_stmt(hstmt1, SQLFreeStmt(hstmt, SQL_CLOSE));

  sprintf(tmpBuff, "DROP TABLE %s,%s CASCADE", tabname2,tabname1);
  ok_stmt(hstmt1, SQLExecDirect(hstmt1, (SQLCHAR*)tmpBuff, SQL_NTS));

  free_basic_handles(&henv1, &hdbc1, &hstmt1);
  return OK;
}


/*
  Bug #18805520: Segmentation Fault in SQLSetPos() when NO_SSPS= 0
*/
DECLARE_TEST(t_setpos_update_no_ssps)
{
  DECLARE_BASIC_HANDLES(henv1, hdbc1, hstmt1);
  SQLINTEGER x[6];
  SQLINTEGER i;
  SQLINTEGER  id;

  ok_sql(hstmt, "drop table if exists t_setpos_update_no_ssps");
  ok_sql(hstmt, "create table t_setpos_update_no_ssps (x int not null, "
                "primary key (x) )");
  ok_sql(hstmt, "insert into t_setpos_update_no_ssps values (1), (2), (3), "
                "(4), (5), (6)");

  /* Connect with SSPS enabled */
  alloc_basic_handles_with_opt(&henv1, &hdbc1, &hstmt1, NULL, NULL, NULL,
                               NULL, "NO_SSPS=0");
  /* create cursor and get first row */
  ok_stmt(hstmt, SQLPrepare(hstmt1, "select x from t_setpos_update_no_ssps "
                                   "where x > ?", SQL_NTS));
  id= 1;
  ok_stmt(hstmt1, SQLBindParameter(hstmt1, 1, SQL_PARAM_INPUT, SQL_C_ULONG,
                        SQL_INTEGER, 0, 0, &id, 0, NULL));
  ok_stmt(hstmt1, SQLExecute(hstmt1));

  ok_stmt(hstmt1, SQLBindCol(hstmt1, 1, SQL_C_LONG, x, 0, NULL));
  ok_stmt(hstmt1, SQLFetchScroll(hstmt1, SQL_FETCH_NEXT, 0));

  for (i= 0; i < 6; ++i)
    x[i]= i * 10;

  ok_stmt(hstmt1, SQLSetPos(hstmt1, 1, SQL_UPDATE, SQL_LOCK_NO_CHANGE));

  ok_stmt(hstmt1, SQLFreeStmt(hstmt1, SQL_CLOSE));
  free_basic_handles(&henv1, &hdbc1, &hstmt1);

  ok_sql(hstmt, "drop table if exists t_setpos_update_no_ssps");
  return OK;
}


BEGIN_TESTS
  ADD_TEST(t_bug69950)
  ADD_TEST(t_bug70642)
  ADD_TEST(t_bug17358838)
  ADD_TEST(t_bug17587913)
  ADD_TEST(t_bug17857204)
  ADD_TEST(t_bug17854697)
  ADD_TEST(t_bug17999659)
  ADD_TEST(t_bug17966018)
  ADD_TEST(t_bug17841121)
  ADD_TEST(t_bookmark_update_zero_rec)
  ADD_TEST(t_bug17085344)
  ADD_TEST(t_bug18165197)
  ADD_TEST(t_bug18325878)
  ADD_TEST(t_bug18286366)
  ADD_TEST(t_bug18286366_2)
  ADD_TEST(t_bug18286118)
  ADD_TEST(t_setpos_update_no_ssps)
END_TESTS

RUN_TESTS
