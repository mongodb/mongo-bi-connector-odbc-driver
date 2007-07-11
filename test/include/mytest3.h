/***************************************************************************
                          mytest3.h  -  description
                             -------------------
    begin                : Wed Aug 8 2001
    copyright            : (C) 2001 by Venu, MySQL AB
    email                : venu@mysql.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __TMYODBC__TEST__H
#define __TMYODBC__TEST__H

#ifdef HAVE_CONFIG_H
# include <myconf.h>
#endif
/* Work around iODBC header bug on Mac OS X 10.3 */
#undef HAVE_CONFIG_H

#ifdef WIN32
# include <windows.h>
#endif

/* STANDARD C HEADERS */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* ODBC HEADERS */
#include <sql.h>
#include <sqlext.h>

/* for clock() */
#include <time.h>

#ifndef bool
# define bool unsigned char
#endif

#ifndef true
# define true 1
#endif

#ifndef false
# define false 0
#endif

#ifndef my_assert
# define my_assert assert
#endif

#ifndef myassert
# define myassert assert
#endif

#if DEBUG_LEVEL > 1
# define printMessage printf
#else
void printMessage(const char *fmt, ...) { fmt= fmt; /* stifle warning */}
#endif

#define MAX_NAME_LEN 255
#define MAX_COLUMNS 500
#define MAX_ROW_DATA_LEN 1000
#define MYSQL_NAME_LEN 64

SQLCHAR *mydsn= (SQLCHAR *)"test";
SQLCHAR *myuid= (SQLCHAR *)"root";
SQLCHAR *mypwd= (SQLCHAR *)"";
SQLCHAR *mysock= NULL;
int      myoption= 0;
SQLCHAR *myserver= (SQLCHAR *)"localhost";
SQLCHAR *mydb= (SQLCHAR *)"test";
SQLCHAR *test_db= (SQLCHAR *)"client_odbc_test";

int g_nCursor;
char g_szHeader[501];

SQLCHAR init_db[3][100]=
{
    "DROP DATABASE IF EXISTS client_odbc_test",
    "CREATE DATABASE client_odbc_test",
    "USE client_odbc_test"
};

/* PROTOTYPE */
void myerror( SQLRETURN rc, SQLSMALLINT htype, SQLHANDLE handle, const char *szFile, int nLine );

#define my_error() fprintf(stdout," ERROR occured at %d@%s\n",__LINE__,__FILE__)

SQLUINTEGER myresult(SQLHSTMT hstmt);

/* UTILITY MACROS */
#define myenv(henv,r)  \
        if ( ((r) != SQL_SUCCESS) ) \
            myerror( r, 1, henv,  __FILE__, __LINE__ ); \
        my_assert( ((r) == SQL_SUCCESS) || ((r) == SQL_SUCCESS_WITH_INFO) )

#define myenv_r(henv,r)  \
        if ( r == SQL_ERROR ) \
            myerror( r, 1, henv,  __FILE__, __LINE__ ); \
        my_assert( r == SQL_ERROR )

#define myenv_err(henv,r,rc)  \
        if ( rc == SQL_ERROR || rc == SQL_SUCCESS_WITH_INFO ) \
            myerror( rc, 1, henv,  __FILE__, __LINE__ ); \
        my_assert( r )

#define mycon(hdbc,r)  \
        if ( ((r) != SQL_SUCCESS) ) \
            myerror( r, 2, hdbc,  __FILE__, __LINE__ ); \
        my_assert( ((r) == SQL_SUCCESS) || ((r) == SQL_SUCCESS_WITH_INFO) )

#define mycon_r(hdbc,r)  \
        if ( r == SQL_ERROR ) \
            myerror( r, 2, hdbc,  __FILE__, __LINE__ ); \
        my_assert(r==SQL_ERROR)

#define mycon_err(hdbc,r,rc)  \
        if ( rc == SQL_ERROR || rc == SQL_SUCCESS_WITH_INFO ) \
            myerror( rc, 2, hdbc, __FILE__, __LINE__ ); \
        my_assert( r )

