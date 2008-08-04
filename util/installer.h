/*
  Copyright (C) 2007 MySQL AB

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
  SQLWCHAR *option;
  SQLWCHAR *charset;
  SQLWCHAR *sslkey;
  SQLWCHAR *sslcert;
  SQLWCHAR *sslca;
  SQLWCHAR *sslcapath;
  SQLWCHAR *sslcipher;

  unsigned int port;

  SQLCHAR *name8;
  SQLCHAR *driver8;
  SQLCHAR *description8;
  SQLCHAR *server8;
  SQLCHAR *uid8;
  SQLCHAR *pwd8;
  SQLCHAR *database8;
  SQLCHAR *socket8;
  SQLCHAR *initstmt8;
  SQLCHAR *option8;
  SQLCHAR *charset8;
  SQLCHAR *sslkey8;
  SQLCHAR *sslcert8;
  SQLCHAR *sslca8;
  SQLCHAR *sslcapath8;
  SQLCHAR *sslcipher8;

  /* flags 1 */
  BOOL dont_optimize_column_width;
  BOOL return_matching_rows;
  BOOL allow_big_results;
  BOOL use_compressed_protocol;
  BOOL change_bigint_columns_to_int;
  BOOL safe;
  BOOL enable_auto_reconnect;
  BOOL enable_auto_increment_null_search;
  BOOL handle_binary_as_char;
  /* flags 2 */
  BOOL dont_prompt_upon_connect;
  BOOL enable_dynamic_cursor;
  BOOL ignore_N_in_name_table;
  BOOL user_manager_cursor;
  BOOL dont_use_set_locale;
  BOOL pad_char_to_full_length;
  BOOL dont_cache_result;
  /* flags 3 */
  BOOL return_table_names_for_SqlDesribeCol;
  BOOL ignore_space_after_function_names;
  BOOL force_use_of_named_pipes;
  BOOL no_catalog;
  BOOL read_options_from_mycnf;
  BOOL disable_transactions;
  BOOL force_use_of_forward_only_cursors;
  BOOL allow_multiple_statements;
  BOOL limit_column_size;
  /* debug */
  BOOL save_queries;
  /* SSL */
  unsigned int sslverify;
} DataSource;

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

extern const SQLWCHAR W_DRIVER_PARAM[];
extern const SQLWCHAR W_DRIVER_NAME[];
extern const SQLWCHAR W_INVALID_ATTR_STR[];

#ifdef __cplusplus
}
#endif

#endif /* _INSTALLER_H */

