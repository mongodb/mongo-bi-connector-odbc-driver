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
    \brief  Add, edit, or remove a Data Source Name (DSN).

            This function should be called from the ODBC Administrator
            program when our driver is being used during a request to
            add, edit or remove a DSN. This allows us to do driver 
            specific stuff such as use our dialogs to work with our
            driver.

            This function is also a viable entry point and a public API
            for use by special function code such as an installer or an
            application which has embedded the driver functionality.
*/  
BOOL INSTAPI ConfigDSN( HWND hWnd, WORD nRequest, LPCSTR pszDriver, LPCSTR pszAttributes )
{
    MYODBCUTIL_DATASOURCE * pDataSource = MYODBCUtilAllocDataSource( MYODBCUTIL_DATASOURCE_MODE_DSN_VIEW );
    BOOL                    bReturn     = FALSE;

    /*
        \note   unixODBC
    
                In some cases on unixODBC a semi-colon will be used
                to indicate the end of name/value pair. This is like
                in SQLDriverConnect(). This is incorrect but we try
                to simply ignore semi-colon and hope rest of format
                is ok.

                So we should call this with MYODBCUTIL_DELIM_NULL but we use
                MYODBCUTIL_DELIM_BOTH instead.

                This was tested with pszAttributes "DSN=test;".    
    */
    if ( !MYODBCUtilReadDataSourceStr( pDataSource, MYODBCUTIL_DELIM_BOTH, pszAttributes ) )
    {
        SQLPostInstallerError( ODBC_ERROR_INVALID_KEYWORD_VALUE, "Data Source string seems invalid." );
        goto exitConfigDSN;
    }

    /*!
        ODBC RULE

        DRIVER is not a valid attribute for ConfigDSN().
        Also; ConfigDSN may not delete or change the value of the Driver keyword...
        when ODBC_CONFIG_DSN.
    */
    if ( pDataSource->pszDRIVER )
    {
        SQLPostInstallerError( ODBC_ERROR_INVALID_KEYWORD_VALUE, "DRIVER is an invalid attribute." );
        goto exitConfigDSN;
    }

    /*!
        ODBC RULE

        Driver description (usually the name of the associated DBMS) presented to users 
        instead of the physical driver name.
    */    
    if ( !pszDriver || !(*pszDriver) )
    {
        SQLPostInstallerError( ODBC_ERROR_INVALID_KEYWORD_VALUE, "Need driver name." );
        goto exitConfigDSN;
    }

    pDataSource->pszDRIVER = (char *)_global_strdup( pszDriver );

    switch ( nRequest )
    {
        case ODBC_ADD_DSN:
            bReturn = MYODBCSetupConfigDSNAdd( hWnd, pDataSource );
            break;
        case ODBC_CONFIG_DSN:
            bReturn = MYODBCSetupConfigDSNEdit( hWnd, pDataSource );
            break;
        case ODBC_REMOVE_DSN:
            bReturn = MYODBCSetupConfigDSNRemove( pDataSource );
            break;
        default:
            SQLPostInstallerError( ODBC_ERROR_INVALID_REQUEST_TYPE, "Invalid request." );
    }

exitConfigDSN:
    MYODBCUtilFreeDataSource( pDataSource );
    return bReturn;
}     
