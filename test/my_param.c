/*
  Copyright (c) 2003, 2012, Oracle and/or its affiliates. All rights reserved.

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
                            timestamp timestamp)");

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


/*I really wonder what is this test about */
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
                ) ENGINE=MyISAM");

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
    rc = SQLPrepare(hstmt,
      (SQLCHAR *)"insert into tmysql_err (TD, NODE, TAG, SQLNAME, SQL_ERR,"
                 "FIX_ERR, PROG_ERR) values (?, ?, ?, ?, ?, ?, ?)", 103);
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

    /* TODO: C and SQL types as numeric consts. Splendid.*/
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

  /* We don't expect errors in paramsets processing, thus we should get SQL_SUCCESS only*/
  expect_stmt(hstmt, SQLExecDirect(hstmt, "INSERT INTO t_bug48310 (bData, intField, strField) " \
    "VALUES (?,?,?)", SQL_NTS), SQL_SUCCESS);

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

  /* We don't expect errors in paramsets processing, thus we should get SQL_SUCCESS only*/
  expect_stmt(hstmt, SQLExecDirect(hstmt, "INSERT INTO t_bug48310 (bData, intField, strField) " \
    "VALUES (?,?,?)", SQL_NTS), SQL_SUCCESS);

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

  /* We don't expect errors in paramsets processing, thus we should get SQL_SUCCESS only*/
  expect_stmt(hstmt, SQLExecDirect(hstmt, "INSERT INTO t_bug48310 (bData, intField, strField) " \
    "VALUES (?,?,?)", SQL_NTS), SQL_SUCCESS);

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

  /* We don't expect errors in paramsets processing, thus we should get SQL_SUCCESS only*/
  expect_stmt(hstmt, SQLExecDirect(hstmt, "SELect ?,'So what'", SQL_NTS), SQL_SUCCESS);
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
  const SQLCHAR bData[6]= "\x01\x80\x00\x80\x01";
  SQLCHAR buff[6];
  SQLULEN len= 5;

  ok_stmt(hstmt, SQLExecDirect(hstmt, "set @@session.sql_mode='NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION,NO_BACKSLASH_ESCAPES'", SQL_NTS));

  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_BINARY,
    0, 0, (SQLPOINTER)bData, 0, &len));

  ok_stmt(hstmt, SQLExecDirect(hstmt, "select ?", SQL_NTS));

  ok_stmt(hstmt, SQLFetch(hstmt));
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_BINARY, (SQLPOINTER)buff, 6, &len));

  is(memcmp((const void*) buff, (const void*)bData, 5)==0);

  return OK;
}


