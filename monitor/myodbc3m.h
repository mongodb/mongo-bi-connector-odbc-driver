
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
    #include <windows.h>
#else
    #include <string.h>
#endif

#include <sqlext.h>

/* Disable _attribute__ on non-gcc compilers. */
#if !defined(__attribute__) && !defined(__GNUC__)
# define __attribute__(arg)
#endif

#ifdef HAVE_STRTOL

char *szSyntax =
"\n" \
"**********************************************\n" \
"* MySQL - myodbc3m                           *\n" \
"**********************************************\n" \
"* Syntax                                     *\n" \
"*                                            *\n" \
"*     myodbc3m DSN | STR  [UID [PWD]] [opts] *\n" \
"*                                            *\n" \
"* Options                                    *\n" \
"*                                            *\n" \
"* -r         Use SQLDriverConnect to connect *\n" \
"*            and use STR as connection string*\n" \
"* -b         batch.(no prompting etc)        *\n" \
"* -dx        delimit columns with x          *\n" \
"* -x0xXX     delimit columns with XX, where  *\n" \
"*            x is in hex, ie 0x09 is tab     *\n" \
"* -w         wrap results in an HTML table   *\n" \
"* -c         column names on first row.      *\n" \
"*            (only used when -d)             *\n" \
"* -mn        limit column display width to n *\n" \
"* -v         verbose.                        *\n" \
"* -lx        set locale to x                 *\n" \
"* --version  version                         *\n" \
"*                                            *\n" \
"* Notes                                      *\n" \
"*                                            *\n" \
"*      myodbc3m supports redirection and     *\n" \
"*      piping for batch processing.          *\n" \
"*                                            *\n" \
"* Examples                                   *\n" \
"*                                            *\n" \
"*  cat My.sql | myodbc3m WebDB MyID MyPWD -w *\n" \
"*                                            *\n" \
"*  myodbc3m                                  *\n" \
"*   'DRIVER=myodbc3;DSN=test;UID=uid;PWD=pwd'*\n" \
"*   -r                                       *\n" \
"*                                            *\n" \
"*      Each line in My.sql must contain      *\n" \
"*      exactly 1 SQL command except for the  *\n" \
"*      last line which must be blank.        *\n" \
"*                                            *\n" \
"* Please visit;                              *\n" \
"*                                            *\n" \
"*      http://www.mysql.com                  *\n" \
"**********************************************\n\n";

#else

char *szSyntax =
"\n" \
"**********************************************\n" \
"* MySQL - myodbc3m                           *\n" \
"**********************************************\n" \
"* Syntax                                     *\n" \
"*                                            *\n" \
"*     myodbc3m DSN [UID [PWD]] [options]     *\n" \
"*                                            *\n" \
"* Options                                    *\n" \
"*                                            *\n" \
"* -r         Use SQLDriverConnect to connect *\n" \
"*            and use STR as connection string*\n" \
"* -b         batch.(no prompting etc)        *\n" \
"* -dx        delimit columns with x          *\n" \
"* -x0xXX     delimit columns with XX, where  *\n" \
"*            x is in hex, ie 0x09 is tab     *\n" \
"* -w         wrap results in an HTML table   *\n" \
"* -c         column names on first row.      *\n" \
"*            (only used when -d)             *\n" \
"* -mn        limit column display width to n *\n" \
"* -v         verbose.                        *\n" \
"* --version  version                         *\n" \
"*                                            *\n" \
"* Notes                                      *\n" \
"*                                            *\n" \
"*      myodbc3m supports redirection and     *\n" \
"*      piping for batch processing.          *\n" \
"*                                            *\n" \
"* Examples                                   *\n" \
"*                                            *\n" \
"*  cat My.sql | myodbc3m WebDB MyID MyPWD -w *\n" \
"*                                            *\n" \
"*  myodbc3m                                  *\n" \
"*   'DRIVER=myodbc3;DSN=test;UID=uid;PWD=pwd'*\n" \
"*   -r                                       *\n" \
"*                                            *\n" \
"*      Each line in My.sql must contain      *\n" \
"*      exactly 1 SQL command except for the  *\n" \
"*      last line which must be blank.        *\n" \
"*                                            *\n" \
"* Please visit;                              *\n" \
"*                                            *\n" \
"*      http://www.mysql.com                  *\n" \
"*      pharvey@mysql.com                     *\n" \
"**********************************************\n\n";

#endif

#define MAX_DATA_WIDTH 300

#ifndef myodbc_max
#define myodbc_max( a, b ) (((a) > (b)) ? (a) : (b))
#endif

#ifndef myodbc_min
#define myodbc_min( a, b ) (((a) < (b)) ? (a) : (b))
#endif

int OpenEnvironment( SQLHENV *phEnv );
int OpenDatabase( SQLHENV hEnv, SQLHDBC *phDbc, char *szDSN, char *szUID, char *szPWD );
int ExecuteSQL( SQLHDBC hDbc, char *szSQL, char cDelimiter, int bColumnNames, int bHTMLTable );
int ExecuteHelp( SQLHDBC hDbc, char *szSQL, char cDelimiter, int bColumnNames, int bHTMLTable );
int	CloseDatabase( SQLHDBC hDbc );
int	CloseEnvironment( SQLHENV hEnv );

void WriteHeaderHTMLTable( SQLHSTMT hStmt );
void WriteHeaderNormal( SQLHSTMT hStmt, SQLCHAR	*szSepLine );
void WriteHeaderDelimited( SQLHSTMT hStmt, char cDelimiter );
void WriteBodyHTMLTable( SQLHSTMT hStmt );
SQLINTEGER WriteBodyNormal( SQLHSTMT hStmt );
void WriteBodyDelimited( SQLHSTMT hStmt, char cDelimiter );
void WriteFooterHTMLTable( SQLHSTMT hStmt );
void WriteFooterNormal( SQLHSTMT hStmt, SQLCHAR	*szSepLine, SQLINTEGER nRows );

int DumpODBCLog( SQLHENV hEnv, SQLHDBC hDbc, SQLHSTMT hStmt );


