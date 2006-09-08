/*! 
    \file     MYODBCDbgDiagFieldString.c
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

const char *MYODBCDbgDiagFieldString( SQLSMALLINT nDiagField )
{
    switch ( nDiagField )
    {
        case SQL_DIAG_CURSOR_ROW_COUNT:
            return "SQL_DIAG_CURSOR_ROW_COUNT";
        case SQL_DIAG_DYNAMIC_FUNCTION:
            return "SQL_DIAG_DYNAMIC_FUNCTION";
        case SQL_DIAG_DYNAMIC_FUNCTION_CODE:
            return "SQL_DIAG_DYNAMIC_FUNCTION_CODE";
        case SQL_DIAG_NUMBER:
            return "SQL_DIAG_NUMBER";
        case SQL_DIAG_RETURNCODE:
            return "SQL_DIAG_RETURNCODE";
        case SQL_DIAG_ROW_COUNT:
            return "SQL_DIAG_ROW_COUNT";
        case SQL_DIAG_CLASS_ORIGIN:
            return "SQL_DIAG_CLASS_ORIGIN";
        case SQL_DIAG_COLUMN_NUMBER:
            return "SQL_DIAG_COLUMN_NUMBER";
        case SQL_DIAG_CONNECTION_NAME:
            return "SQL_DIAG_CONNECTION_NAME";
        case SQL_DIAG_MESSAGE_TEXT:
            return "SQL_DIAG_MESSAGE_TEXT";
        case SQL_DIAG_NATIVE:
            return "SQL_DIAG_NATIVE";
        case SQL_DIAG_ROW_NUMBER:
            return "SQL_DIAG_ROW_NUMBER";
        case SQL_DIAG_SERVER_NAME:
            return "SQL_DIAG_SERVER_NAME";
        case SQL_DIAG_SQLSTATE:
            return "SQL_DIAG_SQLSTATE";
        case SQL_DIAG_SUBCLASS_ORIGIN:
            return "SQL_DIAG_SUBCLASS_ORIGIN";
    }

    return "unknown";
}

