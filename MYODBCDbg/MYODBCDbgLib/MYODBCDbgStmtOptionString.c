/*! 
    \file     MYODBCDbgStmtOptionString.c
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

const char *MYODBCDbgStmtOptionString( SQLUSMALLINT nOption )
{
    switch ( nOption )
    {
        case SQL_ASYNC_ENABLE:
            return "SQL_ASYNC_ENABLE";
        case SQL_BIND_TYPE:
            return "SQL_BIND_TYPE";
        case SQL_CONCURRENCY:
            return "SQL_CONCURRENCY";
        case SQL_CURSOR_TYPE:
            return "SQL_CURSOR_TYPE";
        case SQL_GET_BOOKMARK:
            return "SQL_GET_BOOKMARK";
        case SQL_KEYSET_SIZE:
            return "SQL_KEYSET_SIZE";
        case SQL_MAX_LENGTH:
            return "SQL_MAX_LENGTH";
        case SQL_MAX_ROWS:
            return "SQL_MAX_ROWS";
        case SQL_NOSCAN:
            return "SQL_NOSCAN";
        case SQL_QUERY_TIMEOUT:
            return "SQL_QUERY_TIMEOUT";
        case SQL_RETRIEVE_DATA:
            return "SQL_RETRIEVE_DATA";
        case SQL_ROWSET_SIZE:
            return "SQL_ROWSET_SIZE";
        case SQL_ROW_NUMBER:
            return "SQL_ROW_NUMBER";
        case SQL_SIMULATE_CURSOR:
            return "SQL_SIMULATE_CURSOR";
        case SQL_USE_BOOKMARKS:
            return "SQL_USE_BOOKMARKS";
    }

    return "unknown";
}

