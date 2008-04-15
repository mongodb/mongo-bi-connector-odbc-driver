/*
  Copyright (C) 1995-2007 MySQL AB

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


BEGIN_TESTS
  ADD_TEST(my_init_table)
  ADD_TEST(my_param_insert)
  ADD_TEST(my_param_update)
  ADD_TEST(my_param_delete)
  ADD_TEST(tmysql_fix)
  ADD_TEST(t_param_offset)
END_TESTS


RUN_TESTS
