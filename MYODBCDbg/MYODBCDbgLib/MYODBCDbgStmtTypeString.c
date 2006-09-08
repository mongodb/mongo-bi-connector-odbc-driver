/*! 
    \file     MYODBCDbgStmtTypeString.c
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

const char *MYODBCDbgStmtTypeString( SQLUSMALLINT nOption )
{
    switch ( nOption )
    {
        case SQL_CLOSE:
            return "SQL_CLOSE";
        case SQL_UNBIND:
            return "SQL_UNBIND";
        case SQL_DROP:
            return "SQL_DROP";
        case SQL_RESET_PARAMS:
            return "SQL_RESET_PARAMS";
        case 1000: /* MYSQL_RESET_BUFFERS:  */
            return "MYSQL_RESET_BUFFERS";
        case 1001: /* MYSQL_RESET:          */
            return "MYSQL_RESET";
    }

    return "unknown";
}



