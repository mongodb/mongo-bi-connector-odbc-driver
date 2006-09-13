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
 * CURSOR.C								   *
 *									   *
 * @description: Client side cursor functionality handling		   *
 *									   *
 * @author     : MySQL AB(monty@mysql.com, venu@mysql.com)		   *
 * @date       : 2001-Aug-15						   *
 * @product    : myodbc3						   *
 *									   *
 ****************************************************************************/

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
    MYODBCDbgEnter("find_used_table");

    if ( stmt->table_name && stmt->table_name[0] )
        MYODBCDbgReturn2(stmt->table_name);    

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
                MYODBCDbgReturn2(NULL);
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
                MYODBCDbgReturn2(NULL);
            }
        }
#endif
    }
    /*
      We have to copy the strings as we may have to re-issue the query
      while using cursors.
    */
    stmt->table_name= dupp_str(table_name,SQL_NTS);
    MYODBCDbgReturn2(stmt->table_name);
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


/*
  @type    : myodbc3 internal
  @purpose : checks whether the SQL statement contains WHERE CURRENT OF CURSOR
*/

my_bool check_if_positioned_cursor_exists( STMT FAR *pStmt, STMT FAR **pStmtCursor )
{
    if ( pStmt->query && pStmt->query_end )
    {
        char *      pszQueryTokenPos    = pStmt->query_end;
        const char *pszCursor           = mystr_get_prev_token( (const char **)&pszQueryTokenPos, pStmt->query );

        /*
            Return TRUE if this statement is doing a 'WHERE CURRENT OF' - even when
            we can not find the cursor this statement is referring to.
        */
        if ( !myodbc_casecmp(mystr_get_prev_token((const char **)&pszQueryTokenPos,
                                                  pStmt->query),"OF",2) &&
             !myodbc_casecmp(mystr_get_prev_token((const char **)&pszQueryTokenPos,
                                                  pStmt->query),"CURRENT",7) &&
             !myodbc_casecmp(mystr_get_prev_token((const char **)&pszQueryTokenPos,
                                                  pStmt->query),"WHERE",5) )
        {
            LIST *      list_element;
            LIST *      next_element;
            DBC FAR *   dbc = (DBC FAR*)pStmt->dbc;

            /*
                Scan the list of statements for this connection and see if we can
                find the cursor name this statement is referring to - it must have
                a result set to count.
            */
            for ( list_element = dbc->statements; list_element; list_element = next_element )
            {
                next_element    = list_element->next;
                *pStmtCursor    = (HSTMT)list_element->data;

                /*
                  Might have the cursor in the pStmt without any resultset, so
                  avoid crashes, by keeping check on (*pStmtCursor)->result)
                */
                if ( (*pStmtCursor)->cursor.name &&
                     !myodbc_strcasecmp( (*pStmtCursor)->cursor.name, pszCursor ) &&
                     (*pStmtCursor)->result )
                {
                    /*
                        \todo

                        Well we have found a match so we truncate the 'WHERE CURRENT OF cursorname'
                        but the truncation may be a bad way to go as it may make it broken for future
                        use.
                    */
                    *pszQueryTokenPos = '\0';
                    return ( TRUE );
                }
            }
            /* Did we run out of statements without finding a viable cursor? */ 
            if ( !list_element )
            {
                char buff[200];
                strxmov( buff,"Cursor '", pszCursor, "' does not exist or does not have a result set.", NullS );
                set_stmt_error( pStmt, "34000", buff, ER_INVALID_CURSOR_NAME );
            }
            return ( TRUE );
        }
    }

    return ( FALSE );
}


/*
  @type    : myodbc3 internal
  @purpose : checks whether the Primary Key column exists in the table
  if it exists, returns the PK column name
*/