#define mystmt(hstmt,r)  \
        if ( ((r) != SQL_SUCCESS) ) \
            myerror( r, 3, hstmt,  __FILE__, __LINE__ ); \
        my_assert( ((r) == SQL_SUCCESS) || ((r) == SQL_SUCCESS_WITH_INFO) )

#define mystmt_r(hstmt,r)  \
        if ( r == SQL_ERROR ) \
            myerror( r, 3, hstmt,  __FILE__, __LINE__ ); \
        my_assert( r == SQL_ERROR )

#define mystmt_err(hstmt,r,rc)  \
        if ( rc == SQL_ERROR || rc == SQL_SUCCESS_WITH_INFO ) \
            myerror( rc, 3, hstmt, __FILE__, __LINE__ ); \
        my_assert( r )

#if defined(USE_C99_FUNC_MACRO)
#define printMessageHeader()\
    {\
        g_nCursor = sprintf( g_szHeader, "[%s][%s][%d]\n", __FILE__, __func__, __LINE__ );\
        fprintf( stdout, g_szHeader );\
    }
#elif defined(USE_GNU_FUNC_MACRO)
#define printMessageHeader()\
    {\
        g_nCursor = sprintf( g_szHeader, "[%s][%s][%d]\n", __FILE__, __FUNCTION__, __LINE__ );\
        fprintf( stdout, g_szHeader );\
    }
#else
#define printMessageHeader()\
    {\
        g_nCursor = sprintf( g_szHeader, "[%s][%d]\n", __FILE__, __LINE__ );\
        fprintf( stdout, g_szHeader );\
    }
#endif

#define printMessageFooter( A )\
    {\
        int nDot = g_nCursor;\
        if ( g_nCursor < 1 )\
            fprintf( stdout, "\n" );\
        for ( ; nDot < 71; nDot++ )\
        {\
            fprintf( stdout, "." );\
        }\
        if ( A )\
            fprintf( stdout, "[  OK  ]\n" );\
        else\
            fprintf( stdout, "[FAILED]\n" );\
    }


#define IS_NUM(t) (if
/**
ERROR HANDLER
*/
void myerror( SQLRETURN rc, SQLSMALLINT htype, SQLHANDLE handle, const char *szFile, int nLine )
{
    RETCODE lrc;

    if ( !SQL_SUCCEEDED( rc ) )
    {
        SQLCHAR      szSqlState[6],szErrorMsg[SQL_MAX_MESSAGE_LENGTH];
        SQLINTEGER   pfNativeError;
        SQLSMALLINT  pcbErrorMsg;

        lrc = SQLGetDiagRec(htype, handle,1,    
                            (SQLCHAR *)&szSqlState,
                            (SQLINTEGER *)&pfNativeError,
                            (SQLCHAR *)&szErrorMsg,
                            SQL_MAX_MESSAGE_LENGTH-1,
                            (SQLSMALLINT *)&pcbErrorMsg);
        if ( SQL_SUCCEEDED( lrc ) )
            fprintf( stdout, "\n\n[%s][%d][%s]%s\n\n", szFile, nLine, szSqlState, szErrorMsg );
        else
            fprintf( stdout, "\n\n[%s][%d] SQLGetDiagRec returned :%d, but rc = %d\n\n", szFile, nLine, lrc, rc );

        g_nCursor = 0;
    }
}

