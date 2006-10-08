/* Copyright (C) 1995-2006 MySQL AB

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

/***************************************************************************
 * OPTIONS.C								   *
 *									   *
 * @description: For handling attribute APIs				   *
 *									   *
 * @author     : MySQL AB(monty@mysql.com, venu@mysql.com)		   *
 * @date       : 2001-Aug-15						   *
 * @product    : myodbc3						   *
 *									   *
 ****************************************************************************/

/***************************************************************************
 * The following ODBC APIs are implemented in this file:		   *
 *									   *
 *   SQLSetEnvAttr	 (ISO 92)					   *
 *   SQLGetEnvAttr	 (ISO 92)					   *
 *   SQLSetConnectAttr	 (ISO 92)					   *
 *   SQLGetConnecyAttr	 (ISO 92)					   *
 *   SQLSetStmtAttr	 (ISO 92)					   *
 *   SQLGetStmtAttr	 (ISO 92)					   *
 *   SQLSetConnectOption (ODBC, Deprecated)				   *
 *   SQLGetConnectOption (ODBC, Deprecated)				   *
 *   SQLSetStmtOption	 (ODBC, Deprecated)				   *
 *   SQLGetStmtOption	 (ODBC, Deprecated)				   *
 *									   *
 ****************************************************************************/

#include "myodbc3.h"
#include "errmsg.h"

/*
  @type    : myodbc3 internal
  @purpose : sets the common connection/stmt attributes
*/

