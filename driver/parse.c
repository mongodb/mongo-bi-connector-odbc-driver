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

static const MY_QUERY_TYPE query_type[]=
{
  /*myqtSelect*/      {'\1', '\1', NULL},
  /*myqtInsert*/      {'\0', '\1', NULL},
  /*myqtUpdate*/      {'\0', '\1', NULL},
  /*myqtCall*/        {'\1', '\1', "5.5.3"},
  /*myqtShow*/        {'\1', '\1', NULL},
  /*myqtUse*/         {'\0', '\0', NULL},
  /*myqtCreateTable*/ {'\0', '\0', NULL},
  /*myqtCreateProc*/  {'\0', '\0', NULL},
  /*myqtCreateFunc*/  {'\0', '\0', NULL},
  /*myqtDropProc*/    {'\0', '\0', NULL},
  /*myqtDropFunc*/    {'\0', '\0', NULL},
  /*myqtOptimize*/    {'\0', '\1', "5.0.23"},/*to check*/
  /*myqtOther*/       {'\0', '\1', NULL},
};

/*static? */
static const MY_STRING escape=       {"\\" , 1, 1};
static const MY_STRING odbc_open=    {"{"  , 1, 1};
static const MY_STRING odbc_close=   {"}"  , 1, 1};
static const MY_STRING param_marker= {"?"  , 1, 1};

static const MY_STRING select_=    {"SELECT"   , 6, 6};
static const MY_STRING insert=     {"INSERT"   , 6, 6};
static const MY_STRING update=     {"UPDATE"   , 6, 6};
static const MY_STRING call=       {"CALL"     , 4, 4};
static const MY_STRING show=       {"SHOW"     , 4, 4};
static const MY_STRING use=        {"USE"      , 3, 3};
static const MY_STRING create=     {"CREATE"   , 6, 6};
static const MY_STRING drop=       {"DROP"     , 4, 4};
static const MY_STRING table=      {"TABLE"    , 5, 5};
static const MY_STRING procedure=  {"PROCEDURE", 9, 9};
static const MY_STRING function=   {"FUNCTION" , 8, 8};
static const MY_STRING where_=     {"WHERE"    , 5, 5};
static const MY_STRING current=    {"CURRENT"  , 7, 7};
static const MY_STRING of=         {"OF"       , 2, 2};
static const MY_STRING limit=      {"LIMIT"    , 5, 5};
static const MY_STRING optimize=   {"OPTIMIZE" , 8, 8};

static const MY_SYNTAX_MARKERS ansi_syntax_markers= {/*quote*/
                                              {
                                                {"'", 1, 1}, {"\"", 1, 1},
                                                {"`", 1, 1}
                                              },
                                              /*query_sep*/
                                              {{";", 1, 1}, {"\\g", 2, 2}},
                                              /*escape*/      &escape,
                                              /*odbc open*/   &odbc_open,
                                              /*odbc close*/  &odbc_close,
                                              /*param marker*/&param_marker,
                                              /*keyword*/
                                              {
                                                &select_, &insert, &update,
                                                &call, &show, &use, &create,
                                                &drop, &table, &procedure,
                                                &function, &where_, &current,
                                                &of, &limit
                                              }
                                             };

static const QUERY_TYPE_RESOLVING func_rule=
{ &function,  1,          4,          myqtCreateFunc,   NULL, NULL };
static const QUERY_TYPE_RESOLVING proc_rule=
{ &procedure, 1,          4,          myqtCreateProc,   NULL, &func_rule };
static const QUERY_TYPE_RESOLVING drop_func_rule=
{ &function,  1,          4,          myqtDropFunc,     NULL, NULL };
static const QUERY_TYPE_RESOLVING drop_proc_rule=
{ &procedure, 1,          4,          myqtDropProc,     NULL, &drop_func_rule };
static const QUERY_TYPE_RESOLVING crt_table_rule=
{ &table,     1,          2,          myqtCreateTable,  NULL, &proc_rule };


