/*
  Copyright (C) 2000-2007 MySQL AB

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
  @file  catalog.c
  @brief Catalog functions.
*/

#include "driver.h"

/*
  @type    : internal
  @purpose : returns the next token
*/

static const char *my_next_token(const char *prev_token, 
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


/*
  @type    : internal
  @purpose : validate for give table type from the list
*/
static my_bool check_table_type(const SQLCHAR *TableType, 
                                const char *req_type, 
                                uint       len)
{
    char    req_type_quoted[NAME_LEN+2], req_type_quoted1[NAME_LEN+2];
    char    *type, *table_type= (char *)TableType;
    my_bool found= 0;

    if ( !TableType || !TableType[0] )
        return found;

    /* 
      Check and return only 'user' tables from current DB and 
      don't return any tables when its called with 
      SYSTEM TABLES.
  
      Expected Types:
        "TABLE", "VIEW", "SYSTEM TABLE", "GLOBAL TEMPORARY", 
        "LOCAL TEMPORARY", "ALIAS", "SYNONYM",  
    */

    type= strstr(table_type,",");
    sprintf(req_type_quoted,"'%s'",req_type);
    sprintf(req_type_quoted1,"`%s`",req_type);
    while ( type )
    {
        while ( isspace(*(table_type)) ) table_type++;
        if ( !myodbc_casecmp(table_type,req_type,len) || 
             !myodbc_casecmp(table_type,req_type_quoted,len+2) || 
             !myodbc_casecmp(table_type,req_type_quoted1,len+2) )
        {
            found= 1;
            break;
        }
        table_type= ++type;
        type= strstr(table_type,",");
    }
    if ( !found )
    {
        while ( isspace(*(table_type)) ) table_type++;
        if ( !myodbc_casecmp(table_type,req_type,len) || 
             !myodbc_casecmp(table_type,req_type_quoted,len+2) ||
             !myodbc_casecmp(table_type,req_type_quoted1,len+2) )
            found= 1;
    }
    return found;
}


static MYSQL_ROW fix_fields_copy(STMT FAR *stmt,MYSQL_ROW row)
{
    uint i;
    for ( i=0 ; i < stmt->order_count; i++ )
        stmt->array[stmt->order[i]]= row[i];
    return stmt->array;
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
static SQLRETURN
create_fake_resultset(STMT *stmt, MYSQL_ROW rowval, size_t rowsize,
                      my_ulonglong rowcnt, MYSQL_FIELD *fields, uint fldcnt)
{
  stmt->result= (MYSQL_RES*) my_malloc(sizeof(MYSQL_RES), MYF(MY_ZEROFILL));
  stmt->result_array= (MYSQL_ROW)my_memdup((char *)rowval, rowsize, MYF(0));
  if (!(stmt->result && stmt->result_array))
  {
    if (stmt->result)
      my_free((char *)stmt->result, MYF(0));
    if (stmt->result_array)
      my_free((char *)stmt->result_array, MYF(0));
    set_mem_error(&stmt->dbc->mysql);
    return handle_connection_error(stmt);
  }
  stmt->fake_result= 1;
  stmt->result->row_count= rowcnt;
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
static SQLRETURN
create_empty_fake_resultset(STMT *stmt, MYSQL_ROW rowval, size_t rowsize,
                            MYSQL_FIELD *fields, uint fldcnt)
{
  return create_fake_resultset(stmt, rowval, rowsize, 0 /* rowcnt */,
                               fields, fldcnt);
}


/**
  Get the table status for a table or tables.

  @param[in] stmt           Handle to statement
  @param[in] catalog        Catalog (database) of table, @c NULL for current
  @param[in] catalog_length Length of catalog name, or @c SQL_NTS
  @param[in] table          Name of table
  @param[in] table_length   Length of table name, or @c SQL_NTS
  @param[in] wildcard       Whether the table name is a wildcard

  @return Result of SHOW TABLE STATUS, or NULL if there is an error
          or empty result (check mysql_errno(&stmt->dbc->mysql) != 0)
*/
static MYSQL_RES *mysql_table_status(STMT        *stmt,
                                     SQLCHAR     *catalog,
                                     SQLSMALLINT  catalog_length,
                                     SQLCHAR     *table,
                                     SQLSMALLINT  table_length,
                                     my_bool      wildcard)
{
  MYSQL *mysql= &stmt->dbc->mysql;
  /** @todo determine real size for buffer */
  char buff[255], *to;

  if (table_length == SQL_NTS && table)
    table_length= strlen((char *)table);
  if (catalog_length == SQL_NTS && catalog)
    catalog_length= strlen((char *)catalog);

  to= strmov(buff, "SHOW TABLE STATUS ");
  if (catalog && *catalog)
  {
    to= strmov(to, "FROM `");
    to+= myodbc_escape_string(mysql, to, sizeof(buff) - (to - buff),
                              (char *)catalog, catalog_length, 1);
    to= strmov(to, "` ");
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
    to= strmov(to, "LIKE '");
    if (wildcard)
      to+= mysql_real_escape_string(mysql, to, (char *)table, table_length);
    else
      to+= myodbc_escape_string(mysql, to, sizeof(buff) - (to - buff),
                                (char *)table, table_length, 0);
    to= strmov(to, "'");
  }

  MYLOG_QUERY(stmt, buff);
  if (mysql_query(mysql,buff))
    return NULL;

  return mysql_store_result(mysql);
}


/*
****************************************************************************
SQLTables
****************************************************************************
*/

uint SQLTABLES_qualifier_order[]= {0};
char *SQLTABLES_values[]= {"","",NULL,"TABLE","MySQL table"};
char *SQLTABLES_qualifier_values[]= {"",NULL,NULL,NULL,NULL};
char *SQLTABLES_owner_values[]= {NULL,"",NULL,NULL,NULL};
char *SQLTABLES_type_values[3][5]=
{
    {NULL,NULL,NULL,"TABLE",NULL},
    {NULL,NULL,NULL,"SYSTEM TABLE",NULL},
    {NULL,NULL,NULL,"VIEW",NULL},
};

MYSQL_FIELD SQLTABLES_fields[]=
{
  MYODBC_FIELD_STRING("TABLE_CAT",   NAME_LEN, 0),
  MYODBC_FIELD_STRING("TABLE_SCHEM", NAME_LEN, 0),
  MYODBC_FIELD_STRING("TABLE_NAME",  NAME_LEN, 0),
  MYODBC_FIELD_STRING("TABLE_TYPE",  NAME_LEN, 0),
  MYODBC_FIELD_STRING("REMARKS",     NAME_LEN, 0),
};

const uint SQLTABLES_FIELDS= array_elements(SQLTABLES_values);

SQLRETURN SQL_API
MySQLTables(SQLHSTMT hstmt,
            SQLCHAR *catalog, SQLSMALLINT catalog_len,
            SQLCHAR *schema, SQLSMALLINT schema_len,
            SQLCHAR *table, SQLSMALLINT table_len,
            SQLCHAR *type, SQLSMALLINT type_len)
{
    STMT *stmt= (STMT *)hstmt;
    my_bool all_dbs= 1, user_tables, views;

    CLEAR_STMT_ERROR(hstmt);
    my_SQLFreeStmt(hstmt, MYSQL_RESET);

    if (catalog_len == SQL_NTS)
      catalog_len= catalog ? strlen((char *)catalog) : 0;
    if (schema_len == SQL_NTS)
      schema_len= schema ? strlen((char *)schema) : 0;
    if (table_len == SQL_NTS)
      table_len= table ? strlen((char *)table) : 0;
    if (type_len == SQL_NTS)
      type_len= type ? strlen((char *)type) : 0;

    if (catalog_len && !schema_len && !table_len)
    {
        pthread_mutex_lock(&stmt->dbc->lock);
        {
          char buff[32 + NAME_LEN], *to;
          to= strmov(buff, "SHOW DATABASES LIKE '");
          to+= mysql_real_escape_string(&stmt->dbc->mysql, to,
                                        (char *)catalog, catalog_len);
          to= strmov(to, "'");
          MYLOG_QUERY(stmt, buff);
          if (!mysql_query(&stmt->dbc->mysql, buff))
            stmt->result= mysql_store_result(&stmt->dbc->mysql);
        }
        pthread_mutex_unlock(&stmt->dbc->lock);

        if (!stmt->result)
        {
          return handle_connection_error(stmt);
        }

        stmt->order         = SQLTABLES_qualifier_order;
        stmt->order_count   = array_elements(SQLTABLES_qualifier_order);
        stmt->fix_fields    = fix_fields_copy;
        stmt->array= (MYSQL_ROW) my_memdup((char *)SQLTABLES_qualifier_values,
                                           sizeof(SQLTABLES_qualifier_values),
                                           MYF(0));
        if (!stmt->array)
        {
          set_mem_error(&stmt->dbc->mysql);
          return handle_connection_error(stmt);
        }
        mysql_link_fields(stmt,SQLTABLES_fields,SQLTABLES_FIELDS);
        return SQL_SUCCESS;
    }

    if (!catalog_len &&
        schema_len &&
        !table_len)
    {
        /* Return set of allowed schemas (none) */
        return create_fake_resultset(stmt, SQLTABLES_owner_values,
                                     sizeof(SQLTABLES_owner_values),
                                     1, SQLTABLES_fields, SQLTABLES_FIELDS);
    }

    if (!catalog_len && !schema_len && !table_len && type &&
        !strncmp((char *)type, "%", 2))
    {
        /* Return set of TableType qualifiers */
        return create_fake_resultset(stmt, (MYSQL_ROW)SQLTABLES_type_values,
                                     sizeof(SQLTABLES_type_values),
                                     sizeof(SQLTABLES_type_values) /
                                     sizeof(SQLTABLES_type_values[0]),
                                     SQLTABLES_fields, SQLTABLES_FIELDS);
    }

    user_tables= check_table_type(type, "TABLE", 5);
    views= check_table_type(type, "VIEW", 4);

    /* If no types specified, we want tables and views. */
    if (!user_tables && !views)
    {
      if (!type_len)
        user_tables= views= 1;
    }

    if ((type_len && !views && !user_tables) ||
        (schema_len && strncmp((char *)schema, "%", 2)))
    {
      /*
        Return empty set if unknown TableType or
        if schema is used and not '%'
      */
      goto empty_set;
    }

    /* User Tables with type as 'TABLE' or 'VIEW' */
    if (user_tables || views)
    {
      pthread_mutex_lock(&stmt->dbc->lock);
      stmt->result= mysql_table_status(stmt, catalog, catalog_len,
                                       table, table_len, TRUE);

      if (!stmt->result && mysql_errno(&stmt->dbc->mysql))
      {
        SQLRETURN rc;
        /* unknown DB will return empty set from SQLTables */
        switch (mysql_errno(&stmt->dbc->mysql))
        {
        case ER_BAD_DB_ERROR:
          pthread_mutex_unlock(&stmt->dbc->lock);
          goto empty_set;
        default:
          rc= handle_connection_error(stmt);
          pthread_mutex_unlock(&stmt->dbc->lock);
          return rc;
        }
      }
      pthread_mutex_unlock(&stmt->dbc->lock);
    }

    if (!stmt->result)
      goto empty_set;

    /* assemble final result set */
    {
      MYSQL_ROW    data= 0, row;
      char         *db= "";
      my_ulonglong row_count= stmt->result->row_count;

      if (!row_count)
      {
        mysql_free_result(stmt->result);
        goto empty_set;
      }

      if (!(stmt->result_array=
            (char **)my_malloc((uint)(sizeof(char *) * SQLTABLES_FIELDS *
                                      row_count),
                               MYF(MY_ZEROFILL))))
      {
        set_mem_error(&stmt->dbc->mysql);
        return handle_connection_error(stmt);
      }

      data= stmt->result_array;

      if (!option_flag(stmt, FLAG_NO_CATALOG))
        db= strmake_root(&stmt->result->field_alloc,
                         (char *)catalog, catalog_len);

      while ((row= mysql_fetch_row(stmt->result)))
      {
        int comment_index= (stmt->result->field_count == 18) ? 17 : 15;
        my_bool view= (!row[1] &&
                       myodbc_casecmp(row[comment_index], "view", 4) == 0);

        if ((view && !views) || (!view && !user_tables))
        {
          row_count--;
          continue;
        }

        data[0]= db;
        data[1]= "";
        data[2]= strdup_root(&stmt->result->field_alloc, row[0]);
        data[3]= view ? "VIEW" : "TABLE";
        data[4]= strdup_root(&stmt->result->field_alloc, row[comment_index]);
        data+= SQLTABLES_FIELDS;
      }

      stmt->result->row_count= row_count;
    }

    mysql_link_fields(stmt, SQLTABLES_fields, SQLTABLES_FIELDS);
    return SQL_SUCCESS;

empty_set:
  return create_empty_fake_resultset(stmt, SQLTABLES_values,
                                     sizeof(SQLTABLES_values),
                                     SQLTABLES_fields,
                                     SQLTABLES_FIELDS);
}


/*
****************************************************************************
SQLColumns
****************************************************************************
*/

char SC_type[10],SC_typename[20],SC_precision[10],SC_length[10],SC_scale[10],
SC_nullable[10], SC_coldef[10], SC_sqltype[10],SC_octlen[10],
SC_pos[10],SC_isnull[10];

char *SQLCOLUMNS_values[]= {
    "","",NullS,NullS,SC_type,SC_typename,
    SC_precision,
    SC_length,SC_scale,"10",SC_nullable,"MySQL column",
    SC_coldef,SC_sqltype,NullS,SC_octlen,NullS,SC_isnull
};

MYSQL_FIELD SQLCOLUMNS_fields[]=
{
  MYODBC_FIELD_STRING("TABLE_CAT", NAME_LEN, 0),
  MYODBC_FIELD_STRING("TABLE_SCHEM", NAME_LEN, 0),
  MYODBC_FIELD_STRING("TABLE_NAME", NAME_LEN, NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("COLUMN_NAME", NAME_LEN, NOT_NULL_FLAG),
  MYODBC_FIELD_SHORT("DATA_TYPE", NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("TYPE_NAME", 20, NOT_NULL_FLAG),
  MYODBC_FIELD_LONG("COLUMN_SIZE", 0),
  MYODBC_FIELD_LONG("BUFFER_LENGTH", 0),
  MYODBC_FIELD_SHORT("DECIMAL_DIGITS", 0),
  MYODBC_FIELD_SHORT("NUM_PREC_RADIX", 0),
  MYODBC_FIELD_SHORT("NULLABLE", NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("REMARKS", NAME_LEN, 0),
  MYODBC_FIELD_STRING("COLUMN_DEF", NAME_LEN, 0),
  MYODBC_FIELD_SHORT("SQL_DATA_TYPE", NOT_NULL_FLAG),
  MYODBC_FIELD_SHORT("SQL_DATETIME_SUB", 0),
  MYODBC_FIELD_LONG("CHAR_OCTET_LENGTH", 0),
  MYODBC_FIELD_LONG("ORDINAL_POSITION", NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("IS_NULLABLE", 3, 0),
};

const uint SQLCOLUMNS_FIELDS= array_elements(SQLCOLUMNS_values);


/**
  Get the list of columns in a table matching a wildcard.

  @param[in] stmt             Statement
  @param[in] szCatalog        Name of catalog (database)
  @param[in] cbCatalog        Length of catalog
  @param[in] szTable          Name of table
  @param[in] cbTable          Length of table
  @param[in] szColumn         Pattern of column names to match
  @param[in] cbColumn         Length of column pattern

  @return Result of SHOW COLUMNS FROM, or NULL if there is an error
*/
static
MYSQL_RES *mysql_list_dbcolumns(STMT *stmt,
                                SQLCHAR *szCatalog, SQLSMALLINT cbCatalog,
                                SQLCHAR *szTable, SQLSMALLINT cbTable,
                                SQLCHAR *szColumn, SQLSMALLINT cbColumn)
{
  DBC *dbc= stmt->dbc;
  MYSQL *mysql= &dbc->mysql;
  MYSQL_RES *result;
  MYSQL_ROW row;

  /* If a catalog was specified, we have to do it the hard way. */
  if (cbCatalog)
  {
    char buff[255];
    char *select, *to;
    size_t select_len;
    unsigned long *lengths;

    /* Get a list of column names that match our criteria. */
    to= strmov(buff, "SHOW COLUMNS FROM `");
    if (cbCatalog)
    {
      to+= myodbc_escape_string(mysql, to, sizeof(buff) - (to - buff),
                                (char *)szCatalog, cbCatalog, 1);
      to= strmov(to, "`.`");
    }

    to+= myodbc_escape_string(mysql, to, sizeof(buff) - (to - buff),
                              (char *)szTable, cbTable, 1);
    to= strmov(to, "`");

    if (cbColumn)
    {
      to= strmov(to, " LIKE '");
      to+= mysql_real_escape_string(mysql, to, (char *)szColumn, cbColumn);
      to= strmov(to, "'");
    }

    MYLOG_QUERY(stmt, buff);

    pthread_mutex_lock(&dbc->lock);
    if (mysql_query(mysql,buff) ||
        !(result= mysql_store_result(mysql)))
    {
      pthread_mutex_unlock(&dbc->lock);
      return NULL;
    }
    pthread_mutex_unlock(&dbc->lock);

    /* Build a SELECT ... LIMIT 0 to get the field metadata. */
    select_len = (ulong)result->row_count * (NAME_LEN + 1) + NAME_LEN * 2;
    if (!(select= (char *)my_malloc(select_len, MYF(0))))
    {
      set_mem_error(mysql);
      return NULL;
    }

    to= strxmov(select, "SELECT ", NullS);
    while ((row= mysql_fetch_row(result)))
    {
      to= strmov(to, "`");
      lengths= mysql_fetch_lengths(result);
      to+= myodbc_escape_string(mysql, to, select_len - (to - select),
                                (char *)row[0], lengths[0], 1);
      to= strmov(to, "`,");
    }
    *(--to)= '\0';

    to= strmov(to, " FROM `");
    if (cbCatalog)
    {
      to+= myodbc_escape_string(mysql, to, select_len - (to - select),
                                (char *)szCatalog, cbCatalog, 1);
      to= strmov(to, "`.`");
    }

    to+= myodbc_escape_string(mysql, to, select_len - (to - select),
                              (char *)szTable, cbTable, 1);
    to= strmov(to, "` LIMIT 0");

    mysql_free_result(result);

    MYLOG_QUERY(stmt, select);

    pthread_mutex_lock(&dbc->lock);
    if (mysql_query(mysql, select))
    {
      my_free(select, MYF(0));
      pthread_mutex_unlock(&dbc->lock);
      return NULL;
    }
    result= mysql_store_result(&dbc->mysql);
    pthread_mutex_unlock(&dbc->lock);
    my_free(select, MYF(0));

    return result;
  }

  pthread_mutex_lock(&dbc->lock);
  result= mysql_list_fields(mysql, (char *)szTable, (char *)szColumn);
  pthread_mutex_unlock(&dbc->lock);

  return result;
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
  MYSQL_RES *res;
  MEM_ROOT *alloc;
  MYSQL_ROW table_row;
  unsigned long rows= 0, next_row= 0, *lengths;
  char *db= NULL;

  CLEAR_STMT_ERROR(hstmt);
  my_SQLFreeStmt(hstmt, MYSQL_RESET);

  /* Get the list of tables that match szCatalog and szTable */
  pthread_mutex_lock(&stmt->dbc->lock);
  res= mysql_table_status(stmt, szCatalog, cbCatalog, szTable, cbTable, TRUE);

  if (!res && mysql_errno(&stmt->dbc->mysql))
  {
    SQLRETURN rc= handle_connection_error(stmt);
    pthread_mutex_unlock(&stmt->dbc->lock);
    return rc;
  }
  else if (!res)
  {
    pthread_mutex_unlock(&stmt->dbc->lock);
    goto empty_set;
  }
  pthread_mutex_unlock(&stmt->dbc->lock);

  stmt->result= res;
  alloc= &res->field_alloc;

  if (cbCatalog == SQL_NTS)
    cbCatalog= szCatalog ? strlen((char *)szCatalog) : 0;
  if (cbColumn == SQL_NTS)
    cbColumn= szColumn ? strlen((char *)szColumn) : 0;

  if (!option_flag(stmt, FLAG_NO_CATALOG))
    db= strmake_root(alloc, (char *)szCatalog, cbCatalog);

  while ((table_row= mysql_fetch_row(res)))
  {
    MYSQL_FIELD *field;
    MYSQL_RES *table_res;
    int count= 0;

    /* Get list of columns matching szColumn for each table. */
    lengths= mysql_fetch_lengths(res);
    table_res= mysql_list_dbcolumns(stmt, szCatalog, cbCatalog,
                                    (SQLCHAR *)table_row[0],
                                    (SQLSMALLINT)lengths[0],
                                    szColumn, cbColumn);

    if (!table_res)
    {
      return handle_connection_error(stmt);
    }

    rows+= mysql_num_fields(table_res);

    stmt->result_array= (char **)my_realloc((char *)stmt->result_array,
                                            sizeof(char *) *
                                            SQLCOLUMNS_FIELDS * rows,
                                            MYF(MY_ALLOW_ZERO_PTR));
    if (!stmt->result_array)
    {
      set_mem_error(&stmt->dbc->mysql);
      return handle_connection_error(stmt);
    }

    while ((field= mysql_fetch_field(table_res)))
    {
      SQLSMALLINT type;
      char buff[255]; /* @todo justify the size of this buffer */
      MYSQL_ROW row= stmt->result_array + (SQLCOLUMNS_FIELDS * next_row++);

      row[0]= db;                     /* TABLE_CAT */
      row[1]= NULL;                   /* TABLE_SCHEM */
      row[2]= strdup_root(alloc, field->table); /* TABLE_NAME */
      row[3]= strdup_root(alloc, field->name);  /* COLUMN_NAME */

      type= get_sql_data_type(stmt, field, buff);

      row[5]= strdup_root(alloc, buff); /* TYPE_NAME */

      sprintf(buff, "%d", type);
      row[4]= strdup_root(alloc, buff); /* DATA_TYPE */

      if (type == SQL_TYPE_DATE || type == SQL_TYPE_TIME ||
          type == SQL_TYPE_TIMESTAMP)
      {
        row[14]= row[4];    /* SQL_DATETIME_SUB */
        sprintf(buff, "%d", SQL_DATETIME);
        row[13]= strdup_root(alloc, buff); /* SQL_DATA_TYPE */
      }
      else
      {
        row[13]= row[4];    /* SQL_DATA_TYPE */
        row[14]= NULL;      /* SQL_DATETIME_SUB */
      }

      /* COLUMN_SIZE */
      fill_column_size_buff(buff, stmt, field);
      row[6]= strdup_root(alloc, buff);

      /* BUFFER_LENGTH */
      sprintf(buff, "%ld", get_transfer_octet_length(stmt, field));
      row[7]= strdup_root(alloc, buff);

      if (is_char_sql_type(type) || is_wchar_sql_type(type) ||
          is_binary_sql_type(type))
        row[15]= strdup_root(alloc, buff); /* CHAR_OCTET_LENGTH */
      else
        row[15]= NULL;                     /* CHAR_OCTET_LENGTH */

      {
        SQLLEN digits= get_decimal_digits(stmt, field);
        if (digits != SQL_NO_TOTAL)
        {
          sprintf(buff, "%ld", digits);
          row[8]= strdup_root(alloc, buff);  /* DECIMAL_DIGITS */
          row[9]= "10";                      /* NUM_PREC_RADIX */
        }
        else
        {
          row[8]= row[9]= NullS;             /* DECIMAL_DIGITS, NUM_PREC_RADIX */
        }
      }

      /*
        If a field has TIMESTAMP_FLAG set, it's an auto-updating timestamp
        field, and NULL can be stored to it (although it gets turned into
        something else).

        The same logic applies to fields with AUTO_INCREMENT_FLAG set.
      */
      if ((field->flags & NOT_NULL_FLAG) && !(field->flags & TIMESTAMP_FLAG) &&
          !(field->flags & AUTO_INCREMENT_FLAG))
      {
        sprintf(buff, "%d", SQL_NO_NULLS);
        row[10]= strdup_root(alloc, buff); /* NULLABLE */
        row[17]= strdup_root(alloc, "NO"); /* IS_NULLABLE */
      }
      else
      {
        sprintf(buff, "%d", SQL_NULLABLE);
        row[10]= strdup_root(alloc, buff); /* NULLABLE */
        row[17]= strdup_root(alloc, "YES");/* IS_NULLABLE */
      }

      row[11]= ""; /* REMARKS */

      /*
         The default value of the column. The value in this column should be
         interpreted as a string if it is enclosed in quotation marks.

         if NULL was specified as the default value, then this column is the
         word NULL, not enclosed in quotation marks. If the default value
         cannot be represented without truncation, then this column contains
         TRUNCATED, with no enclosing single quotation marks. If no default
         value was specified, then this column is NULL.

         The value of COLUMN_DEF can be used in generating a new column
         definition, except when it contains the value TRUNCATED
      */
      if (!field->def)
        row[12]= NullS; /* COLUMN_DEF */
      else
      {
        if (field->type == MYSQL_TYPE_TIMESTAMP &&
            !strcmp(field->def,"0000-00-00 00:00:00"))
          row[12]= NullS; /* COLUMN_DEF */
        else
        {
          char *def= alloc_root(alloc, strlen(field->def) + 3);
          if (is_numeric_mysql_type(field))
            sprintf(def, "%s", field->def);
          else
            sprintf(def, "'%s'", field->def);
          row[12]= def; /* COLUMN_DEF */
        }
      }

      sprintf(buff, "%d", ++count);
      row[16]= strdup_root(alloc, buff); /* ORDINAL_POSITION */
    }

    mysql_free_result(table_res);
  }

  stmt->result->row_count= rows;
  mysql_link_fields(stmt, SQLCOLUMNS_fields, SQLCOLUMNS_FIELDS);
  return SQL_SUCCESS;

empty_set:
  return create_empty_fake_resultset(stmt, SQLCOLUMNS_values,
                                     sizeof(SQLCOLUMNS_values),
                                     SQLCOLUMNS_fields,
                                     SQLCOLUMNS_FIELDS);
}


/*
****************************************************************************
SQLStatistics
****************************************************************************
*/

char SS_type[10];
uint SQLSTAT_order[]={2,3,5,7,8,9,10};
char *SQLSTAT_values[]={NullS,NullS,"","",NullS,"",SS_type,"","","","",NullS,NullS};

MYSQL_FIELD SQLSTAT_fields[]=
{
  MYODBC_FIELD_STRING("TABLE_CAT", NAME_LEN, 0),
  MYODBC_FIELD_STRING("TABLE_SCHEM", NAME_LEN, 0),
  MYODBC_FIELD_STRING("TABLE_NAME", NAME_LEN, NOT_NULL_FLAG),
  MYODBC_FIELD_SHORT("NON_UNIQUE", 0),
  MYODBC_FIELD_STRING("INDEX_QUALIFIER", NAME_LEN, 0),
  MYODBC_FIELD_STRING("INDEX_NAME", NAME_LEN, 0),
  MYODBC_FIELD_SHORT("TYPE", NOT_NULL_FLAG),
  MYODBC_FIELD_SHORT("ORDINAL_POSITION", 0),
  MYODBC_FIELD_STRING("COLUMN_NAME", NAME_LEN, 0),
  MYODBC_FIELD_STRING("ASC_OR_DESC", 1, 0),
  MYODBC_FIELD_SHORT("CARDINALITY", 0),
  MYODBC_FIELD_SHORT("PAGES", 0),
  MYODBC_FIELD_STRING("FILTER_CONDITION", 10, 0),
};

const uint SQLSTAT_FIELDS= array_elements(SQLSTAT_fields);

/*
  @type    : internal
  @purpose : returns columns from a particular table, NULL on error
*/
static MYSQL_RES *mysql_list_dbkeys(DBC *dbc,
                                    SQLCHAR *catalog,
                                    SQLSMALLINT catalog_len,
                                    SQLCHAR *table,
                                    SQLSMALLINT table_len)
{
    MYSQL *mysql= &dbc->mysql;
    char  buff[255], *to;

    to= strmov(buff, "SHOW KEYS FROM `");
    if (catalog_len)
    {
      to+= myodbc_escape_string(mysql, to, sizeof(buff) - (to - buff),
                                (char *)catalog, catalog_len, 1);
      to= strmov(to, "`.`");
    }
    to+= myodbc_escape_string(mysql, to, sizeof(buff) - (to - buff),
                              (char *)table, table_len, 1);
    to= strmov(to, "`");

    MYLOG_DBC_QUERY(dbc, buff);
    if (mysql_query(mysql,buff))
        return NULL;
    return mysql_store_result(mysql);
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
    MYSQL *mysql= &stmt->dbc->mysql;
    DBC *dbc= stmt->dbc;

    CLEAR_STMT_ERROR(hstmt);
    my_SQLFreeStmt(hstmt,MYSQL_RESET);

    if (catalog_len == SQL_NTS)
      catalog_len= catalog ? strlen((char *)catalog) : 0;
    if (table_len == SQL_NTS)
      table_len= table ? strlen((char *)table) : 0;

    if (!table_len)
        goto empty_set;

    pthread_mutex_lock(&dbc->lock);
    stmt->result= mysql_list_dbkeys(stmt->dbc, catalog, catalog_len,
                                    table, table_len);
    if (!stmt->result)
    {
      SQLRETURN rc= handle_connection_error(stmt);
      pthread_mutex_unlock(&dbc->lock);
      return rc;
    }
    pthread_mutex_unlock(&dbc->lock);
    my_int2str(SQL_INDEX_OTHER,SS_type,10,0);
    stmt->order=       SQLSTAT_order;
    stmt->order_count= array_elements(SQLSTAT_order);
    stmt->fix_fields=  fix_fields_copy;
    stmt->array= (MYSQL_ROW) my_memdup((char *)SQLSTAT_values,
                                       sizeof(SQLSTAT_values),MYF(0));
    if (!stmt->array)
    {
      set_mem_error(&stmt->dbc->mysql);
      return handle_connection_error(stmt);
    }

    if (option_flag(stmt, FLAG_NO_CATALOG))
      stmt->array[0]= "";
    else
      stmt->array[0]= strmake_root(&stmt->result->field_alloc,
                                   (char *)catalog, catalog_len);

    if ( fUnique == SQL_INDEX_UNIQUE )
    {
        /* This is too low level... */
        MYSQL_ROWS **prev,*pos;
        prev= &stmt->result->data->data;
        for ( pos= *prev; pos; pos= pos->next )
        {
            if ( pos->data[1][0] == '0' ) /* Unlink nonunique index */
            {
                (*prev)=pos;
                prev= &pos->next;
            }
            else
                stmt->result->row_count--;
        }
        (*prev)= 0;
        mysql_data_seek(stmt->result,0);  /* Restore pointer */
    }
    mysql_link_fields(stmt,SQLSTAT_fields,SQLSTAT_FIELDS);
    return SQL_SUCCESS;

empty_set:
  return create_empty_fake_resultset(stmt, SQLSTAT_values,
                                     sizeof(SQLSTAT_values),
                                     SQLSTAT_fields, SQLSTAT_FIELDS);
}

/*
****************************************************************************
SQLTablePrivileges
****************************************************************************
*/

/*
  @type    : internal
  @purpose : checks for the grantability 
*/

static my_bool is_grantable(char *grant_list)
{
    char *grant=dupp_str(grant_list,SQL_NTS);;
    if ( grant_list && grant_list[0] )
    {
        char seps[]   = ",";
        char *token;
        token = strtok( grant, seps );
        while ( token != NULL )
        {
            if ( !strcmp(token,"Grant") )
            {
                x_free(grant);
                return(1);
            }
            token = strtok( NULL, seps );
        }
    }
    x_free(grant);
    return(0);
}

/*
  @type    : internal
  @purpose : returns a table privileges result, NULL on error
*/
static MYSQL_RES *mysql_list_table_priv(DBC *dbc,
                                        SQLCHAR *catalog,
                                        SQLSMALLINT catalog_len,
                                        SQLCHAR *table,
                                        SQLSMALLINT table_len)
{
  MYSQL *mysql= &dbc->mysql;
  char   buff[255+2*NAME_LEN+1], *pos;

  pos= strxmov(buff,
               "SELECT Db,User,Table_name,Grantor,Table_priv ",
               "FROM mysql.tables_priv WHERE Table_name LIKE '",
               NullS);
  pos+= mysql_real_escape_string(mysql, pos, (char *)table, table_len);

  pos= strxmov(pos, "' AND Db = ", NullS);
  if (catalog_len)
  {
    pos= strmov(pos, "'");
    pos+= mysql_real_escape_string(mysql, pos, (char *)catalog, catalog_len);
    pos= strmov(pos, "'");
  }
  else
    pos= strmov(pos, "DATABASE()");

  pos= strxmov(pos, " ORDER BY Db, Table_name, Table_priv, User", NullS);

  MYLOG_DBC_QUERY(dbc, buff);
  if (mysql_query(mysql,buff))
    return NULL;

  return mysql_store_result(mysql);
}


#define MY_MAX_TABPRIV_COUNT 21
#define MY_MAX_COLPRIV_COUNT 3

char *SQLTABLES_priv_values[]= 
{
    NULL,"",NULL,NULL,NULL,NULL,NULL
};

MYSQL_FIELD SQLTABLES_priv_fields[]=
{
  MYODBC_FIELD_STRING("TABLE_CAT", NAME_LEN, 0),
  MYODBC_FIELD_STRING("TABLE_SCHEM", NAME_LEN, 0),
  MYODBC_FIELD_STRING("TABLE_NAME", NAME_LEN, NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("GRANTOR", NAME_LEN, 0),
  MYODBC_FIELD_STRING("GRANTEE", NAME_LEN, NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("PRIVILEGE", NAME_LEN, NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("IS_GRANTABLE", NAME_LEN, 0),
};

const uint SQLTABLES_PRIV_FIELDS= array_elements(SQLTABLES_priv_values);

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
    char     **data, **row;
    MEM_ROOT *alloc;
    STMT     *stmt= (STMT *)hstmt;
    uint     row_count;

    CLEAR_STMT_ERROR(hstmt);
    my_SQLFreeStmt(hstmt,MYSQL_RESET);

    pthread_mutex_lock(&stmt->dbc->lock);
    stmt->result= mysql_list_table_priv(stmt->dbc, catalog, catalog_len,
                                        table, table_len);
    if (!stmt->result)
    {
      SQLRETURN rc= handle_connection_error(stmt);
      pthread_mutex_unlock(&stmt->dbc->lock);
      return rc;
    }
    pthread_mutex_unlock(&stmt->dbc->lock);

    /* Allocate max buffers, to avoid reallocation */
    stmt->result_array= (char**) my_malloc(sizeof(char*)* SQLTABLES_PRIV_FIELDS *
                                           (ulong)stmt->result->row_count * 
                                           MY_MAX_TABPRIV_COUNT, 
                                           MYF(MY_ZEROFILL));
    if (!stmt->result_array)
    {
      set_mem_error(&stmt->dbc->mysql);
      return handle_connection_error(stmt);
    }
    alloc= &stmt->result->field_alloc;
    data= stmt->result_array;
    row_count= 0;
    while ( (row= mysql_fetch_row(stmt->result)) )
    {
        char  *grants= row[4];
        char  token[NAME_LEN+1];
        const char *grant= (const char *)grants;

        for ( ;; )
        {
            data[0]= row[0];
            data[1]= "";
            data[2]= row[2];
            data[3]= row[3];
            data[4]= row[1];
            data[6]= is_grantable(row[4]) ? "YES" : "NO";
            row_count++;

            if ( !(grant= my_next_token(grant,&grants,token,',')) )
            {
                /* End of grants .. */
                data[5]= strdup_root(alloc,grants);
                data+= SQLTABLES_PRIV_FIELDS;
                break;
            }
            data[5]= strdup_root(alloc,token);
            data+= SQLTABLES_PRIV_FIELDS;
        }
    }
    stmt->result->row_count= row_count;
    mysql_link_fields(stmt,SQLTABLES_priv_fields,SQLTABLES_PRIV_FIELDS);
    return SQL_SUCCESS;
}



/*
  @type    : internal
  @purpose : returns a column privileges result, NULL on error
*/
static MYSQL_RES *mysql_list_column_priv(MYSQL *mysql,
                                         SQLCHAR *catalog,
                                         SQLSMALLINT catalog_len,
                                         SQLCHAR *table, SQLSMALLINT table_len,
                                         SQLCHAR *column,
                                         SQLSMALLINT column_len)
{
  char buff[255+3*NAME_LEN+1], *pos;

  pos= strmov(buff,
              "SELECT c.Db, c.User, c.Table_name, c.Column_name,"
              "t.Grantor, c.Column_priv, t.Table_priv "
              "FROM mysql.columns_priv AS c, mysql.tables_priv AS t "
              "WHERE c.Table_name = '");
  pos+= mysql_real_escape_string(mysql, pos, (char *)table, table_len);

  pos= strmov(pos, "' AND c.Db = ");
  if (catalog_len)
  {
    pos= strmov(pos, "'");
    pos+= mysql_real_escape_string(mysql, pos, (char *)catalog, catalog_len);
    pos= strmov(pos, "'");
  }
  else
    pos= strmov(pos, "DATABASE()");

  pos= strmov(pos, "AND c.Column_name LIKE '");
  pos+= mysql_real_escape_string(mysql, pos, (char *)column, column_len);

  pos= strmov(pos,
              "' AND c.Table_name = t.Table_name "
              "ORDER BY c.Db, c.Table_name, c.Column_name, c.Column_priv");

  if (mysql_query(mysql, buff))
    return NULL;

  return mysql_store_result(mysql);
}


char *SQLCOLUMNS_priv_values[]=
{
    NULL,"",NULL,NULL,NULL,NULL,NULL,NULL
};

MYSQL_FIELD SQLCOLUMNS_priv_fields[]=
{
  MYODBC_FIELD_STRING("TABLE_CAT", NAME_LEN, 0),
  MYODBC_FIELD_STRING("TABLE_SCHEM", NAME_LEN, 0),
  MYODBC_FIELD_STRING("TABLE_NAME", NAME_LEN, NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("COLUMN_NAME", NAME_LEN, NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("GRANTOR", NAME_LEN, 0),
  MYODBC_FIELD_STRING("GRANTEE", NAME_LEN, NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("PRIVILEGE", NAME_LEN, NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("IS_GRANTABLE", NAME_LEN, 0),
};


const uint SQLCOLUMNS_PRIV_FIELDS= array_elements(SQLCOLUMNS_priv_values);


SQLRETURN SQL_API
MySQLColumnPrivileges(SQLHSTMT hstmt,
                      SQLCHAR *catalog, SQLSMALLINT catalog_len,
                      SQLCHAR *schema __attribute__((unused)),
                      SQLSMALLINT schema_len __attribute__((unused)),
                      SQLCHAR *table, SQLSMALLINT table_len,
                      SQLCHAR *column, SQLSMALLINT column_len)
{
    STMT *stmt=(STMT *) hstmt;
    char     **row, **data;
    MEM_ROOT *alloc;
    uint     row_count;

    CLEAR_STMT_ERROR(hstmt);
    my_SQLFreeStmt(hstmt,MYSQL_RESET);

    if (catalog_len == SQL_NTS)
      catalog_len= catalog ? strlen((char *)catalog) : 0;
    if (table_len == SQL_NTS)
      table_len= table ? strlen((char *)table) : 0;
    if (column_len == SQL_NTS)
      column_len= column ? strlen((char *)column) : 0;

    pthread_mutex_lock(&stmt->dbc->lock);
    stmt->result= mysql_list_column_priv(&stmt->dbc->mysql,
                                         catalog, catalog_len,
                                         table, table_len,
                                         column, column_len);
    if (!stmt->result)
    {
      SQLRETURN rc= handle_connection_error(stmt);
      pthread_mutex_unlock(&stmt->dbc->lock);
      return rc;
    }
    pthread_mutex_unlock(&stmt->dbc->lock);

    stmt->result_array= (char **)my_malloc(sizeof(char *) *
                                           SQLCOLUMNS_PRIV_FIELDS *
                                           (ulong) stmt->result->row_count *
                                           MY_MAX_COLPRIV_COUNT,
                                           MYF(MY_ZEROFILL));
    if (!stmt->result_array)
    {
      set_mem_error(&stmt->dbc->mysql);
      return handle_connection_error(stmt);
    }
    alloc= &stmt->result->field_alloc;
    data= stmt->result_array;
    row_count= 0;
    while ( (row= mysql_fetch_row(stmt->result)) )
    {
        char  *grants= row[5];
        char  token[NAME_LEN+1];
        const char *grant= (const char *)grants;

        for ( ;; )
        {
            data[0]= row[0];
            data[1]= "";
            data[2]= row[2];
            data[3]= row[3];
            data[4]= row[4];
            data[5]= row[1];
            data[7]= is_grantable(row[6]) ? "YES":"NO";
            row_count++;

            if ( !(grant= my_next_token(grant,&grants,token,',')) )
            {
                /* End of grants .. */
                data[6]= strdup_root(alloc,grants);
                data+= SQLCOLUMNS_PRIV_FIELDS;
                break;
            }
            data[6]= strdup_root(alloc,token);
            data+= SQLCOLUMNS_PRIV_FIELDS;
        }
    }
    stmt->result->row_count= row_count;
    mysql_link_fields(stmt,SQLCOLUMNS_priv_fields,SQLCOLUMNS_PRIV_FIELDS);
    return SQL_SUCCESS;
}


/*
****************************************************************************
SQLSpecialColumns
****************************************************************************
*/

MYSQL_FIELD SQLSPECIALCOLUMNS_fields[]=
{
  MYODBC_FIELD_SHORT("SCOPE", 0),
  MYODBC_FIELD_STRING("COLUMN_NAME", NAME_LEN, NOT_NULL_FLAG),
  MYODBC_FIELD_SHORT("DATA_TYPE", NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("TYPE_NAME", 20, NOT_NULL_FLAG),
  MYODBC_FIELD_LONG("COLUMN_SIZE", 0),
  MYODBC_FIELD_LONG("BUFFER_LENGTH", 0),
  MYODBC_FIELD_LONG("DECIMAL_DIGITS", 0),
  MYODBC_FIELD_SHORT("PSEUDO_COLUMN", 0),
};

char *SQLSPECIALCOLUMNS_values[]= {
    0,NULL,0,NULL,0,0,0,0
};

const uint SQLSPECIALCOLUMNS_FIELDS= array_elements(SQLSPECIALCOLUMNS_fields);


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
    char        buff[80];
    char        **row;
    MYSQL_RES   *result;
    MYSQL_FIELD *field;
    MEM_ROOT    *alloc;
    uint        field_count;
    my_bool     primary_key;

    CLEAR_STMT_ERROR(hstmt);

    if (cbTableQualifier == SQL_NTS)
      cbTableQualifier= szTableQualifier ? strlen((char *)szTableQualifier) : 0;
    if (cbTableName == SQL_NTS)
      cbTableName= szTableName ? strlen((char *)szTableName) : 0;

    /* Reset the statement in order to avoid memory leaks when working with ADODB */
    my_SQLFreeStmt(hstmt, MYSQL_RESET);

    stmt->result= mysql_list_dbcolumns(stmt, szTableQualifier, cbTableQualifier,
                                       szTableName, cbTableName, NULL, 0);
    if (!(result= stmt->result))
    {
      return handle_connection_error(stmt);
    }

    if ( fColType == SQL_ROWVER )
    {
        /* Find possible timestamp */
        if ( !(stmt->result_array= (char**) my_malloc(sizeof(char*)*SQLSPECIALCOLUMNS_FIELDS*
                                                      result->field_count, MYF(MY_ZEROFILL))) )
        {
          set_mem_error(&stmt->dbc->mysql);
          return handle_connection_error(stmt);
        }

        /* Convert mysql fields to data that odbc wants */
        alloc= &result->field_alloc;
        field_count= 0;
        mysql_field_seek(result,0);
        for ( row= stmt->result_array;
            (field = mysql_fetch_field(result)); )
        {
            SQLSMALLINT type;
            if ((field->type != MYSQL_TYPE_TIMESTAMP))
              continue;
            /*
              TIMESTAMP_FLAG is only set on fields that are auto-set or
              auto-updated. We really only want auto-updated, but we can't
              tell the difference because of Bug #30081.
            */
            if (!(field->flags & TIMESTAMP_FLAG))
              continue;
            field_count++;
            row[0]= NULL;
            row[1]= field->name;
            type= get_sql_data_type(stmt, field, buff);
            row[3]= strdup_root(alloc,buff);
            sprintf(buff,"%d",type);
            row[2]= strdup_root(alloc,buff);
            fill_column_size_buff(buff, stmt, field);
            row[4]= strdup_root(alloc,buff);
            sprintf(buff, "%ld", get_transfer_octet_length(stmt, field));
            row[5]= strdup_root(alloc,buff);
            {
              SQLLEN digits= get_decimal_digits(stmt, field);
              if (digits != SQL_NO_TOTAL)
              {
                sprintf(buff,"%ld", digits);
                row[6]= strdup_root(alloc,buff);
              }
              else
                row[6]= NULL;
            }
            sprintf(buff,"%d",SQL_PC_NOT_PSEUDO);
            row[7]= strdup_root(alloc,buff);
            row+= SQLSPECIALCOLUMNS_FIELDS;
        }
        result->row_count= field_count;
        mysql_link_fields(stmt,SQLSPECIALCOLUMNS_fields,SQLSPECIALCOLUMNS_FIELDS);
        return SQL_SUCCESS;
    }

    if ( fColType != SQL_BEST_ROWID )
    {
        return set_error(stmt, MYERR_S1000,
                         "Unsupported argument to SQLSpecialColumns", 4000);
    }

    /*
     * The optimal set of columns for identifing a row is either
     * the primary key, or if there is no primary key, then
     * all the fields.
     */

    /* Check if there is a primary (unique) key */
    primary_key= 0;
    while ( (field= mysql_fetch_field(result)) )
    {
        if ( field->flags & PRI_KEY_FLAG )
        {
            primary_key=1;
            break;
        }
    }
    if ( !(stmt->result_array= (char**) my_malloc(sizeof(char*)*SQLSPECIALCOLUMNS_FIELDS*
                                                  result->field_count, MYF(MY_ZEROFILL))) )
    {
      set_mem_error(&stmt->dbc->mysql);
      return handle_connection_error(stmt);
    }

    /* Convert MySQL fields to data that odbc wants */
    alloc= &result->field_alloc;
    field_count= 0;
    mysql_field_seek(result,0);
    for ( row= stmt->result_array ;
        (field= mysql_fetch_field(result)); )
    {
        SQLSMALLINT type;
        if ( primary_key && !(field->flags & PRI_KEY_FLAG) )
            continue;
#ifndef SQLSPECIALCOLUMNS_RETURN_ALL_COLUMNS
        /* The ODBC 'standard' doesn't want us to return all columns if there is
           no primary or unique key */
        if ( !primary_key )
            continue;
#endif
        field_count++;
        sprintf(buff,"%d",SQL_SCOPE_SESSION);
        row[0]= strdup_root(alloc,buff);
        row[1]= field->name;
        type= get_sql_data_type(stmt, field, buff);
        row[3]= strdup_root(alloc,buff);
        sprintf(buff,"%d",type);
        row[2]= strdup_root(alloc,buff);
        fill_column_size_buff(buff, stmt, field);
        row[4]= strdup_root(alloc,buff);
        sprintf(buff,"%ld", get_transfer_octet_length(stmt, field));
        row[5]= strdup_root(alloc,buff);
        {
          SQLLEN digits= get_decimal_digits(stmt, field);
          if (digits != SQL_NO_TOTAL)
          {
            sprintf(buff,"%ld", digits);
            row[6]= strdup_root(alloc, buff);
          }
          else
            row[6]= NULL;
        }
        sprintf(buff,"%d",SQL_PC_NOT_PSEUDO);
        row[7]= strdup_root(alloc,buff);
        row+= SQLSPECIALCOLUMNS_FIELDS;
    }
    result->row_count= field_count;
    mysql_link_fields(stmt,SQLSPECIALCOLUMNS_fields,SQLSPECIALCOLUMNS_FIELDS);
    return SQL_SUCCESS;
}


/*
****************************************************************************
SQLPrimaryKeys
****************************************************************************
*/

MYSQL_FIELD SQLPRIM_KEYS_fields[]=
{
  MYODBC_FIELD_STRING("TABLE_CAT", NAME_LEN, 0),
  MYODBC_FIELD_STRING("TABLE_SCHEM", NAME_LEN, 0),
  MYODBC_FIELD_STRING("TABLE_NAME", NAME_LEN, NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("COLUMN_NAME", NAME_LEN, NOT_NULL_FLAG),
  MYODBC_FIELD_SHORT("KEY_SEQ", NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("PK_NAME", 128, 0),
};

const uint SQLPRIM_KEYS_FIELDS= array_elements(SQLPRIM_KEYS_fields);

char *SQLPRIM_KEYS_values[]= {
    NULL,"",NULL,NULL,0,NULL
};

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
    char      Qualifier_buff[NAME_LEN+1],Table_buff[NAME_LEN+1],
    *TableQualifier,*TableName;
    STMT FAR  *stmt= (STMT FAR*) hstmt;
    MYSQL_ROW row;
    char      **data;
    uint      row_count;

    CLEAR_STMT_ERROR(hstmt);
    my_SQLFreeStmt(hstmt,MYSQL_RESET);

    if (catalog_len == SQL_NTS)
      catalog_len= catalog ? strlen((char *)catalog) : 0;
    if (table_len == SQL_NTS)
      table_len= table ? strlen((char *)table) : 0;

    pthread_mutex_lock(&stmt->dbc->lock);
    if (!(stmt->result= mysql_list_dbkeys(stmt->dbc, catalog, catalog_len,
                                          table, table_len)))
    {
      SQLRETURN rc= handle_connection_error(stmt);
      pthread_mutex_unlock(&stmt->dbc->lock);
      return rc;
    }
    pthread_mutex_unlock(&stmt->dbc->lock);
    stmt->result_array= (char**) my_malloc(sizeof(char*)*SQLPRIM_KEYS_FIELDS*
                                           (ulong) stmt->result->row_count,
                                           MYF(MY_ZEROFILL));
    if (!stmt->result_array)
    {
      set_mem_error(&stmt->dbc->mysql);
      return handle_connection_error(stmt);
    }
    row_count= 0;
    data= stmt->result_array;
    while ( (row= mysql_fetch_row(stmt->result)) )
    {
        if ( row[1][0] == '0' )     /* If unique index */
        {
            if ( row_count && !strcmp(row[3],"1") )
                break;    /* Allready found unique key */
            row_count++;
            data[0]= data[1]=0;
            data[2]= row[0];
            data[3]= row[4];
            data[4]= row[3];
            data[5]= "PRIMARY";
            data+= SQLPRIM_KEYS_FIELDS;
        }
    }
    stmt->result->row_count= row_count;

    mysql_link_fields(stmt,SQLPRIM_KEYS_fields,SQLPRIM_KEYS_FIELDS);
    return SQL_SUCCESS;
}

/*
****************************************************************************
SQLForeignKeys
****************************************************************************
*/
MYSQL_FIELD SQLFORE_KEYS_fields[]=
{
  MYODBC_FIELD_STRING("PKTABLE_CAT", NAME_LEN, 0),
  MYODBC_FIELD_STRING("PKTABLE_SCHEM", NAME_LEN, 0),
  MYODBC_FIELD_STRING("PKTABLE_NAME", NAME_LEN, NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("PKCOLUMN_NAME", NAME_LEN, NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("FKTABLE_CAT", NAME_LEN, 0),
  MYODBC_FIELD_STRING("FKTABLE_SCHEM", NAME_LEN, 0),
  MYODBC_FIELD_STRING("FKTABLE_NAME", NAME_LEN, NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("FKCOLUMN_NAME", NAME_LEN, NOT_NULL_FLAG),
  MYODBC_FIELD_SHORT("KEY_SEQ", NOT_NULL_FLAG),
  MYODBC_FIELD_SHORT("UPDATE_RULE", 0),
  MYODBC_FIELD_SHORT("DELETE_RULE", 0),
  MYODBC_FIELD_STRING("FK_NAME", NAME_LEN, 0),
  MYODBC_FIELD_STRING("PK_NAME", NAME_LEN, 0),
  MYODBC_FIELD_SHORT("DEFERRABILITY", 0),
};

const uint SQLFORE_KEYS_FIELDS= array_elements(SQLFORE_KEYS_fields);

char *SQLFORE_KEYS_values[]= {
    NULL,"",NULL,NULL,
    NULL,"",NULL,NULL,
    0,0,0,NULL,NULL,0
};


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
    uint row_count= 0;

    CLEAR_STMT_ERROR(hstmt);
    my_SQLFreeStmt(hstmt,MYSQL_RESET);

    /* For 5.0 and later, use INFORMATION_SCHEMA. */
    if (is_minimum_version(stmt->dbc->mysql.server_version, "5.0", 3))
    {
      MYSQL *mysql= &stmt->dbc->mysql;
      char query[2048], *buff; /* This should be big enough. */
      char *update_rule, *delete_rule, *ref_constraints_join;
      SQLRETURN rc;

      /*
         With 5.1, we can use REFERENTIAL_CONSTRAINTS to get even more info.
      */
      if (is_minimum_version(stmt->dbc->mysql.server_version, "5.1", 3))
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
          " ON (R.CONSTRAINT_NAME = B.CONSTRAINT_NAME"
          " AND R.TABLE_NAME = B.TABLE_NAME"
          " AND R.CONSTRAINT_SCHEMA = B.TABLE_SCHEMA)";
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
                    "(SELECT CONSTRAINT_NAME FROM"
                    " INFORMATION_SCHEMA.TABLE_CONSTRAINTS"
                    " WHERE TABLE_SCHEMA = REFERENCED_TABLE_SCHEMA AND"
                    " TABLE_NAME = A.REFERENCED_TABLE_NAME AND"
                    " CONSTRAINT_TYPE IN ('UNIQUE','PRIMARY KEY') LIMIT 1)"
                    " AS PK_NAME,"
                    "7 AS DEFERRABILITY"
                    " FROM INFORMATION_SCHEMA.KEY_COLUMN_USAGE A"
                    " JOIN INFORMATION_SCHEMA.TABLE_CONSTRAINTS B"
                    " USING (CONSTRAINT_NAME,TABLE_NAME)",
                    ref_constraints_join,
                    " WHERE B.CONSTRAINT_TYPE = 'FOREIGN KEY' ",
                    NullS);

      if (szPkTableName && szPkTableName[0])
      {
        buff= strmov(buff, "AND A.REFERENCED_TABLE_SCHEMA = ");
        if (szPkCatalogName && szPkCatalogName[0])
        {
          if (cbPkCatalogName == SQL_NTS)
            cbPkCatalogName= strlen((char *)szPkCatalogName);
          buff= strmov(buff, "'");
          buff+= mysql_real_escape_string(mysql, buff, (char *)szPkCatalogName,
                                          cbPkCatalogName);
          buff= strmov(buff, "' ");
        }
        else
          buff= strmov(buff, "DATABASE() ");
        buff= strmov(buff, "AND A.REFERENCED_TABLE_NAME = '");
        if (cbPkTableName == SQL_NTS)
          cbPkTableName= strlen((char *)szPkTableName);
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
          if (cbFkCatalogName == SQL_NTS)
            cbFkCatalogName= strlen((char *)szFkCatalogName);
          buff= strmov(buff, "'");
          buff+= mysql_real_escape_string(mysql, buff, (char *)szFkCatalogName,
                                          cbFkCatalogName);
          buff= strmov(buff, "' ");
        }
        else
          buff= strmov(buff, "DATABASE() ");
        buff= strmov(buff, "AND A.TABLE_NAME = '");
        if (cbFkTableName == SQL_NTS)
          cbFkTableName= strlen((char *)szFkTableName);
        buff+= mysql_real_escape_string(mysql, buff, (char *)szFkTableName,
                                        cbFkTableName);
        buff= strmov(buff, "' ");

        strmov(buff, "ORDER BY FKTABLE_CAT, FKTABLE_NAME, "
                     "KEY_SEQ, PKTABLE_NAME");
      }

      rc= MySQLPrepare(hstmt, (SQLCHAR *)query, SQL_NTS, FALSE);
      if (!SQL_SUCCEEDED(rc))
        return rc;

      return my_SQLExecute(hstmt);
    }
    /* For 3.23 and later, use comment in SHOW TABLE STATUS (yuck). */
    else if (is_minimum_version(stmt->dbc->mysql.server_version,"3.23",4))
    {
        MEM_ROOT  *alloc;
        MYSQL_ROW row;
        char      **data;
        char      **tempdata; /* We need this array for the cases if key count is greater than 18 */
        uint       comment_id;

        CLEAR_STMT_ERROR(hstmt);

        if (cbPkTableName == SQL_NTS && szPkTableName)
          cbPkTableName= strlen((char *)szPkTableName);

        pthread_mutex_lock(&stmt->dbc->lock);
        if (!(stmt->result= mysql_table_status(stmt,
                                               szFkCatalogName, cbFkCatalogName,
                                               szFkTableName, cbFkTableName,
                                               FALSE)))
        {
          if (mysql_errno(&stmt->dbc->mysql))
          {
            SQLRETURN rc= handle_connection_error(stmt);
            pthread_mutex_unlock(&stmt->dbc->lock);
            return rc;
          }
          else
          {
            pthread_mutex_unlock(&stmt->dbc->lock);
            goto empty_set;
          }
          pthread_mutex_unlock(&stmt->dbc->lock);
        }
        pthread_mutex_unlock(&stmt->dbc->lock);
        tempdata= (char**) my_malloc(sizeof(char*)*SQLFORE_KEYS_FIELDS*
                                               64, /* Maximum index count */
                                               MYF(MY_ZEROFILL));
        if (!tempdata)
        {
          set_mem_error(&stmt->dbc->mysql);
          return handle_connection_error(stmt);
        }

        /* Convert mysql fields to data that odbc wants */
        alloc= &stmt->result->field_alloc;
        data= tempdata;
        comment_id= stmt->result->field_count - 1;

        while ( (row= mysql_fetch_row(stmt->result)) )
        {
            if ( (row[1] && strcmp(row[1],"InnoDB")==0) )
            {
                const char *token,*pktoken,*fk_cols_start,*pk_cols_start;
                char       *comment_token, ref_token[NAME_LEN+1];
                char       *pkcomment,*fkcomment;
                uint       key_seq,pk_length,fk_length;

                if ( !(comment_token= strchr(row[comment_id],';')) )
                    continue; /* MySQL 4.1 and above, the comment field is '15' */

                do
                {
                    /*
                      Found reference information in comment field from
                      InnoDB type, and parse the same to get the FK
                      information ..
                    */
                    key_seq= 1;

                    if ( !(token= my_next_token(NULL,&comment_token,NULL,'(')) )
                        break;
                    fk_cols_start = token + 1;

                    if ( !(token= my_next_token(token,&comment_token,ref_token,')')) )
                        continue;
                    fk_length= (uint)((token-2)-fk_cols_start);

                    if ( !(token= my_next_token(token+8,&comment_token,ref_token,'/')) )
                        continue;

                    data[0]= strdup_root(alloc,ref_token); /* PKTABLE_CAT */

                    if (!(token= my_next_token(token, &comment_token,
                                               ref_token, '(')) ||
                         (szPkTableName &&
                          myodbc_casecmp((char *)szPkTableName, ref_token,
                                         cbPkTableName)))
                        continue;

                    ref_token[strlen(ref_token)- 1] = 0;   /* Remove last quot character */
                    data[2]= strdup_root(alloc,ref_token); /* PKTABLE_TABLE */
                    pk_cols_start = token + 1;

                    if ( !(token= my_next_token(token,&comment_token,ref_token,')')) )
                        continue;
                    pk_length= (uint)((token-2)-pk_cols_start);

                    data[1]= NULL;                         /* PKTABLE_SCHEM */

                    /**
                      @todo clean this up when current database tracking is
                      better
                    */
                    if (!szFkCatalogName && !stmt->dbc->database)
                      reget_current_catalog(stmt->dbc);

                    /* FKTABLE_CAT */
                    data[4]= (szFkCatalogName ?
                              strdup_root(alloc, (char *)szFkCatalogName) :
                              strdup_root(alloc, stmt->dbc->database));
                    data[5]= NULL;                         /* FKTABLE_SCHEM */
                    data[6]= row[0];                       /* FKTABLE_TABLE */

                    /*
                       We could figure out UPDATE_RULE and DELETE_RULE by
                       parsing the comment field. For now, we just return
                       SQL_CASCADE>
                    */
                    data[9]=  "1"; /*SQL_CASCADE*/        /* UPDATE_RULE */
                    data[10]= "1"; /*SQL_CASCADE*/        /* DELETE_RULE */
                    data[11]= NULL;                       /* FK_NAME */
                    data[12]= NULL;                       /* PK_NAME */
                    data[13]= "7"; /*SQL_NOT_DEFERRABLE*/ /* DEFERRABILITY */

                    token = fkcomment = (char *)fk_cols_start;
                    pktoken = pkcomment = (char *)pk_cols_start;
                    fkcomment[fk_length]= '\0';
                    pkcomment[pk_length]= '\0';

                    while ( (token= my_next_token(token,&fkcomment,ref_token,' ')) )
                    {
                        /* Multiple columns exists .. parse them to individual rows */
                        char **prev_data= data;
                        data[7]= strdup_root(alloc,ref_token);    /* FKTABLE_COLUMN */
                        pktoken= my_next_token(pktoken,&pkcomment,ref_token,' ');
                        data[3]= strdup_root(alloc,ref_token);    /* PKTABLE_COLUMN */
                        sprintf(ref_token,"%d",key_seq++);
                        data[8]= strdup_root(alloc,ref_token);    /* KEY_SEQ */
                        data+= SQLFORE_KEYS_FIELDS;
                        row_count++;
                        for ( fk_length= SQLFORE_KEYS_FIELDS; fk_length--; )
                            data[fk_length]= prev_data[fk_length];
                    }
                    data[7]= strdup_root(alloc,fkcomment);      /* FKTABLE_COLUMN */
                    data[3]= strdup_root(alloc,pkcomment);      /* PKTABLE_COLUMN */
                    sprintf(ref_token,"%d",key_seq);
                    data[8]= strdup_root(alloc,ref_token);      /* KEY_SEQ */

                    data+= SQLFORE_KEYS_FIELDS;
                    row_count++;

                } while ( (comment_token = strchr(comment_token,';')) );/* multi table ref */
            }
        }

        /* Copy only the elements that contain fk names */
        stmt->result_array= (MYSQL_ROW)my_memdup((char *)tempdata,
                                                 sizeof(char *) *
                                                 SQLFORE_KEYS_FIELDS *
                                                 row_count,
                                                 MYF(0));
        my_free((char *)tempdata, MYF(0));
        if (!stmt->result_array)
        {
          set_mem_error(&stmt->dbc->mysql);
          return handle_connection_error(stmt);
        }
    }
    /* Versions older than 3.23 don't support foreign keys at all. */
    else
    {
      goto empty_set;
    }
    stmt->result->row_count= row_count;
    mysql_link_fields(stmt,SQLFORE_KEYS_fields,SQLFORE_KEYS_FIELDS);
    return SQL_SUCCESS;

empty_set:
  return create_empty_fake_resultset(stmt, SQLFORE_KEYS_values,
                                     sizeof(SQLFORE_KEYS_values),
                                     SQLFORE_KEYS_fields,
                                     SQLFORE_KEYS_FIELDS);
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
  if (!is_minimum_version(stmt->dbc->mysql.server_version, "5.0", 3))
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
      cbProcName= strlen((const char *)szProcName);
    rc= my_SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_C_CHAR,
                            0, 0, szProcName, cbProcName, NULL);
    if (!SQL_SUCCEEDED(rc))
      return rc;
  }

  if (szCatalogName)
  {
    if (cbCatalogName == SQL_NTS)
      cbCatalogName= strlen((const char *)szCatalogName);
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

char *SQLPROCEDURECOLUMNS_values[]= {
       "", "", NullS, NullS, "", "", "",
       "", "", "", "10", "",
       "MySQL column", "", "", NullS, "",
       NullS, ""
};

MYSQL_FIELD SQLPROCEDURECOLUMNS_fields[]=
{
  MYODBC_FIELD_STRING("PROCEDURE_CAT",     NAME_LEN, 0),
  MYODBC_FIELD_STRING("PROCEDURE_SCHEM",   NAME_LEN, 0),
  MYODBC_FIELD_STRING("PROCEDURE_NAME",    NAME_LEN, NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("COLUMN_NAME",       NAME_LEN, NOT_NULL_FLAG),
  MYODBC_FIELD_SHORT ("COLUMN_TYPE",       NOT_NULL_FLAG),
  MYODBC_FIELD_SHORT ("DATA_TYPE",         NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("TYPE_NAME",         20,       NOT_NULL_FLAG),
  MYODBC_FIELD_LONG  ("COLUMN_SIZE",       0),
  MYODBC_FIELD_LONG  ("BUFFER_LENGTH",     0),
  MYODBC_FIELD_SHORT ("DECIMAL_DIGITS",    0),
  MYODBC_FIELD_SHORT ("NUM_PREC_RADIX",    0),
  MYODBC_FIELD_SHORT ("NULLABLE",          NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("REMARKS",           NAME_LEN, 0),
  MYODBC_FIELD_STRING("COLUMN_DEF",        NAME_LEN, 0),
  MYODBC_FIELD_SHORT ("SQL_DATA_TYPE",     NOT_NULL_FLAG),
  MYODBC_FIELD_SHORT ("SQL_DATETIME_SUB",  0),
  MYODBC_FIELD_LONG  ("CHAR_OCTET_LENGTH", 0),
  MYODBC_FIELD_LONG  ("ORDINAL_POSITION",  NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("IS_NULLABLE",       3,        0),
};

const uint SQLPROCEDURECOLUMNS_FIELDS=
             array_elements(SQLPROCEDURECOLUMNS_fields);

/*
  @type    : ODBC 1.0 API
  @purpose : returns the list of input and output parameters, as well as
  the columns that make up the result set for the specified
  procedures. The driver returns the information as a result
  set on the specified statement
*/

SQLRETURN SQL_API
MySQLProcedureColumns(SQLHSTMT hstmt,
                    SQLCHAR FAR *szProcQualifier __attribute__((unused)),
                    SQLSMALLINT cbProcQualifier __attribute__((unused)),
                    SQLCHAR FAR *szProcOwner __attribute__((unused)),
                    SQLSMALLINT cbProcOwner __attribute__((unused)),
                    SQLCHAR FAR *szProcName __attribute__((unused)),
                    SQLSMALLINT cbProcName __attribute__((unused)),
                    SQLCHAR FAR *szColumnName __attribute__((unused)),
                    SQLSMALLINT cbColumnName __attribute__((unused)))
{
  create_empty_fake_resultset(hstmt, SQLPROCEDURECOLUMNS_values,
                              sizeof(SQLPROCEDURECOLUMNS_values),
                              SQLPROCEDURECOLUMNS_fields,
                              SQLPROCEDURECOLUMNS_FIELDS);

  return set_error(hstmt, MYERR_01000,
                   "MySQL server does not provide the requested information",
                   4000);
}
