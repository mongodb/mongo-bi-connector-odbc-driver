/*
  Copyright (c) 2001, 2012, Oracle and/or its affiliates. All rights reserved.

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

/***************************************************************************
 * MYUTIL.H								   *
 *									   *
 * @description: Prototype definations needed by the driver		   *
 *									   *
 * @author     : MySQL AB(monty@mysql.com, venu@mysql.com)		   *
 * @date       : 2001-Sep-22						   *
 * @product    : myodbc3						   *
 *									   *
 ****************************************************************************/
#ifndef __MYUTIL_H__
#define __MYUTIL_H__

/*
  Utility macros
*/

#define if_dynamic_cursor(st) ((st)->stmt_options.cursor_type == SQL_CURSOR_DYNAMIC)
#define if_forward_cache(st) ((st)->stmt_options.cursor_type == SQL_CURSOR_FORWARD_ONLY && \
			     (st)->dbc->ds->dont_cache_result)
#define is_connected(dbc)    ((dbc)->mysql.net.vio)
#define trans_supported(db) ((db)->mysql.server_capabilities & CLIENT_TRANSACTIONS)
#define autocommit_on(db) ((db)->mysql.server_status & SERVER_STATUS_AUTOCOMMIT)
#define is_no_backslashes_escape_mode(db) ((db)->mysql.server_status & SERVER_STATUS_NO_BACKSLASH_ESCAPES)
#define reset_ptr(x) {if (x) x= 0;}
#define digit(A) ((int) (A - '0'))

#define MYLOG_QUERY(A,B) {if ((A)->dbc->ds->save_queries) \
               query_print((A)->dbc->query_log,(char*) B);}

#define MYLOG_DBC_QUERY(A,B) {if((A)->ds->save_queries) \
               query_print((A)->query_log,(char*) B);}

/* A few character sets we care about. */
#define ASCII_CHARSET_NUMBER  11
#define BINARY_CHARSET_NUMBER 63
#define UTF8_CHARSET_NUMBER   33

/* truncation types in SQL_NUMERIC_STRUCT conversions */
#define SQLNUM_TRUNC_FRAC 1
#define SQLNUM_TRUNC_WHOLE 2

/* Conversion to SQL_TIMESTAMP_STRUCT errors(str_to_ts) */
#define SQLTS_NULL_DATE -1
#define SQLTS_BAD_DATE -2

/* Sizes of buffer for converion of 4 and 8 bytes integer values*/
#define MAX32_BUFF_SIZE 11
#define MAX64_BUFF_SIZE 21

/* Wrappers to hide differences in client library versions. */
#if MYSQL_VERSION_ID >= 40100
# define my_int2str(val, dst, radix, upcase) \
    int2str((val), (dst), (radix), (upcase))
#else
# define my_int2str(val, dst, radix, upcase) \
    int2str((val), (dst), (radix))
#endif

#if MYSQL_VERSION_ID >= 50100
typedef unsigned char * DYNAMIC_ELEMENT;
#else
typedef char * DYNAMIC_ELEMENT;
#endif

#if MYSQL_VERSION_ID >= 50100
/* Same us MYODBC_FIELD_STRING(name, NAME_LEN, flags) */
# define MYODBC_FIELD_NAME(name, flags) \
  {(name), (name), NullS, NullS, NullS, NullS, NullS, NAME_LEN, 0, 0, 0, 0, 0, 0, \
    0, 0, (flags), 0, UTF8_CHARSET_NUMBER, MYSQL_TYPE_VAR_STRING, NULL}
# define MYODBC_FIELD_STRING(name, len, flags) \
  {(name), (name), NullS, NullS, NullS, NullS, NullS, (len*SYSTEM_CHARSET_MBMAXLEN), 0, 0, 0, 0, 0, 0, \
    0, 0, (flags), 0, UTF8_CHARSET_NUMBER, MYSQL_TYPE_VAR_STRING, NULL}
# define MYODBC_FIELD_SHORT(name, flags) \
  {(name), (name), NullS, NullS, NullS, NullS, NullS, 5, 5, 0, 0, 0, 0, 0, 0, \
    0, (flags), 0, 0, MYSQL_TYPE_SHORT, NULL}
