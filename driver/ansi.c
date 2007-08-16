/*
  Copyright (C) 2007 MySQL AB

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  There are special exceptions to the terms and conditions of the GPL
  as it is applied to this software. View the full text of the exception
  in file LICENSE.exceptions in the top-level directory of this software
  distribution.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/**
  @file  ansi.c
  @brief Entry points for ANSI versions of ODBC functions
*/

#include "myodbc3.h"


#define NOT_IMPLEMENTED \
  return SQL_ERROR


SQLRETURN SQL_API
SQLPrepareImpl(SQLHSTMT hstmt, SQLCHAR *str, SQLINTEGER str_len);


SQLRETURN SQL_API
SQLConnect(SQLHDBC hdbc, SQLCHAR *dsn, SQLSMALLINT dsn_len,
           SQLCHAR *user, SQLSMALLINT user_len,
           SQLCHAR *auth, SQLSMALLINT auth_len)
{
  return MySQLConnect(hdbc, dsn, dsn_len, user, user_len, auth, auth_len);
}


SQLRETURN SQL_API
SQLDriverConnect(SQLHDBC hdbc, SQLHWND hwnd, SQLCHAR *in, SQLSMALLINT in_len,
                 SQLCHAR *out, SQLSMALLINT out_max, SQLSMALLINT *out_len,
                 SQLUSMALLINT completion)
{
  return MySQLDriverConnect(hdbc, hwnd, in, in_len, out, out_max, out_len,
                            completion);
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
    uint32 used_bytes, used_chars, bytes;
    uint errors= 0;
    SQLCHAR *conv;

    if (str_len == SQL_NTS)
      str_len= strlen((char *)str);

    bytes= (str_len / stmt->dbc->ansi_charset_info->mbminlen *
                  stmt->dbc->cxn_charset_info->mbmaxlen);
    conv= (SQLCHAR *)my_malloc(bytes + 1, MYF(0));
    if (!conv)
    {
      set_mem_error(&stmt->dbc->mysql);
      return handle_connection_error(stmt);
    }

    str_len= copy_and_convert((char *)conv, bytes, stmt->dbc->cxn_charset_info,
                              (char *)str, str_len,
                              stmt->dbc->ansi_charset_info, &used_bytes,
                              &used_chars, &errors);
    /* Character conversion problems are not tolerated. */
    if (errors)
    {
      x_free(conv);
      return set_stmt_error(stmt, "22018", NULL, 0);
    }

    conv[str_len]= '\0';

    return MySQLPrepare(hstmt, conv, str_len, TRUE);
  }
}


#ifdef NOT_IMPLEMENTED_YET
SQLRETURN SQL_API
SQLBrowseConnect(SQLHDBC hdbc, SQLCHAR *in, SQLSMALLINT in_len,
                 SQLCHAR *out, SQLSMALLINT out_max, SQLSMALLINT *out_len)
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLColAttribute(SQLHSTMT hstmt, SQLUSMALLINT column,
                SQLUSMALLINT field, SQLPOINTER char_attr,
                SQLSMALLINT char_attr_max, SQLSMALLINT *char_attr_len,
                SQLPOINTER num_attr) /* SHould be SQLLEN * */
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLColAttributes(SQLHSTMT hstmt, SQLUSMALLINT column, SQLUSMALLINT type,
                 SQLPOINTER char_attr, SQLSMALLINT char_attr_max,
                 SQLSMALLINT *char_attr_len, SQLLEN *num_attr)
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLColumnPrivileges(SQLHSTMT hstmt,
                    SQLCHAR *catalog, SQLSMALLINT catalog_len,
                    SQLCHAR *schema, SQLSMALLINT schema_len,
                    SQLCHAR *table, SQLSMALLINT table_len,
                    SQLCHAR *column, SQLSMALLINT column_len)
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLColumns(SQLHSTMT hstmt,
           SQLCHAR *catalog, SQLSMALLINT catalog_len,
           SQLCHAR *schema, SQLSMALLINT schema_len,
           SQLCHAR *table, SQLSMALLINT table_len,
           SQLCHAR *column, SQLSMALLINT column_len)
{
  NOT_IMPLEMENTED;
}


//SQLDataSources


SQLRETURN SQL_API
SQLDescribeCol(SQLHSTMT hstmt, SQLUSMALLINT column,
               SQLCHAR *name, SQLSMALLINT name_max, SQLSMALLINT *name_len,
               SQLSMALLINT *type, SQLULEN *def, SQLSMALLINT *scale,
               SQLSMALLINT *nullable)
{
  NOT_IMPLEMENTED;
}


//SQLDrivers


