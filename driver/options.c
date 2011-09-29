/*
  Copyright (c) 2000, 2011, Oracle and/or its affiliates. All rights reserved.

  The MySQL Connector/ODBC is licensed under the terms of the GPLv2
  <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
  MySQL Connectors. There are special exceptions to the terms and
  conditions of the GPLv2 as it is applied to this software, see the
  FLOSS License Exception
  <http://www.mysql.com/about/legal/licensing/foss-exception.html>.
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.
  
  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

/**
  @file  options.c
  @brief Functions for handling handle attributes and options.
*/

#include "driver.h"
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
    switch (Attribute)
    {
        case SQL_ATTR_ASYNC_ENABLE:
            if (ValuePtr == (SQLPOINTER) SQL_ASYNC_ENABLE_ON)
                return set_handle_error(HandleType,Handle,MYERR_01S02,
                                        "Doesn't support asynchronous, changed to default",0);
            break;

        case SQL_ATTR_CURSOR_SENSITIVITY:
            if (ValuePtr != (SQLPOINTER) SQL_UNSPECIFIED)
            {
                return set_handle_error(HandleType,Handle,MYERR_01S02,
                                        "Option value changed to default cursor sensitivity(unspecified)",0);
            }
            break;

        case SQL_ATTR_CURSOR_TYPE:
            if (((STMT FAR*)Handle)->dbc->ds->force_use_of_forward_only_cursors)
            {
                options->cursor_type= SQL_CURSOR_FORWARD_ONLY;
                if (ValuePtr != (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY)
                    return set_handle_error(HandleType,Handle,MYERR_01S02,
                                            "Forcing the use of forward-only cursor)",0);
            }
            else if (((STMT FAR*)Handle)->dbc->ds->dynamic_cursor)
            {
                if (ValuePtr != (SQLPOINTER)SQL_CURSOR_KEYSET_DRIVEN)
                    options->cursor_type= (SQLUINTEGER)(SQLULEN)ValuePtr;

                else
                {
                    options->cursor_type= SQL_CURSOR_STATIC;
                    return set_handle_error(HandleType,Handle,MYERR_01S02,
                                            "Option value changed to default static cursor",0);
                }
            }
            else
            {
                if (ValuePtr == (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY ||
                    ValuePtr == (SQLPOINTER)SQL_CURSOR_STATIC)
                    options->cursor_type= (SQLUINTEGER)(SQLULEN)ValuePtr;

                else
                {
                    options->cursor_type= SQL_CURSOR_STATIC;
                    return set_handle_error(HandleType,Handle,MYERR_01S02,
                                            "Option value changed to default static cursor",0);
                }
            }
            break;

        case SQL_ATTR_MAX_LENGTH:
            options->max_length= (SQLULEN) ValuePtr;
            break;

        case SQL_ATTR_MAX_ROWS:
            options->max_rows= (SQLULEN) ValuePtr;
            break;

        case SQL_ATTR_METADATA_ID:
            if (ValuePtr == (SQLPOINTER) SQL_TRUE)
                return set_handle_error(HandleType,Handle,MYERR_01S02,
                                        "Doesn't support SQL_ATTR_METADATA_ID to true, changed to default",0);
            break;

        case SQL_ATTR_RETRIEVE_DATA:
            break;

        case SQL_ATTR_SIMULATE_CURSOR:
            if (ValuePtr != (SQLPOINTER) SQL_SC_TRY_UNIQUE)
                return set_handle_error(HandleType,Handle,MYERR_01S02,
                                        "Option value changed to default cursor simulation",0);
            break;

        case 1226:/* MS SQL Server Extension */
        case 1227:
        case 1228:
            break;

        case SQL_ATTR_FETCH_BOOKMARK_PTR:
        case SQL_ATTR_USE_BOOKMARKS:
            return set_handle_error(HandleType,Handle,MYERR_S1C00,NULL,0);

        case SQL_ATTR_QUERY_TIMEOUT:
        case SQL_ATTR_KEYSET_SIZE:
        case SQL_ATTR_CONCURRENCY:
        case SQL_ATTR_NOSCAN:
        default:
            /* ignored */
            break;
    }
    return SQL_SUCCESS;
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
            *((SQLULEN *) ValuePtr)= options->max_length;
            break;

        case SQL_ATTR_MAX_ROWS:
            *((SQLULEN *) ValuePtr)= options->max_rows;
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

        case SQL_ATTR_SIMULATE_CURSOR:
            *((SQLUINTEGER *) ValuePtr)= SQL_SC_TRY_UNIQUE;
            break;

        case SQL_ATTR_CONCURRENCY:
            *((SQLUINTEGER *) ValuePtr)= SQL_CONCUR_READ_ONLY;
            break;

        case SQL_KEYSET_SIZE:
            *((SQLUINTEGER *) ValuePtr)= 0L;
            break;

        case SQL_NOSCAN:
            *((SQLUINTEGER *) ValuePtr)= SQL_NOSCAN_ON;
            break;

        case SQL_ATTR_FETCH_BOOKMARK_PTR:
        case SQL_ATTR_USE_BOOKMARKS:
            return set_handle_error(HandleType,Handle,MYERR_S1C00,NULL,0);

        case 1226:/* MS SQL Server Extension */
        case 1227:
        case 1228:
        default:
            /* ignored */
            break;
    }
    return SQL_SUCCESS;
}


