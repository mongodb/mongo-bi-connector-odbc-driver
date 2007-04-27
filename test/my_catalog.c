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

DECLARE_TEST(my_columns_null)
{
    SQLRETURN   rc;

    /* initialize data */
    SQLExecDirect(hstmt,"drop table my_column_null",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table my_column_null(id int not null, name varchar(30))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLColumns(hstmt,NULL,SQL_NTS,NULL,SQL_NTS,"my_column_null",SQL_NTS,NULL,SQL_NTS);
    mystmt(hstmt,rc);

    myassert(2 == my_print_non_format_result(hstmt));

    SQLFreeStmt(hstmt, SQL_CLOSE);

  return OK;
}


DECLARE_TEST(my_drop_table)
{
    SQLRETURN   rc;

    /* initialize data */
    SQLExecDirect(hstmt,"drop table my_drop_table",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table my_drop_table(id int not null)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLColumns(hstmt,NULL,0,NULL,0,"my_drop_table",SQL_NTS,NULL,0);
    mystmt(hstmt,rc);

    myassert(1 == my_print_non_format_result(hstmt));

    rc = SQLExecDirect(hstmt,"drop table my_drop_table",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(my_table_priv)
{
    SQLRETURN   rc;

    rc = SQLTablePrivileges(hstmt,"te%",SQL_NTS,
                            NULL,0,"my_%",SQL_NTS);
    mystmt(hstmt,rc);

    my_print_non_format_result(hstmt);

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_CLOSE);

  return OK;
}


void check_sqlstate(SQLHSTMT hstmt,SQLCHAR *sqlstate)
{
    SQLCHAR     sql_state[6];
    SQLINTEGER    err_code=0;
    SQLCHAR         err_msg[SQL_MAX_MESSAGE_LENGTH]={0};
    SQLSMALLINT   err_len=0;

    memset(err_msg,'C',SQL_MAX_MESSAGE_LENGTH);
    SQLGetDiagRec(SQL_HANDLE_STMT,hstmt,1,
                  (SQLCHAR *)&sql_state,(SQLINTEGER *)&err_code,
                  (SQLCHAR*)&err_msg, SQL_MAX_MESSAGE_LENGTH-1,
                  (SQLSMALLINT *)&err_len);

    printMessage("\n\t ERROR: %s\n",err_msg);
    printMessage("\n SQLSTATE (expected:%s, obtained:%s)\n",sqlstate,sql_state);
    myassert(strcmp(sql_state,sqlstate)==0);

}
#define TODBC_BIND_CHAR(n,buf) SQLBindCol(hstmt,n,SQL_C_CHAR,&buf,sizeof(buf),NULL);


DECLARE_TEST(my_table_dbs)
{
    SQLCHAR    database[100];
    SQLRETURN  rc;
    SQLINTEGER nrows;

    SQLExecDirect(hstmt, "DROP DATABASE my_all_db_test1",   SQL_NTS);
    SQLExecDirect(hstmt, "DROP DATABASE my_all_db_test2",   SQL_NTS);
    SQLExecDirect(hstmt, "DROP DATABASE my_all_db_test3",   SQL_NTS);
    SQLExecDirect(hstmt, "DROP DATABASE my_all_db_test4",   SQL_NTS);

    rc = SQLTables(hstmt,"%",1,NULL,0,NULL,0,NULL,0);
    mystmt(hstmt,rc);

    nrows = my_print_non_format_result(hstmt);
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTables(hstmt,"SQL_ALL_CATALOGS",SQL_NTS,NULL,0,NULL,0,NULL,0);
    mystmt(hstmt,rc);

    assert(nrows == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTables(hstmt,"test",4,NULL,0,NULL,0,NULL,0);
    mystmt(hstmt,rc);

    my_print_non_format_result(hstmt);
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTables(hstmt,"mysql",5,NULL,0,NULL,0,NULL,0);
    mystmt(hstmt,rc);

    assert(my_print_non_format_result(hstmt) != 0);
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTables(hstmt,"%",1,NULL,0,NULL,0,NULL,0);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    memset(database,0,100);
    rc = SQLGetData(hstmt,1,SQL_C_CHAR,(SQLCHAR *)&database,100,NULL);
    mystmt(hstmt,rc);
    printMessage("\n database: %s", database);

    memset(database,0,100);
    rc = SQLGetData(hstmt,2,SQL_C_CHAR,(SQLCHAR *)&database,100,NULL);
    mystmt(hstmt,rc);
    printMessage("\n table: %s", database); 
    myassert(strcmp(database,"")==0);

    memset(database,0,100);
    rc = SQLGetData(hstmt,3,SQL_C_CHAR,(SQLCHAR *)&database,100,NULL);
    mystmt(hstmt,rc);
    printMessage("\n table: %s", database); 
    myassert(strcmp(database,"")==0);

    memset(database,0,100);
    rc = SQLGetData(hstmt,4,SQL_C_CHAR,(SQLCHAR *)&database,100,NULL);
    mystmt(hstmt,rc);
    printMessage("\n table: %s", database); 
    myassert(strcmp(database,"")==0);

    memset(database,0,100);
    rc = SQLGetData(hstmt,5,SQL_C_CHAR, (SQLCHAR*)&database,100,NULL);
    mystmt(hstmt,rc);
    printMessage("\n database remark: %s", database);
    myassert(strcmp(database,"")==0);

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLExecDirect(hstmt, "CREATE DATABASE my_all_db_test1",    SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt, "CREATE DATABASE my_all_db_test2",    SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt, "CREATE DATABASE my_all_db_test3",    SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt, "CREATE DATABASE my_all_db_test4",    SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLTables(hstmt,"%",1,NULL,0,NULL,0,NULL,0);
    mystmt(hstmt,rc);

    nrows += 4;
    assert(nrows == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTables(hstmt,"SQL_ALL_CATALOGS",SQL_NTS,NULL,0,NULL,0,NULL,0);
    mystmt(hstmt,rc);

    assert(nrows == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTables(hstmt,"my_all_db_test",SQL_NTS,NULL,0,NULL,0,NULL,0);
    mystmt(hstmt,rc);

    assert(0 == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTables(hstmt,"my_all_db_test%",SQL_NTS,NULL,0,NULL,0,NULL,0);
    mystmt(hstmt,rc);

    assert(0 == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt, "DROP DATABASE my_all_db_test1",  SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt, "DROP DATABASE my_all_db_test2",  SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt, "DROP DATABASE my_all_db_test3",  SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt, "DROP DATABASE my_all_db_test4",  SQL_NTS);
    mystmt(hstmt,rc);

  return OK;
}


void my_colpriv_init(SQLHDBC hdbc,SQLHSTMT hstmt)
{
    SQLRETURN rc;

    SQLExecDirect( hstmt, "DROP TABLE test_colprev1",SQL_NTS);
    SQLExecDirect( hstmt, "DROP TABLE test_colprev2",SQL_NTS);
    SQLExecDirect( hstmt, "DROP TABLE test_colprev3",SQL_NTS);

    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLExecDirect(hstmt, "CREATE TABLE test_colprev1(a INT,b INT,c INT, d INT)",SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt, "CREATE TABLE test_colprev2(a INT,b INT,c INT, d INT)",   SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt, "CREATE TABLE test_colprev3(a INT,b INT,c INT, d INT)",   SQL_NTS);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt, SQL_CLOSE);

    SQLExecDirect(hstmt, "DELETE FROM mysql.columns_priv where USER='my_colpriv'",SQL_NTS); 
    rc = SQLExecDirect(hstmt, "GRANT SELECT(a,b),INSERT(d), UPDATE(c) ON test_colprev1 TO my_colpriv",SQL_NTS);     
    mystmt(hstmt,rc);
    rc = SQLExecDirect( hstmt, "GRANT SELECT(c,a),UPDATE(a,b) ON test_colprev3 TO my_colpriv",  SQL_NTS);     
    mystmt(hstmt,rc);

    SQLExecDirect(  hstmt, "FLUSH PRIVILEGES",  SQL_NTS);
    SQLFreeStmt(hstmt, SQL_CLOSE);  
}


bool my_tablepriv_init(SQLHDBC hdbc,SQLHSTMT hstmt)
{
    SQLRETURN rc;

    SQLExecDirect( hstmt, "DROP TABLE test_tabprev1",SQL_NTS);
    SQLExecDirect( hstmt, "DROP TABLE test_tabprev2",SQL_NTS);
    SQLExecDirect( hstmt, "DROP TABLE test_tabprev3",SQL_NTS);

    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLExecDirect(hstmt, "CREATE TABLE test_tabprev1(f1 INT)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt, "CREATE TABLE test_tabprev2(f1 INT)", SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt, "CREATE TABLE test_tabprev3(f1 INT)", SQL_NTS);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt, SQL_CLOSE);

    SQLExecDirect(hstmt, "DELETE FROM mysql.tables_priv where USER='my_tabpriv'",SQL_NTS);

    SQLExecDirect(hstmt, "GRANT CONNECT TO my_tabpriv IDENTIFIED BY my_tabpriv",SQL_NTS);/* sybase */

    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLExecDirect(hstmt, "GRANT SELECT,INSERT ON test_tabprev1 TO my_tabpriv",SQL_NTS);
    if (rc == SQL_ERROR)
    {
        /* probably GRANT is disabled or not enough privs */
        return 1;
    }
    mystmt(hstmt,rc);

    rc = SQLExecDirect( hstmt, "GRANT ALL ON test_tabprev3 TO my_tabpriv",  SQL_NTS);
    mystmt(hstmt,rc);

    SQLExecDirect(  hstmt, "FLUSH PRIVILEGES",  SQL_NTS);
    SQLFreeStmt(hstmt, SQL_CLOSE);
    return 0;

}


DECLARE_TEST(my_tablepriv)
{
    SQLRETURN   rc;

    if (my_tablepriv_init(hdbc, hstmt))
      return SKIP;

    printMessage("\n With All Types(CataLog,Schema and TableName)");

    rc  = SQLTablePrivileges(hstmt,
                             "mysql", SQL_NTS,/* CataLog	 */
                             NULL,SQL_NTS,        /* SchemaName */
                             NULL,SQL_NTS);       /* TableName  */
    mystmt(hstmt,rc);

    assert( 0 == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    printMessage("\n With All Types(CataLog,Schema and TableName)");

    rc  = SQLTablePrivileges(hstmt,
                             NULL, 0,                 /* CataLog	 */
                             NULL,SQL_NTS,        /* SchemaName */
                             "test_tabprev1",SQL_NTS);  /* TableName  */
    mystmt(hstmt,rc);

    assert( 2 == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    printMessage("\n With All Types(CataLog,Schema and TableName)");

    rc  = SQLTablePrivileges(hstmt,
                             NULL, 0,                 /* CataLog	 */
                             NULL,SQL_NTS,        /* SchemaName */
                             "test_tabprev2",SQL_NTS);  /* TableName  */
    mystmt(hstmt,rc);

    assert( 0 == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    printMessage("\n With All Types(CataLog,Schema and TableName)");

    rc  = SQLTablePrivileges(hstmt,
                             NULL, 0,                 /* CataLog	 */
                             NULL,SQL_NTS,        /* SchemaName */
                             "test_tabprev3",SQL_NTS);  /* TableName  */
    mystmt(hstmt,rc);

    /* value changed due to two additional rows in the result related to VIEWs */
    assert( 11 == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    printMessage("\n With All Types(CataLog,Schema and TableName)");

    rc  = SQLTablePrivileges(hstmt,
                             NULL, 0,                 /* CataLog	 */
                             NULL,SQL_NTS,        /* SchemaName */
                             "test_%",SQL_NTS);  /* TableName  */
    mystmt(hstmt,rc);

    my_print_non_format_result(hstmt);
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);


    printMessage("\n With All Types(CataLog,Schema and TableName)");

    rc  = SQLTablePrivileges(hstmt,
                             NULL, 0,                 /* CataLog	 */
                             NULL,SQL_NTS,        /* SchemaName */
                             "test_tabprev%",SQL_NTS);  /* TableName  */
    mystmt(hstmt,rc);


    /* value changed due to two additional rows in the result related to VIEWs */
    assert( 13  == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTablePrivileges(hstmt,"mysql",SQL_NTS,NULL,SQL_NTS,"tables_priv",SQL_NTS);
    mystmt(hstmt,rc);

    my_print_non_format_result(hstmt);
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(my_tablepriv_data)
{
    SQLINTEGER i;
    SQLRETURN  rc;
    SQLCHAR       TableQualifier_buf[129];
    SQLCHAR       TableOwner_buf[129];
    SQLCHAR       TableName_buf[129];
    SQLCHAR       Grantor_buf[129];
    SQLCHAR       Grantee_buf[129];
    SQLCHAR       Privilege_buf[129];
    SQLCHAR       IsGrantable_buf[4];

    SQLExecDirect(hstmt, "DROP USER todbc1_test",SQL_NTS);
    SQLExecDirect(hstmt, "DROP TABLE todbc1_tpriv1",SQL_NTS);
    SQLExecDirect(hstmt, "DROP TABLE todbc1_tpriv2",SQL_NTS);
    SQLExecDirect(hstmt, "DELETE FROM mysql.tables_priv where Table_name='todbc1_tpriv1'",SQL_NTS);
    SQLExecDirect(hstmt, "DELETE FROM mysql.tables_priv where User='todbc1_test'",SQL_NTS);
    SQLTransact(NULL, hdbc, SQL_COMMIT); 
    rc = SQLExecDirect(hstmt, "CREATE TABLE todbc1_tpriv1(a INTEGER)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt, "CREATE TABLE todbc1_tpriv2 (a INTEGER)",SQL_NTS);
    mystmt(hstmt,rc);

    SQLTransact(NULL, hdbc, SQL_COMMIT);

    /*--- Test 1: DBA should have at least CREATE, DELETE, INSERT,
          REFERENCES, SELECT and UPDATE privileges to the tables
          (and in this order)
      ---*/

    rc = SQLTablePrivileges(hstmt, NULL, SQL_NTS, NULL, SQL_NTS,
                            "todbc1_tpriv1", SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    assert (rc == SQL_NO_DATA_FOUND);

    /*--- Test 2: Grant SELECT to todbc1_test with GRANT option,
          and INSERT and UPDATE without.
      ---*/

    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLExecDirect(hstmt, "GRANT SELECT ON todbc1_tpriv1\
               TO todbc1_test WITH GRANT OPTION",SQL_NTS);
    mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt, "GRANT INSERT, UPDATE ON todbc1_tpriv1\
               TO todbc1_test", SQL_NTS);mystmt(hstmt,rc);
    rc = SQLExecDirect(hstmt, "GRANT USAGE ON todbc1_tpriv2\
               TO todbc1_test", SQL_NTS);mystmt(hstmt,rc);
    SQLTransact(NULL, hdbc, SQL_COMMIT);
    rc = SQLTablePrivileges(hstmt, NULL, SQL_NTS, NULL, SQL_NTS,
                            "todbc1_tpriv1", SQL_NTS);
    mystmt(hstmt,rc);  

    assert( 4  == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTablePrivileges(hstmt, NULL, SQL_NTS, NULL, SQL_NTS,
                            "todbc1_tpriv1", SQL_NTS);
    mystmt(hstmt,rc);

    TODBC_BIND_CHAR(1, TableQualifier_buf);
    TODBC_BIND_CHAR(2, TableOwner_buf);
    TODBC_BIND_CHAR(3, TableName_buf);
    TODBC_BIND_CHAR(4, Grantor_buf);
    TODBC_BIND_CHAR(5, Grantee_buf);
    TODBC_BIND_CHAR(6, Privilege_buf);
    TODBC_BIND_CHAR(7, IsGrantable_buf);  

    i = 0;
    while (1)
    {
        rc = SQLFetch(hstmt);
        if ((rc == SQL_SUCCESS) || (rc == SQL_SUCCESS_WITH_INFO))
        {
            char *p[] = { "Select", "Insert", "Update","Grant"};
            printMessage ("\n row '%d'",i);
            if (i < 4 && strcmp(Privilege_buf, p[i]) == 0)
            {
                if (strcmp(Privilege_buf, "Select") == 0 ||
                    strcmp(Privilege_buf, "Delete") == 0 ||
                    strcmp(Privilege_buf, "References") == 0)
                {
                    assert(strcmp(Grantee_buf, "todbc1_test") == 0);
                    assert(strcmp(IsGrantable_buf, "YES") == 0);
                }
                else
                {
                    if (strcmp(Grantee_buf, "todbc1_test") == 0)
                    {
                        /*assert(strcmp(Grantor_buf, "venu@localhost") == 0);*/
                    }
                    else
                    {
                        assert(strcmp(Grantee_buf, "todbc1_test") == 0);
                        assert(strcmp(IsGrantable_buf, "YES") == 0);
                    }
                }
                i++;
            }
            else
            {
                assert(strcmp(Grantee_buf, "todbc1_test") != 0);
            }
            assert(strcmp(TableName_buf, "todbc1_tpriv1") == 0);
        }
        else break;
    }
    assert(i == 4);

    /*--- test 3: Test 'LIKE' match patterns ---*/

    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLExecDirect(hstmt, "REVOKE INSERT, UPDATE, SELECT\
               ON todbc1_tpriv1 FROM todbc1_test",SQL_NTS);
    mystmt(hstmt,rc);
    SQLTransact(NULL, hdbc, SQL_COMMIT);
    rc = SQLTablePrivileges(hstmt, NULL, SQL_NTS, NULL, SQL_NTS,
                            "todbc1\\_tpriv_", SQL_NTS);
    mystmt(hstmt,rc);

    assert( 2  == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTablePrivileges(hstmt, NULL, SQL_NTS, NULL, SQL_NTS,
                            "todbc1\\_tpriv_", SQL_NTS);
    mystmt(hstmt,rc);

    TODBC_BIND_CHAR(1, TableQualifier_buf);
    TODBC_BIND_CHAR(2, TableOwner_buf);
    TODBC_BIND_CHAR(3, TableName_buf);
    TODBC_BIND_CHAR(4, Grantor_buf);
    TODBC_BIND_CHAR(5, Grantee_buf);
    TODBC_BIND_CHAR(6, Privilege_buf);
    TODBC_BIND_CHAR(7, IsGrantable_buf);  

    i = 0;

    while (1)
    {
        memset( TableName_buf, 0, sizeof(TableName_buf));
        rc = SQLFetch(hstmt);
        if ((rc == SQL_SUCCESS) || (rc == SQL_SUCCESS_WITH_INFO))
        {
            i++;
            printMessage("\n Name:%s",TableName_buf);
            assert(strncmp(TableName_buf, "todbc1_tpriv", 12) == 0);
            if (strcmp(TableName_buf, "todbc1_tpriv1") == 0)
            {
                assert(strcmp(Grantee_buf, "todbc1_test") == 0);
                assert(strcmp(IsGrantable_buf, "YES") == 0);

            }
            else if (strcmp(TableName_buf, "todbc1_tpriv2") == 0)
            {
                assert(strcmp(Grantee_buf, "todbc1_test") == 0);
                assert(strcmp(IsGrantable_buf, "NO") == 0);
            }
        }
        else break;
    }
    printMessage("\n i:%d",i);
    assert(i == 2);

    SQLFreeStmt(hstmt,SQL_CLOSE);

  return OK;
}


DECLARE_TEST(my_column_priv)
{
    SQLRETURN   rc;

    my_colpriv_init(hdbc, hstmt);

    rc = SQLColumnPrivileges(hstmt,NULL,0,
                             NULL,0,NULL,0,
                             NULL,0);
    mystmt(hstmt,rc);

    my_print_non_format_result(hstmt);

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_CLOSE);

  return OK;
}


DECLARE_TEST(my_colpriv)
{
  ok_stmt(hstmt, SQLColumnPrivileges(hstmt,
                                     "mysql", SQL_NTS, NULL, SQL_NTS,
                                     NULL, SQL_NTS, NULL, SQL_NTS));

  assert(0 == my_print_non_format_result(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLColumnPrivileges(hstmt,
                                     NULL, SQL_NTS, NULL, SQL_NTS,
                                     "test_colprev1", SQL_NTS, NULL, SQL_NTS));

  assert(4 == my_print_non_format_result(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLColumnPrivileges(hstmt,
                                     NULL, SQL_NTS, NULL, SQL_NTS,
                                     "test_colprev1", SQL_NTS, "a", SQL_NTS));

  assert(1 == my_print_non_format_result(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLColumnPrivileges(hstmt,
                                     NULL, SQL_NTS, NULL, SQL_NTS,
                                     "test_colprev2", SQL_NTS, NULL, SQL_NTS));
  assert(0 == my_print_non_format_result(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLColumnPrivileges(hstmt,
                                     NULL, SQL_NTS, NULL, SQL_NTS,
                                     "test_colprev3", SQL_NTS, NULL, SQL_NTS));

  assert(4 == my_print_non_format_result(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLColumnPrivileges(hstmt,
                                     NULL, SQL_NTS, NULL, SQL_NTS,
                                     "test_%", SQL_NTS, NULL, SQL_NTS));

  my_print_non_format_result(hstmt);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLColumnPrivileges(hstmt,
                                     NULL, SQL_NTS, NULL, SQL_NTS,
                                     "test_colprev%", SQL_NTS, NULL, SQL_NTS));

  assert(8 == my_print_non_format_result(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  ok_stmt(hstmt, SQLColumnPrivileges(hstmt,
                                     "mysql", SQL_NTS, NULL, SQL_NTS,
                                     "columns_priv", SQL_NTS, NULL, SQL_NTS));

  my_print_non_format_result(hstmt);

  ok_stmt(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

}


DECLARE_TEST(t_sqlprocedures)
{
  SQLRETURN rc;
  /** @todo check server version */

  rc= SQLExecDirect(hstmt, "DROP FUNCTION IF EXISTS t_sqlproc_func", SQL_NTS);
  mystmt(hstmt,rc);

  rc= SQLExecDirect(hstmt,
                    "CREATE FUNCTION t_sqlproc_func (a INT) RETURNS INT"
                    " RETURN SQRT(a)",
                    SQL_NTS);
  mystmt(hstmt,rc);

  rc= SQLExecDirect(hstmt, "DROP PROCEDURE IF EXISTS t_sqlproc_proc", SQL_NTS);
  mystmt(hstmt,rc);
  rc= SQLExecDirect(hstmt,
                    "CREATE PROCEDURE t_sqlproc_proc (OUT a INT) BEGIN"
                    " SELECT COUNT(*) INTO a FROM t_sqlproc;"
                    "END;",
                    SQL_NTS);
  mystmt(hstmt,rc);

  /* Try without specifying a catalog. */
  rc= SQLProcedures(hstmt, NULL, 0, NULL, 0, "t_sqlproc%", SQL_NTS);
  mystmt(hstmt,rc);

  assert(2 == my_print_non_format_result(hstmt));

  /* And try with specifying a catalog.  */
  rc= SQLProcedures(hstmt, "test", SQL_NTS, NULL, 0, "t_sqlproc%", SQL_NTS);
  mystmt(hstmt,rc);

  assert(2 == my_print_non_format_result(hstmt));

  rc= SQLExecDirect(hstmt, "DROP PROCEDURE IF EXISTS t_sqlproc_proc", SQL_NTS);
  mystmt(hstmt,rc);
  rc= SQLExecDirect(hstmt, "DROP FUNCTION IF EXISTS t_sqlproc_func", SQL_NTS);
  mystmt(hstmt,rc);
}


DECLARE_TEST(t_catalog)
{
    SQLRETURN rc;
    SQLCHAR      name[MYSQL_NAME_LEN+1];
    SQLUSMALLINT i;
    SQLSMALLINT  ncols, len;

    SQLCHAR colnames[19][20]= {
        "TABLE_CAT","TABLE_SCHEM","TABLE_NAME","COLUMN_NAME",
        "DATA_TYPE","TYPE_NAME","COLUMN_SIZE","BUFFER_LENGTH",
        "DECIMAL_DIGITS","NUM_PREC_RADIX","NULLABLE","REMARKS",
        "COLUMN_DEF","SQL_DATA_TYPE","SQL_DATETIME_SUB",
        "CHAR_OCTET_LENGTH","ORDINAL_POSITION","IS_NULLABLE"
    };
    SQLSMALLINT collengths[18]= {
        9,11,10,11,9,9,11,13,14,14,8,7,10,13,16,17,16,11
    };

    SQLExecDirect(hstmt,"drop table t_catalog",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table t_catalog(a tinyint, b char(4))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLColumns(hstmt,NULL,0,NULL,0,"t_catalog",9,NULL,0);
    mystmt(hstmt,rc);

    rc = SQLNumResultCols(hstmt, &ncols);
    mystmt(hstmt,rc);

    printMessage("\n total columns: %d", ncols);
    myassert(ncols == 18);
    myassert(myresult(hstmt) == 2);

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLColumns(hstmt,NULL,0,NULL,0,"t_catalog",9,NULL,0);
    mystmt(hstmt,rc);

    rc = SQLNumResultCols(hstmt,&ncols);
    mystmt(hstmt,rc);

    for (i= 1; i <= (SQLUINTEGER) ncols; i++)
    {
        rc = SQLDescribeCol(hstmt, i, name, MYSQL_NAME_LEN+1, &len, NULL, NULL, NULL, NULL);
        mystmt(hstmt,rc);

        printMessage("\n column %d: %s (%d)", i, name, len);
        myassert(strcmp(name,colnames[i-1]) == 0 && len == collengths[i-1]);
    }
    SQLFreeStmt(hstmt,SQL_CLOSE);

  return OK;
}


DECLARE_TEST(tmysql_specialcols)
{
  SQLRETURN rc;

    tmysql_exec(hstmt,"drop table tmysql_specialcols");
    rc = tmysql_exec(hstmt,"create table tmysql_specialcols(col1 int primary key, col2 varchar(30), col3 int)");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"create index tmysql_ind1 on tmysql_specialcols(col1)");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_specialcols values(100,'venu',1)");
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"insert into tmysql_specialcols values(200,'MySQL',2)");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"select * from tmysql_specialcols");
    mystmt(hstmt,rc);

    myresult(hstmt);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLSpecialColumns(hstmt,
                          SQL_BEST_ROWID,
                          NULL,0,
                          NULL,0,
                          "tmysql_specialcols",SQL_NTS,
                          SQL_SCOPE_SESSION,
                          SQL_NULLABLE);
    mystmt(hstmt,rc);

    myresult(hstmt);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"drop table tmysql_specialcols");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

  return OK;
}


/* To test SQLColumns misc case */
DECLARE_TEST(t_columns)
{
  SQLSMALLINT   NumPrecRadix, DataType, Nullable, DecimalDigits;
  SQLLEN        cbColumnSize, cbDecimalDigits, cbNumPrecRadix,
                cbDatabaseName, cbDataType, cbNullable;
  SQLRETURN     rc;
  SQLUINTEGER   ColumnSize, i;
  SQLUINTEGER   ColumnCount= 7;
  SQLCHAR       ColumnName[MAX_NAME_LEN], DatabaseName[MAX_NAME_LEN];
  SQLINTEGER    Values[7][5][2]=
  {
    { {5,2},  {6,4}, {0,2},  {10,2},  {1,2}},
    { {1,2},  {5,4},  {0,-1}, {10,-1}, {1,2}},
    { {12,2}, {20,4}, {0,-1}, {10,-1}, {0,2}},
    { {3,2},  {10,4}, {2,2},  {10,2},  {1,2}},
    { {-6,2},  {4,4}, {0,2},  {10,2},  {0,2}},
    { {4,2}, {11,4}, {0,2},  {10,2},  {0,2}},
    { {-6,2}, {4,4}, {0,2},  {10,2},  {0,2}}
  };

    SQLFreeStmt(hstmt, SQL_CLOSE);
    SQLExecDirect(hstmt,"DROP TABLE test_column",SQL_NTS);

    rc = SQLExecDirect(hstmt,"CREATE TABLE test_column(col0 smallint, \
                                                       col1 char(5),\
                                                       col2 varchar(20) not null,\
                                                       col3 decimal(10,2),\
                                                       col4 tinyint not null,\
                                                       col5 integer primary key,\
                                                       col6 tinyint not null unique auto_increment)",SQL_NTS);
    mystmt(hstmt,rc);

    mystmt(hstmt,rc);

    rc= SQLSetStmtAttr(hstmt, SQL_ATTR_METADATA_ID,
                      (SQLPOINTER)SQL_FALSE, SQL_IS_UINTEGER);
    mystmt(hstmt,rc);

    SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_STATIC, 0);

    rc= SQLGetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG,(SQLCHAR *)DatabaseName,
                          MAX_NAME_LEN, &cbDatabaseName);/* Current Catalog */
    mycon(hdbc,rc);

    for (i=0; i< ColumnCount; i++)
    {
      sprintf(ColumnName,"col%d",i);

      rc= SQLColumns(hstmt,
                     (SQLCHAR *)DatabaseName, (SQLUSMALLINT)cbDatabaseName,
                     SQL_NULL_HANDLE, 0,
                     (SQLCHAR *)"test_column", SQL_NTS,
                     (SQLCHAR *)ColumnName, SQL_NTS);
      mystmt(hstmt,rc);

      /* 5 -- Data type */
      rc=  SQLBindCol(hstmt, 5, SQL_C_SSHORT, &DataType, 0, &cbDataType);
      mystmt(hstmt,rc);

      /* 7 -- Column Size */
      rc=  SQLBindCol(hstmt, 7, SQL_C_ULONG, &ColumnSize, 0, &cbColumnSize);
      mystmt(hstmt,rc);

      /* 9 -- Decimal Digits */
      rc= SQLBindCol(hstmt, 9, SQL_C_SSHORT, &DecimalDigits, 0, &cbDecimalDigits);
      mystmt(hstmt,rc);

      /* 10 -- Num Prec Radix */
      rc= SQLBindCol(hstmt, 10, SQL_C_SSHORT, &NumPrecRadix, 0, &cbNumPrecRadix);
      mystmt(hstmt,rc);

      /* 11 -- Nullable */
      rc= SQLBindCol(hstmt, 11, SQL_C_SSHORT, &Nullable, 0, &cbNullable);
      mystmt(hstmt,rc);

      rc= SQLFetch(hstmt);
      mystmt(hstmt,rc);

      fprintf(stdout,"Column %s:\n", ColumnName);
      fprintf(stdout,"\t DataType     = %d(%d)\n", DataType, cbDataType);
      fprintf(stdout,"\t ColumnSize   = %d(%d)\n", ColumnSize, cbColumnSize);
      fprintf(stdout,"\t DecimalDigits= %d(%d)\n", DecimalDigits, cbDecimalDigits);
      fprintf(stdout,"\t NumPrecRadix = %d(%d)\n", NumPrecRadix, cbNumPrecRadix);
      fprintf(stdout,"\t Nullable     = %s(%d)\n\n",
                      Nullable == SQL_NO_NULLS ? "NO": "YES", cbNullable);

      myassert(DataType == Values[i][0][0]);
      myassert(cbDataType == Values[i][0][1]);

      myassert(ColumnSize == Values[i][1][0]);
      myassert(cbColumnSize == Values[i][1][1]);

      myassert(DecimalDigits == Values[i][2][0]);
      myassert(cbDecimalDigits == Values[i][2][1]);

      myassert(NumPrecRadix == Values[i][3][0]);
      myassert(cbNumPrecRadix == Values[i][3][1]);

      myassert(Nullable == Values[i][4][0]);
      myassert(cbNullable == Values[i][4][1]);

      rc= SQLFetch(hstmt);
      myassert(rc == SQL_NO_DATA);

      SQLFreeStmt(hstmt,SQL_UNBIND);
      SQLFreeStmt(hstmt,SQL_CLOSE);
    }

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"DROP TABLE test_column",SQL_NTS);
    mystmt(hstmt,rc);
    SQLFreeStmt(hstmt,SQL_CLOSE);

  return OK;
}


/* Test the bug SQLTables */
typedef struct t_table_bug
{
  SQLCHAR     szColName[MAX_NAME_LEN];
  SQLSMALLINT pcbColName;
  SQLSMALLINT pfSqlType;
  SQLUINTEGER pcbColDef;
  SQLSMALLINT pibScale;
  SQLSMALLINT pfNullable;
} t_describe_col;


t_describe_col t_tables_bug_data[5] =
{
  {"TABLE_CAT",   9, SQL_VARCHAR, MYSQL_NAME_LEN, 0, SQL_NULLABLE},
  {"TABLE_SCHEM",11, SQL_VARCHAR, MYSQL_NAME_LEN, 0, SQL_NULLABLE},
  {"TABLE_NAME", 10, SQL_VARCHAR, MYSQL_NAME_LEN, 0, SQL_NULLABLE},
  {"TABLE_TYPE", 10, SQL_VARCHAR, MYSQL_NAME_LEN, 0, SQL_NULLABLE},
  {"REMARKS",     7, SQL_VARCHAR, MYSQL_NAME_LEN, 0, SQL_NULLABLE},
};


DECLARE_TEST(t_tables_bug)
{
  SQLRETURN   rc;
  SQLSMALLINT i, ColumnCount, pcbColName, pfSqlType, pibScale, pfNullable;
  SQLUINTEGER pcbColDef;
  SQLCHAR     szColName[MAX_NAME_LEN];

   SQLFreeStmt(hstmt, SQL_CLOSE);

   rc = SQLTables(hstmt,NULL,0,NULL,0,NULL,0,"'TABLE'",SQL_NTS);
   mystmt(hstmt,rc);

   rc = SQLNumResultCols(hstmt,&ColumnCount);
   mystmt(hstmt,rc);

   fprintf(stdout, "total columns in SQLTables: %d\n", ColumnCount);
   myassert(ColumnCount == 5);

   for (i= 1; i <= ColumnCount; i++)
   {
     rc = SQLDescribeCol(hstmt, (SQLUSMALLINT)i,
                         szColName,MAX_NAME_LEN,&pcbColName,
                         &pfSqlType,&pcbColDef,&pibScale,&pfNullable);
     mystmt(hstmt,rc);

     fprintf(stdout, "Column Number'%d':\n", i);
     fprintf(stdout, "\t Column Name    : %s\n", szColName);
     fprintf(stdout, "\t NameLengh      : %d\n", pcbColName);
     fprintf(stdout, "\t DataType       : %d\n", pfSqlType);
     fprintf(stdout, "\t ColumnSize     : %d\n", pcbColDef);
     fprintf(stdout, "\t DecimalDigits  : %d\n", pibScale);
     fprintf(stdout, "\t Nullable       : %d\n", pfNullable);

     myassert(strcmp(t_tables_bug_data[i-1].szColName,szColName) == 0);
     myassert(t_tables_bug_data[i-1].pcbColName == pcbColName);
     myassert(t_tables_bug_data[i-1].pfSqlType == pfSqlType);
     myassert(t_tables_bug_data[i-1].pcbColDef == pcbColDef);
     myassert(t_tables_bug_data[i-1].pibScale == pibScale);
     myassert(t_tables_bug_data[i-1].pfNullable == pfNullable);
   }
   SQLFreeStmt(hstmt,SQL_CLOSE);

  return OK;
}


DECLARE_TEST(t_current_catalog)
{
  SQLCHAR     cur_db[255], db[255];
  SQLRETURN   rc;
  SQLUINTEGER len;

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLGetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (char *)db, 255, &len);
    mycon(hdbc,rc);
    fprintf(stdout,"current_catalog: %s (%ld)\n", db, len);
    myassert(strcmp(db, "test") == 0 || strlen("test") == len);

    rc = SQLSetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (char *)db, SQL_NTS);
    mycon(hdbc,rc);

    SQLExecDirect(hstmt, "DROP DATABASE IF EXISTS test_odbc_current", SQL_NTS);

    strcpy(cur_db, "test_odbc_current");
    rc = SQLSetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (char *)cur_db, SQL_NTS);
    mycon_r(hdbc,rc);

    rc = SQLExecDirect(hstmt, "CREATE DATABASE test_odbc_current", SQL_NTS);
    mystmt(hstmt,rc);

    strcpy(cur_db, "test_odbc_current");
    rc = SQLSetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (char *)cur_db, SQL_NTS);
    mycon(hdbc,rc);

    rc = SQLGetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (char *)db, 255, &len);
    mycon(hdbc,rc);
    fprintf(stdout,"current_catalog: %s (%ld)\n", db, len);
    myassert(strcmp(cur_db, db) == 0 || strlen(cur_db) == len);

    strcpy(cur_db, "test_odbc_current_12455");
    rc = SQLSetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (char *)cur_db, SQL_NTS);
    mycon_r(hdbc,rc);

    rc = SQLSetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (char *)cur_db, len);
    mycon(hdbc,rc);

    /* reset for further tests */
    rc = SQLSetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (SQLCHAR *)"test", SQL_NTS);
    mycon(hdbc,rc);

    rc = SQLExecDirect(hstmt, "DROP DATABASE test_odbc_current", SQL_NTS);
    mycon(hstmt,rc);

  return OK;
}


DECLARE_TEST(tmysql_showkeys)
{
    SQLRETURN rc;

    tmysql_exec(hstmt,"drop table tmysql_spk");

    rc = tmysql_exec(hstmt,"create table tmysql_spk(col1 int primary key)");
    mystmt(hstmt,rc);

    rc = SQLTransact(NULL,hdbc,SQL_COMMIT);
    mycon(hdbc,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = tmysql_exec(hstmt,"SHOW KEYS FROM tmysql_spk");
    mystmt(hstmt,rc);

    my_assert(1 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_sqltables)
{
    SQLRETURN r;

    r  = SQLTables(hstmt,NULL,0,NULL,0,NULL,0,NULL,0);
    mystmt(hstmt,r);

    myresult(hstmt);

    r = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,r);

    r  = SQLTables(hstmt,NULL,0,NULL,0,NULL,0,"'system table'",SQL_NTS);
    mystmt(hstmt,r);

    if (driver_min_version(hdbc,"03.51.07",8))
        myassert(myresult(hstmt) != 0);
    else
        myassert(0 == myresult(hstmt));

    r = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,r);

    r  = SQLTables(hstmt,NULL,0,NULL,0,NULL,0,"TABLE",SQL_NTS);
    mystmt(hstmt,r);

    myresult(hstmt);

    r = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,r);

    r  = SQLTables(hstmt,"TEST",SQL_NTS,"TEST",SQL_NTS,NULL,0,"TABLE",SQL_NTS);
    mystmt(hstmt,r);

    myresult(hstmt);

    r = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,r);

    r = SQLTables(hstmt,"%",SQL_NTS,NULL,0,NULL,0,NULL,0);
    mystmt(hstmt,r);

    myresult(hstmt);

    r = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,r);

    r = SQLTables(hstmt,NULL,0,"%",SQL_NTS,NULL,0,NULL,0);
    mystmt(hstmt,r);

    myresult(hstmt);

    r = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,r);

    r = SQLTables(hstmt,NULL,0,NULL,0,NULL,0,"%",SQL_NTS);
    mystmt(hstmt,r);

    if (driver_min_version(hdbc,"03.51.07",8))
        myassert( 2 == myresult(hstmt));
    else
        myassert( 1 == myresult(hstmt));


    r = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,r);

  return OK;
}


/**
 Bug #4518: SQLForeignKeys returns too many foreign key
*/
DECLARE_TEST(t_bug4518)
{

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug4518_c, t_bug4518_c2, "
                "                     t_bug4518_p");
  ok_sql(hstmt, "CREATE TABLE t_bug4518_p (id INT PRIMARY KEY) ENGINE=InnoDB");
  ok_sql(hstmt, "CREATE TABLE t_bug4518_c (id INT, parent_id INT,"
                "                          FOREIGN KEY (parent_id)"
                "                           REFERENCES"
                "                             t_bug4518_p(id)"
                "                           ON DELETE SET NULL)"
                " ENGINE=InnoDB");
  ok_sql(hstmt, "CREATE TABLE t_bug4518_c2 (id INT, parent_id INT,"
                "                           FOREIGN KEY (parent_id)"
                "                            REFERENCES"
                "                              t_bug4518_p(id)"
                "                            ON DELETE SET NULL)"
                " ENGINE=InnoDB");

  ok_stmt(hstmt, SQLForeignKeys(hstmt, NULL, 0, NULL, 0, NULL, 0, NULL, 0,
                                NULL, 0, (SQLCHAR *)"t_bug4518_c", SQL_NTS));

  my_assert(1 == my_print_non_format_result(hstmt));

  ok_stmt(hstmt, SQLFreeStmt(hstmt,SQL_CLOSE));

  ok_sql(hstmt, "DROP TABLE IF EXISTS t_bug4518_c, t_bug4518_c2, t_bug4518_p");

  return OK;
}


BEGIN_TESTS
  ADD_TEST(my_columns_null)
  ADD_TEST(my_drop_table)
  ADD_TEST(my_table_priv)
  ADD_TEST(my_table_dbs)
  ADD_TEST(my_tablepriv)
  ADD_TEST(my_tablepriv_data)
  ADD_TEST(my_column_priv)
  ADD_TEST(my_colpriv)
  ADD_TEST(t_sqlprocedures)
  ADD_TEST(t_catalog)
  ADD_TEST(tmysql_specialcols)
  ADD_TEST(t_columns)
  ADD_TEST(t_tables_bug)
  ADD_TEST(t_current_catalog)
  ADD_TEST(tmysql_showkeys)
  ADD_TEST(t_sqltables)
  ADD_TEST(t_bug4518)
END_TESTS


RUN_TESTS
