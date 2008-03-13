/* Copyright (C) 2000-2005 MySQL AB

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   There are special exceptions to the terms and conditions of the GPL as it
   is applied to this software. View the full text of the exception in file
   EXCEPTIONS in the directory of this software distribution.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/*!
    \file		A mysql-like console interface which uses ODBC exclusively.
*/  

#include "myodbc3m.h"
#ifdef HAVE_READLINE
    #include <readline/readline.h>
    #include <readline/history.h>
#endif

#ifdef HAVE_SETLOCALE
    #ifdef HAVE_LOCALE_H
        #include <locale.h>
    #endif 
#endif

int     bDriverConnect = 0;
int     bVerbose       = 0;
int     nUserWidth     = 0;
SQLHENV hEnv           = 0;
SQLHDBC hDbc           = 0;

int main( int argc, char *argv[] )
{
    int     nArg, count;
    int     bHTMLTable                  = 0;
    int     bBatch                      = 0;
    int     cDelimiter                  = 0;
    int     bColumnNames                = 0;
    char    szDSN[MAX_DATA_WIDTH+1]     = "";
    char    szUID[MAX_DATA_WIDTH+1]     = "";
    char    szPWD[MAX_DATA_WIDTH+1]     = "";
    char    *szSQL;
    char    *pEscapeChar;
    int     buffer_size = 9000;

    if ( argc < 2 )
    {
        fprintf( stderr, szSyntax );
        exit( 1 );
    }

#ifdef HAVE_SETLOCALE
    /*
     * Default locale
     */
    setlocale( LC_ALL, "" );
#endif

    /****************************
     * PARSE ARGS
     ***************************/
    for ( nArg = 1, count = 1 ; nArg < argc; nArg++ )
    {
        if ( argv[nArg][0] == '-' )
        {
            /* Options */
            switch ( argv[nArg][1] )
            {
                case 'd':
                    cDelimiter = argv[nArg][2];
                    break;
                case 'm':
                    nUserWidth = atoi( &(argv[nArg][2]) );
                    break;
                case 'r':
                    bDriverConnect = 1;
                    break;
                case 's':
                    buffer_size = atoi( &(argv[nArg][2]) );
                    break;
                case 'w':
                    bHTMLTable = 1;
                    break;
                case 'b':
                    bBatch = 1;
                    break;
                case 'c':
                    bColumnNames = 1;
                    break;
                case 'v':
                    bVerbose = 1;
                    break;
                case '-':
                    printf( "myodbc3m\n" );
                    exit(0);
#ifdef HAVE_STRTOL
                case 'x':
                    cDelimiter = strtol( argv[nArg]+2, NULL, 0 );
                    break;
#endif
#ifdef HAVE_SETLOCALE
                case 'l':
                    if ( !setlocale( LC_ALL, argv[nArg]+2 ))
                    {
                        fprintf( stderr, "myodbc3m: can't set locale to '%s'\n", argv[nArg]+2 );
                        exit ( -1 );
                    }
                    break;
#endif

                default:
                    fprintf( stderr, szSyntax );
                    exit( 1 );
            }
            continue;
        }
        else if ( count == 1 )
            strncpy( szDSN, argv[nArg], MAX_DATA_WIDTH );
        else if ( count == 2 )
            strncpy( szUID, argv[nArg], MAX_DATA_WIDTH );
        else if ( count == 3 )
            strncpy( szPWD, argv[nArg], MAX_DATA_WIDTH );
        count++;
    }

    szSQL = calloc( 1, buffer_size + 1 );

    /****************************
     * CONNECT
     ***************************/
    if ( !OpenEnvironment( &hEnv ) )
        exit( 1 );
    
    if ( !OpenDatabase( hEnv, &hDbc, szDSN, szUID, szPWD ) )
        exit( 1 );

    /****************************
     * EXECUTE
     ***************************/
    if ( !bBatch )
    {
        printf( "+---------------------------------------+\n" );
        printf( "| Connected!                            |\n" );
        printf( "|                                       |\n" );
        printf( "| sql-statement                         |\n" );
        printf( "| help [tablename]                      |\n" );
        printf( "| quit                                  |\n" );
        printf( "|                                       |\n" );
        printf( "+---------------------------------------+\n" );
    }
    do
    {
        if ( !bBatch )
#ifndef HAVE_READLINE
            printf( "SQL> " );
#else
        {
            char *line;
            int malloced;

            line=readline("SQL> ");
            if ( !line )        /* EOF - ctrl D */
            {
                malloced = 1;
                line = strdup( "quit" );
            }
            else
            {
                malloced = 0;
            }
            strncpy(szSQL, line, buffer_size );
            add_history(line);
            if ( malloced )
            {
                free(line);
            }
        }
        else
#endif
        {
            char *line;
            int malloced;

            line = fgets( szSQL, buffer_size, stdin );
            if ( !line )        /* EOF - ctrl D */
            {
                malloced = 1;
                line = (char*)strdup( "quit" );
            }
            else
            {
                malloced = 0;
            }
            strncpy(szSQL, line, buffer_size );
            if ( malloced )
            {
                free(line);
            }
        }

        /* strip away escape chars */
        while ( (pEscapeChar=(char*)strchr(szSQL, '\n')) != NULL || (pEscapeChar=(char*)strchr(szSQL, '\r')) != NULL )
            *pEscapeChar = ' ';

        if ( szSQL[1] != '\0' )
        {
            if ( strncmp( szSQL, "quit", 4 ) == 0 )
                szSQL[1] = '\0';
            else if ( strncmp( szSQL, "help", 4 ) == 0 )
                ExecuteHelp( hDbc, szSQL, cDelimiter, bColumnNames, bHTMLTable );
            else if (memcmp(szSQL, "--", 2) != 0)
                ExecuteSQL( hDbc, szSQL, cDelimiter, bColumnNames, bHTMLTable );
        }

    } while ( szSQL[1] != '\0' );

    /****************************
     * DISCONNECT
     ***************************/
    CloseDatabase( hDbc );
    CloseEnvironment( hEnv );

    exit( 0 );
}

