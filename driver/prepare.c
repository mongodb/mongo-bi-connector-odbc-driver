/*
  Copyright (c) 2000, 2012, Oracle and/or its affiliates. All rights reserved.

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
  @file  prepare.c
  @brief Prepared statement functions.
*/

/***************************************************************************
 * The following ODBC APIs are implemented in this file:		   *
 *									   *
 *   SQLPrepare		 (ISO 92)					   *
 *   SQLBindParameter	 (ODBC)						   *
 *   SQLDescribeParam	 (ODBC)						   *
 *   SQLParamOptions	 (ODBC, Deprecated)				   *
 *   SQLNumParams	 (ISO 92)					   *
 *   SQLSetScrollOptions (ODBC, Deprecated)				   *
 *									   *
 ****************************************************************************/

#include "driver.h"
#ifndef _UNIX_
# include <dos.h>
#endif /* !_UNIX_ */
#include <my_list.h>
#include <m_ctype.h>

/**
  Prepare a statement for later execution.

  @param[in] hStmt  Handle of the statement
  @param[in] query  The statement to prepare (in connection character set)
  @param[in] len    The length of the statement (or @c SQL_NTS if it is
                    NUL-terminated)
  @param[in] dupe   Set to @c TRUE if query is already a duplicate, and
                    freeing the value is now up to the driver
*/
SQLRETURN SQL_API MySQLPrepare(SQLHSTMT hstmt, SQLCHAR *query, SQLINTEGER len,
                               my_bool dupe)
{
  STMT *stmt= (STMT *)hstmt;
  /*
    We free orig_query here, instead of my_SQLPrepare, because
    my_SQLPrepare is used by my_pos_update() when a statement requires
    additional parameters.
  */

  if (GET_QUERY(&stmt->orig_query) != NULL)
  {
    reset_parsed_query(&stmt->orig_query, NULL, NULL, NULL);
  }

  return my_SQLPrepare(hstmt, query, len, dupe);
}


/*
  @type    : myodbc3 internal
  @purpose : prepares an SQL string for execution
*/
SQLRETURN my_SQLPrepare(SQLHSTMT hstmt, SQLCHAR *szSqlStr, SQLINTEGER cbSqlStr,
                        my_bool dupe)
{
  STMT FAR *stmt= (STMT FAR*) hstmt;

  LINT_INIT(end);

  CLEAR_STMT_ERROR(stmt);

  reset_parsed_query(&stmt->query, NULL, NULL, NULL);

  /* If we need to make a copy - !dupe or empty query string pointer
     (dupp_str will make it an empty string) */
  if (!(dupe && szSqlStr))
  {
    if (!(szSqlStr= dupp_str((char *)szSqlStr, cbSqlStr)))
    {
      return set_error(stmt, MYERR_S1001, NULL, 4001);
    }
  }

  return prepare(stmt, szSqlStr, cbSqlStr);
}


/*
  @type    : myodbc3 internal
  @purpose : binds a buffer to a parameter marker in an SQL statement.
*/

