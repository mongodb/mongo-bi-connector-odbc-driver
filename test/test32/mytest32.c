/***************************************************************************
                          mytest32.c  -  description
                             -------------------
    begin                : Thu May 01 2003
    copyright            : (C) MySQL AB 1997-2003
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

/***************************************************************************
 *                                                                         *
 *   Tests for Connector/ODBC 3.52                                         *  
 *                                                                         *
 ***************************************************************************/
#include "mytest3.h"


SQLHENV henv= NULL;
SQLHDBC hdbc= NULL;
SQLHSTMT hstmt= NULL;
SQLRETURN rc;

/*
  Basic prepared statements - binary protocol test
*/

void t_prep_basic()
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

    printMessage( "\n affected rows: %ld", pcrow);
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

    printMessage( "\n outdata: %d(%d), %s(%d)",id,length1,name,length2);
    myassert(id == 100 && length1 == sizeof(SQLINTEGER));
    myassert(strcmp(name,"venu")==0 && length2 == 4);

    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA_FOUND);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}

/*
  to test buffer length
*/

void t_prep_buffer_length()
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

    printMessage( "\n outdata: %s (%ld)", buffer, length);
    myassert(buffer[0] == '\0' && length == 0);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    printMessage("\n outdata: %s (%ld)", buffer, length);
    myassert(strcmp(buffer,"abc") == 0 && length == 3);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    printMessage("\n outdata: %s (%ld)", buffer, length);
    myassert(strcmp(buffer,"abcdefghij") == 0 && length == 10);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    printMessage("\n outdata: %s (%ld)", buffer, length);
    myassert(strcmp(buffer,"abcdefghi") == 0 && length == 9);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    printMessage("\n outdata: %s (%ld)", buffer, length);
    myassert(strcmp(buffer,"abcdefghij") == 0 && length == 10);

    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA_FOUND);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}

/*
  For data truncation
*/

void t_prep_truncate()
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

    printMessage( "\n affected rows: %ld", pcrow);
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

    printMessage("\n str outdata: %s(%d)",name,length);
    myassert(strcmp(name,"v")==0);
    myassert(length == 4);

    bin[4]='M';
    printMessage("\n bin outdata: %s(%d)",bin,length1);
    myassert(strncmp(bin,"venuM",5)==0);
    myassert(length == 4);

    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA_FOUND);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}

/*
  For scrolling
*/

void t_prep_scroll()
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

        printMessage("\n row %ld    : %ld", i, data);
        myassert(data == i);
    }
    printMessage("\n total rows fetched: %ld\n", i-1);
    myassert( i == max_rows+1);

    printMessage("\n scrolling:");
    rc = SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, 3);
    mystmt(hstmt,rc);

    printMessage("\n absolute 3 : %ld", data);
    myassert(data == 3);

    rc = SQLFetchScroll(hstmt, SQL_FETCH_PREV, 3);
    mystmt(hstmt,rc);

    printMessage("\n previous   : %ld", data);
    myassert(data == 2);

    rc = SQLFetchScroll(hstmt, SQL_FETCH_FIRST, 3);
    mystmt(hstmt,rc);

    printMessage("\n first      : %ld", data);
    myassert(data == 1);

    rc = SQLFetchScroll(hstmt, SQL_FETCH_PREV, 3);
    printMessage("\n previous   : %s", (rc == SQL_NO_DATA) ? "SQL_NO_DATA" : "SQL_ERROR;");
    myassert(rc == SQL_NO_DATA);  

    rc = SQLFetchScroll(hstmt, SQL_FETCH_RELATIVE, -2);
    printMessage("\n relative -2: %s", (rc == SQL_NO_DATA) ? "SQL_NO_DATA" : "SQL_ERROR;");
    myassert(rc == SQL_NO_DATA);  

    rc = SQLFetchScroll(hstmt, SQL_FETCH_RELATIVE, 2);
    mystmt(hstmt,rc);

    printMessage("\n relative 2 : %ld", data);
    myassert(data == 2);

    rc = SQLFetchScroll(hstmt, SQL_FETCH_LAST, 3);
    mystmt(hstmt,rc);

    printMessage("\n last       : %ld", data);
    myassert(data == 5);

    rc = SQLFetchScroll(hstmt, SQL_FETCH_RELATIVE, -2);
    mystmt(hstmt,rc);

    printMessage("\n relative -2: %ld", data);
    myassert(data == 3);

    rc = SQLFetchScroll(hstmt, SQL_FETCH_RELATIVE, 3);
    printMessage("\n relative -2: %s", (rc == SQL_NO_DATA) ? "SQL_NO_DATA" : "SQL_ERROR;");
    myassert(rc == SQL_NO_DATA);

    rc = SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 3);
    printMessage("\n next       : %s", (rc == SQL_NO_DATA) ? "SQL_NO_DATA" : "SQL_ERROR;");
    myassert(rc == SQL_NO_DATA);    

    rc = SQLFetchScroll(hstmt, SQL_FETCH_RELATIVE, -2);
    mystmt(hstmt,rc);

    printMessage("\n relative -2: %ld", data);
    myassert(data == 4);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}

