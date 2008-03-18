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
    \internal
    \brief      Put a list of existing data source names in pszBuffer.

    \note       OSX

                SQLGetPrivateProfileString() is broken but fortunately
                the old GetPrivateProfileString() is available and seems
                to work fine.

    \note       XP

                SQLGetPrivateProfileString() with a NULL 1st arg does
                not return anything - ever. This is contrary to the
                specification :(

                Using SQLDataSources() is not a real option here as we
                would need to be calling into the DM for that and we
                are supposed to be below the DM in the depend hierarchy.

                So we use the deprecated precursor to 
                SQLGetPrivateProfileString().

                GetPrivateProfileString() is not aware of changes to the way
                ODBC system information is stored so we get stuff like
                "ODBC 32 bit Data Sources" list as a datasource. It also
                appears to be unaware of User vs System DSN. 
*/  
BOOL MYODBCUtilGetDataSourceNames( char *pszBuffer, int nBuffer, UWORD nScope )
{
    int nChars = 0;

    /*
        sanity check
    */    
    if ( !pszBuffer || nBuffer < 1024 )
    {
        fprintf( stderr, "[%s][%d][ERROR] Insufficient buffer size. Please provide 1k or better yet - 32k.\n", __FILE__, __LINE__ );
        return FALSE;
    }

    /* set scope */
    switch ( nScope )
    {
        case ODBC_BOTH_DSN:
            break;
        case ODBC_USER_DSN:
        case ODBC_SYSTEM_DSN:
            if ( !SQLSetConfigMode( nScope ) )
                return FALSE;
            break;
        default:
            return FALSE;
    }

#if defined(WIN32)
    /* 
        This returns no data as does having a NULL for 1st arg!?

        nChars = SQLGetPrivateProfileString( "ODBC 32 bit Data Sources", NULL, NULL, pszBuffer, nBuffer - 1, "ODBC.INI" ); 
    */
    /*
        This returns our data sources but includes "ODBC 32 bit Data Sources" as a data sources
        and does not respect config mode. Probably best until we use registery (MS is getting sloppy
        in supporting SQLGetPrivateProfileString).
    */    
    nChars = GetPrivateProfileString( NULL, NULL, NULL, pszBuffer, nBuffer - 1, "ODBC.INI" );
#else
    nChars = SQLGetPrivateProfileString( NULL, NULL, "", pszBuffer, nBuffer - 1, "ODBC.INI" );
#endif

    /* unset scope */
    switch ( nScope )
    {
        case ODBC_BOTH_DSN:
            break;
        case ODBC_USER_DSN:
        case ODBC_SYSTEM_DSN:
            SQLSetConfigMode( ODBC_BOTH_DSN );
            break;
    }

    if ( nChars < 1 )
    {
        fprintf( stderr, "[%s][%d][INFO] Call returned no data. Could be an error or just no data to return.\n", __FILE__, __LINE__ );
        return FALSE;
    }

    return TRUE;
}



