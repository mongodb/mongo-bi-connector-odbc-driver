/*
  Copyright (c) 2012, Oracle and/or its affiliates. All rights reserved.

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
  @file  my_stmt.c
  @brief Some "methods" for STMT "class" - functions that dispatch a call to either
         prepared statement version or to regular statement version(i.e. using mysql_stmt_*
         of mysql_* functions of API, depending of that has been used in that STMT.
         also contains "mysql_*" versions of "methods".
*/

#include "driver.h"

BOOL ssps_used(STMT *stmt)
{
  return stmt->ssps != NULL;
}


/* Errors processing? */
BOOL returned_result(STMT *stmt)
{
  if (ssps_used(stmt))
  {
    /* Basically at this point we are supposed to get result already */
    return stmt->result ? TRUE : mysql_stmt_result_metadata(stmt->ssps) != NULL;
  }
  else
  {
    return mysql_field_count(&stmt->dbc->mysql) > 0 ;
  }
}


my_bool free_current_result(STMT *stmt)
{
  if (ssps_used(stmt))
  {
    return mysql_stmt_free_result(stmt->ssps);
  }
  else
  {
    mysql_free_result(stmt->result);
    return '\0';
  }
}


/* Name may be misleading, the idea is stmt - for directly executed statements,
   i.e using mysql_* part of api, ssps - prepared on server, using mysql_stmt
 */
MYSQL_RES * stmt_get_result(STMT *stmt)
{
  /* We can't use USE_RESULT because SQLRowCount will fail in this case! */
  if (if_forward_cache(stmt))
  {
    return mysql_use_result(&stmt->dbc->mysql);
  }
  else
  {
    return mysql_store_result(&stmt->dbc->mysql);
  }
}


MYSQL_RES * get_result(STMT *stmt)
{
  if (ssps_used(stmt))
  {
    /* We do not do store for PS so far */
    stmt->result= mysql_stmt_result_metadata(stmt->ssps);
  }
  else
  {
    stmt->result= stmt_get_result(stmt);
  }

  return stmt->result;
}


unsigned int field_count(STMT *stmt)
{
  if (ssps_used(stmt))
  {
    return mysql_stmt_field_count(stmt->ssps);
  }
  else
  {
    return stmt->result && stmt->result->field_count > 0 ?
      stmt->result->field_count :
      mysql_field_count(&stmt->dbc->mysql);
  }
}


my_ulonglong affected_rows(STMT *stmt)
{
  if (ssps_used(stmt))
  {
    return mysql_stmt_affected_rows(stmt->ssps);
  }
  else
  {
    /* In some cases in c/odbc it cannot be used instead of mysql_num_rows */
    return mysql_affected_rows(&stmt->dbc->mysql);
  }
}

my_ulonglong update_affected_rows(STMT *stmt)
{
  my_ulonglong last_affected;

  last_affected= affected_rows(stmt);

  stmt->affected_rows+= last_affected;

  return last_affected;
}


my_ulonglong num_rows(STMT *stmt)
{
  my_ulonglong offset= scroller_exists(stmt) && stmt->scroller.next_offset > 0 ? 
    stmt->scroller.next_offset - stmt->scroller.row_count : 0;

  if (ssps_used(stmt))
  {
    return  offset + mysql_stmt_num_rows(stmt->ssps);
  }
  else
  {
    return offset + mysql_num_rows(stmt->result);
  }
}


MYSQL_ROW fetch_row(STMT *stmt)
{
  if (ssps_used(stmt))
  {
    int error;
    if (ssps_bind_result(stmt))
    {
      return NULL;
    }

    if ((error= mysql_stmt_fetch(stmt->ssps)))
    {
      if (error != MYSQL_DATA_TRUNCATED || !ssps_0buffers_truncated_only(stmt))
      {
        return NULL;
      }
    }

    return stmt->array;
  }
  else
  {
    return mysql_fetch_row(stmt->result);
  }
}