/*
  For SQLGetData
*/

void t_prep_getdata()
{
    SQLCHAR    name[10];
    long       data;
    SQLINTEGER length;
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

    data = 0;
    rc = SQLExecDirect(hstmt,"select * from t_prep_getdata",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt, 1,SQL_C_TINYINT, &tiny, 0, NULL);
    mystmt(hstmt, rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt, rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt, rc);

    printMessage("\n record 1 : %d", tiny);
    myassert( tiny == 10);

    rc = SQLGetData(hstmt,2,SQL_C_LONG,&data,0,NULL);
    mystmt(hstmt,rc);

    printMessage("\n record 2 : %ld", data);
    myassert( data == 10);

    name[0]= '\0';
    rc = SQLGetData(hstmt,3,SQL_C_CHAR,name,5,&length);
    mystmt(hstmt,rc);

    /*
      FIXME how to do this test nicely? We don't know if SQLINTEGER is
      32 or 64 bits, and to use "%d" or "%ld"? Microsoft changed this.
    */
    printMessage("\n record 3 : %s(%d)", name, length);

    myassert(strcmp(name,"venu")== 0 && length == 4);

    data = 0;
    rc = SQLGetData(hstmt,1,SQL_C_LONG,&data,0,NULL);
    mystmt(hstmt,rc);

    printMessage("\n record 1 : %ld", data);
    myassert( data == 10);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}


/*
  For SQLGetData in truncation
*/

