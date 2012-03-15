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
  if (ssps_used(stmt))
  {
    return mysql_stmt_num_rows(stmt->ssps);
  }
  else
  {
    return mysql_num_rows(stmt->result);
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
