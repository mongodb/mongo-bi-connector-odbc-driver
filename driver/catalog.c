/*
  Copyright (c) 2000, 2012, Oracle and/or its affiliates. All rights reserved.

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
  @file  catalog.c
  @brief Catalog functions.
*/

#include "driver.h"
#include "catalog.h"

/*
  @type    : internal
  @purpose : checks if server supports I_S
  @remark  : All i_s_* functions suppose that parameters specifying other parameters lenthes can't SQL_NTS.
             caller should take care of that.
*/
my_bool server_has_i_s(DBC FAR *dbc)
{
  /*
    According to the server ChangeLog INFORMATION_SCHEMA was introduced
    in the 5.0.2
  */
  return is_minimum_version(dbc->mysql.server_version, "5.0.2");
}
/*
  @type    : internal
  @purpose : returns the next token
*/

const char *my_next_token(const char *prev_token, 
                                char **token, 
                                char *data, 
                          const char chr)
{
    const char *cur_token;

    if ( (cur_token= strchr(*token, chr)) )
    {
        if ( prev_token )
        {
            uint len= (uint)(cur_token-prev_token);
            strncpy(data,prev_token, len);
            data[len]= 0;    
        }
        *token= (char *)cur_token+1;
        prev_token= cur_token;
        return cur_token+1;
    }
    return 0;
}


/**
  Create a fake result set in the current statement

  @param[in] stmt           Handle to statement
  @param[in] rowval         Row array
  @param[in] rowsize        sizeof(row array)
  @param[in] rowcnt         Number of rows
  @param[in] fields         Field array
  @param[in] fldcnt         Number of fields

  @return SQL_SUCCESS or SQL_ERROR (and diag is set)
*/
SQLRETURN
create_fake_resultset(STMT *stmt, MYSQL_ROW rowval, size_t rowsize,
                      my_ulonglong rowcnt, MYSQL_FIELD *fields, uint fldcnt)
{
  stmt->result= (MYSQL_RES*) my_malloc(sizeof(MYSQL_RES), MYF(MY_ZEROFILL));
  stmt->result_array= (MYSQL_ROW)my_memdup((char *)rowval, rowsize, MYF(0));
  if (!(stmt->result && stmt->result_array))
  {
    x_free(stmt->result);
    x_free(stmt->result_array);

    set_mem_error(&stmt->dbc->mysql);
    return handle_connection_error(stmt);
  }
  stmt->fake_result= 1;

  set_row_count(stmt, rowcnt);

  mysql_link_fields(stmt, fields, fldcnt);

  return SQL_SUCCESS;
}


/**
  Create an empty fake result set in the current statement

  @param[in] stmt           Handle to statement
  @param[in] rowval         Row array
  @param[in] rowsize        sizeof(row array)
  @param[in] fields         Field array
  @param[in] fldcnt         Number of fields

  @return SQL_SUCCESS or SQL_ERROR (and diag is set)
*/
SQLRETURN
create_empty_fake_resultset(STMT *stmt, MYSQL_ROW rowval, size_t rowsize,
                            MYSQL_FIELD *fields, uint fldcnt)
{
  return create_fake_resultset(stmt, rowval, rowsize, 0 /* rowcnt */,
                               fields, fldcnt);
}


/**
  Get the table status for a table or tables using Information_Schema DB.
  Lengths may not be SQL_NTS.

  @param[in] stmt           Handle to statement
  @param[in] catalog        Catalog (database) of table, @c NULL for current
  @param[in] catalog_length Length of catalog name
  @param[in] table          Name of table
  @param[in] table_length   Length of table name
  @param[in] wildcard       Whether the table name is a wildcard

  @return Result of SHOW TABLE STATUS, or NULL if there is an error
          or empty result (check mysql_errno(&stmt->dbc->mysql) != 0)
*/
static MYSQL_RES *mysql_table_status_i_s(STMT        *stmt,
                                         SQLCHAR     *catalog,
                                         SQLSMALLINT  catalog_length,
                                         SQLCHAR     *table,
                                         SQLSMALLINT  table_length,
                                         my_bool      wildcard,
                                         my_bool      show_tables,
                                         my_bool      show_views)
{
  MYSQL *mysql= &stmt->dbc->mysql;
  /** the buffer size should count possible escapes */
  char buff[255+4*NAME_CHAR_LEN], *to;
  my_bool clause_added= FALSE;

  to= strmov(buff, "SELECT TABLE_NAME, TABLE_COMMENT, TABLE_TYPE, TABLE_SCHEMA \
                    FROM INFORMATION_SCHEMA.TABLES \
                    WHERE ");

  if (catalog && *catalog)
  {
    to= strmov(to, "TABLE_SCHEMA LIKE '");
    to+= myodbc_escape_string(mysql, to, (ulong)(sizeof(buff) - (to - buff)),
                              (char *)catalog, catalog_length, 1);
    to= strmov(to, "' ");
    clause_added= TRUE;
  }
  else
  {
    to= strmov(to, "TABLE_SCHEMA = DATABASE() ");
  }

  if (show_tables)
  {
    to= strmov(to, "AND ");
    if (show_views)
      to= strmov(to, "( ");
    to= strmov(to, "TABLE_TYPE='BASE TABLE' ");
  }

  if (show_views)
  {
    if (show_tables)
      to= strmov(to, "OR ");
    else
      to= strmov(to, "AND ");

    to= strmov(to, "TABLE_TYPE='VIEW' ");
    if (show_tables)
      to= strmov(to, ") ");
  }

  /*
    As a pattern-value argument, an empty string needs to be treated
    literally. (It's not the same as NULL, which is the same as '%'.)
    But it will never match anything, so bail out now.
  */
  if (table && wildcard && !*table)
    return NULL;

  if (table && *table)
  {
    to= strmov(to, "AND TABLE_NAME LIKE '");
    if (wildcard)
    {
      to+= mysql_real_escape_string(mysql, to, (char *)table, table_length);
    }
    else
    {
      to+= myodbc_escape_string(mysql, to, (ulong)(sizeof(buff) - (to - buff)),
                                (char *)table, table_length, 0);
    }
    to= strmov(to, "'");
  }

  assert(to - buff < sizeof(buff));

  MYLOG_QUERY(stmt, buff);

  if (mysql_real_query(mysql, buff, (unsigned long)(to - buff)))
  {
    return NULL;
  }

  return mysql_store_result(mysql);
}


