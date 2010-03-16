/*
  Copyright (c) 2003, 2010, Oracle and/or its affiliates. All rights reserved.

  The MySQL Connector/ODBC is licensed under the terms of the
  GPL, like most MySQL Connectors. There are special exceptions
  to the terms and conditions of the GPL as it is applied to
  this software, see the FLOSS License Exception available on
  mysql.com.

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

/********************************************************
* initialize tables                                     *
*********************************************************/
DECLARE_TEST(my_init_table)
{
    SQLRETURN   rc;

    ok_sql(hstmt, "DROP TABLE if exists my_demo_param");

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, hdbc, SQL_COMMIT);
    mycon(hdbc,rc);

    /* create the table 'my_demo_param' */
    ok_sql(hstmt, "CREATE TABLE my_demo_param(\
                              id   int,\
                              auto int primary key auto_increment,\
                              name varchar(20),\
                              timestamp timestamp(14))");

  return OK;
}


DECLARE_TEST(my_param_insert)
{
    SQLRETURN   rc;
    SQLINTEGER  id;
    char        name[50];

    /* prepare the insert statement with parameters */
    rc = SQLPrepare(hstmt, (SQLCHAR *)"INSERT INTO my_demo_param(id,name) VALUES(?,?)",SQL_NTS);
    mystmt(hstmt,rc);

    /* now supply data to parameter 1 and 2 */
    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT,
                          SQL_C_LONG, SQL_INTEGER, 0,0,
                          &id, 0, NULL);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT,
                          SQL_C_CHAR, SQL_CHAR, 0,0,
                          name, sizeof(name), NULL);
    mystmt(hstmt,rc);

    /* now insert 10 rows of data */
    for (id = 0; id < 10; id++)
    {
        sprintf(name,"MySQL%d",id);

        rc = SQLExecute(hstmt);
        mystmt(hstmt,rc);
    }

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
    ok_sql(hstmt, "SELECT * FROM my_demo_param");

    is(10 == myresult(hstmt));

  return OK;
}


DECLARE_TEST(my_param_update)
{
    SQLRETURN  rc;
    SQLLEN nRowCount;
    SQLINTEGER id=9;
    char name[]="update";

    /* prepare the insert statement with parameters */
    rc = SQLPrepare(hstmt, (SQLCHAR *)"UPDATE my_demo_param set name = ? WHERE id = ?",SQL_NTS);
    mystmt(hstmt,rc);

    /* now supply data to parameter 1 and 2 */
    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT,
                          SQL_C_CHAR, SQL_CHAR, 0,0,
                          name, sizeof(name), NULL);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT,
                          SQL_C_LONG, SQL_INTEGER, 0,0,
                          &id, 0, NULL);
    mystmt(hstmt,rc);

    /* now execute the update statement */
    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);

    /* check the rows affected by the update statement */
    rc = SQLRowCount(hstmt, &nRowCount);
    mystmt(hstmt,rc);
    printMessage("\n total rows updated:%d\n",nRowCount);
    is( nRowCount == 1);

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
    ok_sql(hstmt, "SELECT * FROM my_demo_param");
    mystmt(hstmt,rc);

    is(10 == myresult(hstmt));

  return OK;
}


