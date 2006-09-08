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

#include <VersionInfo.h>
#include "MYODBCCls.h"

/*!
    Entry point to odbc++
*/
int main( int argc, char *argv[] )
{
    ISQL    isql;

    if ( !isql.doParseArgs( argc, argv ) )
        return 1;

    if ( !isql.doProcess() )
        return 1;

    return 0;
}

ISQL::ISQL()
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT BEGIN\n", __FILE__, __LINE__ );
#endif

    bAsynch                 = false;
    bVerbose                = 0;
    nUserWidth              = 0;
	bHTMLTable              = 0;
	bBatch                  = 0;
	cDelimiter              = 0;
    bColumnNames            = 0;
    pszInsertTable          = 0;
    pszColumnsToQuote       = 0;
    pszQuoteToUse           = "\"";
    pszStatementTerminator  = ";";
	*szDSN                  = '\0';
	*szUID                  = '\0';
	*szPWD                  = '\0';
                            
    penvironment            = 0;
    pconnection             = 0;
    pstatement              = 0;

    // We always want to have an MYODBCEnvironment. We could create more than one in an application
    // but this seldom makes any sense.
    penvironment    = new MYODBCEnvironment();

    // We always want to have an MYODBCConnection. Some applications will want to create more than
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] CONSTRUCT END\n", __FILE__, __LINE__ );
#endif
}

ISQL::~ISQL()
{
#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT BEGIN\n", __FILE__, __LINE__ );
#endif

    // Cleanup. We just need to delete the MYODBCEnvironment. The MYODBCEnvironment will delete all
    // dependent objects (i.e. Connections and Statement) automatically.
    if ( penvironment )
        delete penvironment;

#ifdef DEBUG_DESTRUCTORS
    printf( "[PAH][%s][%d] DESTRUCT END\n", __FILE__, __LINE__ );
#endif
}

/*!
    getOptimalDisplayWidth
    
*/    
SQLUINTEGER ISQL::getOptimalDisplayWidth( SQLINTEGER nCol )
{
	SQLUINTEGER	nLabelWidth                     = 10;
	SQLUINTEGER	nDataWidth                      = 10;
	SQLUINTEGER	nOptimalDisplayWidth            = 10;
	SQLCHAR     szColumnName[MAX_DATA_WIDTH+1]	= "";	

    pstatement->getColAttribute( nCol, SQL_DESC_DISPLAY_SIZE, NULL, 0, NULL, &nDataWidth );
    pstatement->getColAttribute( nCol, SQL_DESC_LABEL, szColumnName, sizeof(szColumnName), NULL, NULL );
    nLabelWidth = strlen((char*) szColumnName );

    nOptimalDisplayWidth = max( nLabelWidth, nDataWidth );

    if ( nUserWidth > 0 )
        nOptimalDisplayWidth = min( nOptimalDisplayWidth, nUserWidth );

    if ( nOptimalDisplayWidth > MAX_DATA_WIDTH ) 
        nOptimalDisplayWidth = MAX_DATA_WIDTH;

    return nOptimalDisplayWidth;
}

/*!
    doParseArgs
    
*/    
bool ISQL::doParseArgs( int argc, char *argv[] )
{
	int nArg, count;

	if ( argc < 2 )
	{
		fprintf( stderr, szSyntax );
		return false;
	}

	for ( nArg = 1, count = 1 ; nArg < argc; nArg++ )
	{
		if ( argv[nArg][0] == '-' )
		{
			/* Options */
			switch ( argv[nArg][1] )
			{
			case 's':
				doExecuteShow( "dsn" );
				return false;
			case 'r':
				doExecuteShow( "driver" );
				return false;
			case 'd':
				cDelimiter = argv[nArg][2];
				break;
            case 'm':
				nUserWidth = atoi( &(argv[nArg][2]) );
				break;
            case 'a':
                bAsynch = true;
                break;
			case 'w':
				bHTMLTable = 1;
				break;
            case 't':
                pszInsertTable = &(argv[nArg][2]);
                break;
            case 'q':
                pszColumnsToQuote = &(argv[nArg][2]);
                break;
            case 'u':
                pszQuoteToUse = &(argv[nArg][2]);
                break;
            case 'e':
                pszStatementTerminator = &(argv[nArg][2]);
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
				printf( "MYODBCCls " MYODBC_VERSION "\n" );
				return false;
            case 'x':
				cDelimiter = strtol( argv[nArg]+2, NULL, 0 );
                break;
			default:
				fprintf( stderr, szSyntax );
				return false;
			}
			continue;
		}
		else if ( count == 1 )
			strncpy( (char*)szDSN, argv[nArg], MAX_DATA_WIDTH );
		else if ( count == 2 )
			strncpy( (char*)szUID, argv[nArg], MAX_DATA_WIDTH );
		else if ( count == 3 )
			strncpy( (char*)szPWD, argv[nArg], MAX_DATA_WIDTH );
		count++;
	}

    return true;
}