/**
  DRV CONNECTION
*/
void mydrvconnect(SQLHENV *henv,SQLHDBC *hdbc, SQLHSTMT *hstmt,SQLCHAR *connIn)
{
    SQLRETURN rc;
    SQLCHAR   connOut[MAX_NAME_LEN];
    SQLSMALLINT StringLength;

    rc = SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,henv);
    myenv(*henv,rc);   

    rc = SQLSetEnvAttr(*henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0);
    myenv(*henv,rc);   

    rc = SQLAllocHandle(SQL_HANDLE_DBC,*henv, hdbc);
    myenv(*henv,rc);    

    printMessage(" Connecting to '%s' \n",connIn);
    rc = SQLDriverConnect(*hdbc,NULL,connIn,MAX_NAME_LEN,
                          connOut,MAX_NAME_LEN,&StringLength,SQL_DRIVER_NOPROMPT);
    mycon(*hdbc,rc);
    printMessage( "output connection string: %s\n", connOut);

    rc = SQLSetConnectAttr(*hdbc,SQL_ATTR_AUTOCOMMIT,(SQLPOINTER)SQL_AUTOCOMMIT_ON,0);
    mycon(*hdbc,rc);

    rc = SQLAllocHandle(SQL_HANDLE_STMT,*hdbc,hstmt);
    mycon(*hdbc,rc);

    {
        size_t i;
        for (i=0; i< sizeof(init_db)/sizeof(init_db[0]); i++)
            SQLExecDirect(*hstmt, (SQLCHAR *)init_db[i], SQL_NTS);
    }
    SQLFreeStmt(*hstmt, SQL_CLOSE);
    SQLSetStmtAttr(*hstmt,SQL_ATTR_CURSOR_TYPE,(SQLPOINTER)SQL_CURSOR_STATIC,0);
    SQLSetStmtOption(*hstmt,SQL_SIMULATE_CURSOR,SQL_SC_NON_UNIQUE);
    SQLSetStmtOption(*hstmt, SQL_CURSOR_TYPE, SQL_CURSOR_KEYSET_DRIVEN);
}

/**
  Print resultset dashes
*/
#if DEBUG_LEVEL > 1
static void my_print_dashes(SQLHSTMT hstmt, SQLSMALLINT nCol)
{
    SQLRETURN  rc;
    SQLLEN     disp_size, nullable;
    SQLCHAR    ColName[MAX_NAME_LEN+1];
    SQLUSMALLINT field_count, i, j;
    SQLSMALLINT  col_len;

    field_count= (SQLUSMALLINT)nCol;
    printMessage( "\t" );
    printMessage( "+" );

    for (i=1; i<= field_count; i++)
    {
        nullable = 0;
        rc = SQLColAttribute(hstmt,i,SQL_DESC_BASE_COLUMN_NAME,&ColName,
                             MAX_NAME_LEN,&col_len,NULL);
        mystmt(hstmt,rc);
        rc = SQLColAttribute(hstmt,i,SQL_DESC_DISPLAY_SIZE,NULL,0,
                             NULL,&disp_size);
        mystmt(hstmt,rc);
        rc = SQLColAttribute(hstmt,i,SQL_DESC_NULLABLE,NULL,0,
                             NULL,&nullable);
        mystmt(hstmt,rc);

        if (disp_size < col_len)
            disp_size = col_len;
        if (disp_size < 4 && nullable)
            disp_size = 4;
        for (j=1; j < disp_size+3; j++)
            printMessage( "-" );
        printMessage( "+" );
    }
    printMessage( "\n" );
}
#else
# define my_print_dashes(a, b)
#endif


#if DEBUG_LEVEL > 1
static void my_print_data(SQLHSTMT hstmt, SQLUSMALLINT index,
                          SQLCHAR *data, SQLINTEGER length)
{
    SQLRETURN  rc;
    SQLLEN     disp_size, nullable;
    SQLCHAR    ColName[MAX_NAME_LEN+1];
    SQLSMALLINT col_len;

    nullable = 0;
    rc = SQLColAttribute(hstmt,index,SQL_DESC_BASE_COLUMN_NAME,&ColName,
                         MAX_NAME_LEN,&col_len,NULL);
    mystmt(hstmt,rc);
    rc = SQLColAttribute(hstmt,index,SQL_DESC_DISPLAY_SIZE,NULL,0,
                         NULL,&disp_size);
    mystmt(hstmt,rc);
    rc = SQLColAttribute(hstmt,index,SQL_DESC_NULLABLE,NULL,0,
                         NULL,&nullable);
    mystmt(hstmt,rc);

    if (disp_size < length)
        disp_size = length;
    if (disp_size < col_len)
        disp_size = col_len;
    if (disp_size < 4 && nullable)
        disp_size = 4;
    if (length == SQL_NULL_DATA)
        printMessage( "%-*s  |",(int)disp_size, "NULL");
    else
        printMessage( "%-*s  |",(int)disp_size,data);
}
#else
# define my_print_data(a, b, c, d)
#endif