static SQLRETURN set_constmt_attr(SQLSMALLINT  HandleType,
                                  SQLHANDLE    Handle,
                                  STMT_OPTIONS *options,
                                  SQLINTEGER   Attribute,
                                  SQLPOINTER   ValuePtr)
{
    MYODBCDbgEnter;

    switch (Attribute)
    {
        case SQL_ATTR_ASYNC_ENABLE:
            if (ValuePtr == (SQLPOINTER) SQL_ASYNC_ENABLE_ON)
                MYODBCDbgReturnReturn(set_handle_error(HandleType,Handle,MYERR_01S02,
                                                 "Doesn't support asynchronous, changed to default",0));
            break;

        case SQL_ATTR_CURSOR_SENSITIVITY:
            if (ValuePtr != (SQLPOINTER) SQL_UNSPECIFIED)
            {
                MYODBCDbgReturnReturn(set_handle_error(HandleType,Handle,MYERR_01S02,
                                                 "Option value changed to default cursor sensitivity(unspecified)",0));
            }
            break;

        case SQL_ATTR_CURSOR_TYPE:
            if (((STMT FAR*)Handle)->dbc->flag & FLAG_FORWARD_CURSOR)
            {
                options->cursor_type= SQL_CURSOR_FORWARD_ONLY;
                if (ValuePtr != (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY)
                    MYODBCDbgReturnReturn(set_handle_error(HandleType,Handle,MYERR_01S02,
                                                     "Forcing the use of forward-only cursor)",0));
            }
            else if (((STMT FAR*)Handle)->dbc->flag & FLAG_DYNAMIC_CURSOR)
            {
                if (ValuePtr != (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN)
                    options->cursor_type= (SQLUINTEGER)ValuePtr;

                else
                {
                    options->cursor_type= SQL_CURSOR_STATIC;
                    MYODBCDbgReturnReturn(set_handle_error(HandleType,Handle,MYERR_01S02,
                                                     "Option value changed to default static cursor",0));
                }
            }
            else
            {
                if (ValuePtr == (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY ||
                    ValuePtr == (SQLPOINTER)SQL_CURSOR_STATIC)
                    options->cursor_type= (SQLUINTEGER)ValuePtr;

                else
                {
                    options->cursor_type= SQL_CURSOR_STATIC;
                    MYODBCDbgReturnReturn(set_handle_error(HandleType,Handle,MYERR_01S02,
                                                     "Option value changed to default static cursor",0));
                }
            }
            break;

        case SQL_ATTR_MAX_LENGTH:
            options->max_length= (SQLUINTEGER) ValuePtr;
            break;

        case SQL_ATTR_MAX_ROWS:
            options->max_rows= (SQLUINTEGER) ValuePtr;
            break;

        case SQL_ATTR_METADATA_ID:
            if (ValuePtr == (SQLPOINTER) SQL_TRUE)
                MYODBCDbgReturnReturn(set_handle_error(HandleType,Handle,MYERR_01S02,
                                                 "Doesn't support SQL_ATTR_METADATA_ID to true, changed to default",0));
            break;

        case SQL_ATTR_RETRIEVE_DATA:
            MYODBCDbgInfo( "%s", "SQL_ATTR_RETRIEVE_DATA value is ignored" );
            break;

        case SQL_ATTR_ROW_BIND_TYPE:
            options->bind_type= (SQLUINTEGER) ValuePtr;
            break;

        case SQL_ATTR_SIMULATE_CURSOR:
            if (ValuePtr != (SQLPOINTER) SQL_SC_TRY_UNIQUE)
                MYODBCDbgReturnReturn(set_handle_error(HandleType,Handle,MYERR_01S02,
                                                 "Option value changed to default cursor simulation",0));
            break;

        case SQL_ATTR_ROW_BIND_OFFSET_PTR:
            options->bind_offset= (SQLINTEGER *)ValuePtr;
            break;

        case 1226:/* MS SQL Server Extension */
        case 1227:
        case 1228:
            break;

        case SQL_ATTR_QUERY_TIMEOUT:
        case SQL_ATTR_KEYSET_SIZE:
        case SQL_ATTR_CONCURRENCY:
        case SQL_ATTR_NOSCAN:
        case SQL_ATTR_ROW_OPERATION_PTR: /* need to support this ....*/
            MYODBCDbgInfo( "Attribute, %d is ignored", Attribute );
            break;

        case SQL_ATTR_FETCH_BOOKMARK_PTR:
        case SQL_ATTR_USE_BOOKMARKS:
            MYODBCDbgReturnReturn(set_handle_error(HandleType,Handle,MYERR_S1C00,NULL,0));
            break;

        default:
            MYODBCDbgInfo( "Attribute, %d is ignored", Attribute );
            break;
    }
    MYODBCDbgReturnReturn(SQL_SUCCESS);
}


/*
  @type    : myodbc3 internal
  @purpose : returns the common connection/stmt attributes
*/

static SQLRETURN
get_constmt_attr(SQLSMALLINT  HandleType,
                 SQLHANDLE    Handle,
                 STMT_OPTIONS *options,
                 SQLINTEGER   Attribute,
                 SQLPOINTER   ValuePtr,
                 SQLINTEGER   *StringLengthPtr __attribute__((unused)))
{
    MYODBCDbgEnter;

    switch (Attribute)
    {
        
        case SQL_ATTR_ASYNC_ENABLE:
            *((SQLUINTEGER *) ValuePtr)= SQL_ASYNC_ENABLE_OFF;
            break;

        case SQL_ATTR_CURSOR_SENSITIVITY:
            *((SQLUINTEGER *) ValuePtr)= SQL_UNSPECIFIED;
            break;

        case SQL_ATTR_CURSOR_TYPE:
            *((SQLUINTEGER *) ValuePtr)= options->cursor_type;
            break;

        case SQL_ATTR_MAX_LENGTH:
            *((SQLUINTEGER *) ValuePtr)= options->max_length;
            break;

        case SQL_ATTR_MAX_ROWS:
            *((SQLUINTEGER *) ValuePtr)= options->max_rows;
            break;

        case SQL_ATTR_METADATA_ID:
            *((SQLUINTEGER *) ValuePtr)= SQL_FALSE;
            break;

        case SQL_ATTR_QUERY_TIMEOUT:
            *((SQLUINTEGER *) ValuePtr)= SQL_QUERY_TIMEOUT_DEFAULT;
            break;

        case SQL_ATTR_RETRIEVE_DATA:
            *((SQLUINTEGER *) ValuePtr)= SQL_RD_DEFAULT;
            break;

        case SQL_ATTR_ROW_BIND_TYPE:
            *((SQLUINTEGER *) ValuePtr)= options->bind_type;
            break;

        case SQL_ATTR_SIMULATE_CURSOR:
            *((SQLUINTEGER *) ValuePtr)= SQL_SC_TRY_UNIQUE;
            break;

        case SQL_ATTR_CONCURRENCY:
            *((SQLUINTEGER *) ValuePtr)= SQL_CONCUR_READ_ONLY;
            break;

        case SQL_KEYSET_SIZE:
            *((SQLUINTEGER *) ValuePtr)= 0L;
            break;

        case SQL_ROWSET_SIZE:
            *(SQLUINTEGER *)ValuePtr= options->rows_in_set;
            break;

        case SQL_NOSCAN:
            *((SQLUINTEGER *) ValuePtr)= SQL_NOSCAN_ON;
            break;

        case SQL_ATTR_ROW_BIND_OFFSET_PTR:
            *((SQLINTEGER *) ValuePtr)= options->bind_offset ?
                                        *(options->bind_offset):
                                        0;
            break;

        case SQL_ATTR_ROW_OPERATION_PTR: /* need to support this ....*/
            MYODBCDbgInfo( "Attribute, %d is ignored", Attribute );
            MYODBCDbgReturnReturn(SQL_SUCCESS_WITH_INFO);

        case SQL_ATTR_FETCH_BOOKMARK_PTR:
        case SQL_ATTR_USE_BOOKMARKS:
            MYODBCDbgReturnReturn(set_handle_error(HandleType,Handle,MYERR_S1C00,NULL,0));

        case 1226:/* MS SQL Server Extension */
        case 1227:
        case 1228:
            break;

        default:
            MYODBCDbgError( "Invalid attribute/option identifier:%d", Attribute );
    }
    MYODBCDbgReturnReturn(SQL_SUCCESS);
}


/*
  @type    : myodbc3 internal
  @purpose : sets the connection attributes
*/

static SQLRETURN
set_con_attr(SQLHDBC    hdbc,
             SQLINTEGER Attribute,
             SQLPOINTER ValuePtr,
             SQLINTEGER StringLengthPtr)
{
    DBC FAR *dbc= (DBC FAR*) hdbc;

    MYODBCDbgEnter;

    MYODBCDbgInfo( "Atrr: %d", Attribute );
    MYODBCDbgInfo( "ValuePtr: 0x%lx", ValuePtr );
    MYODBCDbgInfo( "StrLenPtr: %d", StringLengthPtr );

    switch (Attribute)
    {
        
        case SQL_ATTR_ACCESS_MODE:
            MYODBCDbgInfo( "SQL_ATTR_ACCESS_MODE %ld ignored", (SQLUINTEGER)ValuePtr );
            break;

        case SQL_ATTR_AUTOCOMMIT:
            if (ValuePtr != (SQLPOINTER) SQL_AUTOCOMMIT_ON)
            {
                if (!dbc->server) /* no connection yet */
                {
                    dbc->commit_flag= CHECK_AUTOCOMMIT_OFF;
                    MYODBCDbgReturnReturn(SQL_SUCCESS);
                }
                if (!(trans_supported(dbc)) || (dbc->flag & FLAG_NO_TRANSACTIONS))
                    MYODBCDbgReturnReturn(set_conn_error(dbc,MYERR_S1C00,
                                                   "Transactions are not enabled", 4000));

                if (autocommit_on(dbc))
                    MYODBCDbgReturnReturn(odbc_stmt(dbc,"SET AUTOCOMMIT=0"));
            }
            else if (!dbc->server) /* no connection yet */
            {
                dbc->commit_flag= CHECK_AUTOCOMMIT_ON;
                MYODBCDbgReturnReturn(SQL_SUCCESS);
            }
            else if (trans_supported(dbc) && !(autocommit_on(dbc)))
                MYODBCDbgReturnReturn(odbc_stmt(dbc,"SET AUTOCOMMIT=1"));
            break;

        case SQL_ATTR_CONNECTION_TIMEOUT:
            {
                uint timeout= (SQLUINTEGER)ValuePtr;
                MYODBCDbgReturnReturn(mysql_options(&dbc->mysql, MYSQL_OPT_CONNECT_TIMEOUT,
                                              (const char *)&timeout));
            }
            break;

            /*
                  If this is done before connect (I would say a function 
                  sequence but .NET IDE does this) then we store the value but
                  it is quite likely that it will get replaced by DATABASE in
                  a DSN or connect string.
            */
        case SQL_ATTR_CURRENT_CATALOG:
            {
                char ldb[NAME_LEN+1], *db;

                if (!(db= fix_str((char *)ldb, (char *)ValuePtr, StringLengthPtr)))
                    MYODBCDbgReturnReturn(set_conn_error(hdbc,MYERR_S1009,NULL, 0));

                pthread_mutex_lock(&dbc->lock);
                if ( dbc->mysql.net.vio )
                {
                    if (mysql_select_db(&dbc->mysql,(char*) db))
                    {
                        set_conn_error(dbc,MYERR_S1000,mysql_error(&dbc->mysql),mysql_errno(&dbc->mysql));
                        pthread_mutex_unlock(&dbc->lock);
                        MYODBCDbgReturnReturn(SQL_ERROR);
                    }
                }
                my_free((gptr) dbc->database,MYF(0));
                dbc->database= my_strdup(db,MYF(MY_WME));
                pthread_mutex_unlock(&dbc->lock);
            }
            break;

        case SQL_ATTR_LOGIN_TIMEOUT:
            dbc->login_timeout= (SQLUINTEGER)ValuePtr;
            break;

        case SQL_ATTR_ODBC_CURSORS:
            if ((dbc->flag & FLAG_FORWARD_CURSOR) &&
                ValuePtr != (SQLPOINTER) SQL_CUR_USE_ODBC)
                MYODBCDbgReturnReturn(set_conn_error(hdbc,MYERR_01S02,
                                               "Forcing the Driver Manager to use ODBC cursor library",0));
            break;

        case SQL_OPT_TRACE:
        case SQL_OPT_TRACEFILE:
        case SQL_QUIET_MODE:
        case SQL_TRANSLATE_DLL:
        case SQL_TRANSLATE_OPTION:
            {
                char buff[100];
                sprintf(buff,"Suppose to set this attribute '%d' through driver manager, not by the driver",(int) Attribute);
                MYODBCDbgReturnReturn(set_conn_error(hdbc,MYERR_01S02,buff,0));
            }

        case SQL_ATTR_PACKET_SIZE:
            MYODBCDbgInfo( "SQL_ATTR_PACKET_SIZE %ld ignored", (SQLUINTEGER)ValuePtr );
            break;

        case SQL_ATTR_TXN_ISOLATION:
            if (!dbc->server)  /* no connection yet */
            {
                dbc->txn_isolation= (SQLINTEGER)ValuePtr;
                MYODBCDbgReturnReturn(SQL_SUCCESS);
            }
            if (trans_supported(dbc))
            {
                char buff[80];
                const char *level;

                if ((SQLINTEGER)ValuePtr & SQL_TXN_SERIALIZABLE)
                    level="SERIALIZABLE";
                else if ((SQLINTEGER)ValuePtr & SQL_TXN_REPEATABLE_READ)
                    level="REPEATABLE READ";
                else if ((SQLINTEGER)ValuePtr & SQL_TXN_REPEATABLE_READ)
                    level="READ COMMITTED";
                else
                    level="READ UNCOMMITTED";
                sprintf(buff,"SET SESSION TRANSACTION ISOLATION LEVEL %s",level);
                if (odbc_stmt(dbc,buff) == SQL_SUCCESS)
                    dbc->txn_isolation= (SQLINTEGER)ValuePtr;
            }
            else
                MYODBCDbgInfo( "SQL_ATTR_TXN_ISOLATION %ld ignored", (SQLINTEGER)ValuePtr );
            break;

            /*
              3.x driver doesn't support any statement attributes
              at connection level, but to make sure all 2.x apps
              works fine...lets support it..nothing to loose..
            */
        default:
            MYODBCDbgReturnReturn(set_constmt_attr(2,dbc,&dbc->stmt_options,Attribute,ValuePtr));
    }
    MYODBCDbgReturnReturn(SQL_SUCCESS);
}


/*
  @type    : myodbc3 internal
  @purpose : returns the connection attribute values
*/

static SQLRETURN get_con_attr(SQLHDBC    hdbc,
                              SQLINTEGER Attribute,
                              SQLPOINTER ValuePtr,
                              SQLINTEGER BufferLength,
                              SQLINTEGER *StringLengthPtr)
{
    DBC FAR *dbc= (DBC FAR*) hdbc;
    SQLRETURN result= SQL_SUCCESS;
    SQLINTEGER strlen;
    SQLPOINTER vparam= 0;

    MYODBCDbgEnter;

    MYODBCDbgInfo( "Atrr: %d", Attribute );
    MYODBCDbgInfo( "ValuePtr: 0x%lx", ValuePtr );
    MYODBCDbgInfo( "BufLen: %d", BufferLength );
    MYODBCDbgInfo( "StrLenPtr: 0x%lx", StringLengthPtr );

    if (!ValuePtr)
        ValuePtr= vparam;

    if (!StringLengthPtr)
        StringLengthPtr= &strlen;

    switch (Attribute)
    {
        
        case SQL_ATTR_ACCESS_MODE:
            *((SQLUINTEGER *) ValuePtr)= SQL_MODE_READ_WRITE;
            break;

        case SQL_ATTR_AUTO_IPD:
            *((SQLUINTEGER *) ValuePtr)= SQL_FALSE;
            break;

        case SQL_ATTR_AUTOCOMMIT:
            *((SQLUINTEGER *)ValuePtr)= (autocommit_on(dbc) ||
                                         !(trans_supported(dbc)) ?
                                         SQL_AUTOCOMMIT_ON :
                                         SQL_AUTOCOMMIT_OFF);
            break;

        case SQL_ATTR_CONNECTION_DEAD:
            {
                if ( mysql_ping( &dbc->mysql ) && mysql_errno( &dbc->mysql ) == CR_SERVER_LOST )
                    *((SQLUINTEGER *) ValuePtr)= SQL_CD_TRUE;
                else
                    *((SQLUINTEGER *) ValuePtr)= SQL_CD_FALSE;
            }
            break;

        case SQL_ATTR_CONNECTION_TIMEOUT:
#if MYSQL_VERSION_ID < 40003
            *((SQLUINTEGER *) ValuePtr)= dbc->mysql.net.timeout;
#else
            *((SQLUINTEGER *) ValuePtr)= dbc->mysql.net.read_timeout;
#endif
            break;

        case SQL_ATTR_CURRENT_CATALOG:

            if (reget_current_catalog(dbc))
            {
                result= SQL_ERROR;
            }
            else
            {
                *StringLengthPtr= (SQLSMALLINT) (strmake((char*)ValuePtr,dbc->database,
                                                         BufferLength) -
                                                 (char*) ValuePtr);
            }
            break;

        case SQL_ATTR_LOGIN_TIMEOUT:
            *((SQLUINTEGER *) ValuePtr)= dbc->login_timeout;
            break;

        case SQL_ATTR_ODBC_CURSORS:
            if ((dbc->flag & FLAG_FORWARD_CURSOR))
                *((SQLUINTEGER *) ValuePtr)= SQL_CUR_USE_ODBC;
            else
                *((SQLUINTEGER *) ValuePtr)= SQL_CUR_USE_IF_NEEDED;
            break;

        case SQL_OPT_TRACE:
        case SQL_OPT_TRACEFILE:
        case SQL_QUIET_MODE:
        case SQL_TRANSLATE_DLL:
        case SQL_TRANSLATE_OPTION:
            {
                char buff[100];
                sprintf(buff,
                        "Suppose to get this attribute '%d' through driver manager, not by the driver",
                        (int) Attribute);
                result= set_conn_error(hdbc,MYERR_01S02,buff,0);
                break;
            }

        case SQL_ATTR_PACKET_SIZE:
            *((SQLUINTEGER *) ValuePtr)= dbc->mysql.net.max_packet;
            break;

        case SQL_ATTR_TXN_ISOLATION:
            *((SQLINTEGER *) ValuePtr)= dbc->txn_isolation;
            break;

            /*
              3.x driver doesn't support any statement attributes
              at connection level, but to make sure all 2.x apps
              works fine...lets support it..nothing to loose..
            */
        default:
            result= get_constmt_attr(2,dbc,&dbc->stmt_options,
                                     Attribute,ValuePtr,
                                     StringLengthPtr);
    }
    MYODBCDbgReturnReturn(result);
}


/*
  @type    : myodbc3 internal
  @purpose : sets the statement attributes
*/

static SQLRETURN
set_stmt_attr(SQLHSTMT   hstmt,
              SQLINTEGER Attribute,
              SQLPOINTER ValuePtr,
              SQLINTEGER StringLengthPtr __attribute__((unused)))
{
    STMT FAR *stmt= (STMT FAR*) hstmt;
    SQLRETURN result= SQL_SUCCESS;
    STMT_OPTIONS *options= &stmt->stmt_options;

    MYODBCDbgEnter;

    MYODBCDbgInfo( "Atrr: %d", Attribute );
    MYODBCDbgInfo( "ValuePtr: 0x%lx", ValuePtr );
    MYODBCDbgInfo( "StrLenPtr: %d", StringLengthPtr );

    switch (Attribute)
    {
        
        case SQL_ATTR_CURSOR_SCROLLABLE:
            if (ValuePtr == (SQLPOINTER)SQL_NONSCROLLABLE &&
                options->cursor_type != SQL_CURSOR_FORWARD_ONLY)
                options->cursor_type= SQL_CURSOR_FORWARD_ONLY;

            else if (ValuePtr == (SQLPOINTER)SQL_SCROLLABLE &&
                     options->cursor_type == SQL_CURSOR_FORWARD_ONLY)
                options->cursor_type= SQL_CURSOR_STATIC;
            break;

        case SQL_ATTR_AUTO_IPD:
            if (ValuePtr != (SQLPOINTER)SQL_FALSE)
                MYODBCDbgReturnReturn(set_error(hstmt,MYERR_01S02,
                                          "Option value changed to default auto ipd",0));
            break;

        case SQL_ATTR_PARAM_STATUS_PTR: /* need to support this ....*/
            options->paramStatusPtr= (SQLUSMALLINT *)ValuePtr;
            break;

        case SQL_ATTR_PARAMS_PROCESSED_PTR: /* need to support this ....*/
            options->paramProcessedPtr= (SQLUINTEGER *)ValuePtr;
            break;

        case SQL_ATTR_PARAMSET_SIZE:
            if (ValuePtr != (SQLPOINTER)1)
                MYODBCDbgReturnReturn(set_error(hstmt,MYERR_01S02,
                                          "Option value changed to default parameter size",
                                          0));
            break;

        case SQL_ATTR_ROW_ARRAY_SIZE:
        case SQL_ROWSET_SIZE:
            options->rows_in_set= (SQLUINTEGER)ValuePtr;
            break;

        case SQL_ATTR_ROW_NUMBER:
            MYODBCDbgReturnReturn(set_error(hstmt,MYERR_S1000,
                                      "Trying to set read-only attribute",0));
            break;

        case SQL_ATTR_ROW_OPERATION_PTR:
            options->rowOperationPtr= (SQLUSMALLINT *)ValuePtr;
            break;

        case SQL_ATTR_ROW_STATUS_PTR:
            options->rowStatusPtr= (SQLUSMALLINT *)ValuePtr;
            break;

        case SQL_ATTR_ROWS_FETCHED_PTR:
            options->rowsFetchedPtr= (SQLUINTEGER *)ValuePtr;
            break;

        case SQL_ATTR_SIMULATE_CURSOR:
            options->simulateCursor= (SQLUINTEGER)ValuePtr;
            break;

            /*
              3.x driver doesn't support any statement attributes
              at connection level, but to make sure all 2.x apps
              works fine...lets support it..nothing to loose..
            */
        default:
            result= set_constmt_attr(3,hstmt,options,
                                     Attribute,ValuePtr);
    }
    MYODBCDbgReturnReturn(result);
}


/*
  @type    : myodbc3 internal
  @purpose : returns the statement attribute values
*/

static SQLRETURN get_stmt_attr(SQLHSTMT   hstmt,
                               SQLINTEGER Attribute,
                               SQLPOINTER ValuePtr,
                               SQLINTEGER BufferLength __attribute__((unused)),
                               SQLINTEGER *StringLengthPtr)
{
    SQLRETURN result= SQL_SUCCESS;
    STMT FAR *stmt= (STMT FAR*) hstmt;
    STMT_OPTIONS *options= &stmt->stmt_options;
    SQLPOINTER vparam;
    SQLINTEGER strlen;

    MYODBCDbgEnter;
    MYODBCDbgInfo( "Atrr: %d", Attribute );
    MYODBCDbgInfo( "ValuePtr: 0x%lx", ValuePtr );
    MYODBCDbgInfo( "BufLen: %d", BufferLength );
    MYODBCDbgInfo( "StrLenPtr: 0x%lx", StringLengthPtr );

    if (!ValuePtr)
        ValuePtr= &vparam;

    if (!StringLengthPtr)
        StringLengthPtr= &strlen;

    switch (Attribute)
    {
        
        case SQL_ATTR_CURSOR_SCROLLABLE:
            if (options->cursor_type == SQL_CURSOR_FORWARD_ONLY)
                *(SQLUINTEGER*)ValuePtr= SQL_NONSCROLLABLE;
            else
                *(SQLUINTEGER*)ValuePtr= SQL_SCROLLABLE;
            break;

        case SQL_ATTR_AUTO_IPD:
            *(SQLUINTEGER *)ValuePtr= SQL_FALSE;
            break;

        case SQL_ATTR_PARAM_STATUS_PTR: /* need to support this ....*/
            ValuePtr= (SQLUSMALLINT *)options->paramStatusPtr;
            break;

        case SQL_ATTR_PARAMS_PROCESSED_PTR: /* need to support this ....*/
            ValuePtr= (SQLUSMALLINT *)options->paramProcessedPtr;
            break;

        case SQL_ATTR_PARAMSET_SIZE:
            *(SQLUINTEGER *)ValuePtr= 1;
            break;

        case SQL_ATTR_ROW_ARRAY_SIZE:
            *(SQLUINTEGER *)ValuePtr= options->rows_in_set;
            break;

        case SQL_ATTR_ROW_NUMBER:
            *(SQLUINTEGER *)ValuePtr= stmt->current_row+1;
            break;

        case SQL_ATTR_ROW_OPERATION_PTR: /* need to support this ....*/
            ValuePtr= (SQLUSMALLINT *)options->rowOperationPtr;
            break;

        case SQL_ATTR_ROW_STATUS_PTR:
            ValuePtr= (SQLUSMALLINT *)options->rowStatusPtr;
            break;

        case SQL_ATTR_ROWS_FETCHED_PTR:
            ValuePtr= (SQLUINTEGER *)options->rowsFetchedPtr;
            break;

        case SQL_ATTR_SIMULATE_CURSOR:
            ValuePtr= (SQLUSMALLINT *)options->simulateCursor;
            break;

            /*
              To make iODBC and MS ODBC DM to work, return the following cases
              as success, by just allocating...else
              - iODBC is hanging at the time of stmt allocation
              - MS ODB DM is crashing at the time of stmt allocation
            */
        case SQL_ATTR_APP_ROW_DESC:
            *(SQLPOINTER *)ValuePtr= &stmt->ard;
            *StringLengthPtr= sizeof(SQL_IS_POINTER);
            break;

        case SQL_ATTR_IMP_ROW_DESC:
            *(SQLPOINTER *)ValuePtr= &stmt->ird;
            *StringLengthPtr= sizeof(SQL_IS_POINTER);
            break;

        case SQL_ATTR_APP_PARAM_DESC:
            *(SQLPOINTER *)ValuePtr= &stmt->apd;
            *StringLengthPtr= sizeof(SQL_IS_POINTER);
            break;

        case SQL_ATTR_IMP_PARAM_DESC:
            *(SQLPOINTER *)ValuePtr= &stmt->ipd;
            *StringLengthPtr= sizeof(SQL_IS_POINTER);
            break;

            /*
              3.x driver doesn't support any statement attributes
              at connection level, but to make sure all 2.x apps
              works fine...lets support it..nothing to loose..
            */
        default:
            result= get_constmt_attr(3,hstmt,options,
                                     Attribute,ValuePtr,
                                     StringLengthPtr);
    }

    MYODBCDbgReturnReturn(result);
}


/*
  @type    : ODBC 1.0 API
  @purpose : sets the connection options
*/

SQLRETURN SQL_API SQLSetConnectOption( SQLHDBC      hdbc, 
                                       SQLUSMALLINT fOption,
                                       SQLULEN      vParam )
{
    SQLRETURN result= SQL_SUCCESS;

    MYODBCDbgEnter;
    MYODBCDbgInfo( "hdbc: %p", hdbc );
    MYODBCDbgInfo( "fOption: %s", MYODBCDbgConnectOptionString( fOption ) );
    MYODBCDbgInfo( "fOption: %d", fOption );

    result= set_con_attr(hdbc,fOption,(SQLPOINTER)vParam,SQL_NTS);

    MYODBCDbgReturnReturn(result);
}


/*
  @type    : ODBC 1.0 API
  @purpose : returns the connection options
*/

SQLRETURN SQL_API SQLGetConnectOption(SQLHDBC hdbc,SQLUSMALLINT fOption,
                                      SQLPOINTER vParam)
{
    SQLRETURN result= SQL_SUCCESS;

    MYODBCDbgEnter;
    MYODBCDbgInfo( "hdbc: %p", hdbc );
    MYODBCDbgInfo( "fOption: %s", MYODBCDbgConnectOptionString( fOption ) );
    MYODBCDbgInfo( "fOption: %d", fOption );

    result= get_con_attr(hdbc,fOption,vParam,SQL_NTS,(SQLINTEGER *)NULL);

    MYODBCDbgReturnReturn(result);
}


/*
  @type    : ODBC 1.0 API
  @purpose : sets the statement options
*/

SQLRETURN SQL_API SQLSetStmtOption( SQLHSTMT        hstmt,
                                    SQLUSMALLINT    fOption,
                                    SQLULEN         vParam )
{
    SQLRETURN result= SQL_SUCCESS;

    MYODBCDbgEnter;
    MYODBCDbgInfo( "hstmt: %p", hstmt );
    MYODBCDbgInfo( "fOption: %s", MYODBCDbgStmtOptionString( fOption ) );
    MYODBCDbgInfo( "fOption: %d", fOption );

    result= set_stmt_attr(hstmt,fOption,(SQLPOINTER)vParam,SQL_NTS);

    MYODBCDbgReturnReturn(result);
}


/*
  @type    : ODBC 1.0 API
  @purpose : returns the statement options
*/

SQLRETURN SQL_API SQLGetStmtOption(SQLHSTMT hstmt,SQLUSMALLINT fOption,
                                   SQLPOINTER vParam)
{
    SQLRETURN result= SQL_SUCCESS;

    MYODBCDbgEnter;
    MYODBCDbgInfo( "hstmt: %p", hstmt );
    MYODBCDbgInfo( "fOption: %s", MYODBCDbgStmtOptionString( fOption ) );
    MYODBCDbgInfo( "fOption: %d", fOption );

    result= get_stmt_attr(hstmt,fOption,vParam,SQL_NTS,(SQLINTEGER *)NULL);

    MYODBCDbgReturnReturn(result);
}


/*
  @type    : ODBC 3.0 API
  @purpose : sets the environment attributes
*/

SQLRETURN SQL_API
SQLSetEnvAttr(SQLHENV    henv,
              SQLINTEGER Attribute,
              SQLPOINTER ValuePtr,
              SQLINTEGER StringLength __attribute__((unused)))
{
    MYODBCDbgEnter;
    MYODBCDbgInfo( "henv: %p", henv );
    MYODBCDbgInfo( "Atrr: %s", MYODBCDbgEnvAttrString( Attribute ) );
    MYODBCDbgInfo( "Atrr: %d", Attribute );
    MYODBCDbgInfo( "ValuePtr: 0x%lx", ValuePtr );

    if (((ENV FAR *)henv)->connections)
        MYODBCDbgReturnReturn( set_env_error(henv,MYERR_S1010,NULL,0) );

    switch (Attribute)
    {
        
        case SQL_ATTR_ODBC_VERSION:
            ((ENV FAR *)henv)->odbc_ver= (SQLINTEGER)ValuePtr;
            break;

        case SQL_ATTR_OUTPUT_NTS:
            if (ValuePtr == (SQLPOINTER)SQL_TRUE)
                break;

        default:
            MYODBCDbgReturnReturn(set_env_error(henv,MYERR_S1C00,NULL,0));
    }
    MYODBCDbgReturnReturn(SQL_SUCCESS);
}


/*
  @type    : ODBC 3.0 API
  @purpose : returns the environment attributes
*/

SQLRETURN SQL_API
SQLGetEnvAttr(SQLHENV    henv,
              SQLINTEGER Attribute,
              SQLPOINTER ValuePtr,
              SQLINTEGER BufferLength __attribute__((unused)),
              SQLINTEGER *StringLengthPtr __attribute__((unused)))
{
    MYODBCDbgEnter;
    MYODBCDbgInfo( "henv: %p", henv );
    MYODBCDbgInfo( "Atrr: %s", MYODBCDbgEnvAttrString( Attribute ) );
    MYODBCDbgInfo( "Atrr: %d", Attribute );

    switch ( Attribute )
    {
        case SQL_ATTR_CONNECTION_POOLING:
            *(SQLINTEGER*)ValuePtr = SQL_CP_OFF;
            break;

        case SQL_ATTR_ODBC_VERSION:
            *(SQLINTEGER*)ValuePtr= ((ENV FAR *)henv)->odbc_ver;
            break;

        case SQL_ATTR_OUTPUT_NTS:
            *((SQLINTEGER*)ValuePtr)= SQL_TRUE;
            break;

        default:
            MYODBCDbgReturnReturn(set_env_error(henv,MYERR_S1C00,NULL,0));
    }
    MYODBCDbgReturnReturn(SQL_SUCCESS);
}


/*
  @type    : ODBC 3.0 API
  @purpose : sets the connection attributes
*/

SQLRETURN SQL_API SQLSetConnectAttr(SQLHDBC hdbc,
                                    SQLINTEGER Attribute,
                                    SQLPOINTER ValuePtr,
                                    SQLINTEGER StringLength)
{
    MYODBCDbgEnter;
    MYODBCDbgInfo( "hdbc: %p", hdbc );
    MYODBCDbgInfo( "Attribute: %s", MYODBCDbgConnectAttrString( Attribute ) );
    MYODBCDbgInfo( "Attribute: %d", Attribute );
    MYODBCDbgReturnReturn( set_con_attr(hdbc,Attribute, ValuePtr,StringLength) );
}

/*
  @type    : ODBC 3.0 API
  @purpose : returns the connection attribute values
*/

SQLRETURN SQL_API SQLGetConnectAttr(SQLHDBC hdbc,
                                    SQLINTEGER Attribute,
                                    SQLPOINTER ValuePtr,
                                    SQLINTEGER BufferLength,
                                    SQLINTEGER *StringLengthPtr)
{
    MYODBCDbgEnter;
    MYODBCDbgInfo( "hdbc: %p", hdbc );
    MYODBCDbgInfo( "Attribute: %s", MYODBCDbgConnectAttrString( Attribute ) );
    MYODBCDbgInfo( "Attribute: %d", Attribute );
    MYODBCDbgReturnReturn( get_con_attr( hdbc,Attribute, ValuePtr,BufferLength, StringLengthPtr ) );
}

/*
  @type    : ODBC 3.0 API
  @purpose : sets the statement attributes
*/

SQLRETURN SQL_API SQLSetStmtAttr(SQLHSTMT   hstmt,
                                 SQLINTEGER Attribute,
                                 SQLPOINTER ValuePtr,
                                 SQLINTEGER StringLength)
{
    MYODBCDbgEnter;
    MYODBCDbgInfo( "hstmt: %p", hstmt );
    MYODBCDbgInfo( "Attribute: %s", MYODBCDbgStmtAttrString( Attribute ) );
    MYODBCDbgInfo( "Attribute: %d", Attribute );
    MYODBCDbgReturnReturn( set_stmt_attr( hstmt, Attribute, ValuePtr, StringLength ) );
}

/*
  @type    : ODBC 3.0 API
  @purpose : returns the statement attribute values
*/
SQLRETURN SQL_API SQLGetStmtAttr(SQLHSTMT   hstmt,
                                 SQLINTEGER Attribute,
                                 SQLPOINTER ValuePtr,
                                 SQLINTEGER BufferLength,
                                 SQLINTEGER *StringLengthPtr)
{
    MYODBCDbgEnter;
    MYODBCDbgInfo( "hstmt: %p", hstmt );
    MYODBCDbgInfo( "Attribute: %s", MYODBCDbgStmtAttrString( Attribute ) );
    MYODBCDbgInfo( "Attribute: %d", Attribute );
    MYODBCDbgReturnReturn( get_stmt_attr( hstmt,Attribute, ValuePtr,BufferLength, StringLengthPtr ) );
}
