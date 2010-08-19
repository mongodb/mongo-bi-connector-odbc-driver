/* Copyright (c) 2005, 2010, Oracle and/or its affiliates. All rights reserved.

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
    \brief      Put a list of installed drivers in pszBuffer.

    \note       OSX

                SQLGetPrivateProfileString() is broken but fortunately
                the old GetPrivateProfileString() is available.

    \note       XP

                SQLGetPrivateProfileString() with a NULL 1st arg does
                not return anything - ever. To return a list of drivers
                we can use SQLGetInstalledDrivers() instead.                
*/  
BOOL MYODBCUtilGetDriverNames( char *pszBuffer, int nBuffer )
{
    int     nChars = 0;
    SAVE_MODE();

    /*
        sanity check
    */    
    if ( !pszBuffer || nBuffer < 1024 )
    {
        fprintf( stderr, "[%s][%d][ERROR] Insufficient buffer size. Please provide 1k or better yet - 32k.\n", __FILE__, __LINE__ );
        return FALSE;
    }


#if defined(WIN32)
    nChars = ( SQLGetInstalledDrivers( pszBuffer, nBuffer - 1, NULL ) ? 1 : 0 );
#else
    nChars = SQLGetPrivateProfileString( NULL, NULL, "", pszBuffer, nBuffer - 1, "ODBCINST.INI" );
#endif
    if ( nChars < 1 )
    {
        fprintf( stderr, "[%s][%d][INFO] Call returned no data. Could be an error or just no data to return.\n", __FILE__, __LINE__ );
        return FALSE;
    }

    RESTORE_MODE();

    return TRUE;
}