/*
  @type    : myodbc3 internal
  @purpose : sets the connection attributes
*/

SQLRETURN SQL_API
MySQLSetConnectAttr(SQLHDBC hdbc, SQLINTEGER Attribute,
                    SQLPOINTER ValuePtr, SQLINTEGER StringLengthPtr)
{
    DBC FAR *dbc= (DBC FAR*) hdbc;

    switch (Attribute)
    {
        case SQL_ATTR_ACCESS_MODE:
            break;

        case SQL_ATTR_AUTOCOMMIT:
            if (ValuePtr != (SQLPOINTER) SQL_AUTOCOMMIT_ON)
            {
                if (!is_connected(dbc))
                {
                    dbc->commit_flag= CHECK_AUTOCOMMIT_OFF;
                    return SQL_SUCCESS;
                }
                if (!(trans_supported(dbc)) || dbc->ds->disable_transactions)
                    return set_conn_error(dbc,MYERR_S1C00,
                                          "Transactions are not enabled", 4000);

                if (autocommit_on(dbc))
                    return odbc_stmt(dbc,"SET AUTOCOMMIT=0");
            }
            else if (!is_connected(dbc))
            {
                dbc->commit_flag= CHECK_AUTOCOMMIT_ON;
                return SQL_SUCCESS;
            }
            else if (trans_supported(dbc) && !(autocommit_on(dbc)))
                return odbc_stmt(dbc,"SET AUTOCOMMIT=1");
            break;

        case SQL_ATTR_LOGIN_TIMEOUT:
            {
                /* we can't change timeout values in post connect state */
               if (is_connected(dbc)) {
                  return set_conn_error(dbc, MYERR_S1011, NULL, 0);
               }
               else
               {
                  dbc->login_timeout= (SQLUINTEGER)(SQLULEN)ValuePtr;
                  return SQL_SUCCESS;
               }
            }
            break;

        case SQL_ATTR_CONNECTION_TIMEOUT:
            {
              /*
                We don't do anything with this, but we pretend that we did
                to be consistent with Microsoft SQL Server.
              */
              return SQL_SUCCESS;
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
                    return set_conn_error(hdbc,MYERR_S1009,NULL, 0);

                pthread_mutex_lock(&dbc->lock);
                if (is_connected(dbc))
                {
                    if (mysql_select_db(&dbc->mysql,(char*) db))
                    {
                        set_conn_error(dbc,MYERR_S1000,mysql_error(&dbc->mysql),mysql_errno(&dbc->mysql));
                        pthread_mutex_unlock(&dbc->lock);
                        return SQL_ERROR;
                    }
                }
                x_free(dbc->database);
                dbc->database= my_strdup(db,MYF(MY_WME));
                pthread_mutex_unlock(&dbc->lock);
            }
            break;


        case SQL_ATTR_ODBC_CURSORS:
            if (dbc->ds->force_use_of_forward_only_cursors &&
                ValuePtr != (SQLPOINTER) SQL_CUR_USE_ODBC)
                return set_conn_error(hdbc,MYERR_01S02,
                                      "Forcing the Driver Manager to use ODBC cursor library",0);
            break;

        case SQL_OPT_TRACE:
        case SQL_OPT_TRACEFILE:
        case SQL_QUIET_MODE:
        case SQL_TRANSLATE_DLL:
        case SQL_TRANSLATE_OPTION:
            {
                char buff[100];
                sprintf(buff,"Suppose to set this attribute '%d' through driver manager, not by the driver",(int) Attribute);
                return set_conn_error(hdbc,MYERR_01S02,buff,0);
            }

        case SQL_ATTR_PACKET_SIZE:
            break;

        case SQL_ATTR_TXN_ISOLATION:
            if (!is_connected(dbc))  /* no connection yet */
            {
                dbc->txn_isolation= (SQLINTEGER)(SQLLEN)ValuePtr;
                return SQL_SUCCESS;
            }
            if (trans_supported(dbc))
            {
                char buff[80];
                const char *level= NULL;

                if ((SQLLEN)ValuePtr == SQL_TXN_SERIALIZABLE)
                    level="SERIALIZABLE";
                else if ((SQLLEN)ValuePtr == SQL_TXN_REPEATABLE_READ)
                    level="REPEATABLE READ";
                else if ((SQLLEN)ValuePtr == SQL_TXN_READ_COMMITTED)
                    level="READ COMMITTED";
                else if ((SQLLEN)ValuePtr == SQL_TXN_READ_UNCOMMITTED)
                    level="READ UNCOMMITTED";

                if (level)
                {
                  SQLRETURN rc;
                  sprintf(buff,"SET SESSION TRANSACTION ISOLATION LEVEL %s",
                          level);
                  if (SQL_SUCCEEDED(rc = odbc_stmt(dbc,buff)))
                      dbc->txn_isolation= (SQLINTEGER)ValuePtr;
                  return rc;
                }
                else
                {
                  return set_dbc_error(dbc, "HY024",
                                       "Invalid attribute value", 0);
                }
            }
            break;

        case SQL_ATTR_ENLIST_IN_DTC:
            return set_dbc_error(dbc, "HYC00",
                                 "Optional feature not supported", 0);

            /*
              3.x driver doesn't support any statement attributes
              at connection level, but to make sure all 2.x apps
              works fine...lets support it..nothing to lose..
            */
        default:
            return set_constmt_attr(2, dbc, &dbc->stmt_options, Attribute,
                                    ValuePtr);
    }
    return SQL_SUCCESS;
}


