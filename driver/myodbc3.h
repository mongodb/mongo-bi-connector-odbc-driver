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

/***************************************************************************
 * MYODBC3.H								   *
 *									   *
 * @description: Definations neaded by the myodbc3 driver		   *
 *									   *
 * @author     : MySQL AB(monty@mysql.com, venu@mysql.com)		   *
 * @date       : 2001-Aug-15						   *
 * @product    : myodbc3						   *
 *									   *
****************************************************************************/

#ifndef __MYODBC3_H__
#define __MYODBC3_H__

#include "../MYODBC_MYSQL.h"
#include "../MYODBC_CONF.h"
#include "../MYODBC_ODBC.h"

#ifdef APSTUDIO_READONLY_SYMBOLS
#define WIN32	/* Hack for rc files */
#endif

#ifndef myodbc_max
#define myodbc_max( a, b ) (((a) > (b)) ? (a) : (b))
#define myodbc_min( a, b ) (((a) < (b)) ? (a) : (b))
#endif

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

#ifdef SQL_SPEC_STRING
#undef SQL_SPEC_STRING
#endif

#define SQL_SPEC_STRING   "03.51"
#define ODBC_DRIVER	  "ODBC 3.51 Driver"
#define DRIVER_NAME	  "MySQL ODBC 3.51 Driver"
#define DRIVER_NONDSN_TAG "DRIVER={MySQL ODBC 3.51 Driver}"

#if defined(__APPLE__)

#define DRIVER_DLL_NAME   "libmyodbc3.dylib"
#define DRIVER_QUERY_LOGFILE "/tmp/myodbc.sql"

#elif defined(_UNIX_)

#define DRIVER_DLL_NAME   "libmyodbc3.so"
#define DRIVER_QUERY_LOGFILE "/tmp/myodbc.sql"

#else

#define DRIVER_DLL_NAME   "myodbc3.dll"
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

#define x_free(A) { void *tmp= (A); if (tmp) my_free(tmp,MYF(MY_WME+MY_FAE)); }

/*
  Connection parameters, that affects the driver behaviour
*/
#define FLAG_FIELD_LENGTH	1   /* no longer used */
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

/* We don't make any assumption about what the default may be. */
#ifndef DEFAULT_TXN_ISOLATION
# define DEFAULT_TXN_ISOLATION 0
#endif

/* Connection flags to validate after the connection*/
#define CHECK_AUTOCOMMIT_ON	1  /* AUTOCOMMIT_ON */
#define CHECK_AUTOCOMMIT_OFF	2  /* AUTOCOMMIT_OFF */

/* Statement attributes */

typedef struct stmt_options
{
  SQLUINTEGER	   bind_type,rows_in_set,cursor_type;
  SQLUINTEGER     *paramProcessedPtr;
  SQLULEN         *rowsFetchedPtr;
  SQLUINTEGER	   simulateCursor;
  SQLULEN          max_length, max_rows;
  SQLLEN          *bind_offset;
  SQLUSMALLINT	  *paramStatusPtr;
  SQLUSMALLINT	  *rowStatusPtr;
  SQLUSMALLINT	  *rowStatusPtr_ex; /* set by SQLExtendedFetch */
  SQLUSMALLINT	  *rowOperationPtr;
  my_bool	   retrieve_data;
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
  LIST		*descriptors;
  LIST		list;
  STMT_OPTIONS	stmt_options;
  MYERROR	error;
  FILE		*query_log;
  char		*dsn,*database,
		*user,*password,*server;
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
} DBC;


/* Statement row binding handler */

typedef struct st_bind
{
  MYSQL_FIELD * field;
  SQLSMALLINT	fCType;
  SQLPOINTER	rgbValue;
  SQLINTEGER	cbValueMax;
  SQLLEN *      pcbValue;
  LIST		    list;
} BIND;


/* Statement param binding handler */

typedef struct st_param_bind
{
  SQLSMALLINT   SqlType,CType;
  char *        buffer;
  char *        pos_in_query,*value;
  SQLINTEGER    ValueMax;
  SQLLEN *      actual_len;
  SQLINTEGER    value_length;
  bool	        alloced,used;
  bool	        real_param_done;
} PARAM_BIND;


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


/* IRD decriptor */

typedef struct irdDESC
{
  BIND *bind;
  uint count;
} IRD_DESC;


/* ARD descriptor */
typedef struct ardDESC
{
  DYNAMIC_ARRAY param;
  uint		count;
} ARD_DESC;


/* Descriptor handler */

typedef struct tagDESC
{
  DBC FAR  *dbc;
  IRD_DESC *ird;
  ARD_DESC *ard;
  LIST	   list;
} DESC;


/* Implementation descriptors */
typedef struct tagSTMT MY_STMT;
typedef struct impDESC
{
  MY_STMT *stmt;
  DESC	  *desc;
} IMPDESC;


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
  DYNAMIC_ARRAY params;
  BIND		*bind;
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
  ulong		getdata_offset;
  ulong		*result_lengths;
  uint		last_getdata_col;
  uint		*order,order_count,param_count,current_param,
		rows_found_in_set,bound_columns;
  enum MY_STATE state;
  enum MY_DUMMY_STATE dummy_state;
  SQLSMALLINT	*odbc_types;

  /*
    DESC PART..not yet supported....., to make MS ODBC DM to work
    without crashing, just return the dummy pointers, when it
    internally calls SQLGetStmtAttr at the time of stmt allocation
  */
  IMPDESC	ird,ard;
  IMPDESC	ipd,apd;
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

#endif /* __MYODBC3_H__ */