unsigned long* fetch_lengths(STMT *stmt)
{
  if (ssps_used(stmt))
  {
    return stmt->result_bind[0].length;
  }
  else
  {
    return mysql_fetch_lengths(stmt->result);
  }
}


/* --- Data conversion methods --- */
int get_int(STMT *stmt, ulong column_number, char *value, ulong length)
{
  if (ssps_used(stmt))
  {
     return (int)ssps_get_int64(stmt, column_number, value, length);
  }
  else
  {
    return atoi(value);
  }
}


long long get_int64(STMT *stmt, ulong column_number, char *value, ulong length)
{
  if (ssps_used(stmt))
  {
     return ssps_get_int64(stmt, column_number, value, length);
  }
  else
  {
    return strtoll(value, NULL, 10);
  }
}


char * get_string(STMT *stmt, ulong column_number, char *value, ulong *length,
                  char * buffer)
{
  if (ssps_used(stmt))
  {
     return ssps_get_string(stmt, column_number, value, length, buffer);
  }
  else
  {
    return value;
  }
}


long double get_double(STMT *stmt, ulong column_number, char *value,
                         ulong length)
{
  if (ssps_used(stmt))
  {
    return ssps_get_double(stmt, column_number, value, length);
  }
  else
  {
    return strtold(value, NULL);
  }
}


BOOL is_null(STMT *stmt, ulong column_number, char *value)
{
  if (ssps_used(stmt))
  {
    return *stmt->result_bind[column_number].is_null;
  }
  else
  {
    return value == NULL;
  }
}


void scroller_reset(STMT *stmt)
{
  x_free(stmt->scroller.query);
  stmt->scroller.next_offset= 0;
  stmt->scroller.query= stmt->scroller.offset_pos= NULL;
}

/* @param[in]     selected  - prefetch value in datatsource selected by user
   @param[in]     app_fetchs- how many rows app fetchs at a time,
                              i.e. stmt->ard->array_size
 */
unsigned int calc_prefetch_number(unsigned int selected, unsigned int app_fetchs)
{
  if (selected == 0)
  {
    return 0;
  }

  if (app_fetchs > 1)
  {
    if (app_fetchs > selected)
    {
      return app_fetchs;
    }

    if (selected % app_fetchs > 0)
    {
      return app_fetchs * (selected/app_fetchs + 1);
    }
  }

  return selected;
}


BOOL scroller_exists(STMT * stmt)
{
  return stmt->scroller.offset_pos != NULL;
}

/* Adding limit clause to split resultset retrieving
   TODO it has got too messy...
   @returns updated query length */
void scroller_create(STMT * stmt, char *query, SQLULEN query_len)
{
  /* MAX32_BUFF_SIZE includes place for terminating null, which we do not need
     and will use for comma */
  char rows2fetch[MAX32_BUFF_SIZE];
  size_t len, len2add= 7/*" LIMIT "*/ + MAX64_BUFF_SIZE/*offset*/ - 1;
  MY_LIMIT_CLAUSE limit= find_position4limit(query, query + query_len);

  if (limit.row_count > 0 )
  {
    /* If the query already contains LIMIT we probably do not have to do
       anything. unless maybe "their" limit is much bigger number than ours
       and its absolute value is big enough.
       Numbers 500 and 50000 are tentative
     */
    if (limit.row_count / stmt->scroller.row_count < 500
      && limit.row_count < 50000)
    {
      return;
    }

    stmt->scroller.total_rows= limit.row_count;
  }

  if (limit.offset > 0)
  {
    stmt->scroller.next_offset= limit.offset;
  }

  snprintf(rows2fetch, sizeof(rows2fetch), ",%lu", stmt->scroller.row_count);

  len= strlen(rows2fetch);
  len2add+= len;
  len2add-= (limit.end - limit.begin);

  /*extend_buffer(&stmt->dbc->mysql.net, stmt->query_end, len2add);*/
  stmt->scroller.query_len= query_len+len2add;
  stmt->scroller.query= (char*)my_malloc((size_t)stmt->scroller.query_len + 1,
                                          MYF(MY_ZEROFILL));

  memcpy(stmt->scroller.query, query, limit.begin - query);

  limit.begin= stmt->scroller.query + (limit.begin - query);

  /* If there was no LIMIT clause in the query */
  if (limit.row_count == 0)
  {
    strncpy(limit.begin, " LIMIT ", 7);
  }

  /* That is  where we will update offset */
  stmt->scroller.offset_pos= limit.begin + 7;

  strncpy(stmt->scroller.offset_pos + MAX64_BUFF_SIZE - 1, rows2fetch, len);
  memcpy(stmt->scroller.offset_pos + MAX64_BUFF_SIZE - 1, limit.end,
          query + query_len - limit.end);
  *(stmt->scroller.query + stmt->scroller.query_len)= '\0';
}