# define MYODBC_FIELD_LONG(name, flags) \
  {(name), (name), NullS, NullS, NullS, NullS, NullS, 11, 11, 0, 0, 0, 0, 0, \
    0, 0, (flags), 0, 0, MYSQL_TYPE_LONG, NULL}
# define MYODBC_FIELD_LONGLONG(name, flags) \
  {(name), (name), NullS, NullS, NullS, NullS, NullS, 20, 20, 0, 0, 0, 0, 0, \
    0, 0, (flags), 0, 0, MYSQL_TYPE_LONGLONG, NULL}
#elif MYSQL_VERSION_ID >= 40100
# define MYODBC_FIELD_NAME(name, flags) \
{(name), (name), NullS, NullS, NullS, NullS, NullS, NAME_LEN*3, 0, 0, 0, 0, 0, 0, \
  0, 0, (flags), 0, UTF8_CHARSET_NUMBER, MYSQL_TYPE_VAR_STRING, NULL}
/* we use 3 here as SYSTEM_CHARSET_MBMAXLEN is not defined in v5.0 mysql_com.h */
# define MYODBC_FIELD_STRING(name, len, flags) \
  {(name), (name), NullS, NullS, NullS, NullS, NullS, (len) * 3, 0, 0, 0, 0, \
    0, 0, 0, 0, (flags), 0, UTF8_CHARSET_NUMBER, MYSQL_TYPE_VAR_STRING}
# define MYODBC_FIELD_SHORT(name, flags) \
  {(name), (name), NullS, NullS, NullS, NullS, NullS, 5, 5, 0, 0, 0, 0, 0, 0, \
    0, (flags), 0, 0, MYSQL_TYPE_SHORT}
# define MYODBC_FIELD_LONG(name, flags) \
  {(name), (name), NullS, NullS, NullS, NullS, NullS, 11, 11, 0, 0, 0, 0, 0, \
    0, 0, (flags), 0, 0, MYSQL_TYPE_LONG}
# define MYODBC_FIELD_LONGLONG(name, flags) \
  {(name), (name), NullS, NullS, NullS, NullS, NullS, 20, 20, 0, 0, 0, 0, 0, \
    0, 0, (flags), 0, 0, MYSQL_TYPE_LONGLONG }
#endif

/*
  Utility function prototypes that share among files
*/

SQLRETURN my_SQLPrepare(SQLHSTMT hstmt, SQLCHAR *szSqlStr, SQLINTEGER cbSqlStr,
                        my_bool dupe);
SQLRETURN my_SQLExecute(STMT FAR* stmt);
SQLRETURN SQL_API my_SQLFreeStmt(SQLHSTMT hstmt,SQLUSMALLINT fOption);
SQLRETURN SQL_API my_SQLFreeStmtExtended(SQLHSTMT hstmt,
                    SQLUSMALLINT fOption, uint clearAllResults);
SQLRETURN SQL_API my_SQLAllocStmt(SQLHDBC hdbc,SQLHSTMT FAR *phstmt);
SQLRETURN do_query(STMT FAR *stmt,char *query, SQLULEN query_length);
SQLRETURN insert_params(STMT FAR *stmt, SQLULEN row, char **finalquery,
                        SQLULEN *length);
SQLRETURN odbc_stmt(DBC FAR *dbc, const char *query);
void mysql_link_fields(STMT *stmt,MYSQL_FIELD *fields,uint field_count);
void fix_row_lengths(STMT *stmt, const long* fix_rules, uint row, uint field_count);
void fix_result_types(STMT *stmt);
char *fix_str(char *to,const char *from,int length);
char *dupp_str(char *from,int length);
SQLRETURN my_pos_delete(STMT FAR *stmt,STMT FAR *stmtParam,
			SQLUSMALLINT irow,DYNAMIC_STRING *dynStr);
SQLRETURN my_pos_update(STMT FAR *stmt,STMT FAR *stmtParam,
			SQLUSMALLINT irow,DYNAMIC_STRING *dynStr);