/**
  Get the table status for a table or tables. Lengths may not be SQL_NTS.

  @param[in] stmt           Handle to statement
  @param[in] catalog        Catalog (database) of table, @c NULL for current
  @param[in] catalog_length Length of catalog name
  @param[in] table          Name of table
  @param[in] table_length   Length of table name
  @param[in] wildcard       Whether the table name is a wildcard

  @return Result of SHOW TABLE STATUS, or NULL if there is an error
          or empty result (check mysql_errno(&stmt->dbc->mysql) != 0)
*/
MYSQL_RES *mysql_table_status(STMT        *stmt,
                              SQLCHAR     *catalog,
                              SQLSMALLINT  catalog_length,
                              SQLCHAR     *table,
                              SQLSMALLINT  table_length,
                              my_bool      wildcard,
                              my_bool      show_tables,
                              my_bool      show_views)
{
  if (server_has_i_s(stmt->dbc) && !stmt->dbc->ds->no_information_schema)
    return mysql_table_status_i_s(stmt, catalog, catalog_length,
                                  table, table_length, wildcard,
                                             show_tables, show_views);
  else
    return mysql_table_status_show(stmt, catalog, catalog_length,
                                   table, table_length, wildcard);
}


/*
@type    :  internal
@purpose :  Adding name condition for arguments what depending on SQL_ATTR_METADATA_ID
            are either ordinary argument(oa) or identifier string(id)
            NULL _default parameter means that parameter is mandatory and error is generated
@returns :  1 if required parameter is NULL, 0 otherwise
*/
int add_name_condition_oa_id(HSTMT hstmt, char ** pos, SQLCHAR * name,
                              SQLSMALLINT name_len, char * _default)
{
  SQLUINTEGER metadata_id;

  /* this shouldn't be very expensive, so no need to complicate things with additional parameter etc */
  MySQLGetStmtAttr(hstmt, SQL_ATTR_METADATA_ID, (SQLPOINTER)&metadata_id, 0, NULL);

  /* we can't rely here that column was checked and is not null */
  if (name)
  {
    STMT FAR *stmt= (STMT FAR*) hstmt;

    if (metadata_id)
    {
      *pos= strmov(*pos, "=");
      /* Need also code to remove trailing blanks */
    }
    else
      *pos= strmov(*pos, "= BINARY ");

    *pos= strmov(*pos, "'");
    *pos+= mysql_real_escape_string(&stmt->dbc->mysql, *pos, (char *)name, name_len);
    *pos= strmov(*pos, "' ");
  }
  else
  {
    /* According to http://msdn.microsoft.com/en-us/library/ms714579%28VS.85%29.aspx
    identifier argument cannot be NULL with one exception not actual for mysql) */
    if (!metadata_id && _default)
      *pos= strmov(*pos, _default); 
    else
      return 1;
  }

  return 0;
}


/*
@type    :  internal
@purpose :  Adding name condition for arguments what depending on SQL_ATTR_METADATA_ID
are either pattern value(oa) or identifier string(id)
@returns :  1 if required parameter is NULL, 0 otherwise
*/
int add_name_condition_pv_id(HSTMT hstmt, char ** pos, SQLCHAR * name,
                                       SQLSMALLINT name_len, char * _default)
{
  SQLUINTEGER metadata_id;
  /* this shouldn't be very expensive, so no need to complicate things with additional parameter etc */
  MySQLGetStmtAttr(hstmt, SQL_ATTR_METADATA_ID, (SQLPOINTER)&metadata_id, 0, NULL);

  /* we can't rely here that column was checked and is not null */
  if (name)
  {
    STMT FAR *stmt= (STMT FAR*) hstmt;

    if (metadata_id)
    {
      *pos= strmov(*pos, "=");
      /* Need also code to remove trailing blanks */
    }
    else
      *pos= strmov(*pos, " LIKE BINARY ");

    *pos= strmov(*pos, "'");
    *pos+= mysql_real_escape_string(&stmt->dbc->mysql, *pos, (char *)name, name_len);
    *pos= strmov(*pos, "' ");
  }
  else
  {
    /* According to http://msdn.microsoft.com/en-us/library/ms714579%28VS.85%29.aspx
       identifier argument cannot be NULL with one exception not actual for mysql) */
    if (!metadata_id && _default)
      *pos= strmov(*pos, _default); 
    else
      return 1;
  }

  return 0;
}


/*
****************************************************************************
SQLTables
****************************************************************************
*/

SQLRETURN
i_s_tables(SQLHSTMT hstmt,
           SQLCHAR *catalog, SQLSMALLINT catalog_len,
           SQLCHAR *schema, SQLSMALLINT schema_len,
           SQLCHAR *table, SQLSMALLINT table_len,
           SQLCHAR *type, SQLSMALLINT type_len)
{
  /* The function is just a stub. We call non-I_S version of the function before implementing the I_S one */
  return mysql_tables(hstmt, catalog, catalog_len, schema, schema_len,
                      table, table_len, type, type_len);
}


SQLRETURN SQL_API
MySQLTables(SQLHSTMT hstmt,
            SQLCHAR *catalog, SQLSMALLINT catalog_len,
            SQLCHAR *schema, SQLSMALLINT schema_len,
            SQLCHAR *table, SQLSMALLINT table_len,
            SQLCHAR *type, SQLSMALLINT type_len)
{
  STMT *stmt= (STMT *)hstmt;

  CLEAR_STMT_ERROR(hstmt);
  my_SQLFreeStmt(hstmt, MYSQL_RESET);

  if (catalog_len == SQL_NTS)
    catalog_len= catalog ? (SQLSMALLINT)strlen((char *)catalog) : 0;
  if (schema_len == SQL_NTS)
    schema_len= schema ? (SQLSMALLINT)strlen((char *)schema) : 0;
  if (table_len == SQL_NTS)
    table_len= table ? (SQLSMALLINT)strlen((char *)table) : 0;
  if (type_len == SQL_NTS)
    type_len= type ? (SQLSMALLINT)strlen((char *)type) : 0;

  if(catalog_len > NAME_CHAR_LEN || schema_len > NAME_CHAR_LEN ||
     table_len > NAME_CHAR_LEN)
  {
    return set_stmt_error(stmt, "HY090", "One or more parameters exceed the maximum allowed name length", 0);
  }

  if (server_has_i_s(stmt->dbc) && !stmt->dbc->ds->no_information_schema)
  {
    return i_s_tables(hstmt, catalog, catalog_len, schema, schema_len,
                      table, table_len, type, type_len);
  }
  else
  {
    return mysql_tables(hstmt, catalog, catalog_len, schema, schema_len,
                        table, table_len, type, type_len);
  }
}