DECLARE_TEST(my_param_delete)
{
    SQLRETURN  rc;
    SQLINTEGER id;
    SQLLEN nRowCount;

    /* supply data to parameter 1 */
    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT,
                          SQL_C_LONG, SQL_INTEGER, 0,0,
                          &id, 0, NULL);
    mystmt(hstmt,rc);

    /* execute the DELETE STATEMENT to delete 5th row  */
    id = 5;
    ok_sql(hstmt,"DELETE FROM my_demo_param WHERE id = ?");

    /* check the rows affected by the update statement */
    rc = SQLRowCount(hstmt, &nRowCount);
    mystmt(hstmt,rc);
    printMessage(" total rows deleted:%d\n",nRowCount);
    is( nRowCount == 1);

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    /* execute the DELETE STATEMENT to delete 8th row  */
    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT,
                          SQL_C_LONG, SQL_INTEGER, 0,0,
                          &id, 0, NULL);
    mystmt(hstmt,rc);

    id = 8;
    ok_sql(hstmt,"DELETE FROM my_demo_param WHERE id = ?");

    /* check the rows affected by the update statement */
    rc = SQLRowCount(hstmt, &nRowCount);
    mystmt(hstmt,rc);
    printMessage(" total rows deleted:%d\n",nRowCount);
    is( nRowCount == 1);

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
    ok_sql(hstmt, "SELECT * FROM my_demo_param");

    is(8 == myresult(hstmt));

    /* drop the table */
    ok_sql(hstmt,"DROP TABLE my_demo_param");

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(tmysql_fix)
{
    SQLRETURN rc;

  ok_sql(hstmt, "DROP TABLE IF EXISTS tmysql_err");

  ok_sql(hstmt,"CREATE TABLE tmysql_err (\
                  td date NOT NULL default '0000-00-00',\
                  node varchar(8) NOT NULL default '',\
                  tag varchar(10) NOT NULL default '',\
                  sqlname varchar(8) default NULL,\
                  fix_err varchar(100) default NULL,\
                  sql_err varchar(255) default NULL,\
                  prog_err varchar(100) default NULL\
                ) TYPE=MyISAM");

  ok_sql(hstmt,"INSERT INTO tmysql_err VALUES\
                  ('0000-00-00','0','0','0','0','0','0'),\
                  ('2001-08-29','FIX','SQLT2','ins1',\
                  NULL,NULL, 'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2',\
                  'ins1',NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2',\
                  'ins1',NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2','ins1',\
                  NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2',\
                  'ins1',NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2',\
                  'ins1',NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2',\
                  'ins1',NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000!-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2',\
                  'ins1',NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2',\
                  'ins1',NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.')");

    /* trace based */
    {
        SQLSMALLINT pcpar,pccol,pfSqlType,pibScale,pfNullable;
        SQLSMALLINT index;
        SQLCHAR     td[30]="20010830163225";
        SQLCHAR     node[30]="FIX";
        SQLCHAR     tag[30]="SQLT2";
        SQLCHAR     sqlname[30]="ins1";
        SQLCHAR     sqlerr[30]="error";
        SQLCHAR     fixerr[30]= "fixerr";
        SQLCHAR     progerr[30]="progerr";
        SQLULEN     pcbParamDef;

        SQLFreeStmt(hstmt,SQL_CLOSE);
        rc = SQLPrepare(hstmt, (SQLCHAR *)"insert into tmysql_err (TD, NODE, TAG, SQLNAME, SQL_ERR, FIX_ERR, PROG_ERR)\
                         values (?, ?, ?, ?, ?, ?, ?)",200);
        mystmt(hstmt,rc);

        rc = SQLNumParams(hstmt,&pcpar);
        mystmt(hstmt,rc);

        rc = SQLNumResultCols(hstmt,&pccol);
        mystmt(hstmt,rc);

        for (index=1; index <= pcpar; index++)
        {
            rc = SQLDescribeParam(hstmt,index,&pfSqlType,&pcbParamDef,&pibScale,&pfNullable);
            mystmt(hstmt,rc);

            printMessage("descparam[%d]:%d,%d,%d,%d\n",index,pfSqlType,pcbParamDef,pibScale,pfNullable);
        }

        rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,11,12,0,0,td,100,0);
        mystmt(hstmt,rc);

        rc = SQLBindParameter(hstmt,2,SQL_PARAM_INPUT,1,12,0,0,node,100,0);
        mystmt(hstmt,rc);

        rc = SQLBindParameter(hstmt,3,SQL_PARAM_INPUT,1,12,0,0,tag,100,0);
        mystmt(hstmt,rc);
        rc = SQLBindParameter(hstmt,4,SQL_PARAM_INPUT,1,12,0,0,sqlname,100,0);
        mystmt(hstmt,rc);
        rc = SQLBindParameter(hstmt,5,SQL_PARAM_INPUT,1,12,0,0,sqlerr,0,0);
        mystmt(hstmt,rc);
        rc = SQLBindParameter(hstmt,6,SQL_PARAM_INPUT,1,12,0,0,fixerr,0,0);
        mystmt(hstmt,rc);
        rc = SQLBindParameter(hstmt,7,SQL_PARAM_INPUT,1,12,0,0,progerr,0,0);
        mystmt(hstmt,rc);

        rc = SQLExecute(hstmt);
        mystmt(hstmt,rc);
    }

  ok_sql(hstmt, "DROP TABLE IF EXISTS tmysql_err");

  return OK;
}