char *check_if_positioned_cursor_exists(STMT FAR *stmt, STMT FAR **stmtNew);
SQLRETURN insert_param(STMT *stmt, uchar *to, DESC *apd,
                       DESCREC *aprec, DESCREC *iprec, SQLULEN row);
char *add_to_buffer(NET *net,char *to,const char *from,ulong length);

void reset_getdata_position(STMT *stmt);

SQLRETURN set_sql_select_limit(DBC *dbc, SQLULEN new_value);

uint32
copy_and_convert(char *to, uint32 to_length, CHARSET_INFO *to_cs,
                 const char *from, uint32 from_length, CHARSET_INFO *from_cs,
                 uint32 *used_bytes, uint32 *used_chars, uint *errors);
SQLRETURN
copy_ansi_result(STMT *stmt,
                 SQLCHAR *result, SQLLEN result_bytes, SQLLEN *used_bytes,
                 MYSQL_FIELD *field, char *src, unsigned long src_bytes);
SQLRETURN
copy_binary_result(STMT *stmt,
                   SQLCHAR *result, SQLLEN result_bytes, SQLLEN *used_bytes,
                   MYSQL_FIELD *field, char *src, unsigned long src_bytes);
SQLRETURN copy_binhex_result(STMT *stmt,
			     SQLCHAR FAR *rgbValue, SQLINTEGER cbValueMax,
			     SQLLEN *pcbValue, MYSQL_FIELD *field, char *src,
			     ulong src_length);
SQLRETURN copy_wchar_result(STMT *stmt,
                            SQLWCHAR *rgbValue, SQLINTEGER cbValueMax,
                            SQLLEN *pcbValue, MYSQL_FIELD *field, char *src,
                            long src_length);

SQLRETURN set_dbc_error(DBC FAR *dbc, char *state,const char *message,uint errcode);
#define set_stmt_error myodbc_set_stmt_error
SQLRETURN set_stmt_error(STMT *stmt, const char *state, const char *message, uint errcode);
SQLRETURN set_desc_error(DESC *desc, char *state,
                         const char *message, uint errcode);
SQLRETURN handle_connection_error(STMT *stmt);
my_bool is_connection_lost(uint errcode);
void set_mem_error(MYSQL *mysql);
void translate_error(char *save_state, myodbc_errid errid, uint mysql_err);

SQLSMALLINT get_sql_data_type(STMT *stmt, MYSQL_FIELD *field, char *buff);
SQLULEN get_column_size(STMT *stmt, MYSQL_FIELD *field);
SQLULEN fill_column_size_buff(char *buff, STMT *stmt, MYSQL_FIELD *field);
SQLSMALLINT get_decimal_digits(STMT *stmt, MYSQL_FIELD *field);
SQLLEN get_transfer_octet_length(STMT *stmt, MYSQL_FIELD *field);
SQLLEN fill_transfer_oct_len_buff(char *buff, STMT *stmt, MYSQL_FIELD *field);
SQLLEN get_display_size(STMT *stmt, MYSQL_FIELD *field);
SQLLEN fill_display_size_buff(char *buff, STMT *stmt, MYSQL_FIELD *field);
SQLSMALLINT get_dticode_from_concise_type(SQLSMALLINT concise_type);
SQLSMALLINT get_concise_type_from_datetime_code(SQLSMALLINT dticode);
SQLSMALLINT get_concise_type_from_interval_code(SQLSMALLINT dticode);
SQLSMALLINT get_type_from_concise_type(SQLSMALLINT concise_type);

#define is_char_sql_type(type) \
  ((type) == SQL_CHAR || (type) == SQL_VARCHAR || (type) == SQL_LONGVARCHAR)
#define is_wchar_sql_type(type) \
  ((type) == SQL_WCHAR || (type) == SQL_WVARCHAR || (type) == SQL_WLONGVARCHAR)
#define is_binary_sql_type(type) \
  ((type) == SQL_BINARY || (type) == SQL_VARBINARY || \
   (type) == SQL_LONGVARBINARY)

#define is_numeric_mysql_type(field) \
  ((field)->type <= MYSQL_TYPE_NULL || (field)->type == MYSQL_TYPE_LONGLONG || \
   (field)->type == MYSQL_TYPE_INT24 || \
   ((field)->type == MYSQL_TYPE_BIT && (field)->length == 1) || \
   (field)->type == MYSQL_TYPE_NEWDECIMAL)

