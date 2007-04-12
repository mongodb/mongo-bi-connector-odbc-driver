/*
  Copyright (C) 1997-2007 MySQL AB

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

DECLARE_TEST(t_blob)
{
    SQLRETURN rc;
    SQLUINTEGER j= 0;
    SQLINTEGER l;
    SQLLEN cbValue;
    char *blobbuf;
    SQLUINTEGER blobbuf_size = 1024 * 1 * 6L;
    SQLUINTEGER blob_read;
    SQLPOINTER token;
    clock_t start, finish;
    double duration;
    SQLUINTEGER blob_size = 1 * 1024L * 5L;

    rc = SQLSetConnectOption(hdbc, SQL_AUTOCOMMIT, 0L);
    mycon(hdbc,rc);

    rc = SQLExecDirect(hstmt,
                       "DROP TABLE TBLOB",
                       SQL_NTS);
    rc = SQLTransact(NULL, hdbc, SQL_COMMIT);

    rc = SQLExecDirect(hstmt,
                       "CREATE TABLE TBLOB (I INTEGER NOT NULL PRIMARY KEY, B LONGBLOB)",
                       SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLTransact(NULL, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);

    cbValue = 0;
    rc = SQLPrepare(hstmt, "INSERT INTO TBLOB VALUES (1, ?)", SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLBindParameter(hstmt,SQL_PARAM_INPUT, 1, SQL_C_BINARY, SQL_LONGVARBINARY, 
                          blob_size, 0, NULL, 0, &cbValue);
    mystmt(hstmt,rc);
    cbValue = SQL_DATA_AT_EXEC;        
    blobbuf = (SQLCHAR *)malloc(blobbuf_size);
    memset(blobbuf, 'A', blobbuf_size);

    start = clock();

    rc = SQLExecute(hstmt);
    mystmt_err(hstmt,rc == SQL_NEED_DATA, rc);
    rc = SQLParamData(hstmt, &token);
    if (rc == SQL_NEED_DATA)
    {
        for (j = 0; j < blob_size; )
        {
            SDWORD s;

            s = (SDWORD)blobbuf_size;
            if (s + j > blob_size)
            {
                s -= (s + j) - blob_size;
                myassert(s + j == blob_size);
            }
            rc = SQLPutData(hstmt, blobbuf, s);
            mystmt(hstmt,rc);
            j += (SQLUINTEGER)s;
        }
        rc = SQLParamData(hstmt, &token);
        mystmt(hstmt,rc);
    }
    else
    {
        my_error();
    }
    finish = clock();

    duration = (finish-start)/CLOCKS_PER_SEC;
    printMessage("\n j: %d\n", j);
    myassert(j == blob_size);
    printMessage("Wrote %ld bytes in %3.3lf seconds (%lg bytes/s)\n",
                 j, duration, duration == 0.0 ? 9.99e99 : j / duration);

    rc = SQLTransact(NULL, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    mystmt(hstmt,rc);


    memset(blobbuf, ~0, 100);
    rc = SQLPrepare(hstmt, "SELECT I, B FROM TBLOB WHERE I = 1", SQL_NTS);
    mystmt(hstmt,rc);

    start = clock();

    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);
    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    rc = SQLGetData(hstmt, 1, SQL_C_LONG, &l, 0L, &cbValue);
    mystmt(hstmt,rc);

    blob_read = 0L;
    do
    {
        rc = SQLGetData(hstmt, 2, SQL_C_BINARY, blobbuf, blobbuf_size, &cbValue);
        myassert(cbValue > 0);
        blob_read += (cbValue < blobbuf_size ? cbValue : blobbuf_size);
    } while (rc == SQL_SUCCESS_WITH_INFO);
    myassert(rc == SQL_SUCCESS);
    myassert(blob_read == blob_size);
    finish = clock();
    duration = (finish-start)/CLOCKS_PER_SEC;
    printMessage("Read  %ld bytes in %3.3lf seconds (%lg bytes/s)\n",
                 blob_read, duration, duration == 0.0 ? 9.99e99 : blob_read / duration);

    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);
    free(blobbuf);

  return OK;
}

DECLARE_TEST(t_1piecewrite2)
{
    SQLRETURN rc;
    SQLLEN cbValue,cbValue2;
    SQLINTEGER l;
    SQLCHAR* blobbuf;
    size_t i;

    rc = SQLExecDirect(hstmt,
                       "DROP TABLE TBLOB",
                       SQL_NTS);
    rc = SQLTransact(NULL, hdbc, SQL_COMMIT);

    rc = SQLExecDirect(hstmt,
                       "CREATE TABLE TBLOB (I INTEGER NOT NULL PRIMARY KEY, B LONG VARCHAR NOT NULL)",
                       SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLTransact(NULL, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);

    cbValue = 3510L;

    blobbuf = (SQLCHAR *)malloc((size_t)cbValue + 1);
    for (i = 0; i < (size_t)cbValue; i++)
    {
        blobbuf[i] = (char)((i % ('z' - 'a' + 1)) + 'a');
    }
    blobbuf[i] = '\0';
    l = 1;
    rc = SQLBindParameter(hstmt,SQL_PARAM_INPUT,1, SQL_C_LONG, SQL_INTEGER, 0, 0, &l,0, NULL);
    mystmt(hstmt,rc);
    rc = SQLBindParameter(hstmt,SQL_PARAM_INPUT, 2, SQL_C_CHAR, SQL_LONGVARCHAR, 0, 0, blobbuf,cbValue, NULL);
    mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt, "INSERT INTO TBLOB VALUES (1,?)", SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLTransact(NULL, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);
    memset(blobbuf, 1, (size_t)cbValue);
    rc = SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt, "SELECT B FROM TBLOB WHERE I = 1", SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    rc = SQLGetData(hstmt, 1, SQL_C_BINARY, blobbuf, cbValue, &cbValue2);
    mystmt(hstmt,rc);
    myassert(cbValue2 == cbValue);
    for (i = 0; i < (size_t)cbValue; i++)
    {
        myassert(blobbuf[i] == (char)((i % ('z' - 'a' + 1)) + 'a'));
    }
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);
    rc = SQLTransact(NULL, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);
    free(blobbuf);

  return OK;
}


/* Test for a simple SQLPutData and SQLParamData handling for longtext */
DECLARE_TEST(t_putdata)
{
  SQLRETURN  rc;
  SQLLEN     pcbLength;
  SQLINTEGER c1;
  SQLCHAR    data[255];
  SQLPOINTER token;

    SQLExecDirect(hstmt,"drop table t_putdata",SQL_NTS);
    rc = SQLExecDirect(hstmt,"create table t_putdata(c1 int, c2 long varchar)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"insert into t_putdata values(?,?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_LONG,
                          SQL_INTEGER,0,0,&c1,0,NULL);

    rc = SQLBindParameter(hstmt,2,SQL_PARAM_INPUT,SQL_C_CHAR,
                          SQL_LONGVARCHAR,0,0,
                          (SQLPOINTER)1,0,&pcbLength);

    pcbLength =  SQL_LEN_DATA_AT_EXEC(0);

    c1 = 10;
    rc = SQLExecute(hstmt);
    myassert(rc == SQL_NEED_DATA);

    rc = SQLParamData(hstmt, &token);
    myassert(rc == SQL_NEED_DATA);

    strcpy(data,"mysql ab");
    rc = SQLPutData(hstmt,data,6);
    mystmt(hstmt,rc);

    strcpy(data,"- the open source database company");
    rc = SQLPutData(hstmt,data,strlen(data));
    mystmt(hstmt,rc);

    rc = SQLParamData(hstmt, &token);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"select c2 from t_putdata where c1= 10",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    pcbLength= 0;
    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &pcbLength);
    mystmt(hstmt,rc);
    fprintf(stdout,"data: %s(%d)\n", data, pcbLength);
    myassert(strcmp(data,"mysql - the open source database company")==0);
    myassert(pcbLength == 40);

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);

  return OK;
}