static const QUERY_TYPE_RESOLVING rule[]=
{ /*keyword*/ /*pos_from*/ /*pos_thru*/ /*query_type*/ /*and_rule*/ /*or_rule*/
  { &select_,   0,          0,          myqtSelect,     NULL,       NULL},
  { &call,      0,          0,          myqtCall,       NULL,       NULL},
  { &insert,    0,          0,          myqtInsert,     NULL,       NULL},
  { &update,    0,          0,          myqtUpdate,     NULL,       NULL},
  { &show,      0,          0,          myqtShow,       NULL,       NULL},
  { &create,    0,          0,          myqtOther,      &crt_table_rule, NULL},
  { &drop,      0,          0,          myqtOther,      &drop_proc_rule, NULL},
  { &use,       0,          0,          myqtUse,        NULL,       NULL},
  { &optimize,  0,          0,          myqtOptimize,   NULL,       NULL},
  {NULL, 0, 0, myqtOther, NULL, NULL}
};

MY_PARSED_QUERY * init_parsed_query(MY_PARSED_QUERY *pq)
{
  if (pq != NULL)
  {
    pq->query=      NULL;
    pq->query_end=  NULL;
    pq->last_char=  NULL;
    pq->is_batch=   NULL;

    pq->query_type= myqtOther;

    /* TODO: Store offsets rather than ptrs. In this case we will be fine
       if work with copy of the originally parsed string */
    my_init_dynamic_array(&pq->token,     sizeof(uint), 20, 10);
    my_init_dynamic_array(&pq->param_pos, sizeof(uint), 10, 10);
  }

  return pq;
}


MY_PARSED_QUERY * reset_parsed_query(MY_PARSED_QUERY *pq, char * query,
                                     char * query_end, CHARSET_INFO *cs)
{
  if (pq)
  {
    x_free(pq->query);

    reset_dynamic(&pq->token);
    reset_dynamic(&pq->param_pos);

    pq->last_char= NULL;
    pq->is_batch=  NULL;

    pq->query_type= myqtOther;

    pq->query= query;

    if (pq->query == NULL)
    { 
      pq->cs=         NULL;
      pq->query_end=  NULL;
    }
    else
    {
      pq->cs=         cs;
      pq->query_end=  query_end == NULL ? query + strlen(query) : query_end;
    }
  }

  return pq;
}


void delete_parsed_query(MY_PARSED_QUERY *pq)
{
  if (pq)
  {
    x_free(pq->query);

    delete_dynamic(&pq->token);
    delete_dynamic(&pq->param_pos);
  }
}


int copy_parsed_query(MY_PARSED_QUERY* src, MY_PARSED_QUERY *target)
{
  char * dummy= my_strdup(GET_QUERY(src), MYF(0));

  if (dummy == NULL)
  {
    return 1;
  }

  reset_parsed_query(target, dummy,
                     dummy + GET_QUERY_LENGTH(src), target->cs);


  if (src->last_char != NULL)
  {
    target->last_char= target->query + (src->last_char - src->query);
  }
  if (src->is_batch != NULL)
  {
    target->is_batch= target->query + (src->is_batch - src->query);
  }

  target->query_type= src->query_type;

  if (allocate_dynamic(&target->token, src->token.elements))
  {
    return 1;
  }
  /* it would be still better to use public dyn array interface... */
  memcpy(target->token.buffer, src->token.buffer,
        src->token.elements*src->token.size_of_element);
  target->token.elements= src->token.elements;

  if (allocate_dynamic(&target->param_pos, src->param_pos.elements))
  {
    return 1;
  }
  memcpy(target->param_pos.buffer, src->param_pos.buffer,
        src->param_pos.elements*src->token.size_of_element);
  target->param_pos.elements= src->param_pos.elements;

  return 0;
}


MY_PARSER * init_parser(MY_PARSER * parser, MY_PARSED_QUERY *pq)
{
  parser->query=  pq;
  parser->pos=    GET_QUERY(pq);
  parser->quote=  NULL;

  get_ctype(parser);

  /* TODO: loading it in required encoding */
  parser->syntax= &ansi_syntax_markers;

  return parser;
}