SQLRETURN SQL_API my_SQLBindParameter(SQLHSTMT hstmt,SQLUSMALLINT ipar,
				      SQLSMALLINT fParamType,
				      SQLSMALLINT fCType, SQLSMALLINT fSqlType,
				      SQLULEN cbColDef,
				      SQLSMALLINT ibScale,
				      SQLPOINTER  rgbValue,
				      SQLLEN cbValueMax,
				      SQLLEN *pcbValue);
SQLRETURN SQL_API my_SQLExtendedFetch(SQLHSTMT hstmt, SQLUSMALLINT fFetchType,
				      SQLLEN irow, SQLULEN *pcrow,
				      SQLUSMALLINT FAR *rgfRowStatus, my_bool upd_status);
SQLRETURN SQL_API my_SQLSetPos(SQLHSTMT hstmt, SQLSETPOSIROW irow,
                               SQLUSMALLINT fOption, SQLUSMALLINT fLock);
SQLRETURN copy_stmt_error(STMT FAR *src, STMT FAR *dst);
int unireg_to_c_datatype(MYSQL_FIELD *field);
int default_c_type(int sql_data_type);
ulong bind_length(int sql_data_type,ulong length);
my_bool str_to_date(SQL_DATE_STRUCT *rgbValue, const char *str,
                    uint length, int zeroToMin);
int str_to_ts(SQL_TIMESTAMP_STRUCT *ts, const char *str, int len, int zeroToMin,
              BOOL dont_use_set_locale);
my_bool str_to_time_st(SQL_TIME_STRUCT *ts, const char *str);
ulong str_to_time_as_long(const char *str,uint length);
void init_getfunctions(void);
void myodbc_init(void);
void myodbc_ov_init(SQLINTEGER odbc_version);
void myodbc_sqlstate2_init(void);
void myodbc_sqlstate3_init(void);
int check_if_server_is_alive(DBC FAR *dbc);
my_bool dynstr_append_quoted_name(DYNAMIC_STRING *str, const char *name);
SQLRETURN set_handle_error(SQLSMALLINT HandleType, SQLHANDLE handle,
			   myodbc_errid errid, const char *errtext, SQLINTEGER errcode);
SQLRETURN set_error(STMT *stmt,myodbc_errid errid, const char *errtext,
		    SQLINTEGER errcode);
SQLRETURN set_conn_error(DBC *dbc,myodbc_errid errid, const char *errtext,
			 SQLINTEGER errcode);
SQLRETURN set_env_error(ENV * env,myodbc_errid errid, const char *errtext,
			SQLINTEGER errcode);
SQLRETURN copy_str_data(SQLSMALLINT HandleType, SQLHANDLE Handle,
			SQLCHAR FAR *rgbValue, SQLSMALLINT cbValueMax,
			SQLSMALLINT FAR *pcbValue,char FAR *src);
SQLRETURN SQL_API my_SQLAllocEnv(SQLHENV FAR * phenv);
SQLRETURN SQL_API my_SQLAllocConnect(SQLHENV henv, SQLHDBC FAR *phdbc);
SQLRETURN SQL_API my_SQLFreeConnect(SQLHDBC hdbc);
SQLRETURN SQL_API my_SQLFreeEnv(SQLHENV henv);
char *extend_buffer(NET *net,char *to,ulong length);
void myodbc_end();
my_bool set_dynamic_result(STMT FAR *stmt);
void set_current_cursor_data(STMT FAR *stmt,SQLUINTEGER irow);
my_bool is_minimum_version(const char *server_version,const char *version);
int myodbc_strcasecmp(const char *s, const char *t);
int myodbc_casecmp(const char *s, const char *t, uint len);
my_bool reget_current_catalog(DBC FAR *dbc);

ulong myodbc_escape_string(MYSQL *mysql, char *to, ulong to_length,
                           const char *from, ulong length, int escape_id);

DESCREC *desc_get_rec(DESC *desc, int recnum, my_bool expand);