/* Test for a simple SQLPutData and SQLParamData handling for longtext */
DECLARE_TEST(t_putdata1)
{
  SQLRETURN  rc;
  SQLLEN     pcbLength;
  SQLINTEGER c1;
  SQLCHAR    data[255];
  SQLPOINTER token;

    SQLExecDirect(hstmt,"drop table t_putdata",SQL_NTS);
    rc = SQLExecDirect(hstmt,"create table t_putdata(c1 int, c2 long varchar)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into t_putdata values(10,'venu')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"update t_putdata set c2= ? where c1 = ?",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_CHAR,
                          SQL_LONGVARCHAR,0,0,
                          (SQLPOINTER)1,0,&pcbLength);

    rc = SQLBindParameter(hstmt,2,SQL_PARAM_INPUT,SQL_C_LONG,
                          SQL_INTEGER,0,0,&c1,0,NULL);

    pcbLength =  SQL_LEN_DATA_AT_EXEC(0);

    c1 = 10;
    rc = SQLExecute(hstmt);
    myassert(rc == SQL_NEED_DATA);

    rc = SQLParamData(hstmt, &token);
    myassert(rc == SQL_NEED_DATA);

    strcpy(data,"mysql ab");
    rc = SQLPutData(hstmt,data,6);
    mystmt(hstmt,rc);

    strcpy(data,"- the open source database company");
    rc = SQLPutData(hstmt,data,strlen(data));
    mystmt(hstmt,rc);

    rc = SQLParamData(hstmt, &token);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"select c2 from t_putdata where c1= 10",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    pcbLength= 0;
    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &pcbLength);
    mystmt(hstmt,rc);
    fprintf(stdout,"data: %s(%d)\n", data, pcbLength);
    myassert(strcmp(data,"mysql - the open source database company")==0);
    myassert(pcbLength == 40);

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);

  return OK;
}