#define     INI_NO_DATA             2
#define     INI_SUCCESS             1
#define     INI_ERROR               0

int iniElement( char *pszData, char cSeperator, char cTerminator, int nElement, char *pszElement, int nMaxElement )
{
    int nCurElement         = 0;
    int nChar               = 0;
    int nCharInElement      = 0;

    memset( pszElement, '\0', nMaxElement );
    for ( ; nCurElement <= nElement && (nCharInElement+1) < nMaxElement; nChar++ )
    {
        /* check for end of data */
        if ( cSeperator != cTerminator && pszData[nChar] == cTerminator )
        {
            break;
        }

        if ( cSeperator == cTerminator && pszData[nChar] == cSeperator && pszData[nChar+1] == cTerminator )
        {
            break;
        }

        /* check for end of element */
        if ( pszData[nChar] == cSeperator )
        {
            nCurElement++;
        }
        else if ( nCurElement == nElement )
        {
            pszElement[nCharInElement] = pszData[nChar];
            nCharInElement++;
        }
    }

    if ( pszElement[0] == '\0' )
    {
        return INI_NO_DATA;
    }

    return INI_SUCCESS;
}

/****************************
 * OptimalDisplayWidth
 ***************************/
SQLUINTEGER OptimalDisplayWidth( SQLHSTMT hStmt, SQLUSMALLINT nCol, SQLUINTEGER nUserWidth )
{
    SQLUINTEGER nLabelWidth                     = 10;
    SQLLEN      nDataWidth                      = 10;
    SQLUINTEGER nOptimalDisplayWidth            = 10;
    SQLCHAR     szColumnName[MAX_DATA_WIDTH+1]  = "";   

    SQLColAttribute( hStmt, nCol, SQL_DESC_DISPLAY_SIZE, NULL, 0, NULL, &nDataWidth );
    SQLColAttribute( hStmt, nCol, SQL_DESC_LABEL, szColumnName, sizeof(szColumnName), NULL, NULL );
    nLabelWidth = strlen((char*) szColumnName );

    nOptimalDisplayWidth = myodbc_max( nLabelWidth, nDataWidth );

    if ( nUserWidth > 0 )
        nOptimalDisplayWidth = myodbc_min( nOptimalDisplayWidth, nUserWidth );

    if ( nOptimalDisplayWidth > MAX_DATA_WIDTH )
        nOptimalDisplayWidth = MAX_DATA_WIDTH;

    return nOptimalDisplayWidth;
}

