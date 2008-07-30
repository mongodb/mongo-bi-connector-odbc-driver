/*
  Copyright (C) 1995-2007 MySQL AB

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

/**
  @file driver.h
  @brief Definitions needed by the driver
*/

#ifndef __DRIVER_H__
#define __DRIVER_H__

#include "../MYODBC_MYSQL.h"
#include "../MYODBC_CONF.h"
#include "../MYODBC_ODBC.h"

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

/* Misc definations for AIX .. */
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

#if defined(_WIN32) || defined(WIN32)
# define INTFUNC  __stdcall
# define EXPFUNC  __stdcall
# if !defined(HAVE_LOCALTIME_R)
#  define HAVE_LOCALTIME_R 1
# endif
#else
# define INTFUNC PASCAL
# define EXPFUNC __export CALLBACK
/* Simple macros to make ltdl look like the Windows library funcs. */
# define HMODULE lt_dlhandle
# define LoadLibrary(library) lt_dlopen((library))
# define GetProcAddress(module, proc) lt_dlsym((module), (proc))
# define FreeLibrary(module) lt_dlclose((module))
#endif

#define ODBC_DRIVER	  "ODBC 5.1 Driver"
#define DRIVER_NAME	  "MySQL ODBC 5.1 Driver"
#define DRIVER_NONDSN_TAG "DRIVER={MySQL ODBC 5.1 Driver}"

#if defined(__APPLE__)

#define DRIVER_QUERY_LOGFILE "/tmp/myodbc.sql"

#elif defined(_UNIX_)

#define DRIVER_QUERY_LOGFILE "/tmp/myodbc.sql"

#else

#define DRIVER_QUERY_LOGFILE "c:\\myodbc.sql"

#endif

/*
   Internal driver definations
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

#define x_free(A) { void *tmp= (A); if (tmp) my_free((char *) tmp,MYF(MY_WME+MY_FAE)); }

/*
  Connection parameters, that affects the driver behaviour
*/
#define FLAG_FIELD_LENGTH	1   /* no longer used*/
#define FLAG_FOUND_ROWS		2   /* Access can't handle affected_rows */
#define FLAG_DEBUG		4   /* no longer used */
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

/* We don't make any assumption about what the default may be. */
#ifndef DEFAULT_TXN_ISOLATION
# define DEFAULT_TXN_ISOLATION 0
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
  SQLUINTEGER  *rows_processed_ptr;

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
  SQLINTEGER *indicator_ptr;
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
  ENV		*env;
  MYSQL		mysql;
  LIST		*statements;
  LIST		*exp_desc; /* explicit descriptors */
  LIST		list;
  STMT_OPTIONS	stmt_options;
  MYERROR	error;
  FILE		*query_log;
  char		*dsn,*database,
		*user,*password,*server,*socket;
  char		st_error_prefix[255];
  ulong		flag;
  SQLUINTEGER login_timeout;
  time_t	last_query_time;
  int		txn_isolation;
  uint		port;
  uint		cursor_count;
  uint		commit_flag;
#ifdef THREAD
  pthread_mutex_t lock;
#endif

  my_bool       unicode;             /* Whether SQL*ConnectW was used */
  CHARSET_INFO *ansi_charset_info,   /* 'ANSI' charset (SQL_C_CHAR) */
               *cxn_charset_info;    /* Connection charset ('ANSI' or utf-8) */
} DBC;


/* Statement states */

enum MY_STATE { ST_UNKNOWN, ST_PREPARED, ST_PRE_EXECUTED, ST_EXECUTED };
enum MY_DUMMY_STATE { ST_DUMMY_UNKNOWN, ST_DUMMY_PREPARED, ST_DUMMY_EXECUTED };


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
  DBC FAR	*dbc;
  MYSQL_RES	*result;
  my_bool       fake_result;
  MYSQL_ROW	array,result_array,current_values;
  MYSQL_ROW	(*fix_fields)(struct tagSTMT FAR* stmt,MYSQL_ROW row);
  MYSQL_FIELD	*fields;
  MYSQL_ROW_OFFSET  end_of_set;
  DYNAMIC_ARRAY param_pos; /* param placeholder positions */
  LIST		list;
  MYCURSOR	cursor;
  MYERROR	error;
  STMT_OPTIONS	stmt_options;
  char		*table_name;
  char		*query,*query_end;
  /*
    We save a copy of the original query before we modify it for 'WHERE
    CURRENT OF' cursor handling.
  */
  char          *orig_query,*orig_query_end;
  my_ulonglong	affected_rows;
  long		current_row;
  long		cursor_row;
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
  uint		*order,order_count,param_count,current_param,rows_found_in_set;
  enum MY_STATE state;
  enum MY_DUMMY_STATE dummy_state;

  DESC *ard;
  DESC *ird;
  DESC *apd;
  DESC *ipd;
  /* implicit descriptors */
  DESC *imp_ard;
  DESC *imp_apd;
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
