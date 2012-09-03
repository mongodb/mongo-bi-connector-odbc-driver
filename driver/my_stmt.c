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
  if (returned_result(stmt))
  {
    if (ssps_used(stmt))
 
    {
      my_bool res= mysql_stmt_free_result(stmt->ssps);
      stmt->result= NULL;

      return res;
    }
    else
    {
      mysql_free_result(stmt->result);
      stmt->result= NULL;
      return '\0';
    }
  }
  return '\0';
}


/* Name may be misleading, the idea is stmt - for directly executed statements,
   i.e using mysql_* part of api, ssps - prepared on server, using mysql_stmt
 */
static
MYSQL_RES * stmt_get_result(STMT *stmt, BOOL force_use)
{
  /* We can't use USE_RESULT because SQLRowCount will fail in this case! */
  if (if_forward_cache(stmt) || force_use)
  {
    return mysql_use_result(&stmt->dbc->mysql);
  }
  else
  {
    return mysql_store_result(&stmt->dbc->mysql);
  }
}


/* For text protocol this get result itself as well. Besides for text protocol
   we need to use/store each resultset of multiple resultsets */
MYSQL_RES * get_result_metadata(STMT *stmt, BOOL force_use)
{
  if (ssps_used(stmt))
  {
    stmt->result= mysql_stmt_result_metadata(stmt->ssps);
  }
  else
  {
    stmt->result= stmt_get_result(stmt, force_use);
  }

  return stmt->result;
}


int bind_result(STMT *stmt)
{
  if (ssps_used(stmt))
  {
    return ssps_bind_result(stmt);
  }

  return 0;
}