/****************************
 * OpenEnvironment 
 ***************************/
int OpenEnvironment( SQLHENV *phEnv )
{
    /* this really has no affect here because we just use one connection but we can use it for testing with just a little help */
    /*
    if ( SQLSetEnvAttr( NULL, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER)SQL_CP_ONE_PER_HENV, 0 ) != SQL_SUCCESS )
        fprintf( stderr, "[myodbc3m]WARNING: Could not turn on pooling\n" );
    */

    if ( SQLAllocHandle( SQL_HANDLE_ENV, NULL, phEnv ) != SQL_SUCCESS )
    {
        fprintf( stderr, "[myodbc3m]ERROR: Could not SQLAllocEnv\n" );
        return 0;
    }

    SQLSetEnvAttr( *phEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0 );

    return 1;
}

/****************************
 * OpenDatabase - do everything we have to do to get a viable connection to szDSN
 ***************************/
int OpenDatabase( SQLHENV hEnv, SQLHDBC *phDbc, char *szDSN, char *szUID, char *szPWD )
{
    SQLRETURN nReturn;

    if ( SQLAllocHandle( SQL_HANDLE_DBC, hEnv, phDbc ) != SQL_SUCCESS )
    {
        if ( bVerbose ) DumpODBCLog( hEnv, 0, 0 );
        fprintf( stderr, "[myodbc3m]ERROR: Could not SQLAllocConnect\n" );
        return 0;
    }
    if ( bDriverConnect )
    {
        SQLCHAR         szOut[4096];
        SQLSMALLINT     nLen;
        HWND            hWnd = 0;
                                
        nReturn = SQLDriverConnect( *phDbc, hWnd, (SQLCHAR*)szDSN, strlen(szDSN), szOut, 4096, &nLen, SQL_DRIVER_NOPROMPT );
    }
    else
    {
        nReturn = SQLConnect( *phDbc, (SQLCHAR*)szDSN, SQL_NTS, (SQLCHAR*)szUID, SQL_NTS, (SQLCHAR*)szPWD, SQL_NTS );
    }
    if ( !SQL_SUCCEEDED( nReturn ) )
    {
        if ( bVerbose ) DumpODBCLog( hEnv, hDbc, 0 );
        fprintf( stderr, "[myodbc3m]ERROR: Could not SQLConnect\n" );
        SQLFreeConnect( *phDbc );
        return 0;
    }

    return 1;
}

/****************************
 * ExecuteSQL - create a statement, execute the SQL, and get rid of the statement
 *            - show results as per request; bHTMLTable has precedence over other options
 ***************************/
