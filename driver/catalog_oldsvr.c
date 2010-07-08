/*
  Copyright (c) 2000, 2010, Oracle and/or its affiliates. All rights reserved.

  The MySQL Connector/ODBC is licensed under the terms of the
  GPL, like most MySQL Connectors. There are special exceptions
  to the terms and conditions of the GPL as it is applied to
  this software, see the FLOSS License Exception available on
  mysql.com.

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
  @file  catalog_oldsvr.c
  @brief Catalog functions for server versions that doesn't have I_S.
         Currently designed for simple inclusion into main catalog file
*/



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
static MYSQL_RES *table_privs_raw_data( DBC *       dbc,
                                        SQLCHAR *   catalog,
                                        SQLSMALLINT catalog_len,
                                        SQLCHAR *   table,
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
mysql_list_table_priv(SQLHSTMT hstmt,
                     SQLCHAR *catalog, SQLSMALLINT catalog_len,
                     SQLCHAR *schema __attribute__((unused)),
                     SQLSMALLINT schema_len __attribute__((unused)),
                     SQLCHAR *table, SQLSMALLINT table_len)
{
    STMT     *stmt= (STMT *)hstmt;

    char     **data, **row;
    MEM_ROOT *alloc;
    uint     row_count;

    pthread_mutex_lock(&stmt->dbc->lock);
    stmt->result= table_privs_raw_data(stmt->dbc, catalog, catalog_len,
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
    stmt->result->row_count= row_count;
    mysql_link_fields(stmt,SQLTABLES_priv_fields,SQLTABLES_PRIV_FIELDS);
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
static MYSQL_RES *column_privs_raw_data( MYSQL *    mysql,
                                        SQLCHAR *   catalog,
                                        SQLSMALLINT catalog_len,
                                        SQLCHAR *   table,
                                        SQLSMALLINT table_len,
                                        SQLCHAR *   column,
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
mysql_list_column_priv(SQLHSTMT hstmt,
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
    catalog_len= catalog ? (SQLSMALLINT)strlen((char *)catalog) : 0;
  if (table_len == SQL_NTS)
    table_len= table ? (SQLSMALLINT)strlen((char *)table) : 0;
  if (column_len == SQL_NTS)
    column_len= column ? (SQLSMALLINT)strlen((char *)column) : 0;

  pthread_mutex_lock(&stmt->dbc->lock);
  stmt->result= column_privs_raw_data(&stmt->dbc->mysql,
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
  stmt->result->row_count= row_count;
  mysql_link_fields(stmt,SQLCOLUMNS_priv_fields,SQLCOLUMNS_PRIV_FIELDS);
  return SQL_SUCCESS;
}


/**
Get the table status for a table or tables using SHOW TABLE STATUS.

@param[in] stmt           Handle to statement
@param[in] catalog        Catalog (database) of table, @c NULL for current
@param[in] catalog_length Length of catalog name, or @c SQL_NTS
@param[in] table          Name of table
@param[in] table_length   Length of table name, or @c SQL_NTS
@param[in] wildcard       Whether the table name is a wildcard

@return Result of SHOW TABLE STATUS, or NULL if there is an error
or empty result (check mysql_errno(&stmt->dbc->mysql) != 0)
*/
static MYSQL_RES *mysql_table_status_show(STMT        *stmt,
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
		table_length= (SQLSMALLINT)strlen((char *)table);
	if (catalog_length == SQL_NTS && catalog)
		catalog_length= (SQLSMALLINT)strlen((char *)catalog);

	to= strmov(buff, "SHOW TABLE STATUS ");
	if (catalog && *catalog)
	{
		to= strmov(to, "FROM `");
		to+= myodbc_escape_string(mysql, to, (ulong)(sizeof(buff) - (to - buff)),
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
			to+= myodbc_escape_string(mysql, to, (ulong)(sizeof(buff) - (to - buff)),
			(char *)table, table_length, 0);
		to= strmov(to, "'");
	}

	MYLOG_QUERY(stmt, buff);
	if (mysql_query(mysql,buff))
		return NULL;

	return mysql_store_result(mysql);
}
