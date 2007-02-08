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
    \brief  Get the file name to use for SQLGetPrivateProfileString().

            Not all calls to SQLGetPrivateProfileString will want to use this
            but it should be used when getting DSN details for a SQLConnect.

    \param  bUseEnvVar True if caller wants the ODBCINI env var to be returned (if its set).
*/            
char *MYODBCUtilGetIniFileName( BOOL bUseEnvVar )
{
    char *pszIniFileName = NULL;

    if ( bUseEnvVar )
        pszIniFileName = getenv( "ODBCINI" );

    if ( !pszIniFileName )
        pszIniFileName = "odbc.ini";

    return pszIniFileName;
}