/*
  Bug #56804 - Server with sql mode NO_BACKSLASHES_ESCAPE obviously
  can work incorrectly (at least) with binary parameters
*/
DECLARE_TEST(t_bug56804)
{
#define PARAMSET_SIZE		10

  SQLINTEGER	len 	= 1;
  int i;

  SQLINTEGER	c1[PARAMSET_SIZE]=      {0, 1, 2, 3, 4, 5, 1, 7, 8, 9};
  SQLINTEGER	c2[PARAMSET_SIZE]=      {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
  SQLLEN      d1[PARAMSET_SIZE]=      {4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
  SQLLEN      d2[PARAMSET_SIZE]=      {4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
  SQLUSMALLINT status[PARAMSET_SIZE]= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  SQLSMALLINT	paramset_size	= PARAMSET_SIZE;

  ok_sql(hstmt, "DROP TABLE IF EXISTS bug56804");
  ok_sql(hstmt, "create table bug56804 (c1 int primary key not null, c2 int)");
  ok_sql(hstmt, "insert into bug56804 values( 1, 1 ), (9, 9009)");

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  ok_stmt(hstmt, SQLPrepare(hstmt, (SQLCHAR *)"insert into bug56804 values( ?,? )", SQL_NTS));

  ok_stmt(hstmt, SQLSetStmtAttr( hstmt, SQL_ATTR_PARAMSET_SIZE,
    (SQLPOINTER)paramset_size, SQL_IS_UINTEGER ));

  ok_stmt(hstmt, SQLSetStmtAttr( hstmt, SQL_ATTR_PARAM_STATUS_PTR,
    status, SQL_IS_POINTER ));

  ok_stmt(hstmt, SQLBindParameter( hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG,
    SQL_DECIMAL, 4, 0, c1, 4, d1));

  ok_stmt(hstmt, SQLBindParameter( hstmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG,
    SQL_DECIMAL, 4, 0, c2, 4, d2));

  expect_stmt(hstmt, SQLExecute(hstmt), SQL_SUCCESS_WITH_INFO);

  /* Following tests are here to ensure that driver works how it is currently
     expected to work, and they need to be changed if driver changes smth in the
     way how it reports errors in paramsets and diagnostics */
  for(i = 0; i < PARAMSET_SIZE; ++i )
  {
    printMessage("Paramset #%d (%d, %d)", i, c1[i], c2[i]);
    switch (i)
    {
    case 1:
    case 6:
      /* all errors but last have SQL_PARAM_DIAG_UNAVAILABLE */
      is_num(status[i], SQL_PARAM_DIAG_UNAVAILABLE);
      break;
    case 9:
      /* Last error -  we are supposed to get SQL_PARAM_ERROR for it */
      is_num(status[i], SQL_PARAM_ERROR);
      break;
    default:
      is_num(status[i], SQL_PARAM_SUCCESS);
    }
  }

  {
    SQLCHAR     sqlstate[6]= {0};
    SQLCHAR     message[255]= {0};
    SQLINTEGER  native_err= 0;
    SQLSMALLINT msglen= 0;

    i= 0;
    while(SQL_SUCCEEDED(SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, ++i, sqlstate,
      &native_err, message, sizeof(message), &msglen)))
    {
      printMessage("%d) [%s] %s %d", i, sqlstate, message, native_err);
    }

    /* just to make sure we got 1 diagnostics record ... */
    is_num(i, 2);
    /* ... and what the record is for the last error */
    is(strstr(message, "Duplicate entry '9'"));
  }

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  ok_sql(hstmt, "DROP TABLE IF EXISTS bug56804");

  return OK;
#undef PARAMSET_SIZE
}


/*
  Bug 59772 - Column parameter binding makes SQLExecute not to return
  SQL_ERROR on disconnect
*/
DECLARE_TEST(t_bug59772)
{
#define ROWS_TO_INSERT 3

    SQLRETURN rc;
    SQLCHAR   buf_kill[50];

    SQLINTEGER    intField[ROWS_TO_INSERT] = {123321, 1, 0};
    SQLLEN        intInd[ROWS_TO_INSERT]= {5,4,3};

    SQLUSMALLINT  paramStatusArray[ROWS_TO_INSERT];
    SQLULEN       paramsProcessed, i;

    SQLINTEGER connection_id;

    SQLHENV henv2;
    SQLHDBC  hdbc2;
    SQLHSTMT hstmt2;

    int overall_result= OK;

    /* Create a new connection that we deliberately will kill */
    alloc_basic_handles(&henv2, &hdbc2, &hstmt2);

    ok_sql(hstmt2, "SELECT connection_id()");
    ok_stmt(hstmt2, SQLFetch(hstmt2));
    connection_id= my_fetch_int(hstmt2, 1);
    ok_stmt(hstmt2, SQLFreeStmt(hstmt2, SQL_CLOSE));

    ok_stmt(hstmt, SQLExecDirect(hstmt, "DROP TABLE IF EXISTS t_bug59772", SQL_NTS));
    ok_stmt(hstmt, SQLExecDirect(hstmt, "CREATE TABLE t_bug59772 (id int primary key auto_increment,"\
      "intField int)", SQL_NTS));

    ok_stmt(hstmt2, SQLSetStmtAttr(hstmt2, SQL_ATTR_PARAM_BIND_TYPE, SQL_PARAM_BIND_BY_COLUMN, 0));
    ok_stmt(hstmt2, SQLSetStmtAttr(hstmt2, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)ROWS_TO_INSERT, 0));
    ok_stmt(hstmt2, SQLSetStmtAttr(hstmt2, SQL_ATTR_PARAM_STATUS_PTR, paramStatusArray, 0));
    ok_stmt(hstmt2, SQLSetStmtAttr(hstmt2, SQL_ATTR_PARAMS_PROCESSED_PTR, &paramsProcessed, 0));

    ok_stmt(hstmt2, SQLPrepare(hstmt2, "INSERT INTO t_bug59772 (intField) VALUES (?)", SQL_NTS));

    ok_stmt(hstmt2, SQLBindParameter(hstmt2, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
      0, 0, intField, 0, intInd));

    /* From another connection, kill the connection created above */
    sprintf(buf_kill, "KILL %d", connection_id);
    ok_stmt(hstmt, SQLExecDirect(hstmt, (SQLCHAR *)buf_kill, SQL_NTS));

    rc= SQLExecute(hstmt2);
    
    /* The result should be SQL_ERROR */
    if (rc != SQL_ERROR)
      overall_result= FAIL;

    for (i= 0; i < paramsProcessed; ++i)

      /* We expect error statuses for all parameters */
      if ( paramStatusArray[i] != ((i + 1 < ROWS_TO_INSERT) ? 
            SQL_PARAM_DIAG_UNAVAILABLE : SQL_PARAM_ERROR) )
      {
        printMessage("Parameter #%u status isn't successful(0x%X)", i+1, paramStatusArray[i]);
        overall_result= FAIL;
      }

    ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
    ok_stmt(hstmt, SQLExecDirect(hstmt, "DROP TABLE IF EXISTS t_bug59772", SQL_NTS));

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt2);
    SQLDisconnect(hdbc2);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc2);
    SQLFreeHandle(SQL_HANDLE_ENV, henv2);

    return overall_result;
