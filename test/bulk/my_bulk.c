/***************************************************************************
                          my_bulk.c  -  description
                             -------------------
    begin                : Fri Nov 16 2001
    copyright            : (C) MySQL AB 1997-2001
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

#define MAX_INSERT_COUNT 800

/**
BULK CHECK
*/
#define MAX_TXT_LENGTH 10
void t_bulk_check(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN  rc;
    SQLCHAR    ltxt[MAX_TXT_LENGTH];

    SQLExecDirect(hstmt, "DROP TABLE t_bulk_check", SQL_NTS);

    rc = SQLExecDirect(hstmt, "CREATE TABLE t_bulk_check(id int PRIMARY KEY, ltxt text)", SQL_NTS);
    mystmt(hstmt, rc);

    SQLFreeStmt(hstmt, SQL_CLOSE);

    rc = SQLPrepare(hstmt, "INSERT INTO t_bulk_check(id,ltxt) values(1,?)", SQL_NTS);
    mystmt(hstmt, rc);

    rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR,
                          0,0,ltxt,MAX_TXT_LENGTH,NULL);
    mystmt(hstmt, rc);

    memset(ltxt, 'E', MAX_TXT_LENGTH);
    ltxt[MAX_TXT_LENGTH] = '\0';

    rc = SQLExecute(hstmt);
    mystmt(hstmt, rc);

    SQLFreeStmt(hstmt, SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt, SQL_CLOSE);    

    rc = SQLExecDirect(hstmt, "SELECT * FROM t_bulk_check", SQL_NTS);
    mystmt(hstmt, rc);

    my_assert(1 == my_print_non_format_result(hstmt));

    SQLFreeStmt(hstmt, SQL_CLOSE);

    SQLExecDirect(hstmt, "DROP TABLE t_bulk_check", SQL_NTS);
    SQLFreeStmt(hstmt, SQL_CLOSE);
}

/**
BULK INSERT
*/
void t_bulk_insert(SQLHDBC hdbc, SQLHSTMT hstmt)
{
    SQLRETURN rc;
    SQLINTEGER i,id[MAX_INSERT_COUNT+1];
    char    name[MAX_INSERT_COUNT][40],txt[MAX_INSERT_COUNT][60],ltxt[MAX_INSERT_COUNT][70];
    clock_t start, end;
    double duration,dbl[MAX_INSERT_COUNT];
    double dt;

    SQLExecDirect(hstmt, "DROP TABLE my_bulk", SQL_NTS);

    rc = SQLExecDirect(hstmt, "CREATE TABLE my_bulk ( id   int,          " \
                              "                       v    varchar(100), " \
                              "                       txt  text,         " \
                              "                       ft   float(10),    " \
                              "                       ltxt long varchar )", SQL_NTS);
    mystmt(hstmt, rc);

    SQLFreeStmt(hstmt, SQL_CLOSE);

    dt = 0.23456;

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_STATIC, 0);
    mystmt(hstmt, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)MAX_INSERT_COUNT, 0);
    mystmt(hstmt, rc);

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY , (SQLPOINTER)SQL_CONCUR_ROWVER , 0);
    mystmt(hstmt, rc);

    rc = SQLBindCol(hstmt, 1, SQL_C_LONG, &id[0], 0, NULL);
    mystmt(hstmt, rc);

    rc = SQLBindCol(hstmt, 2, SQL_C_CHAR, &name[0], 30 /* sizeof(name[0]) */, NULL);
    mystmt(hstmt, rc);

    rc = SQLBindCol(hstmt, 3, SQL_C_CHAR, &txt[0], sizeof(txt[0]), NULL);
    mystmt(hstmt, rc);

    rc = SQLBindCol(hstmt, 4, SQL_C_DOUBLE, &dbl[0], sizeof(dbl[0]), NULL);
    mystmt(hstmt, rc);

    rc = SQLBindCol(hstmt, 5, SQL_C_CHAR, &ltxt[0], sizeof(ltxt[0]), NULL);
    mystmt(hstmt, rc);

    rc = SQLExecDirect(hstmt, "SELECT id, v, txt, ft, ltxt FROM my_bulk", SQL_NTS);
    mystmt(hstmt, rc);

    rc = SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 0);
    mystmt_err(hstmt, rc == SQL_NO_DATA_FOUND, rc);

    for (i= 0; i < MAX_INSERT_COUNT; i++)
    {
        id[i]=i;
        dbl[i] = i+dt;      
        sprintf( name[i], "Varchar%d", i );      
        sprintf( txt[i], "Text%d", i );      
        sprintf( ltxt[i], "LongText, id row:%d", i );
    }    

    printMessage( "\n total bulk adds   : %d", MAX_INSERT_COUNT*2);
    start = clock();    

    rc = SQLBulkOperations(hstmt, SQL_ADD);    
    mystmt(hstmt, rc);

    rc = SQLBulkOperations(hstmt, SQL_ADD);    
    mystmt(hstmt, rc);

    end = clock();

    duration = difftime(end,start)/CLOCKS_PER_SEC;
    printMessage( " (in '%lf' secs)", duration);

    SQLFreeStmt(hstmt, SQL_UNBIND);
    SQLFreeStmt(hstmt, SQL_CLOSE);    

    rc = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)1, 0);
    mystmt(hstmt, rc);    

    start= clock();
    rc = SQLExecDirect(hstmt, "SELECT * FROM my_bulk", SQL_NTS);
    mystmt(hstmt, rc);
    my_assert(MAX_INSERT_COUNT*2 == myrowcount(hstmt));
    end= clock();

    duration = difftime(end,start)/CLOCKS_PER_SEC;
    printMessage(" (in '%lf' secs)\n", duration);

    SQLExecDirect(hstmt, "DROP TABLE my_bulk", SQL_NTS);
    SQLFreeStmt(hstmt, SQL_CLOSE);
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
    if (driver_supports_setpos(hdbc))
    {
        t_bulk_check(hdbc,hstmt);
        t_bulk_insert(hdbc,hstmt);      /* bulk inserts */
        t_bulk_insert(hdbc,hstmt);      /* bulk inserts */
        t_bulk_insert(hdbc,hstmt);      /* bulk inserts */
    }
    mydisconnect(&henv,&hdbc,&hstmt);

    printMessageFooter( 1 );

    return(0);
}
