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

#ifndef MYODBCUTIL_H
#define MYODBCUTIL_H

/*
  We define _GNU_SOURCE to get the definition of strndup() from string.h
*/
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* #include "../MYODBC_MYSQL.h" */
#include "../MYODBC_CONF.h"
#include "../MYODBC_ODBC.h"

#ifdef WIN32
#define strcasecmp( a, b ) stricmp( a, b )
#endif

/* 
    Handle case on OSX where we want to use GetPrivateProfileString (because it 
    actually works properly) instead of SQLGetPrivateProfileString but 
    GetPrivateProfileString does not exist because we are dealing with 10.4 or
    newer - I admit - this is a bit of a hack.
*/
#ifndef GetPrivateProfileString
#define GetPrivateProfileString SQLGetPrivateProfileString
#endif

#ifndef FALSE 
#define FALSE 0
#define TRUE 1
#endif

#ifndef myodbc_max
#define myodbc_max( a, b ) (((a) > (b)) ? (a) : (b))
#define myodbc_min( a, b ) (((a) < (b)) ? (a) : (b))
#endif


/*
   Most of the installer API functions in iODBC incorrectly reset the
   config mode, so we need to save and restore it whenever we call those
   functions. These macros reduce the clutter a little bit.
*/
#if USE_IODBC
# define SAVE_MODE() UWORD config_mode; (void)SQLGetConfigMode(&config_mode)
# define RESTORE_MODE() (void)SQLSetConfigMode(config_mode)
#else
# define SAVE_MODE()
# define RESTORE_MODE()
#endif


/* Could use DRIVER_NAME but trying to keep dependency upon driver specific code to a min */
#define MYODBCINST_DRIVER_NAME "MySQL ODBC 3.51 Driver"

/* max dsn's we can have in odbc sys info - need to get rid of this limit */
#define MYODBCUTIL_MAX_DSN_NAMES 50

/*!
    \internal
    \brief      Delimiter type used for an attribute string.

*/  
typedef enum tMYODBCUTIL_DELIM
{
    MYODBCUTIL_DELIM_NULL,
    MYODBCUTIL_DELIM_SEMI,
    MYODBCUTIL_DELIM_BOTH

} MYODBCUTIL_DELIM;

/*!
    \internal
    \brief      Parse state.

                These are the different states we can be in while parsing 
                an attributes string.
*/  
typedef enum tMYODBCUTIL_ATTR_PARSE_STATE
{
    MYODBCUTIL_ATTR_PARSE_STATE_NAME_START,   /* looking for start of name  */
    MYODBCUTIL_ATTR_PARSE_STATE_NAME,         /* looking for end of name    */
    MYODBCUTIL_ATTR_PARSE_STATE_EQUAL,        /* looking for equal sign     */
    MYODBCUTIL_ATTR_PARSE_STATE_VALUE_START,  /* looking for start of value */
    MYODBCUTIL_ATTR_PARSE_STATE_VALUE         /* looking for end of value   */

} MYODBCUTIL_ATTR_PARSE_STATE;


/*!
    Driver fields.

    This reflects what we can have in odbcinst.ini.
*/
typedef struct tMYODBCUTIL_DRIVER
{
    char *pszName;              /* Friendly name for driver.                (ie "MySQL ODBC 3.51 Driver")   */
    char *pszDRIVER;            /* File name - typically complete abs path. (ie "/usr/lib/libmyodbc3.so")   */
    char *pszSETUP;             /* File name - typically complete abs path. (ie "/usr/lib/libmyodbc3S.so")  */

} MYODBCUTIL_DRIVER;

/*!
    \brief      Used to indicate what is being used to do 
                a SQLDriverConnect.
*/
typedef enum tMYODBCUTIL_DATASOURCE_CONNECT
{
    MYODBCUTIL_DATASOURCE_CONNECT_DRIVER,
    MYODBCUTIL_DATASOURCE_CONNECT_DSN

} MYODBCUTIL_DATASOURCE_CONNECT;

