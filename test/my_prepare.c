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

SQLRETURN rc;

/* Basic prepared statements - binary protocol test */
DECLARE_TEST(t_prep_basic)
{
    SQLINTEGER id, pcrow;
    SQLLEN length1, length2;
    char       name[20];

    SQLExecDirect(hstmt,"drop table t_prep_basic",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table t_prep_basic(a int, b char(4))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt, "insert into t_prep_basic values(?,'venu')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &id, 0, NULL);
    mystmt(hstmt,rc);

    id = 100;
    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt, &pcrow);
    mystmt(hstmt,rc);

    printMessage( "affected rows: %ld\n", pcrow);
    myassert(pcrow == 1);

    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"select * from t_prep_basic",SQL_NTS);
    mystmt(hstmt,rc);  

    rc = SQLBindCol(hstmt, 1, SQL_C_LONG, &id, 0, &length1);
    mystmt(hstmt,rc);  

    rc = SQLBindCol(hstmt, 2, SQL_C_CHAR, name, 5, &length2);
    mystmt(hstmt,rc);

    id = 0;
    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    printMessage( "outdata: %d(%d), %s(%d)\n",id,length1,name,length2);
    myassert(id == 100 && length1 == sizeof(SQLINTEGER));
    myassert(strcmp(name,"venu")==0 && length2 == 4);

    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA_FOUND);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


/* to test buffer length */
DECLARE_TEST(t_prep_buffer_length)
{
    SQLLEN length;
    char       buffer[20];

    SQLExecDirect(hstmt,"drop table t_prep_buffer_length",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table t_prep_buffer_length(a varchar(20))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt, "insert into t_prep_buffer_length values(?)",SQL_NTS);
    mystmt(hstmt,rc);

    length= 0;
    strcpy(buffer,"abcdefghij");

    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 15, 10, buffer, 4, &length);
    mystmt(hstmt,rc);

    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);

    length= 3;

    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);

    length= 10;    

    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);

    length= 9;    

    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);

    length= SQL_NTS;    

    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"select * from t_prep_buffer_length",SQL_NTS);
    mystmt(hstmt,rc);  

    rc = SQLBindCol(hstmt, 1, SQL_C_CHAR, buffer, 15, &length);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    printMessage( "outdata: %s (%ld)\n", buffer, length);
    myassert(buffer[0] == '\0' && length == 0);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    printMessage("outdata: %s (%ld)\n", buffer, length);
    myassert(strcmp(buffer,"abc") == 0 && length == 3);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    printMessage("outdata: %s (%ld)\n", buffer, length);
    myassert(strcmp(buffer,"abcdefghij") == 0 && length == 10);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    printMessage("outdata: %s (%ld)\n", buffer, length);
    myassert(strcmp(buffer,"abcdefghi") == 0 && length == 9);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    printMessage("outdata: %s (%ld)\n", buffer, length);
    myassert(strcmp(buffer,"abcdefghij") == 0 && length == 10);

    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA_FOUND);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


/* For data truncation */
DECLARE_TEST(t_prep_truncate)
{
    SQLINTEGER pcrow;
    SQLLEN length, length1;
    SQLCHAR    name[20], bin[10];


    SQLExecDirect(hstmt,"drop table t_prep_truncate",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table t_prep_truncate(a int, b char(4), c binary(4))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt, "insert into t_prep_truncate values(500,'venu','venu')",SQL_NTS);
    mystmt(hstmt,rc);

    strcpy(name,"venu");
    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 10, 10, name, 5, NULL);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_BINARY,SQL_BINARY, 10, 10, name, 5, NULL);
    mystmt(hstmt,rc);

    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);

    rc = SQLRowCount(hstmt, &pcrow);
    mystmt(hstmt,rc);

    printMessage( "affected rows: %ld\n", pcrow);
    myassert(pcrow == 1);

    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"select b,c from t_prep_truncate",SQL_NTS);
    mystmt(hstmt,rc);  

    rc = SQLBindCol(hstmt, 1, SQL_C_CHAR, name, 2, &length);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt, 2, SQL_C_BINARY, bin, 4, &length1);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    printMessage("str outdata: %s(%d)\n",name,length);
    myassert(strcmp(name,"v")==0);
    myassert(length == 4);

    bin[4]='M';
    printMessage("bin outdata: %s(%d)\n",bin,length1);
    myassert(strncmp(bin,"venuM",5)==0);
    myassert(length == 4);

    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA_FOUND);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