/*!
    doProcess
    
    This is the main loop for the application. It accepts and processes
    user input as required.
*/
bool ISQL::doProcess()
{
    // We always want to have an MYODBCConnection. Some applications will want to create more than
    // one of these but this application only needs one.
    pconnection     = new MYODBCConnection( penvironment );
    
    // Turning the ODBC asynch on may result in some feedback while long requests
    // are waited on. Mostly its just here to test asynch.
/*
    if ( bAsynch )
    {
        if ( !SQL_SUCCEEDED( pconnection->setSQL_ATTR_ASYNC_ENABLE( SQL_ASYNC_ENABLE_ON ) ) )
        {
            if ( bVerbose ) doDisplayErrors( pconnection->getMessages() );
            fprintf( stderr, "[isql++]ERROR: Failed to turn asynch on\n" );
            return false;
        }
    }
    if ( bVerbose ) doDisplayErrors( pconnection->getMessages() );
*/

    // CONNECT
    if ( !SQL_SUCCEEDED( pconnection->doConnect( szDSN, SQL_NTS, szUID, SQL_NTS, szPWD, SQL_NTS ) ) )
    {
        if ( bVerbose ) doDisplayErrors( pconnection->getMessages() );
		fprintf( stderr, "[isql++]ERROR: Failed to connect\n" );
        return false;
    }

    // We always want to have an MYODBCStatement. Many applications will want to create more than
    // one of these but this application only needs one. We just reuse it for each user request.
    // Creating it before we establish a connection using MYODBCConnection is ok - but we can not 
    // do much with it until after the connection has been established.
    pstatement      = new MYODBCStatement( pconnection );

    // Turning the ODBC asynch on may result in some feedback while long requests
    // are waited on. Mostly its just here to test asynch.
    if ( bAsynch )
    {
        if ( !SQL_SUCCEEDED( pstatement->setSQL_ATTR_ASYNC_ENABLE( SQL_ASYNC_ENABLE_ON ) ) )
        {
            if ( bVerbose ) doDisplayErrors( pstatement->getMessages() );
            fprintf( stderr, "[isql++]ERROR: Failed to turn asynch on\n" );
            return false;
        }
    }
    if ( bVerbose ) doDisplayErrors( pstatement->getMessages() );

    //
    // PROCESS
    //
    if ( bBatch )
        doProcessBatch();
    else
        doProcessInteractive();

    // DISCONNECT
    // - this will invalidate any results in all child MYODBCStatements
    if ( !SQL_SUCCEEDED( pconnection->doDisconnect() ) )
    {
        if ( bVerbose ) doDisplayErrors( pconnection->getMessages() );
		fprintf( stderr, "[isql++]ERROR: Failed to properly disconnect\n" );
        return false;
    }

    return true;
}