SQLRETURN SQL_API my_SQLBindParameter( SQLHSTMT     StatementHandle,
                                       SQLUSMALLINT ParameterNumber,
                                       SQLSMALLINT  InputOutputType,
                                       SQLSMALLINT  ValueType,
                                       SQLSMALLINT  ParameterType,
                                       SQLULEN      ColumnSize,
                                       SQLSMALLINT  DecimalDigits,
                                       SQLPOINTER   ParameterValuePtr,
                                       SQLLEN       BufferLength,
                                       SQLLEN *     StrLen_or_IndPtr )
{
    STMT *stmt= (STMT *)StatementHandle;
    DESCREC *aprec= desc_get_rec(stmt->apd, ParameterNumber - 1, TRUE);
    DESCREC *iprec= desc_get_rec(stmt->ipd, ParameterNumber - 1, TRUE);
    SQLRETURN rc;
    /* TODO if this function fails, the SQL_DESC_COUNT should be unchanged in apd, ipd */

    CLEAR_STMT_ERROR(stmt);

    if (ParameterNumber < 1)
    {
        set_error(stmt,MYERR_S1093,NULL,0);
        return SQL_ERROR;
    }

    if (aprec->par.alloced)
    {
        aprec->par.alloced= FALSE;
        assert(aprec->par.value);
        x_free(aprec->par.value);
        aprec->par.value = NULL;
    }

    /* reset all param fields */
    desc_rec_init_apd(aprec);
    desc_rec_init_ipd(iprec);

    /* first, set apd fields */
    if (ValueType == SQL_C_DEFAULT)
    {
      ValueType= default_c_type(ParameterType);
      /*
        Access treats BIGINT as a string on linked tables.
        The value is read correctly, but bound as a string.
      */
      if (ParameterType == SQL_BIGINT && stmt->dbc->ds->default_bigint_bind_str)
        ValueType= SQL_C_CHAR;
    }
    if (!SQL_SUCCEEDED(rc = stmt_SQLSetDescField(stmt, stmt->apd,
                                                 ParameterNumber,
                                                 SQL_DESC_CONCISE_TYPE,
                                                 (SQLPOINTER)(SQLLEN)ValueType,
                                                 SQL_IS_SMALLINT)))
        return rc;

    if (!SQL_SUCCEEDED(rc= stmt_SQLSetDescField(stmt, stmt->apd, ParameterNumber,
                                                SQL_DESC_OCTET_LENGTH,
                                                (SQLPOINTER)BufferLength,
                                                SQL_IS_INTEGER)))
        return rc;
    /* these three *must* be the last APD params bound */
    if (!SQL_SUCCEEDED(rc= stmt_SQLSetDescField(stmt, stmt->apd, ParameterNumber,
                                                SQL_DESC_DATA_PTR,
                                                ParameterValuePtr, SQL_IS_POINTER)))
        return rc;
    if (!SQL_SUCCEEDED(rc= stmt_SQLSetDescField(stmt, stmt->apd, ParameterNumber,
                                                SQL_DESC_OCTET_LENGTH_PTR,
                                                StrLen_or_IndPtr, SQL_IS_POINTER)))
        return rc;
    if (!SQL_SUCCEEDED(rc= stmt_SQLSetDescField(stmt, stmt->apd, ParameterNumber,
                                                SQL_DESC_INDICATOR_PTR,
                                                StrLen_or_IndPtr, SQL_IS_POINTER)))
        return rc;

    /* now the ipd fields */
    if (!SQL_SUCCEEDED(rc= stmt_SQLSetDescField(stmt, stmt->ipd,
                                                ParameterNumber,
                                                SQL_DESC_CONCISE_TYPE,
                                                (SQLPOINTER)(SQLINTEGER)ParameterType,
                                                SQL_IS_SMALLINT)))
        return rc;

    if (!SQL_SUCCEEDED(rc= stmt_SQLSetDescField(stmt, stmt->ipd,
                                                ParameterNumber,
                                                SQL_DESC_PARAMETER_TYPE,
                                                (SQLPOINTER)(SQLINTEGER)InputOutputType,
                                                SQL_IS_SMALLINT)))
        return rc;

    /* set fields from ColumnSize and DecimalDigits */
    switch (ParameterType)
    {
    case SQL_TYPE_TIME:
    case SQL_TYPE_TIMESTAMP:
    case SQL_INTERVAL_SECOND:
    case SQL_INTERVAL_DAY_TO_SECOND:
    case SQL_INTERVAL_HOUR_TO_SECOND:
    case SQL_INTERVAL_MINUTE_TO_SECOND:
        rc= stmt_SQLSetDescField(stmt, stmt->ipd, ParameterNumber,
                                 SQL_DESC_PRECISION,
                                 (SQLPOINTER)(SQLINTEGER)DecimalDigits,
                                 SQL_IS_SMALLINT);
        break;
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
    case SQL_BINARY:
    case SQL_VARBINARY:
    case SQL_LONGVARBINARY:
        rc= stmt_SQLSetDescField(stmt, stmt->ipd, ParameterNumber,
                                 SQL_DESC_LENGTH, (SQLPOINTER)ColumnSize,
                                 SQL_IS_ULEN);
        break;
    case SQL_NUMERIC:
    case SQL_DECIMAL:
        rc= stmt_SQLSetDescField(stmt, stmt->ipd, ParameterNumber,
                                 SQL_DESC_SCALE,
                                 (SQLPOINTER)(SQLINTEGER)DecimalDigits,
                                 SQL_IS_SMALLINT);
        if (!SQL_SUCCEEDED(rc))
            return rc;
        /* fall through */
    case SQL_FLOAT:
    case SQL_REAL:
    case SQL_DOUBLE:
        rc= stmt_SQLSetDescField(stmt, stmt->ipd, ParameterNumber,
                                 SQL_DESC_PRECISION,
                                 (SQLPOINTER)ColumnSize,
                                 SQL_IS_ULEN);
        break;
    default:
        rc= SQL_SUCCESS;
    }
    if (!SQL_SUCCEEDED(rc))
        return rc;

    aprec->par.real_param_done= TRUE;

    return SQL_SUCCESS;
}


/**
  Deprecated function, for more details see SQLBindParamater. 

  @param[in] stmt           Handle to statement
  @param[in] ipar           Parameter number
  @param[in] fCType         Value type
  @param[in] fSqlType       Parameter type
  @param[in] cbColDef       Column size
  @param[in] ibScale        Decimal digits
  @param[in] rgbValue       Parameter value pointer
  @param[in] pcbValue       String length or index pointer

  @return SQL_SUCCESS or SQL_ERROR (and diag is set)

*/

SQLRETURN SQL_API SQLSetParam(SQLHSTMT        hstmt,
                              SQLUSMALLINT    ipar, 
                              SQLSMALLINT     fCType, 
                              SQLSMALLINT     fSqlType,
                              SQLULEN         cbColDef, 
                              SQLSMALLINT     ibScale,
                              SQLPOINTER      rgbValue, 
                              SQLLEN *        pcbValue)
{
  return my_SQLBindParameter(hstmt, ipar, SQL_PARAM_INPUT_OUTPUT, fCType, 
                             fSqlType, cbColDef, ibScale, rgbValue, 
                             SQL_SETPARAM_VALUE_MAX, pcbValue);
}


