/*
  Copyright (C) 1995-2007 MySQL AB

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
  @file  cursor.c
  @brief Client-side cursor functions
*/

/***************************************************************************
 * The following ODBC APIs are implemented in this file:		   *
 *									   *
 *   SQLSetCursorName	 (ISO 92)					   *
 *   SQLGetCursorName	 (ISO 92)					   *
 *   SQLCloseCursor	 (ISO 92)					   *
 *   SQLSetPos		 (ODBC)						   *
 *   SQLBulkOperations	 (ODBC)						   *
 *									   *
 ****************************************************************************/

#include "myodbc3.h"
#include <locale.h>

/*
  @type    : myodbc3 internal
  @purpose : returns the table used by this query and
  ensures that all columns are from the same table
*/

static const char *find_used_table(STMT *stmt)
{
    MYSQL_FIELD  *field, *end;
    char *table_name;
    MYSQL_RES *result= stmt->result;

    if ( stmt->table_name && stmt->table_name[0] )
        return stmt->table_name;

    table_name= 0;
    for ( field= result->fields, end= field+ result->field_count;
        field < end ; field++ )
    {

#if MYSQL_VERSION_ID >= 40100
        if ( field->org_table )
        {
            if ( !table_name )
                table_name= field->org_table;
            if ( strcmp(field->org_table, table_name) )
            {
                set_error(stmt,MYERR_S1000,
                          "Can't modify a row from a statement that uses more than one table",0);
                return NULL;
            }
        }
#else
        if ( field->table )
        {
            if ( !table_name )
                table_name= field->table;
            if ( strcmp(field->table, table_name) )
            {
                set_error(stmt,MYERR_S1000,
                          "Can't modify a row from a statement that uses more than one table",0);
                return NULL;
            }
        }
#endif
    }
    /*
      We have to copy the strings as we may have to re-issue the query
      while using cursors.
    */
    stmt->table_name= dupp_str(table_name,SQL_NTS);
    return stmt->table_name;
}


/*
  @type    : myodbc internal
  @purpose : returns the previous token in the query by eating white spaces
  if we found the start of the string, return it
*/

static const char *mystr_get_prev_token( const char **query, const char *start )
{
    const char *pos = *query;

    do
    {
        if ( pos == start )
            return ( *query = start );     /* Return start of string */
    } while ( !isspace( *(--pos) ) ) ;
    *query = pos;      /* Remember pos to space */

    return ( pos + 1 );   /* Return found token */
}


/**
  Check if a statement involves a positioned cursor using the WHERE CURRENT
  OF syntax.

  @param[in]   pStmt       Handle of the statement
  @param[out]  pStmtCursor Pointer to the statement referred to by the cursor

  @return      Pointer to the beginning of 'WHERE CURRENT OF'
*/
char *check_if_positioned_cursor_exists(STMT *pStmt, STMT **pStmtCursor)
{
  if (pStmt->query && pStmt->query_end)
  {
    const char *pszQueryTokenPos= pStmt->query_end;
    const char *pszCursor= mystr_get_prev_token((const char**)&pszQueryTokenPos,
                                                pStmt->query);

    if (!myodbc_casecmp(mystr_get_prev_token(&pszQueryTokenPos,
                                             pStmt->query),"OF",2) &&
        !myodbc_casecmp(mystr_get_prev_token(&pszQueryTokenPos,
                                             pStmt->query),"CURRENT",7) &&
        !myodbc_casecmp(mystr_get_prev_token(&pszQueryTokenPos,
                                             pStmt->query),"WHERE",5) )
    {
      LIST *list_element;
      DBC  *dbc= (DBC *)pStmt->dbc;

      /*
        Scan the list of statements for this connection and see if we
        can find the cursor name this statement is referring to - it
        must have a result set to count.
      */
      for (list_element= dbc->statements;
           list_element;
           list_element= list_element->next)
      {
        *pStmtCursor= (HSTMT)list_element->data;

        /*
          Even if the cursor name matches, the statement must have a
          result set to count.
        */
        if ((*pStmtCursor)->result &&
            (*pStmtCursor)->cursor.name &&
            !myodbc_strcasecmp((*pStmtCursor)->cursor.name,
                               pszCursor))
        {
          return (char *)pszQueryTokenPos;
        }
      }

      /* Did we run out of statements without finding a viable cursor? */
      if (!list_element)
      {
        char buff[200];
        strxmov(buff,"Cursor '", pszCursor,
                "' does not exist or does not have a result set.", NullS);
        set_stmt_error(pStmt, "34000", buff, ER_INVALID_CURSOR_NAME);
      }

      return (char *)pszQueryTokenPos;
    }
  }

  return NULL;
}


/**
  Check if a field exists in a result set.

  @param[in]  name    Name of the field
  @param[in]  result  Result set to check
*/
static my_bool have_field_in_result(const char *name, MYSQL_RES *result)
{
  MYSQL_FIELD  *field;
  unsigned int ncol;

  for (ncol= 0; ncol < result->field_count; ncol++)
  {
    field= result->fields + ncol;
    if (myodbc_strcasecmp(name,
#if MYSQL_VERSION_ID >= 40100
                          field->org_name
#else
                          field->name
#endif
                          ) == 0)
      return TRUE;
  }

  return FALSE;
}