#undef ROWS_TO_INSERT
}


DECLARE_TEST(t_odbcoutparams)
{
  SQLSMALLINT ncol, i;
  SQLINTEGER  par[3]= {10, 20, 30}, val;
  SQLLEN      len;
  SQLSMALLINT type[]= {SQL_PARAM_INPUT, SQL_PARAM_OUTPUT, SQL_PARAM_INPUT_OUTPUT};
  SQLCHAR     str[20]= "initial value", buff[20];

  ok_sql(hstmt, "DROP PROCEDURE IF EXISTS t_odbcoutparams");
  ok_sql(hstmt, "CREATE PROCEDURE t_odbcoutparams("
                "  IN p_in INT, "
                "  OUT p_out INT, "
                "  INOUT p_inout INT) "
                "BEGIN "
                "  SET p_in = p_in*10, p_out = (p_in+p_inout)*10, p_inout = p_inout*10; "
                "END");



  for (i=0; i < sizeof(par)/sizeof(SQLINTEGER); ++i)
  {
    ok_stmt(hstmt, SQLBindParameter(hstmt, i+1, type[i], SQL_C_LONG, SQL_INTEGER, 0,
      0, &par[i], 0, NULL));
  }

  ok_sql(hstmt, "CALL t_odbcoutparams(?, ?, ?)");

  ok_stmt(hstmt, SQLNumResultCols(hstmt,&ncol));
  is_num(ncol, 2);

  is_num(par[1], 1300);
  is_num(par[2], 300);
  
  /* Only 1 row always - we still can get them as a result */
  ok_stmt(hstmt, SQLFetch(hstmt));
  is_num(my_fetch_int(hstmt, 1), 1300);
  is_num(my_fetch_int(hstmt, 2), 300);
  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP PROCEDURE t_odbcoutparams");
  ok_sql(hstmt, "CREATE PROCEDURE t_odbcoutparams("
                "  IN p_in INT, "
                "  OUT p_out INT, "
                "  INOUT p_inout INT) "
                "BEGIN "
                "  SELECT p_in, p_out, p_inout; "
                "  SET p_in = 300, p_out = 100, p_inout = 200; "
                "END");
  ok_sql(hstmt, "CALL t_odbcoutparams(?, ?, ?)");
  /* rs-1 */
  ok_stmt(hstmt, SQLFetch(hstmt));
  ok_stmt(hstmt, SQLNumResultCols(hstmt,&ncol));
  is_num(ncol, 3);

  is_num(my_fetch_int(hstmt, 1), 10);
  /* p_out does not have value at the moment */
  ok_stmt(hstmt, SQLGetData(hstmt, 2, SQL_INTEGER, &val, 0, &len));
  is_num(len, SQL_NULL_DATA);
  is_num(my_fetch_int(hstmt, 3), 300);

  expect_stmt(hstmt, SQLFetch(hstmt), SQL_NO_DATA);
  ok_stmt(hstmt, SQLMoreResults(hstmt));

  is_num(par[1], 100);
  is_num(par[2], 200);

  /* SP execution status */
  ok_stmt(hstmt, SQLMoreResults(hstmt));

  expect_stmt(hstmt, SQLMoreResults(hstmt), SQL_NO_DATA);

  ok_sql(hstmt, "DROP PROCEDURE t_odbcoutparams");
  ok_sql(hstmt, "CREATE PROCEDURE t_odbcoutparams("
                "  OUT p_out VARCHAR(19), "
                "  IN p_in INT, "
                "  INOUT p_inout INT) "
                "BEGIN "
                "  SET p_in = 300, p_out := 'This is OUT param', p_inout = 200; "
                "  SELECT p_inout, p_in, substring(p_out, 9);"
                "END");
  
  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_OUTPUT, SQL_C_CHAR, SQL_VARCHAR, 0,
      0, str, sizeof(str)/sizeof(SQLCHAR), NULL));
  ok_stmt(hstmt, SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0,
      0, &par[0], 0, NULL));
  ok_stmt(hstmt, SQLBindParameter(hstmt, 3, SQL_PARAM_INPUT_OUTPUT, SQL_C_LONG,
      SQL_INTEGER, 0, 0, &par[1], 0, NULL));

  ok_sql(hstmt, "CALL t_odbcoutparams(?, ?, ?)");
  /* rs-1 */
  ok_stmt(hstmt, SQLNumResultCols(hstmt,&ncol));
  is_num(ncol, 3);
  
  ok_stmt(hstmt, SQLFetch(hstmt));
  is_num(my_fetch_int(hstmt, 1), 200);
  is_num(my_fetch_int(hstmt, 2), 300);
  is_str(my_fetch_str(hstmt, buff, 3), "OUT param", 10);

  ok_stmt(hstmt, SQLMoreResults(hstmt));
  is_str(str, "This is OUT param", 18);
  is_num(par[1], 200);

  ok_stmt(hstmt, SQLFetch(hstmt));
  is_str(my_fetch_str(hstmt, buff, 1), "This is OUT param", 18);
  is_num(my_fetch_int(hstmt, 2), 200);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP PROCEDURE t_odbcoutparams");

  return OK;
}