/* Test for a simple SQLPutData and SQLParamData handling for longtext */
DECLARE_TEST(t_putdata2)
{
  SQLRETURN  rc;
  SQLLEN     pcbLength;
  SQLINTEGER c1;
  SQLCHAR    data[255];
  SQLPOINTER token;

    SQLExecDirect(hstmt,"drop table t_putdata",SQL_NTS);
    rc = SQLExecDirect(hstmt,"create table t_putdata(c1 int, c2 long varchar, c3 long varchar)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"insert into t_putdata values(?,?,?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_LONG,
                          SQL_INTEGER,0,0,&c1,0,NULL);

    rc = SQLBindParameter(hstmt,2,SQL_PARAM_INPUT,SQL_C_CHAR,
                          SQL_LONGVARCHAR,0,0,
                          (SQLPOINTER)1,0,&pcbLength);

    rc = SQLBindParameter(hstmt,3,SQL_PARAM_INPUT,SQL_C_CHAR,
                          SQL_LONGVARCHAR,0,0,
                          (SQLPOINTER)1,0,&pcbLength);

    pcbLength =  SQL_LEN_DATA_AT_EXEC(0);

    c1 = 10;
    rc = SQLExecute(hstmt);
    myassert(rc == SQL_NEED_DATA);

    rc = SQLParamData(hstmt, &token);
    myassert(rc == SQL_NEED_DATA);

    strcpy(data,"mysql ab");
    rc = SQLPutData(hstmt,data,6);
    mystmt(hstmt,rc);

    strcpy(data,"- the open source database company");
    rc = SQLPutData(hstmt,data,strlen(data));
    mystmt(hstmt,rc);

    rc = SQLParamData(hstmt, &token);
    myassert(rc == SQL_NEED_DATA);

    strcpy(data,"MySQL AB");
    rc = SQLPutData(hstmt,data, 8);
    mystmt(hstmt,rc);

    rc = SQLParamData(hstmt, &token);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"select c2,c3 from t_putdata where c1= 10",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    pcbLength= 0;
    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, data, sizeof(data), &pcbLength);
    mystmt(hstmt,rc);
    fprintf(stdout,"data: %s(%d)\n", data, pcbLength);
    myassert(strcmp(data,"mysql - the open source database company")==0);
    myassert(pcbLength == 40);

    pcbLength= 0;
    rc = SQLGetData(hstmt, 2, SQL_C_CHAR, data, sizeof(data), &pcbLength);
    mystmt(hstmt,rc);
    fprintf(stdout,"data: %s(%d)\n", data, pcbLength);
    myassert(strcmp(data,"MySQL AB")==0);
    myassert(pcbLength == 8);

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);

  return OK;
}


