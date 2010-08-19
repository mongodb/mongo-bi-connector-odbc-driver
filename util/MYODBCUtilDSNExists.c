/* Copyright (c) 2004, 2010, Oracle and/or its affiliates. All rights reserved.

   The MySQL Connector/ODBC is licensed under the terms of the GPLv2
   <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
   MySQL Connectors. There are special exceptions to the terms and
   conditions of the GPLv2 as it is applied to this software, see the
   FLOSS License Exception
   <http://www.mysql.com/about/legal/licensing/foss-exception.html>.
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published
   by the Free Software Foundation; version 2 of the License.
   
   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
   for more details.
   
   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "MYODBCUtil.h"

/*!
    \internal
    \brief      Checks to see if a DSN exists in the ODBC system information.

    \note       This function uses the current SQLSetConfigMode().
*/                
BOOL MYODBCUtilDSNExists( char *pszDataSourceName )
{
    char    szSectionNames[SQL_MAX_DSN_LENGTH * MYODBCUTIL_MAX_DSN_NAMES];
    char *  pszSectionName;
    SAVE_MODE();

    if ( !pszDataSourceName || !(*pszDataSourceName) )
        return FALSE;

    /*!
        Get all section names. It would seem to be safest to get all sections
        names and then scan the result for match rather than try to do this
        with a single call to SQLGetPrivateProfileString() in off-chance that
        the section name exists with no name/value pairs.
    */    

#if defined(WIN32)
    /*!
        \note   WIN

                SQLGetPrivateProfileString does not work when NULL for first arg.
                so we provide first arg and hope that there is at least one 
                attribute (its likley anyway).
    */
    if ( SQLGetPrivateProfileString( pszDataSourceName, NULL, "", szSectionNames, sizeof( szSectionNames ) - 1, "ODBC.INI" ) > 0 )
        return TRUE;
    else
#else
    if ( SQLGetPrivateProfileString( NULL, NULL, "", szSectionNames, sizeof( szSectionNames ) - 1, "ODBC.INI" ) < 1 )
#endif
    {
      /*! oops - we take easy way out and just say we did not find it */
      return FALSE;
    }

    RESTORE_MODE();

    /*!
        Scan result and return TRUE if we find a match.
    */
    pszSectionName = szSectionNames;
    while( *pszSectionName )
    {
        if ( strcasecmp( pszDataSourceName, pszSectionName ) == 0 )
            return TRUE;
        pszSectionName += strlen( pszSectionName ) + 1;
    }

    return FALSE;
}



