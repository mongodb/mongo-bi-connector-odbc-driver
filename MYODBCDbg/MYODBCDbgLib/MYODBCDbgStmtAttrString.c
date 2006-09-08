/*! 
    \file     MYODBCDbgStmtAttrString.c
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

const char *MYODBCDbgStmtAttrString( SQLINTEGER nAttribute )
{
    switch ( nAttribute )
    {
        case SQL_ATTR_APP_PARAM_DESC:
            return "SQL_ATTR_APP_PARAM_DESC";
        case SQL_ATTR_APP_ROW_DESC:
            return "SQL_ATTR_APP_ROW_DESC";
        case SQL_ATTR_ASYNC_ENABLE:
            return "SQL_ATTR_ASYNC_ENABLE";
        case SQL_ATTR_CONCURRENCY:
            return "SQL_ATTR_CONCURRENCY";
        case SQL_ATTR_CURSOR_SCROLLABLE:
            return "SQL_ATTR_CURSOR_SCROLLABLE";
        case SQL_ATTR_CURSOR_SENSITIVITY:
            return "SQL_ATTR_CURSOR_SENSITIVITY";
        case SQL_ATTR_CURSOR_TYPE:
            return "SQL_ATTR_CURSOR_TYPE";
        case SQL_ATTR_ENABLE_AUTO_IPD:
            return "SQL_ATTR_ENABLE_AUTO_IPD";
        case SQL_ATTR_FETCH_BOOKMARK_PTR:
            return "SQL_ATTR_FETCH_BOOKMARK_PTR";
        case SQL_ATTR_IMP_PARAM_DESC:
            return "SQL_ATTR_IMP_PARAM_DESC";
        case SQL_ATTR_IMP_ROW_DESC:
            return "SQL_ATTR_IMP_ROW_DESC";
        case SQL_ATTR_KEYSET_SIZE:
            return "SQL_ATTR_KEYSET_SIZE";
        case SQL_ATTR_MAX_LENGTH:
            return "SQL_ATTR_MAX_LENGTH";
        case SQL_ATTR_MAX_ROWS:
            return "SQL_ATTR_MAX_ROWS";
        case SQL_ATTR_METADATA_ID:
            return "SQL_ATTR_METADATA_ID";
        case SQL_ATTR_NOSCAN:
            return "SQL_ATTR_NOSCAN";
        case SQL_ATTR_PARAM_BIND_OFFSET_PTR:
            return "SQL_ATTR_PARAM_BIND_OFFSET_PTR";
        case SQL_ATTR_PARAM_BIND_TYPE:
            return "SQL_ATTR_PARAM_BIND_TYPE";
        case SQL_ATTR_PARAM_OPERATION_PTR:
            return "SQL_ATTR_PARAM_OPERATION_PTR";
        case SQL_ATTR_PARAM_STATUS_PTR:
            return "SQL_ATTR_PARAM_STATUS_PTR";
        case SQL_ATTR_PARAMS_PROCESSED_PTR:
            return "SQL_ATTR_PARAMS_PROCESSED_PTR";
        case SQL_ATTR_PARAMSET_SIZE:
            return "SQL_ATTR_PARAMSET_SIZE";
        case SQL_ATTR_QUERY_TIMEOUT:
            return "SQL_ATTR_QUERY_TIMEOUT";
        case SQL_ATTR_RETRIEVE_DATA:
            return "SQL_ATTR_RETRIEVE_DATA";
        case SQL_ATTR_ROW_ARRAY_SIZE:
            return "SQL_ATTR_ROW_ARRAY_SIZE";
        case SQL_ATTR_ROW_BIND_OFFSET_PTR:
            return "SQL_ATTR_ROW_BIND_OFFSET_PTR";
        case SQL_ATTR_ROW_BIND_TYPE:
            return "SQL_ATTR_ROW_BIND_TYPE";
        case SQL_ATTR_ROW_NUMBER:
            return "SQL_ATTR_ROW_NUMBER";
        case SQL_ATTR_ROW_OPERATION_PTR:
            return "SQL_ATTR_ROW_OPERATION_PTR";
        case SQL_ATTR_ROW_STATUS_PTR:
            return "SQL_ATTR_ROW_STATUS_PTR";
        case SQL_ATTR_ROWS_FETCHED_PTR:
            return "SQL_ATTR_ROWS_FETCHED_PTR";
        case SQL_ATTR_SIMULATE_CURSOR:
            return "SQL_ATTR_SIMULATE_CURSOR";
        case SQL_ATTR_USE_BOOKMARKS:
            return "SQL_ATTR_USE_BOOKMARKS";
    }

    return "unknown";
}