/*
****************************************************************************
SQLColumns
****************************************************************************
*/
/**
  Get information about the columns in one or more tables.

  @param[in] hstmt            Handle of statement
  @param[in] szCatalog        Name of catalog (database)
  @param[in] cbCatalog        Length of catalog
  @param[in] szSchema         Name of schema (unused)
  @param[in] cbSchema         Length of schema name
  @param[in] szTable          Pattern of table names to match
  @param[in] cbTable          Length of table pattern
  @param[in] szColumn         Pattern of column names to match
  @param[in] cbColumn         Length of column pattern
*/
SQLRETURN
i_s_columns(SQLHSTMT hstmt, SQLCHAR *szCatalog, SQLSMALLINT cbCatalog,
            SQLCHAR *szSchema __attribute__((unused)),
            SQLSMALLINT cbSchema __attribute__((unused)),
            SQLCHAR *szTable, SQLSMALLINT cbTable,
            SQLCHAR *szColumn, SQLSMALLINT cbColumn)

{
  /* The function is just a stub. We call non-I_S version of the function before implementing the I_S one */
  return mysql_columns(hstmt, szCatalog, cbCatalog,szSchema, cbSchema,
                       szTable, cbTable, szColumn, cbColumn);
}


/**
  Get information about the columns in one or more tables.

  @param[in] hstmt            Handle of statement
  @param[in] szCatalog        Name of catalog (database)
  @param[in] cbCatalog        Length of catalog
  @param[in] szSchema         Name of schema (unused)
  @param[in] cbSchema         Length of schema name
  @param[in] szTable          Pattern of table names to match
  @param[in] cbTable          Length of table pattern
  @param[in] szColumn         Pattern of column names to match
  @param[in] cbColumn         Length of column pattern
*/
SQLRETURN SQL_API
MySQLColumns(SQLHSTMT hstmt, SQLCHAR *szCatalog, SQLSMALLINT cbCatalog,
             SQLCHAR *szSchema __attribute__((unused)),
             SQLSMALLINT cbSchema __attribute__((unused)),
             SQLCHAR *szTable, SQLSMALLINT cbTable,
             SQLCHAR *szColumn, SQLSMALLINT cbColumn)

{
  STMT *stmt= (STMT *)hstmt;

  CLEAR_STMT_ERROR(hstmt);
  my_SQLFreeStmt(hstmt, MYSQL_RESET);

  if (cbCatalog == SQL_NTS)
    cbCatalog= szCatalog != NULL ? (SQLSMALLINT)strlen((char *)szCatalog) : 0;
  if (cbColumn == SQL_NTS)
    cbColumn= szColumn != NULL ? (SQLSMALLINT)strlen((char *)szColumn) : 0;
  if (cbTable == SQL_NTS)
    cbTable= szTable != NULL ? (SQLSMALLINT)strlen((char *)szTable) : 0;

  if (server_has_i_s(stmt->dbc) && !stmt->dbc->ds->no_information_schema)
  {
    return i_s_columns(hstmt, szCatalog, cbCatalog,szSchema, cbSchema,
                         szTable, cbTable, szColumn, cbColumn);
  }
  else
  {
    return mysql_columns(hstmt, szCatalog, cbCatalog,szSchema, cbSchema,
                         szTable, cbTable, szColumn, cbColumn);
  }
}


/*
****************************************************************************
SQLStatistics
****************************************************************************
*/

/*
  @purpose : retrieves a list of statistics about a single table and the
       indexes associated with the table. The driver returns the
       information as a result set.
*/

SQLRETURN
i_s_statistics(SQLHSTMT hstmt,
                SQLCHAR *catalog, SQLSMALLINT catalog_len,
                SQLCHAR *schema __attribute__((unused)),
                SQLSMALLINT schema_len __attribute__((unused)),
                SQLCHAR *table, SQLSMALLINT table_len,
                SQLUSMALLINT fUnique,
                SQLUSMALLINT fAccuracy __attribute__((unused)))
{
  /* The function is just a stub. We call non-I_S version of the function before implementing the I_S one */
  return mysql_statistics(hstmt, catalog, catalog_len, schema, schema_len,
                          table, table_len, fUnique, fAccuracy);
}


/*
  @type    : ODBC 1.0 API
  @purpose : retrieves a list of statistics about a single table and the
       indexes associated with the table. The driver returns the
       information as a result set.
*/

SQLRETURN SQL_API
MySQLStatistics(SQLHSTMT hstmt,
                SQLCHAR *catalog, SQLSMALLINT catalog_len,
                SQLCHAR *schema __attribute__((unused)),
                SQLSMALLINT schema_len __attribute__((unused)),
                SQLCHAR *table, SQLSMALLINT table_len,
                SQLUSMALLINT fUnique,
                SQLUSMALLINT fAccuracy __attribute__((unused)))
{
  STMT *stmt= (STMT *)hstmt;

  CLEAR_STMT_ERROR(hstmt);
  my_SQLFreeStmt(hstmt,MYSQL_RESET);


  if (catalog_len == SQL_NTS)
    catalog_len= catalog ? (SQLSMALLINT)strlen((char *)catalog) : 0;
  if (table_len == SQL_NTS)
    table_len= table ? (SQLSMALLINT)strlen((char *)table) : 0;

  if (server_has_i_s(stmt->dbc) && !stmt->dbc->ds->no_information_schema)
  {
    return i_s_statistics(hstmt, catalog, catalog_len, schema, schema_len,
                          table, table_len, fUnique, fAccuracy);
  }
  else
  {
    return mysql_statistics(hstmt, catalog, catalog_len, schema, schema_len,
                          table, table_len, fUnique, fAccuracy);
  }
}

