/*
  Copyright (c) 2001, 2013, Oracle and/or its affiliates. All rights reserved.

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
  @file driver.h
  @brief Definitions needed by the driver
*/

#ifndef __DRIVER_H__
#define __DRIVER_H__

#include "../MYODBC_MYSQL.h"
#include "../MYODBC_CONF.h"
#include "../MYODBC_ODBC.h"
#include "installer.h"

#ifdef APSTUDIO_READONLY_SYMBOLS
#define WIN32	/* Hack for rc files */
#endif

/* Needed for offsetof() CPP macro */
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif
#ifdef RC_INVOKED
#define stdin
#endif

/* Misc definitions for AIX .. */
#ifndef crid_t
 typedef int crid_t;
#endif
#ifndef class_id_t
 typedef unsigned int class_id_t;
#endif

#ifdef __cplusplus
}
#endif

#include "error.h"
#include "parse.h"

#if defined(_WIN32) || defined(WIN32)
# define INTFUNC  __stdcall
# define EXPFUNC  __stdcall
# if !defined(HAVE_LOCALTIME_R)
#  define HAVE_LOCALTIME_R 1
# endif
#else
# define INTFUNC PASCAL
# define EXPFUNC __export CALLBACK
/* Simple macros to make dl look like the Windows library funcs. */
# define HMODULE void*
# define LoadLibrary(library) dlopen((library), RTLD_GLOBAL | RTLD_LAZY)
# define GetProcAddress(module, proc) dlsym((module), (proc))
# define FreeLibrary(module) dlclose((module))
#endif

#define ODBC_DRIVER	  "ODBC "MYODBC_STRSERIES" Driver"
#define DRIVER_NAME	  "MySQL ODBC "MYODBC_STRSERIES" Driver"
#define DRIVER_NONDSN_TAG "DRIVER={MySQL ODBC "MYODBC_STRSERIES" Driver}"

#if defined(__APPLE__)

#define DRIVER_QUERY_LOGFILE "/tmp/myodbc.sql"

#elif defined(_UNIX_)

#define DRIVER_QUERY_LOGFILE "/tmp/myodbc.sql"

#else

#define DRIVER_QUERY_LOGFILE "myodbc.sql"

#endif

/*
   Internal driver definitions
*/
#define MYSQL_RESET_BUFFERS 1000  /* param to SQLFreeStmt */
#define MYSQL_RESET 1001	  /* param to SQLFreeStmt */
#define MYSQL_3_21_PROTOCOL 10	  /* OLD protocol */
#define CHECK_IF_ALIVE	    1800  /* Seconds between queries for ping */

#define MYSQL_MAX_CURSOR_LEN 18   /* Max cursor name length */
#define MYSQL_STMT_LEN 1024	  /* Max statement length */
#define MYSQL_STRING_LEN 1024	  /* Max string length */
#define MYSQL_MAX_SEARCH_STRING_LEN NAME_LEN+10 /* Max search string length */
/* Max Primary keys in a cursor * WHERE clause */
#define MY_MAX_PK_PARTS 32

#if MYSQL_VERSION_ID >= 50500
# define x_free(A) { void *tmp= (A); if (tmp) my_free((char *) tmp); }
# ifndef NEAR
#  define NEAR 
# endif
#else
# define x_free(A) { void *tmp= (A); if (tmp) my_free((char *) tmp,MYF(MY_WME+MY_FAE)); }
#endif

/* We don't make any assumption about what the default may be. */
#ifndef DEFAULT_TXN_ISOLATION
# define DEFAULT_TXN_ISOLATION 0
#endif

/* For compatibility with old mysql clients - defining error */
#ifndef ER_MUST_CHANGE_PASSWORD_LOGIN
# define ER_MUST_CHANGE_PASSWORD_LOGIN 1820
#endif

#ifndef CR_AUTH_PLUGIN_CANNOT_LOAD_ERROR
# define CR_AUTH_PLUGIN_CANNOT_LOAD_ERROR 2059
#endif

/* Connection flags to validate after the connection*/
#define CHECK_AUTOCOMMIT_ON	1  /* AUTOCOMMIT_ON */
#define CHECK_AUTOCOMMIT_OFF	2  /* AUTOCOMMIT_OFF */