/*!
    doProcessBatch
    
    Main loop.
*/
bool ISQL::doProcessBatch()
{
    char *      pszCommand      = 0;    // command in the making
    char        cRead           = 0;    // contains last char read
    char        cQuote          = 0;    // contains quote if we are in quote
    int         nCommandSize    = 0;

	while ( 1 )
	{
        cRead = fgetc( stdin );
        if ( cRead == EOF )
            break;

        if ( cRead == *pszStatementTerminator && cQuote == 0 )
        {
            // we have a command
            if ( pszCommand )
            {
                // append '\0'
                nCommandSize++;
                pszCommand = (char *)realloc( pszCommand, nCommandSize );
                pszCommand[nCommandSize - 1] = '\0';

                // process command
                if ( !doProcessCommand( pszCommand ) )
                    break;

                // cleanup
                free( pszCommand );
                pszCommand      = 0;
                nCommandSize    = 0;
            }
        }
        else
        {
            // toggle quote state as required
            // - we do not want to interpret a statement terminator when it is in quotes
            // - this is simplistic because it dow not handle quotes which may be escaped in string
            if ( cQuote )
            {
                if ( cQuote == cRead )
                    cQuote = 0;
            }
            else
            {
                if ( cRead == '\'' || cRead == '\"' )
                    cQuote = cRead;
            }
            
            // accumilate
            nCommandSize++;
            pszCommand = (char *)realloc( pszCommand, nCommandSize );
            pszCommand[nCommandSize - 1] = cRead;
        }

    } // while
    
    // cleanup
    if ( pszCommand )
        free( pszCommand );

    return 1;
}

/*!
    doProcessInteractive
    
    Main loop.
*/
bool ISQL::doProcessInteractive()
{
    char *      pszCommand      = 0;    // command in the making
    char *      pszLine         = 0;    // contains last line read
    char        cQuote          = 0;    // contains quote if we are in quote
    int         nCommandSize    = 0;    // current length of pszCommand
    bool        bReadLines      = true;
#ifndef HAVE_READLINE
    char        szLine[4096];
#endif

    // display some help
    printf( szHelp );

    // 
	while ( bReadLines )
	{
#ifdef HAVE_READLINE
        if ( cQuote )
            pszLine = readline( "isql++ (text currently in quotes)> " );
        else
            pszLine = readline( "isql++> " );
#else
        if ( cQuote )
            printf( "isql++ (text currently in quotes)> " );
        else
            printf( "isql++> " );

        pszLine = fgets( szLine, sizeof(szLine), stdin );
#endif
        if ( !pszLine ) // EOF - ctrl D - this should not happen but may if file directed into us without batch flag being set
            break;

        // process line
        for ( char *psz = pszLine; *psz != '\0'; psz++ )
        {
            if ( *psz == *pszStatementTerminator && cQuote == 0 )
            {
                // we have a command
                if ( pszCommand )
                {
                    // append '\0'
                    nCommandSize++;
                    pszCommand = (char *)realloc( pszCommand, nCommandSize );
                    pszCommand[nCommandSize - 1] = '\0';

                    // process command
                    if ( !doProcessCommand( pszCommand ) )
                    {
                        // we have a quit command so quit both loops and exit function
                        bReadLines = false;
                        break; 
                    }

                    // cleanup
                    free( pszCommand );
                    pszCommand      = 0;
                    nCommandSize    = 0;
                }
            }
            else
            {
                // toggle quote state as required
                // - we do not want to interpret a statement terminator when it is in quotes
                // - this is simplistic because it dow not handle quotes which may be escaped in string
                if ( cQuote )
                {
                    if ( cQuote == *psz )
                        cQuote = 0;
                }
                else
                {
                    if ( *psz == '\'' || *psz == '\"' )
                        cQuote = *psz;
                }

                // accumilate
                nCommandSize++;
                pszCommand = (char *)realloc( pszCommand, nCommandSize );
                pszCommand[nCommandSize - 1] = *psz;
            }
        }

#ifdef HAVE_READLINE
        add_history( pszLine );
#endif
	} // while

    // cleanup
    if ( pszCommand )
        free( pszCommand );

    return 1;
}

/*!
    doProcessCommand

    Return false to exit main loop and app.    
*/
bool ISQL::doProcessCommand( char *pszCommand )
{
	char *pEscapeChar;

    // strip away escape chars
    while ( (pEscapeChar=(char*)strchr( (char*)pszCommand, '\n')) != NULL || (pEscapeChar=(char*)strchr( (char*)pszCommand, '\r')) != NULL )
        *pEscapeChar = ' ';

    //
    if ( *pszCommand == '\0' )
        return 1;

    //
    if ( strcmp( pszCommand, "quit" ) == 0 )
        return 0;
    else if ( strncmp( pszCommand, "show ", 5 ) == 0 )      // process the given 'show' command
        doExecuteShow( &(pszCommand[5]) );
    else if ( strncmp( pszCommand, "show", 4 ) == 0 )           // display the 'show' commands
        doExecuteShow();                                    
    else if ( strcmp( pszCommand, "help" ) == 0 )           // display some help
    {
        printf( szHelp );
        printf( "SQL> " );
    }
    else
        doExecuteSQL( pszCommand );                         // process the given 'SQL' statement

    return 1;
}

