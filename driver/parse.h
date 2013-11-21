/*
  Copyright (c) 2013, Oracle and/or its affiliates. All rights reserved.

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

const char *mystr_get_prev_token(CHARSET_INFO *charset,
                                        const char **query, const char *start);
const char *mystr_get_next_token(CHARSET_INFO *charset,
                                        const char **query, const char *end);
const char *find_token(CHARSET_INFO *charset, const char * begin,
                       const char * end, const char * target);
const char *find_first_token(CHARSET_INFO *charset, const char * begin,
                       const char * end, const char * target);
const char *skip_leading_spaces(const char *str);

int is_set_names_statement(SQLCHAR *query);
int is_select_statement(SQLCHAR *query);
#endif
