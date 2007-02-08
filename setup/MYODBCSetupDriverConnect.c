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
    \brief  C entry point for our driver to invoke GUI for SQLDriverConnect.

            This function should be called from the Drivers SQLDriverConnect
            function *when prompting is required*.

            We do not connect the hDbc in here we just try to get a viable
            connect string from the User - however the User may choose to
            test the connection options - in which case we connect/disconnect.. 

            This is a bridge to the GUI code and at the same time a bridge to
            the C++ code.
*/  
BOOL MYODBCSetupDriverConnect( SQLHDBC hDBC, SQLHWND hWnd, MYODBCUTIL_DATASOURCE *pDataSource )
{
    BOOL b;
    b = MYODBCSetupDriverConnectPrompt( hDBC, hWnd, pDataSource );
    return b;
}     