/*
  Test basic handling of SQL_ATTR_PARAM_BIND_OFFSET_PTR
*/
DECLARE_TEST(t_param_offset)
{
  const SQLINTEGER rowcnt= 5;
  SQLINTEGER i;
  struct {
    SQLINTEGER id;
    SQLINTEGER x;
  } rows[25];
  size_t row_size= (sizeof(rows) / 25);
  SQLINTEGER out_id, out_x;
  SQLULEN bind_offset= 20 * row_size;

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_param_offset");
  ok_sql(hstmt, "CREATE TABLE t_param_offset (id int not null, x int)");

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAM_BIND_OFFSET_PTR,
                                &bind_offset, 0));

  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG,
                                  SQL_INTEGER, 0, 0, &rows[0].id, 0, NULL));
  ok_stmt(hstmt, SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG,
                                  SQL_INTEGER, 0, 0, &rows[0].x, 0, NULL));

  for (i= 0; i < rowcnt; ++i)
  {
    rows[20+i].id= i * 10;
    rows[20+i].x= (i * 1000) % 97;
    ok_sql(hstmt, "insert into t_param_offset values (?,?)");
    bind_offset+= row_size;
  }

  /* verify the data */

  ok_sql(hstmt, "select id, x from t_param_offset order by 1");

  ok_stmt(hstmt, SQLBindCol(hstmt, 1, SQL_C_LONG, &out_id, 0, NULL));
  ok_stmt(hstmt, SQLBindCol(hstmt, 2, SQL_C_LONG, &out_x, 0, NULL));

  for (i= 0; i < rowcnt; ++i)
  {
    ok_stmt(hstmt, SQLFetch(hstmt));
    is_num(out_id, rows[20+i].id);
    is_num(out_id, i * 10);
    is_num(out_x, rows[20+i].x);
    is_num(out_x, (i * 1000) % 97);
  }

  return OK;
}


