;/*
;  Copyright (c) 2001, 2012, Oracle and/or its affiliates. All rights reserved.
;
;  The MySQL Connector/ODBC is licensed under the terms of the GPLv2
;  <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
;  MySQL Connectors. There are special exceptions to the terms and
;  conditions of the GPLv2 as it is applied to this software, see the
;  FLOSS License Exception
;  <http://www.mysql.com/about/legal/licensing/foss-exception.html>.
;            
;  This program is free software; you can redistribute it and/or modify
;  it under the terms of the GNU General Public License as published
;  by the Free Software Foundation; version 2 of the License.
;
;  This program is distributed in the hope that it will be useful, but
;  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
;  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
;  for more details.
;
;  You should have received a copy of the GNU General Public License along
;  with this program; if not, write to the Free Software Foundation, Inc.,
;  51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
;*/
;
;/* @EDIT_WARNING_MESSAGE@ */
;
LIBRARY MYODBC5@CONNECTOR_DRIVER_TYPE_SHORT@.DLL
VERSION 0@CONNECTOR_MAJOR@.@CONNECTOR_MINOR_PADDED@.@CONNECTOR_PATCH_PADDED@
;DESCRIPTION "MySQL ODBC @CONNECTOR_MAJOR@.@CONNECTOR_MINOR@(@CONNECTOR_DRIVER_TYPE_SHORT@) Driver, Copyright (c) 1995, 2012, Oracle and/or its affiliates"

;CODE MOVEABLE DISCARDABLE
;DATA MOVEABLE MULTIPLE
;PROTMODE
;SEGMENTS
;DLL_TEXT PRELOAD
;INIT_TEXT PRELOAD
;DATA PRELOAD
;HEAPSIZE 10000
; 
; Export definitions
EXPORTS
;
SQLAllocConnect
SQLAllocEnv
SQLAllocStmt
SQLAllocHandle
SQLBindCol
SQLBindParameter
SQLBrowseConnect@WIDECHARCALL@
SQLBulkOperations
SQLCancel
SQLCloseCursor
SQLColAttribute@WIDECHARCALL@
SQLColAttributes@WIDECHARCALL@
SQLColumnPrivileges@WIDECHARCALL@
SQLColumns@WIDECHARCALL@
SQLConnect@WIDECHARCALL@
SQLCopyDesc
SQLDescribeCol@WIDECHARCALL@
SQLDescribeParam
SQLDisconnect
SQLDriverConnect@WIDECHARCALL@
SQLEndTran
SQLError@WIDECHARCALL@
SQLExecDirect@WIDECHARCALL@
SQLExecute
SQLExtendedFetch
SQLFetch
SQLFetchScroll
SQLFreeConnect
SQLFreeEnv
SQLFreeStmt
SQLFreeHandle
SQLForeignKeys@WIDECHARCALL@
SQLGetConnectAttr@WIDECHARCALL@
SQLGetConnectOption@WIDECHARCALL@
SQLGetCursorName@WIDECHARCALL@
SQLGetDescField@WIDECHARCALL@
SQLGetDescRec@WIDECHARCALL@
SQLGetDiagField@WIDECHARCALL@
SQLGetDiagRec@WIDECHARCALL@
SQLGetData
SQLGetEnvAttr
SQLGetFunctions
SQLGetInfo@WIDECHARCALL@
SQLGetStmtAttr@WIDECHARCALL@
SQLGetStmtOption
SQLGetTypeInfo@WIDECHARCALL@
SQLMoreResults
SQLNativeSql@WIDECHARCALL@
SQLNumParams
SQLNumResultCols
SQLParamData
SQLParamOptions
SQLPrepare@WIDECHARCALL@
SQLPrimaryKeys@WIDECHARCALL@
SQLProcedureColumns@WIDECHARCALL@
SQLProcedures@WIDECHARCALL@
SQLPutData
SQLRowCount
SQLSetCursorName@WIDECHARCALL@
SQLSetDescField@WIDECHARCALL@
SQLSetDescRec@WIDECHARCALL@
SQLSetEnvAttr
SQLSetConnectAttr@WIDECHARCALL@
SQLSetConnectOption@WIDECHARCALL@
SQLSetParam
SQLSetPos
SQLSetScrollOptions
SQLSetStmtAttr@WIDECHARCALL@
SQLSetStmtOption
SQLSpecialColumns@WIDECHARCALL@
SQLStatistics@WIDECHARCALL@
SQLTables@WIDECHARCALL@
SQLTablePrivileges@WIDECHARCALL@
SQLTransact
;
DllMain
LoadByOrdinal
;_DllMainCRTStartup
;




