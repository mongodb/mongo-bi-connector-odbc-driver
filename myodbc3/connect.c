/* Copyright (C) 1995-2006 MySQL AB

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

/***************************************************************************
 * CONNECT.C								   *
 *									   *
 * @description: This is the MyODBC 3.51 sample driver code for connection *
 *									   *
 * @author     : MySQL AB (monty@mysql.com, venu@mysql.com)		   *
 * @date       : 2001-Aug-15						   *
 * @product    : myodbc3						   *
 *									   *
 ****************************************************************************/

/***************************************************************************
 * The following ODBC APIs are implemented in this file:		   *
 *									   *
 *   SQLConnect		(ISO 92)					   *
 *   SQLDriverConnect	(ODBC)						   *
 *   SQLDisconnect	(ISO 92)					   *
 *									   *
 ****************************************************************************/

#include "myodbc3.h"

#ifndef _UNIX_
    #ifdef USE_IODBC
        #include <iodbcinst.h>
    #else
        #include <odbcinst.h>
    #endif
#endif

#include "../myodbc3u/MYODBCUtil.h"

#ifndef CLIENT_NO_SCHEMA
    #define CLIENT_NO_SCHEMA      16
#endif

static SQLRETURN set_connect_defaults(DBC *dbc)
{
    SQLRETURN error= 0;
    my_bool reconnect = 1;
    MYODBCDbgEnter("set_connect_defaults");
#ifndef DBUG_OFF
    if (dbc->flag & FLAG_LOG_QUERY && !dbc->query_log)
        dbc->query_log= init_query_log();
#endif
    /* Set STMT error prefix, one time */
    strxmov(dbc->st_error_prefix,MYODBC3_ERROR_PREFIX,"[mysqld-",
            dbc->mysql.server_version,"]",NullS);

    if (dbc->flag & FLAG_AUTO_RECONNECT)
        mysql_options(&dbc->mysql, MYSQL_OPT_RECONNECT, (char*) &reconnect);
    /*
      Validate pre-connection options like AUTOCOMMIT
      and TXN_ISOLATIONS from SQLSetConnectAttr
    */

    /* AUTOCOMMIT */
    if ((dbc->commit_flag == CHECK_AUTOCOMMIT_OFF))
    {
        if (!(trans_supported(dbc)) || (dbc->flag & FLAG_NO_TRANSACTIONS))
            error= set_conn_error(dbc,MYERR_01S02,
                                  "\
Transactions are not enabled, Option value SQL_AUTOCOMMIT_OFF changed to \
SQL_AUTOCOMMIT_ON",
                                  0);
        else if (autocommit_on(dbc) &&
                 (odbc_stmt(dbc,"SET AUTOCOMMIT=0") != SQL_SUCCESS))
            MYODBCDbgReturn(SQL_ERROR);
    }
    else if ((dbc->commit_flag == CHECK_AUTOCOMMIT_ON) &&
             trans_supported(dbc) && !autocommit_on(dbc))
    {
        if (odbc_stmt(dbc,"SET AUTOCOMMIT=1") != SQL_SUCCESS)
            MYODBCDbgReturn(SQL_ERROR);
    }

    if (!(dbc->txn_isolation & DEFAULT_TXN_ISOLATION))/* TXN_ISOLATION */
    {
        char buff[80];
        const char *level;

        if (dbc->txn_isolation & SQL_TXN_SERIALIZABLE)
            level="SERIALIZABLE";
        else if (dbc->txn_isolation & SQL_TXN_REPEATABLE_READ)
            level="REPEATABLE READ";
        else if (dbc->txn_isolation & SQL_TXN_READ_COMMITTED)
            level="READ COMMITTED";
        else
            level="READ UNCOMMITTED";
        if (trans_supported(dbc))
        {
            MYODBCDbgPrint("info",("setting transaction isolation to level '%s'",
                                   level));
            sprintf(buff,"SET SESSION TRANSACTION ISOLATION LEVEL %s",level);
            if (odbc_stmt(dbc,buff) != SQL_SUCCESS)
                error= SQL_ERROR;
        }
        else
        {
            MYODBCDbgPrint("info",("SQL_ATTR_TXN_ISOLATION %ld ignored",
                                   dbc->txn_isolation));
        }
    }
    MYODBCDbgReturn(error);
}