SQLRETURN SQL_API
SQLError(SQLHENV henv, SQLHDBC hdbc, SQLHSTMT hstmt, SQLCHAR *sqlstate,
         SQLINTEGER *native_error, SQLCHAR *message, SQLSMALLINT message_max,
         SQLSMALLINT *message_len)
{
  NOT_IMPLEMENTED;
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
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLGetConnectAttr(SQLHDBC hdbc, SQLINTEGER attribute, SQLPOINTER value,
                  SQLINTEGER value_max, SQLINTEGER *value_len)
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLGetConnectOption(SQLHDBC hdbc, SQLUSMALLINT option, SQLPOINTER param)
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLGetCursorName(SQLHSTMT hstmt, SQLCHAR *cursor, SQLSMALLINT cursor_max,
                 SQLSMALLINT *cursor_len)
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLGetDescField(SQLHDESC hdesc, SQLSMALLINT record, SQLSMALLINT field,
                SQLPOINTER value, SQLINTEGER value_max, SQLINTEGER *value_len)
{
  NOT_IMPLEMENTED;
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
SQLGetDiagField(SQLSMALLINT handle_type, SQLHANDLE handle,
                SQLSMALLINT record, SQLSMALLINT field,
                SQLPOINTER info, SQLSMALLINT info_max,
                SQLSMALLINT *info_len)
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLGetDiagRec(SQLSMALLINT handle_type, SQLHANDLE handle,
              SQLSMALLINT record, SQLCHAR *sqlstate,
              SQLINTEGER *native_error, SQLCHAR *message,
              SQLSMALLINT message_max, SQLSMALLINT *message_len)
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLGetInfo(SQLHDBC hdbc, SQLUSMALLINT type, SQLPOINTER value,
            SQLSMALLINT value_max, SQLSMALLINT *value_len)
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLGetStmtAttr(SQLHSTMT hstmt, SQLINTEGER attribute, SQLPOINTER value,
                SQLINTEGER value_max, SQLINTEGER *value_len)
{
}


SQLRETURN SQL_API
SQLNativeSql(SQLHDBC hdbc, SQLCHAR *in, SQLINTEGER in_len,
             SQLCHAR *out, SQLINTEGER out_max, SQLINTEGER *out_len)
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLPrimaryKeys(SQLHSTMT hstmt,
               SQLCHAR *catalog, SQLSMALLINT catalog_len,
               SQLCHAR *schema, SQLSMALLINT schema_len,
               SQLCHAR *table, SQLSMALLINT table_len)
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLProcedureColumns(SQLHSTMT hstmt,
                    SQLCHAR *catalog, SQLSMALLINT catalog_len,
                    SQLCHAR *schema, SQLSMALLINT schema_len,
                    SQLCHAR *proc, SQLSMALLINT proc_len,
                    SQLCHAR *column, SQLSMALLINT column_len)
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLProcedures(SQLHSTMT hstmt,
              SQLCHAR *catalog, SQLSMALLINT catalog_len,
              SQLCHAR *schema, SQLSMALLINT schema_len,
              SQLCHAR *proc, SQLSMALLINT proc_len)
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLSetConnectAttr(SQLHDBC hdbc, SQLINTEGER attribute,
                  SQLPOINTER value, SQLINTEGER value_len)
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLSetConnectOption(SQLHDBC hdbc, SQLUSMALLINT option, SQLULEN param)
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLSetCursorName(SQLHSTMT hstmt, SQLCHAR *name, SQLSMALLINT name_len)
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLSetDescField(SQLHDESC hdesc, SQLSMALLINT record, SQLSMALLINT field,
                SQLPOINTER value, SQLINTEGER value_len)
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLSetStmtAttr(SQLHSTMT hstmt, SQLINTEGER attribute,
               SQLPOINTER value, SQLINTEGER value_len)
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLSpecialColumns(SQLHSTMT hstmt, SQLUSMALLINT type,
                  SQLCHAR *catalog, SQLSMALLINT catalog_len,
                  SQLCHAR *schema, SQLSMALLINT schema_len,
                  SQLCHAR *table, SQLSMALLINT table_len,
                  SQLUSMALLINT scope, SQLUSMALLINT nullable)
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLStatistics(SQLHSTMT hstmt,
              SQLCHAR *catalog, SQLSMALLINT catalog_len,
              SQLCHAR *schema, SQLSMALLINT schema_len,
              SQLCHAR *table, SQLSMALLINT table_len,
              SQLUSMALLINT unique, SQLUSMALLINT accuracy)
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLTablePrivileges(SQLHSTMT hstmt,
                   SQLCHAR *catalog, SQLSMALLINT catalog_len,
                   SQLCHAR *schema, SQLSMALLINT schema_len,
                   SQLCHAR *table, SQLSMALLINT table_len)
{
  NOT_IMPLEMENTED;
}


SQLRETURN SQL_API
SQLTables(SQLHSTMT hstmt,
          SQLCHAR *catalog, SQLSMALLINT catalog_len,
          SQLCHAR *schema, SQLSMALLINT schema_len,
          SQLCHAR *table, SQLSMALLINT table_len,
          SQLCHAR *type, SQLSMALLINT type_len)
{
  NOT_IMPLEMENTED;
}
#endif