/**
  Check if a primary or unique key exists in the table referred to by
  the statement for which all of the component fields are in the result
  set. If such a key exists, the field names are stored in the cursor.

  @param[in]  stmt  Statement

  @return  Whether a usable unique keys exists
*/
static my_bool check_if_usable_unique_key_exists(STMT *stmt)
{
  char buff[NAME_LEN * 2 + 18], /* Possibly escaped name, plus text for query */
       *pos, *table;
  MYSQL_RES *res;
  MYSQL_ROW row;
  int seq_in_index= 0;

  if (stmt->cursor.pk_validated)
    return stmt->cursor.pk_count;

#if MYSQL_VERSION_ID >= 40100
  if (stmt->result->fields->org_table)
    table= stmt->result->fields->org_table;
  else
#endif
    table= stmt->result->fields->table;

  /* Use SHOW KEYS FROM table to check for keys. */
  pos= strmov(buff, "SHOW KEYS FROM `");
  pos+= mysql_real_escape_string(&stmt->dbc->mysql, pos, table, strlen(table));
  pos= strmov(pos, "`");

  MYLOG_QUERY(stmt, buff);

  pthread_mutex_lock(&stmt->dbc->lock);
  if (mysql_query(&stmt->dbc->mysql, buff) ||
      !(res= mysql_store_result(&stmt->dbc->mysql)))
  {
    set_error(stmt, MYERR_S1000, mysql_error(&stmt->dbc->mysql),
              mysql_errno(&stmt->dbc->mysql));
    pthread_mutex_unlock(&stmt->dbc->lock);
    return FALSE;
  }

  while ((row= mysql_fetch_row(res)) &&
         stmt->cursor.pk_count < MY_MAX_PK_PARTS)
  {
    int seq= atoi(row[3]);

    /* If this is a new key, we're done! */
    if (seq <= seq_in_index)
      break;

    /* Unless it is non_unique, it does us no good. */
    if (row[1][0] == '1')
      continue;

    /* If this isn't the next part, this key is no good. */
    if (seq != seq_in_index + 1)
      continue;

    /* Check that we have the key field in our result set. */
    if (have_field_in_result(row[4], stmt->result))
    {
      /* We have a unique key field -- copy it, and increment our count. */
      strmov(stmt->cursor.pkcol[stmt->cursor.pk_count++].name, row[4]);
      seq_in_index= seq;
    }
    else
      /* Forget about any key we had in progress, we didn't have it all. */
      stmt->cursor.pk_count= seq_in_index= 0;
  }
  mysql_free_result(res);
  pthread_mutex_unlock(&stmt->dbc->lock);

  /* Remember that we've figured this out already. */
  stmt->cursor.pk_validated= 1;

  return stmt->cursor.pk_count > 0;
}


/*
  @type    : myodbc3 internal
  @purpose : positions the data cursor to appropriate row
*/

void set_current_cursor_data(STMT FAR *stmt,SQLUINTEGER irow)
{
    long       nrow, row_pos;
    MYSQL_RES  *result= stmt->result;
    MYSQL_ROWS *dcursor= result->data->data;

    /*
      If irow exists, then position the current row to point
      to the rowsetsize+irow, this is needed for positioned
      calls
    */
    row_pos= irow ? (long) (stmt->current_row+irow-1) : stmt->current_row;
    if ( stmt->cursor_row != row_pos )
    {
        for ( nrow= 0; nrow < row_pos; nrow++ )
            dcursor= dcursor->next;

        stmt->cursor_row= row_pos;
        result->data_cursor= dcursor;
    }
}


/*
  @type    : myodbc3 internal
  @purpose : sets the dynamic cursor, when the cursor is not set
  explicitly by the application
*/

static void set_dynamic_cursor_name(STMT FAR *stmt)
{
    stmt->cursor.name= (char*) my_malloc(MYSQL_MAX_CURSOR_LEN,MYF(MY_ZEROFILL));
    sprintf((char*) stmt->cursor.name,"SQL_CUR%d",stmt->dbc->cursor_count++);
}


/*
  @type    : myodbc3 internal
  @purpose : updates the stmt status information
*/

static SQLRETURN update_status(STMT FAR *stmt, SQLUSMALLINT status)
{
    if ( stmt->affected_rows == 0 )
        return set_error(stmt,MYERR_01S03,NULL,0);

    else if ( stmt->affected_rows > 1 )
        return set_error(stmt,MYERR_01S04,NULL,0);

    /*
      This only comes from SQLExecute(), not SQLSetPos() or
      SQLBulkOperations(), so we don't have to worry about the row status
      set by SQLExtendedFetch().
    */
    else if ( stmt->stmt_options.rowStatusPtr )
    {
        SQLUSMALLINT *ptr= stmt->stmt_options.rowStatusPtr+stmt->current_row;
        SQLUSMALLINT *end= ptr+stmt->affected_rows;

        for ( ; ptr != end ; ptr++ )
            *ptr= status;
    }
    return SQL_SUCCESS;
}


/*
  @type    : myodbc3 internal
  @purpose : updates the SQLSetPos status information
*/

static SQLRETURN update_setpos_status(STMT FAR *stmt, SQLINTEGER irow,
                                      my_ulonglong rows, SQLUSMALLINT status)
{
  stmt->affected_rows= stmt->dbc->mysql.affected_rows= rows;

  if (irow && rows > 1)
      return set_error(stmt,MYERR_01S04,NULL,0);

  /*
    If all rows successful, then only update status..else
    don't update...just for the sake of performance..
  */
  if (stmt->stmt_options.rowStatusPtr)
  {
    SQLUSMALLINT *ptr= stmt->stmt_options.rowStatusPtr;
    SQLUSMALLINT *end= ptr+rows;

    for ( ; ptr != end; ptr++)
        *ptr= status;
  }

  if (stmt->stmt_options.rowStatusPtr_ex)
  {
    SQLUSMALLINT *ptr= stmt->stmt_options.rowStatusPtr_ex;
    SQLUSMALLINT *end= ptr+rows;

    for ( ; ptr != end; ptr++)
        *ptr= status;
  }

  return SQL_SUCCESS;
}


/*
  @type    : myodbc3 internal
  @purpose : copy row buffers to statement
*/

static SQLRETURN copy_rowdata(STMT FAR *stmt, PARAM_BIND  param,
                              NET **net, SQLCHAR **to)
{
    SQLCHAR *orig_to= *to;
    /* Negative length means either NULL or DEFAULT, so we need 7 chars. */
    SQLUINTEGER length= (*param.actual_len > 0 ? *param.actual_len + 1 : 7);

    if ( !(*to= (SQLCHAR *) extend_buffer(*net,(char*) *to,length)) )
        return set_error(stmt,MYERR_S1001,NULL,4001);

    if ( !(*to= (SQLCHAR*) insert_param(stmt->dbc, (char*) *to, &param)) )
        return set_error(stmt,MYERR_S1001,NULL,4001);

    /* We have to remove zero bytes or we have problems! */
    while ( (*to > orig_to) && (*((*to) - 1) == (SQLCHAR) 0) ) (*to)--;

    if (!(*to= (SQLCHAR *)add_to_buffer(*net, (char *)*to, ",", 1)))
        return set_error(stmt,MYERR_S1001,NULL,4001);

    return(SQL_SUCCESS);
}