/*
  @type    : myodbc3 internal
  @purpose : check the option flag and generate a connect flag
*/

ulong get_client_flag(MYSQL *mysql, ulong option_flag,uint connect_timeout,
                      char *init_stmt)
{
    ulong client_flag= CLIENT_ODBC;
    MYODBCDbgEnter("get_client_flag");

    mysql_init(mysql);

    if (option_flag & (FLAG_FOUND_ROWS | FLAG_SAFE))
        client_flag|=   CLIENT_FOUND_ROWS;
    if (option_flag & FLAG_NO_SCHEMA)
        client_flag|=   CLIENT_NO_SCHEMA;
    if (option_flag & (FLAG_BIG_PACKETS | FLAG_SAFE))
        max_allowed_packet=~0L;
    if (option_flag & FLAG_COMPRESSED_PROTO)
        client_flag |=  CLIENT_COMPRESS;
    if (option_flag & FLAG_IGNORE_SPACE)
        client_flag |=  CLIENT_IGNORE_SPACE;

    client_flag |= CLIENT_MULTI_RESULTS;
#ifdef __WIN__
    if (option_flag & FLAG_NAMED_PIPE)
        mysql_options(mysql,MYSQL_OPT_NAMED_PIPE,NullS);
#endif

    if (option_flag & FLAG_USE_MYCNF)
        mysql_options(mysql,MYSQL_READ_DEFAULT_GROUP,"odbc");
    if (init_stmt && init_stmt[0])
        mysql_options(mysql,MYSQL_INIT_COMMAND,init_stmt);
    if (connect_timeout)
        mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT,(char*) &connect_timeout);

    MYODBCDbgPrint("info",("option_flag: %ld  client_flag: %ld", option_flag,
                           client_flag));
    MYODBCDbgReturn2( client_flag );
}


/*
  @type    : myodbc3 internal
  @purpose : simple help functions to copy arguments if given
*/

void copy_if_not_empty(char *to,uint max_length, char *from,int length)
{
    if (from)
    {
        if (length == SQL_NTS)
            length= max_length-1;
        strmake(to,from,length);
    }
}


/*
  @type    : ODBC 1.o API
  @purpose : to connect to mysql server
*/