/* For scrolling */
DECLARE_TEST(t_prep_scroll)
{
    SQLINTEGER i, data, max_rows= 5;


    SQLExecDirect(hstmt,"drop table t_prep_scroll",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table t_prep_scroll(a tinyint)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"insert into t_prep_scroll values(?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_LONG,SQL_TINYINT,
                          0,0,&i,0,NULL);
    mystmt(hstmt,rc);

    for (i= 1; i <= max_rows; i++)
    {
        rc = SQLExecute(hstmt);
        mystmt(hstmt,rc);
    }

    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"select * from t_prep_scroll",SQL_NTS);
    mystmt(hstmt,rc);  

    rc = SQLBindCol(hstmt, 1, SQL_C_LONG, &data, 0, NULL);
    mystmt(hstmt,rc);

    for (i=1; ;i++)
    {
        rc = SQLFetchScroll(hstmt,SQL_FETCH_NEXT,0);
        if (rc == SQL_NO_DATA)
            break;
        mystmt(hstmt,rc);

        printMessage("row %ld    : %ld\n", i, data);
        myassert(data == i);
    }
    printMessage("total rows fetched: %ld\n\n", i-1);
    myassert( i == max_rows+1);

    printMessage("scrolling:\n");
    rc = SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, 3);
    mystmt(hstmt,rc);

    printMessage("absolute 3 : %ld\n", data);
    myassert(data == 3);

    rc = SQLFetchScroll(hstmt, SQL_FETCH_PREV, 3);
    mystmt(hstmt,rc);

    printMessage("previous   : %ld\n", data);
    myassert(data == 2);

    rc = SQLFetchScroll(hstmt, SQL_FETCH_FIRST, 3);
    mystmt(hstmt,rc);

    printMessage("first      : %ld\n", data);
    myassert(data == 1);

    rc = SQLFetchScroll(hstmt, SQL_FETCH_PREV, 3);
    printMessage("previous   : %s", (rc == SQL_NO_DATA) ? "SQL_NO_DATA" : "SQL_ERROR;\n");
    myassert(rc == SQL_NO_DATA);  

    rc = SQLFetchScroll(hstmt, SQL_FETCH_RELATIVE, -2);
    printMessage("relative -2: %s", (rc == SQL_NO_DATA) ? "SQL_NO_DATA" : "SQL_ERROR;\n");
    myassert(rc == SQL_NO_DATA);  

    rc = SQLFetchScroll(hstmt, SQL_FETCH_RELATIVE, 2);
    mystmt(hstmt,rc);

    printMessage("relative 2 : %ld\n", data);
    myassert(data == 2);

    rc = SQLFetchScroll(hstmt, SQL_FETCH_LAST, 3);
    mystmt(hstmt,rc);

    printMessage("last       : %ld\n", data);
    myassert(data == 5);

    rc = SQLFetchScroll(hstmt, SQL_FETCH_RELATIVE, -2);
    mystmt(hstmt,rc);

    printMessage("relative -2: %ld\n", data);
    myassert(data == 3);

    rc = SQLFetchScroll(hstmt, SQL_FETCH_RELATIVE, 3);
    printMessage("relative -2: %s", (rc == SQL_NO_DATA) ? "SQL_NO_DATA" : "SQL_ERROR;\n");
    myassert(rc == SQL_NO_DATA);

    rc = SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 3);
    printMessage("next       : %s", (rc == SQL_NO_DATA) ? "SQL_NO_DATA" : "SQL_ERROR;\n");
    myassert(rc == SQL_NO_DATA);    

    rc = SQLFetchScroll(hstmt, SQL_FETCH_RELATIVE, -2);
    mystmt(hstmt,rc);

    printMessage("relative -2: %ld\n", data);
    myassert(data == 4);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