DECLARE_TEST(t_bug14501952)
{
  SQLSMALLINT ncol;
  SQLLEN      len= 0;
  SQLCHAR     blobValue[50]= "initial value", buff[100];

  ok_sql(hstmt, "DROP PROCEDURE IF EXISTS bug14501952");
  ok_sql(hstmt, "CREATE PROCEDURE bug14501952 (INOUT param1 BLOB)\
                  BEGIN\
                    SET param1= 'this is blob value from SP ';\
                  END;");



  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT_OUTPUT,
    SQL_C_BINARY, SQL_LONGVARBINARY, 50, 0, &blobValue, sizeof(blobValue),
    &len));

  ok_sql(hstmt, "CALL bug14501952(?)");

  is_str(blobValue, "this is blob value from SP ", 27);
  ok_stmt(hstmt, SQLNumResultCols(hstmt,&ncol));
  is_num(ncol, 1);

  /* Only 1 row always - we still can get them as a result */
  ok_stmt(hstmt, SQLFetch(hstmt));
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_BINARY, buff, sizeof(buff),
                            &len));
  is_str(buff, blobValue, 27);
  
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP PROCEDURE bug14501952");

  return OK;
}


/* Bug#14563386 More than one BLOB(or any big data types) OUT param caused crash
 */
DECLARE_TEST(t_bug14563386)
{
  SQLSMALLINT ncol;
  SQLLEN      len= 0, len1= 0;
  SQLCHAR     blobValue[50]= "initial value", buff[100],
              binValue[50]= "varbinary init value";

  ok_sql(hstmt, "DROP PROCEDURE IF EXISTS b14563386");
  ok_sql(hstmt, "CREATE PROCEDURE b14563386 (INOUT blob_param \
                        BLOB, INOUT bin_param LONG VARBINARY)\
                  BEGIN\
                    SET blob_param = ' BLOB! ';\
                    SET bin_param = ' LONG VARBINARY ';\
                  END;");



  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT_OUTPUT,
    SQL_C_BINARY, SQL_LONGVARBINARY, 50, 0, &blobValue, sizeof(blobValue),
    &len));
  ok_stmt(hstmt, SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT_OUTPUT,
    SQL_C_BINARY, SQL_LONGVARBINARY, 50, 0, &binValue, sizeof(binValue),
    &len1));
  ok_sql(hstmt, "CALL b14563386(?, ?)");

  is_str(blobValue, " BLOB! ", 7);
  is_str(binValue, " LONG VARBINARY ", 16);
  ok_stmt(hstmt, SQLNumResultCols(hstmt,&ncol));
  is_num(ncol, 2);

  /* Only 1 row always - we still can get them as a result */
  ok_stmt(hstmt, SQLFetch(hstmt));
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_BINARY, buff, sizeof(buff),
                            &len));
  is_str(buff, blobValue, 7);
  ok_stmt(hstmt, SQLGetData(hstmt, 2, SQL_C_BINARY, buff, sizeof(buff),
                            &len));
  is_str(buff, binValue, 16);
  
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP PROCEDURE b14563386");

  return OK;
}


