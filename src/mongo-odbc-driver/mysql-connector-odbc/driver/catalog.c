/*
  Copyright (c) 2000, 2017, Oracle and/or its affiliates. All rights reserved.

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
my_bool server_has_i_s(DBC *dbc)
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
                          const char **token, 
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
  free_internal_result_buffers(stmt);
  stmt->result= (MYSQL_RES*) myodbc_malloc(sizeof(MYSQL_RES), MYF(MY_ZEROFILL));
  stmt->result_array= (MYSQL_ROW)myodbc_memdup((char *)rowval, rowsize, MYF(0));
  if (!(stmt->result && stmt->result_array))
  {
    x_free(stmt->result);
    x_free(stmt->result_array);

    set_mem_error(&stmt->dbc->mysql);
    return handle_connection_error(stmt);
  }
  stmt->fake_result= 1;

  set_row_count(stmt, rowcnt);

  myodbc_link_fields(stmt, fields, fldcnt);

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
  @param[in] catalog_name   Catalog (database) of table, @c NULL for current
  @param[in] catalog_len    Length of catalog name
  @param[in] table_name     Name of table
  @param[in] table_len      Length of table name
  @param[in] wildcard       Whether the table name is a wildcard

  @return Result of SHOW TABLE STATUS, or NULL if there is an error
          or empty result (check mysql_errno(&stmt->dbc->mysql) != 0)
*/
static MYSQL_RES *table_status_i_s(STMT        *stmt,
                                         SQLCHAR     *catalog_name,
                                         SQLSMALLINT  catalog_len,
                                         SQLCHAR     *table_name,
                                         SQLSMALLINT  table_len,
                                         my_bool      wildcard,
                                         my_bool      show_tables,
                                         my_bool      show_views)
{
  MYSQL *mysql= &stmt->dbc->mysql;
  /** the buffer size should count possible escapes */
  char buff[300+8*NAME_CHAR_LEN], *to;
  my_bool clause_added= FALSE;

  to= myodbc_stpmov(buff, "SELECT TABLE_NAME, TABLE_COMMENT, TABLE_TYPE, TABLE_SCHEMA " \
                      "FROM ( SELECT * FROM INFORMATION_SCHEMA.TABLES  " \
                      "WHERE ");

  if (catalog_name && *catalog_name)
  {
    to= myodbc_stpmov(to, "TABLE_SCHEMA LIKE '");
    to+= myodbc_escape_string(mysql, to, (ulong)(sizeof(buff) - (to - buff)),
                              (char *)catalog_name, catalog_len, 1);
    to= myodbc_stpmov(to, "' ");
    clause_added= TRUE;
  }
  else
  {
    to= myodbc_stpmov(to, "TABLE_SCHEMA = DATABASE() ");
  }

  if (show_tables)
  {
    to= myodbc_stpmov(to, "AND ");
    if (show_views)
      to= myodbc_stpmov(to, "( ");
    to= myodbc_stpmov(to, "TABLE_TYPE='BASE TABLE' ");
  }

  if (show_views)
  {
    if (show_tables)
      to= myodbc_stpmov(to, "OR ");
    else
      to= myodbc_stpmov(to, "AND ");

    to= myodbc_stpmov(to, "TABLE_TYPE='VIEW' ");
    if (show_tables)
      to= myodbc_stpmov(to, ") ");
  }
  to = myodbc_stpmov(to, ") TABLES ");

  /*
    As a pattern-value argument, an empty string needs to be treated
    literally. (It's not the same as NULL, which is the same as '%'.)
    But it will never match anything, so bail out now.
  */
  if (table_name && wildcard && !*table_name)
    return NULL;

  if (table_name && *table_name)
  {
    to= myodbc_stpmov(to, "WHERE TABLE_NAME LIKE '");
    if (wildcard)
    {
      to+= mysql_real_escape_string(mysql, to, (char *)table_name, table_len);
    }
    else
    {
      to+= myodbc_escape_string(mysql, to, (ulong)(sizeof(buff) - (to - buff)),
                                (char *)table_name, table_len, 0);
    }
    to= myodbc_stpmov(to, "'");
  }

  assert(to - buff < sizeof(buff));

  MYLOG_QUERY(stmt, buff);

  if (exec_stmt_query(stmt, buff, (unsigned long)(to - buff), FALSE))
  {
    return NULL;
  }

  return mysql_store_result(mysql);
}