SQLRETURN SQL_API SQLConnect( SQLHDBC        hdbc, 
                              SQLCHAR FAR *  szDSN,
                              SQLSMALLINT    cbDSN,
                              SQLCHAR FAR *  szUID, 
                              SQLSMALLINT    cbUID,
                              SQLCHAR FAR *  szAuthStr, 
                              SQLSMALLINT    cbAuthStr )
{
    char host[64],user[64],passwd[64],dsn[NAME_LEN+1],database[NAME_LEN+1];
    char port[10],flag[10],init_stmt[256],*dsn_ptr;
    char szTRACE[FILENAME_MAX+1]= "";
    char socket[256]= "";
    ulong flag_nr,client_flag;
    uint port_nr= 0;
    DBC FAR *dbc= (DBC FAR*) hdbc;
    MYODBCDbgEnter("SQLConnect");

    if (dbc->mysql.net.vio != 0)
        MYODBCDbgReturn(set_conn_error(hdbc,MYERR_08002,NULL,0));

    /* Reset error state */
    CLEAR_DBC_ERROR(dbc);

    dsn_ptr= fix_str(dsn, (char*) szDSN, cbDSN);
    if (dsn_ptr && !dsn_ptr[0])
        MYODBCDbgReturn(set_conn_error(hdbc, MYERR_S1000, "Invalid Connection Parameters",0));

    SQLGetPrivateProfileString(dsn_ptr,"user","", user, sizeof(user), MYODBCUtilGetIniFileName( TRUE ) );
    SQLGetPrivateProfileString(dsn_ptr,"password","", passwd, sizeof(passwd), MYODBCUtilGetIniFileName( TRUE ) );
    SQLGetPrivateProfileString(dsn_ptr,"server","localhost", host, sizeof(host), MYODBCUtilGetIniFileName( TRUE ) );
    SQLGetPrivateProfileString(dsn_ptr,"database",dsn_ptr, database, sizeof(database), MYODBCUtilGetIniFileName( TRUE ) );
    SQLGetPrivateProfileString(dsn_ptr,"port","0", port, sizeof(port), MYODBCUtilGetIniFileName( TRUE ) );
    port_nr= (uint) atoi(port);
    SQLGetPrivateProfileString(dsn_ptr,"option","0", flag, sizeof(flag), MYODBCUtilGetIniFileName( TRUE ) );
    flag_nr= (ulong) atol(flag);

#ifdef _UNIX_
    SQLGetPrivateProfileString(dsn_ptr,"socket", "", socket, sizeof(socket), MYODBCUtilGetIniFileName( TRUE ) );
#endif

    SQLGetPrivateProfileString(dsn_ptr,"stmt", "", init_stmt, sizeof(init_stmt), MYODBCUtilGetIniFileName( TRUE ) );

#ifndef DBUG_OFF
    if ( flag_nr & FLAG_DEBUG && !_db_on_ )
    {
        char  szMYODBC_LOG[FILENAME_MAX+20]= "";

        SQLGetPrivateProfileString(dsn_ptr, "TRACE", "", szTRACE, sizeof(szTRACE),  MYODBCUtilGetIniFileName( TRUE ) );
        if ( strcasecmp( szTRACE, "ON" ) == 1 || strcasecmp( szTRACE, "YES" ) == 1 || strcasecmp( szTRACE, "Y" ) == 1 || *szTRACE == '1' )
        {
            char  szTRACEFILE[FILENAME_MAX+1]= "";
            SQLGetPrivateProfileString(dsn_ptr, "TRACEFILE", "", szTRACEFILE, sizeof(szTRACEFILE), MYODBCUtilGetIniFileName( TRUE ) );
            if ( *szTRACEFILE )
                sprintf(szMYODBC_LOG, "d:t:F:L:S:A,%s", szTRACEFILE);
        }

        if ( *szMYODBC_LOG )
            MYODBCDbgSetFile( szMYODBC_LOG )
        else
            MYODBCDbgSetFile( MYODBCDbgGetFileDefault() )
    }
#endif
    client_flag= get_client_flag(&dbc->mysql,flag_nr,(uint) dbc->login_timeout,
                                 init_stmt);

    copy_if_not_empty(passwd,sizeof(passwd), (char FAR*) szAuthStr,cbAuthStr);
    copy_if_not_empty(user, sizeof(user), (char FAR *) szUID, cbUID);

    /* socket[0] is always 0 if you are not under UNIX */
    if (!mysql_real_connect(&dbc->mysql,
                            host,
                            user,
                            passwd[0] ? passwd : 0,
                            database, 
                            port_nr,
                            socket[0] ? socket: NullS,
                            (uint)client_flag))
    {
        set_dbc_error(dbc, "HY000", mysql_error(&dbc->mysql),
                      mysql_errno(&dbc->mysql));
        translate_error(dbc->error.sqlstate,
                        MYERR_S1000,mysql_errno(&dbc->mysql));
        MYODBCDbgReturn(SQL_ERROR);
    }
    dbc->dsn= my_strdup(dsn_ptr ? dsn_ptr : database ,MYF(MY_WME));
    dbc->database= my_strdup(database,MYF(MY_WME));
    dbc->server= my_strdup(host,MYF(MY_WME));
    dbc->user= my_strdup(user,MYF(MY_WME));
    dbc->password= my_strdup(passwd,MYF(MY_WME));
    dbc->port= port_nr;
    dbc->flag= flag_nr;

    MYODBCDbgReturn(set_connect_defaults(dbc));
}


/*

*/
SQLRETURN my_SQLDriverConnectTry( DBC *dbc, MYODBCUTIL_DATASOURCE *pDataSource )
{
    ulong nFlag = 0;

    nFlag = get_client_flag( &dbc->mysql, 
                             pDataSource->pszOPTION ? atoi(pDataSource->pszOPTION) : 0, 
                             (uint)dbc->login_timeout, 
                             pDataSource->pszSTMT ? pDataSource->pszSTMT : "" );

    if ( !mysql_real_connect( &dbc->mysql,
                              pDataSource->pszSERVER, 
                              pDataSource->pszUSER,
                              pDataSource->pszPASSWORD,
                              pDataSource->pszDATABASE,
                              atoi( pDataSource ->pszPORT ),
                              pDataSource->pszSOCKET,
                              nFlag ) )
    {
        set_dbc_error( dbc, "HY000", mysql_error( &dbc->mysql ), mysql_errno( &dbc->mysql ) );
        translate_error( dbc->error.sqlstate, MYERR_S1000, mysql_errno( &dbc->mysql ) );
        return SQL_ERROR;
    }
    return SQL_SUCCESS;
}