/*!
    doExecuteSQL
    
    Prepare and execute SQL then process any results.
*/
bool ISQL::doExecuteSQL( char *pszSQL )
{
	SQLCHAR			szSepLine[32001]		= "";	
    SQLSMALLINT     cols                    = -1;
    SQLLEN          nRows                   = 0;

	if ( !SQL_SUCCEEDED( pstatement->doPrepare( (SQLCHAR*)pszSQL, SQL_NTS ) ) )
	{
        if ( bVerbose ) doDisplayErrors( pstatement->getMessages() );
		fprintf( stderr, "[isql++]ERROR: While preparing statement\n" );
		return false;
	}

    if ( !SQL_SUCCEEDED( pstatement->getExecute() ) )
    {
        if ( bVerbose ) doDisplayErrors( pstatement->getMessages() );
		fprintf( stderr, "[isql++]ERROR: While executing statement\n" );
		return false;
    }

    /*
     * check to see if it has generated a result set
     */
    if ( !SQL_SUCCEEDED( pstatement->getNumResultCols( &cols ) ) )
    {
        if ( bVerbose ) doDisplayErrors( pstatement->getMessages() );
		fprintf( stderr, "[isql++]ERROR: While requesting number of columns\n" );
		return false;
    }

    if ( cols > 0 )
    {
        /****************************
         * WRITE HEADER
         ***************************/
        if ( pszInsertTable )
            ;
        else if ( bHTMLTable )
            doWriteHeaderHTMLTable();
        else if ( cDelimiter == 0 )
            doWriteHeaderNormal( szSepLine );
        else if ( cDelimiter && bColumnNames )
            doWriteHeaderDelimited();

        /****************************
         * WRITE BODY
         ***************************/
        if ( pszInsertTable )
            doWriteBodyInsertTable();
        else if ( bHTMLTable )
            doWriteBodyHTMLTable();
        else if ( cDelimiter == 0 )
            nRows = doWriteBodyNormal();
        else
            doWriteBodyDelimited();
    }

    /****************************
     * WRITE FOOTER
     ***************************/
    if ( pszInsertTable )
        ;
    else if ( bHTMLTable )
        doWriteFooterHTMLTable();
    else if ( cDelimiter == 0 )
        doWriteFooterNormal( szSepLine, nRows );

	return true;
}