/*
  @type    : ODBC 2.0 API
  @purpose : binds a buffer to a parameter marker in an SQL statement.
*/

SQLRETURN SQL_API SQLBindParameter( SQLHSTMT        hstmt,
                                    SQLUSMALLINT    ipar, 
                                    SQLSMALLINT     fParamType,
                                    SQLSMALLINT     fCType, 
                                    SQLSMALLINT     fSqlType,
                                    SQLULEN         cbColDef, 
                                    SQLSMALLINT     ibScale,
                                    SQLPOINTER      rgbValue, 
                                    SQLLEN          cbValueMax,
                                    SQLLEN *        pcbValue )
{
  return my_SQLBindParameter(hstmt, ipar, fParamType, fCType, fSqlType,
                             cbColDef, ibScale, rgbValue, cbValueMax, pcbValue);
}


/*
  @type    : ODBC 1.0 API
  @purpose : returns the description of a parameter marker associated
  with a prepared SQL statement
*/

SQLRETURN SQL_API SQLDescribeParam( SQLHSTMT        hstmt,
                                    SQLUSMALLINT    ipar __attribute__((unused)),
                                    SQLSMALLINT FAR *pfSqlType,
                                    SQLULEN *       pcbColDef,
                                    SQLSMALLINT FAR *pibScale __attribute__((unused)),
                                    SQLSMALLINT FAR *pfNullable )
{
    STMT FAR *stmt= (STMT FAR*) hstmt;

    if (pfSqlType)
        *pfSqlType= SQL_VARCHAR;
    if (pcbColDef)
        *pcbColDef= (stmt->dbc->ds->allow_big_results ? 24*1024*1024L : 255);
    if (pfNullable)
        *pfNullable= SQL_NULLABLE_UNKNOWN;

    return SQL_SUCCESS;
}


/*
  @type    : ODBC 1.0 API
  @purpose : sets multiple values (arrays) for the set of parameter markers
*/

#ifdef USE_SQLPARAMOPTIONS_SQLULEN_PTR
SQLRETURN SQL_API SQLParamOptions( SQLHSTMT     hstmt, 
                                   SQLULEN      crow,
                                   SQLULEN      *pirow )
{
  SQLINTEGER buflen= SQL_IS_ULEN;
#else
SQLRETURN SQL_API SQLParamOptions( SQLHSTMT     hstmt, 
                                   SQLUINTEGER  crow,
                                   SQLUINTEGER *pirow )
{
  SQLINTEGER buflen= SQL_IS_UINTEGER;
#endif
  SQLRETURN rc;
  STMT *stmt= (STMT *)hstmt;
  rc= stmt_SQLSetDescField(stmt, stmt->apd, 0, SQL_DESC_ARRAY_SIZE,
                           (SQLPOINTER)crow, buflen);
  if (!SQL_SUCCEEDED(rc))
    return rc;
  /* 
     We make the assumption that if this is using the pointer to a 64-bit
     value, then it is correct. However the SQL_DESC_ROWS_PROCESSED_PTR is a
     pointer to a 32-bit value so we zero-out the unused half and save a
     pointer to the 32-bits we'll actually use.
     Some more discussion at
        http://mail.easysoft.com/pipermail/unixodbc-dev/2005-July/000627.html
  */
#ifdef USE_SQLPARAMOPTIONS_SQLULEN_PTR
  {
    int x = 1;
    if(*(char *)&x != 1)
    {
      *pirow= 0;
      pirow= (SQLULEN *)((char *)pirow + 4);
    }
  }
#endif
  rc= stmt_SQLSetDescField(stmt, stmt->ipd, 0, SQL_DESC_ROWS_PROCESSED_PTR,
                           pirow, SQL_IS_POINTER);
  return rc;
}


/*
  @type    : ODBC 1.0 API
  @purpose : returns the number of parameter markers.
*/

SQLRETURN SQL_API SQLNumParams(SQLHSTMT hstmt, SQLSMALLINT *pcpar)
{
  STMT *stmt= (STMT *)hstmt;

  if (pcpar)
    *pcpar= stmt->param_count;

  return SQL_SUCCESS;
}


/*
  @type    : ODBC 1.0 API
  @purpose : sets options that control the behavior of cursors.
*/

SQLRETURN SQL_API SQLSetScrollOptions(  SQLHSTMT        hstmt,
                                        SQLUSMALLINT    fConcurrency __attribute__((unused)),
                                        SQLLEN          crowKeyset __attribute__((unused)),
                                        SQLUSMALLINT    crowRowset )
{
    STMT *stmt= (STMT *)hstmt;
    return stmt_SQLSetDescField(stmt, stmt->ard, 0, SQL_DESC_ARRAY_SIZE,
                                (SQLPOINTER)(SQLUINTEGER)crowRowset,
                                SQL_IS_USMALLINT);
}