/*
  @type    : ODBC 1.0 API
  @purpose : An alternative to SQLConnect. It supports data sources that
  require more connection information than the three arguments in
  SQLConnect, dialog boxes to prompt the user for all connection
  information, and data sources that are not defined in the system
  information.
*/

SQLRETURN SQL_API my_SQLDriverConnect( SQLHDBC             hdbc,
                                       SQLHWND             hwnd,
                                       SQLCHAR FAR *       szConnStrIn,
                                       SQLSMALLINT         cbConnStrIn
                                         __attribute__((unused)),
                                       SQLCHAR FAR *       szConnStrOut,
                                       SQLSMALLINT         cbConnStrOutMax,
                                       SQLSMALLINT FAR *   pcbConnStrOut,
                                       SQLUSMALLINT        fDriverCompletion )
{
    MYODBCUTIL_DATASOURCE * pDataSource = MYODBCUtilAllocDataSource( MYODBCUTIL_DATASOURCE_MODE_DRIVER_CONNECT );
    MYODBCUTIL_DRIVER *     pDriver     = MYODBCUtilAllocDriver();      /* we have to read in driver info to get setup lib */
    SQLRETURN               nReturn     = SQL_SUCCESS;
    BOOL                    bPrompt     = FALSE;
    DBC FAR *               dbc         = (DBC FAR*)hdbc;
    char                    szError[1024];
#ifdef WIN32
    HMODULE                 hModule     = NULL;
#else
    void *                  hModule     = NULL;
#endif

    MYODBCDbgEnter("SQLDriverConnect");

    /* parse incoming string */
    if ( !MYODBCUtilReadConnectStr( pDataSource, (LPCSTR)szConnStrIn ) )
    {
        set_dbc_error( dbc, "HY000", "Failed to parse the incoming connect string.", 0 );
        nReturn = SQL_ERROR;
        goto exitDriverConnect;
    }

    /*!
        ODBC RULE

        If the connection string contains the DSN keyword, the driver 
        retrieves the information for the specified data source (and
        merges it into given connection info with given connection info
        having precedence).

        \note
                This also allows us to get pszDRIVER (if not already given).
    */
    if ( pDataSource->pszDSN )
    {
        if ( !MYODBCUtilReadDataSource( pDataSource, pDataSource->pszDSN ) )
        {
            /*!

                ODBC RULE

                Establish a connection to a data source that is not defined in the system 
                information. If the application supplies a partial connection string, the 
                driver can prompt the user for connection information.
            */    
        }
    }

    /* 
        Make pDataSource good for mysql_real_connect(). Mostly
        means making some "" values NULL.
    */
    MYODBCUtilDefaultDataSource( pDataSource );

#ifndef DBUG_OFF
    if ( atol( pDataSource->pszOPTION ) & FLAG_DEBUG && ! _db_on_ )
        MYODBCDbgSetFile( MYODBCDbgGetFileDefault() );
    MYODBCDbgPrint( "enter",( "fDriverCompletion: %d", fDriverCompletion ) );
#endif

    /*!
       MYODBC RULE

       We can not present a prompt unless we can lookup the name of the 
       setup library file name. This means we need a DRIVER. We try to xref
       using a DSN (above) or, hopefully, get one in connection string.

       \note 

       This will, when we need to prompt, trump the ODBC rule where we can 
       connect with a DSN which does not exist. A possible solution is to
       hard-code some fall-back value for pDataSource->pszDRIVER but lets 
       not do it until we see if this is a problem in practice. After all;
       if the DSN does not exist the app can at least provide the driver
       name for us in the connection string.
    */
    if ( !pDataSource->pszDRIVER && !pDataSource->pszDriverFileName && fDriverCompletion != SQL_DRIVER_NOPROMPT )
    {
        sprintf( szError, "Could not determine the driver name; could not lookup setup library. DSN=(%s)\n", pDataSource->pszDSN );
        set_dbc_error( hdbc, "HY000", szError, 0 );
        nReturn = SQL_ERROR;
        goto exitDriverConnect;
    }

    /*!
        ODBC RULE

        We only prompt if we need to. 

        Not so obvious gray area is with (SQL_DRIVER_COMPLETE/SQL_DRIVER_COMPLETE_REQUIRED)
        server and password - particularly password. These can work with defaults/blank but 
        many callers expect prompting when these are blank. So we compromise; we try to 
        connect and if it works we say its ok otherwise we go to a prompt.
    */
    switch ( fDriverCompletion )
    {
        case SQL_DRIVER_PROMPT:
            pDataSource->nPrompt= MYODBCUTIL_DATASOURCE_PROMPT_PROMPT;
            bPrompt             = TRUE;
            break;

        case SQL_DRIVER_COMPLETE:
            pDataSource->nPrompt = MYODBCUTIL_DATASOURCE_PROMPT_COMPLETE;
            if ( my_SQLDriverConnectTry( dbc, pDataSource ) == SQL_SUCCESS )
                goto exitDriverConnect1;
            bPrompt = TRUE;
            break;

        case SQL_DRIVER_COMPLETE_REQUIRED:
            pDataSource->nPrompt = MYODBCUTIL_DATASOURCE_PROMPT_REQUIRED;
            if ( my_SQLDriverConnectTry( dbc, pDataSource ) == SQL_SUCCESS )
                goto exitDriverConnect1;
            bPrompt = TRUE;
            break;

        case SQL_DRIVER_NOPROMPT:
            pDataSource->nPrompt = MYODBCUTIL_DATASOURCE_PROMPT_NOPROMPT;
            /*!
               ODBC RULE

               We need a DSN or DRIVER in order to work without prompting.
            */    
            if ( !pDataSource->pszDSN && !pDataSource->pszDRIVER )
            {
                set_dbc_error( hdbc, "IM007", "Missing DSN and/or DRIVER and SQL_DRIVER_NOPROMPT.", 0 );
                nReturn = SQL_ERROR;
                goto exitDriverConnect;
            }
            break;
        default:
            {
                set_dbc_error( hdbc, "HY110", "Invalid driver completion.", 0 );
                nReturn = SQL_ERROR;
                goto exitDriverConnect;
            }
    }

#ifdef __APPLE__
    if ( bPrompt )
    {
        set_dbc_error( hdbc, "HY000", "Prompting not supported on this platform. Please provide all required connect information.", 0 );
        nReturn = SQL_ERROR;
        goto exitDriverConnect;
    }
#endif

    if ( bPrompt )
    {
        BOOL (*pFunc)( SQLHDBC, SQLHWND, MYODBCUTIL_DATASOURCE * );

        /*!
           ODBC RULE
    
           We can not present a prompt if we have a null window handle.
        */
        if ( !hwnd && fDriverCompletion != SQL_DRIVER_NOPROMPT )
        {
            set_dbc_error( hdbc, "IM008", "Invalid window handle for connection completion argument.", 0 );
            nReturn = SQL_ERROR;
            goto exitDriverConnect;
        }

        /* 
           At this point we should have a driver name (friendly name) either loaded
           from DSN or provided in connection string. So lets determine the setup
           library file name (better to not assume name). We read from ODBC system 
           info. This allows someone configure for a custom setup interface.
        */
        if ( !MYODBCUtilReadDriver( pDriver, pDataSource->pszDRIVER, pDataSource->pszDriverFileName ) )
        {
            char sz[1024];
            if ( pDataSource->pszDRIVER && *(pDataSource->pszDRIVER) )
                sprintf( sz, "Could not find driver %s in system information.", pDataSource->pszDRIVER );
            else
                sprintf( sz, "Could not find driver %s in system information.", pDataSource->pszDriverFileName );
            set_dbc_error( hdbc, "HY000", sz, 0 );
            nReturn = SQL_ERROR;
            goto exitDriverConnect;
        }

        if ( !pDriver->pszSETUP )
        {
            set_dbc_error( hdbc, "HY000", "Could not determine the file name of setup library.", 0 );
            nReturn = SQL_ERROR;
            goto exitDriverConnect;
        }

        /*
           We dynamically load setup lib to avoid introducing gui link dependencies 
           into driver and also allowing the setup library to be pluggable. So 
           a ncurses ver or a gtk ver etc could be created/used and this code is ok.
        */
#ifdef WIN32
        if ( !(hModule = LoadLibrary( pDriver->pszSETUP )) )
#else
        lt_dlinit();
        if ( !(hModule = (void *)lt_dlopen( pDriver->pszSETUP ))  )
#endif
        {
            char sz[1024];
            sprintf( sz, "Could not load the setup library (%s).", pDriver->pszSETUP );
            set_dbc_error( hdbc, "HY000", sz, 0 );
            nReturn = SQL_ERROR;
            goto exitDriverConnect;
        }
        /*
           The setup library should expose a MYODBCSetupDriverConnect() C entry point
           for us to call.
        */
#ifdef WIN32
        pFunc = (BOOL (*)( SQLHDBC, SQLHWND, MYODBCUTIL_DATASOURCE * )) GetProcAddress( hModule, "MYODBCSetupDriverConnect" );
#else
        pFunc = (BOOL (*)( SQLHDBC, SQLHWND, MYODBCUTIL_DATASOURCE * )) lt_dlsym( hModule, "MYODBCSetupDriverConnect" );
#endif
        if ( pFunc == NULL )
        {
#ifdef WIN32
            LPVOID pszMsg;

            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                          NULL,
                          GetLastError(),
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                          (LPTSTR) &pszMsg,
                          0, 
                          NULL );
            fprintf( stderr, pszMsg );
            set_dbc_error( hdbc, "HY000", pszMsg, 0 );
            LocalFree( pszMsg );
#else
            set_dbc_error( hdbc, "HY000", "Could not find MYODBCSetupDriverConnect in setup library.", 0 );
#endif
            nReturn = SQL_ERROR;
            goto exitDriverConnect0;
        }

        /*
           Prompt. Function returns false if user cancels.
        */
        if ( !pFunc( hdbc, hwnd, pDataSource ) )
        {
            set_dbc_error( hdbc, "HY000", "User cancelled.", 0 );
            nReturn = pDataSource->bSaveFileDSN ? SQL_NO_DATA : SQL_ERROR;
            goto exitDriverConnect0;
        }
    }

    if ( my_SQLDriverConnectTry( dbc, pDataSource ) != SQL_SUCCESS )
    {
        if (!pDataSource->bSaveFileDSN)
        {
            nReturn = SQL_ERROR;
            goto exitDriverConnect0;
        }
        else
        {
            set_dbc_error( hdbc, "08001", "Client unable to establish connection.", 0 );
            nReturn = SQL_SUCCESS_WITH_INFO;
        }
    }

    exitDriverConnect1:
    /*! todo: handle error from this without losing memory */

    /*
        save the settings we used to connect
    */   
    my_free( (gptr)dbc->database, MYF(0) ); /* in case SQL_ATTR_CURRENT_CATALOG set */
    dbc->dsn        = my_strdup( pDataSource->pszDSN ? pDataSource->pszDSN : "null", MYF(MY_WME) );
    dbc->database   = my_strdup( pDataSource->pszDATABASE ? pDataSource->pszDATABASE : "null", MYF(MY_WME) );
    dbc->server     = my_strdup( pDataSource->pszSERVER ? pDataSource->pszSERVER : "localhost", MYF(MY_WME) );
    dbc->user       = my_strdup( pDataSource->pszUSER ? pDataSource->pszUSER : "", MYF(MY_WME) );
    dbc->password   = my_strdup( pDataSource->pszPASSWORD ? pDataSource->pszPASSWORD : "", MYF(MY_WME) );
    dbc->port       = atoi( pDataSource ->pszPORT );
    dbc->flag       = atol( pDataSource->pszOPTION );

    set_connect_defaults( dbc );
    /*! 
        ODBC RULE

        Create a return connection string only if we connect. szConnStrOut 'should' 
        have at least 1024 bytes allocated or be null.
    */
    if ( szConnStrOut )
    {
#ifdef WIN32
        /*
            MYODBC RULE

            Do nothing special if in-str and out-str are same address. This is because
            ADO/VB will do this while at the same time *not* provide the space
            recommended by the ODBC specification (1024 bytes min) resulting in a
            "Catastrophic error" - the driver going bye bye.
        */        
        if ( szConnStrOut != szConnStrIn )
#endif
        {
            *szConnStrOut = '\0';
            if ( !MYODBCUtilWriteConnectStr( pDataSource, (char *)szConnStrOut, cbConnStrOutMax ) )
            {
                set_dbc_error( dbc, "01000", "Something went wrong while building the outgoing connect string.", 0 );
                nReturn = SQL_SUCCESS_WITH_INFO;
            }
        }

        if ( pcbConnStrOut )
            *pcbConnStrOut = strlen( (char *)szConnStrOut );
    }

    exitDriverConnect0:
