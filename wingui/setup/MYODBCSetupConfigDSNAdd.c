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
#include "../dialog/callbacks.h"

/*!
    \internal
    \brief      Adds a new DSN.

    \note       This function uses the current SQLSetConfigMode().
*/
BOOL MYODBCSetupConfigDSNAdd( HWND hWnd, DataSource *pDataSource )
{
    /*
      Hang on to the configuration mode, our setup dialog (or maybe just iODBC)
      sometimes does things that cause it to be changed.
    */
    UWORD configMode;
    if (!SQLGetConfigMode(&configMode))
      return FALSE;

    /*pDataSource-> = MYODBCUTIL_DATASOURCE_MODE_DSN_ADD;*/

    /*!
        ODBC RULE

        We must have a driver name.
    */    
    if ( !pDataSource->driver )
    {
        SQLPostInstallerErrorW( ODBC_ERROR_INVALID_NAME, L"Missing driver name." );
        return FALSE;
    }
    if ( !(*pDataSource->driver) )
    {
        SQLPostInstallerErrorW( ODBC_ERROR_INVALID_KEYWORD_VALUE, L"Missing driver name value." );
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
    if ( pDataSource->name )
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
        if ( !SQLValidDSNW( pDataSource->name ) )
        {
            SQLPostInstallerErrorW( ODBC_ERROR_REQUEST_FAILED, L"DSN contains illegal characters or length does not make sense." );
            return FALSE;
        }
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
        if ( !ShowOdbcParamsDialog( L"Adding New DSN", pDataSource, hWnd, myhelp, mytest, mytestaccept, mygetdatabases ) )
            return FALSE;

        cleanUp();
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
    if ( !pDataSource->name )
    {
        SQLPostInstallerErrorW( ODBC_ERROR_INVALID_KEYWORD_VALUE, L"Missing DSN attribute." );
        return FALSE;
    }

    if ( !(*pDataSource->name) )
    {
        SQLPostInstallerErrorW( ODBC_ERROR_INVALID_KEYWORD_VALUE, L"Missing DSN attribute value." );
        return FALSE;
    }

    /* Restore the configuration mode before we write the DSN. */
    if (!SQLSetConfigMode(configMode))
      return FALSE;

    /*!
        ODBC RULE

        If the data source name matches an existing data source name and
        hwndParent is null, ConfigDSN overwrites the existing name. If it
        matches an existing name and hwndParent is not null, ConfigDSN
        prompts the user to overwrite the existing name.
    */
    if (ds_add(pDataSource))
    {
      SQLPostInstallerErrorW(ODBC_ERROR_REQUEST_FAILED,
                            L"Writing the DSN failed." );
      return FALSE;
    }

    return TRUE;
}