/*
Bug 48310 - parameters array support request.
Binding by row test
*/
DECLARE_TEST(paramarray_by_row)
{
#define ROWS_TO_INSERT 3
#define STR_FIELD_LENGTH 255
  typedef struct DataBinding
  {
    SQLCHAR     bData[5];
    SQLINTEGER  intField;
    SQLCHAR     strField[STR_FIELD_LENGTH];
    SQLLEN      indBin;
    SQLLEN      indInt;
    SQLLEN      indStr;
  } DATA_BINDING;

   const SQLCHAR *str[]= {"nothing for 1st", "longest string for row 2", "shortest"  };

  SQLCHAR       buff[50];
  DATA_BINDING  dataBinding[ROWS_TO_INSERT];
  SQLUSMALLINT  paramStatusArray[ROWS_TO_INSERT];
  SQLULEN       paramsProcessed, i, nLen;
  SQLLEN        rowsCount;

  ok_stmt(hstmt, SQLExecDirect(hstmt, "DROP TABLE IF EXISTS t_bug48310", SQL_NTS));
  ok_stmt(hstmt, SQLExecDirect(hstmt, "CREATE TABLE t_bug48310 (id int primary key auto_increment,"\
    "bData binary(5) NULL, intField int not null, strField varchar(255) not null)", SQL_NTS));

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAM_BIND_TYPE, (SQLPOINTER)sizeof(DATA_BINDING), 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)ROWS_TO_INSERT, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAM_STATUS_PTR, paramStatusArray, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMS_PROCESSED_PTR, &paramsProcessed, 0));

  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_BINARY,
    0, 0, dataBinding[0].bData, 0, &dataBinding[0].indBin));
  ok_stmt(hstmt, SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
    0, 0, &dataBinding[0].intField, 0, &dataBinding[0].indInt));
  ok_stmt(hstmt, SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR,
    0, 0, dataBinding[0].strField, 0, &dataBinding[0].indStr ));

  memcpy(dataBinding[0].bData, "\x01\x80\x00\x80\x00", 5);
  dataBinding[0].intField= 1;
 
  memcpy(dataBinding[1].bData, "\x02\x80\x00\x80", 4);
  dataBinding[1].intField= 0;
 
  memcpy(dataBinding[2].bData, "\x03\x80\x00", 3);
  dataBinding[2].intField= 223322;
 
  for (i= 0; i < ROWS_TO_INSERT; ++i)
  {
    strcpy(dataBinding[i].strField, str[i]);
    dataBinding[i].indBin= 5 - i;
    dataBinding[i].indInt= 0;
    dataBinding[i].indStr= SQL_NTS;
  }

  ok_stmt(hstmt, SQLExecDirect(hstmt, "INSERT INTO t_bug48310 (bData, intField, strField) " \
    "VALUES (?,?,?)", SQL_NTS));

  is_num(paramsProcessed, ROWS_TO_INSERT);

  ok_stmt(hstmt, SQLRowCount(hstmt, &rowsCount));
  is_num(rowsCount, ROWS_TO_INSERT);

  for (i= 0; i < paramsProcessed; ++i)
    if ( paramStatusArray[i] != SQL_PARAM_SUCCESS
      && paramStatusArray[i] != SQL_PARAM_SUCCESS_WITH_INFO )
    {
      printMessage("Parameter #%u status isn't successful(0x%X)", i+1, paramStatusArray[i]);
      return FAIL;
    }

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)1, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMS_PROCESSED_PTR, NULL, 0));

  ok_stmt(hstmt, SQLExecDirect(hstmt, "SELECT bData, intField, strField\
                                      FROM t_bug48310\
                                      ORDER BY id", SQL_NTS));

  /* Just to make sure RowCount isn't broken */
  ok_stmt(hstmt, SQLRowCount(hstmt, &rowsCount));
  is_num(rowsCount, ROWS_TO_INSERT);

  for (i= 0; i < paramsProcessed; ++i)
  {
    ok_stmt(hstmt, SQLFetch(hstmt));

    ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_BINARY, (SQLPOINTER)buff, 50, &nLen));
    is(memcmp((const void*) buff, (const void*)dataBinding[i].bData, 5 - i)==0);
    is_num(my_fetch_int(hstmt, 2), dataBinding[i].intField);
    is_str(my_fetch_str(hstmt, buff, 3), dataBinding[i].strField, strlen(str[i]));
  }

  expect_stmt(hstmt,SQLFetch(hstmt), SQL_NO_DATA_FOUND);
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  /* One more check that RowCount isn't broken. check may get broken if input data
     changes */
  ok_sql(hstmt, "update t_bug48310 set strField='changed' where intField > 1");
  ok_stmt(hstmt, SQLRowCount(hstmt, &rowsCount));
  is_num(rowsCount, 1);

  /* Clean-up */
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  ok_stmt(hstmt, SQLExecDirect(hstmt, "DROP TABLE IF EXISTS bug48310", SQL_NTS));

  return OK;

#undef ROWS_TO_INSERT
#undef STR_FIELD_LENGTH
}