/*
****************************************************************************
SQLTablePrivileges
****************************************************************************
*/
/*
  @type    : internal
  @purpose : fetches data from I_S table_privileges. returns SQLRETURN of the operation
*/
SQLRETURN i_s_list_table_priv(SQLHSTMT    hstmt,
                              SQLCHAR *   catalog,
                              SQLSMALLINT catalog_len,
                              SQLCHAR *   schema      __attribute__((unused)),
                              SQLSMALLINT schema_len  __attribute__((unused)),
                              SQLCHAR *   table,
                              SQLSMALLINT table_len)
{
  STMT FAR *stmt=(STMT FAR*) hstmt;
  MYSQL *mysql= &stmt->dbc->mysql;
  char   buff[255+4*NAME_LEN+1], *pos;
  SQLRETURN rc;

  /* Db,User,Table_name,"NULL" as Grantor,Table_priv*/
  pos= strmov(buff,
               "SELECT TABLE_SCHEMA as TABLE_CAT, TABLE_CATALOG as TABLE_SCHEM,"
                      "TABLE_NAME, NULL as GRANTOR, GRANTEE,"
                      "PRIVILEGE_TYPE as PRIVILEGE, IS_GRANTABLE "
               "FROM INFORMATION_SCHEMA.TABLE_PRIVILEGES "
               "WHERE TABLE_NAME");

  add_name_condition_pv_id(hstmt, &pos, table, table_len, " LIKE '%'" );

  pos= strmov(pos, " AND TABLE_SCHEMA");
  add_name_condition_oa_id(hstmt, &pos, catalog, catalog_len, "=DATABASE()");

  /* TABLE_CAT is always NULL in mysql I_S */
  pos= strmov(pos, " ORDER BY /*TABLE_CAT,*/ TABLE_SCHEM, TABLE_NAME, PRIVILEGE, GRANTEE");

  assert(pos - buff < sizeof(buff));

  if( !SQL_SUCCEEDED(rc= MySQLPrepare(hstmt, (SQLCHAR *)buff, (SQLINTEGER)(pos - buff), FALSE)))
    return rc;

  return my_SQLExecute(stmt);
}


/*
  @type    : ODBC 1.0 API
  @purpose : returns a list of tables and the privileges associated with
             each table. The driver returns the information as a result
             set on the specified statement.
*/
SQLRETURN SQL_API
MySQLTablePrivileges(SQLHSTMT hstmt,
                     SQLCHAR *catalog, SQLSMALLINT catalog_len,
                     SQLCHAR *schema __attribute__((unused)),
                     SQLSMALLINT schema_len __attribute__((unused)),
                     SQLCHAR *table, SQLSMALLINT table_len)
{
    STMT     *stmt= (STMT *)hstmt;

    CLEAR_STMT_ERROR(hstmt);
    my_SQLFreeStmt(hstmt,MYSQL_RESET);

    if (catalog_len == SQL_NTS)
      catalog_len= catalog ? (SQLSMALLINT)strlen((char *)catalog) : 0;
    if (table_len == SQL_NTS)
      table_len= table ? (SQLSMALLINT)strlen((char *)table) : 0;

    if (server_has_i_s(stmt->dbc) && !stmt->dbc->ds->no_information_schema)
    {
      /* Since mysql is also the name of the system db, using here i_s prefix to
         distinct functions */
      return i_s_list_table_priv(hstmt, catalog, catalog_len, schema, schema_len,
                                table, table_len);
    }
    else
    {
      return mysql_list_table_priv(hstmt, catalog, catalog_len, schema, schema_len, table, table_len);
    }
}


/*
  @type    : internal
  @purpose : returns a column privileges result, NULL on error
*/
static SQLRETURN i_s_list_column_priv(HSTMT *     hstmt,
                                      SQLCHAR *   catalog,
                                      SQLSMALLINT catalog_len,
                                      SQLCHAR *   schema,
                                      SQLSMALLINT schema_len,
                                      SQLCHAR *   table,
                                      SQLSMALLINT table_len,
                                      SQLCHAR *   column,
                                      SQLSMALLINT column_len)
{
  STMT FAR *stmt=(STMT FAR*) hstmt;
  MYSQL *mysql= &stmt->dbc->mysql;
  /* 3 names theorethically can have all their characters escaped - thus 6*NAME_LEN  */
  char   buff[351+6*NAME_LEN+1], *pos;
  SQLRETURN rc;

  /* Db,User,Table_name,"NULL" as Grantor,Table_priv*/
  pos= strmov(buff,
    "SELECT TABLE_SCHEMA as TABLE_CAT, TABLE_CATALOG as TABLE_SCHEM,"
    "TABLE_NAME, COLUMN_NAME, NULL as GRANTOR, GRANTEE,"
    "PRIVILEGE_TYPE as PRIVILEGE, IS_GRANTABLE "
    "FROM INFORMATION_SCHEMA.COLUMN_PRIVILEGES "
    "WHERE TABLE_NAME");

  if(add_name_condition_oa_id(hstmt, &pos, table, table_len, NULL))
    return set_stmt_error(stmt, "HY009", "Invalid use of NULL pointer(table is required parameter)", 0);

  pos= strmov(pos, " AND TABLE_SCHEMA");
  add_name_condition_oa_id(hstmt, &pos, catalog, catalog_len, "=DATABASE()");


  pos= strmov(pos, " AND COLUMN_NAME");
  add_name_condition_pv_id(hstmt, &pos, column, column_len, " LIKE '%'");


  /* TABLE_CAT is always NULL in mysql I_S */
  pos= strmov(pos, " ORDER BY /*TABLE_CAT,*/ TABLE_SCHEM, TABLE_NAME, COLUMN_NAME, PRIVILEGE");

  assert(pos - buff < sizeof(buff));

  if( !SQL_SUCCEEDED(rc= MySQLPrepare(hstmt, (SQLCHAR *)buff, SQL_NTS, FALSE)))
    return rc;

  return my_SQLExecute(stmt);
}