/**
 Retrieve connection attribute values.

 @param[in]  hdbc
 @param[in]  attrib
 @param[out] char_attr
 @param[out] num_attr
*/
SQLRETURN SQL_API
MySQLGetConnectAttr(SQLHDBC hdbc, SQLINTEGER attrib, SQLCHAR **char_attr,
                    SQLPOINTER num_attr)
{
  DBC *dbc= (DBC *)hdbc;
  SQLRETURN result= SQL_SUCCESS;

  switch (attrib) {
  case SQL_ATTR_ACCESS_MODE:
    *((SQLUINTEGER *)num_attr)= SQL_MODE_READ_WRITE;
    break;

  case SQL_ATTR_AUTO_IPD:
    *((SQLUINTEGER *)num_attr)= SQL_FALSE;
    break;

  case SQL_ATTR_AUTOCOMMIT:
    *((SQLUINTEGER *)num_attr)= (autocommit_on(dbc) ||
                                 (!(trans_supported(dbc)) ?
                                  SQL_AUTOCOMMIT_ON :
                                  SQL_AUTOCOMMIT_OFF));
    break;

  case SQL_ATTR_CONNECTION_DEAD:
    if (mysql_ping(&dbc->mysql) &&
        (mysql_errno(&dbc->mysql) == CR_SERVER_LOST ||
         mysql_errno(&dbc->mysql) == CR_SERVER_GONE_ERROR))
      *((SQLUINTEGER *)num_attr)= SQL_CD_TRUE;
    else
      *((SQLUINTEGER *)num_attr)= SQL_CD_FALSE;
    break;

  case SQL_ATTR_CONNECTION_TIMEOUT:
    /* We don't support this option, so it is always 0. */
    *((SQLUINTEGER *)num_attr)= 0;
    break;

  case SQL_ATTR_CURRENT_CATALOG:
    if (is_connected(dbc) && reget_current_catalog(dbc))
    {
      return set_handle_error(SQL_HANDLE_DBC, hdbc, MYERR_S1000,
                              "Unable to get current catalog", 0);
    }
    else
    {
      *char_attr= (SQLCHAR *)(dbc->database ? dbc->database : "null");
    }
    break;

  case SQL_ATTR_LOGIN_TIMEOUT:
    *((SQLUINTEGER *)num_attr)= dbc->login_timeout;
    break;

  case SQL_ATTR_ODBC_CURSORS:
    if (dbc->ds->force_use_of_forward_only_cursors)
      *((SQLUINTEGER *)num_attr)= SQL_CUR_USE_ODBC;
    else
      *((SQLUINTEGER *)num_attr)= SQL_CUR_USE_IF_NEEDED;
    break;

  case SQL_ATTR_PACKET_SIZE:
    *((SQLUINTEGER *)num_attr)= dbc->mysql.net.max_packet;
    break;

  case SQL_ATTR_TXN_ISOLATION:
    /*
      If we don't know the isolation level already, we need to ask the
      server.
    */
    if (!dbc->txn_isolation)
    {
      /*
        Unless we're not connected yet, then we just assume it will
        be REPEATABLE READ, which is the server default.
      */
      if (!is_connected(dbc))
      {
        *((SQLINTEGER *)num_attr)= SQL_TRANSACTION_REPEATABLE_READ;
        break;
      }

      if (odbc_stmt(dbc, "SELECT @@tx_isolation"))
      {
        return set_handle_error(SQL_HANDLE_DBC, hdbc, MYERR_S1000,
                                "Failed to get isolation level", 0);
      }
      else
      {
        MYSQL_RES *res;
        MYSQL_ROW  row;

        if ((res= mysql_store_result(&dbc->mysql)) &&
            (row= mysql_fetch_row(res)))
        {
          if (strncmp(row[0], "READ-UNCOMMITTED", 16) == 0) {
            dbc->txn_isolation= SQL_TRANSACTION_READ_UNCOMMITTED;
          }
          else if (strncmp(row[0], "READ-COMMITTED", 14) == 0) {
            dbc->txn_isolation= SQL_TRANSACTION_READ_COMMITTED;
          }
          else if (strncmp(row[0], "REPEATABLE-READ", 15) == 0) {
            dbc->txn_isolation= SQL_TRANSACTION_REPEATABLE_READ;
          }
          else if (strncmp(row[0], "SERIALIZABLE", 12) == 0) {
            dbc->txn_isolation= SQL_TRANSACTION_SERIALIZABLE;
          }
        }
        mysql_free_result(res);
      }
    }

    *((SQLINTEGER *)num_attr)= dbc->txn_isolation;
    break;

  default:
    return set_handle_error(SQL_HANDLE_DBC, hdbc, MYERR_S1092, NULL, 0);
  }

  return result;
}


