/*
  Copyright (c) 2007, 2012, Oracle and/or its affiliates. All rights reserved.

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
  @file  ansi.c
  @brief Entry points for ANSI versions of ODBC functions
*/

#include "driver.h"


#define NOT_IMPLEMENTED \
  return SQL_ERROR


SQLRETURN SQL_API
SQLColAttributeImpl(SQLHSTMT hstmt, SQLUSMALLINT column,
                    SQLUSMALLINT field, SQLPOINTER char_attr,
                    SQLSMALLINT char_attr_max, SQLSMALLINT *char_attr_len,
                    SQLLEN *num_attr);
SQLRETURN SQL_API
SQLGetConnectAttrImpl(SQLHDBC hdbc, SQLINTEGER attribute, SQLPOINTER value,
                      SQLINTEGER value_max, SQLINTEGER *value_len);
SQLRETURN SQL_API
SQLGetDiagRecImpl(SQLSMALLINT handle_type, SQLHANDLE handle,
                  SQLSMALLINT record, SQLCHAR *sqlstate,
                  SQLINTEGER *native_error, SQLCHAR *message,
                  SQLSMALLINT message_max, SQLSMALLINT *message_len);
SQLRETURN SQL_API
SQLPrepareImpl(SQLHSTMT hstmt, SQLCHAR *str, SQLINTEGER str_len);

SQLRETURN SQL_API
SQLSetConnectAttrImpl(SQLHDBC hdbc, SQLINTEGER attribute,
                      SQLPOINTER value, SQLINTEGER value_len);


SQLRETURN SQL_API
SQLColAttribute(SQLHSTMT hstmt, SQLUSMALLINT column,
                SQLUSMALLINT field, SQLPOINTER char_attr,
                SQLSMALLINT char_attr_max, SQLSMALLINT *char_attr_len,
#ifdef USE_SQLCOLATTRIBUTE_SQLLEN_PTR
                SQLLEN *num_attr
#else
                SQLPOINTER num_attr
#endif
               )
{
  return SQLColAttributeImpl(hstmt, column, field, char_attr, char_attr_max,
                             char_attr_len, num_attr);
}


SQLRETURN SQL_API
SQLColAttributeImpl(SQLHSTMT hstmt, SQLUSMALLINT column,
                    SQLUSMALLINT field, SQLPOINTER char_attr,
                    SQLSMALLINT char_attr_max, SQLSMALLINT *char_attr_len,
                    SQLLEN *num_attr)
{
  STMT *stmt= (STMT *)hstmt;
  SQLCHAR *value= NULL;
  SQLINTEGER len= SQL_NTS;
  uint errors;
  SQLRETURN rc= MySQLColAttribute(hstmt, column, field, &value, num_attr);

  if (value)
  {
    my_bool free_value= FALSE;
    SQLCHAR *old_value= value;
    if (stmt->dbc->ansi_charset_info->number !=
        stmt->dbc->cxn_charset_info->number)
    {
      value= sqlchar_as_sqlchar(stmt->dbc->ansi_charset_info,
                                stmt->dbc->cxn_charset_info,
                                value, &len, &errors);
      if (free_value)
        x_free(old_value);
      free_value= TRUE;
    }
    else
      len= strlen((char *)value);

    /* We set the error only when the result is intented to be returned */
    if ((char_attr || num_attr) && len > char_attr_max - 1)
      rc= set_error(stmt, MYERR_01004, NULL, 0);

    if (char_attr && char_attr_max > 1)
      strmake((char *)char_attr, (char *)value, char_attr_max - 1);

    if (char_attr_len)
      *char_attr_len= (SQLSMALLINT)len;

    if (free_value)
      x_free(value);
  }

  return rc;
}


SQLRETURN SQL_API
SQLColAttributes(SQLHSTMT hstmt, SQLUSMALLINT column, SQLUSMALLINT field,
                 SQLPOINTER char_attr, SQLSMALLINT char_attr_max,
                 SQLSMALLINT *char_attr_len, SQLLEN *num_attr)
{
  return SQLColAttributeImpl(hstmt, column, field, char_attr, char_attr_max,
                             char_attr_len, num_attr);
}