SQLRETURN SQL_API
MySQLColumnPrivileges(SQLHSTMT hstmt,
                      SQLCHAR *catalog, SQLSMALLINT catalog_len,
                      SQLCHAR *schema __attribute__((unused)),
                      SQLSMALLINT schema_len __attribute__((unused)),
                      SQLCHAR *table, SQLSMALLINT table_len,
                      SQLCHAR *column, SQLSMALLINT column_len)
{
  STMT     *stmt= (STMT *)hstmt;

  CLEAR_STMT_ERROR(hstmt);
  my_SQLFreeStmt(hstmt,MYSQL_RESET);

  if (catalog_len == SQL_NTS)
    catalog_len= catalog ? (SQLSMALLINT)strlen((char *)catalog) : 0;
  if (table_len == SQL_NTS)
    table_len= table ? (SQLSMALLINT)strlen((char *)table) : 0;
  if (column_len == SQL_NTS)
    column_len= column ? (SQLSMALLINT)strlen((char *)column) : 0;

  if (server_has_i_s(stmt->dbc) && !stmt->dbc->ds->no_information_schema)
  {
    /* Since mysql is also the name of the system db, using here i_s prefix to
    distinct functions */
    return i_s_list_column_priv(hstmt, catalog, catalog_len, schema, schema_len,
      table, table_len, column, column_len);
  }
  else
  {
    return mysql_list_column_priv(hstmt, catalog, catalog_len, schema, schema_len,
      table, table_len, column, column_len);
  }
}


/*
****************************************************************************
SQLSpecialColumns
****************************************************************************
*/
SQLRETURN
i_s_special_columns(SQLHSTMT hstmt, SQLUSMALLINT fColType,
                      SQLCHAR *szTableQualifier, SQLSMALLINT cbTableQualifier,
                      SQLCHAR *szTableOwner __attribute__((unused)),
                      SQLSMALLINT cbTableOwner __attribute__((unused)),
                      SQLCHAR *szTableName, SQLSMALLINT cbTableName,
                      SQLUSMALLINT fScope __attribute__((unused)),
                      SQLUSMALLINT fNullable __attribute__((unused)))
{
  /* The function is just a stub. We call non-I_S version of the function before implementing the I_S one */
  return mysql_special_columns(hstmt, fColType, szTableQualifier,
                               cbTableQualifier, szTableOwner, cbTableOwner,
                               szTableName, cbTableName, fScope, fNullable);
}


/*
  @type    : ODBC 1.0 API
  @purpose : retrieves the following information about columns within a
       specified table:
       - The optimal set of columns that uniquely identifies a row
         in the table.
       - Columns that are automatically updated when any value in the
         row is updated by a transaction
*/

SQLRETURN SQL_API
MySQLSpecialColumns(SQLHSTMT hstmt, SQLUSMALLINT fColType,
                    SQLCHAR *szTableQualifier, SQLSMALLINT cbTableQualifier,
                    SQLCHAR *szTableOwner __attribute__((unused)),
                    SQLSMALLINT cbTableOwner __attribute__((unused)),
                    SQLCHAR *szTableName, SQLSMALLINT cbTableName,
                    SQLUSMALLINT fScope __attribute__((unused)),
                    SQLUSMALLINT fNullable __attribute__((unused)))
{
  STMT        *stmt=(STMT *) hstmt;

  CLEAR_STMT_ERROR(hstmt);
  my_SQLFreeStmt(hstmt,MYSQL_RESET);

  if (cbTableQualifier == SQL_NTS)
    cbTableQualifier= szTableQualifier ? (SQLSMALLINT)strlen((char *)szTableQualifier) : 0;
  if (cbTableName == SQL_NTS)
    cbTableName= szTableName ? (SQLSMALLINT)strlen((char *)szTableName) : 0;

  if (server_has_i_s(stmt->dbc) && !stmt->dbc->ds->no_information_schema)
  {
    return i_s_special_columns(hstmt, fColType, szTableQualifier,
                                 cbTableQualifier, szTableOwner, cbTableOwner,
                                 szTableName, cbTableName, fScope, fNullable);
  }
  else
  {
    return mysql_special_columns(hstmt, fColType, szTableQualifier,
                                 cbTableQualifier, szTableOwner, cbTableOwner,
                                 szTableName, cbTableName, fScope, fNullable);
  }
}


/*
****************************************************************************
SQLPrimaryKeys
****************************************************************************
*/
SQLRETURN
i_s_primary_keys(SQLHSTMT hstmt,
                 SQLCHAR *catalog, SQLSMALLINT catalog_len,
                 SQLCHAR *schema __attribute__((unused)),
                 SQLSMALLINT schema_len __attribute__((unused)),
                 SQLCHAR *table, SQLSMALLINT table_len)
{
  /* The function is just a stub. We call non-I_S version of the function before implementing the I_S one */
  return mysql_primary_keys(hstmt, catalog, catalog_len, schema, schema_len,
                            table, table_len);
}


/*
  @type    : ODBC 1.0 API
  @purpose : returns the column names that make up the primary key for a table.
       The driver returns the information as a result set. This function
       does not support returning primary keys from multiple tables in
       a single call
*/

SQLRETURN SQL_API
MySQLPrimaryKeys(SQLHSTMT hstmt,
                 SQLCHAR *catalog, SQLSMALLINT catalog_len,
                 SQLCHAR *schema __attribute__((unused)),
                 SQLSMALLINT schema_len __attribute__((unused)),
                 SQLCHAR *table, SQLSMALLINT table_len)
{
  STMT FAR  *stmt= (STMT FAR*) hstmt;

  CLEAR_STMT_ERROR(hstmt);
  my_SQLFreeStmt(hstmt,MYSQL_RESET);

  if (catalog_len == SQL_NTS)
    catalog_len= catalog ? (SQLSMALLINT)strlen((char *)catalog) : 0;
  if (table_len == SQL_NTS)
    table_len= table ? (SQLSMALLINT)strlen((char *)table) : 0;

  if (server_has_i_s(stmt->dbc) && !stmt->dbc->ds->no_information_schema)
  {
    return i_s_primary_keys(hstmt, catalog, catalog_len, schema, schema_len,
                              table, table_len);
  }
  else
  {
    return mysql_primary_keys(hstmt, catalog, catalog_len, schema, schema_len,
                              table, table_len);
  }
}