#ifdef WIN32
    if ( hModule )
        FreeLibrary( hModule );
#else
    if ( hModule )
        lt_dlclose( hModule );
#endif

    exitDriverConnect:
    MYODBCUtilFreeDriver( pDriver );
    MYODBCUtilFreeDataSource( pDataSource );

    MYODBCDbgReturn( nReturn );
}


/*
  @type    : ODBC 1.0 API
  @purpose : is an alternative to SQLConnect. It supports data sources that
  require more connection information than the three arguments in
  SQLConnect, dialog boxes to prompt the user for all connection
  information, and data sources that are not defined in the
  system information.
*/

SQLRETURN SQL_API
SQLDriverConnect(SQLHDBC hdbc,SQLHWND hwnd,
                 SQLCHAR FAR *szConnStrIn,
                 SQLSMALLINT cbConnStrIn,
                 SQLCHAR FAR *szConnStrOut,
                 SQLSMALLINT cbConnStrOutMax,
                 SQLSMALLINT FAR *pcbConnStrOut,
                 SQLUSMALLINT fDriverCompletion)
{
    return my_SQLDriverConnect(hdbc,hwnd,szConnStrIn,cbConnStrIn,
                               szConnStrOut,cbConnStrOutMax,
                               pcbConnStrOut,fDriverCompletion);
}