int ExecuteSQL( SQLHDBC hDbc, char *szSQL, char cDelimiter, int bColumnNames, int bHTMLTable )
{
    SQLHSTMT        hStmt;
    SQLCHAR         szSepLine[32001] ;
    SQLSMALLINT     cols;
    SQLINTEGER          nRows                   = 0;
    SQLINTEGER      ret;

    /****************************
     * EXECUTE SQL
     ***************************/
    if ( SQLAllocStmt( hDbc, &hStmt ) != SQL_SUCCESS )
    {
        if ( bVerbose ) DumpODBCLog( hEnv, hDbc, 0 );
        fprintf( stderr, "[myodbc3m]ERROR: Could not SQLAllocStmt\n" );
        return 0;
    }

    if ( SQLPrepare( hStmt, (SQLCHAR*)szSQL, SQL_NTS ) != SQL_SUCCESS )
    {
        if ( bVerbose ) DumpODBCLog( hEnv, hDbc, hStmt );
        fprintf( stderr, "[myodbc3m]ERROR: Could not SQLPrepare\n" );
        SQLFreeStmt( hStmt, SQL_DROP );
        return 0;
    }

    ret =  SQLExecute( hStmt );

    if ( ret == SQL_NO_DATA )
    {
        fprintf( stderr, "[myodbc3m]INFO: SQLExecute returned SQL_NO_DATA\n" );
    }
    else if ( ret == SQL_SUCCESS_WITH_INFO )
    {
        if ( bVerbose ) DumpODBCLog( hEnv, hDbc, hStmt );
        fprintf( stderr, "[myodbc3m]INFO: SQLExecute returned SQL_SUCCESS_WITH_INFO\n" );
    }
    else if ( ret != SQL_SUCCESS )
    {
        if ( bVerbose ) DumpODBCLog( hEnv, hDbc, hStmt );
        fprintf( stderr, "[myodbc3m]ERROR: Could not SQLExecute\n" );
        SQLFreeStmt( hStmt, SQL_DROP );
        return 0;
    }

    /*
     * Loop while SQLMoreResults returns success
     */

    do
    {
        strcpy ( (char *)szSepLine, "" ) ;

        /*
         * check to see if it has generated a result set
         */

        if ( SQLNumResultCols( hStmt, &cols ) != SQL_SUCCESS )
        {
            if ( bVerbose ) DumpODBCLog( hEnv, hDbc, hStmt );
            fprintf( stderr, "[myodbc3m]ERROR: Could not SQLNunResultCols\n" );
            SQLFreeStmt( hStmt, SQL_DROP );
            return 0;
        }

        if ( cols > 0 )
        {
            /****************************
             * WRITE HEADER
             ***************************/
            if ( bHTMLTable )
                WriteHeaderHTMLTable( hStmt );
            else if ( cDelimiter == 0 )
                WriteHeaderNormal( hStmt, szSepLine );
            else if ( cDelimiter && bColumnNames )
                WriteHeaderDelimited( hStmt, cDelimiter );

            /****************************
             * WRITE BODY
             ***************************/
            if ( bHTMLTable )
                WriteBodyHTMLTable( hStmt );
            else if ( cDelimiter == 0 )
                nRows = WriteBodyNormal( hStmt );
            else
                WriteBodyDelimited( hStmt, cDelimiter );
        }

        /****************************
         * WRITE FOOTER
         ***************************/
        if ( bHTMLTable )
            WriteFooterHTMLTable( hStmt );
        else if ( cDelimiter == 0 )
            WriteFooterNormal( hStmt, szSepLine, nRows );
    }
    while ( SQL_SUCCEEDED( SQLMoreResults( hStmt ) ) );

    /****************************
     * CLEANUP
     ***************************/
    SQLFreeStmt( hStmt, SQL_DROP );

    return 1;
}

/****************************
 * ExecuteHelp - create a statement, execute the SQL, and get rid of the statement
 *             - show results as per request; bHTMLTable has precedence over other options
 ***************************/
int ExecuteHelp( SQLHDBC hDbc, char *szSQL, char cDelimiter, int bColumnNames, int bHTMLTable )
{
    char            szTable[250]        = "";
    SQLHSTMT        hStmt;
    SQLCHAR         szSepLine[32001]    = "";   
    SQLINTEGER          nRows               = 0;
    SQLRETURN       nReturn;

    /****************************
     * EXECUTE SQL
     ***************************/
    if ( SQLAllocStmt( hDbc, &hStmt ) != SQL_SUCCESS )
    {
        if ( bVerbose ) DumpODBCLog( hEnv, hDbc, 0 );
        fprintf( stderr, "[myodbc3m]ERROR: Could not SQLAllocStmt\n" );
        return 0;
    }

    if ( iniElement( szSQL, ' ', '\0', 1, szTable, sizeof(szTable) ) == INI_SUCCESS )
    {
        /* COLUMNS */
        nReturn = SQLColumns( hStmt, NULL, 0, NULL, 0, (SQLCHAR*)szTable, SQL_NTS, NULL, 0 );
        if ( (nReturn != SQL_SUCCESS) && (nReturn != SQL_SUCCESS_WITH_INFO) )
        {
            if ( bVerbose ) DumpODBCLog( hEnv, hDbc, hStmt );
            fprintf( stderr, "[myodbc3m]ERROR: Could not SQLColumns\n" );
            SQLFreeStmt( hStmt, SQL_DROP );
            return 0;
        }
    }
    else
    {
        /* TABLES */
        nReturn = SQLTables( hStmt, NULL, 0, NULL, 0, NULL, 0, NULL, 0 );
        if ( (nReturn != SQL_SUCCESS) && (nReturn != SQL_SUCCESS_WITH_INFO) )
        {
            if ( bVerbose ) DumpODBCLog( hEnv, hDbc, hStmt );
            fprintf( stderr, "[myodbc3m]ERROR: Could not SQLTables\n" );
            SQLFreeStmt( hStmt, SQL_DROP );
            return 0;
        }
    }

    /****************************
     * WRITE HEADER
     ***************************/
    if ( bHTMLTable )
        WriteHeaderHTMLTable( hStmt );
    else if ( cDelimiter == 0 )
        WriteHeaderNormal( hStmt, szSepLine );
    else if ( cDelimiter && bColumnNames )
        WriteHeaderDelimited( hStmt, cDelimiter );

    /****************************
     * WRITE BODY
     ***************************/
    if ( bHTMLTable )
        WriteBodyHTMLTable( hStmt );
    else if ( cDelimiter == 0 )
        nRows = WriteBodyNormal( hStmt );
    else
        WriteBodyDelimited( hStmt, cDelimiter );

    /****************************
     * WRITE FOOTER
     ***************************/
    if ( bHTMLTable )
        WriteFooterHTMLTable( hStmt );
    else if ( cDelimiter == 0 )
        WriteFooterNormal( hStmt, szSepLine, nRows );

    /****************************
     * CLEANUP
     ***************************/
    SQLFreeStmt( hStmt, SQL_DROP );

    return 1;
}