DESC *desc_alloc(STMT *stmt, SQLSMALLINT alloc_type,
                 desc_ref_type ref_type, desc_desc_type desc_type);
void desc_free_paramdata(DESC *desc);
void desc_free(DESC *desc);
void desc_rec_init_apd(DESCREC *rec);
void desc_rec_init_ipd(DESCREC *rec);
void desc_remove_stmt(DESC *desc, STMT *stmt);
int desc_find_dae_rec(DESC *desc);
SQLRETURN
stmt_SQLSetDescField(STMT *stmt, DESC *desc, SQLSMALLINT recnum,
                     SQLSMALLINT fldid, SQLPOINTER val, SQLINTEGER buflen);
SQLRETURN
stmt_SQLGetDescField(STMT *stmt, DESC *desc, SQLSMALLINT recnum,
                     SQLSMALLINT fldid, SQLPOINTER valptr,
                     SQLINTEGER buflen, SQLINTEGER *strlen);
SQLRETURN stmt_SQLCopyDesc(STMT *stmt, DESC *src, DESC *dest);

void sqlnum_from_str(const char *numstr, SQL_NUMERIC_STRUCT *sqlnum,
                     int *overflow_ptr);
void sqlnum_to_str(SQL_NUMERIC_STRUCT *sqlnum, SQLCHAR *numstr,
                   SQLCHAR **numbegin, SQLCHAR reqprec, SQLSCHAR reqscale,
                   int *truncptr);
void *ptr_offset_adjust(void *ptr, SQLULEN *bind_offset,
                        SQLINTEGER bind_type, SQLINTEGER default_size,
                        SQLULEN row);

/* Functions used when debugging */
void query_print(FILE *log_file,char *query);
FILE *init_query_log(void);
void end_query_log(FILE *query_log);

LIST *list_delete_forward(LIST *elem);

enum enum_field_types map_sql2mysql_type(SQLSMALLINT sql_type);

/* proc_* functions - used to parse prcedures headers in SQLProcedureColumns */
char *proc_param_tokenize(char *str, int *params_num);
SQLCHAR *proc_get_param_type(SQLCHAR *proc, int len, SQLSMALLINT *ptype);
SQLCHAR* proc_get_param_name(SQLCHAR *proc, int len, SQLCHAR *cname);
SQLCHAR* proc_get_param_dbtype(SQLCHAR *proc, int len, SQLCHAR *ptype);
SQLUINTEGER proc_get_param_size(SQLCHAR *ptype, int len, int sql_type_index,
                                SQLSMALLINT *dec);
SQLLEN proc_get_param_octet_len(STMT *stmt, int sql_type_index,
                                SQLULEN col_size, SQLSMALLINT decimal_digits,
                                unsigned int flags, char * str_buff);
SQLLEN proc_get_param_col_len(STMT *stmt, int sql_type_index, SQLULEN col_size, 
                              SQLSMALLINT decimal_digits, unsigned int flags,
                              char * str_buff);
int proc_get_param_sql_type_index(SQLCHAR *ptype, int len);
SQLTypeMap *proc_get_param_map_by_index(int index);
char *proc_param_next_token(char *str, char *str_end);

void set_row_count(STMT * stmt, my_ulonglong rows);
const char *get_fractional_part(const char * str, int len,
                                BOOL dont_use_set_locale,
                                SQLUINTEGER * fraction);
/* Convert MySQL timestamp to full ANSI timestamp format. */
char * complete_timestamp(const char * value, ulong length, char buff[21]);
long double strtold(const char *nptr, char **endptr);
char * extend_buffer(NET *net, char *to, ulong length);
char * add_to_buffer(NET *net,char *to,const char *from,ulong length);
MY_LIMIT_CLAUSE find_position4limit(CHARSET_INFO* cs, char *query,
                                    char * query_end);
BOOL myodbc_isspace(CHARSET_INFO* cs, const char * begin, const char *end);
BOOL got_out_parameters(stmt);

/* handle.c*/
BOOL          allocate_param_bind(DYNAMIC_ARRAY **param_bind, uint elements);
/*results.c*/
long long     binary2numeric(long long *dst, char *src, uint srcLen);
void          fill_ird_data_lengths(DESC *ird, ulong *lengths, uint fields);

