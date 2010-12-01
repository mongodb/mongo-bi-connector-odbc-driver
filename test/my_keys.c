/*
  Copyright (c) 2003, 2010, Oracle and/or its affiliates. All rights reserved.

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


/* UPDATE with no keys ...  */
DECLARE_TEST(my_no_keys)
{
    SQLRETURN rc;
    SQLINTEGER nData;

  ok_sql(hstmt, "DROP TABLE IF EXISTS my_no_keys");
  ok_sql(hstmt, "create table my_no_keys(col1 int,\
                                                      col2 varchar(30),\
                                                      col3 int,\
                                                      col4 int)");

  ok_sql(hstmt, "insert into my_no_keys values(100,'MySQL1',1,3000)");
  ok_sql(hstmt, "insert into my_no_keys values(200,'MySQL1',2,3000)");
  ok_sql(hstmt, "insert into my_no_keys values(300,'MySQL1',3,3000)");
  ok_sql(hstmt, "insert into my_no_keys values(400,'MySQL1',4,3000)");

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);     

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC, 0);
    mystmt(hstmt, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY ,(SQLPOINTER)SQL_CONCUR_ROWVER , 0);
    mystmt(hstmt, rc);  

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE  ,(SQLPOINTER)1 , 0);
    mystmt(hstmt, rc);  

    /* UPDATE ROW[2]COL[4] */
    ok_sql(hstmt, "select col4 from my_no_keys");
    mystmt(hstmt,rc);    

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&nData,100,NULL);
    mystmt(hstmt,rc);

    rc = SQLExtendedFetch(hstmt,SQL_FETCH_ABSOLUTE,2,NULL,NULL);
    mystmt(hstmt,rc);

    nData = 999;

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    ok_sql(hstmt,"select * from my_no_keys");

    is(4 == myresult(hstmt));

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    ok_sql(hstmt,"select * from my_no_keys");

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    is(3000 == my_fetch_int(hstmt,4));

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    is(3000 == my_fetch_int(hstmt,4));

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);
    is(3000 == my_fetch_int(hstmt,4));

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);

  ok_sql(hstmt, "DROP TABLE IF EXISTS my_no_keys");

  return OK;
}


