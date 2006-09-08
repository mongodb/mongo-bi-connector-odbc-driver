/*! 
    \file     MYODBCDbgConnectAttrString.c
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

const char *MYODBCDbgConnectAttrString( SQLINTEGER nAttribute )
{

    switch ( nAttribute )
    {
        case SQL_ATTR_ACCESS_MODE:
            return "SQL_ATTR_ACCESS_MODE";
        case SQL_ATTR_ASYNC_ENABLE:
            return "SQL_ATTR_ASYNC_ENABLE";
        case SQL_ATTR_AUTO_IPD:
            return "SQL_ATTR_AUTO_IPD";
        case SQL_ATTR_AUTOCOMMIT:
            return "SQL_ATTR_AUTOCOMMIT";
        case SQL_ATTR_CONNECTION_DEAD:
            return "SQL_ATTR_CONNECTION_DEAD";
        case SQL_ATTR_CONNECTION_TIMEOUT:
            return "SQL_ATTR_CONNECTION_TIMEOUT";
        case SQL_ATTR_CURRENT_CATALOG:
            return "SQL_ATTR_CURRENT_CATALOG";
        case SQL_ATTR_LOGIN_TIMEOUT:
            return "SQL_ATTR_LOGIN_TIMEOUT";
        case SQL_ATTR_METADATA_ID:
            return "SQL_ATTR_METADATA_ID";
        case SQL_ATTR_ODBC_CURSORS:
            return "SQL_ATTR_ODBC_CURSORS";
        case SQL_ATTR_PACKET_SIZE:
            return "SQL_ATTR_PACKET_SIZE";
        case SQL_ATTR_QUIET_MODE:
            return "SQL_ATTR_QUIET_MODE";
        case SQL_ATTR_TRACE:
            return "SQL_ATTR_TRACE";
        case SQL_ATTR_TRACEFILE:
            return "SQL_ATTR_TRACEFILE";
        case SQL_ATTR_TRANSLATE_LIB:
            return "SQL_ATTR_TRANSLATE_LIB";
        case SQL_ATTR_TRANSLATE_OPTION:
            return "SQL_ATTR_TRANSLATE_OPTION";
        case SQL_ATTR_TXN_ISOLATION:
            return "SQL_ATTR_TXN_ISOLATION";
    }

    return "unknown";
}