/*
  @type    : myodbc3 internal
  @purpose : executes a statement query
*/

static SQLRETURN exec_stmt_query(STMT FAR *stmt,char *query,
                                 SQLUINTEGER len)
{
    DBC FAR *dbc= stmt->dbc;
    SQLRETURN error= SQL_SUCCESS;

    MYLOG_QUERY(stmt, query);
    pthread_mutex_lock(&dbc->lock);
    if ( check_if_server_is_alive(dbc) ||
         mysql_real_query(&dbc->mysql, query, len) )
    {
        error= set_error(stmt,MYERR_S1000,mysql_error(&dbc->mysql),
                         mysql_errno(&dbc->mysql));
    }
    pthread_mutex_unlock(&dbc->lock);
    return(error);
}


/*
  @type    : myodbc3 internal
  @purpose : copy row buffers to statement
*/

static SQLRETURN copy_field_data(STMT FAR *stmt, PARAM_BIND *param,
                                 NET **net, SQLCHAR **to)
{
    SQLUINTEGER length= *(param->actual_len)+5;

    if ( !(*to= (SQLCHAR*) extend_buffer(*net, (char*) *to,length)) )
        return set_error(stmt,MYERR_S1001,NULL,4001);

    if ( !(*to= (SQLCHAR*) insert_param(stmt->dbc, (char*) *to, param)) )
        return set_error(stmt,MYERR_S1001,NULL,4001);

    /* Insert " AND ", where clause with multiple search */
    if (!(*to= (SQLCHAR *)add_to_buffer(*net, (char *)*to, " AND ", 5)))
        return set_error(stmt,MYERR_S1001,NULL,4001);

    return SQL_SUCCESS;
}


/*
  @type    : myodbc3 internal
  @purpose : copies field data to statement
*/

static my_bool insert_field(STMT FAR *stmt, MYSQL_RES *result,
                            DYNAMIC_STRING *dynQuery,
                            SQLUSMALLINT nSrcCol)
{
    PARAM_BIND  param;
    MYSQL_FIELD *field= mysql_fetch_field_direct(result,nSrcCol);
    MYSQL_ROW   row_data= result->data_cursor->data + nSrcCol;
    NET         *net=&stmt->dbc->mysql.net;
    SQLCHAR     *to= net->buff;
    SQLLEN      length;

    /* Copy row buffer data to statement */
    param.used= 1;
    param.SqlType= get_sql_data_type(stmt, field, 0);
    param.CType= SQL_C_CHAR;

    if ( row_data && *row_data )
    {
        param.buffer= *row_data;
        length= strlen(*row_data);

        param.actual_len= &length;

        if ( copy_field_data(stmt,&param,&net,&to) != SQL_SUCCESS )
            return 1;

        length= (uint) ((char *)to - (char*) net->buff);
        dynstr_append_mem(dynQuery, (char*) net->buff, length);
    }
    else
    {
        dynQuery->length--;
        dynstr_append_mem(dynQuery, " IS NULL AND ",13);
    }
    return 0;
}


/*
  @type    : myodbc3 internal
  @purpose : checks for the float comparision in where clause
*/
static my_bool if_float_field(STMT FAR *stmt, MYSQL_FIELD *field)
{
    if ( field->type == MYSQL_TYPE_FLOAT || field->type == MYSQL_TYPE_DOUBLE ||
         field->type == MYSQL_TYPE_DECIMAL )
    {
        set_error(stmt,MYERR_S1000,
                  "Invalid use of floating point comparision in positioned operations",0);
        return 1;
    }
    return 0;
}


/*
  @type    : myodbc3 internal
  @purpose : checks for the existance of pk columns in the resultset,
  if it is, copy that data to query, else we can't find the right row
*/

static SQLRETURN insert_pk_fields(STMT FAR *stmt, DYNAMIC_STRING *dynQuery)
{
    MYSQL_RES    *result= stmt->result;
    MYSQL_FIELD  *field;
    SQLUSMALLINT ncol;
    uint      index;
    MYCURSOR     *cursor= &stmt->cursor;
    SQLUINTEGER  pk_count= 0;

    /* Look for primary key columns in the current result set, */
    for (ncol= 0; ncol < result->field_count; ncol++)
    {
      field= result->fields+ncol;
      for (index= 0; index < cursor->pk_count; index++)
      {
        if (!myodbc_strcasecmp(cursor->pkcol[index].name, field->org_name))
        {
          /* PK data exists...*/
          dynstr_append_quoted_name(dynQuery, field->org_name);
          dynstr_append_mem(dynQuery, "=", 1);
          if (insert_field(stmt, result, dynQuery, ncol))
            return SQL_ERROR;
          cursor->pkcol[index].bind_done= TRUE;
          pk_count++;
          break;
        }
      }
    }

    /*
     If we didn't have data for all the components of the primary key,
     we can't build a correct WHERE clause.
    */
    if (pk_count != cursor->pk_count)
      return set_stmt_error(stmt, "HY000",
                            "Not all components of primary key are available, "
                            "so row to modify cannot be identified", 0);

    return SQL_SUCCESS;
}


/*
  @type    : myodbc3 internal
  @purpose : generate a WHERE clause based on the fields in the result set
*/