/* Bug#14551229(could not repeat) Procedure with signed out parameter */
DECLARE_TEST(t_bug14551229)
{
  SQLINTEGER param, value;

  ok_sql(hstmt, "DROP PROCEDURE IF EXISTS b14551229");
  ok_sql(hstmt, "CREATE PROCEDURE b14551229 (OUT param INT)\
                  BEGIN\
                    SELECT -1 into param from dual;\
                  END;");



  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_OUTPUT,
    SQL_C_SLONG, SQL_INTEGER, 50, 0, &param, 0, 0));

  ok_sql(hstmt, "CALL b14551229(?)");

  is_num(param, -1);

  /* Only 1 row always - we still can get them as a result */
  ok_stmt(hstmt, SQLFetch(hstmt));
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_SLONG, &value, 0, 0));
  is_num(value, -1);
  
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP PROCEDURE b14551229");

  return OK;
}


/* Bug#14560916 ASSERT for INOUT parameter of BIT(10) type */
DECLARE_TEST(t_bug14560916)
{
  char        param[2]={1,1};
  SQLINTEGER  value;
  SQLLEN      len= 0;

  ok_sql(hstmt, "DROP PROCEDURE IF EXISTS b14560916");

  ok_sql(hstmt, "DROP TABLE IF EXISTS bug14560916");
  ok_sql(hstmt, "CREATE TABLE bug14560916 (a BIT(10))");
  ok_sql(hstmt, "INSERT INTO bug14560916  values(b'1001000001')");

  ok_sql(hstmt, "CREATE PROCEDURE b14560916 (INOUT param bit(10))\
                  BEGIN\
                    SELECT a INTO param FROM bug14560916;\
                  END;");



  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT_OUTPUT,
    SQL_C_CHAR, SQL_CHAR, 0, 0, &param, sizeof(param), &len));

  /* Parameter is used to make sure that ssps will be used */
  ok_sql(hstmt, "select a from bug14560916 where ? OR 1");
  ok_stmt(hstmt, SQLFetch(hstmt));
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_SLONG, &value, 0, 0));
  is_num(value, 577);
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_BINARY, &param, sizeof(param),
                            &len));
  is_num(len, 2);
  is_str(param, "\2A", 2);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));
  

  ok_sql(hstmt, "CALL b14560916(?)");

  is_num(len, 2);
  is_str(param, "\2A", 2);

  /* Only 1 row always - we still can get them as a result */
  ok_stmt(hstmt, SQLFetch(hstmt));
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_SLONG, &value, 0, 0));
  is_num(value, 577);
  param[0]= param[1]= 1;
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_BINARY, &param, sizeof(param),
                            &len));
  is_num(len, 2);
  is_str(param, "\2A", 2);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP PROCEDURE b14560916");

  return OK;
}


