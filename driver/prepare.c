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
 * PREPARE.C								   *
 *									   *
 * @description: For handling prepare statements			   *
 *									   *
 * @author     : MySQL AB(monty@mysql.com, venu@mysql.com)		   *
 * @date       : 2001-Aug-15						   *
 * @product    : myodbc3						   *
 *									   *
 ****************************************************************************/

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

#include "myodbc3.h"
#ifndef _UNIX_
    #include <dos.h>
#endif /* !_UNIX_ */
#include <my_list.h>
#include <m_ctype.h>

/*
  @type    : ODBC 1.0 API
  @purpose : prepares an SQL string for execution
*/

SQLRETURN SQL_API SQLPrepare(SQLHSTMT hstmt,SQLCHAR FAR *szSqlStr,
                             SQLINTEGER cbSqlStr)
{
    MYODBCDbgEnter;
    MYODBCDbgReturnReturn( my_SQLPrepare( hstmt, szSqlStr, cbSqlStr ) );
}

/*
  @type    : myodbc3 internal
  @purpose : prepares an SQL string for execution
*/

SQLRETURN my_SQLPrepare( SQLHSTMT hstmt, SQLCHAR FAR *szSqlStr, SQLINTEGER cbSqlStr )
{
    STMT FAR *stmt= (STMT FAR*) hstmt;
    char in_string,*pos;
    uint param_count;
#ifdef	USE_MB
    char *end;
#endif
    CHARSET_INFO *charset_info= stmt->dbc->mysql.charset;
    int bPerhapsEmbraced = 1;
    int bEmbraced = 0;
    char *pcLastCloseBrace = 0;

    MYODBCDbgEnter;

    LINT_INIT(end);

    CLEAR_STMT_ERROR(stmt);
    if (stmt->query)
        my_free(stmt->query,MYF(0));
#ifdef NOT_NEEDED  
    SQLFreeStmt(hstmt,SQL_UNBIND);    /* Not needed according to VB 5.0 */
#endif

    if (!(stmt->query= dupp_str((char*) szSqlStr, cbSqlStr)))
        MYODBCDbgReturnReturn(set_error(stmt,MYERR_S1001,NULL,4001));
    MYODBCDbgInfo( "%s", stmt->query );

    /* Count number of parameters and save position for each parameter */
    in_string= 0;
    param_count= 0;

#ifdef	USE_MB
    if (use_mb(charset_info))
        end= strend(stmt->query);
#endif

    for (pos= stmt->query; *pos ; pos++)
    {
#ifdef	USE_MB
        if (use_mb(charset_info))
        {
            int l;
            if ((l= my_ismbchar(charset_info, pos , end)))
            {
                pos+= l-1;
                continue;
            }
        }
#endif

        /* handle case where we have statement within {} - in this case we want to replace'em with ' ' */
        if ( bPerhapsEmbraced )
        {
            if ( *pos == '{' )
            {
                bPerhapsEmbraced = 0;
                bEmbraced = 1;
                *pos =  ' ';
                pos++;
                continue;
            }
            else if ( !isspace( *pos ) )
                bPerhapsEmbraced = 0;
        }
        else if ( bEmbraced && *pos == '}' )
            pcLastCloseBrace = pos;

        /* escape char? */
        if (*pos == '\\' && pos[1])     /* Next char is escaped */
        {
            pos++;
            continue;
        }

        /* in a string? */
        if (*pos == in_string)
        {
            if (pos[1] == in_string)      /* Two quotes is ok */
                pos++;
            else
                in_string= 0;
            continue;
        }

        /* parameter marker? */
        if (!in_string)
        {
            if (*pos == '\'' || *pos == '"' || *pos == '`') /* start of string? */
            {
                in_string= *pos;
                continue;
            }
            if (*pos == '?')
            {
                PARAM_BIND *param;

                if (param_count >= stmt->params.elements)
                {
                    PARAM_BIND tmp_param;
                    bzero((gptr) &tmp_param,sizeof(tmp_param));
                    if (push_dynamic(&stmt->params,(gptr) &tmp_param))
                    {
                        MYODBCDbgReturnReturn(set_error(stmt,MYERR_S1001,NULL,4001));
                    }
                }
                param= dynamic_element(&stmt->params,param_count,PARAM_BIND*);
                param->pos_in_query= pos;
                param_count++;
            }
        }
    }

    /* remove closing brace if we have one */
    if ( pcLastCloseBrace )
        *pcLastCloseBrace = ' ';

    stmt->param_count= param_count;
    stmt->query_end= pos;
    stmt->state= ST_PREPARED;
    MYODBCDbgInfo( "Parameter count: %ld", stmt->param_count );
    MYODBCDbgReturnReturn(SQL_SUCCESS);
}


/*
  @type    : myodbc3 internal
  @purpose : binds a buffer to a parameter marker in an SQL statement.
*/

