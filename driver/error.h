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
 * ERROR.H								   *
 *									   *
 * @description: Definitions for error handling				   *
 *									   *
 * @author     : MySQL AB(monty@mysql.com, venu@mysql.com)		   *
 * @date       : 2001-Aug-15						   *
 * @product    : myodbc3						   *
 *									   *
****************************************************************************/

#ifndef __ERROR_H__
#define __ERROR_H__

/* Including driver version definitions */
#include "../MYODBC_CONF.h"
/*
  myodbc internal error constants
*/
#define ER_INVALID_CURSOR_NAME	 514
#define ER_ALL_COLUMNS_IGNORED	 537

/*
  myodbc5 error prefix
*/
#define MYODBC_ERROR_PREFIX	 "[MySQL][ODBC "MYODBC_STRDRIVERID" Driver]"
#define MYODBC_ERROR_CODE_START  500

/*
  error handler structure
*/
typedef struct tagERROR {
  SQLRETURN   retcode;
  char        current;

  char	   sqlstate[6];
  char	   message[SQL_MAX_MESSAGE_LENGTH+1];
  SQLINTEGER  native_error;

} MYERROR;

#define CLEAR_ERROR(error) do {\
  error.message[0]= '\0'; \
  error.current= 0; \
} while (0)

#define CLEAR_ENV_ERROR(env)   CLEAR_ERROR(((ENV *)env)->error)
#define CLEAR_DBC_ERROR(dbc)   CLEAR_ERROR(((DBC *)dbc)->error)
#define CLEAR_STMT_ERROR(stmt) CLEAR_ERROR(((STMT *)stmt)->error)
#define CLEAR_DESC_ERROR(desc) CLEAR_ERROR(((DESC *)desc)->error)

#define NEXT_ERROR(error) \
  (error.current ? 2 : (error.current= 1))

#define NEXT_ENV_ERROR(env)   NEXT_ERROR(((ENV *)env)->error)
#define NEXT_DBC_ERROR(dbc)   NEXT_ERROR(((DBC *)dbc)->error)
#define NEXT_STMT_ERROR(stmt) NEXT_ERROR(((STMT *)stmt)->error)
#define NEXT_DESC_ERROR(desc) NEXT_ERROR(((DESC *)desc)->error)

/*
  list of MyODBC3 error codes
*/
typedef enum myodbc_errid
{
    MYERR_01000 = 0,
    MYERR_01004,
    MYERR_01S02,
    MYERR_01S03,
    MYERR_01S04,
    MYERR_01S06,
    MYERR_07001,
    MYERR_07005,
    MYERR_07006,
    MYERR_07009,
    MYERR_08002,
    MYERR_08003,
    MYERR_24000,
    MYERR_25000,
    MYERR_25S01,
    MYERR_34000,
    MYERR_HYT00,
    MYERR_S1000,
    MYERR_S1001,
    MYERR_S1002,
    MYERR_S1003,
    MYERR_S1004,
    MYERR_S1007,
    MYERR_S1009,
    MYERR_S1010,
    MYERR_S1011,
    MYERR_S1012,
    MYERR_S1013,
    MYERR_S1015,
    MYERR_S1016,
    MYERR_S1017,
    MYERR_S1024,
    MYERR_S1090,
    MYERR_S1091,
    MYERR_S1092,
    MYERR_S1093,
    MYERR_S1095,
    MYERR_S1106,
    MYERR_S1107,
    MYERR_S1109,
    MYERR_S1C00,

    MYERR_21S01,
    MYERR_23000,
    MYERR_42000,
    MYERR_42S01,
    MYERR_42S02,
    MYERR_42S12,
    MYERR_42S21,
    MYERR_42S22,
    MYERR_08S01,
    /* Please add new errors to the end of enum, and not in alphabet order */
    MYERR_08004,
} myodbc_errid;

/*
  error handler-predefined structure
  odbc2 state, odbc3 state, message and return code
*/
typedef struct myodbc3_err_str {
  char	   sqlstate[6];  /* ODBC3 STATE, if SQL_OV_ODBC2, then ODBC2 STATE */
  char	   message[SQL_MAX_MESSAGE_LENGTH+1];/* ERROR MSG */
  SQLRETURN   retcode;	    /* RETURN CODE */
} MYODBC3_ERR_STR;

#endif /* __ERROR_H__ */