/*
****************************************************************************
SQLForeignKeys
****************************************************************************
*/
SQLRETURN i_s_foreign_keys(SQLHSTMT hstmt,
                           SQLCHAR    *szPkCatalogName __attribute__((unused)),
                           SQLSMALLINT cbPkCatalogName __attribute__((unused)),
                           SQLCHAR    *szPkSchemaName __attribute__((unused)),
                           SQLSMALLINT cbPkSchemaName __attribute__((unused)),
                           SQLCHAR    *szPkTableName,
                           SQLSMALLINT cbPkTableName,
                           SQLCHAR    *szFkCatalogName,
                           SQLSMALLINT cbFkCatalogName,
                           SQLCHAR    *szFkSchemaName __attribute__((unused)),
                           SQLSMALLINT cbFkSchemaName __attribute__((unused)),
                           SQLCHAR    *szFkTableName,
                           SQLSMALLINT cbFkTableName)
{
  STMT FAR *stmt=(STMT FAR*) hstmt;
  MYSQL *mysql= &stmt->dbc->mysql;
  char query[2048], *buff; /* This should be big enough. */
  char *update_rule, *delete_rule, *ref_constraints_join;
  SQLRETURN rc;

  /*
     With 5.1, we can use REFERENTIAL_CONSTRAINTS to get even more info.
  */
  if (is_minimum_version(stmt->dbc->mysql.server_version, "5.1"))
  {
    update_rule= "CASE"
                 " WHEN R.UPDATE_RULE = 'CASCADE' THEN 0"
                 " WHEN R.UPDATE_RULE = 'SET NULL' THEN 2"
                 " WHEN R.UPDATE_RULE = 'SET DEFAULT' THEN 4"
                 " WHEN R.UPDATE_RULE = 'SET RESTRICT' THEN 1"
                 " WHEN R.UPDATE_RULE = 'SET NO ACTION' THEN 3"
                 " ELSE 3"
                 " END";
    delete_rule= "CASE"
                 " WHEN R.DELETE_RULE = 'CASCADE' THEN 0"
                 " WHEN R.DELETE_RULE = 'SET NULL' THEN 2"
                 " WHEN R.DELETE_RULE = 'SET DEFAULT' THEN 4"
                 " WHEN R.DELETE_RULE = 'SET RESTRICT' THEN 1"
                 " WHEN R.DELETE_RULE = 'SET NO ACTION' THEN 3"
                 " ELSE 3"
                 " END";

    ref_constraints_join=
      " JOIN INFORMATION_SCHEMA.REFERENTIAL_CONSTRAINTS R"
      " ON (R.CONSTRAINT_NAME = A.CONSTRAINT_NAME"
      " AND R.TABLE_NAME = A.TABLE_NAME"
      " AND R.CONSTRAINT_SCHEMA = A.TABLE_SCHEMA)";
  }
  else
  {
    /* Just return '1' to be compatible with pre-I_S version. */
    update_rule= delete_rule= "1";
    ref_constraints_join= "";
  }

  /* This is a big, ugly query. But it works! */
  buff= strxmov(query,
                "SELECT A.REFERENCED_TABLE_SCHEMA AS PKTABLE_CAT,"
                "NULL AS PKTABLE_SCHEM,"
                "A.REFERENCED_TABLE_NAME AS PKTABLE_NAME,"
                "A.REFERENCED_COLUMN_NAME AS PKCOLUMN_NAME,"
                "A.TABLE_SCHEMA AS FKTABLE_CAT, NULL AS FKTABLE_SCHEM,"
                "A.TABLE_NAME AS FKTABLE_NAME,"
                "A.COLUMN_NAME AS FKCOLUMN_NAME,"
                "A.ORDINAL_POSITION AS KEY_SEQ,",
                update_rule, " AS UPDATE_RULE,",
                delete_rule, " AS DELETE_RULE,"
                "A.CONSTRAINT_NAME AS FK_NAME,"
                "'PRIMARY' AS PK_NAME,"
                "7 AS DEFERRABILITY"
                " FROM INFORMATION_SCHEMA.KEY_COLUMN_USAGE A"
                " JOIN INFORMATION_SCHEMA.KEY_COLUMN_USAGE D"
                " ON (D.TABLE_SCHEMA=A.REFERENCED_TABLE_SCHEMA AND D.TABLE_NAME=A.REFERENCED_TABLE_NAME"
                    " AND D.COLUMN_NAME=A.REFERENCED_COLUMN_NAME)",
                ref_constraints_join,
                " WHERE D.CONSTRAINT_NAME='PRIMARY' ",
                NullS);

  if (szPkTableName && szPkTableName[0])
  {
    buff= strmov(buff, "AND A.REFERENCED_TABLE_SCHEMA = ");
    if (szPkCatalogName && szPkCatalogName[0])
    {
      buff= strmov(buff, "'");
      buff+= mysql_real_escape_string(mysql, buff, (char *)szPkCatalogName,
                                      cbPkCatalogName);
      buff= strmov(buff, "' ");
    }
    else
    {
      buff= strmov(buff, "DATABASE() ");
    }

    buff= strmov(buff, "AND A.REFERENCED_TABLE_NAME = '");

    buff+= mysql_real_escape_string(mysql, buff, (char *)szPkTableName,
                                    cbPkTableName);
    buff= strmov(buff, "' ");

    strmov(buff, "ORDER BY PKTABLE_CAT, PKTABLE_NAME, "
                 "KEY_SEQ, FKTABLE_NAME");
  }

  if (szFkTableName && szFkTableName[0])
  {
    buff= strmov(buff, "AND A.TABLE_SCHEMA = ");

    if (szFkCatalogName && szFkCatalogName[0])
    {
      buff= strmov(buff, "'");
      buff+= mysql_real_escape_string(mysql, buff, (char *)szFkCatalogName,
                                      cbFkCatalogName);
      buff= strmov(buff, "' ");
    }
    else
    {
      buff= strmov(buff, "DATABASE() ");
    }

    buff= strmov(buff, "AND A.TABLE_NAME = '");

    buff+= mysql_real_escape_string(mysql, buff, (char *)szFkTableName,
                                    cbFkTableName);
    buff= strmov(buff, "' ");

    buff= strmov(buff, "ORDER BY FKTABLE_CAT, FKTABLE_NAME, "
                 "KEY_SEQ, PKTABLE_NAME");
  }

  assert(buff - query < sizeof(query));

  rc= MySQLPrepare(hstmt, (SQLCHAR *)query, (SQLINTEGER)(buff - query), FALSE);

  if (!SQL_SUCCEEDED(rc))
    return rc;

  return my_SQLExecute(hstmt);
}
/**
  Retrieve either a list of foreign keys in a specified table, or the list
  of foreign keys in other tables that refer to the primary key in the
  specified table. (We currently only support the former, not the latter.)

  @param[in] hstmt           Handle of statement
  @param[in] szPkCatalogName Catalog (database) of table with primary key that
                             we want to see foreign keys for
  @param[in] cbPkCatalogName Length of @a szPkCatalogName
  @param[in] szPkSchemaName  Schema of table with primary key that we want to
                             see foreign keys for (unused)
  @param[in] cbPkSchemaName  Length of @a szPkSchemaName
  @param[in] szPkTableName   Table with primary key that we want to see foreign
                             keys for
  @param[in] cbPkTableName   Length of @a szPkTableName
  @param[in] szFkCatalogName Catalog (database) of table with foreign keys we
                             are interested in
  @param[in] cbFkCatalogName Length of @a szFkCatalogName
  @param[in] szFkSchemaName  Schema of table with foreign keys we are
                             interested in
  @param[in] cbFkSchemaName  Length of szFkSchemaName
  @param[in] szFkTableName   Table with foreign keys we are interested in
  @param[in] cbFkTableName   Length of @a szFkTableName

  @return SQL_SUCCESS

  @since ODBC 1.0
*/
SQLRETURN SQL_API
MySQLForeignKeys(SQLHSTMT hstmt,
                 SQLCHAR *szPkCatalogName __attribute__((unused)),
                 SQLSMALLINT cbPkCatalogName __attribute__((unused)),
                 SQLCHAR *szPkSchemaName __attribute__((unused)),
                 SQLSMALLINT cbPkSchemaName __attribute__((unused)),
                 SQLCHAR *szPkTableName, SQLSMALLINT cbPkTableName,
                 SQLCHAR *szFkCatalogName, SQLSMALLINT cbFkCatalogName,
                 SQLCHAR *szFkSchemaName __attribute__((unused)),
                 SQLSMALLINT cbFkSchemaName __attribute__((unused)),
                 SQLCHAR *szFkTableName, SQLSMALLINT cbFkTableName)
{
    STMT FAR *stmt=(STMT FAR*) hstmt;

    CLEAR_STMT_ERROR(hstmt);
    my_SQLFreeStmt(hstmt,MYSQL_RESET);

    if (cbPkTableName == SQL_NTS)
      cbPkTableName= szPkTableName != NULL ? (SQLSMALLINT)strlen((char *)szPkTableName) : 0;

    if (cbPkCatalogName == SQL_NTS)
      cbPkCatalogName= szPkCatalogName != NULL ? (SQLSMALLINT)strlen((char *)szPkCatalogName) : 0;

    if (cbFkCatalogName == SQL_NTS)
      cbFkCatalogName= szFkCatalogName != NULL ? (SQLSMALLINT)strlen((char *)szFkCatalogName) : 0;

    if (cbFkTableName == SQL_NTS)
      cbFkTableName= szFkTableName != NULL ? (SQLSMALLINT)strlen((char *)szFkTableName) : 0;

    if (server_has_i_s(stmt->dbc) && !stmt->dbc->ds->no_information_schema)
    {
      return i_s_foreign_keys(hstmt, szPkCatalogName, cbPkCatalogName, szPkSchemaName,
                              cbPkSchemaName, szPkTableName, cbPkTableName, szFkCatalogName,
                              cbFkCatalogName, szFkSchemaName, cbFkSchemaName,
                              szFkTableName, cbFkTableName);
    }
    /* For 3.23 and later, use comment in SHOW TABLE STATUS (yuck). */
    else /* We wouldn't get here if we had server version under 3.23 */
    {
      return mysql_foreign_keys(hstmt, szPkCatalogName, cbPkCatalogName, szPkSchemaName,
                              cbPkSchemaName, szPkTableName, cbPkTableName, szFkCatalogName,
                              cbFkCatalogName, szFkSchemaName, cbFkSchemaName,
                              szFkTableName, cbFkTableName);
    }
}