SQLRETURN SQL_API my_SQLBindParameter( SQLHSTMT     hstmt,
                                       SQLUSMALLINT ipar,
                                       SQLSMALLINT  fParamType __attribute__((unused)),
                                       SQLSMALLINT  fCType,
                                       SQLSMALLINT  fSqlType,
                                       SQLULEN      cbColDef __attribute__((unused)),
                                       SQLSMALLINT  ibScale __attribute__((unused)),
                                       SQLPOINTER   rgbValue,
                                       SQLLEN       cbValueMax,
                                       SQLLEN *     pcbValue )
{
    STMT FAR *stmt= (STMT FAR*) hstmt;
    PARAM_BIND param;

    MYODBCDbgEnter;
    MYODBCDbgInfo( "ipar: %d", ipar );
    MYODBCDbgInfo( "Ctype: %d", fCType );
    MYODBCDbgInfo( "SQLtype: %d", fSqlType );
    MYODBCDbgInfo( "rgbValue: 0x%lx", rgbValue );
    MYODBCDbgInfo( "ValueMax: %ld", cbValueMax );
    MYODBCDbgInfo( "Valueptr: 0x%lx", pcbValue );
    MYODBCDbgInfo( "Value: %ld", pcbValue ? *pcbValue : 0L );

    CLEAR_STMT_ERROR(stmt);

    if (ipar-- < 1)
    {
        set_error(stmt,MYERR_S1093,NULL,0);
        MYODBCDbgReturnReturn(SQL_ERROR);
    }
    if (fCType == SQL_C_NUMERIC) /* We don't support this now */
    {
        set_error(stmt,MYERR_07006,
                  "Restricted data type attribute violation(SQL_C_NUMERIC)",0);
        MYODBCDbgReturnReturn(SQL_ERROR);
    }
    if (stmt->params.elements > ipar)
    {
        /* Change old bind parameter */
        PARAM_BIND *old= dynamic_element(&stmt->params,ipar,PARAM_BIND*);
        if (old->alloced)
        {
            old->alloced= 0;
            my_free(old->value,MYF(0));
        }
        memcpy((gptr) &param,(gptr) old,sizeof(param));
    }
    else
        bzero((gptr)&param, sizeof(param));
    /* Simply record the values. These are used later (SQLExecute) */
    param.used= 1;
    param.SqlType= fSqlType;
    param.CType= (fCType == SQL_C_DEFAULT ? default_c_type(fSqlType) : fCType);
    param.buffer= rgbValue;
    param.ValueMax= cbValueMax;
    param.actual_len= pcbValue;
    param.real_param_done= TRUE;

    if (set_dynamic(&stmt->params,(gptr) &param,ipar))
    {
        set_error(stmt,MYERR_S1001,NULL,4001);
        MYODBCDbgReturnReturn(SQL_ERROR);
    }
    MYODBCDbgReturnReturn(SQL_SUCCESS);
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
    MYODBCDbgEnter;
    MYODBCDbgReturnReturn( my_SQLBindParameter( hstmt, 
                                ipar, 
                                fParamType, 
                                fCType, 
                                fSqlType,
                                cbColDef, 
                                ibScale, 
                                rgbValue, 
                                cbValueMax,
                                pcbValue ) );
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

    MYODBCDbgEnter;

    if (pfSqlType)
        *pfSqlType= SQL_VARCHAR;
    if (pcbColDef)
        *pcbColDef= (stmt->dbc->flag & FLAG_BIG_PACKETS ? 24*1024*1024L : 255);
    if (pfNullable)
        *pfNullable= SQL_NULLABLE_UNKNOWN;

    MYODBCDbgReturnReturn(SQL_SUCCESS);
}


/*
  @type    : ODBC 1.0 API
  @purpose : sets multiple values (arrays) for the set of parameter markers
*/

#ifdef USE_SQLPARAMOPTIONS_SQLULEN_PTR
SQLRETURN SQL_API SQLParamOptions( SQLHSTMT     hstmt, 
                                   SQLULEN      crow,
                                   SQLULEN      *pirow __attribute__((unused)) )
#else
SQLRETURN SQL_API SQLParamOptions( SQLHSTMT     hstmt, 
                                   SQLUINTEGER  crow,
                                   SQLUINTEGER *pirow __attribute__((unused)) )
#endif
{
    MYODBCDbgEnter;

    if (crow != 1)
    {
        /*
          Currently return warning for batch processing request,
          but need to handle in the future..
        */
        MYODBCDbgReturnReturn( set_error( hstmt, MYERR_01S02, "Option value changed to default parameter size", 0 ) );
    }
    MYODBCDbgReturnReturn( SQL_SUCCESS );
}


/*
  @type    : ODBC 1.0 API
  @purpose : returns the number of parameter markers.
*/

SQLRETURN SQL_API SQLNumParams(SQLHSTMT hstmt, SQLSMALLINT FAR *pcpar)
{
    STMT FAR *stmt= (STMT FAR*) hstmt;

    MYODBCDbgEnter;

    if (pcpar)
        *pcpar= stmt->param_count;

    MYODBCDbgReturnReturn(SQL_SUCCESS);
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
    STMT FAR *stmt= (STMT FAR*) hstmt;

    MYODBCDbgEnter;

    stmt->stmt_options.rows_in_set= crowRowset;

    MYODBCDbgReturnReturn( SQL_SUCCESS );
}