static SQLRETURN append_all_fields(STMT FAR *stmt,
                                   DYNAMIC_STRING *dynQuery)
{
  MYSQL_RES    *result= stmt->result;
  MYSQL_RES    *presultAllColumns;
  char          select[NAME_LEN+30];
  unsigned int  i,j;
  BOOL          found_field;

  /*
    Get the base table name. If there was more than one table underlying
    the result set, this will fail, and we couldn't build a suitable
    list of fields.
  */
  if (!(find_used_table(stmt)))
    return SQL_ERROR;

  /*
    Get the list of all of the columns of the underlying table by using
    SELECT * FROM <table> LIMIT 0.
  */
  strxmov(select, "SELECT * FROM `", stmt->table_name, "` LIMIT 0", NullS);
  MYLOG_QUERY(stmt, select);
  pthread_mutex_lock(&stmt->dbc->lock);
  if (mysql_query(&stmt->dbc->mysql, select) ||
      !(presultAllColumns= mysql_store_result(&stmt->dbc->mysql)))
  {
    set_error(stmt, MYERR_S1000, mysql_error(&stmt->dbc->mysql),
              mysql_errno(&stmt->dbc->mysql));
    pthread_mutex_unlock(&stmt->dbc->lock);
    return SQL_ERROR;
  }
  pthread_mutex_unlock(&stmt->dbc->lock);

  /*
    If the number of fields in the underlying table is not the same as
    our result set, we bail out -- we need them all!
  */
  if (mysql_num_fields(presultAllColumns) != mysql_num_fields(result))
  {
    mysql_free_result(presultAllColumns);
    return SQL_ERROR;
  }

  /*
    Now we walk through the list of columns in the underlying table,
    appending them to the query along with the value from the row at the
    current cursor position.
  */
  for (i= 0; i < presultAllColumns->field_count; i++)
  {
    MYSQL_FIELD *table_field= presultAllColumns->fields + i;

    /*
      We also can't handle floating-point fields because their comparison
      is inexact.
    */
    if (if_float_field(stmt, table_field))
    {
      mysql_free_result(presultAllColumns);
      return SQL_ERROR;
    }

    found_field= FALSE;
    for (j= 0; j < result->field_count; j++)
    {
      MYSQL_FIELD *cursor_field= result->fields + j;
      if (cursor_field->org_name &&
          !strcmp(cursor_field->org_name, table_field->name))
      {
        dynstr_append_quoted_name(dynQuery, table_field->name);
        dynstr_append_mem(dynQuery, "=", 1);
        if (insert_field(stmt, result, dynQuery, j))
        {
          mysql_free_result(presultAllColumns);
          return SQL_ERROR;
        }
        found_field= TRUE;
        break;
      }
    }

    /*
      If we didn't find the field, we have failed.
    */
    if (!found_field)
    {
      mysql_free_result(presultAllColumns);
      return SQL_ERROR;
    }
  }

  mysql_free_result(presultAllColumns);
  return SQL_SUCCESS;
}

/*
  @type	  : myodbc3 internal
  @purpose : build the where clause
*/

static SQLRETURN build_where_clause( STMT FAR *       pStmt, 
                                     DYNAMIC_STRING * dynQuery,
                                     SQLUSMALLINT     irow )
{
    /* set our cursor to irow - we call assuming irow is valid */
    set_current_cursor_data( pStmt, irow );

    /* simply append WHERE to our statement */
    dynstr_append_mem( dynQuery, " WHERE ", 7 );

    /*
      If a suitable key exists, then we'll use those columns, otherwise
      we'll try to use all of the columns.
    */
    if (check_if_usable_unique_key_exists(pStmt))
    {
      if (insert_pk_fields(pStmt, dynQuery) != SQL_SUCCESS)
        return SQL_ERROR;
    }
    else
    {
      if (append_all_fields(pStmt, dynQuery) != SQL_SUCCESS)
        return set_stmt_error(pStmt, "HY000",
                              "Build WHERE -> insert_fields() failed.",
                              0);
    }
    /* Remove the trailing ' AND ' */
    dynQuery->length -= 5;

    /* IF irow = 0 THEN delete all rows in the rowset ELSE specific (as in one) row */
    if ( irow == 0 )
    {
        char buff[32];

        sprintf( buff, " LIMIT %lu",
                 (unsigned long)pStmt->stmt_options.rows_in_set );
        dynstr_append( dynQuery, buff );
    }
    else
    {
        dynstr_append_mem( dynQuery, " LIMIT 1", 8 );
    }

    return SQL_SUCCESS;
}


/*
  @type	  : myodbc3 internal
  @purpose : if input param buffers exist, copy them to new
  statement
*/

static void copy_input_param(STMT FAR *stmt,STMT FAR *stmtNew,
                             SQLUINTEGER pcount)
{
    while ( pcount-- )
    {
        PARAM_BIND *param= dynamic_element(&stmt->params,pcount,PARAM_BIND*);
        PARAM_BIND *paramNew= dynamic_element(&stmtNew->params,pcount,PARAM_BIND*);
        param->pos_in_query= paramNew->pos_in_query;
        set_dynamic(&stmtNew->params, (DYNAMIC_ELEMENT)param, pcount);
    }
}


/*
  @type    : myodbc3 internal
  @purpose : set clause building..
*/

static SQLRETURN build_set_clause(STMT FAR *stmt, SQLUINTEGER irow,
                                  DYNAMIC_STRING *dynQuery)
{
    PARAM_BIND    param;
    SQLLEN        length;
    uint          ncol, ignore_count= 0;
    MYSQL_FIELD *field;
    MYSQL_RES   *result= stmt->result;
    BIND        *bind;
    NET         *net=&stmt->dbc->mysql.net;
    SQLLEN      *pcbValue;

    dynstr_append_mem(dynQuery," SET ",5);

    /*
      To make sure, it points to correct row in the
      current rowset..
    */
    irow= irow ? irow-1: 0;
    for ( ncol= 0; ncol < stmt->result->field_count; ncol++ )
    {
        SQLCHAR *to= net->buff;
        field= mysql_fetch_field_direct(result,ncol);
        bind= stmt->bind+ncol;

        if (!stmt->bind || (bind && !bind->field))
        {
          ignore_count++;
          continue;
        }
        pcbValue= bind->pcbValue ? bind->pcbValue + irow : 0;
        if ( pcbValue )
        {
            /*
          If the pcbValue is SQL_COLUMN_IGNORE, then ignore the
          column in the SET clause
            */
            if ( *pcbValue == SQL_COLUMN_IGNORE )
            {
                ignore_count++;
                continue;
            }
            /*
          Take care of SQL_NTS in pcbValue
            */
            else if ( *pcbValue == SQL_NTS )
                length= SQL_NTS;
            else
                length= *pcbValue;
        }
        else
            length= SQL_NTS;

        /* TODO : handle ..SQL_DATA_AT_EXEC here....*/

        dynstr_append_quoted_name(dynQuery,field->name);
        dynstr_append_mem(dynQuery,"=",1);

        param.used= 1;
        param.SqlType= get_sql_data_type(stmt, field, NULL);
        param.CType= bind->fCType;
        param.buffer= ((char *)bind->rgbValue)+irow*bind->cbValueMax;
        param.ValueMax= bind->cbValueMax;
        /*
            Check when SQL_LEN_DATA_AT_EXEC() macro was used instead of data length
        */
        if (length == SQL_NTS)
            length= strlen(param.buffer);
        else if (length <= SQL_LEN_DATA_AT_EXEC_OFFSET)
            length= -(length - SQL_LEN_DATA_AT_EXEC_OFFSET);

        param.actual_len= &length;

        if ( copy_rowdata(stmt,param,&net,&to) != SQL_SUCCESS )
            return(SQL_ERROR);

        length= (uint) ((char *)to - (char*) net->buff);
        dynstr_append_mem(dynQuery, (char*) net->buff, length);
    }

    if (ignore_count == result->field_count)
      return ER_ALL_COLUMNS_IGNORED;

    dynQuery->str[--dynQuery->length]='\0';
    return(SQL_SUCCESS);
}