/* Functions to work with prepared and regular statements  */

#ifdef SERVER_PS_OUT_PARAMS
# define IS_PS_OUT_PARAMS(_stmt) ((_stmt)->dbc->mysql.server_status & SERVER_PS_OUT_PARAMS)
#else
/* In case if driver is built against old libmysl. In fact is not quite
   correct */
# define IS_PS_OUT_PARAMS(_stmt) (ssps_used(_stmt) && is_call_procedure(&_stmt->query) && !mysql_more_results(&(_stmt)->dbc->mysql))
#endif

/* my_stmt.c */
BOOL              ssps_used           (STMT *stmt);
BOOL              returned_result     (STMT *stmt);
my_bool           free_current_result (STMT *stmt);
MYSQL_RES *       get_result_metadata (STMT *stmt, BOOL force_use);
int               bind_result         (STMT *stmt);
int               get_result          (STMT *stmt);
unsigned int      field_count         (STMT *stmt);
my_ulonglong      affected_rows       (STMT *stmt);
my_ulonglong      update_affected_rows(STMT *stmt);
my_ulonglong      num_rows            (STMT *stmt);
MYSQL_ROW         fetch_row           (STMT *stmt);
unsigned long*    fetch_lengths       (STMT *stmt);
MYSQL_ROW_OFFSET  row_seek            (STMT *stmt, MYSQL_ROW_OFFSET offset);
void              data_seek           (STMT *stmt, my_ulonglong offset);
MYSQL_ROW_OFFSET  row_tell            (STMT *stmt);
int               next_result         (STMT *stmt);

int           get_int     (STMT *stmt, ulong column_number, char *value,
                          ulong length);
long long     get_int64   (STMT *stmt, ulong column_number, char *value,
                          ulong length);
char *        get_string  (STMT *stmt, ulong column_number, char *value,
                          ulong *length, char * buffer);
long double   get_double  (STMT *stmt, ulong column_number, char *value,
                          ulong length);
BOOL          is_null     (STMT *stmt, ulong column_number, char *value);
SQLRETURN     prepare     (STMT *stmt, char * query, SQLINTEGER query_length);

/* scroller-related functions */
void          scroller_reset      (STMT *stmt);
unsigned int  calc_prefetch_number(unsigned int selected, SQLULEN app_fetchs,
                                   SQLULEN max_rows);
BOOL          scroller_exists     (STMT * stmt);
void          scroller_create     (STMT * stmt, char *query, SQLULEN len);

unsigned long long  scroller_move (STMT * stmt);

SQLRETURN     scroller_prefetch   (STMT * stmt);
BOOL          scrollable          (STMT * stmt, char * query, char * query_end);

/* my_prepared_stmt.c */
void        ssps_init             (STMT *stmt);
BOOL        ssps_get_out_params   (STMT *stmt);
int         ssps_get_result       (STMT *stmt);
void        ssps_close            (STMT *stmt);
int         ssps_bind_result      (STMT *stmt);
void        free_result_bind      (STMT *stmt);
BOOL        ssps_0buffers_truncated_only(STMT *stmt);
long long   ssps_get_int64        (STMT *stmt, ulong column_number, char *value,
                                  ulong length);
long double ssps_get_double       (STMT *stmt, ulong column_number, char *value,
                                  ulong length);
char *      ssps_get_string       (STMT *stmt, ulong column_number, char *value,
                                  ulong *length, char * buffer);

#ifdef __WIN__
#define cmp_database(A,B) myodbc_strcasecmp((const char *)(A),(const char *)(B))
#else
#define cmp_database(A,B) strcmp((A),(B))
#endif

/*
  Check if an octet_length_ptr is a data-at-exec field.
  WARNING: This macro evaluates the argument multiple times.
*/
#define IS_DATA_AT_EXEC(X) ((X) && \
                            (*(X) == SQL_DATA_AT_EXEC || \
                             *(X) <= SQL_LEN_DATA_AT_EXEC_OFFSET))

#endif /* __MYUTIL_H__ */
