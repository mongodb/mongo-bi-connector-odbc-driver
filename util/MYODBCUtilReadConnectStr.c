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
    \internal
    \brief      Parse a string of "name=value" pairs.

                SQLDriverConnect receives connection information 
                as a list of attributes in the form of keyword-value pairs. Each 
                pair is terminated with a ';', and the entire list is 
                terminated with a '\0'.

    \note       This will not replace existing values in pDataSource.
*/  
BOOL MYODBCUtilReadConnectStr( MYODBCUTIL_DATASOURCE *pDataSource, LPCSTR pszStr )
{
    MYODBCUTIL_ATTR_PARSE_STATE nState         = MYODBCUTIL_ATTR_PARSE_STATE_NAME_START;
    char *                  pAnchorChar    = (char *)pszStr;
    char *                  pScanChar      = (char *)pszStr;
    char *                  pszName        = 0;     

    /* short circuit if we have not been given stuff to parse */
    if ( !pszStr || !(*pszStr) )
        return FALSE;

    /* scan the input */
    while ( 1 )
    {
        switch ( nState )
        {
            case MYODBCUTIL_ATTR_PARSE_STATE_NAME_START:
                {
                    if ( isalpha( *pScanChar ) || *pScanChar == '{' )
                    {
                        pAnchorChar = pScanChar;
                        nState = MYODBCUTIL_ATTR_PARSE_STATE_NAME; 
                    }
                }
                break;
            case MYODBCUTIL_ATTR_PARSE_STATE_NAME:
                {
                    if ( (!isalpha( *pScanChar ) && !isdigit( *pScanChar ) && *pScanChar != '}') || *pScanChar == '=' )
                    {
                      /* To prevent a memory leak when use such connection strings UID=root;PWD=;SERVER=localhost;... */
                        if( pszName )
                            free( pszName );
                        pszName = myodbc_strndup( pAnchorChar, pScanChar - pAnchorChar );

                        if ( *pScanChar == '=' )
                            nState = MYODBCUTIL_ATTR_PARSE_STATE_VALUE_START;
                        else
                            nState = MYODBCUTIL_ATTR_PARSE_STATE_EQUAL;       
                    }
                }
                break;
            case MYODBCUTIL_ATTR_PARSE_STATE_EQUAL:
                {
                    if ( *pScanChar == '=' )
                        nState = MYODBCUTIL_ATTR_PARSE_STATE_VALUE_START;
                }
                break;
            case MYODBCUTIL_ATTR_PARSE_STATE_VALUE_START:
                {
                    if ( !isspace( *pScanChar ) )
                    {
                        pAnchorChar = pScanChar;
                        nState = MYODBCUTIL_ATTR_PARSE_STATE_VALUE;
                    }
                }
                break;
            case MYODBCUTIL_ATTR_PARSE_STATE_VALUE:
                {
                    /*
                        ODBC RULE

                        If any keywords are repeated in the connection string, the driver uses the value 
                        associated with the first occurrence of the keyword. If the DSN and DRIVER keywords 
                        are included in the same connection string, the Driver Manager and the driver use 
                        whichever keyword appears first.
                    */

                    /* ';' terminates name/value but we may not find one for last */
                    if ( *pScanChar == ';' || *pScanChar == '\0' )
                    {
                        if ( strcasecmp( pszName, "DATABASE" ) == 0 || strcasecmp( pszName, "DB" ) == 0 )
                        {
                            if ( !pDataSource->pszDATABASE )
                                pDataSource->pszDATABASE = (char *)_global_strndup( pAnchorChar, pScanChar - pAnchorChar ) ;
                        }
                        else if ( strcasecmp( pszName, "DESCRIPTION" ) == 0 || strcasecmp( pszName, "DESC" ) == 0 )
                        {
                            if ( !pDataSource->pszDESCRIPTION )
                                pDataSource->pszDESCRIPTION = (char *)_global_strndup( pAnchorChar, pScanChar - pAnchorChar );
                        }
                        else if ( strcasecmp( pszName, "DRIVER" ) == 0 )
                        {
                            if ( !pDataSource->pszDRIVER && !pDataSource->pszDSN ) /* we use one or other - whichever comes 1st */
                            {
                                pDataSource->pszDRIVER = (char *)_global_strndup( pAnchorChar, pScanChar - pAnchorChar ); /* friendly name such as; "MySQL ODBC 3.51 Driver" */
                                pDataSource->nConnect = MYODBCUTIL_DATASOURCE_CONNECT_DRIVER;
                            }
                        }
                        else if ( strcasecmp( pszName, "DSN" ) == 0 )
                        {
                            if ( !pDataSource->pszDSN && !pDataSource->pszDRIVER ) /* we use one or other - whichever comes 1st */
                            {
                                pDataSource->pszDSN = (char *)_global_strndup( pAnchorChar, pScanChar - pAnchorChar );
                                pDataSource->nConnect = MYODBCUTIL_DATASOURCE_CONNECT_DSN;
                            }
                        }
                        else if ( strcasecmp( pszName, "OPTION" ) == 0 )
                        {    
                            if ( !pDataSource->pszOPTION )
                                pDataSource->pszOPTION = (char *)_global_strndup( pAnchorChar, pScanChar - pAnchorChar );
                        }
                        /*
                            MYODBC RULE

                            We will 'read' variants on the standard keywords (but only write standard version).
                        */    
                        else if ( strcasecmp( pszName, "PWD" ) == 0 || strcasecmp( pszName, "PASSWORD" ) == 0 )
                        {    
                            if ( !pDataSource->pszPASSWORD )
                                pDataSource->pszPASSWORD = (char *)_global_strndup( pAnchorChar, pScanChar - pAnchorChar );
                        }
                        else if ( strcasecmp( pszName, "PORT" ) == 0 )
                        {    
                            if ( !pDataSource->pszPORT )
                                pDataSource->pszPORT = (char *)_global_strndup( pAnchorChar, pScanChar - pAnchorChar );
                        }
                        else if ( strcasecmp( pszName, "SERVER" ) == 0 )
                        {    
                            if ( !pDataSource->pszSERVER )
                                pDataSource->pszSERVER = (char *)_global_strndup( pAnchorChar, pScanChar - pAnchorChar );
                        }
                        else if ( strcasecmp( pszName, "SOCKET" ) == 0 )
                        {    
                            if ( !pDataSource->pszSOCKET )
                                pDataSource->pszSOCKET = (char *)_global_strndup( pAnchorChar, pScanChar - pAnchorChar );
                        }
                        else if ( strcasecmp( pszName, "STMT" ) == 0 )
                        {    
                            if ( !pDataSource->pszSTMT )
                                pDataSource->pszSTMT = (char *)_global_strndup( pAnchorChar, pScanChar - pAnchorChar );
                        }
                        /*
                            MYODBC RULE

                            We will 'read' variants on the standard keywords (but only write standard version).
                        */    
                        else if ( strcasecmp( pszName, "UID" ) == 0 || strcasecmp( pszName, "USER" ) == 0 )
                        {    
                            if ( !pDataSource->pszUSER )
                                pDataSource->pszUSER = (char *)_global_strndup( pAnchorChar, pScanChar - pAnchorChar );
                        }
                        else if ( strcasecmp( pszName, "SSLCA" ) == 0 )
                        {    
                            if ( !pDataSource->pszSSLCA )
                                pDataSource->pszSSLCA = (char *)_global_strndup( pAnchorChar, pScanChar - pAnchorChar );
                        }
                        else if ( strcasecmp( pszName, "SSLCAPATH" ) == 0 )
                        {    
                            if ( !pDataSource->pszSSLCAPATH )
                                pDataSource->pszSSLCAPATH = (char *)_global_strndup( pAnchorChar, pScanChar - pAnchorChar );
                        }
                        else if ( strcasecmp( pszName, "SSLCERT" ) == 0 )
                        {    
                            if ( !pDataSource->pszSSLCERT )
                                pDataSource->pszSSLCERT = (char *)_global_strndup( pAnchorChar, pScanChar - pAnchorChar );
                        }
                        else if ( strcasecmp( pszName, "SSLCIPHER" ) == 0 )
                        {    
                            if ( !pDataSource->pszSSLCIPHER )
                                pDataSource->pszSSLCIPHER = (char *)_global_strndup( pAnchorChar, pScanChar - pAnchorChar );
                        }
                        else if ( strcasecmp( pszName, "SSLKEY" ) == 0 )
                        {    
                            if ( !pDataSource->pszSSLKEY )
                                pDataSource->pszSSLKEY = (char *)_global_strndup( pAnchorChar, pScanChar - pAnchorChar );
                        }
                        else if ( strcasecmp( pszName, "SSLVERIFY" ) == 0 )
                        {    
                            if ( !pDataSource->pszSSLVERIFY )
                                pDataSource->pszSSLVERIFY = (char *)_global_strndup( pAnchorChar, pScanChar - pAnchorChar );
                        }
                        else if (strcasecmp(pszName, "CHARSET") == 0)
                        {
                          if (!pDataSource->pszCHARSET)
                            pDataSource->pszCHARSET=
                              (char *)_global_strndup(pAnchorChar,
                                                      pScanChar - pAnchorChar);
                        }
                        else if ( strcasecmp( pszName, "SAVEFILE" ) == 0 )
                        {
                          pDataSource->bSaveFileDSN = TRUE;
                        }
                        else
                        {
                            /* What the ? */
                        }

                        if ( pszName )
                            free( pszName );
                        pszName = 0;
                    }
                }
                break;
            default:
                fprintf( stderr, "[%s][%d][ERROR] Unhandled state.\n", __FILE__, __LINE__ );
                return FALSE;
        }

        /* ';' is used to terminate a name/value pair */
        if ( *pScanChar == ';' )
            nState = MYODBCUTIL_ATTR_PARSE_STATE_NAME_START;

        /* null terminates */
        if ( pScanChar[0] == '\0' )
            break;

        pScanChar++;

    } /* while scan */

    if ( pszName )
        free( pszName );

    return TRUE;
}



