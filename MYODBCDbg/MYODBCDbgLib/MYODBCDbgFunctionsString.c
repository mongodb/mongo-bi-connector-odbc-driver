/*! 
    \file     MYODBCDbgFunctionsString.c
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

const char *MYODBCDbgFunctionsString( SQLUSMALLINT nFunction )
{
    switch ( nFunction )
    {
        case SQL_API_ALL_FUNCTIONS:
            return "SQL_API_ALL_FUNCTIONS";
        case SQL_API_SQLALLOCCONNECT:
            return "SQL_API_SQLALLOCCONNECT";
        case SQL_API_SQLALLOCENV:
            return "SQL_API_SQLALLOCENV";
        case SQL_API_SQLALLOCHANDLE:
            return "SQL_API_SQLALLOCHANDLE";
        case SQL_API_SQLALLOCSTMT:
            return "SQL_API_SQLALLOCSTMT";
        case SQL_API_SQLBINDCOL:
            return "SQL_API_SQLBINDCOL";
        case SQL_API_SQLCANCEL:
            return "SQL_API_SQLCANCEL";
        case SQL_API_SQLCLOSECURSOR:
            return "SQL_API_SQLCLOSECURSOR";
/* Same value as SQL_API_SQLCOLATTRIBUTES...
        case SQL_API_SQLCOLATTRIBUTE:
            return "SQL_API_SQLCOLATTRIBUTE";
*/
        case SQL_API_SQLCOLUMNS:
            return "SQL_API_SQLCOLUMNS";
        case SQL_API_SQLCONNECT:
            return "SQL_API_SQLCONNECT";
        case SQL_API_SQLCOPYDESC:
            return "SQL_API_SQLCOPYDESC";
        case SQL_API_SQLDATASOURCES:
            return "SQL_API_SQLDATASOURCES";
        case SQL_API_SQLDESCRIBECOL:
            return "SQL_API_SQLDESCRIBECOL";
        case SQL_API_SQLDISCONNECT:
            return "SQL_API_SQLDISCONNECT";
        case SQL_API_SQLENDTRAN:
            return "SQL_API_SQLENDTRAN";
        case SQL_API_SQLERROR:
            return "SQL_API_SQLERROR";
        case SQL_API_SQLEXECDIRECT:
            return "SQL_API_SQLEXECDIRECT";
        case SQL_API_SQLEXECUTE:
            return "SQL_API_SQLEXECUTE";
        case SQL_API_SQLFETCH:
            return "SQL_API_SQLFETCH";
        case SQL_API_SQLFETCHSCROLL:
            return "SQL_API_SQLFETCHSCROLL";
        case SQL_API_SQLFREECONNECT:
            return "SQL_API_SQLFREECONNECT";
        case SQL_API_SQLFREEENV:
            return "SQL_API_SQLFREEENV";
        case SQL_API_SQLFREEHANDLE:
            return "SQL_API_SQLFREEHANDLE";
        case SQL_API_SQLFREESTMT:
            return "SQL_API_SQLFREESTMT";
        case SQL_API_SQLGETCONNECTATTR:
            return "SQL_API_SQLGETCONNECTATTR";
        case SQL_API_SQLGETCONNECTOPTION:
            return "SQL_API_SQLGETCONNECTOPTION";
        case SQL_API_SQLGETCURSORNAME:
            return "SQL_API_SQLGETCURSORNAME";
        case SQL_API_SQLGETDATA:
            return "SQL_API_SQLGETDATA";
        case SQL_API_SQLGETDESCFIELD:
            return "SQL_API_SQLGETDESCFIELD";
        case SQL_API_SQLGETDESCREC:
            return "SQL_API_SQLGETDESCREC";
        case SQL_API_SQLGETDIAGFIELD:
            return "SQL_API_SQLGETDIAGFIELD";
        case SQL_API_SQLGETDIAGREC:
            return "SQL_API_SQLGETDIAGREC";
        case SQL_API_SQLGETENVATTR:
            return "SQL_API_SQLGETENVATTR";
        case SQL_API_SQLGETFUNCTIONS:
            return "SQL_API_SQLGETFUNCTIONS";
        case SQL_API_SQLGETINFO:
            return "SQL_API_SQLGETINFO";
        case SQL_API_SQLGETSTMTATTR:
            return "SQL_API_SQLGETSTMTATTR";
        case SQL_API_SQLGETSTMTOPTION:
            return "SQL_API_SQLGETSTMTOPTION";
        case SQL_API_SQLGETTYPEINFO:
            return "SQL_API_SQLGETTYPEINFO";
        case SQL_API_SQLNUMRESULTCOLS:
            return "SQL_API_SQLNUMRESULTCOLS";
        case SQL_API_SQLPARAMDATA:
            return "SQL_API_SQLPARAMDATA";
        case SQL_API_SQLPREPARE:
            return "SQL_API_SQLPREPARE";
        case SQL_API_SQLPUTDATA:
            return "SQL_API_SQLPUTDATA";
        case SQL_API_SQLROWCOUNT:
            return "SQL_API_SQLROWCOUNT";
        case SQL_API_SQLSETCONNECTATTR:
            return "SQL_API_SQLSETCONNECTATTR";
        case SQL_API_SQLSETCONNECTOPTION:
            return "SQL_API_SQLSETCONNECTOPTION";
        case SQL_API_SQLSETCURSORNAME:
            return "SQL_API_SQLSETCURSORNAME";
        case SQL_API_SQLSETDESCFIELD:
            return "SQL_API_SQLSETDESCFIELD";
        case SQL_API_SQLSETDESCREC:
            return "SQL_API_SQLSETDESCREC";
        case SQL_API_SQLSETENVATTR:
            return "SQL_API_SQLSETENVATTR";
        case SQL_API_SQLSETSTMTATTR:
            return "SQL_API_SQLSETSTMTATTR";
        case SQL_API_SQLSETSTMTOPTION:
            return "SQL_API_SQLSETSTMTOPTION";
        case SQL_API_SQLSPECIALCOLUMNS:
            return "SQL_API_SQLSPECIALCOLUMNS";
        case SQL_API_SQLSTATISTICS:
            return "SQL_API_SQLSTATISTICS";
        case SQL_API_SQLTABLES:
            return "SQL_API_SQLTABLES";
        case SQL_API_SQLTRANSACT:
            return "SQL_API_SQLTRANSACT";
        case SQL_API_SQLBULKOPERATIONS:
            return "SQL_API_SQLBULKOPERATIONS";
        case SQL_API_SQLBINDPARAMETER:
            return "SQL_API_SQLBINDPARAMETER";
        case SQL_API_SQLBROWSECONNECT:
            return "SQL_API_SQLBROWSECONNECT";
        case SQL_API_SQLCOLATTRIBUTES:
            return "SQL_API_SQLCOLATTRIBUTES";
        case SQL_API_SQLCOLUMNPRIVILEGES :
            return "SQL_API_SQLCOLUMNPRIVILEGES";
        case SQL_API_SQLDESCRIBEPARAM:
            return "SQL_API_SQLDESCRIBEPARAM";
        case SQL_API_SQLDRIVERCONNECT:
            return "SQL_API_SQLDRIVERCONNECT";
        case SQL_API_SQLDRIVERS:
            return "SQL_API_SQLDRIVERS";
        case SQL_API_SQLEXTENDEDFETCH:
            return "SQL_API_SQLEXTENDEDFETCH";
        case SQL_API_SQLFOREIGNKEYS:
            return "SQL_API_SQLFOREIGNKEYS";
        case SQL_API_SQLMORERESULTS:
            return "SQL_API_SQLMORERESULTS";
        case SQL_API_SQLNATIVESQL:
            return "SQL_API_SQLNATIVESQL";
        case SQL_API_SQLNUMPARAMS:
            return "SQL_API_SQLNUMPARAMS";
        case SQL_API_SQLPARAMOPTIONS:
            return "SQL_API_SQLPARAMOPTIONS";
        case SQL_API_SQLPRIMARYKEYS:
            return "SQL_API_SQLPRIMARYKEYS";
        case SQL_API_SQLPROCEDURECOLUMNS:
            return "SQL_API_SQLPROCEDURECOLUMNS";
        case SQL_API_SQLPROCEDURES:
            return "SQL_API_SQLPROCEDURES";
        case SQL_API_SQLSETPOS:
            return "SQL_API_SQLSETPOS";
        case SQL_API_SQLSETSCROLLOPTIONS:
            return "SQL_API_SQLSETSCROLLOPTIONS";
        case SQL_API_SQLTABLEPRIVILEGES:
            return "SQL_API_SQLTABLEPRIVILEGES";
        case SQL_API_ODBC3_ALL_FUNCTIONS:
            return "SQL_API_ODBC3_ALL_FUNCTIONS";
    }

    return "unknown";
}

