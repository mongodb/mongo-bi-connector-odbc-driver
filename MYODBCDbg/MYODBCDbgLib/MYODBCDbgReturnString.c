/*! 
    \file     MYODBCDbgReturnString.c
    \author   Peter Harvey <pharvey@mysql.com>
              Copyright (C) MySQL AB 2004-2005, Released under GPL.
    \version  Connector/ODBC v3
    \date     2005          
    \brief    Code to provide debug information.
    
    \license  Copyright (C) 2000-2005 MySQL AB

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
              Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "../include/MYODBCDbg.h"

const char *MYODBCDbgReturnString( SQLRETURN nReturn )
{
    switch ( nReturn )
    {
        case SQL_SUCCESS:
            return "SQL_SUCCESS";
        case SQL_SUCCESS_WITH_INFO:
            return "SQL_SUCCESS_WITH_INFO";
#if (ODBCVER >= 0x0300)
        case SQL_NO_DATA:
            return "SQL_NO_DATA";
#else
        case SQL_NO_DATA_FOUND:
            return "SQL_NO_DATA_FOUND";
#endif
        case SQL_ERROR:
            return "SQL_ERROR or SQL_NULL_DATA";
        case SQL_INVALID_HANDLE:
            return "SQL_INVALID_HANDLE or SQL_DATA_AT_EXEC";
        case SQL_STILL_EXECUTING:
            return "SQL_STILL_EXECUTING";
        case SQL_NEED_DATA:
            return "SQL_NEED_DATA";
    }

    return "unknown";
}

