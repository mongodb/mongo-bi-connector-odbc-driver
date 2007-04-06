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


BEGIN_TESTS
  ADD_TEST(t_blob)
  ADD_TEST(t_1piecewrite2)
END_TESTS


RUN_TESTS

