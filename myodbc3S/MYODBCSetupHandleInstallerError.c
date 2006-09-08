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
    \brief  Prints any installer errors sitting in the installers
            error queue.

            General purpose error dump function for installer 
            errors. Hopefully; it provides useful information about
            any failed installer call.

    \note   Does not try to process all 8 possible records from the queue.
*/            
void MYODBCSetupHandleInstallerError()
{
  WORD      nRecord = 1;
  DWORD     nError;
  char      szError[SQL_MAX_MESSAGE_LENGTH];
  RETCODE   nReturn;

  nReturn = SQLInstallerError( nRecord, &nError, szError, SQL_MAX_MESSAGE_LENGTH - 1, 0 );
  if ( SQL_SUCCEEDED( nReturn ) )
    printf( "[%s][%d][ERROR] ODBC Installer error %d: %s\n", __FILE__, __LINE__, nError, szError );
  else
    printf( "[%s][%d][ERROR] ODBC Installer error (unknown)\n", __FILE__, __LINE__ );
}


