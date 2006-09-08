/***************************************************************************
                          my_catalog.c  -  description
                             -------------------
    begin                : Fri Feb 15 2002
    copyright            : (C) MySQL AB 1995-2002
    author               : venu ( venu@mysql.com )
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "mytest3.h"


/**
* to test the pcbValue on cursor ops
**/
void my_columns_null(SQLHDBC hdbc, SQLHSTMT hstmt)
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
}

/**
* to test the DROP TABLE bug after SQLColumns
**/
void my_drop_table(SQLHDBC hdbc, SQLHSTMT hstmt)
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
}

/**
* to test SQLTablePrivileges
**/
void my_table_priv(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN   rc;

    rc = SQLTablePrivileges(hstmt,"te%",SQL_NTS,
                            NULL,0,"my_%",SQL_NTS);
    mystmt(hstmt,rc);

    my_print_non_format_result(hstmt);

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_CLOSE);
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

void my_table_dbs(SQLHDBC hdbc,SQLHSTMT hstmt)
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
void my_tablepriv(SQLHDBC hdbc,SQLHSTMT hstmt)
{
    SQLRETURN   rc;

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

    assert( 9 == my_print_non_format_result(hstmt));
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


    assert( 11  == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLTablePrivileges(hstmt,"mysql",SQL_NTS,NULL,SQL_NTS,"tables_priv",SQL_NTS);
    mystmt(hstmt,rc);

    my_print_non_format_result(hstmt);
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);
}

void my_tablepriv_data(SQLHDBC hdbc, SQLHSTMT hstmt)
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
}

/**
* to test SQLColumnPrivileges
**/
void my_column_priv(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN   rc;

    rc = SQLColumnPrivileges(hstmt,NULL,0,
                             NULL,0,NULL,0,
                             NULL,0);
    mystmt(hstmt,rc);

    my_print_non_format_result(hstmt);

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_CLOSE);
}

void my_colpriv(SQLHDBC hdbc,SQLHSTMT hstmt)
{
    SQLRETURN   rc;

    printMessage("\n With All Types(CataLog,Schema and TableName)");

    rc  = SQLColumnPrivileges(hstmt,
                              "mysql", SQL_NTS,/* CataLog	 */
                              NULL,SQL_NTS,        /* SchemaName */
                              NULL,SQL_NTS,        /* TableName  */
                              NULL,SQL_NTS);
    mystmt(hstmt,rc);

    assert( 0 == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    printMessage("\n With All Types(CataLog,Schema and TableName)");

    rc  = SQLColumnPrivileges(hstmt,
                              NULL, 0,                 /* CataLog	 */
                              NULL,SQL_NTS,        /* SchemaName */
                              "test_colprev1",SQL_NTS,  /* TableName  */
                              NULL,SQL_NTS);
    mystmt(hstmt,rc);

    assert( 4 == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    printMessage("\n With All Types(CataLog,Schema and TableName)");

    rc  = SQLColumnPrivileges(hstmt,
                              NULL, 0,                 /* CataLog	 */
                              NULL,SQL_NTS,        /* SchemaName */
                              "test_colprev1",SQL_NTS,  /* TableName  */
                              "a",SQL_NTS);
    mystmt(hstmt,rc);

    assert( 1 == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    printMessage("\n With All Types(CataLog,Schema and TableName)");

    rc  = SQLColumnPrivileges(hstmt,
                              NULL, 0,                 /* CataLog	 */
                              NULL,SQL_NTS,        /* SchemaName */
                              "test_colprev2",SQL_NTS,  /* TableName  */
                              NULL,SQL_NTS);
    mystmt(hstmt,rc);

    assert( 0 == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    printMessage("\n With All Types(CataLog,Schema and TableName)");

    rc  = SQLColumnPrivileges(hstmt,
                              NULL, 0,                 /* CataLog	 */
                              NULL,SQL_NTS,        /* SchemaName */
                              "test_colprev3",SQL_NTS,  /* TableName  */
                              NULL,SQL_NTS);
    mystmt(hstmt,rc);

    assert( 4 == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    printMessage("\n With All Types(CataLog,Schema and TableName)");

    rc  = SQLColumnPrivileges(hstmt,
                              NULL, 0,                 /* CataLog	 */
                              NULL,SQL_NTS,        /* SchemaName */
                              "test_%",SQL_NTS,  /* TableName  */
                              NULL,SQL_NTS);
    mystmt(hstmt,rc);

    my_print_non_format_result(hstmt);
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);


    printMessage("\n With All Types(CataLog,Schema and TableName)");

    rc  = SQLColumnPrivileges(hstmt,
                              NULL, 0,                 /* CataLog	 */
                              NULL,SQL_NTS,        /* SchemaName */
                              "test_colprev%",SQL_NTS,  /* TableName  */
                              NULL,SQL_NTS);
    mystmt(hstmt,rc);


    assert( 8  == my_print_non_format_result(hstmt));
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLColumnPrivileges(hstmt,"mysql",SQL_NTS,NULL,SQL_NTS,"columns_priv",SQL_NTS,NULL,SQL_NTS) ;
    mystmt(hstmt,rc);

    my_print_non_format_result(hstmt);
    rc = SQLFreeStmt(hstmt, SQL_CLOSE);
    mystmt(hstmt,rc);
}

/**
MAIN ROUTINE...
*/
int main(int argc, char *argv[])
{
    SQLHENV   henv;
    SQLHDBC   hdbc;
    SQLHSTMT  hstmt;
    SQLINTEGER narg;
    SQLCHAR    conn[255];

    printMessageHeader();

    /*
     * if connection string supplied through arguments, overrite
     * the default one..
    */
    for (narg = 1; narg < argc; narg++)
    {
        if ( narg == 1 )
            mydsn = argv[1];
        else if ( narg == 2 )
            myuid = argv[2];
        else if ( narg == 3 )
            mypwd = argv[3];
    }

    sprintf(conn,"DSN=%s;UID=%s;PWD=%s;OPTION=3;",mydsn,myuid,mypwd);
    mydrvconnect(&henv,&hdbc,&hstmt,conn);

    my_columns_null(hdbc,hstmt);
    my_drop_table(hdbc,hstmt);
    my_table_priv(hdbc,hstmt);
    my_table_dbs(hdbc,hstmt);
    if (!my_tablepriv_init(hdbc,hstmt))
    {
        my_tablepriv(hdbc,hstmt);
        my_tablepriv_data(hdbc,hstmt);
        my_column_priv(hdbc,hstmt);
        my_colpriv_init(hdbc,hstmt);
        my_colpriv(hdbc,hstmt);
    }
    mydisconnect(&henv,&hdbc,&hstmt);


    printMessageFooter( 1 );

    return(0);
}