/*
Bug 48310 - parameters array support request.
Binding by column test
*/
DECLARE_TEST(paramarray_by_column)
{
#define ROWS_TO_INSERT 3
#define STR_FIELD_LENGTH 5
  SQLCHAR       buff[50];

  SQLCHAR       bData[ROWS_TO_INSERT][STR_FIELD_LENGTH]={{0x01, 0x80, 0x00, 0x80, 0x03},
                                          {0x02, 0x80, 0x00, 0x02},
                                          {0x03, 0x80, 0x01}};
  SQLLEN        bInd[ROWS_TO_INSERT]= {5,4,3};

  const SQLCHAR strField[ROWS_TO_INSERT][STR_FIELD_LENGTH]= {{'\0'}, {'x','\0'}, {'x','x','x','\0'} };
  SQLLEN        strInd[ROWS_TO_INSERT]= {SQL_NTS, SQL_NTS, SQL_NTS};

  SQLINTEGER    intField[ROWS_TO_INSERT] = {123321, 1, 0};
  SQLLEN        intInd[ROWS_TO_INSERT]= {5,4,3};

  SQLUSMALLINT  paramStatusArray[ROWS_TO_INSERT];
  SQLULEN       paramsProcessed, i, nLen;

  ok_stmt(hstmt, SQLExecDirect(hstmt, "DROP TABLE IF EXISTS t_bug48310", SQL_NTS));
  ok_stmt(hstmt, SQLExecDirect(hstmt, "CREATE TABLE t_bug48310 (id int primary key auto_increment,"\
    "bData binary(5) NULL, intField int not null, strField varchar(255) not null)", SQL_NTS));

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAM_BIND_TYPE, SQL_PARAM_BIND_BY_COLUMN, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)ROWS_TO_INSERT, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAM_STATUS_PTR, paramStatusArray, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMS_PROCESSED_PTR, &paramsProcessed, 0));

  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_BINARY,
    0, 0, bData, 5, bInd));
  ok_stmt(hstmt, SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
    0, 0, intField, 0, intInd));
  ok_stmt(hstmt, SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR,
    0, 0, (SQLPOINTER)strField, 5, strInd ));

  ok_stmt(hstmt, SQLExecDirect(hstmt, "INSERT INTO t_bug48310 (bData, intField, strField) " \
    "VALUES (?,?,?)", SQL_NTS));

  is_num(paramsProcessed, ROWS_TO_INSERT);

  for (i= 0; i < paramsProcessed; ++i)
    if ( paramStatusArray[i] != SQL_PARAM_SUCCESS
      && paramStatusArray[i] != SQL_PARAM_SUCCESS_WITH_INFO )
    {
      printMessage("Parameter #%u status isn't successful(0x%X)", i+1, paramStatusArray[i]);
      return FAIL;
    }

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)1, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMS_PROCESSED_PTR, NULL, 0));

  ok_stmt(hstmt, SQLExecDirect(hstmt, "SELECT bData, intField, strField\
                                       FROM t_bug48310\
                                       ORDER BY id", SQL_NTS));

  for (i= 0; i < paramsProcessed; ++i)
  {
    ok_stmt(hstmt, SQLFetch(hstmt));

    ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_BINARY, (SQLPOINTER)buff, 50, &nLen));
    if (memcmp((const void*) buff, bData[i], 5 - i)!=0)
    {
      printMessage("Bin data inserted wrongly. Read: 0x%02X%02X%02X%02X%02X Had to be: 0x%02X%02X%02X%02X%02X"
        , buff[0], buff[1], buff[2], buff[3], buff[4]
        , bData[i][0], bData[i][1], bData[i][2], bData[i][3], bData[i][4]);
      return FAIL;
    }
    is_num(my_fetch_int(hstmt, 2), intField[i]);
    is_str(my_fetch_str(hstmt, buff, 3), strField[i], strlen(strField[i]));
  }

  /* Clean-up */
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  ok_stmt(hstmt, SQLExecDirect(hstmt, "DROP TABLE IF EXISTS bug48310", SQL_NTS));

  return OK;

#undef ROWS_TO_INSERT
#undef STR_FIELD_LENGTH
}