/*
  @type    : myodbc3 internal
  @purpose : sets the statement attributes
*/

SQLRETURN SQL_API
MySQLSetStmtAttr(SQLHSTMT hstmt, SQLINTEGER Attribute, SQLPOINTER ValuePtr,
                 SQLINTEGER StringLengthPtr __attribute__((unused)))
{
    STMT *stmt= (STMT *)hstmt;
    SQLRETURN result= SQL_SUCCESS;
    STMT_OPTIONS *options= &stmt->stmt_options;

    CLEAR_STMT_ERROR(stmt);

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

        case SQL_ATTR_APP_PARAM_DESC:
        case SQL_ATTR_APP_ROW_DESC:
            {
              DESC *desc= (DESC *) ValuePtr;
              DESC **dest= NULL;
              desc_desc_type desc_type;

              /* reset to implicit if null */
              if (desc == SQL_NULL_HANDLE)
              {
                if (Attribute == SQL_ATTR_APP_PARAM_DESC)
                  stmt->apd= stmt->imp_apd;
                else if (Attribute == SQL_ATTR_APP_ROW_DESC)
                  stmt->ard= stmt->imp_ard;
                break;
              }

              if (desc->alloc_type == SQL_DESC_ALLOC_AUTO &&
                  desc->stmt != stmt)
                return set_error(hstmt,MYERR_S1017,
                                 "Invalid use of an automatically allocated "
                                 "descriptor handle",0);

              if (desc->alloc_type == SQL_DESC_ALLOC_USER &&
                  stmt->dbc != desc->exp.dbc)
                return set_error(hstmt,MYERR_S1024,
                                 "Invalid attribute value",0);

              if (Attribute == SQL_ATTR_APP_PARAM_DESC)
              {
                dest= &stmt->apd;
                desc_type= DESC_PARAM;
              }
              else if (Attribute == SQL_ATTR_APP_ROW_DESC)
              {
                dest= &stmt->ard;
                desc_type= DESC_ROW;
              }

              if (desc->desc_type != DESC_UNKNOWN &&
                  desc->desc_type != desc_type)
              {
                return set_error(hstmt,MYERR_S1024,
                                 "Descriptor type mismatch",0);
              }

              assert(desc);
              assert(dest);

              if (desc->alloc_type == SQL_DESC_ALLOC_AUTO &&
                  (*dest)->alloc_type == SQL_DESC_ALLOC_USER)
                /*
                  If we're setting back the original implicit
                  descriptor, we must disassociate this statement
                  from the explicit descriptor.
                */
                desc_remove_stmt(*dest, stmt);
              else if (desc->alloc_type == SQL_DESC_ALLOC_USER)
              {
                /* otherwise, associate this statement with the desc */
                LIST *e= (LIST *) my_malloc(sizeof(LIST), MYF(0));
                e->data= stmt;
                desc->exp.stmts= list_add(desc->exp.stmts, e);
              }

              desc->desc_type= desc_type;
              *dest= desc;
            }
            break;

        case SQL_ATTR_AUTO_IPD:
        case SQL_ATTR_ENABLE_AUTO_IPD:
            if (ValuePtr != (SQLPOINTER)SQL_FALSE)
                return set_error(hstmt,MYERR_S1C00,
                                 "Optional feature not implemented",0);
            break;

        case SQL_ATTR_IMP_PARAM_DESC:
        case SQL_ATTR_IMP_ROW_DESC:
            return set_error(hstmt,MYERR_S1024,
                             "Invalid attribute/option identifier",0);

        case SQL_ATTR_PARAM_BIND_OFFSET_PTR:
            return stmt_SQLSetDescField(stmt, stmt->apd, 0,
                                        SQL_DESC_BIND_OFFSET_PTR,
                                        ValuePtr, SQL_IS_POINTER);

        case SQL_ATTR_PARAM_BIND_TYPE:
            return stmt_SQLSetDescField(stmt, stmt->apd, 0,
                                        SQL_DESC_BIND_TYPE,
                                        ValuePtr, SQL_IS_INTEGER);

        case SQL_ATTR_PARAM_OPERATION_PTR: /* need to support this ....*/
            return stmt_SQLSetDescField(stmt, stmt->apd, 0,
                                        SQL_DESC_ARRAY_STATUS_PTR,
                                        ValuePtr, SQL_IS_POINTER);

        case SQL_ATTR_PARAM_STATUS_PTR: /* need to support this ....*/
            return stmt_SQLSetDescField(stmt, stmt->ipd, 0,
                                        SQL_DESC_ARRAY_STATUS_PTR,
                                        ValuePtr, SQL_IS_POINTER);

        case SQL_ATTR_PARAMS_PROCESSED_PTR: /* need to support this ....*/
            return stmt_SQLSetDescField(stmt, stmt->ipd, 0,
                                        SQL_DESC_ROWS_PROCESSED_PTR,
                                        ValuePtr, SQL_IS_POINTER);

        case SQL_ATTR_PARAMSET_SIZE:
            return stmt_SQLSetDescField(stmt, stmt->apd, 0,
                                        SQL_DESC_ARRAY_SIZE,
                                        ValuePtr, SQL_IS_ULEN);


        case SQL_ATTR_ROW_ARRAY_SIZE:
        case SQL_ROWSET_SIZE:
            return stmt_SQLSetDescField(stmt, stmt->ard, 0,
                                        SQL_DESC_ARRAY_SIZE,
                                        ValuePtr, SQL_IS_ULEN);

        case SQL_ATTR_ROW_BIND_OFFSET_PTR:
            return stmt_SQLSetDescField(stmt, stmt->ard, 0,
                                        SQL_DESC_BIND_OFFSET_PTR,
                                        ValuePtr, SQL_IS_POINTER);

        case SQL_ATTR_ROW_BIND_TYPE:
            return stmt_SQLSetDescField(stmt, stmt->ard, 0,
                                        SQL_DESC_BIND_TYPE,
                                        ValuePtr, SQL_IS_INTEGER);

        case SQL_ATTR_ROW_NUMBER:
            return set_error(hstmt,MYERR_S1000,
                             "Trying to set read-only attribute",0);

        case SQL_ATTR_ROW_OPERATION_PTR:
            return stmt_SQLSetDescField(stmt, stmt->ard, 0,
                                        SQL_DESC_ARRAY_STATUS_PTR,
                                        ValuePtr, SQL_IS_POINTER);

        case SQL_ATTR_ROW_STATUS_PTR:
            return stmt_SQLSetDescField(stmt, stmt->ird, 0,
                                        SQL_DESC_ARRAY_STATUS_PTR,
                                        ValuePtr, SQL_IS_POINTER);

        case SQL_ATTR_ROWS_FETCHED_PTR:
            return stmt_SQLSetDescField(stmt, stmt->ird, 0,
                                        SQL_DESC_ROWS_PROCESSED_PTR,
                                        ValuePtr, SQL_IS_POINTER);

        case SQL_ATTR_SIMULATE_CURSOR:
            options->simulateCursor= (SQLUINTEGER)(SQLULEN)ValuePtr;
            break;

            /*
              3.x driver doesn't support any statement attributes
              at connection level, but to make sure all 2.x apps
              works fine...lets support it..nothing to lose..
            */
        default:
            result= set_constmt_attr(3,hstmt,options,
                                     Attribute,ValuePtr);
    }
    return result;
}


