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

#ifndef ODBC_ISQL_H
#define ODBC_ISQL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

//
#include <MYODBCEnvironment.h>
#include <MYODBCConnection.h>
#include <MYODBCStatement.h>

#define MAX_DATA_WIDTH 300

#ifndef max
#define max( a, b ) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min( a, b ) (((a) < (b)) ? (a) : (b))
#endif

char *szSyntax =
"\n" \
"**********************************************\n" \
"* isql++                                     *\n" \
"**********************************************\n" \
"* Syntax                                     *\n" \
"*                                            *\n" \
"*      isql++ [DSN [UID [PWD]]] [options]    *\n" \
"*                                            *\n" \
"* Options                                    *\n" \
"*                                            *\n" \
"* -s         list data source names          *\n" \
"* -r         list drivers                    *\n" \
"* -b         batch.(no prompting etc)        *\n" \
"* -dx        delimit columns with x          *\n" \
"* -x0xXX     delimit columns with XX, where  *\n" \
"*            x is in hex, ie 0x09 is tab     *\n" \
"* -w         wrap results in an HTML table   *\n" \
"* -tTable    wrap results in INSERT SQL      *\n" \
"* -qn,n,...  comma seperated list of column  *\n" \
"*            numbers where values should be  *\n" \
"*            quoted (use with -t)            *\n" \
"* -ux        char to use to quote char data  *\n" \
"* -ex        char(s) to use to terminate     *\n" \
"*            statements (i.e. use with -t)   *\n" \
"* -c         column names on first row.      *\n" \
"*            (only used when -d)             *\n" \
"* -mn        limit column display width to n *\n" \
"* -a         make ODBC calls asynch          *\n" \
"* -v         verbose.                        *\n" \
"* --version  version                         *\n" \
"*                                            *\n" \
"* Notes                                      *\n" \
"*                                            *\n" \
"*      isql++ supports redirection and       *\n" \
"*      piping for batch processing.          *\n" \
"*                                            *\n" \
"* Examples                                   *\n" \
"*                                            *\n" \
"*  isql++ MyDSN MyID MyPWD -w < My.sql       *\n" \
"*                                            *\n" \
"* Please visit;                              *\n" \
"*                                            *\n" \
"*      http://www.unixodbc.org               *\n" \
"*      pharvey@codebydesign.com              *\n" \
"*      nick@easysoft.com                     *\n" \
"**********************************************\n\n";

char *szHelp = 
"\n" \
"+---------------------------------------+\n" \
"| HELP                                  |\n" \
"|                                       |\n" \
"| sql-statement                         |\n" \
"| show                                  |\n" \
"| quit                                  |\n" \
"|                                       |\n" \
"| IMPORTANT                             |\n" \
"|                                       |\n" \
"| ALL commands and statements should end|\n" \
"| with a ';'. This can be changed using |\n" \
"| -e option.                            |\n" \
"|                                       |\n" \
"+---------------------------------------+\n\n"; 

char *szShow =
"\n" \
"show\n" \
"show driver [name]\n" \
"show dsn [name]\n" \
"show catalog [catalog]\n" \
"show schema [[catalog.]schema]\n" \
"show table [[[catalog.]schema.]table]\n" \
"show column [[[[catalog.]schema.]table.]column]\n" \
"show types\n" \
"\n" \
"NOTE: use %% as a wildcard as needed\n" \
"\n\n";

class ISQL 
{
public:
    ISQL();
    virtual ~ISQL();

    bool bAsynch;
    int bVerbose;
    SQLUINTEGER nUserWidth;
	int bHTMLTable;
	int	bBatch;
	int cDelimiter;
    int bColumnNames;
    char *      pszInsertTable;
    char *      pszColumnsToQuote;
    char *      pszQuoteToUse;
    char *      pszStatementTerminator;
	SQLCHAR     szDSN[MAX_DATA_WIDTH+1];
	SQLCHAR     szUID[MAX_DATA_WIDTH+1];
	SQLCHAR     szPWD[MAX_DATA_WIDTH+1];
	
    // SETTERS

    // GETTERS
    SQLUINTEGER getOptimalDisplayWidth( SQLINTEGER nCol );

    // DO'rs
    bool doParseArgs( int argc, char *argv[] );
    bool doProcess();
    bool doProcessBatch();
    bool doProcessInteractive();
    bool doProcessCommand( char *pszCommand );
    bool doExecuteSQL( char *pszCommand );
    bool doExecuteShow( char *pszShow = NULL );
    void doWriteHeaderHTMLTable();
    void doWriteHeaderNormal( SQLCHAR	*szSepLine );
    void doWriteHeaderDelimited();
    void doWriteBodyInsertTable();
    void doWriteBodyHTMLTable();
    SQLLEN doWriteBodyNormal();
    void doWriteBodyDelimited();
    void doWriteFooterHTMLTable();
    void doWriteFooterNormal( SQLCHAR *szSepLine, SQLLEN nRows );

    void doDisplayErrors( MYODBCObjectList *pmessages );

protected:
    MYODBCEnvironment *   penvironment;
    MYODBCConnection *    pconnection;
    MYODBCStatement *     pstatement;
};

#endif