/*
  @type    : myodbc3 internal
  @purpose : deletes the positioned cursor row
*/

SQLRETURN my_pos_delete(STMT FAR *stmt, STMT FAR *stmtParam,
                        SQLUSMALLINT irow, DYNAMIC_STRING *dynQuery)
{
    SQLRETURN nReturn;

    /* Delete only the positioned row, by building where clause */
    nReturn = build_where_clause( stmt, dynQuery, irow );
    if ( !SQL_SUCCEEDED( nReturn ) )
        return nReturn;

    /* DELETE the row(s) */
    nReturn= exec_stmt_query(stmt, dynQuery->str, dynQuery->length);
    if ( nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO )
    {
        stmtParam->affected_rows= mysql_affected_rows(&stmt->dbc->mysql);
        nReturn= update_status(stmtParam,SQL_ROW_DELETED);
    }
    return nReturn;
}


/*
  @type    : myodbc3 internal
  @purpose : updates the positioned cursor row
*/

SQLRETURN my_pos_update( STMT FAR *         pStmtCursor, 
                         STMT FAR *         pStmt,
                         SQLUSMALLINT       nRow, 
                         DYNAMIC_STRING *   dynQuery )
{
    SQLRETURN   nReturn;
    SQLHSTMT    hStmtTemp;
    STMT FAR  * pStmtTemp;

    nReturn = build_where_clause( pStmtCursor, dynQuery, nRow );
    if ( !SQL_SUCCEEDED( nReturn ) )
        return nReturn;

    /*
      Prepare and check if parameters exists in set clause..
      this happens with WHERE CURRENT OF statements ..
    */
    if ( my_SQLAllocStmt( pStmt->dbc, &hStmtTemp ) != SQL_SUCCESS )
    {
        return set_stmt_error( pStmt, "HY000", "my_SQLAllocStmt() failed.", 0 );
    }

    pStmtTemp = (STMT FAR*)hStmtTemp;

    if ( my_SQLPrepare( pStmtTemp, (SQLCHAR FAR*)dynQuery->str, dynQuery->length ) != SQL_SUCCESS )
    {
        my_SQLFreeStmt( pStmtTemp, SQL_DROP );
        return set_stmt_error( pStmt, "HY000", "my_SQLPrepare() failed.", 0 );
    }
    if ( pStmtTemp->param_count )      /* SET clause has parameters */
        copy_input_param( pStmt, pStmtTemp, pStmtTemp->param_count );

    nReturn = my_SQLExecute( pStmtTemp );
    if ( SQL_SUCCEEDED( nReturn ) )
    {
        pStmt->affected_rows = mysql_affected_rows( &pStmtTemp->dbc->mysql );
        nReturn = update_status( pStmt, SQL_ROW_UPDATED );
    }
    else if (nReturn == SQL_NEED_DATA)
    {
      /*
        Re-prepare the statement, which will leave us with a prepared
        statement that is a non-positioned update.
      */
      if (my_SQLPrepare(pStmt, (SQLCHAR *)dynQuery->str, dynQuery->length) !=
          SQL_SUCCESS)
        return SQL_ERROR;
    }

    my_SQLFreeStmt( pStmtTemp, SQL_DROP );

    return nReturn;
}


/*
  @type    : myodbc3 internal
  @purpose : deletes the positioned cursor row - will del all rows in rowset if irow = 0
*/

static SQLRETURN setpos_delete(STMT FAR *stmt, SQLUSMALLINT irow,
                               DYNAMIC_STRING *dynQuery)
{
    SQLUINTEGER  rowset_pos,rowset_end;
    my_ulonglong affected_rows= 0;
    SQLRETURN    nReturn= SQL_SUCCESS;
    ulong        query_length;
    const char   *table_name;

    /* we want to work with base table name - we expect call to fail if more than one base table involved */
    if ( !(table_name= find_used_table(stmt)) )
        return SQL_ERROR;

    /* appened our table name to our DELETE statement */
    dynstr_append_quoted_name(dynQuery,table_name);
    query_length= dynQuery->length;

    /* IF irow == 0 THEN delete all rows in the current rowset ELSE specific (as in one) row */
    if ( irow == 0 )
    {
        rowset_pos= 1;
        rowset_end= stmt->rows_found_in_set;
    }
    else
        rowset_pos= rowset_end= irow;

    /* process all desired rows in the rowset - we assume rowset_pos is valid */
    do
    {
        dynQuery->length= query_length;

        /* append our WHERE clause to our DELETE statement */
        nReturn = build_where_clause( stmt, dynQuery, (SQLUSMALLINT)rowset_pos );
        if ( !SQL_SUCCEEDED( nReturn ) )
            return nReturn;

        /* execute our DELETE statement */
        if ( !(nReturn= exec_stmt_query(stmt, dynQuery->str, dynQuery->length)) )
            affected_rows+= stmt->dbc->mysql.affected_rows;

    } while ( ++rowset_pos <= rowset_end );

    if ( nReturn == SQL_SUCCESS )
        nReturn= update_setpos_status(stmt,irow,affected_rows,SQL_ROW_DELETED);

    return nReturn;
}


