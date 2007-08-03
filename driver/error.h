/*
  Copyright (C) 1995-2006 MySQL AB

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
 * ERROR.H								   *
 *									   *
 * @description: Definations for error handling				   *
 *									   *
 * @author     : MySQL AB(monty@mysql.com, venu@mysql.com)		   *
 * @date       : 2001-Aug-15						   *
 * @product    : myodbc3						   *
 *									   *
****************************************************************************/

#ifndef __ERROR_H__
#define __ERROR_H__

/*
  myodbc internal error constants
*/
#define ER_INVALID_CURSOR_NAME	 514
#define ER_ALL_COLUMNS_IGNORED	 537

/*
  myodbc3 error prefix
*/
#define MYODBC3_ERROR_PREFIX	 "[MySQL][ODBC 3.51 Driver]"
#define MYODBC_ERROR_CODE_START  500

/*
  error handler structure
*/
typedef struct tagERROR {

  char	   sqlstate[6];
  char	   message[SQL_MAX_MESSAGE_LENGTH+1];
  SQLINTEGER  native_error;
  SQLRETURN   retcode;

} MYERROR;


#define CLEAR_ENV_ERROR(env)   ((ENV FAR *)env)->error.message[0]='\0'
#define CLEAR_DBC_ERROR(dbc)   ((DBC FAR *)dbc)->error.message[0]='\0'
#define CLEAR_STMT_ERROR(stmt) ((STMT FAR *)stmt)->error.message[0]='\0'

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
    MYERR_S1009,
    MYERR_S1010,
    MYERR_S1011,
    MYERR_S1012,
    MYERR_S1013,
    MYERR_S1015,
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
    MYERR_08S01

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