/****************************
 * CloseEnvironment
 ***************************/
int CloseEnvironment( SQLHENV hEnv )
{
    SQLFreeEnv( hEnv );

    return 1;
}

/****************************
 * CloseDatabase - cleanup in prep for exiting the program
 ***************************/
int CloseDatabase( SQLHDBC hDbc )
{
    SQLDisconnect( hDbc );
    SQLFreeConnect( hDbc );

    return 1;
}


/****************************
 * WRITE HTML
 ***************************/
void WriteHeaderHTMLTable( SQLHSTMT hStmt )
{
    SQLUSMALLINT    nCol                            = 0;
    SQLSMALLINT     nColumns                        = 0;
    SQLCHAR         szColumnName[MAX_DATA_WIDTH+1]  = "";   

    printf( "<table BORDER>\n" );
    printf( "<tr BGCOLOR=#000099>\n" );

    if ( SQLNumResultCols( hStmt, &nColumns ) != SQL_SUCCESS )
        nColumns = -1;

    for ( nCol = 1; nCol <= nColumns; nCol++ )
    {
        SQLColAttribute( hStmt, nCol, SQL_DESC_LABEL, szColumnName, sizeof(szColumnName), NULL, NULL );
        printf( "<td>\n" );
        printf( "<font face=Arial,Helvetica><font color=#FFFFFF>\n" );
        printf( "%s\n", szColumnName );
        printf( "</font></font>\n" );
        printf( "</td>\n" );
    }
    printf( "</tr>\n" );
}

void WriteBodyHTMLTable( SQLHSTMT hStmt )
{
    SQLUSMALLINT    nCol                            = 0;
    SQLSMALLINT     nColumns                        = 0;
    SQLLEN          nIndicator                      = 0;
    SQLCHAR         szColumnValue[MAX_DATA_WIDTH+1] = "";
    SQLRETURN       nReturn                         = 0;
    SQLRETURN       ret;

    if ( SQLNumResultCols( hStmt, &nColumns ) != SQL_SUCCESS )
        nColumns = -1;

    while ( (ret = SQLFetch( hStmt )) == SQL_SUCCESS ) /* ROWS */
    {
        printf( "<tr>\n" );

        for ( nCol = 1; nCol <= nColumns; nCol++ ) /* COLS */
        {
            printf( "<td>\n" );
            printf( "<font face=Arial,Helvetica>\n" );

            nReturn = SQLGetData( hStmt, nCol, SQL_C_CHAR, (SQLPOINTER)szColumnValue, sizeof(szColumnValue), &nIndicator );
            if ( nReturn == SQL_SUCCESS && nIndicator != SQL_NULL_DATA )
            {
                fputs((char*) szColumnValue, stdout );
            }
            else if ( nReturn == SQL_ERROR )
            {
                ret = SQL_ERROR;
                break;
            }
            else
                printf( "%s\n", "" );

            printf( "</font>\n" );
            printf( "</td>\n" );
        }
        if (ret != SQL_SUCCESS)
            break;
        printf( "</tr>\n" );
    }
}

