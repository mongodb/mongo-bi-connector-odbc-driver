/*
  Copyright (c) 2007, 2013, Oracle and/or its affiliates. All rights reserved.

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

/*
 * Function prototypes and structures for installer-wrapper functionality.
 */

#ifndef _INSTALLER_H
#define _INSTALLER_H

#include "../MYODBC_CONF.h"
#include "../MYODBC_ODBC.h"

#ifdef __cplusplus
extern "C" {
#endif

/* the different modes used when calling MYODBCSetupDataSourceConfig */
#define CONFIG_ADD 1
#define CONFIG_EDIT 2
#define CONFIG_VIEW 3
#define CONFIG_DRIVER_CONNECT 4

UWORD config_get();
UWORD config_set(UWORD mode);

typedef struct {
  SQLWCHAR *name;
  SQLWCHAR *lib;
  SQLWCHAR *setup_lib;

  SQLCHAR *name8;
  SQLCHAR *lib8;
  SQLCHAR *setup_lib8;
} Driver;

/* SQL_MAX_OPTION_STRING_LENGTH = 256, should be ok */
#define ODBCDRIVER_STRLEN SQL_MAX_OPTION_STRING_LENGTH
#define ODBCDATASOURCE_STRLEN SQL_MAX_OPTION_STRING_LENGTH

Driver *driver_new();
void driver_delete(Driver *driver);
int driver_lookup_name(Driver *driver);
int driver_lookup(Driver *driver);
int driver_from_kvpair_semicolon(Driver *driver, const SQLWCHAR *attrs);
int driver_to_kvpair_null(Driver *driver, SQLWCHAR *attrs, size_t attrslen);

typedef struct {
  SQLWCHAR *name;
  SQLWCHAR *driver; /* driver filename */
  SQLWCHAR *description;
  SQLWCHAR *server;
  SQLWCHAR *uid;
  SQLWCHAR *pwd;
  SQLWCHAR *database;
  SQLWCHAR *socket;
  SQLWCHAR *initstmt;
  SQLWCHAR *charset;
  SQLWCHAR *sslkey;
  SQLWCHAR *sslcert;
  SQLWCHAR *sslca;
  SQLWCHAR *sslcapath;
  SQLWCHAR *sslcipher;

  unsigned int port;
  unsigned int readtimeout;
  unsigned int writetimeout;
  unsigned int clientinteractive;

  SQLCHAR *name8;
  SQLCHAR *driver8;
  SQLCHAR *description8;
  SQLCHAR *server8;
  SQLCHAR *uid8;
  SQLCHAR *pwd8;
  SQLCHAR *database8;
  SQLCHAR *socket8;
  SQLCHAR *initstmt8;
  SQLCHAR *charset8;
  SQLCHAR *sslkey8;
  SQLCHAR *sslcert8;
  SQLCHAR *sslca8;
  SQLCHAR *sslcapath8;
  SQLCHAR *sslcipher8;

  /*  */
  BOOL return_matching_rows;
  BOOL allow_big_results;
  BOOL use_compressed_protocol;
  BOOL change_bigint_columns_to_int;
  BOOL safe;
  BOOL auto_reconnect;
  BOOL auto_increment_null_search;
  BOOL handle_binary_as_char;
  BOOL can_handle_exp_pwd;
  BOOL enable_cleartext_plugin;
  /*  */
  BOOL dont_prompt_upon_connect;
  BOOL dynamic_cursor;
  BOOL ignore_N_in_name_table;
  BOOL user_manager_cursor;
  BOOL dont_use_set_locale;
  BOOL pad_char_to_full_length;
  BOOL dont_cache_result;
  /*  */
  BOOL return_table_names_for_SqlDescribeCol;
  BOOL ignore_space_after_function_names;
  BOOL force_use_of_named_pipes;
  BOOL no_catalog;
  BOOL read_options_from_mycnf;
  BOOL disable_transactions;
  BOOL force_use_of_forward_only_cursors;
  BOOL allow_multiple_statements;
  BOOL limit_column_size;

  BOOL min_date_to_zero;
  BOOL zero_date_to_min;
  BOOL default_bigint_bind_str;
  /* debug */
  BOOL save_queries;
  BOOL no_information_schema;
  /* SSL */
  unsigned int sslverify;
  unsigned int cursor_prefetch_number;
  BOOL no_ssps;
} DataSource;

/* perhaps that is a good idea to have const ds object with defaults */
extern const unsigned int default_cursor_prefetch;

typedef struct{
  SQLCHAR *type_name;
  SQLINTEGER name_length;
  SQLSMALLINT sql_type;
  SQLSMALLINT mysql_type;
  SQLUINTEGER type_length;
  BOOL binary;
}SQLTypeMap;

#define TYPE_MAP_SIZE 32

DataSource *ds_new();
void ds_delete(DataSource *ds);
int ds_set_strattr(SQLWCHAR **attr, const SQLWCHAR *val);
int ds_set_strnattr(SQLWCHAR **attr, const SQLWCHAR *val, size_t charcount);
int ds_lookup(DataSource *ds);
int ds_from_kvpair(DataSource *ds, const SQLWCHAR *attrs, SQLWCHAR delim);
int ds_to_kvpair(DataSource *ds, SQLWCHAR *attrs, size_t attrslen,
                 SQLWCHAR delim);
size_t ds_to_kvpair_len(DataSource *ds);
int ds_add(DataSource *ds);
int ds_exists(SQLWCHAR *name);
char *ds_get_utf8attr(SQLWCHAR *attrw, SQLCHAR **attr8);
int ds_setattr_from_utf8(SQLWCHAR **attr, SQLCHAR *val8);
void ds_set_options(DataSource *ds, ulong options);
ulong ds_get_options(DataSource *ds);

extern const SQLWCHAR W_DRIVER_PARAM[];
extern const SQLWCHAR W_DRIVER_NAME[];
extern const SQLWCHAR W_INVALID_ATTR_STR[];

/*
 * Deprecated connection parameters
 */
#define FLAG_FOUND_ROWS		2   /* Access can't handle affected_rows */
#define FLAG_BIG_PACKETS	8   /* Allow BIG packets. */
#define FLAG_NO_PROMPT		16  /* Don't prompt on connection */
#define FLAG_DYNAMIC_CURSOR	32  /* Enables the dynamic cursor */
#define FLAG_NO_SCHEMA		64  /* Ignore the schema defination */
#define FLAG_NO_DEFAULT_CURSOR	128 /* No default cursor */
#define FLAG_NO_LOCALE		256  /* No locale specification */
#define FLAG_PAD_SPACE		512  /* Pad CHAR:s with space to max length */
#define FLAG_FULL_COLUMN_NAMES	1024 /* Extends SQLDescribeCol */
#define FLAG_COMPRESSED_PROTO	2048 /* Use compressed protocol */
#define FLAG_IGNORE_SPACE	4096 /* Ignore spaces after function names */
#define FLAG_NAMED_PIPE		8192 /* Force use of named pipes */
#define FLAG_NO_BIGINT		16384	/* Change BIGINT to INT */
#define FLAG_NO_CATALOG		32768	/* No catalog support */
#define FLAG_USE_MYCNF		65536L	/* Read my.cnf at start */
#define FLAG_SAFE		131072L /* Try to be as safe as possible */
#define FLAG_NO_TRANSACTIONS  (FLAG_SAFE << 1) /* Disable transactions */
#define FLAG_LOG_QUERY	      (FLAG_SAFE << 2) /* Query logging, debug */
#define FLAG_NO_CACHE	      (FLAG_SAFE << 3) /* Don't cache the resultset */
 /* Force use of forward-only cursors */
#define FLAG_FORWARD_CURSOR   (FLAG_SAFE << 4)
 /* Force auto-reconnect */
#define FLAG_AUTO_RECONNECT   (FLAG_SAFE << 5)
#define FLAG_AUTO_IS_NULL     (FLAG_SAFE << 6) /* 8388608 Enables SQL_AUTO_IS_NULL */
#define FLAG_ZERO_DATE_TO_MIN (1 << 24) /* Convert XXXX-00-00 date to ODBC min date on results */
#define FLAG_MIN_DATE_TO_ZERO (1 << 25) /* Convert ODBC min date to 0000-00-00 on query */
#define FLAG_MULTI_STATEMENTS (1 << 26) /* Allow multiple statements in a query */
#define FLAG_COLUMN_SIZE_S32 (1 << 27) /* Limit column size to a signed 32-bit value (automatically set for ADO) */
#define FLAG_NO_BINARY_RESULT (1 << 28) /* Disables charset 63 for columns with empty org_table */
/*
  When binding SQL_BIGINT as SQL_C_DEFAULT, treat it as a string
  (automatically set for MS Access) see bug#24535
*/
#define FLAG_DFLT_BIGINT_BIND_STR (1 << 29)
/*
  Use SHOW TABLE STATUS instead Information_Schema DB for table metadata
*/
#define FLAG_NO_INFORMATION_SCHEMA (1 << 30)


#ifdef __cplusplus
}
#endif

#endif /* _INSTALLER_H */

