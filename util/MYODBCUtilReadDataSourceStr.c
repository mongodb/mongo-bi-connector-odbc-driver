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

#ifndef HAVE_STRNDUP
char *myodbc_strndup( const char *s, size_t n )
{
    size_t nAvail;
    char *p;

    if ( !s )
        return 0;

    nAvail = myodbc_min( strlen(s) + 1, n + 1 );

    if ( nAvail < 1 )
        return 0;

    p      = (char *)malloc( nAvail );
    memcpy( p, s, nAvail );
    p[nAvail - 1] = '\0';

    return p; 
}
#endif /* ! HAVE_STRNDUP */

#if defined(WIN32)
char *strglobaldup( const char *s )
{
	size_t nAvail;
	char *p;
	
	nAvail = strlen(s) + 1;
	p = (char*)GlobalAlloc( GMEM_FIXED, nAvail );
	memcpy ( p, s, nAvail );

	return p;
}

char *strnglobaldup( const char *s, size_t n )
{
    size_t nAvail;
    char *p;

    if ( !s )
        return 0;

    nAvail = myodbc_min( strlen(s) + 1, n + 1 );

    if ( nAvail < 1 )
        return 0;

    p      = (char *)GlobalAlloc( GMEM_FIXED, nAvail );
    memcpy( p, s, nAvail );
    p[nAvail - 1] = '\0';

    return p; 
}
#endif


BOOL MYODBCUtilReadDataSourceStrValTerm( MYODBCUTIL_DELIM nDelim, char cChar )
{
    switch ( nDelim )
    {
        case MYODBCUTIL_DELIM_NULL:
            /* Terminator is a '\0'.                                                    */
            if ( cChar == '\0' )
                return TRUE;
            break;
        case MYODBCUTIL_DELIM_SEMI:
            /* Terminator is a ';'.                                                     */
            /* We check for '\0' to handle case where last value does not have a ';'.   */
            /* In such a case the '\0' is the string terminator. This makes us          */
            /* effectively same as MYODBCUTIL_DELIM_BOTH.                               */
            if ( cChar == ';' || cChar == '\0' )
                return TRUE;
            break;
        case MYODBCUTIL_DELIM_BOTH:
            /* Terminator can be either ';' or '\0'.                                    */
            if ( cChar == ';' || cChar == '\0' )
                return TRUE;
            break;
    }

    return FALSE;
}

BOOL MYODBCUtilReadDataSourceStrTerm( MYODBCUTIL_DELIM nDelim, char *pcScanChar )
{
    switch ( nDelim )
    {
        case MYODBCUTIL_DELIM_NULL:
            if ( pcScanChar[0] == '\0' && pcScanChar[1] == '\0' )
                return TRUE;
            break;
        case MYODBCUTIL_DELIM_SEMI:
            if ( pcScanChar[0] == '\0' )
                return TRUE;
            break;
        case MYODBCUTIL_DELIM_BOTH:
            /* we can not really handle both - so handle as per MYODBCUTIL_DELIM_NULL */
            if ( pcScanChar[0] == '\0' && pcScanChar[1] == '\0' )
                return TRUE;
            break;
    }

    return FALSE;
}

/*!
    \internal
    \brief      Parse a string of "name=value" pairs.

                ConfigDSN receives connection information from the installer DLL 
                as a list of attributes in the form of keyword-value pairs. 
                
    \note       This will not replace existing values in pDataSource.
*/  
BOOL MYODBCUtilReadDataSourceStr( MYODBCUTIL_DATASOURCE *pDataSource, MYODBCUTIL_DELIM nDelim, LPCSTR pszStr )
{
    MYODBCUTIL_ATTR_PARSE_STATE nState         = MYODBCUTIL_ATTR_PARSE_STATE_NAME_START;
    char *                  pAnchorChar    = (char *)pszStr;
    char *                  pScanChar      = (char *)pszStr;
    char *                  pszName        = 0;     

    /* short circuit if we have not been given stuff to parse */
    if ( !pszStr || !(*pszStr) )
        return TRUE; /* done and no errors :) */

    /* scan the input */
    while ( 1 )
    {
        switch ( nState )
        {
            case MYODBCUTIL_ATTR_PARSE_STATE_NAME_START:
                {
                    if ( isalpha( *pScanChar ) )
                    {
                        pAnchorChar = pScanChar;
                        nState = MYODBCUTIL_ATTR_PARSE_STATE_NAME; 
                    }
                }
                break;
            case MYODBCUTIL_ATTR_PARSE_STATE_NAME:
                {
                    if ( (!isalpha( *pScanChar ) && !isdigit( *pScanChar )) || *pScanChar == '=' )
                    {
                        pszName = (char *)_global_strndup( pAnchorChar, pScanChar - pAnchorChar );

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
                    if ( MYODBCUtilReadDataSourceStrValTerm( nDelim, *pScanChar ) )
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
                            if ( !pDataSource->pszDRIVER )
                                pDataSource->pszDRIVER = (char *)_global_strndup( pAnchorChar, pScanChar - pAnchorChar );
                        }
                        else if ( strcasecmp( pszName, "DSN" ) == 0 )
                        {
                            if ( !pDataSource->pszDSN )
                                pDataSource->pszDSN = (char *)_global_strndup( pAnchorChar, pScanChar - pAnchorChar );
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
                                                        pScanChar -
                                                        pAnchorChar);
                        }
                        else
                        {
                            fprintf( stderr, "[%s][%d][ERROR] Unhandled attribute (%s).\n", __FILE__, __LINE__, pszName );
                        }

                        if ( pszName )
                            _global_free( pszName );
                        pszName = 0;
                    }
                }
                break;
            default:
                fprintf( stderr, "[%s][%d][ERROR] Unhandled state.\n", __FILE__, __LINE__ );
                return FALSE;
        } /* switch */

        /* is attribute value term ? */
        if ( MYODBCUtilReadDataSourceStrValTerm( nDelim, *pScanChar ) )
            nState = MYODBCUTIL_ATTR_PARSE_STATE_NAME_START;

        /* is attribute list term ? */
        if ( MYODBCUtilReadDataSourceStrTerm( nDelim, pScanChar ) )
            break;

        pScanChar++;

    } /* while scan */

    if ( pszName )
        _global_free( pszName );

    return TRUE;
}