void WriteFooterHTMLTable( SQLHSTMT hStmt __attribute__((unused)))
{
    printf( "</table>\n" );
}

/****************************
 * WRITE DELIMITED
 * - this output can be used by the ODBC Text File driver
 * - last column no longer has a delimit char (it is implicit)...
 *   this is consistent with odbctxt
 ***************************/
void WriteHeaderDelimited( SQLHSTMT hStmt, char cDelimiter )
{
    SQLUSMALLINT    nCol                            = 0;
    SQLSMALLINT     nColumns                        = 0;
    SQLCHAR         szColumnName[MAX_DATA_WIDTH+1]  = "";   

    if ( SQLNumResultCols( hStmt, &nColumns ) != SQL_SUCCESS )
        nColumns = -1;

    for ( nCol = 1; nCol <= nColumns; nCol++ )
    {
        SQLColAttribute( hStmt, nCol, SQL_DESC_LABEL, szColumnName, sizeof(szColumnName), NULL, NULL );
        fputs((char*) szColumnName, stdout );
        if ( nCol < nColumns )
            putchar( cDelimiter );
    }
    putchar( '\n' );
}

void WriteBodyDelimited( SQLHSTMT hStmt, char cDelimiter )
{
    SQLUSMALLINT    nCol                            = 0;
    SQLSMALLINT     nColumns                        = 0;
    SQLLEN          nIndicator                      = 0;
    SQLCHAR         szColumnValue[MAX_DATA_WIDTH+1] = "";
    SQLRETURN       nReturn                         = 0;
    SQLRETURN       ret;

    if ( SQLNumResultCols( hStmt, &nColumns ) != SQL_SUCCESS )
        nColumns = -1;

    /* ROWS */
    while (( ret = SQLFetch( hStmt )) == SQL_SUCCESS )
    {
        /* COLS */
        for ( nCol = 1; nCol <= nColumns; nCol++ )
        {
            nReturn = SQLGetData( hStmt, nCol, SQL_C_CHAR, (SQLPOINTER)szColumnValue, sizeof(szColumnValue), &nIndicator );
            if ( nReturn == SQL_SUCCESS && nIndicator != SQL_NULL_DATA )
            {
                fputs((char*) szColumnValue, stdout );
                if ( nCol < nColumns )
                    putchar( cDelimiter );
            }
            else if ( nReturn == SQL_ERROR )
            {
                ret = SQL_ERROR;
                break;
            }
            else
            {
                if ( nCol < nColumns )
                    putchar( cDelimiter );
            }
        }
        if (ret != SQL_SUCCESS)
            break;
        printf( "\n" );
    }
    if ( ret == SQL_ERROR )
    {
        if ( bVerbose ) DumpODBCLog( 0, 0, hStmt );
    }
}

/****************************
 * WRITE NORMAL
 ***************************/
void WriteHeaderNormal( SQLHSTMT hStmt, SQLCHAR *szSepLine )
{
    SQLUSMALLINT    nCol                            = 0;
    SQLSMALLINT     nColumns                        = 0;
    SQLCHAR         szColumn[MAX_DATA_WIDTH+20]     = "";   
    SQLCHAR         szColumnName[MAX_DATA_WIDTH+1]  = "";   
    SQLCHAR         szHdrLine[32001]                = "";   
    SQLUINTEGER     nOptimalDisplayWidth            = 10;

    if ( SQLNumResultCols( hStmt, &nColumns ) != SQL_SUCCESS )
        nColumns = -1;

    for ( nCol = 1; nCol <= nColumns; nCol++ )
    {
        nOptimalDisplayWidth = OptimalDisplayWidth( hStmt, nCol, nUserWidth );
        SQLColAttribute( hStmt, nCol, SQL_DESC_LABEL, szColumnName, sizeof(szColumnName), NULL, NULL );

        /* SEP */
        memset( szColumn, '\0', sizeof(szColumn) );
        memset( szColumn, '-', nOptimalDisplayWidth + 1 );
        strcat((char*) szSepLine, "+" );
        strcat((char*) szSepLine,(char*) szColumn );

        /* HDR */
        sprintf( (char*)szColumn, "| %-*.*s", (int)nOptimalDisplayWidth, (int)nOptimalDisplayWidth, szColumnName );
        strcat( (char*)szHdrLine,(char*) szColumn );
    }
    strcat((char*) szSepLine, "+\n" );
    strcat((char*) szHdrLine, "|\n" );

    printf((char*) szSepLine );
    printf((char*) szHdrLine );
    printf((char*) szSepLine );

}

