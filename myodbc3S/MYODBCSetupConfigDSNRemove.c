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

#include "MYODBCSetup.h"

/*!
    \internal
    \brief      Remove given DSN.

    \note       This function uses the current SQLSetConfigMode().
*/    
BOOL MYODBCSetupConfigDSNRemove( MYODBCUTIL_DATASOURCE *pDataSource )
{
    /*!
        ODBC RULE

        To delete a data source, a data source name must be passed to ConfigDSN 
        in lpszAttributes.
    */    
    if ( !pDataSource->pszDSN )
    {
        SQLPostInstallerError( ODBC_ERROR_INVALID_KEYWORD_VALUE, "Missing DSN attribute." );
        return FALSE;
    }

    if ( !(*pDataSource->pszDSN) )
    {
        SQLPostInstallerError( ODBC_ERROR_INVALID_KEYWORD_VALUE, "Missing DSN attribute value." );
        return FALSE;
    }

    /*!
        ODBC RULE

        ConfigDSN should call SQLValidDSN to check the length of the data source 
        name and to verify that no invalid characters are included in the name.
    */    
    /*!
        MYODBC RULE

        Assumption is that this also checks to ensure we are not trying to create 
        a DSN using an odbc.ini reserved section name. 
    */
    if ( !SQLValidDSN( pDataSource->pszDSN ) )
    {
        SQLPostInstallerError( ODBC_ERROR_REQUEST_FAILED, "DSN contains illegal characters or length does not make sense." );
        return FALSE;
    }

    /*!
        ODBC RULE

        ConfigDSN checks that the data source name is in the Odbc.ini file (or 
        registry).
    */    
    if ( !MYODBCUtilDSNExists( pDataSource->pszDSN ) )
    {
        SQLPostInstallerError( ODBC_ERROR_REQUEST_FAILED, "DSN does not exist." );
        return FALSE;
    }

    /*!
        ODBC RULE

        It then calls SQLRemoveDSNFromIni in the installer DLL to remove the 
        data source.
    */    
    return SQLRemoveDSNFromIni( pDataSource->pszDSN );
}



