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
  \brief    Creates/replaces the given driver registration.

            This should not be used for general register/deregister
            driver tasks - use SQLInstallDriverEx() and SQLRemoveDriver()
            for that.

            Use this when you want a more rudementary method to play with
            the driver registration.

  \note     If section pDriver->pszName does not exist it will be created
            otherwise it is updated.
*/
BOOL MYODBCUtilWriteDriver( MYODBCUTIL_DRIVER *pDriver )
{
    SAVE_MODE();
    if ( pDriver->pszName &&
         !SQLWritePrivateProfileString( pDriver->pszName, NULL, NULL, "ODBCINST.INI" ) )
    {
      return FALSE;
    }
    RESTORE_MODE();
    if ( pDriver->pszDRIVER &&
         !SQLWritePrivateProfileString( pDriver->pszName, "DRIVER", pDriver->pszDRIVER, "ODBCINST.INI" ) )
    {
      return FALSE;
    }
    RESTORE_MODE();
    if ( pDriver->pszSETUP &&
         !SQLWritePrivateProfileString( pDriver->pszName, "SETUP", pDriver->pszSETUP, "ODBCINST.INI" ) )
    {
      return FALSE;
    }
    RESTORE_MODE();

    return TRUE;
}