/* For SQLGetData */
DECLARE_TEST(t_prep_getdata)
{
    SQLCHAR    name[10];
    SQLINTEGER data;
    SQLLEN     length;
    SQLCHAR    tiny;

    SQLExecDirect(hstmt,"drop table t_prep_getdata",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table t_prep_getdata(a tinyint, b int, c char(4))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"insert into t_prep_getdata values(?,?,?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_LONG,SQL_TINYINT,
                          0,0,&data,0,NULL);

    rc = SQLBindParameter(hstmt,2,SQL_PARAM_INPUT,SQL_C_LONG,SQL_INTEGER,
                          0,0,&data,0,NULL);

    rc = SQLBindParameter(hstmt,3,SQL_PARAM_INPUT,SQL_C_CHAR,SQL_CHAR,
                          10,10,name,6,NULL);
    mystmt(hstmt,rc);

    sprintf(name,"venu"); data = 10;

    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);

    rc = SQLExecute(hstmt);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    data= 0;
    rc = SQLExecDirect(hstmt,"select * from t_prep_getdata",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt, 1,SQL_C_TINYINT, &tiny, 0, NULL);
    mystmt(hstmt, rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt, rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt, rc);

    printMessage("record 1 : %d\n", tiny);
    myassert( tiny == 10);

    rc = SQLGetData(hstmt,2,SQL_C_LONG,&data,0,NULL);
    mystmt(hstmt,rc);

    printMessage("record 2 : %ld\n", data);
    myassert( data == 10);

    name[0]= '\0';
    rc = SQLGetData(hstmt,3,SQL_C_CHAR,name,5,&length);
    mystmt(hstmt,rc);

    printMessage("record 3 : %s(%ld)\n", name, (long)length);

    myassert(strcmp(name,"venu")== 0 && length == 4);

    data = 0;
    rc = SQLGetData(hstmt,1,SQL_C_LONG,&data,0,NULL);
    mystmt(hstmt,rc);

    printMessage("record 1 : %ld\n", data);
    myassert( data == 10);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


/* For SQLGetData in truncation */
DECLARE_TEST(t_prep_getdata1)
{
    SQLCHAR     data[11];
    SQLLEN length;

    SQLExecDirect(hstmt,"drop table t_prep_getdata",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table t_prep_getdata(a char(10), b int)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into t_prep_getdata values('abcdefghij',12345)",SQL_NTS);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt,SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"select * from t_prep_getdata",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt, rc);

    data[0]= 'M'; data[1]= '\0';
    rc = SQLGetData(hstmt,1,SQL_C_CHAR,data,0,&length);
    mystmt(hstmt,rc);

    printMessage("data: %s (%ld)\n", data, length);
    myassert(strcmp(data,"M") == 0 && length == 10);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,data,4,&length);
    mystmt(hstmt,rc);

    printMessage("data: %s (%ld)\n", data, length);
    myassert(strcmp(data,"abc") == 0 && length == 10);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,data,4,&length);
    mystmt(hstmt,rc);

    printMessage("data: %s (%ld)\n", data, length);
    myassert(strcmp(data,"def") == 0 && length == 7);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,data,4,&length);
    mystmt(hstmt,rc);

    printMessage("data: %s (%ld)\n", data, length);
    myassert(strcmp(data,"ghi") == 0 && length == 4);

    data[0]= 'M';
    rc = SQLGetData(hstmt,1,SQL_C_CHAR,data,0,&length);
    mystmt(hstmt,rc);

    printMessage("data: %s (%ld)\n", data, length);
    myassert(data[0] == 'M' && length == 1);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,data,1,&length);
    mystmt(hstmt,rc);

    printMessage("data: %s (%ld)\n", data, length);
    myassert(data[0] == '\0' && length == 1);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,data,2,&length);
    mystmt(hstmt,rc);

    printMessage("data: %s (%ld)\n", data, length);
    myassert(strcmp(data,"j") == 0 && length == 1);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,data,2,&length);
    myassert(rc == SQL_NO_DATA);

    data[0]= 'M'; data[1]= '\0';
    rc = SQLGetData(hstmt,2,SQL_C_CHAR,data,0,&length);
    mystmt(hstmt,rc);

    printMessage("data: %s (%ld)\n", data, length);
    myassert(strcmp(data,"M") == 0 && length == 5);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,data,3,&length);
    mystmt(hstmt,rc);

    printMessage("data: %s (%ld)\n", data, length);
    myassert(strcmp(data,"12") == 0 && length == 5);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,data,2,&length);
    mystmt(hstmt,rc);

    printMessage("data: %s (%ld)\n", data, length);
    myassert(strcmp(data,"3") == 0 && length == 3);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,data,2,&length);
    mystmt(hstmt,rc);

    printMessage("data: %s (%ld)\n", data, length);
    myassert(strcmp(data,"4") == 0 && length == 2);

    data[0]= 'M';
    rc = SQLGetData(hstmt,2,SQL_C_CHAR,data,0,&length);
    mystmt(hstmt,rc);

    printMessage("data: %s (%ld)\n", data, length);
    myassert(data[0] == 'M' && length == 1);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,data,1,&length);
    mystmt(hstmt,rc);

    printMessage("data: %s (%ld)\n", data, length);
    myassert(data[0] == '\0' && length == 1);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,data,2,&length);
    mystmt(hstmt,rc);

    printMessage("data: %s (%ld)\n", data, length);
    myassert(strcmp(data,"5") == 0 && length == 1);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,data,2,&length);
    myassert(rc == SQL_NO_DATA);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_prep_catalog)
{
    SQLCHAR     table[20];
    SQLINTEGER  length;

    SQLExecDirect(hstmt,"drop table t_prep_catalog",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table t_prep_catalog(a int default 100)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLTables(hstmt,NULL,0,NULL,0,"t_prep_catalog",14,"TABLE",5);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt,3,SQL_C_CHAR,table,0,&length);
    mystmt(hstmt,rc);
    myassert(length == 14);

    rc = SQLGetData(hstmt,3,SQL_C_CHAR,table,15,&length);
    mystmt(hstmt,rc);
    myassert(strcmp(table,"t_prep_catalog") == 0 && length == 14);

    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

    rc = SQLColumns(hstmt,NULL,0,NULL,0,"t_prep_catalog",14,NULL,0);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt,3,SQL_C_CHAR,table,15,&length);
    mystmt(hstmt,rc);
    myassert(strcmp(table,"t_prep_catalog") == 0 && length == 14);

    rc = SQLGetData(hstmt,4,SQL_C_CHAR,table,0,&length);
    mystmt(hstmt,rc);
    myassert(length == 1);

    rc = SQLGetData(hstmt,4,SQL_C_CHAR,table,2,&length);
    mystmt(hstmt,rc);
    myassert(strcmp(table,"a") == 0 && length == 1);

    rc = SQLGetData(hstmt,13,SQL_C_CHAR,table,10,&length);
    mystmt(hstmt,rc);
    printMessage("table: %s(%d)\n", table, length);
    myassert(strcmp(table,"100") == 0 && length == 3);

    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);

  return OK;
}