/*
Bug 48310 - parameters array support request.
Ignore paramset test
*/
DECLARE_TEST(paramarray_ignore_paramset)
{
#define ROWS_TO_INSERT 4
#define STR_FIELD_LENGTH 5
  SQLCHAR       buff[50];

  SQLCHAR       bData[ROWS_TO_INSERT][STR_FIELD_LENGTH]={{0x01, 0x80, 0x00, 0x80, 0x03},
                                                        {0x02, 0x80, 0x00, 0x02},
                                                        {0x03, 0x80, 0x01}};
  SQLLEN        bInd[ROWS_TO_INSERT]= {5,4,3};

  const SQLCHAR strField[ROWS_TO_INSERT][STR_FIELD_LENGTH]= {{'\0'}, {'x','\0'}, {'x','x','x','\0'} };
  SQLLEN        strInd[ROWS_TO_INSERT]= {SQL_NTS, SQL_NTS, SQL_NTS};

  SQLINTEGER    intField[ROWS_TO_INSERT] = {123321, 1, 0};
  SQLLEN        intInd[ROWS_TO_INSERT]= {5,4,3};

  SQLUSMALLINT  paramOperationArr[ROWS_TO_INSERT]={0,SQL_PARAM_IGNORE,0,SQL_PARAM_IGNORE};
  SQLUSMALLINT  paramStatusArr[ROWS_TO_INSERT];
  SQLULEN       paramsProcessed, i, nLen, rowsInserted= 0;

  ok_stmt(hstmt, SQLExecDirect(hstmt, "DROP TABLE IF EXISTS t_bug48310", SQL_NTS));
  ok_stmt(hstmt, SQLExecDirect(hstmt, "CREATE TABLE t_bug48310 (id int primary key auto_increment,"\
    "bData binary(5) NULL, intField int not null, strField varchar(255) not null)", SQL_NTS));

  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAM_BIND_TYPE, SQL_PARAM_BIND_BY_COLUMN, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)ROWS_TO_INSERT, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAM_STATUS_PTR, paramStatusArr, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAM_OPERATION_PTR, paramOperationArr, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMS_PROCESSED_PTR, &paramsProcessed, 0));

  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_BINARY,
    0, 0, bData, 5, bInd));
  ok_stmt(hstmt, SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
    0, 0, intField, 0, intInd));
  ok_stmt(hstmt, SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR,
    0, 0, (SQLPOINTER)strField, 5, strInd ));

  ok_stmt(hstmt, SQLExecDirect(hstmt, "INSERT INTO t_bug48310 (bData, intField, strField) " \
    "VALUES (?,?,?)", SQL_NTS));

  is_num(paramsProcessed, ROWS_TO_INSERT);

  for (i= 0; i < paramsProcessed; ++i)
  {
    if (paramOperationArr[i] == SQL_PARAM_IGNORE)
    {
      is_num(paramStatusArr[i], SQL_PARAM_UNUSED);
    }
    else if ( paramStatusArr[i] != SQL_PARAM_SUCCESS
      && paramStatusArr[i] != SQL_PARAM_SUCCESS_WITH_INFO )
    {
      printMessage("Parameter #%u status isn't successful(0x%X)", i+1, paramStatusArr[i]);
      return FAIL;
    }
  }

  /* Resetting statements attributes */
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)1, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMS_PROCESSED_PTR, NULL, 0));

  ok_stmt(hstmt, SQLExecDirect(hstmt, "SELECT bData, intField, strField\
                                      FROM t_bug48310\
                                      ORDER BY id", SQL_NTS));

  i= 0;
  while(i < paramsProcessed)
  {
    if (paramStatusArr[i] == SQL_PARAM_UNUSED)
    {
      ++i;
      continue;
    }

    ok_stmt(hstmt, SQLFetch(hstmt));

    ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_BINARY, (SQLPOINTER)buff, 50, &nLen));

    if (memcmp((const void*) buff, bData[i], 5 - i)!=0)
    {
      printMessage("Bin data inserted wrongly. Read: 0x%02X%02X%02X%02X%02X Had to be: 0x%02X%02X%02X%02X%02X"
        , buff[0], buff[1], buff[2], buff[3], buff[4]
      , bData[i][0], bData[i][1], bData[i][2], bData[i][3], bData[i][4]);
      return FAIL;
    }
    is_num(my_fetch_int(hstmt, 2), intField[i]);
    is_str(my_fetch_str(hstmt, buff, 3), strField[i], strlen(strField[i]));

    ++rowsInserted;
    ++i;
  }

  /* Making sure that there is nothing else to fetch ... */
  expect_stmt(hstmt,SQLFetch(hstmt), SQL_NO_DATA_FOUND);

  /* ... and that inserted was less than SQL_ATTR_PARAMSET_SIZE rows */
  is( rowsInserted < ROWS_TO_INSERT);
  
  /* Clean-up */
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  ok_stmt(hstmt, SQLExecDirect(hstmt, "DROP TABLE IF EXISTS bug48310", SQL_NTS));

  return OK;