/* implementation or application descriptor? */
typedef enum { DESC_IMP, DESC_APP } desc_ref_type;

/* parameter or row descriptor? */
typedef enum { DESC_PARAM, DESC_ROW, DESC_UNKNOWN } desc_desc_type;

/* header or record field? (location in descriptor) */
typedef enum { DESC_HDR, DESC_REC } fld_loc;

/* permissions - header, and base for record */
#define P_RI 1 /* imp */
#define P_WI 2
#define P_RA 4 /* app */
#define P_WA 8

/* macros to encode the constants above */
#define P_ROW(P) (P)
#define P_PAR(P) ((P) << 4)

#define PR_RIR  P_ROW(P_RI)
#define PR_WIR (P_ROW(P_WI) | PR_RIR)
#define PR_RAR  P_ROW(P_RA)
#define PR_WAR (P_ROW(P_WA) | PR_RAR)
#define PR_RIP  P_PAR(P_RI)
#define PR_WIP (P_PAR(P_WI) | PR_RIP)
#define PR_RAP  P_PAR(P_RI)
#define PR_WAP (P_PAR(P_WA) | PR_RAP)

/* macros to test type */
#define IS_APD(d) ((d)->desc_type == DESC_PARAM && (d)->ref_type == DESC_APP)
#define IS_IPD(d) ((d)->desc_type == DESC_PARAM && (d)->ref_type == DESC_IMP)
#define IS_ARD(d) ((d)->desc_type == DESC_ROW && (d)->ref_type == DESC_APP)
#define IS_IRD(d) ((d)->desc_type == DESC_ROW && (d)->ref_type == DESC_IMP)

/* additional field types needed, but not defined in ODBC */
#define SQL_IS_ULEN (-9)
#define SQL_IS_LEN (-10)

/* check if ARD record is a bound column */
#define ARD_IS_BOUND(d) ((d)->data_ptr || (d)->octet_length_ptr)

/* get the dbc from a descriptor */
#define DESC_GET_DBC(X) (((X)->alloc_type == SQL_DESC_ALLOC_USER) ? \
                         (X)->exp.dbc : (X)->stmt->dbc)

/* data-at-exec type */
#define DAE_NORMAL 1 /* normal SQLExecute() */
#define DAE_SETPOS_INSERT 2 /* SQLSetPos() insert */
#define DAE_SETPOS_UPDATE 3 /* SQLSetPos() update */
/* data-at-exec handling done for current SQLSetPos() call */
#define DAE_SETPOS_DONE 10

typedef struct {
  int perms;
  SQLSMALLINT data_type; /* SQL_IS_SMALLINT, etc */
  fld_loc loc;
  size_t offset; /* offset of field in struct */
} desc_field;

/* descriptor */
struct tagSTMT;
struct tagDBC;
typedef struct {
  /* header fields */
  SQLSMALLINT   alloc_type;
  SQLULEN       array_size;
  SQLUSMALLINT *array_status_ptr;
  /* NOTE: This field is defined as SQLINTEGER* in the descriptor
   * documentation, but corresponds to SQL_ATTR_ROW_BIND_OFFSET_PTR or
   * SQL_ATTR_PARAM_BIND_OFFSET_PTR when set via SQLSetStmtAttr(). The
   * 64-bit ODBC addendum says that when set via SQLSetStmtAttr(), this
   * is now a 64-bit value. These two are conflicting, so we opt for
   * the 64-bit value.
   */
  SQLULEN      *bind_offset_ptr;
  SQLINTEGER    bind_type;
  SQLLEN        count;
  /* Everywhere(http://msdn.microsoft.com/en-us/library/ms713560(VS.85).aspx
     http://msdn.microsoft.com/en-us/library/ms712631(VS.85).aspx) I found 
     it's referred as SQLULEN* */
  SQLULEN      *rows_processed_ptr;

  /* internal fields */
  desc_desc_type  desc_type;
  desc_ref_type   ref_type;
  DYNAMIC_ARRAY   records;
  MYERROR         error;
  struct tagSTMT *stmt;

  /* SQL_DESC_ALLOC_USER-specific */
  struct {
    /*
      We keep a list of all statements we've been set on because we need
      to put the implicit descriptor back if this one is freed.
    */
    LIST *stmts;
    /* connection we were allocated on */
    struct tagDBC *dbc;
  } exp;
} DESC;