char * get_token(MY_PARSED_QUERY *pq, uint index)
{
  if (index < pq->token.elements)
  {
    return GET_QUERY(pq) + ((uint *) pq->token.buffer)[index];
  }

  return NULL;
}


char * get_param_pos(MY_PARSED_QUERY *pq, uint index)
{
  if (index < pq->param_pos.elements)
  {
    return GET_QUERY(pq) + ((uint *) pq->param_pos.buffer)[index];
  }

  return NULL;
}


BOOL returns_result(MY_PARSED_QUERY *pq)
{
  return query_type[pq->query_type].returns_rs;
}


BOOL preparable_on_server(MY_PARSED_QUERY *pq, const char *server_version)
{
  if (query_type[pq->query_type].preparable_on_server)
  {
    return query_type[pq->query_type].server_version == NULL
        || is_minimum_version(server_version,
                            query_type[pq->query_type].server_version);
  }

  return FALSE;
}


char * get_cursor_name(MY_PARSED_QUERY *pq)
{
  if (TOKEN_COUNT(pq) > 4)
  {
    if ( case_compare(pq, get_token(pq, TOKEN_COUNT(pq) - 4), &where_)
      && case_compare(pq, get_token(pq, TOKEN_COUNT(pq) - 3), &current)
      && case_compare(pq, get_token(pq, TOKEN_COUNT(pq) - 2), &of))
    {
      return get_token(pq, TOKEN_COUNT(pq) - 1);
    }
  }

  return NULL;
}


/* But returns bytes in current character. not sure that is needed though */
int  get_ctype(MY_PARSER *parser)
{
  if (END_NOT_REACHED(parser))
  {
    parser->bytes_at_pos= parser->query->cs->cset->ctype(parser->query->cs,
                                      &parser->ctype, (const uchar*)parser->pos,
                                      (const uchar*) parser->query->query_end);
  }
  else
  {
    parser->bytes_at_pos= 0;
  }

  return parser->bytes_at_pos;
}