/*
  @type    : myodbc3 internal
  @purpose : returns the statement attribute values
*/

SQLRETURN SQL_API
MySQLGetStmtAttr(SQLHSTMT hstmt, SQLINTEGER Attribute, SQLPOINTER ValuePtr,
                 SQLINTEGER BufferLength __attribute__((unused)),
                 SQLINTEGER *StringLengthPtr)
{
    SQLRETURN result= SQL_SUCCESS;
    STMT FAR *stmt= (STMT FAR*) hstmt;
    STMT_OPTIONS *options= &stmt->stmt_options;
    SQLINTEGER vparam= 0;
    SQLINTEGER len;

    if (!ValuePtr)
        ValuePtr= &vparam;

    if (!StringLengthPtr)
        StringLengthPtr= &len;

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

        case SQL_ATTR_PARAM_BIND_OFFSET_PTR:
            *(SQLPOINTER *)ValuePtr= stmt->apd->bind_offset_ptr;
            break;

        case SQL_ATTR_PARAM_BIND_TYPE:
            *(SQLINTEGER *)ValuePtr= stmt->apd->bind_type;
            break;

        case SQL_ATTR_PARAM_OPERATION_PTR: /* need to support this ....*/
            *(SQLPOINTER *)ValuePtr= stmt->apd->array_status_ptr;
            break;

        case SQL_ATTR_PARAM_STATUS_PTR: /* need to support this ....*/
            *(SQLPOINTER *)ValuePtr= stmt->ipd->array_status_ptr;
            break;

        case SQL_ATTR_PARAMS_PROCESSED_PTR: /* need to support this ....*/
            *(SQLPOINTER *)ValuePtr= stmt->ipd->rows_processed_ptr;
            break;

        case SQL_ATTR_PARAMSET_SIZE:
            *(SQLUINTEGER *)ValuePtr= stmt->apd->array_size;
            break;

        case SQL_ATTR_ROW_ARRAY_SIZE:
        case SQL_ROWSET_SIZE:
            *(SQLUINTEGER *)ValuePtr= stmt->ard->array_size;
            break;

        case SQL_ATTR_ROW_BIND_OFFSET_PTR:
            *((SQLPOINTER *) ValuePtr)= stmt->ard->bind_offset_ptr;
            break;

        case SQL_ATTR_ROW_BIND_TYPE:
            *((SQLINTEGER *) ValuePtr)= stmt->ard->bind_type;
            break;

        case SQL_ATTR_ROW_NUMBER:
            *(SQLUINTEGER *)ValuePtr= stmt->current_row+1;
            break;

        case SQL_ATTR_ROW_OPERATION_PTR: /* need to support this ....*/
            *(SQLPOINTER *)ValuePtr= stmt->ard->array_status_ptr;
            break;

        case SQL_ATTR_ROW_STATUS_PTR:
            *(SQLPOINTER *)ValuePtr= stmt->ird->array_status_ptr;
            break;

        case SQL_ATTR_ROWS_FETCHED_PTR:
            *(SQLPOINTER *)ValuePtr= stmt->ird->rows_processed_ptr;
            break;

        case SQL_ATTR_SIMULATE_CURSOR:
            *(SQLUINTEGER *)ValuePtr= options->simulateCursor;
            break;

        case SQL_ATTR_APP_ROW_DESC:
            *(SQLPOINTER *)ValuePtr= stmt->ard;
            *StringLengthPtr= sizeof(SQLPOINTER);
            break;

        case SQL_ATTR_IMP_ROW_DESC:
            *(SQLPOINTER *)ValuePtr= stmt->ird;
            *StringLengthPtr= sizeof(SQLPOINTER);
            break;

        case SQL_ATTR_APP_PARAM_DESC:
            *(SQLPOINTER *)ValuePtr= stmt->apd;
            *StringLengthPtr= sizeof(SQLPOINTER);
            break;

        case SQL_ATTR_IMP_PARAM_DESC:
            *(SQLPOINTER *)ValuePtr= stmt->ipd;
            *StringLengthPtr= sizeof(SQLPOINTER);
            break;

            /*
              3.x driver doesn't support any statement attributes
              at connection level, but to make sure all 2.x apps
              works fine...lets support it..nothing to lose..
            */
        default:
            result= get_constmt_attr(3,hstmt,options,
                                     Attribute,ValuePtr,
                                     StringLengthPtr);
    }

    return result;
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

    if (((ENV FAR *)henv)->connections)
        return set_env_error(henv, MYERR_S1010, NULL, 0);

    switch (Attribute)
    {
        case SQL_ATTR_ODBC_VERSION:
            ((ENV FAR *)henv)->odbc_ver= (SQLINTEGER)(SQLLEN)ValuePtr;
            break;

        case SQL_ATTR_OUTPUT_NTS:
            if (ValuePtr == (SQLPOINTER)SQL_TRUE)
                break;

        default:
            return set_env_error(henv,MYERR_S1C00,NULL,0);
    }
    return SQL_SUCCESS;
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
            return set_env_error(henv,MYERR_S1C00,NULL,0);
    }
    return SQL_SUCCESS;
}


SQLRETURN SQL_API
SQLGetStmtOption(SQLHSTMT hstmt,SQLUSMALLINT option, SQLPOINTER param)
{
  return MySQLGetStmtAttr(hstmt, option, param, SQL_NTS, (SQLINTEGER *)NULL);
}


SQLRETURN SQL_API
SQLSetStmtOption(SQLHSTMT hstmt, SQLUSMALLINT option, SQLULEN param)
{
  return MySQLSetStmtAttr(hstmt, option, (SQLPOINTER)param, SQL_NTS);
}
