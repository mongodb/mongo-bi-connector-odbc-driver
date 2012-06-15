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
  @file  parse.c
  @brief utilities to parse queries
*/

#include "driver.h"

const char *mystr_get_prev_token(CHARSET_INFO *charset,
                                        const char **query, const char *start)
{
  const char *pos= *query;

  do
  {
    if (pos == start)
      return (*query = start);     /* Return start of string */
    --pos;
  } while (*pos < 0 || !my_isspace(charset, *pos)) ;

  *query= pos;      /* Remember pos to space */

  return pos + 1;   /* Return found token */
}


/*TODO test it*/
const char *mystr_get_next_token(CHARSET_INFO *charset,
                                        const char **query, const char *end)
{
  const char *pos= *query;

  do
  {
    if (pos == end)
      return (*query = end);     /* Return start of string */
    ++pos;
  } while (*pos > 0 && my_isspace(charset, *pos)) ;

  /* Looking for space after token */
  *query= pos + 1;

  while (*query != end && (**query < 0 || !myodbc_isspace(charset, *query, end)))
    ++*query;

  return pos;   /* Return found token */
}


const char * find_token(CHARSET_INFO *charset, const char * begin,
                        const char * end, const char * target)
{
  const char * token, *before= end;

  /* we will not check 1st token in the string - no need at the moment */
  while ((token= mystr_get_prev_token(charset,&before, begin)) != begin)
  {
    if (!myodbc_casecmp(token, target, strlen(target)))
    {
      return token;
    }
  }

  return NULL;
}


const char * skip_leading_spaces(const char *str)
{
  while (str && isspace(*str))
    ++str;

  return str;
}

/* TODO: We can't have a separate function for detecting of
         each type of a query */
/**
 Detect if a statement is a SET NAMES statement.
*/
int is_set_names_statement(const SQLCHAR *query)
{
  query= skip_leading_spaces(query);
  return myodbc_casecmp((char *)query, "SET NAMES", 9) == 0;
}


/**
Detect if a statement is a SELECT statement.
*/
int is_select_statement(const SQLCHAR *query)
{
  query= skip_leading_spaces(query);
  return myodbc_casecmp((char *)query, "SELECT", 6) == 0;
}


/* That has to be rewritten in a better way */
BOOL is_batch_of_statements(const SQLCHAR *query)
{
  const char * sep1=strchr(query, ';'), *sep2= strstr(query, "\\g");

  while (sep1 != NULL || sep2 != NULL)
  {
    const char *token= sep1 + 1;

    if (sep1 != NULL && sep2 != NULL)
    {
      token= myodbc_min(sep1, sep2);
    }
    else if (sep2 != NULL)
    {
      token= sep2;
      token+= 2;
    }
    
    token= skip_leading_spaces(token);

    if (*token == '\0')
    {
      return TRUE;
    }

    if ((!myodbc_casecmp(token, "INSERT", 6) || !myodbc_casecmp(token, "DELETE", 6)
      || !myodbc_casecmp(token, "UPDATE", 6) || !myodbc_casecmp(token, "SELECT", 6)
      || !myodbc_casecmp(token, "CREATE", 6))
      && (*(token+6) == '\0' || isspace(*(token+6)))
      || (!myodbc_casecmp(token, "CALL", 4) || !myodbc_casecmp(token, "SHOW", 4)
      || !myodbc_casecmp(token, "DROP", 4))
      && (*(token+4) == '\0' || isspace(*(token+4))))
    {
      return TRUE;
    }

    sep1= strchr(token, ';');
    sep2= strstr(token, "\\g");
  }

  return FALSE;
}


/* These functions expect that leasding spaces have been skipped */
BOOL is_drop_procedure(const SQLCHAR * query)
{
  if (myodbc_casecmp(query, "DROP", 4) == 0 && *(query+4) != '\0'
    && isspace(*(query+4)))
  {
    query= skip_leading_spaces(query+5);
    return myodbc_casecmp(query, "PROCEDURE", 9) == 0;
  }

  return FALSE;
}


BOOL is_drop_function(const SQLCHAR * query)
{
  if (myodbc_casecmp(query, "DROP", 4) == 0 && *(query+4) != '\0'
    && isspace(*(query+4)))
  {
    query= skip_leading_spaces(query+5);
    return myodbc_casecmp(query, "FUNCTION", 8) == 0;
  }

  return FALSE;
}


/* In fact this function catches all CREATE queries with DEFINER as well.
   But so far we are fine with that and even are using that.*/
BOOL is_create_procedure(const SQLCHAR * query)
{
  if (myodbc_casecmp(query, "CREATE", 6) == 0 && *(query+6) != '\0'
    && isspace(*(query+6)))
  {
    query= skip_leading_spaces(query+7);

    if (myodbc_casecmp(query, "DEFINER", 7) == 0)
    {
      return TRUE;
    }

    return myodbc_casecmp(query, "PROCEDURE", 9) == 0;
  }

  return FALSE;
}


BOOL is_create_function(const SQLCHAR * query)
{
  if (myodbc_casecmp(query, "CREATE", 6) == 0 && *(query+6) != '\0'
    && isspace(*(query+6)))
  {
    query= skip_leading_spaces(query+7);
    return myodbc_casecmp(query, "FUNCTION", 8) == 0;
  }

  return FALSE;
}


BOOL is_use_db(const SQLCHAR * query)
{
  if (myodbc_casecmp(query, "USE", 3) == 0 && *(query+3) != '\0'
    && isspace(*(query+3)))
  {
    return TRUE;
  }

  return FALSE;
}


BOOL is_call_procedure(const SQLCHAR * query)
{
  query= skip_leading_spaces(query);

  if (myodbc_casecmp(query, "CALL", 4) == 0 && *(query+4) != '\0'
    && isspace(*(query+4)))
  {
    return TRUE;
  }

  return FALSE;
}