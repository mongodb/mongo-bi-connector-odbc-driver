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
    \brief      Adds a new DSN.

    \note       This function uses the current SQLSetConfigMode().
*/
BOOL MYODBCSetupConfigDSNAdd( HWND hWnd, MYODBCUTIL_DATASOURCE *pDataSource )
{
    SAVE_MODE();

    pDataSource->nMode = MYODBCUTIL_DATASOURCE_MODE_DSN_ADD;

    /*!
        ODBC RULE

        We must have a driver name.
    */    
    if ( !pDataSource->pszDRIVER )
    {
        SQLPostInstallerError( ODBC_ERROR_INVALID_NAME, "Missing driver name." );
        return FALSE;
    }
    if ( !(*pDataSource->pszDRIVER) )
    {
        SQLPostInstallerError( ODBC_ERROR_INVALID_KEYWORD_VALUE, "Missing driver name value." );
        return FALSE;
    }

    /*! 
        \todo 

        Use pDataSource->pszDriverFileName to get pDataSource->pszDRIVER
    */

    /*!
        ODBC RULE

        If a data source name is passed to ConfigDSN in lpszAttributes, ConfigDSN 
        checks that the name is valid.
    */    
    if ( pDataSource->pszDSN )
    {
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
            RESTORE_MODE();
            SQLPostInstallerError( ODBC_ERROR_REQUEST_FAILED, "DSN contains illegal characters or length does not make sense." );
            return FALSE;
        }

        RESTORE_MODE();
    }

    /*!
        ODBC RULE

        If lpszAttributes contains enough information to connect to a data source, 
        ConfigDSN can add the data source or display a dialog box with which the user 
        can change the connection information. If lpszAttributes does not contain 
        enough information to connect to a data source, ConfigDSN must determine the 
        necessary information; if hwndParent is not null, it displays a dialog box to 
        retrieve the information from the user.
    */
    /*!
        MYODBC RULE

        We always show the dialog if given hWnd.
    */    
    if ( hWnd )
    {
        if ( !MYODBCSetupDataSourceConfig( hWnd, pDataSource ) )
            return TRUE; /* user cancelled */
    }

    /*!
        ODBC RULE
        
        If ConfigDSN cannot get complete connection information for a data source, it 
        returns FALSE.
    */
    /*!
        MYODBC RULE

        We want pszDriver and a DSN attribute - we can default the rest.
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

        If the data source name matches an existing data source name and
        hwndParent is null, ConfigDSN overwrites the existing name. If it
        matches an existing name and hwndParent is not null, ConfigDSN
        prompts the user to overwrite the existing name.
    */
    if (!MYODBCUtilWriteDataSource(pDataSource))
    {
      SQLPostInstallerError(ODBC_ERROR_REQUEST_FAILED,
                            "Writing the DSN failed." );
      return FALSE;
    }

    return TRUE;
}