/*
  @type    : myodbc3 internal
  @purpose : updates the positioned cursor row.
*/

static SQLRETURN setpos_update(STMT FAR *stmt, SQLUSMALLINT irow,
                               DYNAMIC_STRING *dynQuery)
{
    SQLUINTEGER  rowset_pos,rowset_end;
    my_ulonglong affected_rows= 0;
    SQLRETURN    nReturn= SQL_SUCCESS;
    ulong        query_length;
    const char   *table_name;

    if ( !(table_name= find_used_table(stmt)) )
        return SQL_ERROR;

    dynstr_append_quoted_name(dynQuery,table_name);
    query_length= dynQuery->length;

    if ( !irow )
    {
        /*
          If irow == 0, then update all rows in the current rowset
        */
        rowset_pos= 1;
        rowset_end= stmt->rows_found_in_set;
    }
    else
        rowset_pos= rowset_end= irow;

    do /* UPDATE, irow from current row set */
    {
        dynQuery->length= query_length;
        nReturn= build_set_clause(stmt,rowset_pos,dynQuery);
        if (nReturn == ER_ALL_COLUMNS_IGNORED)
        {
          /*
            If we're updating more than one row, having all columns ignored
            is fine. If it's just one row, that's an error.
          */
          if (!irow)
          {
            nReturn= SQL_SUCCESS;
            continue;
          }
          else
          {
            set_stmt_error(stmt, "21S02",
                           "Degree of derived table does not match column list",
                           0);
            return SQL_ERROR;
          }
        }
        else if (nReturn == SQL_ERROR)
          return SQL_ERROR;

        nReturn= build_where_clause(stmt, dynQuery, (SQLUSMALLINT)rowset_pos);
        if (!SQL_SUCCEEDED(nReturn))
          return nReturn;

        if ( !(nReturn= exec_stmt_query(stmt, dynQuery->str, dynQuery->length)) )
            affected_rows+= stmt->dbc->mysql.affected_rows;

    } while ( ++rowset_pos <= rowset_end );

    if ( nReturn == SQL_SUCCESS )
        nReturn= update_setpos_status(stmt,irow,affected_rows,SQL_ROW_UPDATED);

    return nReturn;
}


/*!
    \brief  Insert 1 or more rows.

            This function has been created to support SQLSetPos where
            SQL_ADD. For each row it will complete the given INSERT
            statement (ext_query) and call exec_stmt_query() to execute.

    \note   We have a limited capacity to shove data/sql across the wire. We try
            to handle this. see break_insert.

    \param  stmt            Viable statement.
    \param  irow            Position of the row in the rowset on which to perform the operation. 
                            If RowNumber is 0, the operation applies to every row in the rowset.
    \param  ext_query       The INSERT statement up to and including the VALUES. So something 
                            like; "INSERT .... VALUES"
                            
    \return SQLRETURN
    
    \retval SQLERROR        Something went wrong.
    \retval SQL_SUCCESS     Success!
*/

static SQLRETURN batch_insert( STMT FAR *stmt, SQLUSMALLINT irow, DYNAMIC_STRING *ext_query )
{
    MYSQL_RES    *result= stmt->result;     /* result set we are working with                               */
    SQLUINTEGER  insert_count= 1;           /* num rows to insert - will be real value when row is 0 (all)  */
    SQLUINTEGER  count= 0;                  /* current row                                                  */
    SQLLEN       length;
    NET         *net= &stmt->dbc->mysql.net;
    SQLUSMALLINT ncol;
    SQLCHAR      *to;
    ulong        query_length= 0;           /* our original query len so we can reset pos if break_insert   */
    my_bool      break_insert= FALSE;       /* true if we are to exceed max data size for transmission      
                                               but this seems to be misused                                 */
    PARAM_BIND   param;

    /* determine the number of rows to insert when irow = 0 */
    if ( !irow && stmt->stmt_options.rows_in_set > 1 ) /* batch wise */
    {
        insert_count= stmt->stmt_options.rows_in_set;
        query_length= ext_query->length;
    }

    do
    {
        /* Have we called exec_stmt_query() as a result of exceeding data size for transmission? If
           so then we need to reset the pos. and start building a new statement. */
        if ( break_insert )
        {
            ext_query->length= query_length;
            /* "break_insert=FALSE" here? */
        }

        /* For each row, build the value list from its columns */
        while (count < insert_count)
        {
            to= net->buff;

            /* Append values for each column. */
            dynstr_append_mem(ext_query,"(", 1);
            for ( ncol= 0; ncol < result->field_count; ncol++ )
            {
                MYSQL_FIELD *field= mysql_fetch_field_direct(result,ncol);
                BIND        *bind= stmt->bind+ncol;
                SQLINTEGER   binding_offset= 0;
                SQLUINTEGER  element_size= 0;
                SQLLEN       ind_or_len;

                if (stmt->stmt_options.bind_type != SQL_BIND_BY_COLUMN &&
                    stmt->stmt_options.bind_offset)
                  binding_offset= *(stmt->stmt_options.bind_offset);

                if (stmt->stmt_options.bind_type != SQL_BIND_BY_COLUMN)
                  element_size= stmt->stmt_options.bind_type;

                if (bind->pcbValue)
                  ind_or_len= *(SQLLEN *)((char *)bind->pcbValue +
                                          binding_offset +
                                          count * (element_size ?
                                                   element_size :
                                                   sizeof(SQLLEN)));
                else
                  ind_or_len= bind->cbValueMax;

                param.SqlType= get_sql_data_type(stmt, field, NULL);
                param.CType = bind->fCType;
                param.buffer= ((char *)bind->rgbValue +
                               binding_offset +
                               count * (element_size ?
                                        element_size :
                                        bind_length(bind->fCType,
                                                    bind->cbValueMax)));

                switch (ind_or_len) {
                case SQL_NTS:
                  if (param.buffer)
                    length= strlen(param.buffer);
                  break;
                /*
                  We pass through SQL_COLUMN_IGNORE and SQL_NULL_DATA,
                  because the insert_data() that is eventually called knows
                  how to deal with them.
                */
                case SQL_COLUMN_IGNORE:
                case SQL_NULL_DATA:
                default:
                  length= ind_or_len;
                }

                param.actual_len= &length;

                if (copy_rowdata(stmt, param, &net, &to) != SQL_SUCCESS)
                  return SQL_ERROR;

            } /* END OF for (ncol= 0; ncol < result->field_count; ncol++) */

            length= (uint) ((char *)to - (char*) net->buff);
            dynstr_append_mem(ext_query, (char*) net->buff, length-1);
            dynstr_append_mem(ext_query, "),", 2);
            count++;

            /*
              We have a limited capacity to shove data across the wire, but
              we handle this by sending in multiple calls to exec_stmt_query()
            */
            if ((size_t)ext_query->length + (size_t)length >=
                (size_t)net_buffer_length)
            {
                break_insert= TRUE;
                break;
            }

        }  /* END OF while(count < insert_count) */

        ext_query->str[--ext_query->length]= '\0';
        if ( exec_stmt_query(stmt, ext_query->str, ext_query->length) !=
             SQL_SUCCESS )
            return(SQL_ERROR);

    } while ( break_insert && count < insert_count );

    /* get rows affected count */
    stmt->affected_rows= stmt->dbc->mysql.affected_rows= insert_count;

    /* update row status pointer(s) */
    if (stmt->stmt_options.rowStatusPtr)
    {
      for (count= insert_count; count--; )
        stmt->stmt_options.rowStatusPtr[count]= SQL_ROW_ADDED;
    }
    if (stmt->stmt_options.rowStatusPtr_ex)
    {
      for (count= insert_count; count--; )
        stmt->stmt_options.rowStatusPtr_ex[count]= SQL_ROW_ADDED;
    }

    return SQL_SUCCESS;
}