const char *mystr_get_prev_token(CHARSET_INFO *charset,
                                        const char **query, const char *start)
{
  const char *pos= *query, *end= *query;

  do
  {
    if (pos == start)
      return (*query = start);     /* Return start of string */
    --pos;
  } while (*pos < 0 || !myodbc_isspace(charset, pos, end)) ;

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
  } while (*pos > 0 && myodbc_isspace(charset, pos, pos + 1)) ;

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
int is_select_statement(const MY_PARSED_QUERY *query)
{
  return query->query_type == myqtSelect;
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


BOOL is_call_procedure(const MY_PARSED_QUERY * query)
{
  return query->query_type == myqtCall;
}


/*!
    \brief  Returns true if we are dealing with a statement which
            is likely to result in reading only (SELECT || SHOW).

            Some ODBC calls require knowledge about a statement
            which we can not determine until we have executed 
            the statement. This is because we do not parse the SQL
            - the server does.

            However if we silently execute a pending statement we
            may insert rows.

            So we do a very crude check of the SQL here to reduce 
            the chance of a problem.

    \sa     BUG 5778            
*/
BOOL stmt_returns_result(const MY_PARSED_QUERY *query)
{
  if (query->query_type <= myqtOther)
  {
    return query_type[query->query_type].returns_rs;
  }
  return FALSE;
}


/* TRUE if end has been reached */
BOOL skip_spaces(MY_PARSER *parser)
{
  while(END_NOT_REACHED(parser) && IS_SPACE(parser))
  {
    step_char(parser);
  }

  return !END_NOT_REACHED(parser);
}


my_bool add_token(MY_PARSER *parser)
{
  if (END_NOT_REACHED(parser))
  {
    uint offset= (uint)(parser->pos - GET_QUERY(parser->query));

    return push_dynamic(&parser->query->token, (uchar *)&offset);
  }

  return '\0';
}


BOOL is_escape(MY_PARSER *parser)
{
  return parser->bytes_at_pos == parser->syntax->escape->bytes
      && memcmp(parser->pos, parser->syntax->escape->str,
                parser->bytes_at_pos) == 0;
}

const MY_STRING * is_quote(MY_PARSER *parser)
{
  int i;

  for (i=0; i < sizeof(parser->syntax->quote)/sizeof(MY_STRING); ++i)
  {
    if (parser->bytes_at_pos == parser->syntax->quote[i].bytes
      && memcmp(parser->pos, parser->syntax->quote[i].str,
                parser->bytes_at_pos) == 0)
    {
      return &parser->syntax->quote[i];
    }
  }

  return NULL;
}

/*static?*/
BOOL is_closing_quote(MY_PARSER *parser)
{
  return  parser->bytes_at_pos == parser->quote->bytes
      && memcmp(parser->pos, parser->quote->str,
                parser->bytes_at_pos) == 0;
}


/* Installs position on the character next after closing quote */
char * find_closing_quote(MY_PARSER *parser)
{
  char *closing_quote= NULL;
  while(END_NOT_REACHED(parser))
  {
    if (is_escape(parser))
    {
      step_char(parser);
    }
    else if (is_closing_quote(parser))
    {
      closing_quote= parser->pos;

      step_char(parser);

      /* if end of atr or not a new quote
         Basically that does not have to be the same quote type - mysql will
         concat them */
      if (!get_ctype(parser) || !open_quote(parser, is_quote(parser)))
      {
        break;
      }
    }

    step_char(parser);
  }

  return closing_quote;
}


BOOL is_param_marker(MY_PARSER *parser)
{
  return parser->bytes_at_pos == parser->syntax->param_marker->bytes
      && memcmp(parser->pos, parser->syntax->param_marker->str,
                parser->bytes_at_pos) == 0;

}


my_bool add_parameter(MY_PARSER *parser)
{
  uint offset= (uint)(parser->pos - GET_QUERY(parser->query));
  return push_dynamic(&parser->query->param_pos, (uchar *)&offset);
}


void step_char(MY_PARSER *parser)
{
  parser->pos+= parser->bytes_at_pos;

  if (END_NOT_REACHED(parser))
  {
    get_ctype(parser);
  }
}


BOOL open_quote(MY_PARSER *parser, const MY_STRING * quote)
{
  if (quote != NULL)
  {
    parser->quote= quote;
    return TRUE;
  }

  return FALSE;
}


BOOL is_query_separator(MY_PARSER *parser)
{
  int i;

  for (i=0; i < sizeof(parser->syntax->query_sep)/sizeof(MY_STRING); ++i)
  {
    if (compare(parser, &parser->syntax->query_sep[i]))
    {
      parser->pos+= parser->syntax->query_sep[i].bytes;
      get_ctype(parser);

      return TRUE;
    }
  }

  return FALSE;
}


/* Perhaps it can be just int(failed/succeeded) */
BOOL tokenize(MY_PARSER *parser)
{
  /* TODO: token info should contain length of a token */
  
  skip_spaces(parser);
  /* 1st token - otherwise we lose it if it is on 0 position without spaces
     ahead of it */
  if (add_token(parser))
  {
    return TRUE;
  }

  while(END_NOT_REACHED(parser))
  {
    if (parser->quote)
    {
      parser->query->last_char= find_closing_quote(parser);
      /*assert(parser->last_char!=NULL); /* no closing quote? */

      CLOSE_QUOTE(parser);
      /* find_closing_quote puts cursor after the closing quote
         thus we need to "continue" and not to lose that character */
      continue;
    }
    else
    {
      if (IS_SPACE(parser))
      {
        step_char(parser);

        if (skip_spaces(parser))
        {
          continue;
        }

        /* adding token after spaces */
        if (add_token(parser))
        {
          return TRUE;
        }
      }

      /* is_query_separator moves position to the 1st char of the next query */
      if (is_query_separator(parser))
      {
        skip_spaces(parser);

        if (add_token(parser))
        {
          return TRUE;
        }

        continue;
      }

      parser->query->last_char= parser->pos;

      if (open_quote(parser, is_quote(parser)))
      {
        /* Separate token for a quote (mind select"a")*/
        if (add_token(parser))
        {
          return TRUE;
        }
      }
      else if (is_param_marker(parser))
      {
        if (add_parameter(parser))
        {
          return TRUE;
        }
      }
    }

    step_char(parser);
  }

  return FALSE;
}


/* Returns TRUE if the rule has succeded and type has been identified */
static
BOOL process_rule(MY_PARSER *parser, const QUERY_TYPE_RESOLVING *rule)
{
  uint i;
  char *token;

  for (i= rule->pos_from;
       i <= myodbc_min(rule->pos_thru > 0 ? rule->pos_thru : rule->pos_from,
                      TOKEN_COUNT(parser->query) - 1);
       ++i)
  {
    token= get_token(parser->query, i);

    if (parser->pos && case_compare(parser->query, token, rule->keyword))
    {
      if (rule->and_rule)
      {
        return process_rule(parser, rule->and_rule);
      }
      else
      {
        parser->query->query_type= rule->query_type;
        return TRUE;
      }
    }
  }

  if (rule->or_rule)
  {
    return process_rule(parser, rule->or_rule);
  }

  return FALSE;
}


QUERY_TYPE_ENUM detect_query_type(MY_PARSER *parser,
                                  const QUERY_TYPE_RESOLVING *rule)
{
  while (rule->keyword != NULL)
  {
    if (process_rule(parser, rule))
    {
      return parser->query->query_type;
    }

    ++rule;
  }

  return myqtOther;
}

BOOL compare(MY_PARSER *parser, const MY_STRING *str)
{
  if (str && BYTES_LEFT(parser->query, parser->pos) >= (int)str->bytes)
  {
    return memcmp(parser->pos, str->str, str->bytes) == 0;
  }

  return FALSE;
}


BOOL case_compare(MY_PARSED_QUERY *pq, const char *pos, const MY_STRING *str)
{
  if (str && BYTES_LEFT(pq, pos) >= (int)str->bytes)
  {
    /* to check: if myodbc_casecmp suits */
    return myodbc_casecmp(pos, str->str, str->bytes) == 0;
  }

  return FALSE;
}


BOOL parse(MY_PARSED_QUERY *pq)
{
  MY_PARSER parser;
    
  init_parser(&parser, pq);

  if (tokenize(&parser))
  {
    return TRUE;
  }

  /* If the query is embrased in curly braces - we just need to remove them
   or server won't understand us */
  remove_braces(&parser);

  detect_query_type(&parser, rule);

  return FALSE;
}


/* Removes qurly braces off embraced query. Query has to be parsed
   Returns TRUE if braces were removed */
BOOL remove_braces(MY_PARSER *parser)
{
  /* To remove brace we need to parse the query to the end anyway */
  /* parse(parser);*/

  /* TODO: multibyte case */
  if (parser->query->token.elements > 0)
  {
    char *token= get_token(parser->query, 0);

    /* TODO: what about batch of queries - do we need to care? */
    /* only doing our unthankful job if we have both opening and closing braces
       on 1st ans last position.*/
    if (token && token[0] == parser->syntax->odbc_escape_open->str[0]
      && parser->query->last_char
      && *parser->query->last_char == parser->syntax->odbc_escape_close->str[0])
    {
      token[0]= ' ';
      *parser->query->last_char= ' ';

      parser->pos= token;

      get_ctype(parser);

      /* If next character after opening brace is space - then we have erased
         1st token and need to delete it */
      if (IS_SPACE(parser))
      {
        delete_dynamic_element(&parser->query->token, 0);
      }

      /* If we had "{}" - we would have erase the only token */
      if (TOKEN_COUNT(parser->query) > 0)
      {
        token= get_token(parser->query, TOKEN_COUNT(parser->query) - 1);

        if (parser->query->last_char == token)
        {
          delete_dynamic_element(&parser->query->token,
                                  TOKEN_COUNT(parser->query) - 1);
        }
      }
      
      /* Not the last char form now on */
      parser->query->last_char= NULL;

      return TRUE;
    }
  }

  return FALSE;
}