DECLARE_TEST(t_sps)
{
    SQLINTEGER a, a1;
    SQLLEN length, length1;
    char b[]= "abcdefghij", b1[10];

/*
    if (!mysql_min_version(hdbc, "5.0",3))
    {
        printMessage("server doesn't support stored procedures..skipped\n");
        return;
    }
*/

    SQLExecDirect(hstmt,"drop procedure t_sp",SQL_NTS);

    SQLExecDirect(hstmt,"drop table t_tabsp",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table t_tabsp(a int, b varchar(10))",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"create procedure t_sp(x int, y char(10)) \
                              begin \
                                insert into t_tabsp values(x, y); \
                              end;",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLPrepare(hstmt,"call t_sp(?,?)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindParameter(hstmt,1,SQL_PARAM_INPUT,SQL_C_LONG,SQL_INTEGER,
                          0,0,&a,0,NULL);

    rc = SQLBindParameter(hstmt,2,SQL_PARAM_INPUT,SQL_C_CHAR,SQL_CHAR,
                          0,0,b,0,&length);


    for (length= 0, a= 0; a < 10; a++, length++)
    {
        rc = SQLExecute(hstmt);
        mystmt(hstmt, rc);
    }

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"select * from t_tabsp",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,1,SQL_C_LONG,&a,0,NULL);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,2,SQL_C_CHAR,b1,11,&length);
    mystmt(hstmt,rc);

    for (length1= 0, a1= 0; a1 < 10; a1++, length1++)
    {
        rc = SQLFetch(hstmt);
        mystmt(hstmt, rc);

        printMessage( "data: %d, %s(%d)\n", a, b1, length);
        myassert( a == a1);
        myassert(strncmp(b1,b,length1) == 0 && length1 == length);
    }

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLExecDirect(hstmt,"drop procedure t_sp",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"drop table t_tabsp",SQL_NTS);
    mystmt(hstmt,rc);
    SQLFreeStmt(hstmt, SQL_CLOSE);

  return OK;
}


BEGIN_TESTS
  ADD_TEST(t_prep_basic)
  ADD_TEST(t_prep_buffer_length)
  ADD_TEST(t_prep_truncate)
  ADD_TEST(t_prep_scroll)
  ADD_TEST(t_prep_getdata)
  ADD_TEST(t_prep_getdata1)
  ADD_TEST(t_prep_catalog)
  ADD_TEST(t_sps)
END_TESTS


RUN_TESTS