void t_prep_getdata1()
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

    printMessage("\n data: %s (%ld)", data, length);
    myassert(strcmp(data,"M") == 0 && length == 10);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,data,4,&length);
    mystmt(hstmt,rc);

    printMessage("\n data: %s (%ld)", data, length);
    myassert(strcmp(data,"abc") == 0 && length == 10);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,data,4,&length);
    mystmt(hstmt,rc);

    printMessage("\n data: %s (%ld)", data, length);
    myassert(strcmp(data,"def") == 0 && length == 7);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,data,4,&length);
    mystmt(hstmt,rc);

    printMessage("\n data: %s (%ld)", data, length);
    myassert(strcmp(data,"ghi") == 0 && length == 4);

    data[0]= 'M';
    rc = SQLGetData(hstmt,1,SQL_C_CHAR,data,0,&length);
    mystmt(hstmt,rc);

    printMessage("\n data: %s (%ld)", data, length);
    myassert(data[0] == 'M' && length == 1);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,data,1,&length);
    mystmt(hstmt,rc);

    printMessage("\n data: %s (%ld)", data, length);
    myassert(data[0] == '\0' && length == 1);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,data,2,&length);
    mystmt(hstmt,rc);

    printMessage("\n data: %s (%ld)", data, length);
    myassert(strcmp(data,"j") == 0 && length == 1);

    rc = SQLGetData(hstmt,1,SQL_C_CHAR,data,2,&length);
    myassert(rc == SQL_NO_DATA);

    data[0]= 'M'; data[1]= '\0';
    rc = SQLGetData(hstmt,2,SQL_C_CHAR,data,0,&length);
    mystmt(hstmt,rc);

    printMessage("\n data: %s (%ld)", data, length);
    myassert(strcmp(data,"M") == 0 && length == 5);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,data,3,&length);
    mystmt(hstmt,rc);

    printMessage("\n data: %s (%ld)", data, length);
    myassert(strcmp(data,"12") == 0 && length == 5);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,data,2,&length);
    mystmt(hstmt,rc);

    printMessage("\n data: %s (%ld)", data, length);
    myassert(strcmp(data,"3") == 0 && length == 3);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,data,2,&length);
    mystmt(hstmt,rc);

    printMessage("\n data: %s (%ld)", data, length);
    myassert(strcmp(data,"4") == 0 && length == 2);

    data[0]= 'M';
    rc = SQLGetData(hstmt,2,SQL_C_CHAR,data,0,&length);
    mystmt(hstmt,rc);

    printMessage("\n data: %s (%ld)", data, length);
    myassert(data[0] == 'M' && length == 1);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,data,1,&length);
    mystmt(hstmt,rc);

    printMessage("\n data: %s (%ld)", data, length);
    myassert(data[0] == '\0' && length == 1);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,data,2,&length);
    mystmt(hstmt,rc);

    printMessage("\n data: %s (%ld)", data, length);
    myassert(strcmp(data,"5") == 0 && length == 1);

    rc = SQLGetData(hstmt,2,SQL_C_CHAR,data,2,&length);
    myassert(rc == SQL_NO_DATA);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);
}

void t_prep_catalog()
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
    printMessage("\n table: %s(%d)", table, length);
    myassert(strcmp(table,"100") == 0 && length == 3);

    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA);

    rc = SQLFreeStmt(hstmt,SQL_UNBIND);
    mystmt(hstmt,rc);

    rc = SQLFreeStmt(hstmt,SQL_CLOSE);
    mystmt(hstmt,rc);


}

void t_sys_catalog()
{
#if ALLOW_CRAZY_TESTS
  /*
    This test is just broken -- it assumes you can pass a dotted table-name
    to SQLColumns. And it relies on the wacky behavior of SQLTables()
    returning such nonsense when you ask for system tables.
  */
    SQLHSTMT    hstmt_x;
    SQLCHAR     sys_table[MAX_NAME_LEN];
    SQLUINTEGER row_count= 0, columns;
    bool        grants_ok= server_supports_grant(hstmt);

    rc = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt_x);
    mycon(hdbc, rc);

    rc = SQLTables(hstmt,NULL,0,NULL,0,NULL,0,"SYSTEM TABLE",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLBindCol(hstmt,3,SQL_C_CHAR,sys_table,MAX_NAME_LEN,NULL);
    mystmt(hstmt,rc);

    rc = SQLFetchScroll(hstmt, SQL_FETCH_FIRST, 0);
    while ( rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    {
        printMessage( "\n system table %d: '%s'", ++row_count, sys_table);

        rc = SQLColumns(hstmt_x,NULL,0,NULL,0,sys_table,SQL_NTS,NULL,0);
        mystmt(hstmt_x,rc);

        columns= 0;
        while (1)
        {
            rc = SQLFetch(hstmt_x);
            if (rc == SQL_ERROR || rc == SQL_NO_DATA)
                break;
            columns++;
        }
        printMessage(" columns: '%ld'", columns);
        SQLFreeStmt(hstmt_x,SQL_CLOSE);

        if (grants_ok)
            myassert(columns != 0);
        rc = SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0);
    }
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt_x);
    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);
#endif
}

void t_catalog()
{
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
}

