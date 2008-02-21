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

#define MYODBCUTILWRITEDATASOURCESTR_DELIM  \
        if ( nIndex )                       \
        {                                   \
            if ( nIndex >= nMaxLen )        \
                return FALSE;               \
            pszStr[nIndex] = cDelim;        \
            nIndex++;                       \
        }

/*!
  \brief    Formulates a connection string reflecting any user modifications.

            The string can be used to pass back to SQLDriverConnect.
*/  
BOOL MYODBCUtilWriteDataSourceStr( MYODBCUTIL_DATASOURCE *pDataSource, MYODBCUTIL_DELIM nDelim, char *pszStr, SQLSMALLINT nMaxLen )
{
    int     nIndex      = 0;
    char    cDelim      = ';';

    if ( nMaxLen < 2 )
        return FALSE;

    switch ( nDelim )
    {
        case MYODBCUTIL_DELIM_NULL:
            cDelim = '\0';
            break;
        case MYODBCUTIL_DELIM_SEMI:
        case MYODBCUTIL_DELIM_BOTH:
            break;
    }

    pszStr[nIndex] = '\0';

    if ( pDataSource->pszDATABASE )
    {
        if ( !MYODBCUtilInsertStr( pszStr, "DATABASE=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszDATABASE, nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszDESCRIPTION )
    {
        MYODBCUTILWRITEDATASOURCESTR_DELIM;

        if ( !MYODBCUtilInsertStr( pszStr, "DESCRIPTION=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszDATABASE, nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszDRIVER )
    {
        MYODBCUTILWRITEDATASOURCESTR_DELIM;

        if ( !MYODBCUtilInsertStr( pszStr, "DRIVER=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszDRIVER, nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszDSN )
    {
        MYODBCUTILWRITEDATASOURCESTR_DELIM;

        if ( !MYODBCUtilInsertStr( pszStr, "DSN=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszDSN, nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszOPTION )
    {
        MYODBCUTILWRITEDATASOURCESTR_DELIM;

        if ( !MYODBCUtilInsertStr( pszStr, "OPTION=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszOPTION, nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszPASSWORD )
    {
        MYODBCUTILWRITEDATASOURCESTR_DELIM;

        if ( !MYODBCUtilInsertStr( pszStr, "PWD=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszPASSWORD, nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszPORT )
    {
        MYODBCUTILWRITEDATASOURCESTR_DELIM;

        if ( !MYODBCUtilInsertStr( pszStr, "PORT=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszPORT, nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszSERVER )
    {
        MYODBCUTILWRITEDATASOURCESTR_DELIM;

        if ( !MYODBCUtilInsertStr( pszStr, "SERVER=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszSERVER, nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszSOCKET )
    {
        MYODBCUTILWRITEDATASOURCESTR_DELIM;

        if ( !MYODBCUtilInsertStr( pszStr, "SOCKET=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszSOCKET, nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszSTMT )
    {
        MYODBCUTILWRITEDATASOURCESTR_DELIM;

        if ( !MYODBCUtilInsertStr( pszStr, "STMT=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszSTMT, nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszUSER )
    {
        MYODBCUTILWRITEDATASOURCESTR_DELIM;

        if ( !MYODBCUtilInsertStr( pszStr, "UID=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszUSER, nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszSSLCA )
    {
        MYODBCUTILWRITEDATASOURCESTR_DELIM;

        if ( !MYODBCUtilInsertStr( pszStr, "SSLCA=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszSSLCA, nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszSSLCAPATH )
    {
        MYODBCUTILWRITEDATASOURCESTR_DELIM;

        if ( !MYODBCUtilInsertStr( pszStr, "SSLCAPATH=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszSSLCAPATH, nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszSSLCERT )
    {
        MYODBCUTILWRITEDATASOURCESTR_DELIM;

        if ( !MYODBCUtilInsertStr( pszStr, "SSLCERT=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszSSLCERT, nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszSSLCIPHER )
    {
        MYODBCUTILWRITEDATASOURCESTR_DELIM;

        if ( !MYODBCUtilInsertStr( pszStr, "SSLCIPHER=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszSSLCIPHER, nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszSSLKEY )
    {
        MYODBCUTILWRITEDATASOURCESTR_DELIM;

        if ( !MYODBCUtilInsertStr( pszStr, "SSLKEY=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszSSLKEY, nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszSSLVERIFY )
    {
        MYODBCUTILWRITEDATASOURCESTR_DELIM;

        if ( !MYODBCUtilInsertStr( pszStr, "SSLVERIFY=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszSSLVERIFY, nMaxLen, &nIndex ) )
            return FALSE;
    }

    if (pDataSource->pszCHARSET)
    {
      MYODBCUTILWRITEDATASOURCESTR_DELIM;

      if (!MYODBCUtilInsertStr(pszStr, "CHARSET=", nMaxLen, &nIndex))
        return FALSE;
      if (!MYODBCUtilInsertStr(pszStr, pDataSource->pszCHARSET, nMaxLen,
                               &nIndex))
        return FALSE;
    }

    if ( nDelim == MYODBCUTIL_DELIM_NULL )
    {
        MYODBCUTILWRITEDATASOURCESTR_DELIM;
        MYODBCUTILWRITEDATASOURCESTR_DELIM;
    }

    return TRUE;
}