/* descriptor record */
typedef struct {
  /* ODBC spec fields */
  SQLINTEGER  auto_unique_value; /* row only */
  SQLCHAR *   base_column_name; /* row only */
  SQLCHAR *   base_table_name; /* row only */
  SQLINTEGER  case_sensitive; /* row only */
  SQLCHAR *   catalog_name; /* row only */
  SQLSMALLINT concise_type;
  SQLPOINTER  data_ptr;
  SQLSMALLINT datetime_interval_code;
  SQLINTEGER  datetime_interval_precision;
  SQLLEN      display_size; /* row only */
  SQLSMALLINT fixed_prec_scale;
  SQLLEN  *   indicator_ptr;
  SQLCHAR *   label; /* row only */
  SQLULEN     length;
  SQLCHAR *   literal_prefix; /* row only */
  SQLCHAR *   literal_suffix; /* row only */
  SQLCHAR *   local_type_name;
  SQLCHAR *   name;
  SQLSMALLINT nullable;
  SQLINTEGER  num_prec_radix;
  SQLLEN      octet_length;
  SQLLEN     *octet_length_ptr;
  SQLSMALLINT parameter_type; /* param only */
  SQLSMALLINT precision;
  SQLSMALLINT rowver;
  SQLSMALLINT scale;
  SQLCHAR *   schema_name; /* row only */
  SQLSMALLINT searchable; /* row only */
  SQLCHAR *   table_name; /* row only */
  SQLSMALLINT type;
  SQLCHAR *   type_name;
  SQLSMALLINT unnamed;
  SQLSMALLINT is_unsigned;
  SQLSMALLINT updatable; /* row only */

  /* internal descriptor fields */

  /* parameter-specific */
  struct {
    /* value, value_length, and alloced are used for data
     * at exec parameters */
    char *value;
    SQLINTEGER value_length;
    /*
      this parameter is data-at-exec. this is needed as cursor updates
      in ADO change the bind_offset_ptr between SQLSetPos() and the
      final call to SQLParamData() which makes it impossible for us to
      know any longer it was a data-at-exec param.
    */
    char is_dae;
    my_bool alloced;
    /* Whether this parameter has been bound by the application
     * (if not, was created by dummy execution) */
    my_bool real_param_done;
  } par;

  /* row-specific */
  struct {
    MYSQL_FIELD * field; /* Used *only* by IRD */
    ulong datalen; /* actual length, maintained for *each* row */
    /* TODO ugly, but easiest way to handle memory */
    SQLCHAR type_name[40];
  } row;
} DESCREC;


/* Statement attributes */

typedef struct stmt_options
{
  SQLUINTEGER      cursor_type;
  SQLUINTEGER      simulateCursor;
  SQLULEN          max_length, max_rows;
  SQLUSMALLINT    *rowStatusPtr_ex; /* set by SQLExtendedFetch */
  my_bool      retrieve_data;
} STMT_OPTIONS;


/* Environment handler */

typedef struct	tagENV
{
  SQLINTEGER   odbc_ver;
  LIST	       *connections;
  MYERROR      error;

} ENV;


/* Connection handler */

typedef struct tagDBC
{
  ENV           *env;
  MYSQL         mysql;
  LIST          *statements;
  LIST          *exp_desc; /* explicit descriptors */
  LIST          list;
  STMT_OPTIONS  stmt_options;
  MYERROR       error;
  FILE          *query_log;
  char          st_error_prefix[255];
  char          *database;
  SQLUINTEGER   login_timeout;
  time_t        last_query_time;
  int           txn_isolation;
  uint          port;
  uint          cursor_count;
  uint          commit_flag;
#ifdef THREAD
  pthread_mutex_t lock;
#endif

  my_bool       unicode;            /* Whether SQL*ConnectW was used */
  CHARSET_INFO  *ansi_charset_info, /* 'ANSI' charset (SQL_C_CHAR) */
                *cxn_charset_info;  /* Connection charset ('ANSI' or utf-8) */
  MY_SYNTAX_MARKERS *syntax;
  DataSource    *ds;                /* data source used to connect (parsed or stored) */
  SQLULEN       sql_select_limit;   /* value of the sql_select_limit currently set for a session
                                       (SQLULEN)(-1) if wasn't set */
} DBC;