/* Bug#14586094 Crash while executing SP having blob and varchar OUT parameters
  (could not repeat)
 */
DECLARE_TEST(t_bug14586094)
{
  SQLSMALLINT ncol;
  SQLLEN      len= SQL_NTS, len1= SQL_NTS;
  SQLCHAR     blobValue[50]= {0}/*"initial value"*/, buff[101],
              vcValue[101]= "varchar init value";

  ok_sql(hstmt, "DROP PROCEDURE IF EXISTS b14586094");
  ok_sql(hstmt, "CREATE PROCEDURE b14586094 (INOUT blob_param \
                    BLOB(50), INOUT vc_param VARCHAR(100))\
                  BEGIN\
                    SET blob_param = ' BLOB! ';\
                    SET vc_param = 'varchar';\
                  END;");



  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT_OUTPUT,
    SQL_C_BINARY, SQL_LONGVARBINARY, 50, 0, &blobValue, sizeof(blobValue),
    &len));
  ok_stmt(hstmt, SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT_OUTPUT,
    SQL_C_CHAR, SQL_VARCHAR, 50, 0, &vcValue, sizeof(vcValue), &len1));
  ok_sql(hstmt, "CALL b14586094(?, ?)");

  is_str(blobValue, " BLOB! ", 7);
  is_str(vcValue, "varchar", 9);
  ok_stmt(hstmt, SQLNumResultCols(hstmt,&ncol));
  is_num(ncol, 2);

  /* Only 1 row always - we still can get them as a result */
  ok_stmt(hstmt, SQLFetch(hstmt));
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_BINARY, buff, sizeof(buff),
                            &len));
  is_str(buff, blobValue, 7);
  ok_stmt(hstmt, SQLGetData(hstmt, 2, SQL_C_CHAR, buff, sizeof(buff),
                            &len));
  is_str(buff, vcValue, 9);
  
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP PROCEDURE b14586094");

  return OK;
}


DECLARE_TEST(t_longtextoutparam)
{
  SQLSMALLINT ncol;
  SQLLEN      len= 0;
  SQLCHAR     blobValue[50]= "initial value", buff[100];

  ok_sql(hstmt, "DROP PROCEDURE IF EXISTS t_longtextoutparam");
  ok_sql(hstmt, "CREATE PROCEDURE t_longtextoutparam (INOUT param1 LONGTEXT)\
                  BEGIN\
                    SET param1= 'this is LONGTEXT value from SP ';\
                  END;");



  ok_stmt(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT_OUTPUT,
    SQL_C_BINARY, SQL_LONGVARBINARY, 50, 0, &blobValue, sizeof(blobValue),
    &len));

  ok_sql(hstmt, "CALL t_longtextoutparam(?)");

  is_str(blobValue, "this is LONGTEXT value from SP ", 32);
  ok_stmt(hstmt, SQLNumResultCols(hstmt,&ncol));
  is_num(ncol, 1);

  /* Only 1 row always - we still can get them as a result */
  ok_stmt(hstmt, SQLFetch(hstmt));
  ok_stmt(hstmt, SQLGetData(hstmt, 1, SQL_C_CHAR, buff, sizeof(buff),
                            &len));
  is_str(buff, blobValue, 32);
  
  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_sql(hstmt, "DROP PROCEDURE t_longtextoutparam");

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
  ADD_TEST(t_bug56804)
  ADD_TEST(t_bug59772)
  ADD_TEST(t_odbcoutparams)
  ADD_TEST(t_bug14501952)
  ADD_TEST(t_bug14563386)
  ADD_TEST(t_bug14551229)
  ADD_TEST(t_bug14560916)
  ADD_TEST(t_bug14586094)
  ADD_TEST(t_longtextoutparam)
END_TESTS


RUN_TESTS