/*
****************************************************************************
SQLProcedures and SQLProcedureColumns
****************************************************************************
*/

/**
  Get the list of procedures stored in a catalog (database). This is done by
  generating the appropriate query against INFORMATION_SCHEMA. If no
  database is specified, the current database is used.

  @param[in] hstmt            Handle of statement
  @param[in] szCatalogName    Name of catalog (database)
  @param[in] cbCatalogName    Length of catalog
  @param[in] szSchemaName     Pattern of schema (unused)
  @param[in] cbSchemaName     Length of schema name
  @param[in] szProcName       Pattern of procedure names to fetch
  @param[in] cbProcName       Length of procedure name
*/
SQLRETURN SQL_API
MySQLProcedures(SQLHSTMT hstmt,
                SQLCHAR *szCatalogName, SQLSMALLINT cbCatalogName,
                SQLCHAR *szSchemaName __attribute__((unused)),
                SQLSMALLINT cbSchemaName __attribute__((unused)),
                SQLCHAR *szProcName, SQLSMALLINT cbProcName)
{
  SQLRETURN rc;
  STMT *stmt= (STMT *)hstmt;

  CLEAR_STMT_ERROR(hstmt);
  my_SQLFreeStmt(hstmt,MYSQL_RESET);

  /* If earlier than 5.0, the server doesn't even support stored procs. */
  if (!server_has_i_s(stmt->dbc))
  {
    /*
      We use the server to generate a fake result with no rows, but
      reasonable column information.
    */
    if ((rc= MySQLPrepare(hstmt, (SQLCHAR *)"SELECT "
                          "'' AS PROCEDURE_CAT,"
                          "'' AS PROCEDURE_SCHEM,"
                          "'' AS PROCEDURE_NAME,"
                          "NULL AS NUM_INPUT_PARAMS,"
                          "NULL AS NUM_OUTPUT_PARAMS,"
                          "NULL AS NUM_RESULT_SETS,"
                          "'' AS REMARKS,"
                          "0 AS PROCEDURE_TYPE "
                          "FROM DUAL WHERE 1=0", SQL_NTS, FALSE)))
      return rc;

    return my_SQLExecute(hstmt);
  }

  /*
    If a catalog (database) was specified, we use that, otherwise we
    look up procedures from the current database. (This is not standard
    behavior, but seems useful.)
  */
  if (szCatalogName && szProcName)
    rc= MySQLPrepare(hstmt, (SQLCHAR *)
                     "SELECT ROUTINE_SCHEMA AS PROCEDURE_CAT,"
                     "NULL AS PROCEDURE_SCHEM,"
                     "ROUTINE_NAME AS PROCEDURE_NAME,"
                     "NULL AS NUM_INPUT_PARAMS,"
                     "NULL AS NUM_OUTPUT_PARAMS,"
                     "NULL AS NUM_RESULT_SETS,"
                     "ROUTINE_COMMENT AS REMARKS,"
                     "IF(ROUTINE_TYPE = 'FUNCTION', 2,"
                       "IF(ROUTINE_TYPE= 'PROCEDURE', 1, 0)) AS PROCEDURE_TYPE"
                     "  FROM INFORMATION_SCHEMA.ROUTINES"
                     " WHERE ROUTINE_NAME LIKE ? AND ROUTINE_SCHEMA = ?",
                     SQL_NTS, FALSE);
  else if (szProcName)
    rc= MySQLPrepare(hstmt, (SQLCHAR *)
                     "SELECT ROUTINE_SCHEMA AS PROCEDURE_CAT,"
                     "NULL AS PROCEDURE_SCHEM,"
                     "ROUTINE_NAME AS PROCEDURE_NAME,"
                     "NULL AS NUM_INPUT_PARAMS,"
                     "NULL AS NUM_OUTPUT_PARAMS,"
                     "NULL AS NUM_RESULT_SETS,"
                     "ROUTINE_COMMENT AS REMARKS,"
                     "IF(ROUTINE_TYPE = 'FUNCTION', 2,"
                       "IF(ROUTINE_TYPE= 'PROCEDURE', 1, 0)) AS PROCEDURE_TYPE"
                     "  FROM INFORMATION_SCHEMA.ROUTINES"
                     " WHERE ROUTINE_NAME LIKE ?"
                     " AND ROUTINE_SCHEMA = DATABASE()",
                     SQL_NTS, FALSE);
  else
    rc= MySQLPrepare(hstmt, (SQLCHAR *)
                     "SELECT ROUTINE_SCHEMA AS PROCEDURE_CAT,"
                     "NULL AS PROCEDURE_SCHEM,"
                     "ROUTINE_NAME AS PROCEDURE_NAME,"
                     "NULL AS NUM_INPUT_PARAMS,"
                     "NULL AS NUM_OUTPUT_PARAMS,"
                     "NULL AS NUM_RESULT_SETS,"
                     "ROUTINE_COMMENT AS REMARKS,"
                     "IF(ROUTINE_TYPE = 'FUNCTION', 2,"
                       "IF(ROUTINE_TYPE= 'PROCEDURE', 1, 0)) AS PROCEDURE_TYPE"
                     " FROM INFORMATION_SCHEMA.ROUTINES"
                     " WHERE ROUTINE_SCHEMA = DATABASE()",
                     SQL_NTS, FALSE);
  if (!SQL_SUCCEEDED(rc))
    return rc;

  if (szProcName)
  {
    if (cbProcName == SQL_NTS)
      cbProcName= (SQLSMALLINT)strlen((const char *)szProcName);
    rc= my_SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_C_CHAR,
                            0, 0, szProcName, cbProcName, NULL);
    if (!SQL_SUCCEEDED(rc))
      return rc;
  }

  if (szCatalogName)
  {
    if (cbCatalogName == SQL_NTS)
      cbCatalogName= (SQLSMALLINT)strlen((const char *)szCatalogName);
    rc= my_SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR,
                            SQL_C_CHAR, 0, 0, szCatalogName, cbCatalogName,
                            NULL);
    if (!SQL_SUCCEEDED(rc))
      return rc;
  }

  return my_SQLExecute(hstmt);
}


