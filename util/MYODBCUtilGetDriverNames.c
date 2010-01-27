/* Copyright (C) 2000-2005 MySQL AB

   The MySQL Connector/ODBC is licensed under the terms of the
   GPL, like most MySQL Connectors. There are special exceptions
   to the terms and conditions of the GPL as it is applied to
   this software, see the FLOSS License Exception available on
   mysql.com.

   This program is free software; you can redistribute it and/or modify
   it under the terms of version 2 of the GNU General Public License as
   published by the Free Software Foundation.

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