/**
RESULT SET
*/
int my_print_non_format_result(SQLHSTMT hstmt)
{
    SQLRETURN   rc;
    SQLUINTEGER nRowCount=0;
    SQLULEN     pcColDef;
    SQLCHAR     szColName[MAX_NAME_LEN];
    SQLCHAR     szData[MAX_COLUMNS][MAX_ROW_DATA_LEN]={{0}};
    SQLSMALLINT nIndex,ncol,pfSqlType, pcbScale, pfNullable;

    rc = SQLNumResultCols(hstmt,&ncol);
    mystmt(hstmt,rc);

    printMessage("\n");

    for (nIndex = 1; nIndex <= ncol; nIndex++)
    {
        rc = SQLDescribeCol(hstmt,nIndex,szColName, MAX_NAME_LEN+1, NULL,
                            &pfSqlType,&pcColDef,&pcbScale,&pfNullable);
        mystmt(hstmt,rc);

        printMessage("%s\t",szColName);

        rc = SQLBindCol(hstmt,nIndex, SQL_C_CHAR, szData[nIndex-1],
                        MAX_ROW_DATA_LEN+1,NULL);
        mystmt(hstmt,rc);
    }

    printMessage("\n------------------------------------------------------\n");

    rc = SQLFetch(hstmt);
    while (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    {
        nRowCount++;
        for (nIndex=0; nIndex< ncol; nIndex++)
            printMessage("%s\t",szData[nIndex]);

        printMessage("\n");
        rc = SQLFetch(hstmt);
    }

    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    printMessage("Total rows fetched: %d\n",(int)nRowCount);
    return(nRowCount);
}

/**
  RESULT SET
*/
SQLUINTEGER myresult(SQLHSTMT hstmt)
{
    SQLRETURN   rc;
    SQLUINTEGER nRowCount;
    SQLCHAR     ColName[MAX_NAME_LEN+1];
    SQLCHAR     Data[MAX_ROW_DATA_LEN+1];
    SQLLEN      pcbLength, size;
    SQLUSMALLINT nIndex;
    SQLSMALLINT  ncol;

    rc = SQLNumResultCols(hstmt,&ncol);
    mystmt(hstmt,rc);

    if (ncol > 10)
        return my_print_non_format_result(hstmt);

    printMessage( "\n" );
    my_print_dashes(hstmt, ncol);
    printMessage( "\t" );
    printMessage( "|" );

    for (nIndex = 1; nIndex <= ncol; nIndex++)
    {
        rc = SQLColAttribute(hstmt,nIndex,SQL_DESC_BASE_COLUMN_NAME,ColName,MAX_NAME_LEN,
                             NULL,NULL);
        mystmt(hstmt,rc);
        my_print_data(hstmt,nIndex,ColName,0);
    }
    printMessage( "\n" );
    my_print_dashes(hstmt, ncol);

    nRowCount= 0;

    rc = SQLFetch(hstmt);
    while (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    {
        nRowCount++;
        printMessage( "\t" );
        printMessage( "|" );

        for (nIndex=1; nIndex<= ncol; nIndex++)
        {
            rc = SQLColAttribute(hstmt,nIndex,SQL_DESC_DISPLAY_SIZE,NULL,0,
                                 NULL,&size);
            mystmt(hstmt,rc);
            rc = SQLGetData(hstmt, nIndex, SQL_C_CHAR, Data,
                            MAX_ROW_DATA_LEN,&pcbLength);
            mystmt(hstmt,rc);
            my_print_data(hstmt, nIndex, Data, pcbLength);
        }
        printMessage( "\t" );
        printMessage( "\n" );
        rc = SQLFetch(hstmt);
    }
    my_print_dashes(hstmt,ncol);
    SQLFreeStmt(hstmt,SQL_UNBIND);
    SQLFreeStmt(hstmt,SQL_CLOSE);

    printMessage("Total rows fetched: %d\n",(int)nRowCount);
    return nRowCount;
}

/**
  ROWCOUNT
*/
SQLUINTEGER myrowcount(SQLHSTMT hstmt)
{
    SQLRETURN   rc;
    SQLUINTEGER nRowCount= 0;

    rc = SQLFetch(hstmt);
    while (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    {
        nRowCount++;
        rc = SQLFetch(hstmt);
    }
    SQLFreeStmt(hstmt,SQL_UNBIND);
    printMessage("Total rows fetched: %d\n",(int)nRowCount);
    return(nRowCount);
}
/**
  SQLExecDirect
*/
SQLRETURN tmysql_exec(SQLHSTMT hstmt, char *sql_stmt)
{
    return(SQLExecDirect(hstmt,(SQLCHAR *)sql_stmt,SQL_NTS));
}
/**
  SQLPrepare
*/
SQLRETURN tmysql_prepare(SQLHSTMT hstmt, char *sql_stmt)
{
    return(SQLPrepare(hstmt, (SQLCHAR *)sql_stmt, SQL_NTS));
}
/**
  return integer data by fetching it
*/
SQLINTEGER my_fetch_int(SQLHSTMT hstmt, SQLUSMALLINT irow)
{
    SQLINTEGER nData;

    SQLGetData(hstmt,irow,SQL_INTEGER,&nData,0,NULL);
    printMessage(" my_fetch_int: %ld\n", (long int)nData);
    return(nData);
}
/**
  return string data, by fetching it
*/
const char *my_fetch_str(SQLHSTMT hstmt, SQLCHAR *szData,SQLUSMALLINT irow)
{
    SQLLEN nLen;

    SQLGetData(hstmt,irow,SQL_CHAR,szData,MAX_ROW_DATA_LEN+1,&nLen);
    printMessage( " my_fetch_str: %s(%ld)\n",szData,nLen);
    return((const char *)szData);
}

/*
  Check if server running is MySQL
*/
bool server_is_mysql(SQLHDBC hdbc)
{
    char driver_name[MYSQL_NAME_LEN];

    SQLGetInfo(hdbc,SQL_DRIVER_NAME,driver_name,MYSQL_NAME_LEN,NULL);

    if (strncmp(driver_name,"myodbc",6) >= 0 || strncmp(driver_name,"libmyodbc",9) >= 0)
        return true;

    return false;
}

/*
  Check if driver supports SQLSetPos
*/
bool driver_supports_setpos(SQLHDBC hdbc)
{
    SQLRETURN    rc;
    SQLUSMALLINT status= true;

    rc = SQLGetFunctions(hdbc, SQL_API_SQLSETPOS, &status);
    mycon(hdbc,rc);

    if (!status)
        return false;
    return true;
}

/*
  Check for minimal MySQL version
*/
bool mysql_min_version(SQLHDBC hdbc, char *min_version, unsigned int length)
{
    SQLCHAR server_version[MYSQL_NAME_LEN];
    SQLRETURN rc;

    if (!server_is_mysql(hdbc))
        return true;

    rc = SQLGetInfo(hdbc,SQL_DBMS_VER,server_version,MYSQL_NAME_LEN,NULL);
    mycon(hdbc, rc);

    if (strncmp((char *)server_version, min_version, length) >= 0)
        return true;

    return false;
}

/*
  Check for minimal Connector/ODBC version
*/
bool driver_min_version(SQLHDBC hdbc, char *min_version, unsigned int length)
{
    SQLCHAR driver_version[MYSQL_NAME_LEN];
    SQLRETURN rc;

    if (!server_is_mysql(hdbc))
        return true;

    rc = SQLGetInfo(hdbc,SQL_DRIVER_VER,driver_version,MYSQL_NAME_LEN,NULL);
    mycon(hdbc, rc);

    if (strncmp((char *)driver_version, min_version, length) >= 0)
        return true;

    return false;
}

/*
  Check if server supports transactions
*/
bool server_supports_trans(SQLHDBC hdbc)
{
    SQLSMALLINT trans;
    SQLRETURN   rc;

    rc = SQLGetInfo(hdbc,SQL_TXN_CAPABLE,&trans,0,NULL);
    mycon(hdbc,rc);

    if (trans != SQL_TC_NONE)
        return true;
    return false;
}

#endif /* __TMYODBC__TEST__H */