/*!
    \brief  Shadow function for SQLSetPos.
    
            Sets the cursor position in a rowset and allows an application
            to refresh data in the rowset or to update or delete data in
            the result set.

    \param  hstmt   see SQLSetPos
    \param  irow    see SQLSetPos
    \param  fOption see SQLSetPos
    \param  fLock   see SQLSetPos

    \return SQLRETURN   see SQLSetPos

    \sa     SQLSetPos
*/

static const char *alloc_error= "Driver Failed to set the internal dynamic result";


static SQLRETURN SQL_API my_SQLSetPos( SQLHSTMT hstmt, SQLUSMALLINT irow, SQLUSMALLINT fOption, SQLUSMALLINT fLock )
{
    STMT FAR  *stmt= (STMT FAR*) hstmt;
    SQLRETURN sqlRet= SQL_SUCCESS;
    MYSQL_RES *result= stmt->result;

    CLEAR_STMT_ERROR(stmt);

    if ( !result )
        return set_error(stmt,MYERR_S1010,NULL,0);

    /* If irow > maximum rows in the resultset */
    if ( fOption != SQL_ADD && irow > result->row_count )
        return set_error(stmt,MYERR_S1107,NULL,0);

    /* Not a valid lock type ..*/
    if ( fLock != SQL_LOCK_NO_CHANGE )
        return set_error(stmt,MYERR_S1C00,NULL,0);

    switch ( fOption )
    {
        case SQL_POSITION:
            {
                if ( irow == 0 )
                    return set_error(stmt,MYERR_S1109,NULL,0);

                if ( irow > stmt->rows_found_in_set )
                    return set_error(stmt,MYERR_S1107,NULL,0);

                /* If Dynamic cursor, fetch the latest resultset */
                if ( if_dynamic_cursor(stmt) && set_dynamic_result(stmt) )
                {
                    return set_error(stmt,MYERR_S1000, alloc_error, 0);
                }

                pthread_mutex_lock(&stmt->dbc->lock);
                irow--;
                sqlRet= SQL_SUCCESS;
                stmt->cursor_row= (long)(stmt->current_row+irow);
                mysql_data_seek(stmt->result,(my_ulonglong)stmt->cursor_row);
                stmt->current_values= mysql_fetch_row(stmt->result);
                stmt->last_getdata_col= (uint)  ~0;; /* reset */
                if ( stmt->fix_fields )
                    stmt->current_values= (*stmt->fix_fields)(stmt,stmt->current_values);
                else
                    stmt->result_lengths= mysql_fetch_lengths(stmt->result);
                /*
                 The call to mysql_fetch_row() moved stmt->result's internal
                 cursor, but we don't want that. We seek back to this row
                 so the MYSQL_RES is in the state we expect.
                */
                mysql_data_seek(stmt->result,(my_ulonglong)stmt->cursor_row);
                pthread_mutex_unlock(&stmt->dbc->lock);
                break;
            }

        case SQL_DELETE:
            {
                DYNAMIC_STRING dynQuery;

                if ( irow > stmt->rows_found_in_set )
                    return set_error(stmt,MYERR_S1107,NULL,0);

                /* IF dynamic cursor THEN rerun query to refresh resultset */
                if ( if_dynamic_cursor(stmt) && set_dynamic_result(stmt) )
                    return set_error(stmt,MYERR_S1000, alloc_error, 0);

                /* start building our DELETE statement */
                if ( init_dynamic_string(&dynQuery, "DELETE FROM ", 1024, 1024) )
                    return set_error(stmt,MYERR_S1001,NULL,4001);

                sqlRet = setpos_delete( stmt, irow, &dynQuery );
                dynstr_free(&dynQuery);
                break;
            }

        case SQL_UPDATE:
            {
                DYNAMIC_STRING dynQuery;

                if ( irow > stmt->rows_found_in_set )
                    return set_error(stmt,MYERR_S1107,NULL,0);

                /* IF dynamic cursor THEN rerun query to refresh resultset */
                if ( if_dynamic_cursor(stmt) && set_dynamic_result(stmt) )
                    return set_error(stmt,MYERR_S1000, alloc_error, 0);

                if ( init_dynamic_string(&dynQuery, "UPDATE ", 1024, 1024) )
                    return set_error(stmt,MYERR_S1001,NULL,4001);

                sqlRet= setpos_update(stmt,irow,&dynQuery);
                dynstr_free(&dynQuery);
                break;
            }

        case SQL_ADD:
            {
                const char  *   table_name;
                DYNAMIC_STRING  dynQuery;
                SQLUSMALLINT    nCol        = 0;

                if ( if_dynamic_cursor(stmt) && set_dynamic_result(stmt) )
                    return set_error(stmt,MYERR_S1000, alloc_error, 0);
                result= stmt->result;

                if ( !(table_name= find_used_table(stmt)) )
                    return SQL_ERROR;

                if ( init_dynamic_string(&dynQuery, "INSERT INTO ", 1024,1024) )
                    return set_stmt_error(stmt, "S1001", "Not enough memory",
                                          4001);

                dynstr_append_quoted_name(&dynQuery,table_name);
                dynstr_append_mem(&dynQuery,"(",1);

                /* build list of column names */
                for (nCol= 0; nCol < result->field_count; nCol++)
                {
                    MYSQL_FIELD *field= mysql_fetch_field_direct(result, nCol);
                    dynstr_append_quoted_name(&dynQuery, field->name);
                    dynstr_append_mem(&dynQuery, ",", 1);
                }
                dynQuery.length--;        /* Remove last ',' */
                dynstr_append_mem(&dynQuery,") VALUES ",9);

                /* process row(s) using our INSERT as base */
                sqlRet= batch_insert(stmt, irow, &dynQuery);

                dynstr_free(&dynQuery);
                break;
            }

        case SQL_REFRESH:
            {
                /*
                  Bug ...SQL_REFRESH is not suppose to fetch any
                  new rows, instead it needs to refresh the positioned
                  buffers
                */
                sqlRet= my_SQLExtendedFetch(hstmt, SQL_FETCH_ABSOLUTE, irow,
                                            stmt->stmt_options.rowsFetchedPtr,
                                            stmt->stmt_options.rowStatusPtr_ex ?
                                            stmt->stmt_options.rowStatusPtr_ex :
                                            stmt->stmt_options.rowStatusPtr, 0);
                break;
            }

        default:
            return set_error(stmt,MYERR_S1009,NULL,0);
    }
    return sqlRet;
}