/*!
    \brief      Prompting used during SQLDriverConnect.

                Could be a SQLUSMALLINT and then standard
                ODBC values such as SQL_DRIVER_PROMPT but there
                are worthwhile advantages to creating this type
                internally.
*/
typedef enum tMYODBCUTIL_DATASOURCE_PROMPT
{
    MYODBCUTIL_DATASOURCE_PROMPT_PROMPT,
    MYODBCUTIL_DATASOURCE_PROMPT_COMPLETE, 
    MYODBCUTIL_DATASOURCE_PROMPT_REQUIRED,
    MYODBCUTIL_DATASOURCE_PROMPT_NOPROMPT

} MYODBCUTIL_DATASOURCE_PROMPT;


/*!
    \brief      DSN edit mode.

                Reason why we have created a MYODBCUTIL_DATASOURCE.
*/
typedef enum tMYODBCUTIL_DATASOURCE_MODE
{
    MYODBCUTIL_DATASOURCE_MODE_DSN_ADD,         /* For ConfigDSN() - ODBC_ADD_DSN.              */
    MYODBCUTIL_DATASOURCE_MODE_DSN_EDIT,        /* For ConfigDSN() - ODBC_CONFIG_DSN.           */
    MYODBCUTIL_DATASOURCE_MODE_DSN_VIEW,        /* For command-line tools to list DSN details.  */
    MYODBCUTIL_DATASOURCE_MODE_DRIVER_CONNECT   /* For SQLDriverConnect().                      */

} MYODBCUTIL_DATASOURCE_MODE;


/*!
    DSN fields.

    This reflects what we can have in odbc.ini and also includes information
    used during an SQLDriverConnect().
*/
typedef struct tMYODBCUTIL_DATASOURCE
{
    char *pszDriverFileName;    /* As used in DSN; often the abs path to driver.        (ie "/usr/lib/libmyodbc3.so")           */
    char *pszDSN;               /* The name of our data source.                         (ie "test")                             */    
    char *pszDRIVER;            /* Friendly driver name as used in DSN and connect str. (ie "MySQL ODBC 3.51 Driver (32 bit)")  */
    char *pszDESCRIPTION;       /* General description or long name of DSN.                                                     */
    char *pszSERVER;            /* The hostname of the MySQL server.                                                            */
    char *pszUSER;              /* The username used to connect to MySQL.                                                       */
    char *pszPASSWORD;          /* The password for the server user combination.                                                */ 
    char *pszDATABASE;          /* The default database.                                                                        */
    char *pszPORT;              /* The TCP/IP port to use if SERVER is not localhost.                                           */
    char *pszSOCKET;            /* Unix socket file or Windows named pipe to connect to.                                        */ 
    char *pszSTMT;              /* Statement that will be exec when connecting to MySQL.                                        */ 
    char *pszOPTION;            /* Options that specify how MyODBC should work.                                                 */

    char *pszSSLKEY;            /* pathname to SSL key file                                                                              */
    char *pszSSLCERT;           /* pathname to SSL certificate file                                                                              */
    char *pszSSLCA;             /* pathname to SSL certificate authority file                                                                  */
    char *pszSSLCAPATH;         /* pathname to a directory that contains SSL ceritificate authority files                                      */
    char *pszSSLCIPHER;         /* pathname to a list of allowable ciphers                                                                    */
    char *pszSSLVERIFY;         /* verify server certificate (see --ssl-verify-server-cert                                      */
    char *pszCHARSET;           /* default charset to use for connection */

    MYODBCUTIL_DATASOURCE_MODE      nMode;      /* ConfigDSN mode or SQLDriverConnect mode.                                     */
    MYODBCUTIL_DATASOURCE_CONNECT   nConnect;   /* SQLDriverConnect() using a DSN or a DRIVER to connect.                       */
    MYODBCUTIL_DATASOURCE_PROMPT    nPrompt;    /* SQLDriverConnect() kind of prompting (if any).                               */
    
    BOOL  bSaveFileDSN;         /* Flag specifying that SQLConnect was invoked as part of creation of a file DSN                */

} MYODBCUTIL_DATASOURCE;

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__APPLE__) && 0
int GetPrivateProfileString( LPCSTR lpszSection, LPCSTR lpszEntry, LPCSTR lpszDefault, LPSTR lpszRetBuffer, int cbRetBuffer, LPCSTR lpszFilename );
#endif