/*
  @type    : ODBC 1.0 API
  @purpose : supports an iterative method of discovering and enumerating
  the attributes and attribute values required to connect to a
  data source
*/

SQLRETURN SQL_API
SQLBrowseConnect(SQLHDBC hdbc,
                 SQLCHAR FAR *szConnStrIn __attribute__((unused)),
                 SQLSMALLINT cbConnStrIn __attribute__((unused)),
                 SQLCHAR FAR *szConnStrOut __attribute__((unused)),
                 SQLSMALLINT cbConnStrOutMax __attribute__((unused)),
                 SQLSMALLINT FAR *pcbConnStrOut __attribute__((unused)))
{
    MYODBCDbgEnter("SQLBrowseConnect");
    MYODBCDbgReturn(set_conn_error(hdbc,MYERR_S1000,
                                   "Driver Does not support this API",0));
}


/*
  @type    : myodbc3 internal
  @purpose : closes the connection associated with a specific connection handle
*/

SQLRETURN SQL_API my_SQLDisconnect(SQLHDBC hdbc)
{
    LIST *list_element,*next_element;
    DBC FAR *dbc= (DBC FAR*) hdbc;
    MYODBCDbgEnter("my_SQLDisconnect");

    for (list_element= dbc->statements ; list_element ;
        list_element= next_element)
    {
        next_element= list_element->next;
        my_SQLFreeStmt((SQLHSTMT) list_element->data, SQL_DROP);
    }
    mysql_close(&dbc->mysql);
    my_free(dbc->dsn,MYF(0));
    my_free(dbc->database,MYF(0));
    my_free(dbc->server,MYF(0));
    my_free(dbc->user,MYF(0));
    my_free(dbc->password,MYF(0));
    dbc->dsn= dbc->database= dbc->server= dbc->user= dbc->password= 0;
#ifndef DBUG_OFF
    if (dbc->flag & FLAG_LOG_QUERY)
        end_query_log(dbc->query_log);
#endif
    MYODBCDbgReturn(SQL_SUCCESS);
}


/*
  @type    : ODBC 1.0 API
  @purpose : closes the connection associated with a specific connection handle
*/

SQLRETURN SQL_API SQLDisconnect(SQLHDBC hdbc)
{
    return my_SQLDisconnect(hdbc);
}