/*!
    doExecuteShow
    
    Process "show" commands.
*/
bool ISQL::doExecuteShow( char *pszShow )
{
    if ( !pszShow || *pszShow == '\0' )
	{
        printf( szShow );
        return true;
	}

    // show driver
    if ( strncmp( pszShow, "driver", 6 ) == 0 )
    {
        SQLRETURN       nReturn;
        SQLCHAR         szDescription[21];
        SQLSMALLINT     nDescriptionLength;
        SQLCHAR         szAttributes[4096];
        SQLSMALLINT     nAttributesLength;
        
        printf( "+----------------------+------------------------------------------------------------------------------------------------------+\n" );
        printf( "| Driver               | First Attribute                                                                                      |\n" );
        printf( "+----------------------+------------------------------------------------------------------------------------------------------+\n" );
        nReturn = penvironment->getDriver( SQL_FETCH_FIRST, szDescription, sizeof(szDescription) - 1, &nDescriptionLength, szAttributes, sizeof(szAttributes) - 1, &nAttributesLength );
        while ( SQL_SUCCEEDED( nReturn ) )
        {
            printf( "| %-*s | %-*s |\n", sizeof(szDescription) - 1, szDescription, 100, szAttributes );
            nReturn = penvironment->getDriver( SQL_FETCH_NEXT, szDescription, sizeof(szDescription) - 1, &nDescriptionLength, szAttributes, sizeof(szAttributes) - 1, &nAttributesLength );
        }
        printf( "+----------------------+------------------------------------------------------------------------------------------------------+\n" );
        if ( nReturn != SQL_NO_DATA )
        {
            if ( bVerbose ) doDisplayErrors( penvironment->getMessages() );
			fprintf( stderr, "[isql++]ERROR: While getting driver listing\n" );
			return false;
        }
        return true;
    }
    // show dsn
    else if ( strncmp( pszShow, "dsn", 3 ) == 0 )
    {
        SQLRETURN       nReturn;
        SQLCHAR         szDSN[21];
        SQLSMALLINT     nNameLength1;
        SQLCHAR         szDescription[101];
        SQLSMALLINT     nNameLength2;

        printf( "+----------------------+------------------------------------------------------------------------------------------------------+\n" );
        printf( "| Data Source Name     | Description                                                                                          |\n" );
        printf( "+----------------------+------------------------------------------------------------------------------------------------------+\n" );
        nReturn = penvironment->getDataSource( SQL_FETCH_FIRST, szDSN, sizeof(szDSN) - 1, &nNameLength1, szDescription, sizeof(szDescription) - 1, &nNameLength2 );
        while ( SQL_SUCCEEDED( nReturn ) )
        {
            printf( "| %-*s | %-*s |\n", sizeof(szDSN) - 1, szDSN, sizeof(szDescription) - 1, szDescription );
            nReturn = penvironment->getDataSource( SQL_FETCH_NEXT, szDSN, sizeof(szDSN) - 1, &nNameLength1, szDescription, sizeof(szDescription) - 1, &nNameLength2 );
        }
        printf( "+----------------------+------------------------------------------------------------------------------------------------------+\n" );
        if ( nReturn != SQL_NO_DATA )
        {
            if ( bVerbose ) doDisplayErrors( penvironment->getMessages() );
			fprintf( stderr, "[isql++]ERROR: While getting data source listing\n" );
			return false;
        }
        return true;
    }
    // show catalog
    else if ( strncmp( pszShow, "catalog", 7 ) == 0 )
    {
        SQLRETURN nReturn;

        if ( pszShow[7] == ' ' )
            nReturn = pstatement->getTables( (SQLCHAR*)&pszShow[8] );
        else
            nReturn = pstatement->getTables( (SQLCHAR*)SQL_ALL_CATALOGS );

		if ( !SQL_SUCCEEDED( nReturn ) )
        {
            if ( bVerbose ) doDisplayErrors( pstatement->getMessages() );
			fprintf( stderr, "[isql++]ERROR: Failed to get catalogs\n" );
			return false;
        }
    }
    // show schema
    else if ( strncmp( pszShow, "schema", 6 ) == 0 )
    {
        SQLRETURN nReturn;

        if ( pszShow[6] == ' ' )
            nReturn = pstatement->getTables( NULL, 0, (SQLCHAR*)&pszShow[7], SQL_NTS );
        else
            nReturn = pstatement->getTables( NULL, 0, (SQLCHAR*)SQL_ALL_SCHEMAS, SQL_NTS );

		if ( !SQL_SUCCEEDED( nReturn ) )
        {
            if ( bVerbose ) doDisplayErrors( pstatement->getMessages() );
			fprintf( stderr, "[isql++]ERROR: Failed to get schemas\n" );
			return false;
        }
    }
    // show table
    else if ( strncmp( pszShow, "table", 5 ) == 0 )
    {
        SQLRETURN nReturn;

        if ( pszShow[5] == ' ' )
            nReturn = pstatement->getTables( NULL, 0, NULL, 0, (SQLCHAR*)&pszShow[6], SQL_NTS, (SQLCHAR*)"TABLE", SQL_NTS );
        else
            nReturn = pstatement->getTables( NULL, 0, NULL, 0, NULL, 0, (SQLCHAR*)"TABLE", SQL_NTS );

		if ( !SQL_SUCCEEDED( nReturn ) )
        {
            if ( bVerbose ) doDisplayErrors( pstatement->getMessages() );
			fprintf( stderr, "[isql++]ERROR: Failed to get tables\n" );
			return false;
        }
    }
    // show column
    else if ( strncmp( pszShow, "column", 6 ) == 0 )
    {
        pszShow = &(pszShow[7]);

		if ( !SQL_SUCCEEDED( pstatement->getColumns( NULL, 0, NULL, 0, (SQLCHAR*)pszShow ) ) )
//        if ( !SQL_SUCCEEDED( pstatement->getColumns( (SQLCHAR*)"%", SQL_NTS, (SQLCHAR*)"%", SQL_NTS, pszShow ) ) )
        {
            if ( bVerbose ) doDisplayErrors( pstatement->getMessages() );
			fprintf( stderr, "[isql++]ERROR: While requesting column listing\n" );
			return false;
        }
    }
    // show types
    else if ( strncmp( pszShow, "types", 5 ) == 0 )
    {
		if ( !SQL_SUCCEEDED( pstatement->getTypeInfo() ) )
        {
            if ( bVerbose ) doDisplayErrors( pstatement->getMessages() );
			fprintf( stderr, "[isql++]ERROR: Failed to get type info\n" );
			return false;
        }
    }

	/****************************
	 * WRITE HEADER
	 ***************************/
	SQLCHAR			szSepLine[32001]	= "";	
    SQLLEN          nRows               = 0;

	if ( bHTMLTable )
		doWriteHeaderHTMLTable();
	else if ( cDelimiter == 0 )
		doWriteHeaderNormal( szSepLine );
    else if ( cDelimiter && bColumnNames )
        doWriteHeaderDelimited();

	/****************************
	 * WRITE BODY
	 ***************************/
	if ( bHTMLTable )
		doWriteBodyHTMLTable();
	else if ( cDelimiter == 0 )
		nRows = doWriteBodyNormal();
	else
		doWriteBodyDelimited();

	/****************************
	 * WRITE FOOTER
	 ***************************/
	if ( bHTMLTable )
		doWriteFooterHTMLTable();
	else if ( cDelimiter == 0 )
		doWriteFooterNormal( szSepLine, nRows );

	return true;
}