/**
  Get the table status for a table or tables. Lengths may not be SQL_NTS.

  @param[in] stmt           Handle to statement
  @param[in] catalog_name   Catalog (database) of table, @c NULL for current
  @param[in] catalog_len    Length of catalog name
  @param[in] table_name     Name of table
  @param[in] table_len      Length of table name
  @param[in] wildcard       Whether the table name is a wildcard

  @return Result of SHOW TABLE STATUS, or NULL if there is an error
          or empty result (check mysql_errno(&stmt->dbc->mysql) != 0)
*/
MYSQL_RES *table_status(STMT        *stmt,
                        SQLCHAR     *catalog_name,
                        SQLSMALLINT  catalog_len,
                        SQLCHAR     *table_name,
                        SQLSMALLINT  table_len,
                        my_bool      wildcard,
                        my_bool      show_tables,
                        my_bool      show_views)
{
  if (server_has_i_s(stmt->dbc) && !stmt->dbc->ds->no_information_schema)
    return table_status_i_s(stmt, catalog_name, catalog_len,
                                  table_name, table_len, wildcard,
                                             show_tables, show_views);
  else
    return table_status_no_i_s(stmt, catalog_name, catalog_len,
                                   table_name, table_len, wildcard);
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
    STMT *stmt= (STMT *) hstmt;

    if (metadata_id)
    {
      *pos= myodbc_stpmov(*pos, "=");
      /* Need also code to remove trailing blanks */
    }
    else
      *pos= myodbc_stpmov(*pos, "= BINARY ");

    *pos= myodbc_stpmov(*pos, "'");
    *pos+= mysql_real_escape_string(&stmt->dbc->mysql, *pos, (char *)name, name_len);
    *pos= myodbc_stpmov(*pos, "' ");
  }
  else
  {
    /* According to http://msdn.microsoft.com/en-us/library/ms714579%28VS.85%29.aspx
    identifier argument cannot be NULL with one exception not actual for mysql) */
    if (!metadata_id && _default)
      *pos= myodbc_stpmov(*pos, _default); 
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
    STMT *stmt= (STMT *) hstmt;

    if (metadata_id)
    {
      *pos= myodbc_stpmov(*pos, "=");
      /* Need also code to remove trailing blanks */
    }
    else
      *pos= myodbc_stpmov(*pos, " LIKE BINARY ");

    *pos= myodbc_stpmov(*pos, "'");
    *pos+= mysql_real_escape_string(&stmt->dbc->mysql, *pos, (char *)name, name_len);
    *pos= myodbc_stpmov(*pos, "' ");
  }
  else
  {
    /* According to http://msdn.microsoft.com/en-us/library/ms714579%28VS.85%29.aspx
       identifier argument cannot be NULL with one exception not actual for mysql) */
    if (!metadata_id && _default)
      *pos= myodbc_stpmov(*pos, _default); 
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
tables_i_s(SQLHSTMT hstmt,
           SQLCHAR *catalog_name, SQLSMALLINT catalog_len,
           SQLCHAR *schema_name, SQLSMALLINT schema_len,
           SQLCHAR *table_name, SQLSMALLINT table_len,
           SQLCHAR *type_name, SQLSMALLINT type_len)
{
  /* The function is just a stub. We call non-I_S version of the function before implementing the I_S one */
  return tables_no_i_s(hstmt, catalog_name, catalog_len, schema_name, schema_len,
                       table_name, table_len, type_name, type_len);
}


SQLRETURN SQL_API
MySQLTables(SQLHSTMT hstmt,
            SQLCHAR *catalog_name, SQLSMALLINT catalog_len,
            SQLCHAR *schema_name, SQLSMALLINT schema_len,
            SQLCHAR *table_name, SQLSMALLINT table_len,
            SQLCHAR *type_name, SQLSMALLINT type_len)
{
  STMT *stmt= (STMT *)hstmt;

  CLEAR_STMT_ERROR(hstmt);
  my_SQLFreeStmt(hstmt, MYSQL_RESET);

  GET_NAME_LEN(stmt, catalog_name, catalog_len);
  GET_NAME_LEN(stmt, schema_name, schema_len);
  GET_NAME_LEN(stmt, table_name, table_len);
  GET_NAME_LEN(stmt, type_name, type_len);

  if (server_has_i_s(stmt->dbc) && !stmt->dbc->ds->no_information_schema)
  {
    return tables_i_s(hstmt, catalog_name, catalog_len, schema_name, schema_len,
                      table_name, table_len, type_name, type_len);
  }
  else
  {
    return tables_no_i_s(hstmt, catalog_name, catalog_len, schema_name, schema_len,
                         table_name, table_len, type_name, type_len);
  }
}


/*
****************************************************************************
SQLColumns
****************************************************************************
*/
/**
  Get information about the columns in one or more tables.

  @param[in] hstmt           Handle of statement
  @param[in] catalog_name    Name of catalog (database)
  @param[in] catalog_len     Length of catalog
  @param[in] schema_name     Name of schema (unused)
  @param[in] schema_len      Length of schema name
  @param[in] table_name      Pattern of table names to match
  @param[in] table_len       Length of table pattern
  @param[in] column_name     Pattern of column names to match
  @param[in] column_len      Length of column pattern
*/
SQLRETURN
columns_i_s(SQLHSTMT hstmt, SQLCHAR *catalog_name, SQLSMALLINT catalog_len,
            SQLCHAR *schema_name __attribute__((unused)),
            SQLSMALLINT schema_len __attribute__((unused)),
            SQLCHAR *table_name, SQLSMALLINT table_len,
            SQLCHAR *column_name, SQLSMALLINT column_len)

{
  /* The function is just a stub. We call non-I_S version of the function before implementing the I_S one */
  return columns_no_i_s(hstmt, catalog_name, catalog_len,schema_name, schema_len,
                        table_name, table_len, column_name, column_len);
}


/**
  Get information about the columns in one or more tables.

  @param[in] hstmt            Handle of statement
  @param[in] catalog_name     Name of catalog (database)
  @param[in] catalog_len      Length of catalog
  @param[in] schema_name      Name of schema (unused)
  @param[in] schema_len       Length of schema name
  @param[in] table_name       Pattern of table names to match
  @param[in] table_len        Length of table pattern
  @param[in] column_name      Pattern of column names to match
  @param[in] column_len       Length of column pattern
*/
SQLRETURN SQL_API
MySQLColumns(SQLHSTMT hstmt, SQLCHAR *catalog_name, SQLSMALLINT catalog_len,
             SQLCHAR *schema_name __attribute__((unused)),
             SQLSMALLINT schema_len __attribute__((unused)),
             SQLCHAR *table_name, SQLSMALLINT table_len,
             SQLCHAR *column_name, SQLSMALLINT column_len)

{
  STMT *stmt= (STMT *)hstmt;

  CLEAR_STMT_ERROR(hstmt);
  my_SQLFreeStmt(hstmt, MYSQL_RESET);

  GET_NAME_LEN(stmt, catalog_name, catalog_len);
  GET_NAME_LEN(stmt, schema_name, schema_len);
  GET_NAME_LEN(stmt, table_name, table_len);
  GET_NAME_LEN(stmt, column_name, column_len);

  if (server_has_i_s(stmt->dbc) && !stmt->dbc->ds->no_information_schema)
  {
    return columns_i_s(hstmt, catalog_name, catalog_len,schema_name, schema_len,
                       table_name, table_len, column_name, column_len);
  }
  else
  {
    return columns_no_i_s(hstmt, catalog_name, catalog_len,schema_name, schema_len,
                          table_name, table_len, column_name, column_len);
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
statistics_i_s(SQLHSTMT hstmt,
               SQLCHAR *catalog_name, SQLSMALLINT catalog_len,
               SQLCHAR *schema_name __attribute__((unused)),
               SQLSMALLINT schema_len __attribute__((unused)),
               SQLCHAR *table_name, SQLSMALLINT table_len,
               SQLUSMALLINT fUnique,
               SQLUSMALLINT fAccuracy __attribute__((unused)))
{
  /* The function is just a stub. We call non-I_S version of the function before implementing the I_S one */
  return statistics_no_i_s(hstmt, catalog_name, catalog_len, schema_name, schema_len,
                           table_name, table_len, fUnique, fAccuracy);
}


/*
  @type    : ODBC 1.0 API
  @purpose : retrieves a list of statistics about a single table and the
       indexes associated with the table. The driver returns the
       information as a result set.
*/

SQLRETURN SQL_API
MySQLStatistics(SQLHSTMT hstmt,
                SQLCHAR *catalog_name, SQLSMALLINT catalog_len,
                SQLCHAR *schema_name __attribute__((unused)),
                SQLSMALLINT schema_len __attribute__((unused)),
                SQLCHAR *table_name, SQLSMALLINT table_len,
                SQLUSMALLINT fUnique,
                SQLUSMALLINT fAccuracy __attribute__((unused)))
{
  STMT *stmt= (STMT *)hstmt;

  CLEAR_STMT_ERROR(hstmt);
  my_SQLFreeStmt(hstmt,MYSQL_RESET);

  GET_NAME_LEN(stmt, catalog_name, catalog_len);
  GET_NAME_LEN(stmt, schema_name, schema_len);
  GET_NAME_LEN(stmt, table_name, table_len);

  if (server_has_i_s(stmt->dbc) && !stmt->dbc->ds->no_information_schema)
  {
    return statistics_i_s(hstmt, catalog_name, catalog_len, schema_name, schema_len,
                          table_name, table_len, fUnique, fAccuracy);
  }
  else
  {
    return statistics_no_i_s(hstmt, catalog_name, catalog_len, schema_name, schema_len,
                             table_name, table_len, fUnique, fAccuracy);
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
SQLRETURN list_table_priv_i_s(SQLHSTMT    hstmt,
                              SQLCHAR *   catalog_name,
                              SQLSMALLINT catalog_len,
                              SQLCHAR *   schema_name __attribute__((unused)),
                              SQLSMALLINT schema_len  __attribute__((unused)),
                              SQLCHAR *   table_name,
                              SQLSMALLINT table_len)
{
  STMT *stmt=(STMT *) hstmt;
  MYSQL *mysql= &stmt->dbc->mysql;
  char   buff[300+6*NAME_LEN+1], *pos;
  SQLRETURN rc;

  /* Db,User,Table_name,"NULL" as Grantor,Table_priv*/
  pos= myodbc_stpmov(buff,
               "SELECT TABLE_SCHEMA as TABLE_CAT, TABLE_CATALOG as TABLE_SCHEM,"
                      "TABLE_NAME, NULL as GRANTOR, GRANTEE,"
                      "PRIVILEGE_TYPE as PRIVILEGE, IS_GRANTABLE "
               "FROM INFORMATION_SCHEMA.TABLE_PRIVILEGES "
               "WHERE TABLE_NAME");

  add_name_condition_pv_id(hstmt, &pos, table_name, table_len, " LIKE '%'" );

  pos= myodbc_stpmov(pos, " AND TABLE_SCHEMA");
  add_name_condition_oa_id(hstmt, &pos, catalog_name, catalog_len, "=DATABASE()");

  /* TABLE_CAT is always NULL in mysql I_S */
  pos= myodbc_stpmov(pos, " ORDER BY /*TABLE_CAT,*/ TABLE_SCHEM, TABLE_NAME, PRIVILEGE, GRANTEE");

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
                     SQLCHAR *catalog_name, SQLSMALLINT catalog_len,
                     SQLCHAR *schema_name __attribute__((unused)),
                     SQLSMALLINT schema_len __attribute__((unused)),
                     SQLCHAR *table_name, SQLSMALLINT table_len)
{
    STMT     *stmt= (STMT *)hstmt;

    CLEAR_STMT_ERROR(hstmt);
    my_SQLFreeStmt(hstmt,MYSQL_RESET);

    GET_NAME_LEN(stmt, catalog_name, catalog_len);
    GET_NAME_LEN(stmt, schema_name, schema_len);
    GET_NAME_LEN(stmt, table_name, table_len);

    if (server_has_i_s(stmt->dbc) && !stmt->dbc->ds->no_information_schema)
    {
      /* Since mysql is also the name of the system db, using here i_s prefix to
         distinct functions */
      return list_table_priv_i_s(hstmt, catalog_name, catalog_len, schema_name, schema_len,
                                 table_name, table_len);
    }
    else
    {
      return list_table_priv_no_i_s(hstmt, catalog_name, catalog_len, schema_name, schema_len, table_name, table_len);
    }
}


/*
  @type    : internal
  @purpose : returns a column privileges result, NULL on error
*/
static SQLRETURN list_column_priv_i_s(HSTMT *     hstmt,
                                      SQLCHAR *   catalog_name,
                                      SQLSMALLINT catalog_len,
                                      SQLCHAR *   schema_name,
                                      SQLSMALLINT schema_len,
                                      SQLCHAR *   table_name,
                                      SQLSMALLINT table_len,
                                      SQLCHAR *   column_name,
                                      SQLSMALLINT column_len)
{
  STMT *stmt=(STMT *) hstmt;
  MYSQL *mysql= &stmt->dbc->mysql;
  /* 3 names theorethically can have all their characters escaped - thus 6*NAME_LEN  */
  char   buff[400+6*NAME_LEN+1], *pos;
  SQLRETURN rc;

  /* Db,User,Table_name,"NULL" as Grantor,Table_priv*/
  pos= myodbc_stpmov(buff,
    "SELECT TABLE_SCHEMA as TABLE_CAT, TABLE_CATALOG as TABLE_SCHEM,"
    "TABLE_NAME, COLUMN_NAME, NULL as GRANTOR, GRANTEE,"
    "PRIVILEGE_TYPE as PRIVILEGE, IS_GRANTABLE "
    "FROM INFORMATION_SCHEMA.COLUMN_PRIVILEGES "
    "WHERE TABLE_NAME");

  if(add_name_condition_oa_id(hstmt, &pos, table_name, table_len, NULL))
    return set_stmt_error(stmt, "HY009", "Invalid use of NULL pointer(table is required parameter)", 0);

  pos= myodbc_stpmov(pos, " AND TABLE_SCHEMA");
  add_name_condition_oa_id(hstmt, &pos, catalog_name, catalog_len, "=DATABASE()");


  pos= myodbc_stpmov(pos, " AND COLUMN_NAME");
  add_name_condition_pv_id(hstmt, &pos, column_name, column_len, " LIKE '%'");


  /* TABLE_CAT is always NULL in mysql I_S */
  pos= myodbc_stpmov(pos, " ORDER BY /*TABLE_CAT,*/ TABLE_SCHEM, TABLE_NAME, COLUMN_NAME, PRIVILEGE");

  assert(pos - buff < sizeof(buff));

  if( !SQL_SUCCEEDED(rc= MySQLPrepare(hstmt, (SQLCHAR *)buff, SQL_NTS, FALSE)))
    return rc;

  return my_SQLExecute(stmt);
}


SQLRETURN SQL_API
MySQLColumnPrivileges(SQLHSTMT hstmt,
                      SQLCHAR *catalog_name, SQLSMALLINT catalog_len,
                      SQLCHAR *schema_name __attribute__((unused)),
                      SQLSMALLINT schema_len __attribute__((unused)),
                      SQLCHAR *table_name, SQLSMALLINT table_len,
                      SQLCHAR *column_name, SQLSMALLINT column_len)
{
  STMT     *stmt= (STMT *)hstmt;

  CLEAR_STMT_ERROR(hstmt);
  my_SQLFreeStmt(hstmt,MYSQL_RESET);

  GET_NAME_LEN(stmt, catalog_name, catalog_len);
  GET_NAME_LEN(stmt, schema_name, schema_len);
  GET_NAME_LEN(stmt, table_name, table_len);
  GET_NAME_LEN(stmt, column_name, column_len);

  if (server_has_i_s(stmt->dbc) && !stmt->dbc->ds->no_information_schema)
  {
    /* Since mysql is also the name of the system db, using here i_s prefix to
    distinct functions */
    return list_column_priv_i_s(hstmt, catalog_name, catalog_len, schema_name, schema_len,
      table_name, table_len, column_name, column_len);
  }
  else
  {
    return list_column_priv_no_i_s(hstmt, catalog_name, catalog_len, schema_name, schema_len,
      table_name, table_len, column_name, column_len);
  }
}


/*
****************************************************************************
SQLSpecialColumns
****************************************************************************
*/
SQLRETURN
special_columns_i_s(SQLHSTMT hstmt, SQLUSMALLINT fColType,
                    SQLCHAR *table_qualifier, SQLSMALLINT table_qualifier_len,
                    SQLCHAR *table_owner __attribute__((unused)),
                    SQLSMALLINT table_owner_len __attribute__((unused)),
                    SQLCHAR *table_name, SQLSMALLINT table_len,
                    SQLUSMALLINT fScope __attribute__((unused)),
                    SQLUSMALLINT fNullable __attribute__((unused)))
{
  /* The function is just a stub. We call non-I_S version of the function before implementing the I_S one */
  return special_columns_no_i_s(hstmt, fColType, table_qualifier,
                                table_qualifier_len, table_owner, table_owner_len,
                                table_name, table_len, fScope, fNullable);
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
                    SQLCHAR *table_qualifier, SQLSMALLINT table_qualifier_len,
                    SQLCHAR *table_owner __attribute__((unused)),
                    SQLSMALLINT table_owner_len __attribute__((unused)),
                    SQLCHAR *table_name, SQLSMALLINT table_len,
                    SQLUSMALLINT fScope __attribute__((unused)),
                    SQLUSMALLINT fNullable __attribute__((unused)))
{
  STMT        *stmt=(STMT *) hstmt;

  CLEAR_STMT_ERROR(hstmt);
  my_SQLFreeStmt(hstmt,MYSQL_RESET);

  GET_NAME_LEN(stmt, table_qualifier, table_qualifier_len);
  GET_NAME_LEN(stmt, table_owner, table_owner_len);
  GET_NAME_LEN(stmt, table_name, table_len);

  if (server_has_i_s(stmt->dbc) && !stmt->dbc->ds->no_information_schema)
  {
    return special_columns_i_s(hstmt, fColType, table_qualifier,
                               table_qualifier_len, table_owner, table_owner_len,
                               table_name, table_len, fScope, fNullable);
  }
  else
  {
    return special_columns_no_i_s(hstmt, fColType, table_qualifier,
                                  table_qualifier_len, table_owner, table_owner_len,
                                  table_name, table_len, fScope, fNullable);
  }
}


/*
****************************************************************************
SQLPrimaryKeys
****************************************************************************
*/
SQLRETURN
primary_keys_i_s(SQLHSTMT hstmt,
                 SQLCHAR *catalog_name, SQLSMALLINT catalog_len,
                 SQLCHAR *schema_name __attribute__((unused)),
                 SQLSMALLINT schema_len __attribute__((unused)),
                 SQLCHAR *table_name, SQLSMALLINT table_len)
{
  /* The function is just a stub. We call non-I_S version of the function before implementing the I_S one */
  return primary_keys_no_i_s(hstmt, catalog_name, catalog_len, schema_name, schema_len,
                             table_name, table_len);
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
                 SQLCHAR *catalog_name, SQLSMALLINT catalog_len,
                 SQLCHAR *schema_name __attribute__((unused)),
                 SQLSMALLINT schema_len __attribute__((unused)),
                 SQLCHAR *table_name, SQLSMALLINT table_len)
{
  STMT *stmt= (STMT *) hstmt;

  CLEAR_STMT_ERROR(hstmt);
  my_SQLFreeStmt(hstmt,MYSQL_RESET);

  GET_NAME_LEN(stmt, catalog_name, catalog_len);
  GET_NAME_LEN(stmt, schema_name, schema_len);
  GET_NAME_LEN(stmt, table_name, table_len);

  if (server_has_i_s(stmt->dbc) && !stmt->dbc->ds->no_information_schema)
  {
    return primary_keys_i_s(hstmt, catalog_name, catalog_len, schema_name, schema_len,
                            table_name, table_len);
  }
  else
  {
    return primary_keys_no_i_s(hstmt, catalog_name, catalog_len, schema_name, schema_len,
                               table_name, table_len);
  }
}


/*
****************************************************************************
SQLForeignKeys
****************************************************************************
*/
SQLRETURN foreign_keys_i_s(SQLHSTMT hstmt,
                           SQLCHAR    *pk_catalog_name __attribute__((unused)),
                           SQLSMALLINT pk_catalog_len __attribute__((unused)),
                           SQLCHAR    *pk_schema_name __attribute__((unused)),
                           SQLSMALLINT pk_schema_len __attribute__((unused)),
                           SQLCHAR    *pk_table_name,
                           SQLSMALLINT pk_table_len,
                           SQLCHAR    *fk_catalog_name,
                           SQLSMALLINT fk_catalog_len,
                           SQLCHAR    *fk_schema_name __attribute__((unused)),
                           SQLSMALLINT fk_schema_len __attribute__((unused)),
                           SQLCHAR    *fk_table_name,
                           SQLSMALLINT fk_table_len)
{
  STMT *stmt=(STMT *) hstmt;
  MYSQL *mysql= &stmt->dbc->mysql;
  char query[3062], *buff; /* This should be big enough. */
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
                " WHERE D.CONSTRAINT_NAME IS NOT NULL ",
                NullS);

  if (pk_table_name && pk_table_name[0])
  {
    buff= myodbc_stpmov(buff, "AND A.REFERENCED_TABLE_SCHEMA = ");
    if (pk_catalog_name && pk_catalog_name[0])
    {
      buff= myodbc_stpmov(buff, "'");
      buff+= mysql_real_escape_string(mysql, buff, (char *)pk_catalog_name,
                                      pk_catalog_len);
      buff= myodbc_stpmov(buff, "' ");
    }
    else
    {
      buff= myodbc_stpmov(buff, "DATABASE() ");
    }

    buff= myodbc_stpmov(buff, "AND A.REFERENCED_TABLE_NAME = '");

    buff+= mysql_real_escape_string(mysql, buff, (char *)pk_table_name,
                                    pk_table_len);
    buff= myodbc_stpmov(buff, "' ");

    myodbc_stpmov(buff, "ORDER BY PKTABLE_CAT, PKTABLE_NAME, "
                 "KEY_SEQ, FKTABLE_NAME");
  }

  if (fk_table_name && fk_table_name[0])
  {
    buff= myodbc_stpmov(buff, "AND A.TABLE_SCHEMA = ");

    if (fk_catalog_name && fk_catalog_name[0])
    {
      buff= myodbc_stpmov(buff, "'");
      buff+= mysql_real_escape_string(mysql, buff, (char *)fk_catalog_name,
                                      fk_catalog_len);
      buff= myodbc_stpmov(buff, "' ");
    }
    else
    {
      buff= myodbc_stpmov(buff, "DATABASE() ");
    }

    buff= myodbc_stpmov(buff, "AND A.TABLE_NAME = '");

    buff+= mysql_real_escape_string(mysql, buff, (char *)fk_table_name,
                                    fk_table_len);
    buff= myodbc_stpmov(buff, "' ");

    buff= myodbc_stpmov(buff, "ORDER BY FKTABLE_CAT, FKTABLE_NAME, "
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
  @param[in] pk_catalog_name Catalog (database) of table with primary key that
                             we want to see foreign keys for
  @param[in] pk_catalog_len  Length of @a pk_catalog_name
  @param[in] pk_schema_name  Schema of table with primary key that we want to
                             see foreign keys for (unused)
  @param[in] pk_schema_len   Length of @a pk_schema_name
  @param[in] pk_table_name   Table with primary key that we want to see foreign
                             keys for
  @param[in] pk_table_len    Length of @a pk_table_name
  @param[in] fk_catalog_name Catalog (database) of table with foreign keys we
                             are interested in
  @param[in] fk_catalog_len  Length of @a fk_catalog_name
  @param[in] fk_schema_name  Schema of table with foreign keys we are
                             interested in
  @param[in] fk_schema_len   Length of fk_schema_name
  @param[in] fk_table_name   Table with foreign keys we are interested in
  @param[in] fk_table_len    Length of @a fk_table_name

  @return SQL_SUCCESS

  @since ODBC 1.0
*/
SQLRETURN SQL_API
MySQLForeignKeys(SQLHSTMT hstmt,
                 SQLCHAR *pk_catalog_name __attribute__((unused)),
                 SQLSMALLINT pk_catalog_len __attribute__((unused)),
                 SQLCHAR *pk_schema_name __attribute__((unused)),
                 SQLSMALLINT pk_schema_len __attribute__((unused)),
                 SQLCHAR *pk_table_name, SQLSMALLINT pk_table_len,
                 SQLCHAR *fk_catalog_name, SQLSMALLINT fk_catalog_len,
                 SQLCHAR *fk_schema_name __attribute__((unused)),
                 SQLSMALLINT fk_schema_len __attribute__((unused)),
                 SQLCHAR *fk_table_name, SQLSMALLINT fk_table_len)
{
    STMT *stmt=(STMT *) hstmt;

    CLEAR_STMT_ERROR(hstmt);
    my_SQLFreeStmt(hstmt,MYSQL_RESET);

    GET_NAME_LEN(stmt, pk_catalog_name, pk_catalog_len);
    GET_NAME_LEN(stmt, fk_catalog_name, fk_catalog_len);
    GET_NAME_LEN(stmt, pk_schema_name, pk_schema_len);
    GET_NAME_LEN(stmt, fk_schema_name, fk_schema_len);
    GET_NAME_LEN(stmt, pk_table_name, pk_table_len);
    GET_NAME_LEN(stmt, fk_table_name, fk_table_len);

    if (server_has_i_s(stmt->dbc) && !stmt->dbc->ds->no_information_schema)
    {
      return foreign_keys_i_s(hstmt, pk_catalog_name, pk_catalog_len, pk_schema_name,
                              pk_schema_len, pk_table_name, pk_table_len, fk_catalog_name,
                              fk_catalog_len, fk_schema_name, fk_schema_len,
                              fk_table_name, fk_table_len);
    }
    /* For 3.23 and later, use comment in SHOW TABLE STATUS (yuck). */
    else /* We wouldn't get here if we had server version under 3.23 */
    {
      return foreign_keys_no_i_s(hstmt, pk_catalog_name, pk_catalog_len, pk_schema_name,
                                 pk_schema_len, pk_table_name, pk_table_len, fk_catalog_name,
                                 fk_catalog_len, fk_schema_name, fk_schema_len,
                                 fk_table_name, fk_table_len);
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
  @param[in] catalog_name     Name of catalog (database)
  @param[in] catalog_len      Length of catalog
  @param[in] schema_name      Pattern of schema (unused)
  @param[in] schema_len       Length of schema name
  @param[in] proc_name        Pattern of procedure names to fetch
  @param[in] proc_len         Length of procedure name
*/
SQLRETURN SQL_API
MySQLProcedures(SQLHSTMT hstmt,
                SQLCHAR *catalog_name, SQLSMALLINT catalog_len,
                SQLCHAR *schema_name __attribute__((unused)),
                SQLSMALLINT schema_len __attribute__((unused)),
                SQLCHAR *proc_name, SQLSMALLINT proc_len)
{
  SQLRETURN rc;
  STMT *stmt= (STMT *)hstmt;

  CLEAR_STMT_ERROR(hstmt);
  my_SQLFreeStmt(hstmt,MYSQL_RESET);

  GET_NAME_LEN(stmt, catalog_name, catalog_len);
  GET_NAME_LEN(stmt, schema_name, schema_len);
  GET_NAME_LEN(stmt, proc_name, proc_len);

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
  if (catalog_name && proc_name)
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
  else if (proc_name)
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

  if (proc_name)
  {
    rc= my_SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_C_CHAR,
                            0, 0, proc_name, proc_len, NULL);
    if (!SQL_SUCCEEDED(rc))
      return rc;
  }

  if (catalog_name)
  {
    rc= my_SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR,
                            SQL_C_CHAR, 0, 0, catalog_name, catalog_len,
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
procedure_columns_i_s(SQLHSTMT hstmt,
                      SQLCHAR *catalog_name, SQLSMALLINT catalog_len,
                      SQLCHAR *schema_name __attribute__((unused)),
                      SQLSMALLINT schema_len __attribute__((unused)),
                      SQLCHAR *proc_name, SQLSMALLINT proc_len,
                      SQLCHAR *column_name, SQLSMALLINT column_len)
{
  /* The function is just a stub. We call non-I_S version of the function before implementing the I_S one */
  return procedure_columns_no_i_s(hstmt, catalog_name, catalog_len, schema_name,
                                  schema_len, proc_name, proc_len, column_name,
                                  column_len);
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
                    SQLCHAR *catalog_name, SQLSMALLINT catalog_len,
                    SQLCHAR *schema_name __attribute__((unused)),
                    SQLSMALLINT schema_len __attribute__((unused)),
                    SQLCHAR *proc_name, SQLSMALLINT proc_len,
                    SQLCHAR *column_name, SQLSMALLINT column_len)
{
  STMT *stmt= (STMT *)hstmt;

  CLEAR_STMT_ERROR(hstmt);
  my_SQLFreeStmt(hstmt,MYSQL_RESET);

  GET_NAME_LEN(stmt, catalog_name, catalog_len);
  GET_NAME_LEN(stmt, schema_name, schema_len);
  GET_NAME_LEN(stmt, proc_name, proc_len);
  GET_NAME_LEN(stmt, column_name, column_len);

  if (server_has_i_s(stmt->dbc) && !stmt->dbc->ds->no_information_schema)
  {
    return procedure_columns_i_s(hstmt, catalog_name, catalog_len, schema_name,
                                 schema_len, proc_name, proc_len, column_name,
                                 column_len);
  }
  else
  {
    return procedure_columns_no_i_s(hstmt, catalog_name, catalog_len, schema_name,
                                    schema_len, proc_name, proc_len, column_name,
                                    column_len);
  }
}