void t_rows_fetched_ptr()
{
    SQLINTEGER   rowsFetched, rowsSize;
    long         i;

    SQLExecDirect(hstmt,"drop table t_rows_fetched_ptr",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table t_rows_fetched_ptr(a int)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into t_rows_fetched_ptr values(0)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into t_rows_fetched_ptr values(1)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into t_rows_fetched_ptr values(2)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into t_rows_fetched_ptr values(3)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into t_rows_fetched_ptr values(4)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"insert into t_rows_fetched_ptr values(5)",SQL_NTS);
    mystmt(hstmt,rc);

    rowsSize= 1;
    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)rowsSize, 0);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROWS_FETCHED_PTR, &rowsFetched, 0);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt, "SELECT * FROM t_rows_fetched_ptr",SQL_NTS);
    mystmt(hstmt,rc);

    i= 0;
    rc = SQLFetchScroll(hstmt,SQL_FETCH_NEXT,0);
    while (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    {
        printMessage("\n total rows fetched: %ld", rowsFetched);
        myassert(rowsFetched == rowsSize);
        i++; rowsFetched= 0;
        rc = SQLFetch(hstmt);
    }
    myassert( i == 6);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rowsSize= 2;
    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)rowsSize, 0);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROWS_FETCHED_PTR, &rowsFetched, 0);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt, "SELECT * FROM t_rows_fetched_ptr",SQL_NTS);
    mystmt(hstmt,rc);

    i= 0;
    rc = SQLFetchScroll(hstmt,SQL_FETCH_NEXT,0);
    while (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    {
        printMessage("\n total rows fetched: %ld", rowsFetched);
        myassert(rowsFetched == rowsSize);
        i++;rowsFetched= 0;
        rc = SQLFetch(hstmt);
    }
    myassert( i == 3);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rowsSize= 3;
    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)rowsSize, 0);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROWS_FETCHED_PTR, &rowsFetched, 0);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt, "SELECT * FROM t_rows_fetched_ptr",SQL_NTS);
    mystmt(hstmt,rc);

    i= 0;
    rc = SQLFetchScroll(hstmt,SQL_FETCH_NEXT,0);
    while (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    {
        printMessage("\n total rows fetched: %ld", rowsFetched);
        myassert(rowsFetched == rowsSize);
        i++;rowsFetched= 0;
        rc = SQLFetch(hstmt);
    }
    myassert( i == 2);
    SQLFreeStmt(hstmt, SQL_CLOSE);

    rowsSize= 4;
    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)rowsSize, 0);
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROWS_FETCHED_PTR, &rowsFetched, 0);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt, "SELECT * FROM t_rows_fetched_ptr",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    printMessage("\n total rows fetched: %ld", rowsFetched);
    myassert(rowsFetched == rowsSize);

    rc = SQLFetch(hstmt);
    mystmt(hstmt,rc);

    printMessage("\n total rows fetched: %ld", rowsFetched);
    myassert(rowsFetched == 2);

    rc = SQLFetch(hstmt);
    myassert(rc == SQL_NO_DATA);

    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)1, 0);/* reset */
    mystmt(hstmt,rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROWS_FETCHED_PTR, NULL, 0);
    mystmt(hstmt,rc);
}

void t_sps()
{
    SQLINTEGER a, a1;
    SQLLEN length, length1;
    char b[]= "abcdefghij", b1[10];

/*
    if (!mysql_min_version(hdbc, "5.0",3))
    {
        printMessage("\n server doesn't support stored procedures..skipped");
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

        printMessage( "\n data: %d, %s(%d)", a, b1, length);
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
}

/*
  Main routine ..
*/

int main(int argc, char *argv[])
{
    SQLINTEGER narg;      

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

    myconnect(&henv,&hdbc,&hstmt);

    t_prep_catalog();
    t_sps();
    t_prep_buffer_length();
    t_prep_getdata();
    t_catalog();
    t_prep_catalog();
    t_prep_getdata1();
    t_prep_scroll();
    t_prep_basic();
    t_prep_truncate();
    t_rows_fetched_ptr();
    t_sys_catalog();

    mydisconnect(&henv,&hdbc,&hstmt);

    printMessageFooter( 1 );

    return(0);
}

