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
  @file   catalog_no_i_s.c
  @brief  Catalog functions not using I_S.
  @remark All functions suppose that parameters specifying other parameters lenthes can't SQL_NTS.
          caller should take care of that.
*/

#include "driver.h"
#include "catalog.h"


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
        while ( isspace(*(table_type)) ) ++table_type;
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
        while ( isspace(*(table_type)) ) ++table_type;
        if ( !myodbc_casecmp(table_type,req_type,len) || 
             !myodbc_casecmp(table_type,req_type_quoted,len+2) ||
             !myodbc_casecmp(table_type,req_type_quoted1,len+2) )
            found= 1;
    }
    return found;
}


static MYSQL_ROW fix_fields_copy(STMT *stmt,MYSQL_ROW row)
{
    uint i;
    for ( i=0 ; i < stmt->order_count; ++i )
        stmt->array[stmt->order[i]]= row[i];
    return stmt->array;
}


/*
  @type    : internal
  @purpose : returns columns from a particular table, NULL on error
*/
static MYSQL_RES *server_list_dbkeys(STMT *stmt,
                                     SQLCHAR *catalog,
                                     SQLSMALLINT catalog_len,
                                     SQLCHAR *table,
                                     SQLSMALLINT table_len)
{
    DBC   *dbc = stmt->dbc;
    MYSQL *mysql= &dbc->mysql;
    char  buff[255 + 4 * NAME_LEN], *to;

    to= myodbc_stpmov(buff, "SHOW KEYS FROM `");
    if (catalog_len)
    {
      to+= myodbc_escape_string(mysql, to, (ulong)(sizeof(buff) - (to - buff)),
                                (char *)catalog, catalog_len, 1);
      to= myodbc_stpmov(to, "`.`");
    }
    to+= myodbc_escape_string(mysql, to, (ulong)(sizeof(buff) - (to - buff)),
                              (char *)table, table_len, 1);
    to= myodbc_stpmov(to, "`");

    MYLOG_DBC_QUERY(dbc, buff);
    if (exec_stmt_query(stmt, buff, strlen(buff), FALSE))
        return NULL;
    return mysql_store_result(mysql);
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
  MYODBC_FIELD_NAME("TABLE_CAT", 0),
  MYODBC_FIELD_NAME("TABLE_SCHEM", 0),
  MYODBC_FIELD_NAME("TABLE_NAME", NOT_NULL_FLAG),
  MYODBC_FIELD_NAME("COLUMN_NAME", NOT_NULL_FLAG),
  MYODBC_FIELD_SHORT("DATA_TYPE", NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("TYPE_NAME", 20, NOT_NULL_FLAG),
  MYODBC_FIELD_LONG("COLUMN_SIZE", 0),
  MYODBC_FIELD_LONG("BUFFER_LENGTH", 0),
  MYODBC_FIELD_SHORT("DECIMAL_DIGITS", 0),
  MYODBC_FIELD_SHORT("NUM_PREC_RADIX", 0),
  MYODBC_FIELD_SHORT("NULLABLE", NOT_NULL_FLAG),
  MYODBC_FIELD_NAME("REMARKS", 0),
  MYODBC_FIELD_NAME("COLUMN_DEF", 0),
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

  @return Result of mysql_list_fields, or NULL if there is an error
*/
static MYSQL_RES *
server_list_dbcolumns(STMT *stmt,
                      SQLCHAR *szCatalog, SQLSMALLINT cbCatalog,
                      SQLCHAR *szTable, SQLSMALLINT cbTable,
                      SQLCHAR *szColumn, SQLSMALLINT cbColumn)
{
  DBC *dbc= stmt->dbc;
  MYSQL *mysql= &dbc->mysql;
  MYSQL_RES *result;
  char buff[NAME_LEN * 2 + 64], column_buff[NAME_LEN * 2 + 64];

  /* If a catalog was specified, we have to change working catalog
     to be able to use mysql_list_fields. */
  if (cbCatalog)
  {
    if (reget_current_catalog(dbc))
      return NULL;

    /* reget_current_catalog locks and release mutex, so locking
       here again */
    myodbc_mutex_lock(&dbc->lock);

    strncpy(buff, szCatalog, cbCatalog);
    buff[cbCatalog]= '\0';

    if (mysql_select_db(mysql, buff))
    {
      myodbc_mutex_unlock(&dbc->lock);
      return NULL;
    }
  }
  else
    myodbc_mutex_lock(&dbc->lock);

  strncpy(buff, szTable, cbTable);
  buff[cbTable]= '\0';
  strncpy(column_buff, szColumn, cbColumn);
  column_buff[cbColumn]= '\0';

  result= mysql_list_fields(mysql, buff, column_buff);

  /* If before this call no database were selected - we cannot revert that */
  if (cbCatalog && dbc->database)
  {
    if (mysql_select_db( mysql, dbc->database))
    {
      /* Well, probably have to return error here */
      mysql_free_result(result);
      myodbc_mutex_unlock(&dbc->lock);
      return NULL;
    }
  }
  myodbc_mutex_unlock(&dbc->lock);

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
SQLRETURN
columns_no_i_s(STMT * stmt, SQLCHAR *szCatalog, SQLSMALLINT cbCatalog,
               SQLCHAR *szSchema __attribute__((unused)),
               SQLSMALLINT cbSchema __attribute__((unused)),
               SQLCHAR *szTable, SQLSMALLINT cbTable,
               SQLCHAR *szColumn, SQLSMALLINT cbColumn)

{
  MYSQL_RES *res;
  MEM_ROOT *alloc;
  MYSQL_ROW table_row;
  unsigned long rows= 0, next_row= 0, *lengths;
  char *db= NULL;
  BOOL is_access= FALSE;

  if (cbColumn > NAME_LEN || cbTable > NAME_LEN || cbCatalog > NAME_LEN)
  {
    return set_stmt_error(stmt, "HY090", "Invalid string or buffer length", 4001);
  }

  /* Get the list of tables that match szCatalog and szTable */
  myodbc_mutex_lock(&stmt->dbc->lock);
  res= table_status(stmt, szCatalog, cbCatalog, szTable, cbTable, TRUE,
                    TRUE, TRUE);

  if (!res && mysql_errno(&stmt->dbc->mysql))
  {
    SQLRETURN rc= handle_connection_error(stmt);
    myodbc_mutex_unlock(&stmt->dbc->lock);
    return rc;
  }
  else if (!res)
  {
    myodbc_mutex_unlock(&stmt->dbc->lock);
    goto empty_set;
  }
  myodbc_mutex_unlock(&stmt->dbc->lock);

#ifdef _WIN32
  if (GetModuleHandle("msaccess.exe") != NULL)
    is_access= TRUE;
#endif

  stmt->result= res;
  alloc= &stmt->alloc_root;

  if (!stmt->dbc->ds->no_catalog)
    db= strmake_root(alloc, (char *)szCatalog, cbCatalog);

  while ((table_row= mysql_fetch_row(res)))
  {
    MYSQL_FIELD *field;
    MYSQL_RES *table_res;
    int count= 0;

    /* Get list of columns matching szColumn for each table. */
    lengths= mysql_fetch_lengths(res);
    table_res= server_list_dbcolumns(stmt, szCatalog, cbCatalog,
                                     (SQLCHAR *)table_row[0],
                                     (SQLSMALLINT)lengths[0],
                                     szColumn, cbColumn);

    if (!table_res)
    {
      return handle_connection_error(stmt);
    }

    rows+= mysql_num_fields(table_res);

    stmt->result_array= (char **)myodbc_realloc((char *)stmt->result_array,
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
      {
        row[15]= strdup_root(alloc, buff); /* CHAR_OCTET_LENGTH */
      }
      else
      {
        row[15]= NULL;                     /* CHAR_OCTET_LENGTH */
      }

      {
        SQLSMALLINT digits= get_decimal_digits(stmt, field);
        if (digits != SQL_NO_TOTAL)
        {
          sprintf(buff, "%d", digits);
          row[8]= strdup_root(alloc, buff);  /* DECIMAL_DIGITS */
          row[9]= "10";                      /* NUM_PREC_RADIX */
        }
        else
        {
          row[8]= row[9]= NullS;             /* DECIMAL_DIGITS, NUM_PREC_RADIX */
        }
      }

      /*
        If a field is a TIMESTAMP, NULL can be stored to it (although it gets turned into
        something else).

        The same logic applies to fields with AUTO_INCREMENT_FLAG set.
      */
      if ((field->flags & NOT_NULL_FLAG) && !(field->type == MYSQL_TYPE_TIMESTAMP) &&
          !(field->flags & AUTO_INCREMENT_FLAG))
      {
        /* Bug#31067. Access seems to try to put NULL value when not null field
           is cleared. And that contradicts with its knowledge of that the field
           is not nullable, and it yields an error. Here is a little trick for
           such case - we don't tell Access the whole truth we know, and
           return for such field SQL_NULLABLE_UNKNOWN instead*/
        if (is_access)
        {
          sprintf(buff, "%d", SQL_NULLABLE_UNKNOWN);
          row[10]= strdup_root(alloc, buff); /* NULLABLE */
          row[17]= strdup_root(alloc, "NO");/* IS_NULLABLE */
        }
        else
        {
          sprintf(buff, "%d", SQL_NO_NULLS);
          row[10]= strdup_root(alloc, buff); /* NULLABLE */
          row[17]= strdup_root(alloc, "NO"); /* IS_NULLABLE */
        }
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
        {
          row[12]= NullS; /* COLUMN_DEF */
        }
        else
        {
          char *def= alloc_root(alloc, strlen(field->def) + 3);
          if (is_numeric_mysql_type(field))
          {
            sprintf(def, "%s", field->def);
          }
          else
          {
            sprintf(def, "'%s'", field->def);
          }
          row[12]= def; /* COLUMN_DEF */
        }
      }

      sprintf(buff, "%d", ++count);
      row[16]= strdup_root(alloc, buff); /* ORDINAL_POSITION */
    }

    mysql_free_result(table_res);
  }

  set_row_count(stmt, rows);
  myodbc_link_fields(stmt, SQLCOLUMNS_fields, SQLCOLUMNS_FIELDS);

  return SQL_SUCCESS;

empty_set:
  return create_empty_fake_resultset(stmt, SQLCOLUMNS_values,
                                     sizeof(SQLCOLUMNS_values),
                                     SQLCOLUMNS_fields,
                                     SQLCOLUMNS_FIELDS);
}


/****************************************************************************
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
@purpose : returns a table privileges result, NULL on error. Uses mysql db tables
*/
static MYSQL_RES *table_privs_raw_data( STMT *      stmt,
                                        SQLCHAR *   catalog,
                                        SQLSMALLINT catalog_len,
                                        SQLCHAR *   table,
                                        SQLSMALLINT table_len)
{
  DBC *dbc= stmt->dbc;
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
    pos= myodbc_stpmov(pos, "'");
    pos+= mysql_real_escape_string(mysql, pos, (char *)catalog, catalog_len);
    pos= myodbc_stpmov(pos, "'");
  }
  else
    pos= myodbc_stpmov(pos, "DATABASE()");

  pos= strxmov(pos, " ORDER BY Db, Table_name, Table_priv, User", NullS);

  MYLOG_DBC_QUERY(dbc, buff);
  if (exec_stmt_query(stmt, buff, strlen(buff), FALSE))
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
  MYODBC_FIELD_NAME("TABLE_CAT", 0),
  MYODBC_FIELD_NAME("TABLE_SCHEM", 0),
  MYODBC_FIELD_NAME("TABLE_NAME", NOT_NULL_FLAG),
  MYODBC_FIELD_NAME("GRANTOR", 0),
  MYODBC_FIELD_NAME("GRANTEE", NOT_NULL_FLAG),
  MYODBC_FIELD_NAME("PRIVILEGE", NOT_NULL_FLAG),
  MYODBC_FIELD_NAME("IS_GRANTABLE", 0),
};

const uint SQLTABLES_PRIV_FIELDS= array_elements(SQLTABLES_priv_values);

/*
  @type    : ODBC 1.0 API
  @purpose : returns a list of tables and the privileges associated with
             each table. The driver returns the information as a result
             set on the specified statement.
*/

SQLRETURN
list_table_priv_no_i_s(SQLHSTMT hstmt,
                       SQLCHAR *catalog, SQLSMALLINT catalog_len,
                       SQLCHAR *schema __attribute__((unused)),
                       SQLSMALLINT schema_len __attribute__((unused)),
                       SQLCHAR *table, SQLSMALLINT table_len)
{
    STMT     *stmt= (STMT *)hstmt;

    char     **data, **row;
    MEM_ROOT *alloc;
    uint     row_count;

    myodbc_mutex_lock(&stmt->dbc->lock);
    stmt->result= table_privs_raw_data(stmt, catalog, catalog_len,
      table, table_len);

    if (!stmt->result)
    {
      SQLRETURN rc= handle_connection_error(stmt);
      myodbc_mutex_unlock(&stmt->dbc->lock);
      return rc;
    }
    myodbc_mutex_unlock(&stmt->dbc->lock);

    /* Allocate max buffers, to avoid reallocation */
    stmt->result_array= (char**) myodbc_malloc(sizeof(char*)* SQLTABLES_PRIV_FIELDS *
      (ulong)stmt->result->row_count *
      MY_MAX_TABPRIV_COUNT,
      MYF(MY_ZEROFILL));

    if (!stmt->result_array)
    {
      set_mem_error(&stmt->dbc->mysql);
      return handle_connection_error(stmt);
    }

    alloc= &stmt->alloc_root;
    data= stmt->result_array;
    row_count= 0;

    while ( (row= mysql_fetch_row(stmt->result)) )
    {
      const char  *grants= row[4];
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
            ++row_count;

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

    set_row_count(stmt, row_count);
    myodbc_link_fields(stmt,SQLTABLES_priv_fields,SQLTABLES_PRIV_FIELDS);

    return SQL_SUCCESS;
}


/*
****************************************************************************
SQLColumnPrivileges
****************************************************************************
*/
/*
@type    : internal
@purpose : returns a column privileges result, NULL on error
*/
static MYSQL_RES *column_privs_raw_data(STMT *      stmt,
                                        SQLCHAR *   catalog,
                                        SQLSMALLINT catalog_len,
                                        SQLCHAR *   table,
                                        SQLSMALLINT table_len,
                                        SQLCHAR *   column,
                                        SQLSMALLINT column_len)
{
  DBC   *dbc = stmt->dbc;
  MYSQL *mysql = &dbc->mysql;

  char buff[400+6*NAME_LEN+1], *pos;

  pos= myodbc_stpmov(buff,
    "SELECT c.Db, c.User, c.Table_name, c.Column_name,"
    "t.Grantor, c.Column_priv, t.Table_priv "
    "FROM mysql.columns_priv AS c, mysql.tables_priv AS t "
    "WHERE c.Table_name = '");
  pos+= mysql_real_escape_string(mysql, pos, (char *)table, table_len);

  pos= myodbc_stpmov(pos, "' AND c.Db = ");
  if (catalog_len)
  {
    pos= myodbc_stpmov(pos, "'");
    pos+= mysql_real_escape_string(mysql, pos, (char *)catalog, catalog_len);
    pos= myodbc_stpmov(pos, "'");
  }
  else
    pos= myodbc_stpmov(pos, "DATABASE()");

  pos= myodbc_stpmov(pos, "AND c.Column_name LIKE '");
  pos+= mysql_real_escape_string(mysql, pos, (char *)column, column_len);

  pos= myodbc_stpmov(pos,
    "' AND c.Table_name = t.Table_name "
    "ORDER BY c.Db, c.Table_name, c.Column_name, c.Column_priv");

  if (exec_stmt_query(stmt, buff, strlen(buff), FALSE))
    return NULL;

  return mysql_store_result(mysql);
}


char *SQLCOLUMNS_priv_values[]=
{
  NULL,"",NULL,NULL,NULL,NULL,NULL,NULL
};

MYSQL_FIELD SQLCOLUMNS_priv_fields[]=
{
  MYODBC_FIELD_NAME("TABLE_CAT", 0),
  MYODBC_FIELD_NAME("TABLE_SCHEM", 0),
  MYODBC_FIELD_NAME("TABLE_NAME", NOT_NULL_FLAG),
  MYODBC_FIELD_NAME("COLUMN_NAME", NOT_NULL_FLAG),
  MYODBC_FIELD_NAME("GRANTOR", 0),
  MYODBC_FIELD_NAME("GRANTEE", NOT_NULL_FLAG),
  MYODBC_FIELD_NAME("PRIVILEGE", NOT_NULL_FLAG),
  MYODBC_FIELD_NAME("IS_GRANTABLE", 0),
};


const uint SQLCOLUMNS_PRIV_FIELDS= array_elements(SQLCOLUMNS_priv_values);


SQLRETURN 
list_column_priv_no_i_s(SQLHSTMT hstmt,
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

  myodbc_mutex_lock(&stmt->dbc->lock);
  stmt->result= column_privs_raw_data(stmt,
    catalog, catalog_len,
    table, table_len,
    column, column_len);
  if (!stmt->result)
  {
    SQLRETURN rc= handle_connection_error(stmt);
    myodbc_mutex_unlock(&stmt->dbc->lock);
    return rc;
  }
  myodbc_mutex_unlock(&stmt->dbc->lock);

  stmt->result_array= (char **)myodbc_malloc(sizeof(char *) *
    SQLCOLUMNS_PRIV_FIELDS *
    (ulong) stmt->result->row_count *
    MY_MAX_COLPRIV_COUNT,
    MYF(MY_ZEROFILL));
  if (!stmt->result_array)
  {
    set_mem_error(&stmt->dbc->mysql);
    return handle_connection_error(stmt);
  }
  alloc= &stmt->alloc_root;
  data= stmt->result_array;
  row_count= 0;
  while ( (row= mysql_fetch_row(stmt->result)) )
  {
    const char  *grants= row[5];
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
            ++row_count;

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
  set_row_count(stmt, row_count);
  myodbc_link_fields(stmt,SQLCOLUMNS_priv_fields,SQLCOLUMNS_PRIV_FIELDS);
  return SQL_SUCCESS;
}


/**
Get the table status for a table or tables using SHOW TABLE STATUS.
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
MYSQL_RES *table_status_no_i_s(STMT        *stmt,
                               SQLCHAR     *catalog,
                               SQLSMALLINT  catalog_length,
                               SQLCHAR     *table,
                               SQLSMALLINT  table_length,
                               my_bool      wildcard)
{
	MYSQL *mysql= &stmt->dbc->mysql;
	/** @todo determine real size for buffer */
	char buff[36 + 4*NAME_LEN + 1], *to;

	to= myodbc_stpmov(buff, "SHOW TABLE STATUS ");
	if (catalog && *catalog)
	{
		to= myodbc_stpmov(to, "FROM `");
		to+= myodbc_escape_string(mysql, to, (ulong)(sizeof(buff) - (to - buff)),
			(char *)catalog, catalog_length, 1);
		to= myodbc_stpmov(to, "` ");
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
		to= myodbc_stpmov(to, "LIKE '");
		if (wildcard)
			to+= mysql_real_escape_string(mysql, to, (char *)table, table_length);
		else
			to+= myodbc_escape_string(mysql, to, (ulong)(sizeof(buff) - (to - buff)),
			(char *)table, table_length, 0);
		to= myodbc_stpmov(to, "'");
	}

  MYLOG_QUERY(stmt, buff);

  assert(to - buff < sizeof(buff));

  if (exec_stmt_query(stmt, buff, (unsigned long)(to - buff), FALSE))
  {
    return NULL;
  }

  return mysql_store_result(mysql);
}


/**
Get the CREATE TABLE statement for the given table.
Lengths may not be SQL_NTS.

@param[in] stmt           Handle to statement
@param[in] catalog        Catalog (database) of table, @c NULL for current
@param[in] catalog_length Length of catalog name
@param[in] table          Name of table
@param[in] table_length   Length of table name

@return Result of SHOW CREATE TABLE , or NULL if there is an error
or empty result (check mysql_errno(&stmt->dbc->mysql) != 0)
*/
MYSQL_RES *server_show_create_table(STMT        *stmt,
                                    SQLCHAR     *catalog,
                                    SQLSMALLINT  catalog_length,
                                    SQLCHAR     *table,
                                    SQLSMALLINT  table_length)
{
  MYSQL *mysql= &stmt->dbc->mysql;
  /** @todo determine real size for buffer */
  char buff[36 + 4*NAME_LEN + 1], *to;

  to= myodbc_stpmov(buff, "SHOW CREATE TABLE ");
  if (catalog && *catalog)
  {
    to= myodbc_stpmov(to, " `");
    to= myodbc_stpmov(to, (char *)catalog);
    to= myodbc_stpmov(to, "`.");
  }

  /* Empty string won't match anything. */
  if (!*table)
    return NULL;

  if (table && *table)
  {
    to= myodbc_stpmov(to, " `");
    to= myodbc_stpmov(to, (char *)table);
    to= myodbc_stpmov(to, "`");
  }

  MYLOG_QUERY(stmt, buff);

  assert(to - buff < sizeof(buff));

  if (mysql_real_query(mysql,buff,(unsigned long)(to - buff)))
  {
    return NULL;
  }

  return mysql_store_result(mysql);
}


/*
****************************************************************************
SQLForeignKeys
****************************************************************************
*/
MYSQL_FIELD SQLFORE_KEYS_fields[]=
{
  MYODBC_FIELD_NAME("PKTABLE_CAT", 0),
  MYODBC_FIELD_NAME("PKTABLE_SCHEM", 0),
  MYODBC_FIELD_NAME("PKTABLE_NAME", NOT_NULL_FLAG),
  MYODBC_FIELD_NAME("PKCOLUMN_NAME", NOT_NULL_FLAG),
  MYODBC_FIELD_NAME("FKTABLE_CAT", 0),
  MYODBC_FIELD_NAME("FKTABLE_SCHEM", 0),
  MYODBC_FIELD_NAME("FKTABLE_NAME", NOT_NULL_FLAG),
  MYODBC_FIELD_NAME("FKCOLUMN_NAME", NOT_NULL_FLAG),
  MYODBC_FIELD_SHORT("KEY_SEQ", NOT_NULL_FLAG),
  MYODBC_FIELD_SHORT("UPDATE_RULE", 0),
  MYODBC_FIELD_SHORT("DELETE_RULE", 0),
  MYODBC_FIELD_NAME("FK_NAME", 0),
  MYODBC_FIELD_NAME("PK_NAME", 0),
  MYODBC_FIELD_SHORT("DEFERRABILITY", 0),
};

const uint SQLFORE_KEYS_FIELDS= array_elements(SQLFORE_KEYS_fields);

/* Multiple array of Struct to store and sort SQLForeignKeys field */
typedef struct SQL_FOREIGN_KEY_FIELD
{
  char PKTABLE_CAT[NAME_LEN + 1];
  char PKTABLE_SCHEM[NAME_LEN + 1];
  char PKTABLE_NAME[NAME_LEN + 1];
  char PKCOLUMN_NAME[NAME_LEN + 1];
  char FKTABLE_CAT[NAME_LEN + 1];
  char FKTABLE_SCHEM[NAME_LEN + 1];
  char FKTABLE_NAME[NAME_LEN + 1];
  char FKCOLUMN_NAME[NAME_LEN + 1];
  int  KEY_SEQ;
  int  UPDATE_RULE;
  int  DELETE_RULE;
  char FK_NAME[NAME_LEN + 1];
  char PK_NAME[NAME_LEN + 1];
  int  DEFERRABILITY;
} MY_FOREIGN_KEY_FIELD;

char *SQLFORE_KEYS_values[]= {
    NULL,"",NULL,NULL,
    NULL,"",NULL,NULL,
    0,0,0,NULL,NULL,0
};


/*
 * Get a record from the array if exists otherwise allocate a new 
 * record and return.  
 *
 * @param records MY_FOREIGN_KEY_FIELD record 
 * @param recnum  0-based record number
 *
 * @return The requested record or NULL if it doesn't exist
 *         (and isn't created).
 */
MY_FOREIGN_KEY_FIELD *fk_get_rec(DYNAMIC_ARRAY *records, unsigned int recnum)
{
  MY_FOREIGN_KEY_FIELD *rec= NULL;
  if (recnum < records->elements)
  {
    rec= ((MY_FOREIGN_KEY_FIELD *)records->buffer) + recnum;
  }
  else
  {
    rec= (MY_FOREIGN_KEY_FIELD *) alloc_dynamic(records);
    if (!rec)
      return NULL;
    memset(rec, 0, sizeof(MY_FOREIGN_KEY_FIELD));
  }
  return rec;
}


/* 
  If the foreign keys associated with a primary key are requested, the 
  result set is ordered by FKTABLE_CAT, FKTABLE_NAME, KEY_SEQ, PKTABLE_NAME
*/
static int sql_fk_sort(const void *var1, const void *var2)
{
  int ret;
  if ((ret= strcmp(((MY_FOREIGN_KEY_FIELD *) var1)->FKTABLE_CAT,
               ((MY_FOREIGN_KEY_FIELD *) var2)->FKTABLE_CAT)) == 0)
  {
    if ((ret= strcmp(((MY_FOREIGN_KEY_FIELD *) var1)->FKTABLE_NAME,
                  ((MY_FOREIGN_KEY_FIELD *) var2)->FKTABLE_NAME)) == 0)
    {
      if ((ret= ((MY_FOREIGN_KEY_FIELD *) var1)->KEY_SEQ -
                  ((MY_FOREIGN_KEY_FIELD *) var2)->KEY_SEQ) == 0)
      {
        if ((ret= strcmp(((MY_FOREIGN_KEY_FIELD *) var1)->PKTABLE_NAME,
                  ((MY_FOREIGN_KEY_FIELD *) var2)->PKTABLE_NAME)) == 0)
        {
          return 0;
        }
      }
    }
  }
  return ret;
}


/* 
  If the primary keys associated with a foreign key are requested, the 
  result set is ordered by PKTABLE_CAT, PKTABLE_NAME, KEY_SEQ, FKTABLE_NAME
*/
static int sql_pk_sort(const void *var1, const void *var2)
{
  int ret;
  if ((ret= strcmp(((MY_FOREIGN_KEY_FIELD *) var1)->PKTABLE_CAT,
               ((MY_FOREIGN_KEY_FIELD *) var2)->PKTABLE_CAT)) == 0)
  {
    if ((ret= strcmp(((MY_FOREIGN_KEY_FIELD *) var1)->PKTABLE_NAME,
                  ((MY_FOREIGN_KEY_FIELD *) var2)->PKTABLE_NAME)) == 0)
    {
      if ((ret= ((MY_FOREIGN_KEY_FIELD *) var1)->KEY_SEQ -
                  ((MY_FOREIGN_KEY_FIELD *) var2)->KEY_SEQ) == 0)
      {
        if ((ret= strcmp(((MY_FOREIGN_KEY_FIELD *) var1)->FKTABLE_NAME,
                  ((MY_FOREIGN_KEY_FIELD *) var2)->FKTABLE_NAME)) == 0)
        {
          return 0;
        }
      }
    }
  }
  return ret;
}


SQLRETURN foreign_keys_no_i_s(SQLHSTMT hstmt,
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
  STMT *stmt=(STMT *) hstmt;
  uint row_count= 0;

  MEM_ROOT  *alloc;
  MYSQL_ROW row, table_row;
  MYSQL_RES *local_res;
  char      **data= NULL;
  /* We need this array for the cases if key count is greater than 18 */
  char      **tempdata= NULL;
  char      buffer[NAME_LEN + 1];
  unsigned int index= 0;
  DYNAMIC_ARRAY records;
  MY_FOREIGN_KEY_FIELD *fkRows= NULL;
  unsigned long *lengths;
  SQLRETURN rc= SQL_SUCCESS;

  myodbc_init_dynamic_array(&records, sizeof(MY_FOREIGN_KEY_FIELD), 0, 0);

  /* Get the list of tables that match szCatalog and szTable */
  myodbc_mutex_lock(&stmt->dbc->lock);
  local_res= table_status(stmt, szFkCatalogName, cbFkCatalogName, szFkTableName, 
                    cbFkTableName, FALSE, TRUE, TRUE);
  if (!local_res && mysql_errno(&stmt->dbc->mysql))
  {
    rc= handle_connection_error(stmt);
    goto unlock_and_free;
  }
  else if (!local_res)
  {
    goto empty_set_unlock;
  }
  free_internal_result_buffers(stmt);
  myodbc_mutex_unlock(&stmt->dbc->lock);

  while ((table_row = mysql_fetch_row(local_res)))
  {
    myodbc_mutex_lock(&stmt->dbc->lock);
    lengths = mysql_fetch_lengths(local_res);
    if (stmt->result)
      mysql_free_result(stmt->result);
    stmt->result= server_show_create_table(stmt,
                                           szFkCatalogName, cbFkCatalogName,
                                           (SQLCHAR *)table_row[0], 
                                           (SQLSMALLINT)lengths[0]);

    if (!stmt->result)
    {
      if (mysql_errno(&stmt->dbc->mysql))
      {
        rc= handle_connection_error(stmt);
        goto unlock_and_free;
      }
      goto empty_set_unlock;
    }
    myodbc_mutex_unlock(&stmt->dbc->lock);
   
    /* Convert mysql fields to data that odbc wants */
    alloc= &stmt->alloc_root;

    while (row= mysql_fetch_row(stmt->result))
    {
      lengths= mysql_fetch_lengths(stmt->result);
      if (lengths[1])
      {    
        const char Fk_keywords[2][12]= {"FOREIGN KEY", "REFERENCES"};
        const char Fk_ref_action[2][12]= {"ON UPDATE", "ON DELETE"};
        const char *pos, *end_pos, *bracket_end, *comma_pos;
        char       table_name[NAME_LEN+1], constraint_name[NAME_LEN+1];
        char       quote_char;
        unsigned int last_index= 0, quote_char_length= 1, key_seq, key_search;
        const char *end= row[1] + lengths[1];
        const char *token= row[1];

        quote_char= get_identifier_quote(stmt);
        while ((token= find_first_token(stmt->dbc->ansi_charset_info, 
                              token, end, "CONSTRAINT")) != NULL)
        {
          pos= token;
          last_index= index;
          key_seq= 0;

          /* get constraint name */
          pos= my_next_token(NULL, &pos, NULL, 
                 quote_char ? quote_char : ' ');
          end_pos= my_next_token(pos, &pos, constraint_name, 
                     quote_char ? quote_char : ' ');
          token= end_pos;

          for (key_search= 0; key_search < 2; ++key_search)
          {
            /* get [FOREIGN KEY | REFERENCES] position */
            token= find_first_token(stmt->dbc->ansi_charset_info, token - 1, 
                                      end, Fk_keywords[key_search]);
            token += strlen(Fk_keywords[key_search]);
            token= skip_leading_spaces(token);
            *table_name= 0;

            /* if '(' not present get primary key table name */
            if (*token != '(')
            {
              pos= token;
              pos= my_next_token(NULL, &pos, NULL, 
                     quote_char ? quote_char : ' ');
              end_pos= my_next_token(pos, &pos, table_name, 
                         quote_char ? quote_char : ' ');
              token= end_pos;
            }

            token= skip_leading_spaces(token);
            /* 
               get foreign key and primary column name 
               in loop 1 and 2 respectively
            */
            if (*token == '(')
            {
              bracket_end= pos= token + 1;

              /* Get past opening quote */
              bracket_end= my_next_token(NULL, &bracket_end, NULL,
                                         quote_char ? quote_char : ' ');

              /* Get past closing quote */
              bracket_end= my_next_token(NULL, &bracket_end, NULL,
                                         quote_char ? quote_char : ' ');

              /* Only now it is safe to look for closing parenthese */
              bracket_end= my_next_token(NULL, &bracket_end, NULL, ')');
              /* 
                index position need to be maintained for both PK column 
                and FK column to fetch proper record    
              */
              if (key_search == 0)
                last_index= index;
              else
                index= last_index;
              do 
              {
                fkRows= fk_get_rec(&records, index);
                if (!fkRows)
                {
                  goto empty_set;
                }

                comma_pos= pos;
                comma_pos= my_next_token(NULL, &comma_pos, NULL, ',');

                if (comma_pos > bracket_end || comma_pos == NULL)
                {
                  /* 
                    TODO: make the length calculation more efficient and simple.
                          Add checking for negative values.
                  */
                  memcpy(buffer, pos + quote_char_length, 
                           bracket_end - pos - quote_char_length * 2 - 1);
                  buffer[bracket_end - pos - quote_char_length * 2 - 1]= '\0';
                  if (key_search == 0)
                  {
                    myodbc_stpmov(fkRows->FKCOLUMN_NAME, buffer);
                  }
                  else
                  {
                    myodbc_stpmov(fkRows->PKCOLUMN_NAME, buffer);
                    myodbc_stpmov(fkRows->PKTABLE_NAME, table_name);
                    myodbc_stpmov(fkRows->FK_NAME, constraint_name);
                    myodbc_stpmov(fkRows->FKTABLE_NAME, row[0]);
                    myodbc_stpmov(fkRows->FKTABLE_CAT, (szFkCatalogName ?
                            strdup_root(alloc, (char *)szFkCatalogName) :
                            strdup_root(alloc, stmt->dbc->database ?
                            stmt->dbc->database : "null")));
                    myodbc_stpmov(fkRows->PKTABLE_CAT, (szPkCatalogName ?
                            strdup_root(alloc, (char *)szPkCatalogName) :
                            strdup_root(alloc, stmt->dbc->database ?
                            stmt->dbc->database : "null")));
                    /* key_seq incremented once for each PK column */
                    fkRows->KEY_SEQ= ++key_seq;
                  }
                  ++index;
                  break;
                }
                else
                {
                  memcpy(buffer, pos + quote_char_length, 
                           comma_pos - pos - quote_char_length * 2 - 1);
                  buffer[comma_pos - pos - quote_char_length * 2 - 1]= '\0';
                  if (key_search == 0)
                  {    
                    myodbc_stpmov(fkRows->FKCOLUMN_NAME, buffer);
                  }
                  else
                  {
                    myodbc_stpmov(fkRows->PKCOLUMN_NAME, buffer);
                    myodbc_stpmov(fkRows->PKTABLE_NAME, table_name);
                    myodbc_stpmov(fkRows->FK_NAME, constraint_name);
                    myodbc_stpmov(fkRows->FKTABLE_NAME, row[0]);
                    myodbc_stpmov(fkRows->FKTABLE_CAT, (szFkCatalogName ?
                            strdup_root(alloc, (char *)szFkCatalogName) :
                            strdup_root(alloc, stmt->dbc->database ?
                            stmt->dbc->database : "null")));
                    myodbc_stpmov(fkRows->PKTABLE_CAT, (szPkCatalogName ?
                            strdup_root(alloc, (char *)szPkCatalogName) :
                            strdup_root(alloc, stmt->dbc->database ?
                            stmt->dbc->database : "null")));
                    /* key_seq incremented once for each PK column */
                    fkRows->KEY_SEQ= ++key_seq;
                  }
                  pos= comma_pos + 1;
                  ++index;
                }
              } while (1);
              token= bracket_end + 1;
            }
          }

          /* OPTIONAL (UPDATE|DELETE) operations */
          for (key_search= 0; key_search < 2; ++key_search)
          {
            unsigned int curr_index;
            int action= SQL_NO_ACTION;
            bracket_end= comma_pos= pos= token;
            bracket_end= my_next_token(NULL, &bracket_end, NULL, ')');
            comma_pos= my_next_token(NULL, &comma_pos, NULL, ',');
            pos= find_first_token(stmt->dbc->ansi_charset_info, pos - 1, 
              comma_pos ?
               ((comma_pos < bracket_end) ? comma_pos : bracket_end)
               : bracket_end,
              Fk_ref_action[key_search]);
            if (pos)
            {
              pos += strlen(Fk_ref_action[key_search]);
              pos= skip_leading_spaces(pos);
              if (*pos == 'R')  /* RESTRICT */
              {
                action= SQL_NO_ACTION;
              }
              else if (*pos == 'C')  /* CASCADE */
              {
                action= SQL_CASCADE;
              }
              else if (*pos == 'S')  /* SET NULL */
              {
                action= SQL_SET_NULL;
              }
              else if (*pos == 'N')  /* NO ACTION */
              {
                action= SQL_NO_ACTION;
              }
            }

            for (curr_index= last_index; curr_index < index; ++curr_index)
            {
              fkRows= fk_get_rec(&records, curr_index);
              if (!fkRows)
              {
                goto empty_set;
              }

              if (key_search == 0)
                fkRows->UPDATE_RULE= action;
              else
                fkRows->DELETE_RULE= action;
            }
          }
        }
      }
    }
  }

  if (!records.elements)
  {
    goto empty_set;
  }

  /* 
    If the primary keys associated with a foreign key are requested, then
    sort order is PKTABLE_CAT, PKTABLE_NAME, KEY_SEQ, FKTABLE_NAME
    Sort order used same as present in no_i_s case, but it is different from
    http://msdn.microsoft.com/en-us/library/windows/desktop/ms709315(v=vs.85).aspx
  */
  if (szPkTableName && szPkTableName[0])
  {
    sort_dynamic(&records, sql_pk_sort);
  }

  /* 
    if foreign keys associated with a primary key are requested 
    then sort order is FKTABLE_CAT, FKTABLE_NAME, KEY_SEQ, PKTABLE_NAME
    Sort order used same as present in no_i_s case, but it is different from
    http://msdn.microsoft.com/en-us/library/windows/desktop/ms709315(v=vs.85).aspx
  */
  if (szFkTableName && szFkTableName[0])
  {
    sort_dynamic(&records, sql_fk_sort);
  }

  if (records.elements)
  {
    tempdata= (char**) myodbc_malloc(sizeof(char*)*SQLFORE_KEYS_FIELDS*
                                         records.elements,
                                         MYF(MY_ZEROFILL));
    if (!tempdata)
    {
      set_mem_error(&stmt->dbc->mysql);
      rc= handle_connection_error(stmt);
      goto free_and_return;
    }
  }

  data= tempdata;
  fkRows= (MY_FOREIGN_KEY_FIELD *) records.buffer;
  index= 0;  
  while (index < records.elements)
  {
    if (szPkTableName && szPkTableName[0])
    {
      if (myodbc_strcasecmp(szPkTableName, fkRows[index].PKTABLE_NAME))
      {
        ++index;
        continue;
      }
    }

    data[0]= strdup_root(alloc, fkRows[index].PKTABLE_CAT);   /* PKTABLE_CAT */
    data[1]= NULL;                                            /* PKTABLE_SCHEM */
    data[2]= strdup_root(alloc, fkRows[index].PKTABLE_NAME);  /* PKTABLE_NAME */
    data[3]= strdup_root(alloc, fkRows[index].PKCOLUMN_NAME); /* PKCOLUMN_NAME */

    data[4]= strdup_root(alloc, fkRows[index].FKTABLE_CAT);   /* FKTABLE_CAT */ 
    data[5]= NULL;                                            /* FKTABLE_SCHEM */
    data[6]= strdup_root(alloc, fkRows[index].FKTABLE_NAME);  /* FKTABLE_NAME */
    data[7]= strdup_root(alloc, fkRows[index].FKCOLUMN_NAME); /* FKCOLUMN_NAME */

    sprintf(buffer,"%d", fkRows[index].KEY_SEQ);
    data[8]= strdup_root(alloc, buffer);                      /* KEY_SEQ */

    sprintf(buffer,"%d", fkRows[index].UPDATE_RULE);
    data[9]= strdup_root(alloc, buffer);                      /* UPDATE_RULE */ 
  
    sprintf(buffer,"%d", fkRows[index].DELETE_RULE);
    data[10]= strdup_root(alloc, buffer);                     /* DELETE_RULE */

    data[11]= strdup_root(alloc, fkRows[index].FK_NAME);      /* FK_NAME */
    data[12]= strdup_root(alloc, "PRIMARY");                  /* PK_NAME */
    data[13]= "7";  /*SQL_NOT_DEFERRABLE*/                    /* DEFERRABILITY */

    data+= SQLFORE_KEYS_FIELDS;
    ++row_count;
    ++index;
  }
  delete_dynamic(&records);
  mysql_free_result(local_res);

  /* Copy only the elements that contain fk names */
  stmt->result_array= (MYSQL_ROW)myodbc_memdup((char *)tempdata,
                                           sizeof(char *) *
                                           SQLFORE_KEYS_FIELDS *
                                           row_count,
                                           MYF(0));
  x_free((char *)tempdata);

  if (!stmt->result_array)
  {
    set_mem_error(&stmt->dbc->mysql);
    return handle_connection_error(stmt);
  }

  set_row_count(stmt, row_count);
  myodbc_link_fields(stmt,SQLFORE_KEYS_fields,SQLFORE_KEYS_FIELDS);
  return SQL_SUCCESS;

empty_set_unlock:
  myodbc_mutex_unlock(&stmt->dbc->lock);

empty_set:
  x_free((char *)tempdata);
  delete_dynamic(&records);
  mysql_free_result(local_res);
  free_internal_result_buffers(stmt);
  if (stmt->result)
    mysql_free_result(stmt->result);
  
  return create_empty_fake_resultset(stmt, SQLFORE_KEYS_values,
                                     sizeof(SQLFORE_KEYS_values),
                                     SQLFORE_KEYS_fields,
                                     SQLFORE_KEYS_FIELDS);
unlock_and_free:
  myodbc_mutex_unlock(&stmt->dbc->lock);
  mysql_free_result(local_res);
  local_res= NULL;

free_and_return:
  x_free((char *)tempdata);
  delete_dynamic(&records);

  free_internal_result_buffers(stmt);
  if (stmt->result)
    mysql_free_result(stmt->result);

  if (local_res)
    mysql_free_result(local_res);

  return rc;
}


/*
****************************************************************************
SQLPrimaryKeys
****************************************************************************
*/

MYSQL_FIELD SQLPRIM_KEYS_fields[]=
{
  MYODBC_FIELD_NAME("TABLE_CAT", 0),
  MYODBC_FIELD_NAME("TABLE_SCHEM", 0),
  MYODBC_FIELD_NAME("TABLE_NAME", NOT_NULL_FLAG),
  MYODBC_FIELD_NAME("COLUMN_NAME", NOT_NULL_FLAG),
  MYODBC_FIELD_SHORT("KEY_SEQ", NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("PK_NAME", 128, 0),
};

const uint SQLPRIM_KEYS_FIELDS= array_elements(SQLPRIM_KEYS_fields);

const long SQLPRIM_LENGTHS[]= {0, 0, 1, 5, 4, -7};

char *SQLPRIM_KEYS_values[]= {
    NULL,"",NULL,NULL,0,NULL
};

/*
  @purpose : returns the column names that make up the primary key for a table.
       The driver returns the information as a result set. This function
       does not support returning primary keys from multiple tables in
       a single call
*/

SQLRETURN
primary_keys_no_i_s(SQLHSTMT hstmt,
                    SQLCHAR *catalog, SQLSMALLINT catalog_len,
                    SQLCHAR *schema __attribute__((unused)),
                    SQLSMALLINT schema_len __attribute__((unused)),
                    SQLCHAR *table, SQLSMALLINT table_len)
{
    STMT *stmt= (STMT *) hstmt;
    MYSQL_ROW row;
    char      **data;
    uint      row_count;

    myodbc_mutex_lock(&stmt->dbc->lock);
    if (!(stmt->result= server_list_dbkeys(stmt, catalog, catalog_len,
                                           table, table_len)))
    {
      SQLRETURN rc= handle_connection_error(stmt);
      myodbc_mutex_unlock(&stmt->dbc->lock);
      return rc;
    }
    myodbc_mutex_unlock(&stmt->dbc->lock);
    stmt->result_array= (char**) myodbc_malloc(sizeof(char*)*SQLPRIM_KEYS_FIELDS*
                                            (ulong) stmt->result->row_count,
                                            MYF(MY_ZEROFILL));
    if (!stmt->result_array)
    {
      set_mem_error(&stmt->dbc->mysql);
      return handle_connection_error(stmt);
    }

    stmt->lengths= (unsigned long*) myodbc_malloc( sizeof(long)*SQLPRIM_KEYS_FIELDS*
                                            (ulong) stmt->result->row_count,
                                            MYF(MY_ZEROFILL));
    if (!stmt->lengths)
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
                break;    /* Already found unique key */

            fix_row_lengths(stmt, SQLPRIM_LENGTHS, row_count, SQLPRIM_KEYS_FIELDS);

            ++row_count;
            data[0]= data[1]=0;
            data[2]= row[0];
            data[3]= row[4];
            data[4]= row[3];
            data[5]= "PRIMARY";
            data+= SQLPRIM_KEYS_FIELDS;
        }
    }

    set_row_count(stmt, row_count);
    myodbc_link_fields(stmt,SQLPRIM_KEYS_fields,SQLPRIM_KEYS_FIELDS);

    return SQL_SUCCESS;
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

/* TODO make LONGLONG fields just LONG if SQLLEN is 4 bytes */
MYSQL_FIELD SQLPROCEDURECOLUMNS_fields[]=
{
  MYODBC_FIELD_NAME("PROCEDURE_CAT",     0),
  MYODBC_FIELD_NAME("PROCEDURE_SCHEM",   0),
  MYODBC_FIELD_NAME("PROCEDURE_NAME",    NOT_NULL_FLAG),
  MYODBC_FIELD_NAME("COLUMN_NAME",       NOT_NULL_FLAG),
  MYODBC_FIELD_SHORT ("COLUMN_TYPE",       NOT_NULL_FLAG),
  MYODBC_FIELD_SHORT ("DATA_TYPE",         NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("TYPE_NAME",         20,       NOT_NULL_FLAG),
  MYODBC_FIELD_LONGLONG("COLUMN_SIZE",       0),
  MYODBC_FIELD_LONGLONG("BUFFER_LENGTH",     0),
  MYODBC_FIELD_SHORT ("DECIMAL_DIGITS",    0),
  MYODBC_FIELD_SHORT ("NUM_PREC_RADIX",    0),
  MYODBC_FIELD_SHORT ("NULLABLE",          NOT_NULL_FLAG),
  MYODBC_FIELD_NAME("REMARKS",           0),
  MYODBC_FIELD_NAME("COLUMN_DEF",        0),
  MYODBC_FIELD_SHORT ("SQL_DATA_TYPE",     NOT_NULL_FLAG),
  MYODBC_FIELD_SHORT ("SQL_DATETIME_SUB",  0),
  MYODBC_FIELD_LONGLONG("CHAR_OCTET_LENGTH", 0),
  MYODBC_FIELD_LONG  ("ORDINAL_POSITION",  NOT_NULL_FLAG),
  MYODBC_FIELD_STRING("IS_NULLABLE",       3,        0),
};

const uint SQLPROCEDURECOLUMNS_FIELDS=
             array_elements(SQLPROCEDURECOLUMNS_fields);


/*
  @type    : internal
  @purpose : returns procedure params as resultset
*/
static MYSQL_RES *server_list_proc_params(STMT *stmt,
                                          SQLCHAR *catalog,
                                          SQLSMALLINT catalog_len,
                                          SQLCHAR *proc_name,
                                          SQLSMALLINT proc_name_len)
{
  DBC   *dbc = stmt->dbc;
  MYSQL *mysql= &dbc->mysql;
  char   buff[255+4*NAME_LEN+1], *pos;

  pos= myodbc_stpmov(buff, "SELECT name, CONCAT(IF(length(returns)>0, CONCAT('RETURN_VALUE ', returns, if(length(param_list)>0, ',', '')),''), param_list),"
                    "db, type FROM mysql.proc WHERE Db=");


  if (catalog_len)
  {
    pos= myodbc_stpmov(pos, "'");
    pos+= mysql_real_escape_string(mysql, pos, (char *)catalog, catalog_len);
    pos= myodbc_stpmov(pos, "'");
  }
  else
    pos= myodbc_stpmov(pos, "DATABASE()");

  if (proc_name_len)
  {
    pos= myodbc_stpmov(pos, " AND name LIKE '");
    pos+= mysql_real_escape_string(mysql, pos, (char *)proc_name, proc_name_len);
    pos= myodbc_stpmov(pos, "'");
  }

  pos= myodbc_stpmov(pos, " ORDER BY Db, name");

  assert(pos - buff < sizeof(buff));
  MYLOG_DBC_QUERY(dbc, buff);
  if (exec_stmt_query(stmt, buff, (unsigned long)(pos - buff), FALSE))
    return NULL;

  return mysql_store_result(mysql);
}


/*
  @type    : internal
  @purpose : releases memory allocated for internal use in
             SQLProcedureColumns
*/
static void free_procedurecolumn_res(int total_records, LIST *params)
{
  int i;
  uint j;
  LIST *cur_params;

  for (i= 1; i <= total_records; ++i)
  {
    if(params && params->data)
    {
      cur_params= params;

      for (j= 0; j < SQLPROCEDURECOLUMNS_FIELDS; ++j)
      {
        /* check for constant values that do not need to be freed */
        if ((j != mypcPROCEDURE_SCHEM)
         && (j != mypcNUM_PREC_RADIX)
         && (j != mypcNULLABLE)
         && (j != mypcREMARKS)
         && (j != mypcCOLUMN_DEF)
         && (j != mypcIS_NULLABLE))
        {
          x_free(((char**)cur_params->data)[j]);
        }
      }
      /* cleanup the list */
      params= list_delete_forward(params);
      x_free(cur_params->data);
      x_free(cur_params);
    }
  }
}


/*
  @type    : ODBC 1.0 API
  @purpose : returns the list of input and output parameters, as well as
  the columns that make up the result set for the specified
  procedures. The driver returns the information as a result
  set on the specified statement
*/
SQLRETURN
procedure_columns_no_i_s(SQLHSTMT hstmt,
                         SQLCHAR *szCatalogName, SQLSMALLINT cbCatalogName,
                         SQLCHAR *szSchemaName __attribute__((unused)),
                         SQLSMALLINT cbSchemaName __attribute__((unused)),
                         SQLCHAR *szProcName, SQLSMALLINT cbProcName,
                         SQLCHAR *szColumnName, SQLSMALLINT cbColumnName)
{
  STMT *stmt= (STMT *)hstmt;
  LIST *params= 0, *params_r, *cur_params= 0;
  SQLRETURN nReturn= SQL_SUCCESS;
  DYNAMIC_STRING  dynQuery;
  MYSQL_ROW row;
  MYSQL_RES *proc_list_res, *columns_res= 0;
  char **tempdata;
  int params_num= 0, return_params_num= 0;
  unsigned int i, j, total_params_num= 0;

  if (init_dynamic_string(&dynQuery, "SELECT 1", 1024,1024))
    return set_stmt_error(stmt, "HY001", "Not enough memory", 4001);

  params_r= params= (LIST *) myodbc_malloc(sizeof(LIST), MYF(MY_ZEROFILL));

  if (params_r == NULL)
  {
    dynstr_free(&dynQuery);
    set_mem_error(&stmt->dbc->mysql);
    return handle_connection_error(stmt);
  }

  /* get procedures list */
  myodbc_mutex_lock(&stmt->dbc->lock);

  if (!(proc_list_res= server_list_proc_params(stmt, 
      szCatalogName, cbCatalogName, szProcName, cbProcName)))
  {
    myodbc_mutex_unlock(&stmt->dbc->lock);

    nReturn= set_error(stmt, MYERR_S1000, mysql_error(&stmt->dbc->mysql),
                      mysql_errno(&stmt->dbc->mysql));
    goto clean_exit;
  }

  myodbc_mutex_unlock(&stmt->dbc->lock);

  while ((row= mysql_fetch_row(proc_list_res)))
  {
    char *token;
    char *param_str;
    char *param_str_end;
    SQLINTEGER param_ordinal_position= 1;

    /* Return value parameter must have 0 as ordinal position */
    if(!myodbc_strcasecmp(row[3], "FUNCTION"))
      param_ordinal_position= 0;

    param_str= row[1];
    if(!param_str[0])
      continue;

    param_str_end= param_str + strlen(param_str);

    token = proc_param_tokenize(param_str, &params_num);

    if (params_num == 0)
    {
      goto empty_set;
    }

    while (token != NULL)
    {
      SQLSMALLINT  ptype= 0;
      int sql_type_index;
      unsigned int flags= 0;
      SQLCHAR param_name[NAME_LEN]= "\0";
      SQLCHAR param_dbtype[1024]= "\0";
      SQLCHAR param_type[4]= "\0";
      SQLCHAR param_sql_type[6]= "\0";
      SQLCHAR param_size_buf[21]= "\0";
      SQLCHAR param_buffer_len[21]= "\0";
      SQLCHAR param_decimal[6]= "\0";
      SQLCHAR param_desc_type[6]= "\0";
      SQLCHAR param_pos[6]= "\0";

      SQLTypeMap *type_map;
      SQLSMALLINT dec;
      SQLULEN param_size= 0;
      MYSQL_ROW data= myodbc_malloc(sizeof(SQLPROCEDURECOLUMNS_values), MYF(MY_ZEROFILL));
      /* temp variables for debugging */
      SQLUINTEGER dec_int= 0;
      SQLINTEGER sql_type_int= 0;

      if (data ==  NULL)
      {
        set_mem_error(&stmt->dbc->mysql);
        nReturn= handle_connection_error(stmt);
        goto exit_with_free;
      }

      token= proc_get_param_type(token, (int)strlen(token), &ptype);
      token= proc_get_param_name(token, (int)strlen(token), param_name);
      token= proc_get_param_dbtype(token, (int)strlen(token), param_dbtype);

      /* param_dbtype is lowercased in the proc_get_param_dbtype */
      if (strstr(param_dbtype, "unsigned"))
        flags |= UNSIGNED_FLAG;

      sql_type_index= proc_get_param_sql_type_index(param_dbtype, (int)strlen(param_dbtype));
      type_map= proc_get_param_map_by_index(sql_type_index);

      param_size= proc_get_param_size(param_dbtype, (int)strlen(param_dbtype), sql_type_index, &dec);

      proc_get_param_octet_len(stmt, sql_type_index, param_size, dec, flags, param_buffer_len);

      data[mypcPROCEDURE_CAT]= myodbc_strdup(row[2], MYF(0));   /* PROCEDURE_CAT */
      data[mypcPROCEDURE_SCHEM]= NULL;                      /* PROCEDURE_SCHEM */
      data[mypcPROCEDURE_NAME]= myodbc_strdup(row[0], MYF(0));  /* PROCEDURE_NAME */
      data[mypcCOLUMN_NAME]= myodbc_strdup(param_name, MYF(0)); /* COLUMN_NAME */

      if (cbColumnName)
      {
        dynstr_append_mem(&dynQuery, ",", 1);
        dynstr_append_os_quoted(&dynQuery, (char *)param_name, NullS);
        dynstr_append_mem(&dynQuery, " LIKE ", 6);
        dynstr_append_os_quoted(&dynQuery, (char *)szColumnName, NullS);
      }

      if (param_ordinal_position == 0)
      {
        ptype= SQL_RETURN_VALUE;
      }

      sprintf(param_type, "%d", ptype);
      data[mypcCOLUMN_TYPE]= myodbc_strdup(param_type, MYF(0)); /* COLUMN_TYPE */

      if (!myodbc_strcasecmp(type_map->type_name, "bit") && param_size > 1)
      {
        sprintf(param_sql_type, "%d", SQL_BINARY);
      }
      else
      {
        sprintf(param_sql_type, "%d", (int)type_map->sql_type);
      }
      data[mypcDATA_TYPE]= myodbc_strdup(param_sql_type, MYF(0)); /* DATA_TYPE */
      
      if (!myodbc_strcasecmp(type_map->type_name, "set") ||
         !myodbc_strcasecmp(type_map->type_name, "enum"))
      {
        data[mypcTYPE_NAME]= myodbc_strdup("char", MYF(0));
      }
      else
      {
        data[mypcTYPE_NAME]= myodbc_strdup(type_map->type_name, MYF(0));
      }

       /* TYPE_NAME */
      
      proc_get_param_col_len(stmt, sql_type_index, param_size, dec, flags, param_size_buf);
      data[mypcCOLUMN_SIZE]= myodbc_strdup(param_size_buf, MYF(0)); /* COLUMN_SIZE */

      data[mypcBUFFER_LENGTH]= myodbc_strdup(param_buffer_len, MYF(0)); /* BUFFER_LENGTH */
      
      if (dec != SQL_NO_TOTAL)
      {
        sprintf(param_decimal, "%d", (int)dec);
        data[mypcDECIMAL_DIGITS]= myodbc_strdup(param_decimal, MYF(0)); /* DECIMAL_DIGITS */
        data[mypcNUM_PREC_RADIX]= "10"; /* NUM_PREC_RADIX */
      }
      else
      {
        data[mypcDECIMAL_DIGITS]= NullS;
        data[mypcNUM_PREC_RADIX]= NullS; /* NUM_PREC_RADIX */
      }
      data[mypcNULLABLE]= "1";  /* NULLABLE */
      data[mypcREMARKS]= "";   /* REMARKS */
      data[mypcCOLUMN_DEF]= NullS; /* COLUMN_DEF */
      
      if(type_map->sql_type == SQL_TYPE_DATE || 
         type_map->sql_type == SQL_TYPE_TIME ||
         type_map->sql_type == SQL_TYPE_TIMESTAMP)
      {
        sprintf(param_desc_type, "%d", SQL_DATETIME);
        data[mypcSQL_DATA_TYPE]= myodbc_strdup(param_desc_type, MYF(0)); /* SQL_DATA_TYPE  */
        data[mypcSQL_DATETIME_SUB]= myodbc_strdup(data[mypcDATA_TYPE], MYF(0)); /* SQL_DATETIME_SUB */
      }
      else
      {
        data[mypcSQL_DATA_TYPE]= myodbc_strdup(data[mypcDATA_TYPE], MYF(0)); /* SQL_DATA_TYPE  */
        data[mypcSQL_DATETIME_SUB]= NULL;  /* SQL_DATETIME_SUB */
      }

      if (is_char_sql_type(type_map->sql_type) || is_wchar_sql_type(type_map->sql_type) ||
          is_binary_sql_type(type_map->sql_type))
      {
        /* Actualy can use data[mypcBUFFER_LENGTH] here and don't do myodbc_strdup */
        data[mypcCHAR_OCTET_LENGTH]= myodbc_strdup(param_buffer_len, MYF(0)); /* CHAR_OCTET_LENGTH */
      }
      else
      {
        data[mypcCHAR_OCTET_LENGTH]= NULL;                     /* CHAR_OCTET_LENGTH */
      }

      sprintf(param_pos, "%d", (int) param_ordinal_position);
      data[mypcORDINAL_POSITION]= myodbc_strdup(param_pos, MYF(0)); /* ORDINAL_POSITION */
      ++param_ordinal_position;

      data[mypcIS_NULLABLE]= "YES"; /* IS_NULLABLE */

      {
        LIST *new_elem= (LIST *) myodbc_malloc(sizeof(LIST), MYF(MY_ZEROFILL));

        if (new_elem == NULL)
        {
          set_mem_error(&stmt->dbc->mysql);
          nReturn= handle_connection_error(stmt);
          goto exit_with_free;
        }

        new_elem->data= data;
        params->next= new_elem;
        new_elem->prev= params;
        params= new_elem;
        ++total_params_num;
      }

      token = proc_param_next_token(token, param_str_end);
    }
  }
  
  return_params_num= total_params_num;

  if (cbColumnName)
  {
    myodbc_mutex_lock(&stmt->dbc->lock);
    if (exec_stmt_query(stmt, dynQuery.str, (unsigned long)dynQuery.length, FALSE) ||
        !(columns_res= mysql_store_result(&stmt->dbc->mysql)))
    {
      myodbc_mutex_unlock(&stmt->dbc->lock);

      nReturn= set_error(stmt, MYERR_S1000, mysql_error(&stmt->dbc->mysql),
                mysql_errno(&stmt->dbc->mysql));
      goto exit_with_free;
    }

    myodbc_mutex_unlock(&stmt->dbc->lock);

    /* should be only one row */
    row= mysql_fetch_row(columns_res);

    if (row == NULL)
    {
      nReturn= set_error(stmt, MYERR_S1000, mysql_error(&stmt->dbc->mysql),
                mysql_errno(&stmt->dbc->mysql));
      goto exit_with_free;
    }

    if(params_r->next)
    {
      params= params_r->next;
    }

    /* 1st element is always 1 */
    for (i= 1; i < columns_res->field_count; ++i)
    {
      if(strcmp(row[i], "1"))
      {
        --return_params_num;
      }
    }

    if (return_params_num == 0)
    {
      goto empty_set;
    }
  }

  stmt->result= proc_list_res;
  stmt->result_array= (MYSQL_ROW) myodbc_malloc(sizeof(char*) * SQLPROCEDURECOLUMNS_FIELDS *
                                            (return_params_num ? return_params_num : total_params_num), 
                                            MYF(MY_ZEROFILL));
  tempdata= stmt->result_array;
  
  if(params_r->next)
  {
    params= params_r->next;
  }

  /* copy data */
  for (i= 0; i < total_params_num; ++i)
  {
    int skip_result= (columns_res ? strcmp(row[i+1], "1") : 0);
    if(params && params->data)
    {
      cur_params= params;

      for (j= 0; j < SQLPROCEDURECOLUMNS_FIELDS; ++j)
      {
        char *cur_field_val= ((char**)cur_params->data)[j];

        /* copy data only if  */
        if(!skip_result)
        {
          if(cur_field_val && cur_field_val[0])
            tempdata[j]= strdup_root(&stmt->alloc_root, cur_field_val);
          else
            tempdata[j]= 0;
        }
      }
      params= params->next ? params->next : params;
      if(!skip_result)
        tempdata += SQLPROCEDURECOLUMNS_FIELDS;
    }
  }

  set_row_count(stmt, return_params_num);

  myodbc_link_fields(stmt, SQLPROCEDURECOLUMNS_fields, SQLPROCEDURECOLUMNS_FIELDS);

  goto clean_exit;

empty_set:

  nReturn= create_empty_fake_resultset(hstmt, SQLPROCEDURECOLUMNS_values,
                                      sizeof(SQLPROCEDURECOLUMNS_values),
                                      SQLPROCEDURECOLUMNS_fields,
                                      SQLPROCEDURECOLUMNS_FIELDS);
exit_with_free:

  free_internal_result_buffers(stmt);
  mysql_free_result(proc_list_res);

clean_exit:

  free_procedurecolumn_res(total_params_num, params_r->next);

  if(columns_res)
  {
    mysql_free_result(columns_res);
  }

  dynstr_free(&dynQuery);
  x_free(params_r);

  return nReturn;
}


/*
****************************************************************************
SQLSpecialColumns
****************************************************************************
*/

MYSQL_FIELD SQLSPECIALCOLUMNS_fields[]=
{
  MYODBC_FIELD_SHORT("SCOPE", 0),
  MYODBC_FIELD_NAME("COLUMN_NAME", NOT_NULL_FLAG),
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

SQLRETURN
special_columns_no_i_s(SQLHSTMT hstmt, SQLUSMALLINT fColType,
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

    /* Reset the statement in order to avoid memory leaks when working with ADODB */
    my_SQLFreeStmt(hstmt, MYSQL_RESET);

    stmt->result= server_list_dbcolumns(stmt, szTableQualifier, cbTableQualifier,
                                        szTableName, cbTableName, NULL, 0);
    if (!(result= stmt->result))
    {
      return handle_connection_error(stmt);
    }

    if ( fColType == SQL_ROWVER )
    {
        /* Find possible timestamp */
        if ( !(stmt->result_array= (char**) myodbc_malloc(sizeof(char*)*SQLSPECIALCOLUMNS_FIELDS*
                                                      result->field_count, MYF(MY_ZEROFILL))) )
        {
          set_mem_error(&stmt->dbc->mysql);
          return handle_connection_error(stmt);
        }

        /* Convert mysql fields to data that odbc wants */
        alloc= &stmt->alloc_root;
        field_count= 0;
        mysql_field_seek(result,0);
        for ( row= stmt->result_array;
            (field = mysql_fetch_field(result)); )
        {
            SQLSMALLINT type;
            if ((field->type != MYSQL_TYPE_TIMESTAMP))
              continue;
#ifdef ON_UPDATE_NOW_FLAG
            if (!(field->flags & ON_UPDATE_NOW_FLAG))
              continue;
#else
            /*
              TIMESTAMP_FLAG is only set on fields that are auto-set or
              auto-updated. We really only want auto-updated, but we can't
              tell the difference because of Bug #30081.
            */
            if (!(field->flags & TIMESTAMP_FLAG))
              continue;
#endif
            ++field_count;
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
              SQLSMALLINT digits= get_decimal_digits(stmt, field);
              if (digits != SQL_NO_TOTAL)
              {
                sprintf(buff,"%d", digits);
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
        myodbc_link_fields(stmt,SQLSPECIALCOLUMNS_fields,SQLSPECIALCOLUMNS_FIELDS);
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
    if ( !(stmt->result_array= (char**) myodbc_malloc(sizeof(char*)*SQLSPECIALCOLUMNS_FIELDS*
                                                  result->field_count, MYF(MY_ZEROFILL))) )
    {
      set_mem_error(&stmt->dbc->mysql);
      return handle_connection_error(stmt);
    }

    /* Convert MySQL fields to data that odbc wants */
    alloc= &stmt->alloc_root;
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
        ++field_count;
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
          SQLSMALLINT digits= get_decimal_digits(stmt, field);
          if (digits != SQL_NO_TOTAL)
          {
            sprintf(buff,"%d", digits);
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
    myodbc_link_fields(stmt,SQLSPECIALCOLUMNS_fields,SQLSPECIALCOLUMNS_FIELDS);
    return SQL_SUCCESS;
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
  MYODBC_FIELD_NAME("TABLE_CAT", 0),
  MYODBC_FIELD_NAME("TABLE_SCHEM", 0),
  MYODBC_FIELD_NAME("TABLE_NAME", NOT_NULL_FLAG),
  MYODBC_FIELD_SHORT("NON_UNIQUE", 0),
  MYODBC_FIELD_NAME("INDEX_QUALIFIER", 0),
  MYODBC_FIELD_NAME("INDEX_NAME", 0),
  MYODBC_FIELD_SHORT("TYPE", NOT_NULL_FLAG),
  MYODBC_FIELD_SHORT("ORDINAL_POSITION", 0),
  MYODBC_FIELD_NAME("COLUMN_NAME", 0),
  MYODBC_FIELD_STRING("ASC_OR_DESC", 1, 0),
  MYODBC_FIELD_SHORT("CARDINALITY", 0),
  MYODBC_FIELD_SHORT("PAGES", 0),
  MYODBC_FIELD_STRING("FILTER_CONDITION", 10, 0),
};

const uint SQLSTAT_FIELDS= array_elements(SQLSTAT_fields);

/*
  @purpose : retrieves a list of statistics about a single table and the
       indexes associated with the table. The driver returns the
       information as a result set.
*/

SQLRETURN
statistics_no_i_s(SQLHSTMT hstmt,
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

    if (!table_len)
        goto empty_set;

    myodbc_mutex_lock(&dbc->lock);
    stmt->result= server_list_dbkeys(stmt, catalog, catalog_len,
                                     table, table_len);
    if (!stmt->result)
    {
      SQLRETURN rc= handle_connection_error(stmt);
      myodbc_mutex_unlock(&dbc->lock);
      return rc;
    }
    myodbc_mutex_unlock(&dbc->lock);
    my_int2str(SQL_INDEX_OTHER,SS_type,10,0);
    stmt->order=       SQLSTAT_order;
    stmt->order_count= array_elements(SQLSTAT_order);
    stmt->fix_fields=  fix_fields_copy;
    stmt->array= (MYSQL_ROW) myodbc_memdup((char *)SQLSTAT_values,
                                       sizeof(SQLSTAT_values),MYF(0));
    if (!stmt->array)
    {
      set_mem_error(&stmt->dbc->mysql);
      return handle_connection_error(stmt);
    }

    if (stmt->dbc->ds->no_catalog)
      stmt->array[0]= "";
    else
      stmt->array[0]= strmake_root(&stmt->alloc_root,
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
            {
                --stmt->result->row_count;
            }
        }
        (*prev)= 0;
        mysql_data_seek(stmt->result,0);  /* Restore pointer */
    }

    set_row_count(stmt, stmt->result->row_count);
    myodbc_link_fields(stmt,SQLSTAT_fields,SQLSTAT_FIELDS);
    return SQL_SUCCESS;

empty_set:
  return create_empty_fake_resultset(stmt, SQLSTAT_values,
                                     sizeof(SQLSTAT_values),
                                     SQLSTAT_fields, SQLSTAT_FIELDS);
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
  MYODBC_FIELD_NAME("TABLE_CAT",   0),
  MYODBC_FIELD_NAME("TABLE_SCHEM", 0),
  MYODBC_FIELD_NAME("TABLE_NAME",  0),
  MYODBC_FIELD_NAME("TABLE_TYPE",  0),
/*
  Table remark length is 80 characters
*/
  MYODBC_FIELD_STRING("REMARKS",     80, 0),
};

const uint SQLTABLES_FIELDS= array_elements(SQLTABLES_values);

SQLRETURN
tables_no_i_s(SQLHSTMT hstmt,
              SQLCHAR *catalog, SQLSMALLINT catalog_len,
              SQLCHAR *schema, SQLSMALLINT schema_len,
              SQLCHAR *table, SQLSMALLINT table_len,
              SQLCHAR *type, SQLSMALLINT type_len)
{
    STMT *stmt= (STMT *)hstmt;
    my_bool all_dbs= 1, user_tables, views;

    MYSQL_RES *catalog_res= NULL;
    my_ulonglong row_count= 0;
    MYSQL_ROW catalog_row;
    unsigned long *lengths;    
    unsigned long count= 0;
    my_bool is_info_schema= 0;
    SQLRETURN rc = SQL_SUCCESS;

    /* 
      empty (but non-NULL) schema and table returns catalog list 
      If no_i_s then call 'show database' to list all catalogs (database).
    */
    if (catalog_len && ((!schema_len && schema && !table_len && table)
        || (catalog && (!server_has_i_s(stmt->dbc) || 
                        stmt->dbc->ds->no_information_schema))))
    {
      myodbc_mutex_lock(&stmt->dbc->lock);
      {
        char buff[32 + NAME_LEN * 2], *to;
        to= myodbc_stpmov(buff, "SHOW DATABASES LIKE '");
        to+= mysql_real_escape_string(&stmt->dbc->mysql, to,
                                      (char *)catalog, catalog_len);
        to= myodbc_stpmov(to, "'");
        MYLOG_QUERY(stmt, buff);
        if (!mysql_query(&stmt->dbc->mysql, buff))
          catalog_res= mysql_store_result(&stmt->dbc->mysql);
      }
      myodbc_mutex_unlock(&stmt->dbc->lock);

      if (!catalog_res)
      {
        return handle_connection_error(stmt);
      }

    }
    else
    {
      /* 
        Set is_info_schema for determining mysql_table_status 
        parameters later 
      */
      is_info_schema= 1;
    }

    /* empty (but non-NULL) schema and table returns catalog list */
    if (catalog_len && !schema_len && schema && !table_len && table)
    {
      stmt->order         = SQLTABLES_qualifier_order;
      stmt->order_count   = array_elements(SQLTABLES_qualifier_order);
      stmt->fix_fields    = fix_fields_copy;
      stmt->array= (MYSQL_ROW) myodbc_memdup((char *)SQLTABLES_qualifier_values,
                                             sizeof(SQLTABLES_qualifier_values),
                                             MYF(0));
      stmt->result= catalog_res;
      if (!stmt->array)
      {
        set_mem_error(&stmt->dbc->mysql);
        return handle_connection_error(stmt);
      }
      myodbc_link_fields(stmt, SQLTABLES_fields, SQLTABLES_FIELDS);
      return SQL_SUCCESS;
    }

    if (!catalog_len && catalog && schema_len && !table_len && table)
    {
      /* Return set of allowed schemas (none) */
        rc = create_fake_resultset(stmt, SQLTABLES_owner_values,
                                   sizeof(SQLTABLES_owner_values),
                                   1, SQLTABLES_fields, SQLTABLES_FIELDS);
        goto free_and_return;
    }

    if (!catalog_len && catalog && !schema_len && schema &&
        !table_len && table && type && !strncmp((char *)type, "%", 2))
    {
        /* Return set of TableType qualifiers */
        rc = create_fake_resultset(stmt, (MYSQL_ROW)SQLTABLES_type_values,
                                   sizeof(SQLTABLES_type_values),
                                   sizeof(SQLTABLES_type_values) /
                                   sizeof(SQLTABLES_type_values[0]),
                                   SQLTABLES_fields, SQLTABLES_FIELDS);
        goto free_and_return;
    }

    /* any other use of catalog="" returns an empty result */
    if (catalog && !catalog_len)
      goto empty_set;

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
       /*
        If database result set (catalog_res) was produced loop  
        through all database to fetch table list inside database
      */
      while (catalog_res && (catalog_row= mysql_fetch_row(catalog_res)) 
             || is_info_schema)
      {
        myodbc_mutex_lock(&stmt->dbc->lock);

        if (is_info_schema)
        {
          /* 
            If i_s then all databases are fetched in single loop 
            for SQL_ALL_CATALOGS (%) and catalog selection is handled 
            inside mysql_table_status_i_s 
          */
          stmt->result= table_status(stmt, catalog, catalog_len,
                                     table, table_len, TRUE,
                                     user_tables, views);
        }
        else
        {
          /* 
            If no i_s then all databases, except information_schema as 
						it contains SYSTEM VIEW table types, are fetched are sent to 
            mysql_table_status_show to get result for SQL_ALL_CATALOGS 
            (%) and catalog selection is handled in this function
          */
          if(myodbc_strcasecmp(catalog_row[0], "information_schema") == 0)
          {
            myodbc_mutex_unlock(&stmt->dbc->lock);
            continue;
          }

          lengths= mysql_fetch_lengths(catalog_res);

          if (stmt->result)
            mysql_free_result(stmt->result);

          stmt->result= table_status(stmt, catalog_row[0], (SQLSMALLINT)lengths[0],
                                     table, (SQLSMALLINT)table_len, TRUE,
                                     user_tables, views);
        }

        if (!stmt->result && mysql_errno(&stmt->dbc->mysql))
        {
          /* unknown DB will return empty set from SQLTables */
          switch (mysql_errno(&stmt->dbc->mysql))
          {
          case ER_BAD_DB_ERROR:
            myodbc_mutex_unlock(&stmt->dbc->lock);
            goto empty_set;
          default:
            rc= handle_connection_error(stmt);
            myodbc_mutex_unlock(&stmt->dbc->lock);
            goto free_and_return;
          }
        }
        myodbc_mutex_unlock(&stmt->dbc->lock);

        if (!stmt->result)
          goto empty_set; ///////////////////////

        /* assemble final result set */
        {
          MYSQL_ROW    data= 0, row;
          char         *db= "";
          row_count += stmt->result->row_count;

          if (!row_count)
          {
            free_internal_result_buffers(stmt);
            mysql_free_result(stmt->result);
            is_info_schema= 0;
            continue;
          }

          if (!(stmt->result_array=
                (char **)myodbc_realloc((char *)stmt->result_array,
                                       sizeof(char *) * 
                                       SQLTABLES_FIELDS * row_count,
                                       MYF(MY_ZEROFILL))))
          {
            set_mem_error(&stmt->dbc->mysql);
            rc = handle_connection_error(stmt);
            goto free_and_return;
          }

          data= stmt->result_array;

          if (!stmt->dbc->ds->no_catalog)
          {
            /* Set db to fetched database row from show database result set */
            if (!is_info_schema && lengths[0])
            {  
              db= strmake_root(&stmt->alloc_root,
                                catalog_row[0], lengths[0]);
            }
            else if (!catalog)
            {
              if (!reget_current_catalog(stmt->dbc))
              {
                const char *dbname= stmt->dbc->database ? stmt->dbc->database
                                                        : "null";
                db= strmake_root(&stmt->alloc_root,
                                 dbname, strlen(dbname));
              }
              else
              {
                /* error was set in reget_current_catalog */
                rc = SQL_ERROR;
                goto free_and_return;
              }
            }
            else
              db= strmake_root(&stmt->alloc_root,
                               (char *)catalog, catalog_len);
          }

          while ((row= mysql_fetch_row(stmt->result)))
          {
            int cat_index= 3;
            int type_index= 2;
            int comment_index= 1;
            my_bool view;

            /* If if did not use I_S */
            if (stmt->dbc->ds->no_information_schema
              || !server_has_i_s(stmt->dbc))
            {
              type_index= comment_index= (stmt->result->field_count == 18) ? 17 : 15;
              /* We do not have catalog in result */
              cat_index= -1;
            }

            view= (myodbc_casecmp(row[type_index], "VIEW", 4) == 0);

            if ((view && !views) || (!view && !user_tables))
            {
              --row_count;
              continue;
            }

            data[0+count]= (cat_index >= 0 ?
                            strdup_root(&stmt->alloc_root, row[cat_index]) :
                    db);
            data[1+count]= "";
            data[2+count]= strdup_root(&stmt->alloc_root, row[0]);
            data[3+count]= view ? "VIEW" : "TABLE";
            data[4+count] = strdup_root(&stmt->alloc_root, row[comment_index]);
            count+= SQLTABLES_FIELDS;
          }
        }
        /* 
          If i_s then loop only once to fetch database name,
          as mysql_table_status_i_s handles SQL_ALL_CATALOGS (%) 
          functionality and all databases with tables are returned 
          in one result set and so no further loops are required.
        */
        is_info_schema= 0;
      }

      if (!row_count)
      {
        goto empty_set;
      }

      set_row_count(stmt, row_count);
    }

    if (catalog_res)
      mysql_free_result(catalog_res);

    myodbc_link_fields(stmt, SQLTABLES_fields, SQLTABLES_FIELDS);
    return SQL_SUCCESS;

empty_set:
  if (catalog_res)
    mysql_free_result(catalog_res);

  return create_empty_fake_resultset(stmt, SQLTABLES_values,
                                     sizeof(SQLTABLES_values),
                                     SQLTABLES_fields,
                                     SQLTABLES_FIELDS);
free_and_return:
  if (catalog_res)
    mysql_free_result(catalog_res);
  return rc;
}