int get_result(STMT *stmt)
{
  if (ssps_used(stmt))
  {
    return ssps_get_result(stmt);
  }
  /* Nothing to do here for text protocol */

  return 0;
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


MYSQL_ROW_OFFSET row_seek(STMT *stmt, MYSQL_ROW_OFFSET offset)
{
  if (ssps_used(stmt))
  {
    return mysql_stmt_row_seek(stmt->ssps, offset);
  }
  else
  {
    return mysql_row_seek(stmt->result, offset);
  }
}


void data_seek(STMT *stmt, my_ulonglong offset)
{
  if (ssps_used(stmt))
  {
    mysql_stmt_data_seek(stmt->ssps, offset);
  }
  else
  {
    mysql_data_seek(stmt->result, offset);
  }
}


MYSQL_ROW_OFFSET row_tell(STMT *stmt)
{
  if (ssps_used(stmt))
  {
    return mysql_stmt_row_tell(stmt->ssps);
  }
  else
  {
    return mysql_row_tell(stmt->result);
  }
}


int next_result(STMT *stmt)
{
  free_current_result(stmt);

  if (ssps_used(stmt))
  {
    return mysql_stmt_next_result(stmt->ssps);
  }
  else
  {
    return mysql_next_result(&stmt->dbc->mysql);
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

/* Prepares statement depending on connection option either on a client or
   on a server. Returns SQLRETURN result code since preparing on client or
   server can produce errors, memory allocation to name one.  */
SQLRETURN prepare(STMT *stmt, char * query, SQLINTEGER query_length)
{
  /* TODO: I guess we always have to have query length here */
  if (query_length <= 0)
  {
    query_length= strlen(query);
  }

  reset_parsed_query(&stmt->query, query, query + query_length,
                     stmt->dbc->cxn_charset_info);
  /* Tokenising string, detecting and storing parameters placeholders, removing {}
     So far the only possible error is memory allocation. Thus setting it here.
     If that changes we will need to make "parse" to set error and return rc */
  if (parse(&stmt->query))
  {
    return set_error(stmt, MYERR_S1001, NULL, 4001);
  }

  ssps_close(stmt);
  stmt->param_count= PARAM_COUNT(&stmt->query);
  /* Trusting our parsing we are not using prepared statments unsless there are
     actually parameter markers in it */
  if (!stmt->dbc->ds->no_ssps && PARAM_COUNT(&stmt->query) && !IS_BATCH(&stmt->query)
    && preparable_on_server(&stmt->query, stmt->dbc->mysql.server_version))
  {
    MYLOG_QUERY(stmt, "Using prepared statement");
    ssps_init(stmt);

    /* If the query is in the form of "WHERE CURRENT OF" - we do not need to prepare
       it at the moment */
    if (!get_cursor_name(&stmt->query))
    {
      if (mysql_stmt_prepare(stmt->ssps, query, query_length))
      {
        MYLOG_QUERY(stmt, mysql_error(&stmt->dbc->mysql));

        set_stmt_error(stmt,"HY000",mysql_error(&stmt->dbc->mysql),
                       mysql_errno(&stmt->dbc->mysql));
        translate_error(stmt->error.sqlstate,MYERR_S1000,
                        mysql_errno(&stmt->dbc->mysql));

        return SQL_ERROR;
      }

      stmt->param_count= mysql_stmt_param_count(stmt->ssps);

      /* Getting result metadata */
      if ((stmt->result= mysql_stmt_result_metadata(stmt->ssps)))
      {
        /*stmt->state= ST_SS_PREPARED;*/
        fix_result_types(stmt);
       /*Should we reset stmt->result?*/
      }
    /*assert(stmt->param_count==PARAM_COUNT(&stmt->query));*/
    }
  }

  {
    /* Creating desc records for each parameter */
    uint i;
    for (i= 0; i < stmt->param_count; ++i)
    {
      DESCREC *aprec= desc_get_rec(stmt->apd, i, TRUE);
      DESCREC *iprec= desc_get_rec(stmt->ipd, i, TRUE);
    }
  }

  /* Reset current_param so that SQLParamData starts fresh. */
  stmt->current_param= 0;
  stmt->state= ST_PREPARED;

  return SQL_SUCCESS;
}

/* Scrolled cursor related stuff */
void scroller_reset(STMT *stmt)
{
  x_free(stmt->scroller.query);
  stmt->scroller.next_offset= 0;
  stmt->scroller.query= stmt->scroller.offset_pos= NULL;
}

/* @param[in]     selected  - prefetch value in datatsource selected by user
   @param[in]     app_fetchs- how many rows app fetchs at a time,
                              i.e. stmt->ard->array_size
   @param[in]     max_rows  - limit for maximal number of rows to fetch
 */
unsigned int calc_prefetch_number(unsigned int selected, SQLULEN app_fetchs,
                                  SQLULEN max_rows)
{
  unsigned int result= selected;

  if (selected == 0)
  {
    return 0;
  }

  if (app_fetchs > 1)
  {
    if (app_fetchs > selected)
    {
      result= app_fetchs;
    }

    if (selected % app_fetchs > 0)
    {
      result= app_fetchs * (selected/app_fetchs + 1);
    }
  }

  if (max_rows > 0 && max_rows < result)
  {
    return max_rows;
  }

  return result;
}


BOOL scroller_exists(STMT * stmt)
{
  return stmt->scroller.offset_pos != NULL;
}

/* Initialization of a scroller */
void scroller_create(STMT * stmt, char *query, SQLULEN query_len)
{
  /* MAX32_BUFF_SIZE includes place for terminating null, which we do not need
     and will use for comma */
  const size_t len2add= 7/*" LIMIT "*/ + MAX64_BUFF_SIZE/*offset*/ /*- 1*/ + MAX32_BUFF_SIZE;
  MY_LIMIT_CLAUSE limit= find_position4limit(stmt->dbc->ansi_charset_info,
                                            query, query + query_len);

  stmt->scroller.total_rows= myodbc_max(stmt->stmt_options.max_rows, 0);

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

    stmt->scroller.total_rows= stmt->scroller.total_rows > 0 ?
      myodbc_min(limit.row_count, stmt->scroller.total_rows) :
      limit.row_count;
  }

  stmt->scroller.next_offset= myodbc_max(limit.offset, 0);

  /*extend_buffer(&stmt->dbc->mysql.net, stmt->query_end, len2add);*/
  stmt->scroller.query_len= query_len + len2add - (limit.end - limit.begin);
  stmt->scroller.query= (char*)my_malloc((size_t)stmt->scroller.query_len + 1,
                                          MYF(MY_ZEROFILL));

  memcpy(stmt->scroller.query, query, limit.begin - query);

  /* Forgive me - now limit.begin points to beginning of limit in scroller's
     copy of the query */
  limit.begin= stmt->scroller.query + (limit.begin - query);

  /* If there was no LIMIT clause in the query */
  if (limit.row_count == 0)
  {
    strncpy(limit.begin, " LIMIT ", 7);
  }

  /* That is  where we will update offset */
  stmt->scroller.offset_pos= limit.begin + 7;

  /* putting row count in place. normally should not change or only once */
  snprintf(stmt->scroller.offset_pos + MAX64_BUFF_SIZE - 1, MAX32_BUFF_SIZE + 1,
    ",%*u", MAX32_BUFF_SIZE-1, stmt->scroller.row_count);
  /* cpy'ing end of query from original query - not sure if we will allow to
     have one */
  memcpy(stmt->scroller.offset_pos + MAX64_BUFF_SIZE + MAX32_BUFF_SIZE - 1, limit.end,
          query + query_len - limit.end);
  *(stmt->scroller.query + stmt->scroller.query_len)= '\0';
}


/* Returns next offset/maxrow for current fetch*/
unsigned long long scroller_move(STMT * stmt)
{
  snprintf(stmt->scroller.offset_pos, MAX64_BUFF_SIZE, "%*llu", MAX64_BUFF_SIZE - 1,
    stmt->scroller.next_offset);
  stmt->scroller.offset_pos[MAX64_BUFF_SIZE - 1]=',';

  stmt->scroller.next_offset+= stmt->scroller.row_count;

  return stmt->scroller.next_offset;
}


SQLRETURN scroller_prefetch(STMT * stmt)
{
  if (stmt->scroller.total_rows > 0
    && stmt->scroller.next_offset >= stmt->scroller.total_rows)
  {
    /* (stmt->scroller.next_offset - stmt->scroller.row_count) - current offset,
       0 minimum. scroller initialization makes impossible row_count to be > 
       stmt's max_rows */
     long long count= stmt->scroller.total_rows -
      (stmt->scroller.next_offset - stmt->scroller.row_count);

    if (count > 0)
    {
      snprintf(stmt->scroller.offset_pos + MAX64_BUFF_SIZE, MAX32_BUFF_SIZE,
              "%*u", MAX32_BUFF_SIZE - 1, count);
    }
    else
    {
      return SQL_NO_DATA;
    }
  }

  MYLOG_QUERY(stmt, stmt->scroller.query);

  pthread_mutex_lock(&stmt->dbc->lock);

  if (mysql_real_query(&stmt->dbc->mysql, stmt->scroller.query,
                        (unsigned long)stmt->scroller.query_len))
  {
    pthread_mutex_unlock(&stmt->dbc->lock);
    return SQL_ERROR;
  }

  get_result_metadata(stmt, FALSE);

  /* I think there is no need to do fix_result_types here */
  pthread_mutex_unlock(&stmt->dbc->lock);

  return SQL_SUCCESS;
}


BOOL scrollable(STMT * stmt, char * query, char * query_end)
{
  if (!is_select_statement(&stmt->query))
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
       skipping such queries so far */
    if ( !myodbc_casecmp(prev,"LIMIT", 5)
      || find_token(stmt->dbc->ansi_charset_info, query, before_token, "LIMIT"))
    {
      return FALSE;
    }
  }

  return TRUE;
}