/*
  @type    : ODBC 1.0 API
  @purpose : associates a cursor name with an active statement.
  If an application does not call SQLSetCursorName, the driver
  generates cursor names as needed for SQL statement processing
*/

SQLRETURN SQL_API SQLSetCursorName(SQLHSTMT hstmt, SQLCHAR *szCursor,
                                   SQLSMALLINT cbCursor)
{
    STMT FAR *stmt= (STMT FAR*) hstmt;

    CLEAR_STMT_ERROR(stmt);

    if ( !szCursor )
        return set_error(stmt,MYERR_S1009,NULL,0);

    if ( cbCursor == SQL_NTS )
        cbCursor= (SQLSMALLINT) strlen((char*) szCursor);

    if ( cbCursor < 0 )
        return set_error(stmt,MYERR_S1090,NULL,0);

    if ( (cbCursor == 0) ||
         (cbCursor > MYSQL_MAX_CURSOR_LEN) ||
         (myodbc_casecmp((char*) szCursor, "SQLCUR", 6) == 0)  ||
         (myodbc_casecmp((char*) szCursor, "SQL_CUR", 7) == 0) )
        return set_error(stmt,MYERR_34000,NULL,0);

    x_free(stmt->cursor.name);
    stmt->cursor.name= dupp_str((char*) szCursor,cbCursor);
    return SQL_SUCCESS;
}


/*
  @type    : ODBC 1.0 API
  @purpose : returns the cursor name associated with a specified statement
*/

SQLRETURN SQL_API SQLGetCursorName(SQLHSTMT hstmt, SQLCHAR FAR *szCursor,
                                   SQLSMALLINT cbCursorMax,
                                   SQLSMALLINT FAR *pcbCursor)
{
    STMT FAR    *stmt= (STMT FAR*) hstmt;
    SQLINTEGER  nLength;
    SQLSMALLINT nDummyLength;

    CLEAR_STMT_ERROR(stmt);

    if ( cbCursorMax < 0 )
        return set_error(stmt,MYERR_S1090,NULL,0);

    if ( !pcbCursor )
        pcbCursor= &nDummyLength;

    if ( cbCursorMax )
        cbCursorMax-= sizeof(char);

    if ( !stmt->cursor.name )
        set_dynamic_cursor_name(stmt);

    *pcbCursor= strlen(stmt->cursor.name);
    if ( szCursor && cbCursorMax > 0 )
        strmake((char*) szCursor, stmt->cursor.name, cbCursorMax);

    nLength= myodbc_min(*pcbCursor , cbCursorMax);

    if ( nLength != *pcbCursor )
        return set_error(stmt,MYERR_01004,NULL,0);

    return SQL_SUCCESS;
}


/*
  @type    : ODBC 1.0 API
  @purpose : sets the cursor position in a rowset and allows an application
  to refresh data in the rowset or to update or delete data in
  the result set
*/

SQLRETURN SQL_API SQLSetPos(SQLHSTMT hstmt, SQLSETPOSIROW irow,
                            SQLUSMALLINT fOption, SQLUSMALLINT fLock)
{
    return my_SQLSetPos(hstmt,irow,fOption,fLock);
}

/*
  @type    : ODBC 1.0 API
  @purpose : performs bulk insertions and bulk bookmark operations,
  including update, delete, and fetch by bookmark
*/

SQLRETURN SQL_API SQLBulkOperations(SQLHSTMT  Handle, SQLSMALLINT Operation)
{
    if ( Operation == SQL_ADD )
        return my_SQLSetPos(Handle, 0, SQL_ADD, SQL_LOCK_NO_CHANGE);

    return set_error(Handle,MYERR_S1C00,NULL,0);
}


/*
  @type    : ODBC 3.0 API
  @purpose : closes a cursor that has been opened on a statement
  and discards any pending results
*/

SQLRETURN SQL_API SQLCloseCursor(SQLHSTMT Handle)
{
    return  my_SQLFreeStmt(Handle, SQL_CLOSE);
}