/* Statement states */

enum MY_STATE { ST_UNKNOWN, ST_PREPARED, ST_PRE_EXECUTED, ST_EXECUTED };
enum MY_DUMMY_STATE { ST_DUMMY_UNKNOWN, ST_DUMMY_PREPARED, ST_DUMMY_EXECUTED };


typedef struct limit
{
  unsigned long long  offset;
  unsigned int        row_count;
  char                *begin, *end;

} MY_LIMIT_CLAUSE;

typedef struct limit_scroller
{
   char               *query, *offset_pos;
   unsigned int       row_count;
   unsigned long long next_offset, total_rows, query_len;

} MY_LIMIT_SCROLLER;

/* Statement primary key handler for cursors */
typedef struct pk_column
{
  char	      name[NAME_LEN+1];
  my_bool     bind_done;
} MY_PK_COLUMN;


/* Statement cursor handler */
typedef struct cursor
{
  char        *name;
  uint	       pk_count;
  my_bool      pk_validated;
  MY_PK_COLUMN pkcol[MY_MAX_PK_PARTS];
} MYCURSOR;


/* Main statement handler */

typedef struct tagSTMT
{
  DBC FAR           *dbc;
  MYSQL_RES         *result;
  my_bool           fake_result;
  MYSQL_ROW	        array,result_array,current_values;
  MYSQL_ROW	        (*fix_fields)(struct tagSTMT FAR* stmt,MYSQL_ROW row);
  MYSQL_FIELD	      *fields;
  MYSQL_ROW_OFFSET  end_of_set;

  LIST              list;
  MYCURSOR          cursor;
  MYERROR           error;
  STMT_OPTIONS      stmt_options;
  char              *table_name;

  MY_PARSED_QUERY	query, orig_query;
  DYNAMIC_ARRAY     *param_bind;

  unsigned long     *lengths; /* used to set lengths if we shuffle field values
                         of the resultset of auxiliary query or if we fix_fields. */
  /*
    We save a copy of the original query before we modify it for 'WHERE
    CURRENT OF' cursor handling.
  */
  my_ulonglong      affected_rows;
  long              current_row;
  long              cursor_row;
  char              dae_type; /* data-at-exec type */
  struct {
    uint column;      /* Which column is being used with SQLGetData() */
    char *source;     /* Our current position in the source. */
    uchar latest[7];  /* Latest character to be converted. */
    int latest_bytes; /* Bytes of data in latest. */
    int latest_used;  /* Bytes of latest that have been used. */
    ulong src_offset; /* @todo remove */
    ulong dst_bytes;  /* Length of data once it is all converted (in chars). */
    ulong dst_offset; /* Current offset into dest. (ulong)~0L when not set. */
  } getdata;

  uint		*order, order_count, param_count, current_param, rows_found_in_set;

  enum MY_STATE state;
  enum MY_DUMMY_STATE dummy_state;

  DESC *ard;
  DESC *ird;
  DESC *apd;
  DESC *ipd;
  /* implicit descriptors */
  DESC *imp_ard;
  DESC *imp_apd;
  /* APD for data-at-exec on SQLSetPos() */
  DESC *setpos_apd;
  SQLSETPOSIROW setpos_row;
  SQLUSMALLINT setpos_lock;

  MYSQL_STMT *ssps;
  MYSQL_BIND *result_bind;

  MY_LIMIT_SCROLLER scroller;

  /* TODO: enum */
  int out_params_state;
} STMT;


extern char *default_locale, *decimal_point, *thousands_sep;
extern uint decimal_point_length,thousands_sep_length;
#ifndef _UNIX_
extern HINSTANCE NEAR s_hModule;  /* DLL handle. */
#endif
#ifdef THREAD
extern pthread_mutex_t myodbc_lock;
#endif

/*
  Resource defines for "SQLDriverConnect" dialog box
*/
#define ID_LISTBOX  100
#define CONFIGDSN 1001
#define CONFIGDEFAULT 1002
#define EDRIVERCONNECT	1003