SQLRETURN SQL_API
SQLColumnPrivileges(SQLHSTMT hstmt,
                    SQLCHAR *catalog, SQLSMALLINT catalog_len,
                    SQLCHAR *schema, SQLSMALLINT schema_len,
                    SQLCHAR *table, SQLSMALLINT table_len,
                    SQLCHAR *column, SQLSMALLINT column_len)
{
  SQLRETURN rc;
  DBC *dbc= ((STMT *)hstmt)->dbc;

  if (dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
  {
    SQLINTEGER len= SQL_NTS;
    uint errors= 0;

    if (catalog)
    {
      catalog= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                  catalog, &len, &errors);
      catalog_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (schema)
    {
      schema= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                 schema, &len, &errors);
      schema_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (table)
    {
      table= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                table, &len, &errors);
      table_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (column)
    {
      column= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                 column, &len, &errors);
      column_len= (SQLSMALLINT)len;
    }
  }

  rc= MySQLColumnPrivileges(hstmt, catalog, catalog_len, schema, schema_len,
                            table, table_len, column, column_len);

  if (dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
  {
    x_free(catalog);
    x_free(schema);
    x_free(table);
    x_free(column);
  }

  return rc;
}


SQLRETURN SQL_API
SQLColumns(SQLHSTMT hstmt, SQLCHAR *catalog, SQLSMALLINT catalog_len,
           SQLCHAR *schema, SQLSMALLINT schema_len,
           SQLCHAR *table, SQLSMALLINT table_len,
           SQLCHAR *column, SQLSMALLINT column_len)
{
  SQLRETURN rc;
  DBC *dbc= ((STMT *)hstmt)->dbc;

  if (dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
  {
    SQLINTEGER len= SQL_NTS;
    uint errors= 0;

    if (catalog)
    {
      catalog= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                  catalog, &len, &errors);
      catalog_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (schema)
    {
      schema= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                 schema, &len, &errors);
      schema_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (table)
    {
      table= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                table, &len, &errors);
      table_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (column)
    {
      column= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                 column, &len, &errors);
      column_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }
  }

  rc= MySQLColumns(hstmt, catalog, catalog_len, schema, schema_len,
                   table, table_len, column, column_len);

  if (dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
  {
    x_free(catalog);
    x_free(schema);
    x_free(table);
    x_free(column);
  }

  return rc;
}


SQLRETURN SQL_API
SQLConnect(SQLHDBC hdbc, SQLCHAR *dsn, SQLSMALLINT dsn_len_in,
           SQLCHAR *user, SQLSMALLINT user_len_in,
           SQLCHAR *auth, SQLSMALLINT auth_len_in)
{
  uint errors;
  SQLRETURN rc;
  SQLINTEGER dsn_len= dsn_len_in, user_len= user_len_in,
             auth_len= auth_len_in;

  SQLWCHAR *dsnw=  sqlchar_as_sqlwchar(default_charset_info,
                                       dsn, &dsn_len, &errors);
  SQLWCHAR *userw= sqlchar_as_sqlwchar(default_charset_info,
                                       user, &user_len, &errors);
  SQLWCHAR *authw= sqlchar_as_sqlwchar(default_charset_info,
                                       auth, &auth_len, &errors);

  rc= MySQLConnect(hdbc, dsnw, dsn_len_in, userw, user_len_in,
                   authw, auth_len_in);

  x_free(dsnw);
  x_free(userw);
  x_free(authw);

  return rc;
}


SQLRETURN SQL_API
SQLDescribeCol(SQLHSTMT hstmt, SQLUSMALLINT column,
               SQLCHAR *name, SQLSMALLINT name_max, SQLSMALLINT *name_len,
               SQLSMALLINT *type, SQLULEN *size, SQLSMALLINT *scale,
               SQLSMALLINT *nullable)
{
  STMT *stmt= (STMT *)hstmt;
  SQLCHAR *value= NULL;
  SQLINTEGER len= SQL_NTS;
  SQLSMALLINT free_value;
  uint errors;

  SQLRETURN rc= MySQLDescribeCol(hstmt, column, &value, &free_value, type,
                                 size, scale, nullable);

  if (free_value == -1)
  {
    set_mem_error(&stmt->dbc->mysql);
    return handle_connection_error(stmt);
  }

  if (value)
  {
    SQLCHAR *old_value= value;
    if (stmt->dbc->ansi_charset_info->number !=
        stmt->dbc->cxn_charset_info->number)
    {
      value= sqlchar_as_sqlchar(stmt->dbc->cxn_charset_info,
                                stmt->dbc->ansi_charset_info,
                                value, &len, &errors);
      if (free_value)
        x_free(old_value);
      free_value= TRUE;
    }
    else
      len= strlen((char *)value);

    /* We set the error only when the result is intented to be returned */
    if (name && len > name_max - 1)
      rc= set_error(stmt, MYERR_01004, NULL, 0);

    if (name && name_max > 1)
      strmake((char *)name, (char *)value, name_max - 1);

    if (name_len)
      *name_len= (SQLSMALLINT)len;

    if (free_value)
      x_free(value);
  }

  return rc;
}


SQLRETURN SQL_API
SQLDriverConnect(SQLHDBC hdbc, SQLHWND hwnd, SQLCHAR *in, SQLSMALLINT in_len,
                 SQLCHAR *out, SQLSMALLINT out_max, SQLSMALLINT *out_len,
                 SQLUSMALLINT completion)
{
  SQLRETURN rc;
  uint errors;
  SQLWCHAR *outw= NULL;
  SQLINTEGER inw_len;
  SQLWCHAR *inw;
  SQLSMALLINT outw_max, dummy_out;

  if (in_len == SQL_NTS)
    in_len= strlen((char *)in);
  if (!out_len)
    out_len= &dummy_out;

  inw_len= in_len;
  inw= sqlchar_as_sqlwchar(utf8_charset_info, in, &inw_len, &errors);

  outw_max= (sizeof(SQLWCHAR) * out_max) / MAX_BYTES_PER_UTF8_CP;

  if (outw_max)
  {
    outw= (SQLWCHAR *)my_malloc(sizeof(SQLWCHAR) * out_max, MYF(0));
    if (!outw)
    {
      rc= set_dbc_error((DBC *)hdbc, "HY001", NULL, 0);
      goto error;
    }
  }

  rc= MySQLDriverConnect(hdbc, hwnd, inw, in_len,
                         outw, out_max, out_len, completion);

#ifdef WIN32
  /*
    Microsoft ADO/VB? specifies in and out to be the same, but then gives a
    nonsense value for out_max. Just don't do anything in this case.

    @todo verify that this actually happens.
  */
  if (rc == SQL_SUCCESS && out && in != out)
#else
  if (rc == SQL_SUCCESS && out)
#endif
  {
    uint errors;
    /* Now we have to convert SQLWCHAR back into a SQLCHAR. */
    *out_len= (SQLSMALLINT)sqlwchar_as_sqlchar_buf(default_charset_info, out, out_max,
                                      outw, *out_len, &errors);
    /* TODO what to do with errors? */

    if (*out_len > out_max - 1)
      rc= set_dbc_error((DBC *)hdbc, "01004", NULL, 0);
  }

error:
  x_free(outw);
  x_free(inw);

  return rc;
}


SQLRETURN SQL_API
SQLError(SQLHENV henv, SQLHDBC hdbc, SQLHSTMT hstmt, SQLCHAR *sqlstate,
         SQLINTEGER *native_error, SQLCHAR *message, SQLSMALLINT message_max,
         SQLSMALLINT *message_len)
{
  SQLRETURN rc= SQL_INVALID_HANDLE;

  if (hstmt)
  {
    rc= SQLGetDiagRecImpl(SQL_HANDLE_STMT, hstmt, NEXT_STMT_ERROR(hstmt),
                          sqlstate, native_error, message, message_max,
                          message_len);
  }
  else if (hdbc)
  {
    rc= SQLGetDiagRecImpl(SQL_HANDLE_DBC, hdbc, NEXT_DBC_ERROR(hdbc),
                          sqlstate, native_error, message, message_max,
                          message_len);
  }
  else if (henv)
  {
    rc= SQLGetDiagRecImpl(SQL_HANDLE_ENV, henv, NEXT_ENV_ERROR(henv),
                          sqlstate, native_error, message, message_max,
                          message_len);
  }

  return rc;
}


SQLRETURN SQL_API
SQLExecDirect(SQLHSTMT hstmt, SQLCHAR *str, SQLINTEGER str_len)
{
  int error;

  if ((error= SQLPrepareImpl(hstmt, str, str_len)))
    return error;
  error= my_SQLExecute((STMT *)hstmt);

  return error;
}


SQLRETURN SQL_API
SQLForeignKeys(SQLHSTMT hstmt,
               SQLCHAR *pk_catalog, SQLSMALLINT pk_catalog_len,
               SQLCHAR *pk_schema, SQLSMALLINT pk_schema_len,
               SQLCHAR *pk_table, SQLSMALLINT pk_table_len,
               SQLCHAR *fk_catalog, SQLSMALLINT fk_catalog_len,
               SQLCHAR *fk_schema, SQLSMALLINT fk_schema_len,
               SQLCHAR *fk_table, SQLSMALLINT fk_table_len)
{
  SQLRETURN rc;
  DBC *dbc= ((STMT *)hstmt)->dbc;

  if (dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
  {
    SQLINTEGER len= SQL_NTS;
    uint errors= 0;

    if (pk_catalog)
    {
      pk_catalog= sqlchar_as_sqlchar(dbc->ansi_charset_info,
                                     dbc->cxn_charset_info,
                                     pk_catalog, &len, &errors);
      pk_catalog_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (pk_schema)
    {
      pk_schema= sqlchar_as_sqlchar(dbc->ansi_charset_info,
                                    dbc->cxn_charset_info,
                                    pk_schema, &len, &errors);
      pk_schema_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (pk_table)
    {
      pk_table= sqlchar_as_sqlchar(dbc->ansi_charset_info,
                                   dbc->cxn_charset_info,
                                   pk_table, &len, &errors);
      pk_table_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (fk_catalog)
    {
      fk_catalog= sqlchar_as_sqlchar(dbc->ansi_charset_info,
                                     dbc->cxn_charset_info,
                                     fk_catalog, &len, &errors);
      fk_catalog_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (fk_schema)
    {
      fk_schema= sqlchar_as_sqlchar(dbc->ansi_charset_info,
                                    dbc->cxn_charset_info,
                                    fk_schema, &len, &errors);
      fk_schema_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (fk_table)
    {
      fk_table= sqlchar_as_sqlchar(dbc->ansi_charset_info,
                                   dbc->cxn_charset_info,
                                   fk_table, &len, &errors);
      fk_table_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }
  }

  rc= MySQLForeignKeys(hstmt, pk_catalog, pk_catalog_len,
                       pk_schema, pk_schema_len, pk_table, pk_table_len,
                       fk_catalog, fk_catalog_len, fk_schema, fk_schema_len,
                       fk_table, fk_table_len);

  if (dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
  {
    x_free(pk_catalog);
    x_free(pk_schema);
    x_free(pk_table);
    x_free(fk_catalog);
    x_free(fk_schema);
    x_free(fk_table);
  }

  return rc;
}


SQLRETURN SQL_API
SQLGetConnectAttr(SQLHDBC hdbc, SQLINTEGER attribute, SQLPOINTER value,
                  SQLINTEGER value_max, SQLINTEGER *value_len)
{
  return SQLGetConnectAttrImpl(hdbc, attribute, value, value_max, value_len);
}


SQLRETURN SQL_API
SQLGetConnectAttrImpl(SQLHDBC hdbc, SQLINTEGER attribute, SQLPOINTER value,
                      SQLINTEGER value_max, SQLINTEGER *value_len)
{
  DBC *dbc= (DBC *)hdbc;
  SQLCHAR *char_value= NULL;
  SQLRETURN rc= 0;

  /* 
    for numeric attributes value_max can be 0, so we must check for 
    the valid output buffer to prevent crashes
  */
  if (value)
    rc= MySQLGetConnectAttr(hdbc, attribute, &char_value, value);

  if (char_value)
  {
    SQLSMALLINT free_value= FALSE;
    SQLINTEGER len= SQL_NTS;
    uint errors;

    /* 
      When SQLGetConnectAttr is called before connecting
      dbc->ansi_charset_info and dbc->cxn_charset_info will be NULL,
      so we can just copy the char_value as is.
    */
    if (dbc->ansi_charset_info && dbc->cxn_charset_info &&
        dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
    {
      char_value= sqlchar_as_sqlchar(dbc->cxn_charset_info,
                                     dbc->ansi_charset_info,
                                     char_value, &len, &errors);
      free_value= TRUE;
    }
    else
      len= strlen((char *)char_value);

    /* 
      This check is inside the statement, which does not
      execute if output buffer is NULL 
      see: "if (char_value)"
    */
    if (len > value_max - 1)
      rc= set_conn_error(dbc, MYERR_01004, NULL, 0);

    if (value && value_max > 1)
      strmake((char *)value, (char *)char_value, value_max - 1);

    if (value_len)
      *value_len= len;

    if (free_value)
      x_free(char_value);
  }

  return rc;
}


SQLRETURN SQL_API
SQLGetConnectOption(SQLHDBC hdbc, SQLUSMALLINT option, SQLPOINTER value)
{
  return SQLGetConnectAttrImpl(hdbc, option, value,
                               ((option == SQL_ATTR_CURRENT_CATALOG) ?
                                SQL_MAX_OPTION_STRING_LENGTH : 0), NULL);
}


SQLRETURN SQL_API
SQLGetCursorName(SQLHSTMT hstmt, SQLCHAR *cursor, SQLSMALLINT cursor_max,
                 SQLSMALLINT *cursor_len)
{
  STMT *stmt= (STMT *)hstmt;
  SQLCHAR *name;
  my_bool free_name= FALSE;
  SQLINTEGER len;
  uint errors;

  CLEAR_STMT_ERROR(stmt);

  if (cursor_max < 0)
    return set_error(stmt, MYERR_S1090, NULL, 0);

  if (stmt->dbc->ansi_charset_info->number ==
      stmt->dbc->cxn_charset_info->number)
  {
    name= MySQLGetCursorName(hstmt);
    len= strlen((char *)name);
  }
  else
  {
    name= sqlchar_as_sqlchar(stmt->dbc->cxn_charset_info,
                             stmt->dbc->ansi_charset_info,
                             MySQLGetCursorName(hstmt),
                             &len, &errors);
    free_name= TRUE;
  }

  if (cursor && cursor_max > 1)
    strmake((char *)cursor, (char *)name, cursor_max - 1);

  if (cursor_len)
    *cursor_len= (SQLSMALLINT)len;

  if (free_name)
    x_free(name);

  /* We set the error only when the result is intented to be returned */
  if (cursor && len > cursor_max - 1)
    return set_error(stmt, MYERR_01004, NULL, 0);

  return SQL_SUCCESS;
}


SQLRETURN SQL_API
SQLGetDiagField(SQLSMALLINT handle_type, SQLHANDLE handle,
                SQLSMALLINT record, SQLSMALLINT field,
                SQLPOINTER info, SQLSMALLINT info_max,
                SQLSMALLINT *info_len)
{
  DBC *dbc;
  SQLCHAR *value= NULL;
  SQLINTEGER len= SQL_NTS;

  SQLRETURN rc= MySQLGetDiagField(handle_type, handle, record, field,
                                  &value, info);

  switch (handle_type) {
  case SQL_HANDLE_DBC:
    dbc= (DBC *)handle;
    break;
  case SQL_HANDLE_STMT:
    dbc= ((STMT *)handle)->dbc;
    break;
  case SQL_HANDLE_DESC:
    dbc= DESC_GET_DBC((DESC *)handle);
    break;
  case SQL_HANDLE_ENV:
  default:
    dbc= NULL;
  }

  if (value)
  {
    SQLINTEGER free_value= FALSE;
    uint errors;
    if (dbc && dbc->ansi_charset_info &&
        dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
    {
      value= sqlchar_as_sqlchar(dbc->cxn_charset_info,
                                    dbc->ansi_charset_info,
                                    value, &len, &errors);
      free_value= TRUE;
    }
    else
      len= strlen((char *)value);

    /* We set the error only when the result is intented to be returned */
    if (info && len > info_max - 1)
      rc= set_conn_error(dbc, MYERR_01004, NULL, 0);

    if (info_len)
      *info_len= (SQLSMALLINT)len;

    if (info && info_max > 1)
      strmake((char *)info, (char *)value, info_max - 1);

    if (free_value)
      x_free(value);
  }

  return rc;
}


SQLRETURN SQL_API
SQLGetDiagRec(SQLSMALLINT handle_type, SQLHANDLE handle,
              SQLSMALLINT record, SQLCHAR *sqlstate,
              SQLINTEGER *native_error, SQLCHAR *message,
              SQLSMALLINT message_max, SQLSMALLINT *message_len)
{
  return SQLGetDiagRecImpl(handle_type, handle, record, sqlstate, native_error,
                           message, message_max, message_len);
}


SQLRETURN SQL_API
SQLGetDiagRecImpl(SQLSMALLINT handle_type, SQLHANDLE handle,
                  SQLSMALLINT record, SQLCHAR *sqlstate,
                  SQLINTEGER *native_error, SQLCHAR *message,
                  SQLSMALLINT message_max, SQLSMALLINT *message_len)
{
  SQLRETURN rc;
  DBC *dbc;
  SQLCHAR *msg_value= NULL, *sqlstate_value= NULL;
  SQLINTEGER len= SQL_NTS;
  SQLSMALLINT free_value= FALSE;
  uint errors;

  switch (handle_type) {
  case SQL_HANDLE_DBC:
    dbc= (DBC *)handle;
    break;
  case SQL_HANDLE_STMT:
    dbc= ((STMT *)handle)->dbc;
    break;
  case SQL_HANDLE_DESC:
    dbc= DESC_GET_DBC((DESC *)handle);
    break;
  case SQL_HANDLE_ENV:
  default:
    dbc= NULL;
  }

  if (message_max < 0)
    return SQL_ERROR;

  rc= MySQLGetDiagRec(handle_type, handle, record, &sqlstate_value,
                      native_error, &msg_value);

  if (rc == SQL_NO_DATA_FOUND)
  {
    return SQL_NO_DATA_FOUND;
  }

  if (msg_value)
  {
    if (dbc && dbc->ansi_charset_info && dbc->cxn_charset_info &&
        dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
    {
      msg_value= sqlchar_as_sqlchar(dbc->cxn_charset_info,
                                    dbc->ansi_charset_info,
                                    msg_value, &len, &errors);
      free_value= TRUE;
    }
    else
      len= strlen((char *)msg_value);

    /* 
      We set the error only when the result is intented to be returned
      and message_max is greaater than 0
    */
    if (message && message_max && len > message_max - 1)
      rc= set_conn_error(dbc, MYERR_01004, NULL, 0);

    if (message_len)
      *message_len= (SQLSMALLINT)len;

    if (message && message_max > 1)
      strmake((char *)message, (char *)msg_value, message_max - 1);

    if (free_value)
      x_free(msg_value);
  }

  if (sqlstate && sqlstate_value)
  {
    if (dbc && dbc->ansi_charset_info && dbc->cxn_charset_info &&
        dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
    {
      len= SQL_NTS;
      sqlstate_value= sqlchar_as_sqlchar(dbc->cxn_charset_info,
                                         dbc->ansi_charset_info,
                                         sqlstate_value, &len, &errors);
      free_value= TRUE;
    }
    else
      free_value= FALSE;

    strmake((char *)sqlstate,
            sqlstate_value ? (char *)sqlstate_value : "00000", 5);

    if (free_value)
      x_free(sqlstate_value);
  }

  return rc;
}


SQLRETURN SQL_API
SQLGetInfo(SQLHDBC hdbc, SQLUSMALLINT type, SQLPOINTER value,
            SQLSMALLINT value_max, SQLSMALLINT *value_len)
{
  DBC *dbc= (DBC *)hdbc;
  SQLCHAR *char_value= NULL;
  SQLINTEGER len= SQL_NTS;
  SQLSMALLINT free_value= FALSE;
  uint errors;

  SQLRETURN rc= MySQLGetInfo(hdbc, type, &char_value, value, value_len);

  if (char_value)
  {
    if (dbc->ansi_charset_info && dbc->cxn_charset_info
      && dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
    {
      char_value= sqlchar_as_sqlchar(dbc->cxn_charset_info,
                                     dbc->ansi_charset_info,
                                     char_value, &len, &errors);
      free_value= TRUE;
    }
    else
      len= strlen((char *)char_value);

    /* 
      MSSQL implementation does not return the truncation warning if the
      value is not NULL and value_max is 0
     */
    if (value && value_max && len > value_max - 1)
      rc= set_conn_error(dbc, MYERR_01004, NULL, 0);

    if (value && value_max > 1)
      strmake((char *)value, (char *)char_value, value_max - 1);

    if (value_len)
      *value_len= (SQLSMALLINT)len;

    if (free_value)
      x_free(char_value);
  }

  return rc;
}


SQLRETURN SQL_API
SQLGetStmtAttr(SQLHSTMT hstmt, SQLINTEGER attribute, SQLPOINTER value,
                SQLINTEGER value_max, SQLINTEGER *value_len)
{
  /* Nothing special to do, since we don't have any string stmt attribs */
  return MySQLGetStmtAttr(hstmt, attribute, value, value_max, value_len);
}


SQLRETURN SQL_API
SQLGetTypeInfo(SQLHSTMT hstmt, SQLSMALLINT type)
{
  return MySQLGetTypeInfo(hstmt, type);
}


SQLRETURN SQL_API
SQLNativeSql(SQLHDBC hdbc, SQLCHAR *in, SQLINTEGER in_len,
             SQLCHAR *out, SQLINTEGER out_max, SQLINTEGER *out_len)
{
  SQLRETURN rc= SQL_SUCCESS;

  if (in_len == SQL_NTS)
  {
    in_len= strlen((char *)in);
  }

  if (out_len)
  {
    *out_len= in_len;
  }

  if (out && in_len >= out_max)
  {
    rc= set_conn_error((DBC *)hdbc, MYERR_01004, NULL, 0);
  }

  if(out_max > 0)
  {
    if (in_len > out_max - 1)
    {
      in_len= out_max - 1;
    }

    (void)memcpy((char *)out, (const char *)in, in_len);
    out[in_len]= '\0';
  }

  return rc;
}


SQLRETURN SQL_API
SQLPrepare(SQLHSTMT hstmt, SQLCHAR *str, SQLINTEGER str_len)
{
  return SQLPrepareImpl(hstmt, str, str_len);
}


SQLRETURN SQL_API
SQLPrepareImpl(SQLHSTMT hstmt, SQLCHAR *str, SQLINTEGER str_len)
{
  STMT *stmt= (STMT *)hstmt;

  /*
    If the ANSI character set is the same as the connection character set,
    we can pass it straight through. Otherwise it needs to be converted to
    the connection character set (probably utf-8).
  */
  if (stmt->dbc->ansi_charset_info->number ==
      stmt->dbc->cxn_charset_info->number)
    return MySQLPrepare(hstmt, str, str_len, FALSE);
  else
  {
    uint errors= 0;

    str= sqlchar_as_sqlchar(stmt->dbc->ansi_charset_info,
                            stmt->dbc->cxn_charset_info,
                            str, &str_len, &errors);

    if (!str && str_len == -1)
    {
      set_mem_error(&stmt->dbc->mysql);
      return handle_connection_error(stmt);
    }

    /* Character conversion problems are not tolerated. */
    if (errors)
    {
      x_free(str);
      return set_stmt_error(stmt, "22018", NULL, 0);
    }

    return MySQLPrepare(hstmt, str, str_len, TRUE);
  }
}


SQLRETURN SQL_API
SQLPrimaryKeys(SQLHSTMT hstmt,
               SQLCHAR *catalog, SQLSMALLINT catalog_len,
               SQLCHAR *schema, SQLSMALLINT schema_len,
               SQLCHAR *table, SQLSMALLINT table_len)
{
  SQLRETURN rc;
  DBC *dbc= ((STMT *)hstmt)->dbc;

  if (dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
  {
    SQLINTEGER len= SQL_NTS;
    uint errors= 0;

    if (catalog)
    {
      catalog= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                  catalog, &len, &errors);
      catalog_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (schema)
    {
      schema= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                 schema, &len, &errors);
      schema_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (table)
    {
      table= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                table, &len, &errors);
      table_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }
  }

  rc= MySQLPrimaryKeys(hstmt, catalog, catalog_len, schema, schema_len,
                       table, table_len);

  if (dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
  {
    x_free(catalog);
    x_free(schema);
    x_free(table);
  }

  return rc;
}


SQLRETURN SQL_API
SQLProcedureColumns(SQLHSTMT hstmt,
                    SQLCHAR *catalog, SQLSMALLINT catalog_len,
                    SQLCHAR *schema, SQLSMALLINT schema_len,
                    SQLCHAR *proc, SQLSMALLINT proc_len,
                    SQLCHAR *column, SQLSMALLINT column_len)
{
  SQLRETURN rc;
  DBC *dbc= ((STMT *)hstmt)->dbc;

  if (dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
  {
    SQLINTEGER len= SQL_NTS;
    uint errors= 0;

    if (catalog)
    {
      catalog= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                  catalog, &len, &errors);
      catalog_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (schema)
    {
      schema= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                 schema, &len, &errors);
      schema_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (proc)
    {
      proc= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                               proc, &len, &errors);
      proc_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (column)
    {
      column= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                 column, &len, &errors);
      column_len= (SQLSMALLINT)len;
    }
  }

  rc= MySQLProcedureColumns(hstmt, catalog, catalog_len,
                            schema, schema_len, proc, proc_len,
                            column, column_len);

  if (dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
  {
    x_free(catalog);
    x_free(schema);
    x_free(proc);
  }

  return rc;
}


SQLRETURN SQL_API
SQLProcedures(SQLHSTMT hstmt,
              SQLCHAR *catalog, SQLSMALLINT catalog_len,
              SQLCHAR *schema, SQLSMALLINT schema_len,
              SQLCHAR *proc, SQLSMALLINT proc_len)
{
  SQLRETURN rc;
  DBC *dbc= ((STMT *)hstmt)->dbc;

  if (dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
  {
    SQLINTEGER len= SQL_NTS;
    uint errors= 0;

    if (catalog)
    {
      catalog= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                  catalog, &len, &errors);
      catalog_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (schema)
    {
      schema= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                 schema, &len, &errors);
      schema_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (proc)
    {
      proc= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                proc, &len, &errors);
      proc_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }
  }

  rc= MySQLProcedures(hstmt, catalog, catalog_len, schema, schema_len,
                      proc, proc_len);

  if (dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
  {
    x_free(catalog);
    x_free(schema);
    x_free(proc);
  }

  return rc;
}


SQLRETURN SQL_API
SQLSetConnectAttr(SQLHDBC hdbc, SQLINTEGER attribute,
                  SQLPOINTER value, SQLINTEGER value_len)
{
  return SQLSetConnectAttrImpl(hdbc, attribute, value, value_len);
}


SQLRETURN SQL_API
SQLSetConnectAttrImpl(SQLHDBC hdbc, SQLINTEGER attribute,
                      SQLPOINTER value, SQLINTEGER value_len)
{
  SQLRETURN rc;
  DBC *dbc= (DBC *)hdbc;
  my_bool free_value= FALSE;

  if (dbc->ansi_charset_info &&
      dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
  {
    /* SQL_ATTR_CURRENT_CATALOG is the only string attribute we support. */
    if (attribute == SQL_ATTR_CURRENT_CATALOG)
    {
      uint errors= 0;
      value= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                value, &value_len, &errors);
      if (!value && value_len == -1)
      {
        set_mem_error(&dbc->mysql);
        return set_conn_error(dbc, MYERR_S1001, mysql_error(&dbc->mysql),
                              mysql_errno(&dbc->mysql));
      }
      free_value= TRUE;
    }
  }

  rc= MySQLSetConnectAttr(hdbc, attribute, value, value_len);

  if (free_value)
    x_free(value);

  return rc;
}


SQLRETURN SQL_API
SQLSetConnectOption(SQLHDBC hdbc, SQLUSMALLINT option, SQLULEN param)
{
  SQLINTEGER value_len= 0;
  if (option == SQL_ATTR_CURRENT_CATALOG)
    value_len= SQL_NTS;

  return SQLSetConnectAttrImpl(hdbc, option, (SQLPOINTER)param, value_len);
}


SQLRETURN SQL_API
SQLSetCursorName(SQLHSTMT hstmt, SQLCHAR *name, SQLSMALLINT name_len)
{
  STMT *stmt= (STMT *)hstmt;
  SQLINTEGER len= name_len;
  uint errors= 0;

  if (stmt->dbc->ansi_charset_info->number ==
      stmt->dbc->cxn_charset_info->number)
    return MySQLSetCursorName(hstmt, name, name_len);

  name= sqlchar_as_sqlchar(stmt->dbc->ansi_charset_info,
                           stmt->dbc->cxn_charset_info,
                           name, &len, &errors);

  if (!name && len == -1)
  {
    set_mem_error(&stmt->dbc->mysql);
    return handle_connection_error(stmt);
  }

  /* Character conversion problems are not tolerated. */
  if (errors)
  {
    x_free(name);
    return set_stmt_error(stmt, "HY000",
                          "Cursor name included characters that could not "
                          "be converted to connection character set", 0);
  }

  return MySQLSetCursorName(hstmt, name, (SQLSMALLINT)len);
}


SQLRETURN SQL_API
SQLSetStmtAttr(SQLHSTMT hstmt, SQLINTEGER attribute,
               SQLPOINTER value, SQLINTEGER value_len)
{
  /* Nothing special to do, since we don't have any string stmt attribs */
  return MySQLSetStmtAttr(hstmt, attribute, value, value_len);
}


SQLRETURN SQL_API
SQLSpecialColumns(SQLHSTMT hstmt, SQLUSMALLINT type,
                  SQLCHAR *catalog, SQLSMALLINT catalog_len,
                  SQLCHAR *schema, SQLSMALLINT schema_len,
                  SQLCHAR *table, SQLSMALLINT table_len,
                  SQLUSMALLINT scope, SQLUSMALLINT nullable)
{
  SQLRETURN rc;
  DBC *dbc= ((STMT *)hstmt)->dbc;

  if (dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
  {
    SQLINTEGER len= SQL_NTS;
    uint errors= 0;

    if (catalog)
    {
      catalog= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                  catalog, &len, &errors);
      catalog_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (schema)
    {
      schema= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                 schema, &len, &errors);
      schema_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (table)
    {
      table= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                table, &len, &errors);
      table_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }
  }

  rc= MySQLSpecialColumns(hstmt, type, catalog, catalog_len, schema, schema_len,
                          table, table_len, scope, nullable);

  if (dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
  {
    x_free(catalog);
    x_free(schema);
    x_free(table);
  }

  return rc;
}


SQLRETURN SQL_API
SQLStatistics(SQLHSTMT hstmt,
              SQLCHAR *catalog, SQLSMALLINT catalog_len,
              SQLCHAR *schema, SQLSMALLINT schema_len,
              SQLCHAR *table, SQLSMALLINT table_len,
              SQLUSMALLINT unique, SQLUSMALLINT accuracy)
{
  SQLRETURN rc;
  DBC *dbc= ((STMT *)hstmt)->dbc;

  if (dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
  {
    SQLINTEGER len= SQL_NTS;
    uint errors= 0;

    if (catalog)
    {
      catalog= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                  catalog, &len, &errors);
      catalog_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (schema)
    {
      schema= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                 schema, &len, &errors);
      schema_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (table)
    {
      table= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                table, &len, &errors);
      table_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }
  }

  rc= MySQLStatistics(hstmt, catalog, catalog_len, schema, schema_len,
                      table, table_len, unique, accuracy);

  if (dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
  {
    x_free(catalog);
    x_free(schema);
    x_free(table);
  }

  return rc;
}


SQLRETURN SQL_API
SQLTablePrivileges(SQLHSTMT hstmt,
                   SQLCHAR *catalog, SQLSMALLINT catalog_len,
                   SQLCHAR *schema, SQLSMALLINT schema_len,
                   SQLCHAR *table, SQLSMALLINT table_len)
{
  SQLRETURN rc;
  DBC *dbc= ((STMT *)hstmt)->dbc;

  if (dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
  {
    SQLINTEGER len= SQL_NTS;
    uint errors= 0;

    if (catalog)
    {
      catalog= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                  catalog, &len, &errors);
      catalog_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (schema)
    {
      schema= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                 schema, &len, &errors);
      schema_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (table)
    {
      table= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                table, &len, &errors);
      table_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }
  }

  rc= MySQLTablePrivileges(hstmt, catalog, catalog_len, schema, schema_len,
                           table, table_len);

  if (dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
  {
    x_free(catalog);
    x_free(schema);
    x_free(table);
  }

  return rc;
}


SQLRETURN SQL_API
SQLTables(SQLHSTMT hstmt,
          SQLCHAR *catalog, SQLSMALLINT catalog_len,
          SQLCHAR *schema, SQLSMALLINT schema_len,
          SQLCHAR *table, SQLSMALLINT table_len,
          SQLCHAR *type, SQLSMALLINT type_len)
{
  SQLRETURN rc;
  DBC *dbc= ((STMT *)hstmt)->dbc;

  if (dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
  {
    SQLINTEGER len= SQL_NTS;
    uint errors= 0;

    if (catalog)
    {
      catalog= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                  catalog, &len, &errors);
      if (!len)
        catalog= "";
      catalog_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (schema)
    {
      schema= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                 schema, &len, &errors);
      if (!len)
        schema= "";
      schema_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (table)
    {
      table= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                table, &len, &errors);
      if (!len)
        table= "";
      table_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }

    if (type)
    {
      type= sqlchar_as_sqlchar(dbc->ansi_charset_info, dbc->cxn_charset_info,
                                 type, &len, &errors);
      type_len= (SQLSMALLINT)len;
      len= SQL_NTS;
    }
  }

  rc= MySQLTables(hstmt, catalog, catalog_len, schema, schema_len,
                  table, table_len, type, type_len);

  if (dbc->ansi_charset_info->number != dbc->cxn_charset_info->number)
  {
    if (catalog_len)
      x_free(catalog);
    if (schema_len)
      x_free(schema);
    if (table_len)
      x_free(table);
    x_free(type);
  }

  return rc;
}


SQLRETURN SQL_API
SQLGetDescField(SQLHDESC hdesc, SQLSMALLINT record, SQLSMALLINT field,
                SQLPOINTER value, SQLINTEGER value_max, SQLINTEGER *value_len)
{
  return MySQLGetDescField(hdesc, record, field, value, value_max, value_len);
}


SQLRETURN SQL_API
SQLGetDescRec(SQLHDESC hdesc, SQLSMALLINT record, SQLCHAR *name,
              SQLSMALLINT name_max, SQLSMALLINT *name_len, SQLSMALLINT *type,
              SQLSMALLINT *subtype, SQLLEN *length, SQLSMALLINT *precision,
              SQLSMALLINT *scale, SQLSMALLINT *nullable)
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLSetDescField(SQLHDESC hdesc, SQLSMALLINT record, SQLSMALLINT field,
                SQLPOINTER value, SQLINTEGER value_len)
{
  return MySQLSetDescField(hdesc, record, field, value, value_len);
}


SQLRETURN SQL_API
SQLSetDescRec(SQLHDESC hdesc, SQLSMALLINT record, SQLSMALLINT type,
              SQLSMALLINT subtype, SQLLEN length, SQLSMALLINT precision,
              SQLSMALLINT scale, SQLPOINTER data_ptr,
              SQLLEN *octet_length_ptr, SQLLEN *indicator_ptr)
{
  NOT_IMPLEMENTED;
}


/**
  Discover and enumerate the attributes and attribute values required to
  connect.

  @return Always returns @c SQL_ERROR, because the driver does not support this.

  @since ODBC 1.0
*/
SQLRETURN SQL_API
SQLBrowseConnect(SQLHDBC hdbc, SQLCHAR *in, SQLSMALLINT in_len,
                 SQLCHAR *out, SQLSMALLINT out_max, SQLSMALLINT *out_len)
{
  return set_conn_error(hdbc,MYERR_S1000,
                        "Driver does not support this API", 0);
}


#ifdef NOT_IMPLEMENTED_YET
//SQLDataSources


//SQLDrivers


#endif