/*
****************************************************************************
SQLProcedure Columns
****************************************************************************
*/

/*
  @purpose : returns the list of input and output parameters, as well as
  the columns that make up the result set for the specified
  procedures. The driver returns the information as a result
  set on the specified statement
*/
SQLRETURN
i_s_procedure_columns(SQLHSTMT hstmt,
                      SQLCHAR *szCatalogName, SQLSMALLINT cbCatalogName,
                      SQLCHAR *szSchemaName __attribute__((unused)),
                      SQLSMALLINT cbSchemaName __attribute__((unused)),
                      SQLCHAR *szProcName, SQLSMALLINT cbProcName,
                      SQLCHAR *szColumnName, SQLSMALLINT cbColumnName)
{
  /* The function is just a stub. We call non-I_S version of the function before implementing the I_S one */
  return mysql_procedure_columns(hstmt, szCatalogName, cbCatalogName, szSchemaName,
                                 cbSchemaName, szProcName, cbProcName, szColumnName,
                                 cbColumnName);
}


/*
  @type    : ODBC 1.0 API
  @purpose : returns the list of input and output parameters, as well as
  the columns that make up the result set for the specified
  procedures. The driver returns the information as a result
  set on the specified statement
*/

SQLRETURN SQL_API
MySQLProcedureColumns(SQLHSTMT hstmt,
                    SQLCHAR *szCatalogName, SQLSMALLINT cbCatalogName,
                    SQLCHAR *szSchemaName __attribute__((unused)),
                    SQLSMALLINT cbSchemaName __attribute__((unused)),
                    SQLCHAR *szProcName, SQLSMALLINT cbProcName,
                    SQLCHAR *szColumnName, SQLSMALLINT cbColumnName)
{
  STMT *stmt= (STMT *)hstmt;

  CLEAR_STMT_ERROR(hstmt);
  my_SQLFreeStmt(hstmt,MYSQL_RESET);

  if (cbCatalogName == SQL_NTS)
    cbCatalogName= szCatalogName ? (SQLSMALLINT)strlen((char *)szCatalogName) : 0;

  if (cbProcName == SQL_NTS)
    cbProcName= szProcName ? (SQLSMALLINT)strlen((char *)szProcName) : 0;

  if (cbColumnName == SQL_NTS)
    cbColumnName= szColumnName ? (SQLSMALLINT)strlen((char *)szColumnName) : 0;

  if (server_has_i_s(stmt->dbc) && !stmt->dbc->ds->no_information_schema)
  {
    return i_s_procedure_columns(hstmt, szCatalogName, cbCatalogName, szSchemaName,
                                 cbSchemaName, szProcName, cbProcName, szColumnName,
                                 cbColumnName);
  }
  else
  {
    return mysql_procedure_columns(hstmt, szCatalogName, cbCatalogName, szSchemaName,
                                 cbSchemaName, szProcName, cbProcName, szColumnName,
                                 cbColumnName);
  }
}