/****************************
 * WRITE HTML
 ***************************/
void ISQL::doWriteHeaderHTMLTable()
{
    SQLINTEGER    	nCol            				= 0;
	SQLSMALLINT		nColumns						= 0;
	SQLCHAR			szColumnName[MAX_DATA_WIDTH+1]	= "";	

	printf( "<table BORDER>\n" );
	printf( "<tr BGCOLOR=#000099>\n" );

    if ( !SQL_SUCCEEDED( pstatement->getNumResultCols( &nColumns ) ) )
        nColumns = -1;

	for( nCol = 1; nCol <= nColumns; nCol++ )
	{
		pstatement->getColAttribute( nCol, SQL_DESC_LABEL, szColumnName, sizeof(szColumnName), NULL, NULL );
		printf( "<td>\n" );
		printf( "<font face=Arial,Helvetica><font color=#FFFFFF>\n" );
		printf( "%s\n", (char*)szColumnName );
		printf( "</font></font>\n" );
		printf( "</td>\n" );
	}
	printf( "</tr>\n" );
}

void ISQL::doWriteBodyHTMLTable()
{
    SQLINTEGER    	nCol            				= 0;
	SQLSMALLINT		nColumns						= 0;
	SQLLEN		    nIndicator						= 0;
	SQLCHAR			szColumnValue[MAX_DATA_WIDTH+1]	= "";
	SQLRETURN		nReturn							= 0;
    SQLRETURN       ret;

    if ( !SQL_SUCCEEDED( pstatement->getNumResultCols( &nColumns ) ) )
        nColumns = -1;

    while( (ret = pstatement->getFetch()) == SQL_SUCCESS ) /* ROWS */
    {
		printf( "<tr>\n" );
		
        for( nCol = 1; nCol <= nColumns; nCol++ ) /* COLS */
        {
			printf( "<td>\n" );
			printf( "<font face=Arial,Helvetica>\n" );

            nReturn = pstatement->getData( nCol, SQL_C_CHAR, (SQLPOINTER)szColumnValue, sizeof(szColumnValue), &nIndicator );
            if ( SQL_SUCCEEDED( nReturn ) && nIndicator != SQL_NULL_DATA )
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

void ISQL::doWriteFooterHTMLTable()
{
	printf( "</table>\n" );
}

/****************************
 * WRITE INSERT TABLE 
 ***************************/
void ISQL::doWriteBodyInsertTable()
{
    SQLINTEGER    	nCol            				= 0;
	SQLSMALLINT		nColumns						= 0;
	SQLLEN		    nIndicator						= 0;
	SQLCHAR			szColumnValue[MAX_DATA_WIDTH+1]	= "";
	SQLRETURN		nReturn							= 0;
    bool *          pbQuote                         = 0;

    if ( !SQL_SUCCEEDED( pstatement->getNumResultCols( &nColumns ) ) )
        nColumns = -1;

    if ( nColumns < 0 )
        return;

    pbQuote = (bool*)calloc( sizeof(bool), nColumns );

    // parse pszColumnsToQuote
    if ( pszColumnsToQuote )
    {
        int     nCursorColumnsToQuote   = 0;
        int     nCursorColumn           = 0;
        char    szColumn[100];

        while ( 1 )
        {
            if ( pszColumnsToQuote[nCursorColumnsToQuote] == ',' || pszColumnsToQuote[nCursorColumnsToQuote] == '\0' )
            {
                if ( nCursorColumn )
                {
                    szColumn[nCursorColumn] = '\0';
                    int nColumn = atoi( szColumn );
                    if ( nColumn >= 0 && nColumn < nColumns )
                    {
                        pbQuote[nColumn] = 1;
                    }
                    nCursorColumn = 0;

                }
            }
            else
            {
                szColumn[nCursorColumn] = pszColumnsToQuote[nCursorColumnsToQuote];
                nCursorColumn++;
            }

            if ( pszColumnsToQuote[nCursorColumnsToQuote] == '\0' )
                break;

            nCursorColumnsToQuote++;
        }
    }

    // process results
    while( SQL_SUCCEEDED( pstatement->getFetch() ) ) /* ROWS */
    {
		printf( "INSERT INTO %s VALUES ( ", pszInsertTable );
        for( nCol = 1; nCol <= nColumns; nCol++ ) /* COLS */
        {
            if ( nCol > 1 )
    			printf( ", " );

            nReturn = pstatement->getData( nCol, SQL_C_CHAR, (SQLPOINTER)szColumnValue, sizeof(szColumnValue), &nIndicator );
            if ( SQL_SUCCEEDED( nReturn ) )
            {
                if ( nIndicator == SQL_NULL_DATA )
                    printf( "NULL" );
                else
                {
                    // TODO: Escape any quote char embedded in char data
                    printf( "%s%s%s", 
                            ( pbQuote[nCol - 1] ? pszQuoteToUse : "" ), 
                            szColumnValue, 
                            ( pbQuote[nCol - 1] ? pszQuoteToUse : "" ) );
                }
            }
    	    else 
    		    break;
        } // cols
        printf( " )%s\n", ( pszStatementTerminator ? pszStatementTerminator : "" ) );
    } // rows

    free( pbQuote );
}

/****************************
 * WRITE DELIMITED
 * - this output can be used by the ODBC Text File driver
 * - last column no longer has a delimit char (it is implicit)...
 *   this is consistent with odbctxt
 ***************************/
void ISQL::doWriteHeaderDelimited()
{
    SQLINTEGER    	nCol            				= 0;
	SQLSMALLINT		nColumns						= 0;
	SQLCHAR			szColumnName[MAX_DATA_WIDTH+1]	= "";	

    if ( !SQL_SUCCEEDED( pstatement->getNumResultCols( &nColumns ) ) )
        nColumns = -1;

	for( nCol = 1; nCol <= nColumns; nCol++ )
	{
		pstatement->getColAttribute( nCol, SQL_DESC_LABEL, szColumnName, sizeof(szColumnName), NULL, NULL );
        fputs((char*) szColumnName, stdout );
        if ( nCol < nColumns )
            putchar( cDelimiter );
	}
    putchar( '\n' );
}

void ISQL::doWriteBodyDelimited()
{
    SQLINTEGER    	nCol            				= 0;
	SQLSMALLINT		nColumns						= 0;
	SQLLEN		    nIndicator						= 0;
	SQLCHAR			szColumnValue[MAX_DATA_WIDTH+1]	= "";
	SQLRETURN		nReturn							= 0;
    SQLRETURN       ret;

    if ( !SQL_SUCCEEDED( pstatement->getNumResultCols( &nColumns ) ) )
        nColumns = -1;

	/* ROWS */
    while(( ret = pstatement->getFetch()) == SQL_SUCCESS )
    {
		/* COLS */
        for( nCol = 1; nCol <= nColumns; nCol++ )
        {
            nReturn = pstatement->getData( nCol, SQL_C_CHAR, (SQLPOINTER)szColumnValue, sizeof(szColumnValue), &nIndicator );
            if ( SQL_SUCCEEDED( nReturn ) && nIndicator != SQL_NULL_DATA )
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
//        if ( bVerbose ) DumpODBCLog( 0, 0, hStmt );
    }
}

/****************************
 * WRITE NORMAL
 ***************************/
void ISQL::doWriteHeaderNormal( SQLCHAR *szSepLine )
{
    SQLINTEGER    	nCol            				= 0;
	SQLSMALLINT		nColumns						= 0;
	SQLCHAR			szColumn[MAX_DATA_WIDTH+20]		= "";	
	SQLCHAR			szColumnName[MAX_DATA_WIDTH+1]	= "";	
	SQLCHAR			szHdrLine[32001]				= "";	
	SQLUINTEGER		nOptimalDisplayWidth            = 10;

    if ( !SQL_SUCCEEDED( pstatement->getNumResultCols( &nColumns ) ) )
        nColumns = -1;

	for( nCol = 1; nCol <= nColumns; nCol++ )
	{
        nOptimalDisplayWidth = getOptimalDisplayWidth( nCol );
        pstatement->getColAttribute( nCol, SQL_DESC_LABEL, szColumnName, sizeof(szColumnName), NULL, NULL );

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

SQLLEN ISQL::doWriteBodyNormal()
{
    SQLINTEGER    	nCol            				= 0;
	SQLSMALLINT		nColumns						= 0;
	SQLLEN		    nIndicator						= 0;
	SQLCHAR			szColumn[MAX_DATA_WIDTH+20]		= "";
	SQLCHAR			szColumnValue[MAX_DATA_WIDTH+1]	= "";
	SQLRETURN		nReturn							= 0;
    SQLLEN          nRows                           = 0;
	SQLUINTEGER		nOptimalDisplayWidth            = 10;

    if ( !SQL_SUCCEEDED( pstatement->getNumResultCols( &nColumns ) ) )
        nColumns = -1;

	/* ROWS */
    while ( SQL_SUCCEEDED( pstatement->getFetch() ) )
    {
		/* COLS */
        for( nCol = 1; nCol <= nColumns; nCol++ )
        {
            nOptimalDisplayWidth = getOptimalDisplayWidth( nCol );
            nReturn = pstatement->getData( nCol, SQL_C_CHAR, (SQLPOINTER)szColumnValue, sizeof(szColumnValue), &nIndicator );
            szColumnValue[MAX_DATA_WIDTH] = '\0';

            if ( SQL_SUCCEEDED( nReturn ) && nIndicator != SQL_NULL_DATA )
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
    } /* while rows */

    if ( pstatement->getMessages()->getFirst() )
    {
        if ( bVerbose ) doDisplayErrors( pstatement->getMessages() );
        fprintf( stderr, "[isql++]INFO: There were warnings or errors\n" );
        return false;
    }

    return nRows;
}

void ISQL::doWriteFooterNormal( SQLCHAR	*szSepLine, SQLLEN nRows )
{
    SQLLEN  nRowsAffected	= -1;

	printf( (char*)szSepLine );

    pstatement->getRowCount( &nRowsAffected );
    if ( nRowsAffected >= 0 && !bBatch )
    	printf( "%d rows affected\n", (int)nRowsAffected );

    if ( nRows && !bBatch )
    	printf( "%d rows returned\n", (int)nRows );
}

void ISQL::doDisplayErrors( MYODBCObjectList *pmessages )
{
    if ( !pmessages )
        return;

    // display messages on stderr
    MYODBCMessage *pmessage = (MYODBCMessage*)pmessages->getFirst();

    while ( pmessage )
    {
        fprintf( stderr, "%s %s %s\n", pmessage->getTypeText(), pmessage->getState(), pmessage->getMessage() );
        pmessage = (MYODBCMessage*)pmessage->getZNext();
    }

    // remove messages
    // - bAutoDelete should be on by default so messages will get deleted as they are removed
    pmessages->doClear(); 
}