/* Test for a simple SQLPutData and SQLParamData handling bug #1316 */
DECLARE_TEST(t_putdata3)
{
  char buffer[]= "MySQL - The worlds's most popular open source database";
  SQLRETURN  rc;
  const int MAX_PART_SIZE = 5;

  char *pdata= 0, data[50];
  int dynData;
  int commonLen= 20;

  SQLINTEGER  id, id1, id2, id3, resId;
  SQLINTEGER  resUTimeSec;
  SQLINTEGER  resUTimeMSec;
  SQLINTEGER  resDataLen;
  SQLLEN      resData;

    SQLExecDirect(hstmt,"drop table t_putdata3",SQL_NTS);
    rc = SQLExecDirect(hstmt,"CREATE TABLE t_putdata3 ( id INT, id1  INT, \
                     id2 INT, id3  INT, pdata blob);",SQL_NTS);
    mystmt(hstmt,rc);

    dynData = 1;

    rc = SQLPrepare(hstmt, "INSERT INTO t_putdata3 VALUES ( ?, ?, ?, ?, ? )", SQL_NTS);
    mystmt(hstmt,rc);

    id= 1, id1= 2, id2= 3, id3= 4;
    resId = 0;
    resUTimeSec = 0;
    resUTimeMSec = 0;
    resDataLen = 0;
    resData = SQL_LEN_DATA_AT_EXEC(0);

    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG,
                          SQL_INTEGER, 0, 0, &id, 0, &resId);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG,
                          SQL_INTEGER, 0, 0, &id1, 0, &resUTimeSec);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_SLONG,
                          SQL_INTEGER, 0, 0, &id2, 0, &resUTimeMSec);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_SLONG,
                          SQL_INTEGER, 0, 0, &id3, 0,
                          &resDataLen);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt, 5, SQL_PARAM_INPUT,
                          SQL_C_BINARY, SQL_LONGVARBINARY, 10, 10,
                          dynData ? (SQLPOINTER)5 :
                          pdata, 0, &resData);
    mystmt(hstmt,rc);

    rc = SQLExecute(hstmt);
    if (rc == SQL_NEED_DATA)
    {
      int parameter;
      if (SQLParamData(hstmt,(void**)&parameter) == SQL_NEED_DATA && parameter == 5)
      {
        int len = 0;
        int partsize;

        /* storing long data by parts */
        while (len < commonLen)
        {
          partsize = commonLen - len;
          if (partsize > MAX_PART_SIZE)
            partsize = MAX_PART_SIZE;

          rc = SQLPutData(hstmt, buffer+len, partsize);
          mystmt(hstmt,rc);
          len += partsize;
        }
        if (SQLParamData(hstmt,(void**)&parameter) == SQL_ERROR)
        {

        }
      }
    } /* end if (rc == SQL_NEED_DATA) */

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    if (mysql_min_version(hdbc, "4.0", 3))
    {
      rc = tmysql_exec(hstmt,"select id, id1, id2, id3,  convert(pdata,char) from t_putdata3");
      mystmt(hstmt,rc);

      rc = SQLFetch(hstmt);

      my_assert(1 == my_fetch_int(hstmt,1));
      my_assert(2 == my_fetch_int(hstmt,2));
      my_assert(3 == my_fetch_int(hstmt,3));
      my_assert(4 == my_fetch_int(hstmt,4));

      my_assert(strncmp(buffer, my_fetch_str(hstmt,data,5), commonLen) == 0);
    }
    else
    {
      rc = tmysql_exec(hstmt,"select id, id1, id2, id3,  pdata from t_putdata3");
      mystmt(hstmt,rc);

      rc = SQLFetch(hstmt);

      my_assert(1 == my_fetch_int(hstmt,1));
      my_assert(2 == my_fetch_int(hstmt,2));
      my_assert(3 == my_fetch_int(hstmt,3));
      my_assert(4 == my_fetch_int(hstmt,4));
      my_assert(strncmp("4D7953514C202D2054686520776F726C64732773",
                my_fetch_str(hstmt,data,5), commonLen) == 0);
    }

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    /*
     output:

      ######################################
      t_putdata3
      ######################################
       my_fetch_int: 1
       my_fetch_int: 2
       my_fetch_int: 3
       my_fetch_int: 4
       my_fetch_str: MySQL - The worlds's(20)
    */

  return OK;
}


/* Test the bug when blob size > 8k */
DECLARE_TEST(t_blob_bug)
{
  SQLRETURN  rc;
  SQLCHAR    *data;
  SQLINTEGER i;
  SQLLEN     length;
  const SQLINTEGER max_blob_size=1024*100;

    SQLExecDirect(hstmt,"drop table t_blob",SQL_NTS);
    rc = SQLExecDirect(hstmt,"create table t_blob(blb long varbinary)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"insert into t_blob values(?)",SQL_NTS);
    mystmt(hstmt,rc);

    if (!(data = (SQLCHAR *)calloc(max_blob_size,sizeof(SQLCHAR))))
    {
      SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
      SQLFreeStmt(hstmt,SQL_CLOSE);
      return;
    }

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_CHAR,SQL_VARBINARY,
                          0,0,data,0,&length);
    mystmt(hstmt,rc);

    memset(data,'X',max_blob_size);

    fprintf(stdout,"inserting %d rows\n\n", max_blob_size / 1024);
    for (length=1024; length <= max_blob_size; length+= 1024)
    {
      fprintf(stdout,"\r %d", length/1024);
      rc = SQLExecute(hstmt);
      mystmt(hstmt,rc);
    }

    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"SELECT length(blb) FROM t_blob",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&length,0,NULL);
    mystmt(hstmt,rc);

    for (i= 1; i <= max_blob_size/1024; i++)
    {
      rc = SQLFetch(hstmt);
      mystmt(hstmt,rc);

      fprintf(stdout,"row %d length: %d\n", i, length);
      myassert(length == i * 1024);
    }
    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA);

    free(data);

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);

  return OK;
}