DECLARE_TEST(my_foreign_keys)
{
    SQLRETURN   rc=0;
    SQLCHAR     dbc[255];

    ok_sql(hstmt,"DROP DATABASE IF EXISTS test_odbc_fk");
    ok_sql(hstmt,"CREATE DATABASE test_odbc_fk");
    ok_sql(hstmt,"use test_odbc_fk");

    ok_sql(hstmt,"DROP TABLE IF EXISTS test_fkey_c1");
    ok_sql(hstmt,"DROP TABLE IF EXISTS test_fkey_3");
    ok_sql(hstmt,"DROP TABLE IF EXISTS test_fkey_2");
    ok_sql(hstmt,"DROP TABLE IF EXISTS test_fkey_1");
    ok_sql(hstmt,"DROP TABLE IF EXISTS test_fkey_p1");
    ok_sql(hstmt,"DROP TABLE IF EXISTS test_fkey_comment_f");
    ok_sql(hstmt,"DROP TABLE IF EXISTS test_fkey_comment_p");

    ok_sql(hstmt,
                       "CREATE TABLE test_fkey1(\
                 A INTEGER NOT NULL,B INTEGER NOT NULL,C INTEGER NOT NULL,\
                 D INTEGER,PRIMARY KEY (C,B,A)) ENGINE=InnoDB;");

    ok_sql(hstmt,
                       "CREATE TABLE test_fkey_p1(\
                 A INTEGER NOT NULL,B INTEGER NOT NULL,C INTEGER NOT NULL,\
                 D INTEGER NOT NULL,E INTEGER NOT NULL,F INTEGER NOT NULL,\
                 G INTEGER NOT NULL,H INTEGER NOT NULL,I INTEGER NOT NULL,\
                 J INTEGER NOT NULL,K INTEGER NOT NULL,L INTEGER NOT NULL,\
                 M INTEGER NOT NULL,N INTEGER NOT NULL,O INTEGER NOT NULL,\
                 P INTEGER NOT NULL,Q INTEGER NOT NULL,R INTEGER NOT NULL,\
                 PRIMARY KEY (D,F,G,H,I,J,K,L,M,N,O)) ENGINE=InnoDB;");
    ok_sql(hstmt,
                        "CREATE TABLE test_fkey2 (\
                 E INTEGER NOT NULL,C INTEGER NOT NULL,B INTEGER NOT NULL,\
                 A INTEGER NOT NULL,PRIMARY KEY (E),\
                 INDEX test_fkey2_ind(C,B,A),\
                 FOREIGN KEY (C,B,A) REFERENCES test_fkey1(C,B,A)) ENGINE=InnoDB;");

    ok_sql(hstmt,
                        "CREATE TABLE test_fkey3 (\
                 F INTEGER NOT NULL,C INTEGER NOT NULL,E INTEGER NOT NULL,\
                 G INTEGER, A INTEGER NOT NULL, B INTEGER NOT NULL,\
                 PRIMARY KEY (F),\
                 INDEX test_fkey3_ind(C,B,A),\
                 INDEX test_fkey3_ind3(E),\
                 FOREIGN KEY (C,B,A) REFERENCES test_fkey1(C,B,A),\
                 FOREIGN KEY (E) REFERENCES test_fkey2(E)) ENGINE=InnoDB;");

    ok_sql(hstmt,
                       "CREATE TABLE test_fkey_c1(\
                 A INTEGER NOT NULL,B INTEGER NOT NULL,C INTEGER NOT NULL,\
                 D INTEGER NOT NULL,E INTEGER NOT NULL,F INTEGER NOT NULL,\
                 G INTEGER NOT NULL,H INTEGER NOT NULL,I INTEGER NOT NULL,\
                 J INTEGER NOT NULL,K INTEGER NOT NULL,L INTEGER NOT NULL,\
                 M INTEGER NOT NULL,N INTEGER NOT NULL,O INTEGER NOT NULL,\
                 P INTEGER NOT NULL,Q INTEGER NOT NULL,R INTEGER NOT NULL,\
                 PRIMARY KEY (A,B,C,D,E,F,G,H,I,J,K,L,M,N,O),\
                INDEX test_fkey_c1_ind1(D,F,G,H,I,J,K,L,M,N,O),\
                INDEX test_fkey_c1_ind2(C,B,A),\
                INDEX test_fkey_c1_ind3(E),\
                 FOREIGN KEY (D,F,G,H,I,J,K,L,M,N,O) REFERENCES \
                   test_fkey_p1(D,F,G,H,I,J,K,L,M,N,O),\
                 FOREIGN KEY (C,B,A) REFERENCES test_fkey1(C,B,A),\
                 FOREIGN KEY (E) REFERENCES test_fkey2(E)) ENGINE=InnoDB;");

    ok_sql(hstmt,
                       "CREATE TABLE test_fkey_comment_p ( \
                ISP_ID SMALLINT NOT NULL, \
	              CUSTOMER_ID VARCHAR(10), \
	              ABBREVIATION VARCHAR(20) NOT NULL, \
	              NAME VARCHAR(40) NOT NULL, \
	              SEQUENCE INT NOT NULL, \
	              PRIMARY KEY PL_ISP_PK (ISP_ID), \
	              UNIQUE INDEX PL_ISP_CUSTOMER_ID_UK (CUSTOMER_ID), \
	              UNIQUE INDEX PL_ISP_ABBR_UK (ABBREVIATION) \
                ) ENGINE=InnoDB COMMENT='Holds the information of (customers)'");

    ok_sql(hstmt,
                       "CREATE TABLE test_fkey_comment_f ( \
	              CAMPAIGN_ID INT NOT NULL , \
	              ISP_ID SMALLINT NOT NULL, \
	              NAME  VARCHAR(40) NOT NULL, \
	              DISPLAY_NAME VARCHAR(255) NOT NULL, \
	              BILLING_TYPE SMALLINT NOT NULL, \
	              BROADBAND_OK BOOL, \
	              EMAIL_OK BOOL, \
	              PRIMARY KEY PL_ISP_CAMPAIGN_PK (CAMPAIGN_ID), \
	              UNIQUE INDEX PL_ISP_CAMPAIGN_BT_UK (BILLING_TYPE), \
	              INDEX PL_ISP_CAMPAIGN_ISP_ID_IX (ISP_ID), \
	              FOREIGN KEY (ISP_ID) REFERENCES test_fkey_comment_p(ISP_ID) \
              ) ENGINE=InnoDB COMMENT='List of campaigns (test comment)'");

    SQLFreeStmt(hstmt,SQL_CLOSE);

    strcpy((char *)dbc, "test_odbc_fk");

    printMessage("\n WITH ONLY PK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,               /*PK CATALOG*/
                        NULL, SQL_NTS,                /*PK SCHEMA*/
                        (SQLCHAR *)"test_fkey1", SQL_NTS,  /*PK TABLE*/
                        dbc, SQL_NTS,               /*FK CATALOG*/
                        NULL, SQL_NTS,                /*FK SCHEMA*/
                        NULL, SQL_NTS);               /*FK TABLE*/
    mystmt(hstmt,rc);
    is(9 == myresult(hstmt));

    printMessage("\n WITH ONLY FK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        NULL, SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey1", SQL_NTS);
    mystmt(hstmt,rc);
    is(0 == myresult(hstmt));

    printMessage("\n WITH ONLY FK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        NULL, SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey_c1", SQL_NTS);
    mystmt(hstmt,rc);
    is(15 == myresult(hstmt));

    printMessage("\n WITH ONLY FK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        NULL, SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey2", SQL_NTS);
    mystmt(hstmt,rc);
    is(3 == myresult(hstmt));

    printMessage("\n WITH ONLY PK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey_p1", SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        NULL, SQL_NTS);
    mystmt(hstmt,rc);
    is(11 == myresult(hstmt));

    printMessage("\n WITH ONLY PK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey3", SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        NULL, SQL_NTS);
    mystmt(hstmt,rc);
    is(0 == myresult(hstmt));

    printMessage("\n WITH ONLY PK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey2", SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        NULL, SQL_NTS);
    mystmt(hstmt,rc);
    is(2 == myresult(hstmt));

    printMessage("\n WITH ONLY PK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey1", SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        NULL, SQL_NTS);
    mystmt(hstmt,rc);
    is(9 == myresult(hstmt));

    printMessage("\n WITH ONLY FK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        NULL, SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey3", SQL_NTS);
    mystmt(hstmt,rc);
    is(4 == myresult(hstmt));

    printMessage("\n WITH BOTH PK and FK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey1",SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey3", SQL_NTS);
    mystmt(hstmt,rc);
    is(3 == myresult(hstmt));

    printMessage("\n WITH BOTH PK and FK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey_p1",SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey_c1", SQL_NTS);
    mystmt(hstmt,rc);
    is(11 == myresult(hstmt));

    printMessage("\n WITH BOTH PK and FK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey1",SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey2", SQL_NTS);
    mystmt(hstmt,rc);
    is(3 == myresult(hstmt));

    printMessage("\n WITH BOTH PK and FK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey_p1",SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey2", SQL_NTS);
    mystmt(hstmt,rc);
    is(0 == myresult(hstmt));

    printMessage("\n WITH BOTH PK and FK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey3", SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey1", SQL_NTS);
    mystmt(hstmt,rc);
    is(0 == myresult(hstmt));

    printMessage("\n WITH BOTH PK and FK OPTION");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey2", SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey1", SQL_NTS);
    mystmt(hstmt,rc);
    is(0 == myresult(hstmt));

    printMessage("\n WITH ACTUAL LENGTH INSTEAD OF SQL_NTS");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey1",10,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey2",10);
    mystmt(hstmt,rc);
    is(3 == myresult(hstmt));
    SQLFreeStmt(hstmt,SQL_CLOSE);

    printMessage("\n WITH NON-EXISTANT TABLES");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey_junk", SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey_junk", SQL_NTS);
    mystmt(hstmt,rc);
    is(0 == myresult(hstmt));
    SQLFreeStmt(hstmt,SQL_CLOSE);

    printMessage("\n WITH COMMENT FIELD");
    rc = SQLForeignKeys(hstmt,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey_comment_p", SQL_NTS,
                        dbc, SQL_NTS,
                        NULL, SQL_NTS,
                        (SQLCHAR *)"test_fkey_comment_f", SQL_NTS);
    mystmt(hstmt,rc);
    is(1 == myresult(hstmt));

    {
        char buff[255];
        sprintf(buff,"use %s",mydb);
        ok_sql(hstmt, "DROP DATABASE test_odbc_fk");
        SQLFreeStmt(hstmt, SQL_CLOSE);
        SQLExecDirect(hstmt, (SQLCHAR *)buff, SQL_NTS);
        SQLFreeStmt(hstmt, SQL_CLOSE);
    }

  return OK;
}


void t_strstr()
{
    char    *string = "'TABLE','VIEW','SYSTEM TABLE'";
    char    *str=",";
    char    *type;

    type = strstr((const char *)string,(const char *)str);
    while (type++)
    {
        int len = type - string;
        printMessage("\n Found '%s' at position '%d[%s]", str, len, type);
        type = strstr(type,str);
    }
}


BEGIN_TESTS
  ADD_TEST(my_no_keys)
  ADD_TEST(my_foreign_keys)
END_TESTS


RUN_TESTS
