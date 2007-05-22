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
  \brief    Creates/replaces the given data source.

            The use of SQLWriteDSNToIni() means this function
            does not really update an existing DSN so much as
            replaces it.
*/  
BOOL MYODBCUtilWriteDataSource( MYODBCUTIL_DATASOURCE *pDataSource )
{
    /* 
        SQLWriteDSNToIni is *supposed* to replace any existing DSN
        with same name but fails (at least on unixODBC) to do so.
        So we ensure that any existing DSN with same name is removed
        with the following call.
    */     
    if ( !SQLRemoveDSNFromIni( pDataSource->pszDSN ) )
        return FALSE;

    /* create/replace data source name */
    if ( !SQLWriteDSNToIni( pDataSource->pszDSN, pDataSource->pszDRIVER ) )
        return FALSE;

    /* add details */
    if ( pDataSource->pszDATABASE && 
         !SQLWritePrivateProfileString( pDataSource->pszDSN, "DATABASE", pDataSource->pszDATABASE, "odbc.ini" ) )
        return FALSE;
    if ( pDataSource->pszDESCRIPTION &&
         !SQLWritePrivateProfileString( pDataSource->pszDSN, "DESCRIPTION", pDataSource->pszDESCRIPTION, "odbc.ini" ) )
        return FALSE;
    if ( pDataSource->pszOPTION &&
         !SQLWritePrivateProfileString( pDataSource->pszDSN, "OPTION", pDataSource->pszOPTION, "odbc.ini" ) )
        return FALSE;
    if ( pDataSource->pszPASSWORD &&
         !SQLWritePrivateProfileString( pDataSource->pszDSN, "PWD", pDataSource->pszPASSWORD, "odbc.ini" ) )
        return FALSE;
    if ( pDataSource->pszPORT &&
         !SQLWritePrivateProfileString( pDataSource->pszDSN, "PORT", pDataSource->pszPORT, "odbc.ini" ) )
        return FALSE;
    if ( pDataSource->pszSERVER &&
         !SQLWritePrivateProfileString( pDataSource->pszDSN, "SERVER", pDataSource->pszSERVER, "odbc.ini" ) )
        return FALSE;
    if ( pDataSource->pszSOCKET &&
         !SQLWritePrivateProfileString( pDataSource->pszDSN, "SOCKET", pDataSource->pszSOCKET, "odbc.ini" ) )
        return FALSE;
    if ( pDataSource->pszSTMT && 
         !SQLWritePrivateProfileString( pDataSource->pszDSN, "STMT", pDataSource->pszSTMT, "odbc.ini" ) )
        return FALSE;
    if ( pDataSource->pszUSER &&
         !SQLWritePrivateProfileString( pDataSource->pszDSN, "UID", pDataSource->pszUSER, "odbc.ini" ) )
        return FALSE;
    if ( pDataSource->pszSSLCA &&
         !SQLWritePrivateProfileString( pDataSource->pszDSN, "SSLCA", pDataSource->pszSSLCA, "odbc.ini" ) )
        return FALSE;
    if ( pDataSource->pszSSLCAPATH &&
         !SQLWritePrivateProfileString( pDataSource->pszDSN, "SSLCAPATH", pDataSource->pszSSLCAPATH, "odbc.ini" ) )
        return FALSE;
    if ( pDataSource->pszSSLCERT &&
         !SQLWritePrivateProfileString( pDataSource->pszDSN, "SSLCERT", pDataSource->pszSSLCERT, "odbc.ini" ) )
        return FALSE;
    if ( pDataSource->pszSSLCIPHER &&
         !SQLWritePrivateProfileString( pDataSource->pszDSN, "SSLCIPHER", pDataSource->pszSSLCIPHER, "odbc.ini" ) )
        return FALSE;
    if ( pDataSource->pszSSLKEY &&
         !SQLWritePrivateProfileString( pDataSource->pszDSN, "SSLKEY", pDataSource->pszSSLKEY, "odbc.ini" ) )
        return FALSE;

    return TRUE;
}