static SQLRETURN check_if_pk_exists(STMT FAR *stmt)
{
    char buff[NAME_LEN+18];
    MYSQL_ROW row;
    MYSQL_RES *presult;

    if ( stmt->cursor.pk_validated )
        return(stmt->cursor.pk_count);

    /*
      Check for the existence of keys in the table
      We quote the table name to allow weird table names.
      TODO: Write a table-name-quote function and use this instead.
    */
#if MYSQL_VERSION_ID >= 40100
    strxmov(buff,"show keys from `",stmt->result->fields->org_table,"`",NullS);
#else
    strxmov(buff,"show keys from `",stmt->result->fields->table,"`",NullS);
#endif
    MYLOG_QUERY(stmt, buff);
    pthread_mutex_lock(&stmt->dbc->lock);
    if ( mysql_query(&stmt->dbc->mysql,buff) ||
         !(presult= mysql_store_result(&stmt->dbc->mysql)) )
    {
        set_error(stmt,MYERR_S1000,mysql_error(&stmt->dbc->mysql),
                  mysql_errno(&stmt->dbc->mysql));
        pthread_mutex_unlock(&stmt->dbc->lock);
        return(0);
    }

    /*
      TODO: Fix this loop to only return columns that are part of the
      primary key.
    */
    while ( (row= mysql_fetch_row(presult)) &&
            (stmt->cursor.pk_count < MY_MAX_PK_PARTS) )
    {
        /*
          Collect all keys, it may be
          - PRIMARY or
          - UNIQUE NOT NULL
          TODO: Fix this seperately and use the priority..
        */
        strmov(stmt->cursor.pkcol[stmt->cursor.pk_count++].name,row[4]);
    }
    mysql_free_result(presult);
    pthread_mutex_unlock(&stmt->dbc->lock);
    stmt->cursor.pk_validated= 1;
    return(stmt->cursor.pk_count);
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

    if ( irow && rows > 1 )
        return set_error(stmt,MYERR_01S04,NULL,0);

    /*
      If all rows successful, then only update status..else
      don't update...just for the sake of performance..
    */
    else if ( stmt->stmt_options.rowStatusPtr )
    {
        SQLUSMALLINT *ptr= stmt->stmt_options.rowStatusPtr;
        SQLUSMALLINT *end= ptr+rows;

        for ( ; ptr != end ; ptr++ )
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
    MYSQL mysql= stmt->dbc->mysql;
    SQLUINTEGER length= *(param.actual_len)+1;

    if ( !(*to= (SQLCHAR *) extend_buffer(*net,(char*) *to,length)) )
        return set_error(stmt,MYERR_S1001,NULL,4001);

    if ( !(*to= (SQLCHAR*) insert_param(&mysql, (char*) *to, &param)) )
        return set_error(stmt,MYERR_S1001,NULL,4001);

    /* We have to remove zero bytes or we have problems! */
    while ( (*to > orig_to) && (*((*to) - 1) == (SQLCHAR) 0) ) (*to)--;

    /* insert "," */
    param.SqlType= SQL_INTEGER;
    param.CType= SQL_C_CHAR;
    param.buffer= (gptr) ",";
    *param.actual_len= 1;

    if ( !(*to= (SQLCHAR*) insert_param(&mysql,(char*) *to, &param)) )
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
    PARAM_BIND dummy;
    SQLLEN     dummy_len= 5; /* sizeof(" AND ") */
    MYSQL mysql= stmt->dbc->mysql;
    SQLUINTEGER length= *(param->actual_len)+5;

    if ( !(*to= (SQLCHAR*) extend_buffer(*net, (char*) *to,length)) )
        return set_error(stmt,MYERR_S1001,NULL,4001);

    if ( !(*to= (SQLCHAR*) insert_param(&mysql, (char*) *to, param)) )
        return set_error(stmt,MYERR_S1001,NULL,4001);

    /* Insert " AND ", where clause with multiple search */
    dummy.SqlType= SQL_INTEGER;
    dummy.CType= SQL_C_CHAR;
    dummy.buffer= (gptr) " AND ";
    dummy.actual_len= &dummy_len;

    if ( !(*to= (SQLCHAR*) insert_param(&mysql, (char*) *to, &dummy)) )
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
    ulong       transfer_length,precision,display_size;
    MYSQL_FIELD *field= mysql_fetch_field_direct(result,nSrcCol);
    MYSQL_ROW   row_data= result->data_cursor->data + nSrcCol;
    NET         *net=&stmt->dbc->mysql.net;
    SQLCHAR     *to= net->buff;
    SQLLEN      length;

    /* Copy row buffer data to statement */
    param.used= 1;
    param.SqlType= unireg_to_sql_datatype( stmt,
                                           field,
                                           0,
                                           &transfer_length,
                                           &precision,
                                           &display_size );
    param.CType= SQL_C_CHAR;

    if ( row_data && *row_data )
    {
        param.buffer= (gptr) *row_data;
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
    if ( field->type == FIELD_TYPE_FLOAT || field->type == FIELD_TYPE_DOUBLE ||
         field->type == FIELD_TYPE_DECIMAL )
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
  if it is, copy that data to query, else get the data by
  building a temporary resultset
*/

static SQLRETURN insert_pk_fields(STMT FAR *stmt, DYNAMIC_STRING *dynQuery)
{
    MYSQL_RES    *result= stmt->result;
    MYSQL_FIELD  *field;
    SQLUSMALLINT ncol;
    uint      index;
    MYCURSOR     *cursor= &stmt->cursor;
    SQLUINTEGER  pk_count= 0;

    /*
      Look for primary key columns in the current result set,
      if it exists, take that data else query new resultset
    */
    for ( ncol= 0; ncol < result->field_count; ncol++ )
    {
        field= result->fields+ncol;
        for ( index= 0; index < cursor->pk_count; index++ )
        {
            if ( !myodbc_strcasecmp(cursor->pkcol[index].name,field->name) )
            {
                /* PK data exists...*/
                dynstr_append_quoted_name(dynQuery,field->name);
                dynstr_append_mem(dynQuery,"=",1);
                if ( insert_field(stmt,result,dynQuery,ncol) )
                    return(SQL_ERROR);
                cursor->pkcol[index].bind_done= TRUE;
                pk_count++;
                break;
            }
        }
    }
    if ( pk_count != cursor->pk_count )
    {
        /*
          Primary key column doesn't exists in the opened rs, so
          get the data by executing a query
        */
        DYNAMIC_STRING query;
        MYSQL_RES *presult;
        SQLUSMALLINT field_count= 0;

        if ( init_dynamic_string(&query, "SELECT ", 1024,1024) )
            return set_error(stmt,MYERR_S1001,NULL,4001);

        for ( index= 0; index < cursor->pk_count; index++ )
        {
            if ( !cursor->pkcol[index].bind_done )
            {
                dynstr_append_quoted_name(&query,stmt->cursor.pkcol[index].name);
                dynstr_append_mem(&query,",",1);
            }
        }
        query.length-= 1;
        dynstr_append_mem(&query," FROM ",6);

        if ( !find_used_table(stmt) )
        {
            dynstr_free(&query);
            return(SQL_ERROR);
        }

        dynstr_append_quoted_name(&query,stmt->table_name);
        MYLOG_QUERY(stmt, query.str);
        pthread_mutex_lock(&stmt->dbc->lock);
        if ( mysql_query(&stmt->dbc->mysql,query.str) ||
             !(presult= mysql_store_result(&stmt->dbc->mysql)) )
        {
            set_error(stmt,MYERR_S1000,mysql_error(&stmt->dbc->mysql),
                      mysql_errno(&stmt->dbc->mysql));
            pthread_mutex_unlock(&stmt->dbc->lock);
            dynstr_free(&query);
            return(SQL_ERROR);
        }
        pthread_mutex_unlock(&stmt->dbc->lock);

        for ( index= 0;index< (uint) stmt->current_row;index++ )
            presult->data_cursor= presult->data_cursor->next;

        for ( index= 0; index < cursor->pk_count; index++ )
        {
            if ( !cursor->pkcol[index].bind_done )
            {
                dynstr_append_quoted_name(dynQuery,cursor->pkcol[index].name);
                dynstr_append_mem(dynQuery,"=",1);

                /*
                  Might have multiple pk fields in the missing list ..
                  so avoid the wrong query by having internal field_count..
                */
                if ( insert_field(stmt,presult,dynQuery,field_count++) )
                {
                    mysql_free_result(presult);
                    dynstr_free(&query);
                    return(SQL_ERROR);
                }
            }
        }
        mysql_free_result(presult);
        dynstr_free(&query);
    }
    return(SQL_SUCCESS);
}


/*
  @type    : myodbc3 internal
  @purpose : copies all resultset column data to where clause
*/

static SQLRETURN insert_fields( STMT FAR *      stmt, 
                                DYNAMIC_STRING *dynQuery )
{
    MYSQL_RES *     result = stmt->result;
    MYSQL_FIELD *   field;
    SQLUSMALLINT    ncol;
    MYSQL_RES *     presultAllColumns;
    char            select[NAME_LEN+15];

    /* get base table name - fails if we have more than one */
    if ( !( find_used_table( stmt ) ) )
        return ( SQL_ERROR );

    /* get a list of all table cols using "SELECT & LIMIT 0" method */
    strxmov( select, "SELECT * FROM `", stmt->table_name, "` LIMIT 0", NullS );
    MYLOG_QUERY( stmt, select );
    pthread_mutex_lock( &stmt->dbc->lock );
    if ( ( mysql_query( &stmt->dbc->mysql, select ) || !( presultAllColumns = mysql_store_result( &stmt->dbc->mysql ) ) ) )
    {
        set_error( stmt, MYERR_S1000, mysql_error( &stmt->dbc->mysql ), mysql_errno( &stmt->dbc->mysql ) );
        pthread_mutex_unlock( &stmt->dbc->lock );
        return ( SQL_ERROR );
    }
    pthread_mutex_unlock( &stmt->dbc->lock );

    /*
      If current result set field count is not the total
      count from the actual table, then use the temp result
      to have a search condition from all table fields ..
  
      This can be buggy, if multiple times the same
      column is used in the select ..rare case ..
    */
/* printf( "\n[PAH][%s][%d]\n", __FILE__, __LINE__ ); */
    if ( presultAllColumns->row_count != result->row_count && !if_dynamic_cursor(stmt) )
    {
/* printf( "\n[PAH][%s][%d]\n", __FILE__, __LINE__ ); */
        mysql_free_result(presultAllColumns);
        presultAllColumns= 0;
    }
    else if ( presultAllColumns->field_count != result->field_count ||
              !result->data_cursor ||
              (if_dynamic_cursor(stmt) &&
               presultAllColumns->row_count != result->row_count) )
    {
        for ( ncol= 0; ncol < (SQLUSMALLINT)stmt->current_row; ncol++ )
        {
/* printf( "\n[PAH][%s][%d] Column %d of %d\n", __FILE__, __LINE__, ncol, stmt->current_row ); */
            presultAllColumns->data_cursor = presultAllColumns->data_cursor->next;
        }
        result = presultAllColumns;
    }

/* printf( "\n[PAH][%s][%d]\n", __FILE__, __LINE__ ); */
    pthread_mutex_lock( &stmt->dbc->lock );
    /* Copy all row buffers to query search clause */
    for ( ncol = 0; ncol < result->field_count; ncol++ )
    {
        field = result->fields + ncol;
        dynstr_append_quoted_name( dynQuery, field->name );
        dynstr_append_mem( dynQuery, "=", 1 );

        if ( if_float_field( stmt, field ) || insert_field( stmt, result, dynQuery, ncol ) )
        {
            mysql_free_result( presultAllColumns );
            pthread_mutex_unlock( &stmt->dbc->lock );
            return ( SQL_ERROR );
        }
    }

    mysql_free_result( presultAllColumns );
    pthread_mutex_unlock( &stmt->dbc->lock );

    return( SQL_SUCCESS );
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

    /* IF pk exists THEN use pk cols for where ELSE use all cols */
    if ( check_if_pk_exists( pStmt ) )
    {
        if ( insert_pk_fields( pStmt, dynQuery ) != SQL_SUCCESS )
            return set_stmt_error( pStmt, "HY000", "Build WHERE -> insert_pk_fields() failed.", 0 );
    }
    else
    {
        if ( insert_fields( pStmt, dynQuery ) != SQL_SUCCESS )
            return set_stmt_error( pStmt, "HY000", "Build WHERE -> insert_fields() failed.", 0 );
    }
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
        set_dynamic(&stmtNew->params,(gptr) param,pcount);
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
    ulong         transfer_length,precision,display_size;
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

        if ( bind && !bind->field )
        {
            set_stmt_error(stmt,"21S02",
                           "Degree of derived table does not match column list",0);
            return(SQL_ERROR);
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
        param.SqlType= unireg_to_sql_datatype( stmt,field,
                                               0,
                                               &transfer_length,
                                               &precision,
                                               &display_size );
        param.CType= bind->fCType;
        param.buffer= (gptr) bind->rgbValue+irow*bind->cbValueMax;
        param.ValueMax= bind->cbValueMax;
        /*
            Check when SQL_LEN_DATA_AT_EXEC() macro was used instead of data length
        */
        if ( length == SQL_NTS )
            length= strlen(param.buffer);
                else if ( length <= SQL_LEN_DATA_AT_EXEC_OFFSET )
                    length= -( length - SQL_LEN_DATA_AT_EXEC_OFFSET );

        param.actual_len= &length;

        if ( copy_rowdata(stmt,param,&net,&to) != SQL_SUCCESS )
            return(SQL_ERROR);

        length= (uint) ((char *)to - (char*) net->buff);
        dynstr_append_mem(dynQuery, (char*) net->buff, length);
    }
    if ( ignore_count == result->field_count )
        return(ER_ALL_COLUMNS_IGNORED);

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
    MYODBCDbgPrint("SQL_DELETE:",("%s",dynQuery->str));
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

    my_SQLFreeStmt( pStmtTemp, SQL_DROP );

    return ( nReturn );
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
/* printf( "\n[PAH][%s][%d] %d\n", __FILE__, __LINE__, rowset_pos ); */
        nReturn = build_where_clause( stmt, dynQuery, (SQLUSMALLINT)rowset_pos );
/* printf( "\n[PAH][%s][%d] (%s)\n", __FILE__, __LINE__, dynQuery ); */
        if ( !SQL_SUCCEEDED( nReturn ) )
            return nReturn;

        /* execute our DELETE statement */
        MYODBCDbgPrint("SQLPOS_DELETE:",("%s",dynQuery->str));
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
        if ( nReturn == ER_ALL_COLUMNS_IGNORED )
        {
            /*
          All columns ignored in the update list, continue
          to the next statement ..
            */
            nReturn= SQL_SUCCESS;
            continue;
        }
        else if ( nReturn == SQL_ERROR )
            return SQL_ERROR;

        nReturn = build_where_clause( stmt, dynQuery, (SQLUSMALLINT)rowset_pos );
        if ( !SQL_SUCCEEDED( nReturn ) )
            return nReturn;

        MYODBCDbgPrint("SQLPOS_UPDATE:",("%s",dynQuery->str));
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
    NET          *net;
    SQLUSMALLINT ncol;
    SQLCHAR      *to;
    ulong        query_length= 0;           /* our original query len so we can reset pos if break_insert   */
    my_bool      break_insert= FALSE;       /* true if we are to exceed max data size for transmission      
                                               but this seems to be misused                                 */
    MYSQL        mysql= stmt->dbc->mysql ;
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

        /* 
            for each row build and execute INSERT statement 
        */
        while ( count < insert_count )
        {
            net= &mysql.net;
            to = net->buff;

            /* 
                for each *relevant* column append to INSERT statement 
            */
            dynstr_append_mem(ext_query,"(", 1);
            for ( ncol= 0; ncol < result->field_count; ncol++ )
            {
                ulong       transfer_length,precision,display_size;
                MYSQL_FIELD *field= mysql_fetch_field_direct(result,ncol);
                BIND        *bind= stmt->bind+ncol;

                param.SqlType= unireg_to_sql_datatype(stmt,
                                                      field,
                                                      0,
                                                      &transfer_length,
                                                      &precision,
                                                      &display_size);
                param.CType = bind->fCType;
                param.buffer= (gptr) bind->rgbValue+count*(stmt->stmt_options.bind_type);

                if ( !( bind->pcbValue && ( *bind->pcbValue == SQL_COLUMN_IGNORE ) ) )
                {
                    if ( param.buffer )
                    {
                        if ( bind->pcbValue )
                        {
                            if ( *bind->pcbValue == SQL_NTS )
                                length= strlen(param.buffer);
                            else if ( *bind->pcbValue == SQL_COLUMN_IGNORE )
                            {
                                /* should not happen see CSC-3985 */
                                length= SQL_NULL_DATA;
                            }
                            else
                                length= *bind->pcbValue;
                        }
                        else
                            length= bind->cbValueMax;
                    }
                    else
                        length= SQL_NULL_DATA;

                    param.actual_len= &length;

                    if ( copy_rowdata(stmt,param,&net,&to) != SQL_SUCCESS )
                        return(SQL_ERROR);
                }

            } /* END OF for (ncol= 0; ncol < result->field_count; ncol++) */

            length= (uint) ((char *)to - (char*) net->buff);
            dynstr_append_mem(ext_query, (char*) net->buff, length-1);
            dynstr_append_mem(ext_query, "),", 2);
            count++;

            /* We have a limited capacity to shove data across the wire. 
               but we handle this by sending in multiple calls to exec_stmt_query(). */
            if ( ext_query->length + length >= (SQLLEN) net_buffer_length )
            {
                break_insert= TRUE;
                break;
            }

        }  /* END OF while(count < insert_count) */

        ext_query->str[--ext_query->length]= '\0';
        MYODBCDbgPrint("batch_insert:",("%s",ext_query->str));
        if ( exec_stmt_query(stmt, ext_query->str, ext_query->length) !=
             SQL_SUCCESS )
            return(SQL_ERROR);

    } while ( break_insert && count < insert_count );

    /* get rows affected count */
    stmt->affected_rows= stmt->dbc->mysql.affected_rows= insert_count;

    /* update row status pointer(s) */
    if ( stmt->stmt_options.rowStatusPtr )
    {
        for ( count= insert_count; count--; )
            stmt->stmt_options.rowStatusPtr[count]= SQL_ROW_ADDED;
    }

    return(SQL_SUCCESS);
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
    MYODBCDbgEnter("SQLSetPos");
    MYODBCDbgPrint("enter",("irow: %d fOption: %s   Lock: %d",
                        irow,MYODBCDbgPosTypeString(fOption),fLock));

    CLEAR_STMT_ERROR(stmt);

    if ( !result )
        MYODBCDbgReturn(set_error(stmt,MYERR_S1010,NULL,0));

    /* If irow > maximum rows in the resultset */
    if ( fOption != SQL_ADD && irow > result->row_count )
        MYODBCDbgReturn(set_error(stmt,MYERR_S1107,NULL,0));

    /* Not a valid lock type ..*/
    if ( fLock != SQL_LOCK_NO_CHANGE )
        MYODBCDbgReturn(set_error(stmt,MYERR_S1C00,NULL,0));

    switch ( fOption )
    {
        case SQL_POSITION:
            {
                if ( irow == 0 )
                    MYODBCDbgReturn(set_error(stmt,MYERR_S1109,NULL,0));

                if ( irow > stmt->rows_found_in_set )
                    MYODBCDbgReturn(set_error(stmt,MYERR_S1107,NULL,0));

                /* If Dynamic cursor, fetch the latest resultset */
                if ( if_dynamic_cursor(stmt) && set_dynamic_result(stmt) )
                {
                    MYODBCDbgReturn(set_error(stmt,MYERR_S1000, alloc_error, 0));
                }

                pthread_mutex_lock(&stmt->dbc->lock);
                irow--;
                sqlRet= SQL_SUCCESS;
                stmt->cursor_row= (long)(stmt->current_row+irow);
                mysql_data_seek(stmt->result,(my_ulonglong)stmt->cursor_row);
                stmt->current_values= stmt->result->data_cursor->data;
                stmt->last_getdata_col= (uint)  ~0;; /* reset */
                if ( stmt->fix_fields )
                    stmt->current_values= (*stmt->fix_fields)(stmt,stmt->current_values);
                else
                    stmt->result_lengths= mysql_fetch_lengths(stmt->result);
                pthread_mutex_unlock(&stmt->dbc->lock);
                break;
            }

        case SQL_DELETE:
            {
                DYNAMIC_STRING dynQuery;

                if ( irow > stmt->rows_found_in_set )
                    MYODBCDbgReturn(set_error(stmt,MYERR_S1107,NULL,0));

                /* IF dynamic cursor THEN rerun query to refresh resultset */
                if ( if_dynamic_cursor(stmt) && set_dynamic_result(stmt) )
                    MYODBCDbgReturn(set_error(stmt,MYERR_S1000, alloc_error, 0));

                /* start building our DELETE statement */
                if ( init_dynamic_string(&dynQuery, "DELETE FROM ", 1024, 1024) )
                    MYODBCDbgReturn(set_error(stmt,MYERR_S1001,NULL,4001));

/* printf( "\n[PAH][%s][%d]\n", __FILE__, __LINE__ ); */
                sqlRet = setpos_delete( stmt, irow, &dynQuery );
/* printf( "\n[PAH][%s][%d]\n", __FILE__, __LINE__ ); */
                dynstr_free(&dynQuery);
                break;
            }

        case SQL_UPDATE:
            {
                DYNAMIC_STRING dynQuery;

                if ( irow > stmt->rows_found_in_set )
                    MYODBCDbgReturn(set_error(stmt,MYERR_S1107,NULL,0));

                /* IF dynamic cursor THEN rerun query to refresh resultset */
                if ( if_dynamic_cursor(stmt) && set_dynamic_result(stmt) )
                    MYODBCDbgReturn(set_error(stmt,MYERR_S1000, alloc_error, 0));

                if ( init_dynamic_string(&dynQuery, "UPDATE ", 1024, 1024) )
                    MYODBCDbgReturn(set_error(stmt,MYERR_S1001,NULL,4001));

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
                    MYODBCDbgReturn(set_error(stmt,MYERR_S1000, alloc_error, 0));
                result= stmt->result;

                if ( !(table_name= find_used_table(stmt)) )
                    MYODBCDbgReturn(SQL_ERROR);

                if ( init_dynamic_string(&dynQuery, "INSERT INTO ", 1024,1024) )
                    MYODBCDbgReturn(set_stmt_error(stmt,"S1001","Not enough memory",4001));

                dynstr_append_quoted_name(&dynQuery,table_name);
                dynstr_append_mem(&dynQuery,"(",1);

                /*
                    for each *relevant* column append name to INSERT statement
                */    
                for ( nCol= 0; nCol < result->field_count; nCol++ )
                {
                    MYSQL_FIELD *   field   = mysql_fetch_field_direct( result, nCol );
                    BIND *          bind    = stmt->bind + nCol;

                    if ( !( bind->pcbValue && ( *bind->pcbValue == SQL_COLUMN_IGNORE ) ) )
                    {
                        dynstr_append_quoted_name( &dynQuery, field->name );
                        dynstr_append_mem( &dynQuery, ",", 1 );
                    }
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
                                            stmt->stmt_options.rowStatusPtr, 0);
                break;
            }

        default:
            MYODBCDbgReturn(set_error(stmt,MYERR_S1009,NULL,0));
    }
    MYODBCDbgReturn(sqlRet);
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
    MYODBCDbgEnter("SQLSetCursorName");

    CLEAR_STMT_ERROR(stmt);

    if ( !szCursor )
        MYODBCDbgReturn(set_error(stmt,MYERR_S1009,NULL,0));

    if ( cbCursor == SQL_NTS )
        cbCursor= (SQLSMALLINT) strlen((char*) szCursor);

    if ( cbCursor < 0 )
        MYODBCDbgReturn(set_error(stmt,MYERR_S1090,NULL,0));

    if ( (cbCursor == 0) ||
         (cbCursor > MYSQL_MAX_CURSOR_LEN) ||
         (myodbc_casecmp((char*) szCursor, "SQLCUR", 6) == 0)  ||
         (myodbc_casecmp((char*) szCursor, "SQL_CUR", 7) == 0) )
        MYODBCDbgReturn(set_error(stmt,MYERR_34000,NULL,0));

    x_free((gptr) stmt->cursor.name);
    stmt->cursor.name= dupp_str((char*) szCursor,cbCursor);
    MYODBCDbgReturn(SQL_SUCCESS);
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
    MYODBCDbgEnter("SQLGetCursorName");

    CLEAR_STMT_ERROR(stmt);

    if ( cbCursorMax < 0 )
        MYODBCDbgReturn(set_error(stmt,MYERR_S1090,NULL,0));

    if ( !pcbCursor )
        pcbCursor= &nDummyLength;

    if ( cbCursorMax )
        cbCursorMax-= sizeof(char);

    if ( !stmt->cursor.name )
        set_dynamic_cursor_name(stmt);

    *pcbCursor= strlen(stmt->cursor.name);
    if ( szCursor && cbCursorMax > 0 )
        strmake((char*) szCursor, stmt->cursor.name, cbCursorMax);

    nLength= min(*pcbCursor , cbCursorMax);

    if ( nLength != *pcbCursor )
        MYODBCDbgReturn(set_error(stmt,MYERR_01004,NULL,0));

    MYODBCDbgReturn(SQL_SUCCESS);
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
    MYODBCDbgEnter("SQLBulkOperations");

    if ( Operation == SQL_ADD )
        MYODBCDbgReturn(my_SQLSetPos(Handle, 0, SQL_ADD, SQL_LOCK_NO_CHANGE));

    MYODBCDbgReturn(set_error(Handle,MYERR_S1C00,NULL,0));
}


/*
  @type    : ODBC 3.0 API
  @purpose : closes a cursor that has been opened on a statement
  and discards any pending results
*/

SQLRETURN SQL_API SQLCloseCursor(SQLHSTMT Handle)
{
    MYODBCDbgEnter("SQLCloseCursor");
    MYODBCDbgReturn(my_SQLFreeStmt(Handle, SQL_CLOSE));
}
