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
  @file  parse.h
  @brief utilities to parse queries
*/

#ifndef __MYODBC_PARSE_H__
# define __MYODBC_PARSE_H__

typedef struct my_string
{
  char *str;
  uint chars; /* probably it is not needed and is useless */
  uint bytes;
} MY_STRING;

typedef enum myodbcQueryType
{
  myqtSelect= 0,
  myqtInsert,
  myqtUpdate,
  myqtCall,
  myqtShow,
  myqtUse,        /*5*/
  myqtCreateTable,
  myqtCreateProc,
  myqtCreateFunc,
  myqtDropProc,
  myqtDropFunc,   /*10*/
  myqtOptimize,
  myqtOther       /* Any type of query(including those above) that we do not
                     care about for that or other reason */
} QUERY_TYPE_ENUM;

typedef struct qt_resolving
{
  const MY_STRING *           keyword;
  uint                        pos_from;
  uint                        pos_thru;
  QUERY_TYPE_ENUM             query_type;
  const struct qt_resolving * and_rule;
  const struct qt_resolving * or_rule;
} QUERY_TYPE_RESOLVING;

typedef struct
{
  my_bool       returns_rs;
  my_bool       preparable_on_server;
  const char *  server_version;
} MY_QUERY_TYPE;

/* To organize constant data needed for parsing - to keep it in required encoding */
typedef struct syntax_markers
{
  const MY_STRING quote[3];
  const MY_STRING query_sep[2];
  const MY_STRING *escape;
  const MY_STRING *odbc_escape_open;
  const MY_STRING *odbc_escape_close;
  const MY_STRING *param_marker;

  struct my_keywords
  {
    const MY_STRING * select;
    const MY_STRING * insert;
    const MY_STRING * update;
    const MY_STRING * call;
    const MY_STRING * show;
    const MY_STRING * use;
    const MY_STRING * create;
    const MY_STRING * drop;
    const MY_STRING * table;
    const MY_STRING * procedure;
    const MY_STRING * function;

    const MY_STRING * where_;
    const MY_STRING * current;
    const MY_STRING * of;
    const MY_STRING * limit;

  } keyword;
  /* TODO: comments */
  
} MY_SYNTAX_MARKERS;

typedef struct parsed_query
{
  CHARSET_INFO  *cs;        /* We need it for parsing                  */
  char          *query;     /* Original query itself                   */
  char          *query_end; /* query end                               */
  char          *last_char; /* mainly for remove_braces                */
  /*unsigned int  begin;    /* offset 1st meaningful character - 1st token */
  DYNAMIC_ARRAY token;      /* absolute positions of tokens            */
  DYNAMIC_ARRAY param_pos;  /* absolute positions of parameter markers */

  QUERY_TYPE_ENUM query_type;
  const char *  is_batch;   /* Pointer to the begin of a 2nd query in a batch */

} MY_PARSED_QUERY;


typedef struct parser
{
  char              *pos;
  int               bytes_at_pos;
  int               ctype;
  const MY_STRING   *quote;  /* If quote was open - pointer to the quote char */
  MY_PARSED_QUERY   *query;

  const MY_SYNTAX_MARKERS *syntax;
} MY_PARSER;

MY_PARSED_QUERY * init_parsed_query(MY_PARSED_QUERY *pq);
MY_PARSED_QUERY * reset_parsed_query(MY_PARSED_QUERY *pq, char * query,
                                     char * query_end, CHARSET_INFO  *cs);
void              delete_parsed_query(MY_PARSED_QUERY *pq);
int               copy_parsed_query(MY_PARSED_QUERY *src,
                                    MY_PARSED_QUERY *target);

/* Those are taking pointer to MY_PARSED_QUERY as a parameter*/
#define GET_QUERY(pq) (pq)->query
#define GET_QUERY_END(pq) (pq)->query_end
#define GET_QUERY_LENGTH(pq) (GET_QUERY_END(pq) - GET_QUERY(pq))
#define TOKEN_COUNT(pq) (pq)->token.elements
#define PARAM_COUNT(pq) (pq)->param_pos.elements
#define IS_BATCH(pq) ((pq)->is_batch != NULL)

char * get_token(MY_PARSED_QUERY *pq, uint index);
char * get_param_pos(MY_PARSED_QUERY *pq, uint index);

BOOL   returns_result       (MY_PARSED_QUERY *pq);
BOOL   preparable_on_server (MY_PARSED_QUERY *pq, const char *server_version);

char * get_cursor_name      (MY_PARSED_QUERY *pq);

MY_PARSER * init_parser(MY_PARSER *parser, MY_PARSED_QUERY *pq);

/* this will not work for some mb charsets */
#define END_NOT_REACHED(parser) ((parser)->pos < (parser)->query->query_end)
#define BYTES_LEFT(pq, pos) ((pq)->query_end - (pos))
#define CLOSE_QUOTE(parser) (parser)->quote= NULL
#define IS_SPACE(parser) ((parser)->ctype & _MY_SPC)

/* But returns bytes in current character */
int               get_ctype(MY_PARSER *parser);
BOOL              skip_spaces(MY_PARSER *parser);
my_bool           add_token(MY_PARSER *parser);
BOOL              is_escape(MY_PARSER *parser);
const MY_STRING * is_quote(MY_PARSER *parser);
BOOL              open_quote(MY_PARSER *parser, const MY_STRING * quote);
BOOL              is_query_separator(MY_PARSER *parser);
/* Installs position on the character next after closing quote */
char *            find_closing_quote(MY_PARSER *parser);
BOOL              is_param_marker(MY_PARSER *parser);
my_bool           add_parameter(MY_PARSER *parser);
void              step_char(MY_PARSER *parser);
BOOL              tokenize(MY_PARSER *parser);
QUERY_TYPE_ENUM   detect_query_type(MY_PARSER *parser,
                                    const QUERY_TYPE_RESOLVING *rule);

BOOL              compare     (MY_PARSER *parser, const MY_STRING *str);
BOOL              case_compare(MY_PARSED_QUERY *parser, const char *pos,
                               const MY_STRING *str);

BOOL              parse(MY_PARSED_QUERY *pq);


const char *mystr_get_prev_token(CHARSET_INFO *charset,
                                        const char **query, const char *start);
const char *mystr_get_next_token(CHARSET_INFO *charset,
                                        const char **query, const char *end);
const char *find_token(CHARSET_INFO *charset, const char * begin,
                       const char * end, const char * target);
const char *skip_leading_spaces(const char *str);

int         is_set_names_statement  (const SQLCHAR *query);
int         is_select_statement     (const MY_PARSED_QUERY *query);
BOOL        is_drop_procedure       (const SQLCHAR * query);
BOOL        is_drop_function        (const SQLCHAR * query);
BOOL        is_create_procedure     (const SQLCHAR * query);
BOOL        is_create_function      (const SQLCHAR * query);
BOOL        is_use_db               (const SQLCHAR * query);
BOOL        is_call_procedure       (const MY_PARSED_QUERY *query);
BOOL        stmt_returns_result     (const MY_PARSED_QUERY *query);

BOOL        remove_braces           (MY_PARSER *query);

#endif
