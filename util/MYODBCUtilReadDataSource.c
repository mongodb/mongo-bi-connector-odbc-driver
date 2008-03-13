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

#include "MYODBCUtil.h"

/*!
    \note   - unixODBC does not have a header section in ini files.
            - On XP, the "32 bit" parts are secretly stripped away
            inside SQLGetPrivateProfileString() somewhere.
*/            
#define MYODBCUTIL_ODBCINI_HEADER_SECTION "ODBC Data Sources"

/*!
    \brief  Read data source from system information.

    \param  pDataSource Data Source struct.
    \param  pszDSN The data source name to lookup.

    \note   This will not replace existing values in pDataSource.

            On XP we use hard-coded driver name and driver
            file name (if not given) as it seems MS is in the middle
            of changing odbc system info (probably to better support
            64 bit).
*/            
BOOL MYODBCUtilReadDataSource( MYODBCUTIL_DATASOURCE *pDataSource, LPCSTR pszDSN )
{
    char    szEntryNames[SQL_MAX_DSN_LENGTH * MYODBCUTIL_MAX_DSN_NAMES];
    char *  pszEntryName;
    char    szValue[4096];
    int     nChars;
#ifdef _WIN32
    UWORD   nMode   = ODBC_BOTH_DSN;
#endif
    SAVE_MODE();

#ifdef _WIN32
    if ( !SQLGetConfigMode( &nMode ) )
    {
        fprintf( stderr, "[%s][%d][ERROR] SQLGetConfigMode failed!\n", __FILE__, __LINE__ );
        return FALSE;
    }
#endif

    if ( !pszDSN || !(*pszDSN) )
        return TRUE;

    *szEntryNames = '\0';

    /* Get the list of key names for the DSN's section. */
    nChars= SQLGetPrivateProfileString(pszDSN, NULL, "",
                                       szEntryNames, sizeof(szEntryNames) - 1,
                                       "ODBC.INI");
    if (nChars < 1)
      return FALSE;

    RESTORE_MODE();

#if defined(WIN32)
    {
        /*
          There is a bug in SQLGetPrivateProfileString when mode is
          ODBC_BOTH_DSN and we are looking for a system DSN. In this case
          SQLGetPrivateProfileString will find the system DSN but return
          a corrupt list of attributes.

          A corrupt list of attributes can be identified because the first
          attribute (if any) will be followed by more than one '\0'.

          The solution is to detect this condition and set mode to
          ODBC_SYSTEM_DSN and try again. We also ensure we reset the mode
          when done - regardless of outcome.

          This may be the issue explained here:
            http://support.microsoft.com/kb/909122/

          And this work-around may not be correct:
            http://bugs.mysql.com/27599
        */
        int     nLen    = strlen( szEntryNames );

        if ( nMode == ODBC_BOTH_DSN && nLen < nChars && szEntryNames[nLen + 1 ] == '\0' )
        {
            *szEntryNames = '\0';
            if ( !SQLSetConfigMode( ODBC_SYSTEM_DSN ) )
                fprintf( stderr, "[%s][%d][ERROR] SQLSetConfigMode failed!\n", __FILE__, __LINE__ );

            if ( ( nChars = SQLGetPrivateProfileString( pszDSN, NULL, NULL, szEntryNames, sizeof( szEntryNames ) - 1, "ODBC.INI" ) ) < 1 )
                return FALSE;
        }
    }
#endif

    /* Looks like things are going to work. Lets set DSN.  */
    if ( !pDataSource->pszDSN )
        pDataSource->pszDSN = _global_strdup( pszDSN ) ;

    /* Handle each key we found for this DSN */
    pszEntryName = szEntryNames;
    while ( *pszEntryName )
    {
        *szValue = '\0';
        if ( SQLGetPrivateProfileString( pszDSN, pszEntryName, "", szValue, sizeof( szValue ) - 1, "ODBC.INI" ) > 0 )
        {
            /*!
                ODBC RULE
                (SQLDriverConnect)

                The driver uses any information it retrieves from the system information 
                to augment the information passed to it in the connection string. If the 
                information in the system information duplicates information in the 
                connection string, the driver uses the information in the connection 
                string.
            */    
            if ( strcasecmp( pszEntryName, "DATABASE" ) == 0 || strcasecmp( pszEntryName, "DB" ) == 0 )
            {
                if ( !pDataSource->pszDATABASE )
                    pDataSource->pszDATABASE = _global_strdup( szValue ) ;
            }
            else if ( strcasecmp( pszEntryName, "DESCRIPTION" ) == 0 || strcasecmp( pszEntryName, "DESC" ) == 0 )
            {
                if ( !pDataSource->pszDESCRIPTION )
                    pDataSource->pszDESCRIPTION = _global_strdup( szValue );
            }
            else if ( strcasecmp( pszEntryName, "DRIVER" ) == 0 )
            {
#if defined(WIN32)
                if ( !pDataSource->pszDriverFileName )
                    pDataSource->pszDriverFileName = _global_strdup( szValue );
#else
                if ( *szValue == '/' )
                {
                    if ( !pDataSource->pszDriverFileName )
                        pDataSource->pszDriverFileName = _global_strdup( szValue );
                }
                else
                {
                    if ( !pDataSource->pszDRIVER )
                        pDataSource->pszDRIVER = _global_strdup( szValue );
                }
#endif
            }
            else if ( strcasecmp( pszEntryName, "OPTION" ) == 0 )
            {    
                if ( !pDataSource->pszOPTION )
                    pDataSource->pszOPTION = _global_strdup( szValue );
            }
            else if ( strcasecmp( pszEntryName, "PWD" ) == 0 || strcasecmp( pszEntryName, "PASSWORD" ) == 0 )
            {    
                if ( !pDataSource->pszPASSWORD )
                    pDataSource->pszPASSWORD = _global_strdup( szValue );
            }
            else if ( strcasecmp( pszEntryName, "PORT" ) == 0 )
            {    
                if ( !pDataSource->pszPORT )
                    pDataSource->pszPORT = _global_strdup( szValue );
            }
            else if ( strcasecmp( pszEntryName, "SERVER" ) == 0 )
            {    
                if ( !pDataSource->pszSERVER )
                    pDataSource->pszSERVER = _global_strdup( szValue );
            }
            else if ( strcasecmp( pszEntryName, "SOCKET" ) == 0 )
            {    
                if ( !pDataSource->pszSOCKET )
                    pDataSource->pszSOCKET = _global_strdup( szValue );
            }
            else if ( strcasecmp( pszEntryName, "STMT" ) == 0 )
            {    
                if ( !pDataSource->pszSTMT )
                    pDataSource->pszSTMT = _global_strdup( szValue );
            }
            else if ( strcasecmp( pszEntryName, "UID" ) == 0 || strcasecmp( pszEntryName, "USER" ) == 0 )
            {    
                if ( !pDataSource->pszUSER )
                    pDataSource->pszUSER = _global_strdup( szValue );
            }
            else if ( strcasecmp( pszEntryName, "SSLCA" ) == 0 )
            {    
                if ( !pDataSource->pszSSLCA )
                    pDataSource->pszSSLCA = _global_strdup( szValue );
            }
            else if ( strcasecmp( pszEntryName, "SSLCAPATH" ) == 0 )
            {    
                if ( !pDataSource->pszSSLCAPATH )
                    pDataSource->pszSSLCAPATH = _global_strdup( szValue );
            }
            else if ( strcasecmp( pszEntryName, "SSLCERT" ) == 0 )
            {    
                if ( !pDataSource->pszSSLCERT )
                    pDataSource->pszSSLCERT = _global_strdup( szValue );
            }
            else if ( strcasecmp( pszEntryName, "SSLCIPHER" ) == 0 )
            {    
                if ( !pDataSource->pszSSLCIPHER )
                    pDataSource->pszSSLCIPHER = _global_strdup( szValue );
            }
            else if ( strcasecmp( pszEntryName, "SSLKEY" ) == 0 )
            {    
                if ( !pDataSource->pszSSLKEY )
                    pDataSource->pszSSLKEY = _global_strdup( szValue );
            }
            else if ( strcasecmp( pszEntryName, "SSLVERIFY" ) == 0 )
            {    
                if ( !pDataSource->pszSSLVERIFY )
                    pDataSource->pszSSLVERIFY = _global_strdup( szValue );
            }
            else if (strcasecmp(pszEntryName, "CHARSET") == 0)
            {
              if (!pDataSource->pszCHARSET)
                pDataSource->pszCHARSET= _global_strdup(szValue);
            }
            else
            {
                /* What the ? */
                fprintf( stderr, "[%s][%d][ERROR] Unknown attribute (%s).\n", __FILE__, __LINE__, pszEntryName );
            }
        }
        else
        {
            /*  
                At least two diff Users stated they did not want this message and I think we can do with out
                this one so lets silently move on.

                see; CSC 3985, email 52189

                fprintf( stderr, "[%s][%d][WARNING] Failed to get value for attribute (%s).\n", __FILE__, __LINE__, pszEntryName );
            */    
        }

        RESTORE_MODE();

        pszEntryName += strlen( pszEntryName ) + 1;
    } /* while */


    /* try harder to get the friendly driver name */
    if ( !pDataSource->pszDRIVER )
    {
        if ( SQLGetPrivateProfileString( MYODBCUTIL_ODBCINI_HEADER_SECTION, "", "", szEntryNames, sizeof( szEntryNames ) - 1, "ODBC.INI" ) < 1 )
        {
          return FALSE;
        }

        RESTORE_MODE();

        pszEntryName = szEntryNames;
        while ( *pszEntryName )
        {
            *szValue = '\0';
            if ( SQLGetPrivateProfileString( MYODBCUTIL_ODBCINI_HEADER_SECTION, pszEntryName, "", szValue, sizeof( szValue ) - 1, "ODBC.INI" ) > 0 )
            {
                if ( strcasecmp( pszEntryName, pszDSN ) == 0 )
                {
                    pDataSource->pszDRIVER = _global_strdup( szValue );
                }
            }
            RESTORE_MODE();
            pszEntryName += strlen( pszEntryName ) + 1;
        }
    }

#if defined(WIN32)
    /*!
        XP

        Some people have reported that the friendly driver name is never found so...
    */
    if ( !pDataSource->pszDRIVER )
        pDataSource->pszDRIVER = _global_strdup(MYODBCINST_DRIVER_NAME);
#endif

#if defined(WIN32)
    /* ALWAYS restore mode */
    SQLSetConfigMode( nMode );
#endif

    return TRUE;
}