/* Returns next offset/maxrow for current fetch*/
unsigned long long scroller_move(STMT * stmt)
{
  snprintf(stmt->scroller.offset_pos, MAX64_BUFF_SIZE - 1 , "%20llu",
    stmt->scroller.next_offset);

  stmt->scroller.next_offset+= stmt->scroller.row_count;

  /* TODO if total rows is set have to check if we do not go beyond it and
          change LIMIT's row count in query if needed. or control fetching! */

  return stmt->scroller.next_offset;
}


SQLRETURN scroller_prefetch(STMT * stmt)
{
  if (stmt->scroller.total_rows > 0
    && stmt->scroller.next_offset > stmt->scroller.total_rows)
  {
    return SQL_NO_DATA;
  }

  MYLOG_QUERY(stmt, stmt->scroller.query);

  pthread_mutex_lock(&stmt->dbc->lock);

  if (mysql_real_query(&stmt->dbc->mysql, stmt->scroller.query,
                        (unsigned long)stmt->scroller.query_len))
  {
    pthread_mutex_unlock(&stmt->dbc->lock);
    return SQL_ERROR;
  }

  get_result(stmt);

  /* I think there is no need to do fix_result_types here */
  pthread_mutex_unlock(&stmt->dbc->lock);

  return SQL_SUCCESS;
}


BOOL scrollable(STMT * stmt, char * query, char * query_end)
{
  if (!is_select_statement(query))
  {
    return FALSE;
  }

  /* FOR UPDATE*/
  {
    const char *before_token= query_end;
    const char *last= mystr_get_prev_token(stmt->dbc->ansi_charset_info,
                                                &before_token,
                                                query);
    const char *prev= mystr_get_prev_token(stmt->dbc->ansi_charset_info,
                                                &before_token,
                                                query);

    if (!myodbc_casecmp(prev,"FOR",3) && !myodbc_casecmp(last,"UPDATE",6) 
      || !myodbc_casecmp(prev,"SHARE",5) && !myodbc_casecmp(last,"MODE",4)
        && !myodbc_casecmp(mystr_get_prev_token(stmt->dbc->ansi_charset_info,
                                             &before_token, query),"LOCK",4)
        && !myodbc_casecmp(mystr_get_prev_token(stmt->dbc->ansi_charset_info,
                                             &before_token, query),"IN",2)
        )
    {
      return FALSE;
    }

    /* we have to tokens - nothing to do*/
    if (prev == query)
    {
      return FALSE;
    }

    before_token= prev - 1;
    /* FROM can be only token before a last one at most
       no need to scroll if there is no FROM clause
     */
    if ( myodbc_casecmp(prev,"FROM", 4)
      && !find_token(stmt->dbc->ansi_charset_info, query, before_token, "FROM"))
    {
      return FALSE;
    }

    /* If there there is LIMIT - most probably there is no need to scroll
       skipping such queries before */
    if ( !myodbc_casecmp(prev,"LIMIT", 5)
      || find_token(stmt->dbc->ansi_charset_info, query, before_token, "LIMIT"))
    {
      return FALSE;
    }
  }

  return TRUE;
}