#define TEST_ODBC_TEXT_LEN 3000
DECLARE_TEST(t_text_fetch)
{
  SQLRETURN  rc;
  SQLINTEGER i;
  SQLLEN     row_count, length;
  SQLCHAR    data[TEST_ODBC_TEXT_LEN+1];

    SQLExecDirect(hstmt,"drop table t_text_fetch",SQL_NTS);
    rc = SQLExecDirect(hstmt,"create table t_text_fetch(t1 tinytext, \
                                                      t2 text, \
                                                      t3 mediumtext, \
                                                      t4 longtext)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"insert into t_text_fetch values(?,?,?,?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR,
                          0,0,(char *)data, TEST_ODBC_TEXT_LEN/3, NULL);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR,
                          0,0,(char *)data, TEST_ODBC_TEXT_LEN/2, NULL);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR,
                          0,0,(char *)data,
                          (SQLINTEGER)(TEST_ODBC_TEXT_LEN/1.5), NULL);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR,
                          0,0,(char *)data, TEST_ODBC_TEXT_LEN-1, NULL);
    mystmt(hstmt,rc);

    memset(data,'A',TEST_ODBC_TEXT_LEN);
    data[TEST_ODBC_TEXT_LEN]='\0';

    for (i=0; i < 10; i++)
    {
      rc = SQLExecute(hstmt);
      mystmt(hstmt,rc);
    }

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"SELECT * FROM t_text_fetch",SQL_NTS);
    mystmt(hstmt,rc);

    row_count= 0;
    rc = SQLFetch(hstmt);
    while (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    {
       fprintf(stdout,"row '%d' (lengths: \n", row_count);
       rc = SQLGetData(hstmt,1,SQL_C_CHAR,(char *)data,TEST_ODBC_TEXT_LEN,&length);
       mystmt(hstmt,rc);
       fprintf(stdout,"%d", length);
       myassert(length == 255);

       rc = SQLGetData(hstmt,2,SQL_C_CHAR,(char *)data,TEST_ODBC_TEXT_LEN,&length);
       mystmt(hstmt,rc);
       fprintf(stdout,",%d", length);
       myassert(length == TEST_ODBC_TEXT_LEN/2);

       rc = SQLGetData(hstmt,3,SQL_C_CHAR,(char *)data,TEST_ODBC_TEXT_LEN,&length);
       mystmt(hstmt,rc);
       fprintf(stdout,",%d", length);
       myassert(length == (SQLINTEGER)(TEST_ODBC_TEXT_LEN/1.5));

       rc = SQLGetData(hstmt,4,SQL_C_CHAR,(char *)data,TEST_ODBC_TEXT_LEN,&length);
       mystmt(hstmt,rc);
       fprintf(stdout,",%d)", length);
       myassert(length == TEST_ODBC_TEXT_LEN-1);
       row_count++;

       rc = SQLFetch(hstmt);
    }
    fprintf(stdout,"total rows: %d\n", row_count);
    myassert(row_count == i);

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"DROP TABLE t_text_fetch",SQL_NTS);
    mystmt(hstmt,rc);

  return OK;
}


/*
  Test retrieving the length of a field with a non-null zero-length buffer.
  This is how ADO does it for long-type fields.
*/
DECLARE_TEST(getdata_lenonly)
{
  SQLLEN     len;
  SQLCHAR    buf[1];

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_getdata_lenonly");
  ok_sql(hstmt, "CREATE TABLE t_getdata_lenonly (a CHAR(4))");
  ok_sql(hstmt, "INSERT INTO t_getdata_lenonly VALUES ('venu')");

  ok_sql(hstmt, "SELECT a FROM t_getdata_lenonly");
  ok_stmt(hstmt, SQLFetch(hstmt));

  expect_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, buf, 0, &len),
              SQL_SUCCESS_WITH_INFO);
  is_num(len, 4);

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_getdata_lenonly");

  return OK;
}


BEGIN_TESTS
  ADD_TEST(t_blob)
  ADD_TEST(t_1piecewrite2)
  ADD_TEST(t_putdata)
  ADD_TEST(t_putdata1)
  ADD_TEST(t_putdata2)
  ADD_TEST(t_putdata3)
  ADD_TEST(t_blob_bug)
  ADD_TEST(t_text_fetch)
  ADD_TEST(getdata_lenonly)
END_TESTS


RUN_TESTS