/* New data type definitions for compatibility with MySQL 5 */
#ifndef MYSQL_TYPE_NEWDECIMAL
# define MYSQL_TYPE_NEWDECIMAL 246
#endif
#ifndef MYSQL_TYPE_BIT
# define MYSQL_TYPE_BIT 16
#endif

#include "myutil.h"
#include "stringutil.h"

SQLRETURN SQL_API MySQLColAttribute(SQLHSTMT hstmt, SQLUSMALLINT column,
                                    SQLUSMALLINT attrib, SQLCHAR **char_attr,
                                    SQLLEN *num_attr);
SQLRETURN SQL_API MySQLColumnPrivileges(SQLHSTMT hstmt,
                                        SQLCHAR *catalog,
                                        SQLSMALLINT catalog_len,
                                        SQLCHAR *schema, SQLSMALLINT schema_len,
                                        SQLCHAR *table, SQLSMALLINT table_len,
                                        SQLCHAR *column,
                                        SQLSMALLINT column_len);
SQLRETURN SQL_API MySQLColumns(SQLHSTMT hstmt,
                               SQLCHAR *catalog, SQLSMALLINT catalog_len,
                               SQLCHAR *schema, SQLSMALLINT schema_len,
                               SQLCHAR *sztable, SQLSMALLINT table_len,
                               SQLCHAR *column, SQLSMALLINT column_len);
SQLRETURN SQL_API MySQLConnect(SQLHDBC hdbc, SQLWCHAR *szDSN, SQLSMALLINT cbDSN,
                               SQLWCHAR *szUID, SQLSMALLINT cbUID,
                               SQLWCHAR *szAuth, SQLSMALLINT cbAuth);
SQLRETURN SQL_API MySQLDescribeCol(SQLHSTMT hstmt, SQLUSMALLINT column,
                                   SQLCHAR **name, SQLSMALLINT *need_free,
                                   SQLSMALLINT *type, SQLULEN *def,
                                   SQLSMALLINT *scale, SQLSMALLINT *nullable);
SQLRETURN SQL_API MySQLDriverConnect(SQLHDBC hdbc, SQLHWND hwnd,
                                     SQLWCHAR *in, SQLSMALLINT in_len,
                                     SQLWCHAR *out, SQLSMALLINT out_max,
                                     SQLSMALLINT *out_len,
                                     SQLUSMALLINT completion);
SQLRETURN SQL_API MySQLForeignKeys(SQLHSTMT hstmt,
                                   SQLCHAR *pkcatalog,
                                   SQLSMALLINT pkcatalog_len,
                                   SQLCHAR *pkschema, SQLSMALLINT pkschema_len,
                                   SQLCHAR *pktable, SQLSMALLINT pktable_len,
                                   SQLCHAR *fkcatalog,
                                   SQLSMALLINT fkcatalog_len,
                                   SQLCHAR *fkschema, SQLSMALLINT fkschema_len,
                                   SQLCHAR *fktable, SQLSMALLINT fktable_len);
SQLCHAR *MySQLGetCursorName(HSTMT hstmt);
SQLRETURN SQL_API MySQLGetInfo(SQLHDBC hdbc, SQLUSMALLINT fInfoType,
                               SQLCHAR **char_info, SQLPOINTER num_info,
                               SQLSMALLINT *value_len);
SQLRETURN SQL_API MySQLGetConnectAttr(SQLHDBC hdbc, SQLINTEGER attrib,
                                      SQLCHAR **char_attr, SQLPOINTER num_attr);
SQLRETURN MySQLGetDescField(SQLHDESC hdesc, SQLSMALLINT recnum,
                            SQLSMALLINT fldid, SQLPOINTER valptr,
                            SQLINTEGER buflen, SQLINTEGER *strlen);
SQLRETURN SQL_API MySQLGetDiagField(SQLSMALLINT handle_type, SQLHANDLE handle,
                                    SQLSMALLINT record, SQLSMALLINT identifier,
                                    SQLCHAR **char_value, SQLPOINTER num_value);
