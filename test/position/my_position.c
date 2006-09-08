/***************************************************************************
                          my_position.c  -  description
                             -------------------
    begin                : Wed Nov 27 2001
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
#include "mytest3.h" /* MyODBC 3.51 sample utility header */

/**
Test SQL_POSITION
*/
void t_chunk(SQLHDBC hdbc,SQLHSTMT hstmt)
{
    SQLRETURN rc;
    SQLCHAR   txt[100];
    SQLINTEGER len;

    if (!driver_supports_setpos(hdbc))
        return;

    SQLExecDirect(hstmt,"drop table t_chunk",SQL_NTS);

    rc = SQLExecDirect(hstmt,"create table t_chunk(id int not null primary key, description varchar(50), txt text)",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"INSERT INTO t_chunk VALUES(1,'venu','Developer, MySQL AB')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"INSERT INTO t_chunk VALUES(2,'monty','Michael Monty Widenius - main MySQL developer')",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLExecDirect(hstmt,"INSERT INTO t_chunk VALUES(3,'mysql','MySQL AB- Speed, Power and Precision')",SQL_NTS);
    mystmt(hstmt,rc);

    SQLFreeStmt(hstmt,SQL_CLOSE);    

    rc = SQLExecDirect(hstmt,"SELECT * from t_chunk",SQL_NTS);
    mystmt(hstmt,rc);

    rc = SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 1);
    mystmt(hstmt,rc);

    rc = SQLSetPos(hstmt, 1, SQL_POSITION, SQL_LOCK_NO_CHANGE);
    mystmt(hstmt,rc);

    rc = SQLGetData(hstmt, 2, SQL_C_CHAR, txt, 100, &len);
    mystmt(hstmt,rc);
    printMessage("\ntxt:%s(%d)",txt,len);

    SQLFreeStmt(hstmt,SQL_RESET_PARAMS);
    SQLFreeStmt(hstmt,SQL_UNBIND);    
    SQLFreeStmt(hstmt,SQL_CLOSE);
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
    t_chunk(hdbc,hstmt);
    mydisconnect(&henv,&hdbc,&hstmt);

    printMessageFooter( 1 );

    return(0);
}