BOOL                    MYODBCUtilGetDriverNames( char *pszBuffer, int nBuffer );
BOOL                    MYODBCUtilGetDataSourceNames( char *pszBuffer, int nBuffer, UWORD nScope );
char *                  MYODBCUtilGetIniFileName( BOOL bUseEnvVar );

/* MYODBCUTIL_DRIVER */
MYODBCUTIL_DRIVER *     MYODBCUtilAllocDriver();
void                    MYODBCUtilClearDriver( MYODBCUTIL_DRIVER *pDriver );
void                    MYODBCUtilFreeDriver( MYODBCUTIL_DRIVER *pDriver );
BOOL                    MYODBCUtilReadDriver( MYODBCUTIL_DRIVER *pDriver, LPCSTR pszName, LPCSTR pszFileName );
BOOL                    MYODBCUtilWriteDriver( MYODBCUTIL_DRIVER *pDriver );

/* MYODBCUTIL_DATASOURCE */
MYODBCUTIL_DATASOURCE * MYODBCUtilAllocDataSource( MYODBCUTIL_DATASOURCE_MODE nMode );
void                    MYODBCUtilClearDataSource( MYODBCUTIL_DATASOURCE *pDataSource );
void                    MYODBCUtilFreeDataSource( MYODBCUTIL_DATASOURCE *pDataSource );
BOOL                    MYODBCUtilDSNExists( char *pszDataSourceName );
BOOL                    MYODBCUtilReadDataSource( MYODBCUTIL_DATASOURCE *pDataSource, LPCSTR pszDSN );
BOOL                    MYODBCUtilWriteDataSource( MYODBCUTIL_DATASOURCE *pDataSource );
BOOL                    MYODBCUtilReadDataSourceStr( MYODBCUTIL_DATASOURCE *pDataSource, MYODBCUTIL_DELIM nDelim, LPCSTR pszStr );
BOOL                    MYODBCUtilWriteDataSourceStr( MYODBCUTIL_DATASOURCE *pDataSource, MYODBCUTIL_DELIM nDelim, char *pszStr, SQLSMALLINT nMaxLen );
BOOL                    MYODBCUtilDefaultDataSource( MYODBCUTIL_DATASOURCE *pDataSource );
BOOL                    MYODBCUtilReadConnectStr( MYODBCUTIL_DATASOURCE *pDataSource, LPCSTR pszStr );
BOOL                    MYODBCUtilWriteConnectStr( MYODBCUTIL_DATASOURCE *pDataSource, char *pszStr, SQLSMALLINT nMaxLen );
BOOL                    MYODBCUtilInsertStr( char *pszStr, LPCSTR pszStrIn, SQLSMALLINT nMaxLen, int *pnIndex );

#if defined(WIN32)
   char *strglobaldup( const char *s);
   char *strnglobaldup( const char *s, size_t n);
#  define _global_strdup(s) strglobaldup(s)
#  define _global_strndup(s, n) strnglobaldup(s, n)
#  define _global_alloc(n) GlobalAlloc(GMEM_FIXED, (n))
#  define _global_free(p) GlobalFree(p)
#else
#  define _global_strdup(s) strdup(s)
#  ifdef HAVE_STRNDUP
#    define myodbc_strndup(s, n) strndup(s, n)
#    define _global_strndup(s, n) strndup(s, n)
#  else
     char *myodbc_strndup( const char *s, size_t n );
#    define _global_strndup(s, n) myodbc_strndup(s, n)
#  endif
#  define _global_alloc malloc
#  define _global_free(p) free(p)
#endif

#ifdef __cplusplus
}
#endif

#endif