SQLRETURN SQL_API MySQLGetDiagRec(SQLSMALLINT handle_type, SQLHANDLE handle,
                                  SQLSMALLINT record, SQLCHAR **sqlstate,
                                  SQLINTEGER *native, SQLCHAR **message);
SQLRETURN SQL_API MySQLGetStmtAttr(SQLHSTMT hstmt, SQLINTEGER Attribute,
                                   SQLPOINTER ValuePtr,
                                   SQLINTEGER BufferLength
                                     __attribute__((unused)),
                                  SQLINTEGER *StringLengthPtr);
SQLRETURN SQL_API MySQLGetTypeInfo(SQLHSTMT hstmt, SQLSMALLINT fSqlType);
SQLRETURN SQL_API MySQLPrepare(SQLHSTMT hstmt, SQLCHAR *query, SQLINTEGER len,
                               my_bool dupe);
SQLRETURN SQL_API MySQLPrimaryKeys(SQLHSTMT hstmt,
                                   SQLCHAR *catalog, SQLSMALLINT catalog_len,
                                   SQLCHAR *schema, SQLSMALLINT schema_len,
                                   SQLCHAR *table, SQLSMALLINT table_len);
SQLRETURN SQL_API MySQLProcedureColumns(SQLHSTMT hstmt,
                                        SQLCHAR *catalog,
                                        SQLSMALLINT catalog_len,
                                        SQLCHAR *schema,
                                        SQLSMALLINT schema_len,
                                        SQLCHAR *proc,
                                        SQLSMALLINT proc_len,
                                        SQLCHAR *column,
                                        SQLSMALLINT column_len);
SQLRETURN SQL_API MySQLProcedures(SQLHSTMT hstmt,
                                  SQLCHAR *catalog, SQLSMALLINT catalog_len,
                                  SQLCHAR *schema, SQLSMALLINT schema_len,
                                  SQLCHAR *proc, SQLSMALLINT proc_len);
SQLRETURN SQL_API MySQLSetConnectAttr(SQLHDBC hdbc, SQLINTEGER Attribute,
                                      SQLPOINTER ValuePtr,
                                      SQLINTEGER StringLengthPtr);
SQLRETURN SQL_API MySQLSetCursorName(SQLHSTMT hstmt, SQLCHAR *name,
                                     SQLSMALLINT len);
SQLRETURN MySQLSetDescField(SQLHDESC hdesc, SQLSMALLINT recnum,
                            SQLSMALLINT fldid, SQLPOINTER val,
                            SQLINTEGER buflen);
SQLRETURN SQL_API MySQLSetStmtAttr(SQLHSTMT hstmt, SQLINTEGER attribute,
                                   SQLPOINTER value, SQLINTEGER len);
SQLRETURN SQL_API MySQLSpecialColumns(SQLHSTMT hstmt, SQLUSMALLINT type,
                                      SQLCHAR *catalog, SQLSMALLINT catalog_len,
                                      SQLCHAR *schema, SQLSMALLINT schema_len,
                                      SQLCHAR *table, SQLSMALLINT table_len,
                                      SQLUSMALLINT scope,
                                      SQLUSMALLINT nullable);
SQLRETURN SQL_API MySQLStatistics(SQLHSTMT hstmt,
                                  SQLCHAR *catalog, SQLSMALLINT catalog_len,
                                  SQLCHAR *schema, SQLSMALLINT schema_len,
                                  SQLCHAR *table, SQLSMALLINT table_len,
                                  SQLUSMALLINT unique, SQLUSMALLINT accuracy);
SQLRETURN SQL_API MySQLTablePrivileges(SQLHSTMT hstmt,
                                       SQLCHAR *catalog,
                                       SQLSMALLINT catalog_len,
                                       SQLCHAR *schema, SQLSMALLINT schema_len,
                                       SQLCHAR *table, SQLSMALLINT table_len);
SQLRETURN SQL_API MySQLTables(SQLHSTMT hstmt,
                              SQLCHAR *catalog, SQLSMALLINT catalog_len,
                              SQLCHAR *schema, SQLSMALLINT schema_len,
                              SQLCHAR *table, SQLSMALLINT table_len,
                              SQLCHAR *type, SQLSMALLINT type_len);

#endif /* __DRIVER_H__ */
