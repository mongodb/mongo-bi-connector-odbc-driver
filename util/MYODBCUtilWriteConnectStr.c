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
  \brief    Formulates a connection string reflecting any user modifications.

            The string can be used to pass back to SQLDriverConnect.
*/  
BOOL MYODBCUtilWriteConnectStr( MYODBCUTIL_DATASOURCE *pDataSource, char *pszStr, SQLSMALLINT nMaxLen )
{
    int nIndex = 0;

    *pszStr = '\0';

    if ( pDataSource->pszDATABASE )
    {
        if ( !MYODBCUtilInsertStr( pszStr, "DATABASE=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszDATABASE, nMaxLen, &nIndex ) )
            return FALSE;
        if ( nIndex && !MYODBCUtilInsertStr( pszStr, ";", nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszDESCRIPTION )
    {
        if ( !MYODBCUtilInsertStr( pszStr, "DESCRIPTION=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszDESCRIPTION, nMaxLen, &nIndex ) )
            return FALSE;
        if ( nIndex && !MYODBCUtilInsertStr( pszStr, ";", nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszDRIVER && pDataSource->nConnect == MYODBCUTIL_DATASOURCE_CONNECT_DRIVER )
    {
        if ( !MYODBCUtilInsertStr( pszStr, "DRIVER=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszDRIVER, nMaxLen, &nIndex ) )
            return FALSE;
        if ( nIndex && !MYODBCUtilInsertStr( pszStr, ";", nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszDSN && pDataSource->nConnect == MYODBCUTIL_DATASOURCE_CONNECT_DSN )
    {
        if ( !MYODBCUtilInsertStr( pszStr, "DSN=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszDSN, nMaxLen, &nIndex ) )
            return FALSE;
        if ( nIndex && !MYODBCUtilInsertStr( pszStr, ";", nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszOPTION )
    {
        if ( !MYODBCUtilInsertStr( pszStr, "OPTION=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszOPTION, nMaxLen, &nIndex ) )
            return FALSE;
        if ( nIndex && !MYODBCUtilInsertStr( pszStr, ";", nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszPASSWORD )
    {
        if ( !MYODBCUtilInsertStr( pszStr, "PWD=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszPASSWORD, nMaxLen, &nIndex ) )
            return FALSE;
        if ( nIndex && !MYODBCUtilInsertStr( pszStr, ";", nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszPORT )
    {
        if ( !MYODBCUtilInsertStr( pszStr, "PORT=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszPORT, nMaxLen, &nIndex ) )
            return FALSE;
        if ( nIndex && !MYODBCUtilInsertStr( pszStr, ";", nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszSERVER )
    {
        if ( !MYODBCUtilInsertStr( pszStr, "SERVER=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszSERVER, nMaxLen, &nIndex ) )
            return FALSE;
        if ( nIndex && !MYODBCUtilInsertStr( pszStr, ";", nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszSOCKET )
    {
        if ( !MYODBCUtilInsertStr( pszStr, "SOCKET=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszSOCKET, nMaxLen, &nIndex ) )
            return FALSE;
        if ( nIndex && !MYODBCUtilInsertStr( pszStr, ";", nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszSTMT )
    {
        if ( !MYODBCUtilInsertStr( pszStr, "STMT=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszSTMT, nMaxLen, &nIndex ) )
            return FALSE;
        if ( nIndex && !MYODBCUtilInsertStr( pszStr, ";", nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszUSER )
    {
        if ( !MYODBCUtilInsertStr( pszStr, "UID=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszUSER, nMaxLen, &nIndex ) )
            return FALSE;
        if ( nIndex && !MYODBCUtilInsertStr( pszStr, ";", nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszSSLCA )
    {
        if ( !MYODBCUtilInsertStr( pszStr, "SSLCA=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszSSLCA, nMaxLen, &nIndex ) )
            return FALSE;
        if ( nIndex && !MYODBCUtilInsertStr( pszStr, ";", nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszSSLCAPATH )
    {
        if ( !MYODBCUtilInsertStr( pszStr, "SSLCAPATH=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszSSLCAPATH, nMaxLen, &nIndex ) )
            return FALSE;
        if ( nIndex && !MYODBCUtilInsertStr( pszStr, ";", nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszSSLCERT )
    {
        if ( !MYODBCUtilInsertStr( pszStr, "SSLCERT=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszSSLCERT, nMaxLen, &nIndex ) )
            return FALSE;
        if ( nIndex && !MYODBCUtilInsertStr( pszStr, ";", nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszSSLCIPHER )
    {
        if ( !MYODBCUtilInsertStr( pszStr, "SSLCIPHER=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszSSLCIPHER, nMaxLen, &nIndex ) )
            return FALSE;
        if ( nIndex && !MYODBCUtilInsertStr( pszStr, ";", nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszSSLKEY )
    {
        if ( !MYODBCUtilInsertStr( pszStr, "SSLKEY=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszSSLKEY, nMaxLen, &nIndex ) )
            return FALSE;
        if ( nIndex && !MYODBCUtilInsertStr( pszStr, ";", nMaxLen, &nIndex ) )
            return FALSE;
    }

    if ( pDataSource->pszSSLVERIFY )
    {
        if ( !MYODBCUtilInsertStr( pszStr, "SSLVERIFY=", nMaxLen, &nIndex ) )
            return FALSE;
        if ( !MYODBCUtilInsertStr( pszStr, pDataSource->pszSSLVERIFY, nMaxLen, &nIndex ) )
            return FALSE;
        if ( nIndex && !MYODBCUtilInsertStr( pszStr, ";", nMaxLen, &nIndex ) )
            return FALSE;
    }

    if (pDataSource->pszCHARSET)
    {
      if (nIndex && !MYODBCUtilInsertStr(pszStr, ";", nMaxLen, &nIndex))
        return FALSE;
      if (!MYODBCUtilInsertStr(pszStr, "CHARSET=", nMaxLen, &nIndex))
        return FALSE;
      if (!MYODBCUtilInsertStr(pszStr, pDataSource->pszCHARSET, nMaxLen,
                               &nIndex))
        return FALSE;
      if (nIndex && !MYODBCUtilInsertStr(pszStr, ";", nMaxLen, &nIndex))
        return FALSE;
    }

    return TRUE;
}