SQLINTEGER WriteBodyNormal( SQLHSTMT hStmt )
{
    SQLUSMALLINT    nCol                            = 0;
    SQLSMALLINT     nColumns                        = 0;
    SQLLEN          nIndicator                      = 0;
    SQLCHAR         szColumn[MAX_DATA_WIDTH+20]     = "";
    SQLCHAR         szColumnValue[MAX_DATA_WIDTH+1] = "";
    SQLRETURN       nReturn                         = 0;
    SQLINTEGER      nRows                           = 0;
    SQLUINTEGER     nOptimalDisplayWidth            = 10;

    nReturn = SQLNumResultCols( hStmt, &nColumns );
    if ( nReturn != SQL_SUCCESS && nReturn != SQL_SUCCESS_WITH_INFO )
        nColumns = -1;

    /* ROWS */
    nReturn = SQLFetch( hStmt );
    while ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
    {
        /* COLS */
        for ( nCol = 1; nCol <= nColumns; nCol++ )
        {
            nOptimalDisplayWidth = OptimalDisplayWidth( hStmt, nCol, nUserWidth );
            nReturn = SQLGetData( hStmt, nCol, SQL_C_CHAR, (SQLPOINTER)szColumnValue, sizeof(szColumnValue), &nIndicator );
            szColumnValue[MAX_DATA_WIDTH] = '\0';

            if ( nReturn == SQL_SUCCESS && nIndicator != SQL_NULL_DATA )
            {
              sprintf( (char*)szColumn, "| %-*.*s", (int)nOptimalDisplayWidth, (int)nOptimalDisplayWidth, szColumnValue );
            }
            else if ( nReturn == SQL_ERROR )
            {
                break;
            }
            else
            {
              sprintf( (char*)szColumn, "| %-*s", (int)nOptimalDisplayWidth, "" );
            }
            fputs( (char*)szColumn, stdout );
        } /* for columns */

        nRows++;
        printf( "|\n" );
        nReturn = SQLFetch( hStmt );
    } /* while rows */

    if ( nReturn == SQL_ERROR )
    {
        if ( bVerbose ) DumpODBCLog( 0, 0, hStmt );
    }

    return nRows;
}

void WriteFooterNormal( SQLHSTMT hStmt, SQLCHAR *szSepLine, SQLINTEGER nRows )
{
    SQLLEN     nRowsAffected    = -1;

    printf( (char*)szSepLine );

    SQLRowCount( hStmt, &nRowsAffected );
    printf( "SQLRowCount returns %ld\n", nRowsAffected );

    if ( nRows )
    {
        printf( "%d rows fetched\n", (int)nRows );
    }
}



int DumpODBCLog( SQLHENV hEnv, SQLHDBC hDbc, SQLHSTMT hStmt )
{
    SQLCHAR     szError[501];
    SQLCHAR     szSqlState[10];
    SQLINTEGER  nNativeError;
    SQLSMALLINT nErrorMsg;

    if ( hStmt )
    {
        while ( SQLError( hEnv, hDbc, hStmt, szSqlState, &nNativeError, szError, 500, &nErrorMsg ) == SQL_SUCCESS )
        {
            printf( "[%s]%s\n", szSqlState, szError );
        }
    }

    if ( hDbc )
    {
        while ( SQLError( hEnv, hDbc, 0, szSqlState, &nNativeError, szError, 500, &nErrorMsg ) == SQL_SUCCESS )
        {
            printf( "[%s]%s\n", szSqlState, szError );
        }
    }

    if ( hEnv )
    {
        while ( SQLError( hEnv, 0, 0, szSqlState, &nNativeError, szError, 500, &nErrorMsg ) == SQL_SUCCESS )
        {
            printf( "[%s]%s\n", szSqlState, szError );
        }
    }

    return 1;
}