#undef ROWS_TO_INSERT
#undef STR_FIELD_LENGTH
}


/*
  Bug 48310 - parameters array support request.
  Select statement.
*/
DECLARE_TEST(paramarray_select)
{
#define STMTS_TO_EXEC 3

  SQLINTEGER    intField[STMTS_TO_EXEC] = {3, 1, 2};
  SQLLEN        intInd[STMTS_TO_EXEC]= {5,4,3};

  SQLUSMALLINT  paramStatusArray[STMTS_TO_EXEC];
  SQLULEN       paramsProcessed, i;


  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAM_BIND_TYPE, SQL_PARAM_BIND_BY_COLUMN, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)STMTS_TO_EXEC, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAM_STATUS_PTR, paramStatusArray, 0));
  ok_stmt(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMS_PROCESSED_PTR, &paramsProcessed, 0));

  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
    0, 0, intField, 0, intInd));

  ok_stmt(hstmt, SQLExecDirect(hstmt, "SELect ?,'So what'", SQL_NTS));

  is_num(paramsProcessed, STMTS_TO_EXEC);

  for (i= 0; i < paramsProcessed; ++i)
  {
    if ( paramStatusArray[i] != SQL_PARAM_SUCCESS
      && paramStatusArray[i] != SQL_PARAM_SUCCESS_WITH_INFO )
    {
      printMessage("Parameter #%u status isn't successful(0x%X)", i+1, paramStatusArray[i]);
      return FAIL;
    }

    ok_stmt(hstmt, SQLFetch(hstmt));

    is_num(my_fetch_int(hstmt, 1), intField[i]);
  }

  /* Clean-up */
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  return OK;

#undef STMTS_TO_EXEC
}


/*
  Bug #49029 - Server with sql mode NO_BACKSLASHES_ESCAPE obviously
  can work incorrectly (at least) with binary parameters
*/
DECLARE_TEST(t_bug49029)
{
  const SQLCHAR bData[6]= "\x01\x80\x00\x80\x01", buff[6];
  SQLULEN len= 5;

  ok_stmt(hstmt, SQLExecDirect(hstmt, "set @@session.sql_mode='NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION,NO_BACKSLASH_ESCAPES'", SQL_NTS));

  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_BINARY,
    0, 0, (SQLCHAR*)bData, 0, &len));

  ok_stmt(hstmt, SQLExecDirect(hstmt, "select ?", SQL_NTS));

  ok_stmt(hstmt, SQLFetch(hstmt));
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_BINARY, (SQLPOINTER)buff, 6, &len));

  is(memcmp((const void*) buff, (const void*)bData, 5)==0);

  return OK;
}


BEGIN_TESTS
  ADD_TEST(my_init_table)
  ADD_TEST(my_param_insert)
  ADD_TEST(my_param_update)
  ADD_TEST(my_param_delete)
  ADD_TEST(tmysql_fix)
  ADD_TEST(t_param_offset)
  ADD_TEST(paramarray_by_row)
  ADD_TEST(paramarray_by_column)
  ADD_TEST(paramarray_ignore_paramset)
  ADD_TEST(paramarray_select)
  ADD_TEST(t_bug49029)
END_TESTS


RUN_TESTS
