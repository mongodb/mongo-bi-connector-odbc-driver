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
    \brief  Read driver from system information.

    \param  pDriver     Driver struct.
    \param  pszName     User friendly name of driver such as "MySQL ODBC 3.51 Driver (32 bit)"
    \param  pszFileName File name such as value found in DSN->Driver; "/usr/lib/libmyodbc3.dylib"
        
    \note   This will not replace existing values in pDriver.
*/            
BOOL MYODBCUtilReadDriver( MYODBCUTIL_DRIVER *pDriver, LPCSTR pszName, LPCSTR pszFileName )
{
    char    szSectionNames[SQL_MAX_DSN_LENGTH * MYODBCUTIL_MAX_DSN_NAMES];
    char *  pszSectionName  = NULL;
    char    szDriverName[SQL_MAX_DSN_LENGTH + 1];
    char    szEntryNames[SQL_MAX_DSN_LENGTH * MYODBCUTIL_MAX_DSN_NAMES];
    char *  pszEntryName    = NULL;
    char    szValue[4096];
    SAVE_MODE();

    /*
     * Ensure that we have the friendly name of the driver...
     *
     */
    if ( pszName && *pszName )
    {
        /*
         * Ensure that driver name is not enclosed in {} which prevents
         * information lookup with SQLGetPrivateProfileString().
         */
        strncpy(szDriverName, pszName, SQL_MAX_DSN_LENGTH + 1);
        szDriverName[SQL_MAX_DSN_LENGTH]= 0;
        pszSectionName = szDriverName;

        if (pszSectionName[0] == '{')
          pszSectionName++;
        if (pszSectionName[strlen(pszSectionName) - 1] == '}')
          pszSectionName[strlen(pszSectionName) - 1]= 0;
    }
    else if ( pszFileName && *pszFileName )
    {
        /* get list of sections (friendly driver names)... */
        if (!MYODBCUtilGetDriverNames(szSectionNames, sizeof(szSectionNames)))
          return FALSE;

        RESTORE_MODE();

        /* get value of DRIVER entry... */
        pszSectionName = szSectionNames;
        while ( *pszSectionName )
        {
            if ( SQLGetPrivateProfileString( pszSectionName, "DRIVER", "", szValue, sizeof( szValue ) - 1, "ODBCINST.INI" ) > 0 )
            {
                RESTORE_MODE();

                if ( strcmp( szValue, pszFileName ) == 0 )
                    break;
            }

            RESTORE_MODE();

            pszSectionName += strlen( pszSectionName ) + 1;
        } /* while */
    }

    /*
     * No friendly driver name - no joy!
     *
     */
    if ( !pszSectionName )
        return FALSE;

#if defined(__APPLE__)
    if ( SQLGetPrivateProfileString( pszSectionName, "", "", szEntryNames, sizeof( szEntryNames ) - 1, "ODBCINST.INI" ) < 1 )
#else
    if ( SQLGetPrivateProfileString( pszSectionName, NULL, NULL, szEntryNames, sizeof( szEntryNames ) - 1, "ODBCINST.INI" ) < 1 )
#endif
    {
        return FALSE;
    }

    RESTORE_MODE();

    pszEntryName = szEntryNames;
    while ( *pszEntryName )
    {
        *szValue = '\0';

        if ( SQLGetPrivateProfileString( pszSectionName, pszEntryName, "", szValue, sizeof( szValue ) - 1, "ODBCINST.INI" ) > 0 )
        {
            if ( strcasecmp( pszEntryName, "DRIVER" ) == 0 )
            {
                if ( !pDriver->pszDRIVER )
                    pDriver->pszDRIVER = _global_strdup( szValue ) ;
            }
            else if ( strcasecmp( pszEntryName, "SETUP" ) == 0 )
            {
                if ( !pDriver->pszSETUP )
                    pDriver->pszSETUP = _global_strdup( szValue );
            }
            else
            {
                /* What the ? */
            }
        }

        RESTORE_MODE();

        pszEntryName += strlen( pszEntryName ) + 1;
    } /* while */

    /*
     * Success!
     *
     */
    if ( !pDriver->pszName )
        pDriver->pszName = _global_strdup( pszSectionName );

    return TRUE;
}

