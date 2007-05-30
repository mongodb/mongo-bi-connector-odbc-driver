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
  @file  results.c
  @brief Result set and related information functions.
*/

/***************************************************************************
 * The following ODBC APIs are implemented in this file:		   *
 *									   *
 *   SQLRowCount	  (ISO 92)					   *
 *   SQLNumResultCols	  (ISO 92)					   *
 *   SQLDescribeCol	  (ISO 92)					   *
 *   SQLColAttribute	  (ISO 92)					   *
 *   SQLColAttributes	  (ODBC, Deprecated)				   *
 *   SQLBindCol		  (ISO 92)					   *
 *   SQLFetch		  (ISO 92)					   *
 *   SQLFetchScroll	  (ISO 92)					   *
 *   SQLGetData		  (ISO 92)					   *
 *   SQLExtendedFetch	  (ODBC, Deprecated)				   *
 *   SQLMoreResults	  (ODBC)					   *
 *									   *
 ****************************************************************************/

#include "myodbc3.h"
#include <errmsg.h>
#include <ctype.h>
#include <locale.h>

#define SQL_MY_PRIMARY_KEY 1212

SQLRETURN SQL_API sql_get_data( STMT *          stm,
                                SQLSMALLINT     fCType,
                                MYSQL_FIELD *   field,
                                SQLPOINTER      rgbValue,
                                SQLINTEGER      cbValueMax,
                                SQLLEN *        pcbValue,
                                char *          value,
                                uint            length );

/*!
    \brief  Returns true if we are dealing with a statement which
            is likely to result in reading only (SELECT || SHOW).

            Some ODBC calls require knowledge about a statement
            which we can not determine until we have executed 
            the statement. This is because we do not parse the SQL
            - the server does.

            However if we silently execute a pending statement we
            may insert rows.

            So we do a very crude check of the SQL here to reduce 
            the chance of a problem.

    \sa     BUG 5778            
*/
BOOL isStatementForRead( STMT FAR *stmt )
{
    char *pCursor;
    int n = 0;
    char szToken[55];

    if ( !stmt )
        return FALSE;

    if ( !stmt->query )
        return FALSE;

    /* eat up any space */
    for ( pCursor = stmt->query; pCursor != stmt->query_end && isspace( *pCursor ); )
    {
        pCursor++; 
    }

    /* continue while alpha-numeric */
    for ( ; pCursor != stmt->query_end && !isspace( *pCursor ) && n < 50; )
    {
        szToken[n] = toupper( *pCursor );
        pCursor++; 
        n++;
    }

    szToken[n] = '\0';

    if ( strcmp( szToken, "SELECT" ) == 0 || strcmp( szToken, "SHOW" ) == 0 )
        return TRUE;

    return FALSE;
}

/*
  @type    : myodbc3 internal
  @purpose : execute the query if it is only prepared. This is needed
  because the ODBC standard allows calling some functions
  before SQLExecute(). (Stupid!)
*/

static SQLRETURN check_result(STMT FAR *stmt )
{
    SQLRETURN error= 0;

    switch ( stmt->state )
    {
        case ST_UNKNOWN:
            error= set_stmt_error(stmt,"24000","Invalid cursor state",0);
            break;
        case ST_PREPARED:
            if ( isStatementForRead( stmt ) )
            {
                if ( (error= my_SQLExecute(stmt)) == SQL_SUCCESS )
                    stmt->state= ST_PRE_EXECUTED;  /* mark for execute */
            }
            else
                error = SQL_SUCCESS;
            break;
        case ST_PRE_EXECUTED:
        case ST_EXECUTED:
            error= SQL_SUCCESS;
    }
    return(error);
}

/*
  @type    : myodbc3 internal
  @purpose : does the any open param binding
*/

SQLRETURN do_dummy_parambind(SQLHSTMT hstmt)
{
    STMT FAR *stmt= (STMT FAR *)hstmt;
    uint     nparam;

    for ( nparam= 0; nparam < stmt->param_count; nparam++ )
    {
        PARAM_BIND *param= dynamic_element(&stmt->params,nparam,PARAM_BIND*);
        if ( param->real_param_done != TRUE && param->used != 1 )
        {
            /*
          do the dummy bind temporarily to get the result set
          and once everything is done, remove it
            */
            param->used= 1;
            param->SqlType= SQL_VARCHAR;
            param->CType= SQL_C_CHAR;
            param->buffer= "NULL";
            param->actual_len= 0;

            if ( set_dynamic(&stmt->params,(gptr)param,nparam) )
                return set_stmt_error(stmt,"S1001","Not enough memory",4001);
        }
    }
    stmt->dummy_state= ST_DUMMY_EXECUTED;
    return(SQL_SUCCESS);
}

/*
  @type    : ODBC 1.0 API
  @purpose : returns the number of columns in a result set
*/

SQLRETURN SQL_API SQLNumResultCols(SQLHSTMT  hstmt, SQLSMALLINT FAR *pccol)
{
    SQLRETURN error;
    STMT FAR *stmt= (STMT FAR*) hstmt;

    MYODBCDbgEnter;

    if ( stmt->param_count > 0 && stmt->dummy_state == ST_DUMMY_UNKNOWN &&
         (stmt->state != ST_PRE_EXECUTED || stmt->state != ST_EXECUTED) )
    {
        if ( do_dummy_parambind(hstmt) != SQL_SUCCESS )
            MYODBCDbgReturnReturn(SQL_ERROR);
    }
    if ( (error= check_result(stmt)) != SQL_SUCCESS )
        MYODBCDbgReturnReturn(error);

    if ( !stmt->result )
        *pccol= 0;      /* Not a select */
    else
        *pccol= stmt->result->field_count;

    MYODBCDbgInfo( "columns: %d", *pccol );
    MYODBCDbgReturnReturn(SQL_SUCCESS);
}

/*
  @type    : ODBC 1.0 API
  @purpose : returns the result column name, type, column size, decimal
  digits, and nullabilityfor one column in the result set
*/

SQLRETURN SQL_API SQLDescribeCol( SQLHSTMT          hstmt, 
                                  SQLUSMALLINT      icol,
                                  SQLCHAR FAR *     szColName,
                                  SQLSMALLINT       cbColNameMax,
                                  SQLSMALLINT FAR * pcbColName,
                                  SQLSMALLINT FAR * pfSqlType,
                                  SQLULEN *         pnColumnSize,
                                  SQLSMALLINT FAR * pibScale,
                                  SQLSMALLINT FAR * pfNullable )
{
    SQLRETURN error;
    MYSQL_FIELD *field;
    STMT FAR *stmt= (STMT FAR*) hstmt;
    ulong transfer_length,precision,display_size;
    int type;

    MYODBCDbgEnter;

    if ( (error= check_result(stmt)) != SQL_SUCCESS )
        MYODBCDbgReturnReturn(error);
    if ( ! stmt->result )
        MYODBCDbgReturnReturn(set_stmt_error(stmt,"07005","No result set",0));

    mysql_field_seek(stmt->result,icol-1);
    if ( !(field= mysql_fetch_field(stmt->result)) )
        MYODBCDbgReturnReturn(set_error(stmt,MYERR_S1002,"Invalid column number",0));

    type= unireg_to_sql_datatype(stmt,field,0,&transfer_length,&precision,&display_size );
/*
printf( "[PAH][%s][%d][%s] type=%d\n", __FILE__, __LINE__, __FUNCTION__, type );
*/
    if ( pnColumnSize )
        *pnColumnSize = precision;
    if ( pfSqlType )
        *pfSqlType= type;
    if ( pibScale )
        *pibScale= field->decimals;
    if ( pfNullable )
        *pfNullable= (((field->flags & (NOT_NULL_FLAG)) ==
                       NOT_NULL_FLAG) ?
                      SQL_NO_NULLS :
                      SQL_NULLABLE);
    MYODBCDbgInfo( "col: %d", icol );
    MYODBCDbgInfo( "type: %d", type );
    MYODBCDbgInfo( "precision: %ld", precision );
    MYODBCDbgInfo( "decimals: %d", field->decimals );

    if ( stmt->dbc->flag & FLAG_FULL_COLUMN_NAMES && field->table )
    {
        char *tmp= my_malloc(strlen(field->name)+strlen(field->table)+2,
                             MYF(MY_WME));
        SQLRETURN error;
        if ( !tmp )
        {
            MYODBCDbgReturnReturn(set_error(stmt,MYERR_S1001,NULL,4001));
        }
        strxmov(tmp,field->table,".",field->name,NullS);
        error= copy_str_data(SQL_HANDLE_STMT, stmt, szColName,
                             cbColNameMax, pcbColName, tmp);
        my_free((gptr) tmp,MYF(0));
        MYODBCDbgReturnReturn(error);
    }
    MYODBCDbgReturnReturn(copy_str_data(SQL_HANDLE_STMT, stmt, szColName,
                                     cbColNameMax, pcbColName, field->name));
}

/*
  @type    : myodbc3 internal
  @purpose : rerunrs column atribute values
*/

SQLRETURN SQL_API
get_col_attr(SQLHSTMT     StatementHandle,
             SQLUSMALLINT ColumnNumber,
             SQLUSMALLINT FieldIdentifier,
             SQLPOINTER   CharacterAttributePtr,
             SQLSMALLINT  BufferLength,
             SQLSMALLINT  *StringLengthPtr,
             SQLPOINTER   NumericAttributePtr)
{
    MYSQL_FIELD *field;
    STMT FAR *stmt= (STMT FAR*) StatementHandle;
    SQLSMALLINT str_length;
    SQLPOINTER strparam= 0;
    SQLPOINTER nparam= 0;
    ulong transfer_length,precision,display_size;
    SQLRETURN error;

    MYODBCDbgEnter;
    MYODBCDbgInfo( "column: %d", ColumnNumber );
    MYODBCDbgInfo( "type: %d", FieldIdentifier );

    if ( check_result(stmt) != SQL_SUCCESS )
        MYODBCDbgReturnReturn(SQL_ERROR);

    if ( !stmt->result )
        MYODBCDbgReturnReturn(set_stmt_error(stmt,"07005","No result set",0));
#ifdef CHECK_EXTRA_ARGUMENTS
    if ( ColumnNumber > stmt->result->field_count )
        MYODBCDbgReturnReturn(set_error(StatementHandle, MYERR_07009,NULL,0));
#endif
    if ( !StringLengthPtr )
        StringLengthPtr= &str_length;

    if ( !CharacterAttributePtr )
        CharacterAttributePtr= nparam;

    if ( !NumericAttributePtr )
        NumericAttributePtr= strparam;

    if ( (error= check_result(stmt)) != SQL_SUCCESS )
        MYODBCDbgReturnReturn(error);

    if ( FieldIdentifier == SQL_DESC_COUNT ||
         FieldIdentifier == SQL_COLUMN_COUNT )
    {
        *(SQLINTEGER *)NumericAttributePtr= stmt->result->field_count;
        MYODBCDbgReturnReturn(SQL_SUCCESS);
    }
    if ( FieldIdentifier == SQL_DESC_TYPE && ColumnNumber == 0 )
    {
        *(SQLINTEGER *) NumericAttributePtr= SQL_INTEGER;
        MYODBCDbgReturnReturn(SQL_SUCCESS);
    }
    mysql_field_seek(stmt->result,ColumnNumber-1);
    if ( !(field= mysql_fetch_field(stmt->result)) )
        MYODBCDbgReturnReturn(set_error(stmt,MYERR_S1002,"Invalid column number",0));

    switch ( FieldIdentifier )
    {
        
        case SQL_DESC_AUTO_UNIQUE_VALUE:
            *(SQLINTEGER *)NumericAttributePtr= (field->flags & AUTO_INCREMENT_FLAG ?
                                                 SQL_TRUE : SQL_FALSE);
            break;

            /* We need support from server, when aliasing is there */
        case SQL_DESC_BASE_COLUMN_NAME:
        case SQL_DESC_LABEL:
        case SQL_DESC_NAME:
        case SQL_COLUMN_NAME:
            MYODBCDbgReturnReturn(copy_str_data(SQL_HANDLE_STMT, stmt,
                                             CharacterAttributePtr,
                                             BufferLength,StringLengthPtr,
                                             field->name));

        case SQL_DESC_BASE_TABLE_NAME:
        case SQL_DESC_TABLE_NAME:
            MYODBCDbgReturnReturn(copy_str_data(SQL_HANDLE_STMT, stmt,
                                             CharacterAttributePtr,
                                             BufferLength,StringLengthPtr,
                                             field->table ? field->table : ""));

        case SQL_DESC_CASE_SENSITIVE:
            *(SQLINTEGER *)NumericAttributePtr= (field->flags & BINARY_FLAG ?
                                                 SQL_FALSE : SQL_TRUE);
            break;

        case SQL_DESC_CATALOG_NAME:
#if MYSQL_VERSION_ID < 40100
            MYODBCDbgReturnReturn(copy_str_data(SQL_HANDLE_STMT, stmt,
                                             CharacterAttributePtr,
                                             BufferLength, StringLengthPtr,
                                             stmt->dbc->database));
#else
            {
                char *ldb=  (field->db && field->db[0] != '\0') ? field->db : stmt->dbc->database;      
                MYODBCDbgReturnReturn(copy_str_data(SQL_HANDLE_STMT, stmt,
                                                 CharacterAttributePtr,
                                                 BufferLength, StringLengthPtr,
                                                 ldb));
            }
#endif
            break;

        case SQL_DESC_DISPLAY_SIZE:
            (void) unireg_to_sql_datatype(stmt,field,0,&transfer_length,&precision,
                                          &display_size);
            *(SQLINTEGER *)NumericAttributePtr= display_size;
            break;

        case SQL_DESC_FIXED_PREC_SCALE: /* need to verify later */
            {
                if ( field->type == MYSQL_TYPE_DECIMAL )
                    *(SQLINTEGER *)NumericAttributePtr= SQL_TRUE;
                else
                    *(SQLINTEGER *)NumericAttributePtr= SQL_FALSE;
                break;
            }

        case SQL_COLUMN_LENGTH:
        case SQL_DESC_LENGTH:
        case SQL_DESC_OCTET_LENGTH: /* need to validate again for octet length..*/
            (void) unireg_to_sql_datatype(stmt,field,0,&transfer_length,&precision,
                                          &display_size);
            *(SQLINTEGER *)NumericAttributePtr= transfer_length;
            break;

        case SQL_DESC_LITERAL_PREFIX:
        case SQL_DESC_LITERAL_SUFFIX:
            switch ( field->type )
            {
                case MYSQL_TYPE_LONG_BLOB:
                case MYSQL_TYPE_TINY_BLOB:
                case MYSQL_TYPE_MEDIUM_BLOB:
                case MYSQL_TYPE_BLOB:
                    MYODBCDbgReturnReturn(copy_str_data(SQL_HANDLE_STMT, stmt,
                                                     CharacterAttributePtr,
                                                     BufferLength,StringLengthPtr,"0x"));

                case MYSQL_TYPE_DATE:
                case MYSQL_TYPE_DATETIME:
                case MYSQL_TYPE_NEWDATE:
                case MYSQL_TYPE_VAR_STRING:
                case MYSQL_TYPE_STRING:
                case MYSQL_TYPE_TIMESTAMP:
                case MYSQL_TYPE_TIME:
                case MYSQL_TYPE_YEAR:
                    MYODBCDbgReturnReturn(copy_str_data(SQL_HANDLE_STMT, stmt,
                                                     CharacterAttributePtr,
                                                     BufferLength,StringLengthPtr,"'"));

                default:
                    MYODBCDbgReturnReturn(copy_str_data(SQL_HANDLE_STMT, stmt,
                                                     CharacterAttributePtr,
                                                     BufferLength,StringLengthPtr,""));
            }
            break;

        case SQL_DESC_NULLABLE:
        case SQL_COLUMN_NULLABLE:
            *(SQLINTEGER *)NumericAttributePtr= (((field->flags &
                                                   (NOT_NULL_FLAG)) ==
                                                  NOT_NULL_FLAG) ?
                                                 SQL_NO_NULLS :
                                                 SQL_NULLABLE);
            break;

        case SQL_DESC_NUM_PREC_RADIX:
            switch ( field->type )
            {
                case MYSQL_TYPE_SHORT:
                case MYSQL_TYPE_LONG:
                case MYSQL_TYPE_LONGLONG:
                case MYSQL_TYPE_INT24:
                case MYSQL_TYPE_TINY:
                case MYSQL_TYPE_DECIMAL:
                    *(SQLINTEGER *)NumericAttributePtr= 10;
                    break;

                case MYSQL_TYPE_FLOAT:
                case MYSQL_TYPE_DOUBLE:
                    *(SQLINTEGER *) NumericAttributePtr= 2;
                    break;

                default:
                    *(SQLINTEGER *)NumericAttributePtr= 0;
                    break;
            }
            break;

        case SQL_COLUMN_PRECISION:
        case SQL_DESC_PRECISION:
            (void) unireg_to_sql_datatype(stmt,field,0,&transfer_length,&precision,
                                          &display_size);
            *(SQLINTEGER *)NumericAttributePtr= precision;
            break;

        case SQL_COLUMN_SCALE:
        case SQL_DESC_SCALE:
            *(SQLINTEGER *)NumericAttributePtr= field->decimals;
            break;

        case SQL_DESC_SCHEMA_NAME:
            MYODBCDbgReturnReturn(copy_str_data(SQL_HANDLE_STMT, stmt,
                                             CharacterAttributePtr,
                                             BufferLength,StringLengthPtr, ""));

        case SQL_DESC_SEARCHABLE:
            *(SQLINTEGER *)NumericAttributePtr= SQL_SEARCHABLE;
            break;

        case SQL_DESC_TYPE:
        case SQL_DESC_CONCISE_TYPE:
            *(SQLINTEGER *)NumericAttributePtr =
            unireg_to_sql_datatype(stmt, field, 0, &transfer_length, &precision,
                                   &display_size);
            break;

        case SQL_DESC_TYPE_NAME:
            {
                char buff[40];
                (void)unireg_to_sql_datatype(stmt,field,(char *)buff,&transfer_length,
                                             &precision, &display_size);
                MYODBCDbgReturnReturn(copy_str_data(SQL_HANDLE_STMT, stmt,
                                                 CharacterAttributePtr,
                                                 BufferLength,StringLengthPtr, buff));
            }

        case SQL_DESC_UNNAMED:
            *(SQLINTEGER *)NumericAttributePtr= SQL_NAMED;
            break;

        case SQL_DESC_UNSIGNED:
            *(SQLINTEGER *) NumericAttributePtr= (field->flags & UNSIGNED_FLAG ?
                                                  SQL_TRUE : SQL_FALSE);
            break;

        case SQL_DESC_UPDATABLE:
            *(SQLINTEGER *)NumericAttributePtr= (field->table && field->table[0] ?
                                                 SQL_ATTR_READWRITE_UNKNOWN :
                                                 SQL_ATTR_READONLY);
            break;

            /*
              Hack : Fix for the error from ADO 'rs.resync' "Key value for this
              row was changed or deleted at the data store.  The local
              row is now deleted. This should also fix some Multi-step
              generated error cases from ADO
            */
        case SQL_MY_PRIMARY_KEY: /* MSSQL extension !! */
            *(SQLINTEGER *)NumericAttributePtr= (field->flags & PRI_KEY_FLAG ?
                                                 SQL_TRUE : SQL_FALSE);
            break;

        default:
            MYODBCDbgWarning(  "Type: %d is not supported by the driver", FieldIdentifier );
    }
    MYODBCDbgReturnReturn(SQL_SUCCESS);
}


/*
  Retrieve an attribute of a column in a result set.

  @param[in]  hstmt      Handle to statement
  @param[in]  icol       The column to retrieve data for, indexed from 1
  @param[in]  fDescType  The attribute to be retrieved
  @param[out] rgbDesc    Pointer to buffer in which to return data
  @param[in]  cbDescMax  Length of @a rgbDesc in bytes
  @param[out] pcbDesc    Pointer to integer to return the total number of bytes
                         available to be returned in @a rgbDesc
  @param[out] pfDesc     Pointer to an integer to return the value if the
                         @a fDescType corresponds to a numeric type

  @since ODBC 1.0
*/
SQLRETURN SQL_API SQLColAttribute(SQLHSTMT     hstmt,
                                  SQLUSMALLINT icol,
                                  SQLUSMALLINT fDescType,
                                  SQLPOINTER   rgbDesc,
                                  SQLSMALLINT  cbDescMax,
                                  SQLSMALLINT *pcbDesc,
#ifdef USE_SQLCOLATTRIBUTE_SQLLEN_PTR
                                  SQLLEN      *pfDesc
#else
                                  SQLPOINTER   pfDesc
#endif
                                 )
{
  SQLRETURN  rc;
  SQLINTEGER num;

  MYODBCDbgEnter;

  /*
   get_col_attr() expects an SQLINTEGER * for its last argument, but sometimes
   this function can use SQLLEN *.
  */
  rc= get_col_attr(hstmt, icol, fDescType, rgbDesc, cbDescMax, pcbDesc, &num);
  if (pfDesc)
  {
#ifdef USE_SQLCOLATTRIBUTE_SQLLEN_PTR
    *pfDesc= num;
#else
    *(SQLINTEGER *)pfDesc= num;
#endif
  }

  MYODBCDbgReturnReturn(rc);
}


/*
  Retrieve an attribute of a column in a result set.

  @deprecated This function is deprecated. SQLColAttribute() should be used
  instead.

  @param[in]  hstmt      Handle to statement
  @param[in]  icol       The column to retrieve data for, indexed from 1
  @param[in]  fDescType  The attribute to be retrieved
  @param[out] rgbDesc    Pointer to buffer in which to return data
  @param[in]  cbDescMax  Length of @a rgbDesc in bytes
  @param[out] pcbDesc    Pointer to integer to return the total number of bytes
                         available to be returned in @a rgbDesc
  @param[out] pfDesc     Pointer to an integer to return the value if the
                         @a fDescType corresponds to a numeric type

  @since ODBC 1.0
*/
SQLRETURN SQL_API SQLColAttributes(SQLHSTMT     hstmt,
                                   SQLUSMALLINT icol,
                                   SQLUSMALLINT fDescType,
                                   SQLPOINTER   rgbDesc,
                                   SQLSMALLINT  cbDescMax,
                                   SQLSMALLINT *pcbDesc,
                                   SQLLEN      *pfDesc)
{
  SQLRETURN  rc;
  SQLINTEGER num;

  MYODBCDbgEnter;

  /*
   get_col_attr() is built for SQLColAttribute(), which uses an SQLINTEGER *
   for its last argument, but this legacy function uses SQLLEN *.
  */
  rc= get_col_attr(hstmt, icol, fDescType, rgbDesc, cbDescMax, pcbDesc, &num);
  if (pfDesc)
    *pfDesc= num;

  MYODBCDbgReturnReturn(rc);
}


/*
  @type    : ODBC 1.0 API
  @purpose : binds application data buffers to columns in the result set
*/

SQLRETURN SQL_API SQLBindCol( SQLHSTMT      hstmt, 
                              SQLUSMALLINT  icol,
                              SQLSMALLINT   fCType, 
                              SQLPOINTER    rgbValue,
                              SQLLEN        cbValueMax, 
                              SQLLEN *      pcbValue )
{
    BIND *bind;
    STMT FAR *stmt= (STMT FAR*) hstmt;
    SQLRETURN error;

    MYODBCDbgEnter;
    MYODBCDbgInfo( "icol: %d", icol );
    MYODBCDbgInfo( "Type: %d", fCType );
    MYODBCDbgInfo( "ValueMax: %ld", cbValueMax );
    MYODBCDbgInfo( "Valueptr: 0x%lx", (ulong)pcbValue );
    MYODBCDbgInfo( "pcbValue: %ld", (long) (pcbValue ? *pcbValue : 0L) );

    icol--;
    /*
      The next case if because of VB 5.0 that binds columns before preparing
      a statement
    */

    if ( stmt->state == ST_UNKNOWN )
    {
        MYODBCDbgInfo( "%s", "Binding columns without a statement; Hope you know what you are doing" );
        if ( fCType == SQL_C_NUMERIC ) /* We don't support this */
        {
            set_error(stmt,MYERR_07006,
                      "Restricted data type attribute violation(SQL_C_NUMERIC)",0);
            MYODBCDbgReturnReturn(SQL_ERROR);
        }
        if ( icol >= stmt->bound_columns )
        {
            if ( !(stmt->bind= (BIND*) my_realloc((char*) stmt->bind,
                                                  (icol+1)*sizeof(BIND),
                                                  MYF(MY_ALLOW_ZERO_PTR |
                                                      MY_FREE_ON_ERROR))) )
            {
                stmt->bound_columns= 0;
                MYODBCDbgReturnReturn(set_error(stmt,MYERR_S1001,NULL,4001));
            }
            bzero((gptr) (stmt->bind+stmt->bound_columns),
                  (icol+1-stmt->bound_columns)*sizeof(BIND));
            stmt->bound_columns= icol+1;
        }
    }
    else
    {
        /* Bind parameter to current set  (The normal case) */
        /* select stmt with parameters */
        if ( stmt->param_count > 0 && stmt->dummy_state == ST_DUMMY_UNKNOWN &&
             (stmt->state != ST_PRE_EXECUTED || stmt->state != ST_EXECUTED) )
        {
            if ( do_dummy_parambind(hstmt) != SQL_SUCCESS )
                MYODBCDbgReturnReturn(SQL_ERROR);
        }
        if ( fCType == SQL_C_NUMERIC ) /* We don't support this */
        {
            set_error(stmt,MYERR_07006,
                      "Restricted data type attribute violation(SQL_C_NUMERIC)",0);
            MYODBCDbgReturnReturn(SQL_ERROR);
        }
        if ( (error= check_result(stmt)) != SQL_SUCCESS )
            MYODBCDbgReturnReturn(error);

        if ( !stmt->result || (uint) icol >= stmt->result->field_count )
        {
            set_error(stmt,MYERR_S1002,"Invalid column number",0);
            MYODBCDbgReturnReturn(error);
        }
        if ( !stmt->bind )
        {
            if ( !(stmt->bind= (BIND*) my_malloc(sizeof(BIND)*
                                                 stmt->result->field_count,
                                                 MYF(MY_ZEROFILL))) )
                MYODBCDbgReturnReturn(set_error(stmt,MYERR_S1001,NULL,4001));
            stmt->bound_columns= stmt->result->field_count;
        }
        mysql_field_seek(stmt->result,icol);
        stmt->bind[icol].field= mysql_fetch_field(stmt->result);
    }
    bind= stmt->bind+icol;
    bind->fCType= fCType;
    if ( fCType == SQL_C_DEFAULT && stmt->odbc_types )
        bind->fCType= stmt->odbc_types[icol];
    bind->rgbValue= rgbValue;
    bind->cbValueMax= bind_length(bind->fCType,cbValueMax);
    bind->pcbValue= pcbValue;
    MYODBCDbgReturnReturn(SQL_SUCCESS);
}


/*
  @type    : myodbc3 internal
  @purpose : returns the latest resultset(dynamic)
*/

my_bool set_dynamic_result(STMT FAR *stmt)
{
    if ( odbc_stmt(stmt->dbc, stmt->query) != SQL_SUCCESS )
        return 1;

    pthread_mutex_lock(&stmt->dbc->lock);
    x_free((gptr) stmt->odbc_types);
    if (!stmt->fake_result)
      mysql_free_result(stmt->result);
    else
      x_free((gptr)stmt->result);
    stmt->result= 0;
    stmt->fake_result= 0;
    stmt->odbc_types= 0;
    stmt->cursor_row= 0;
    stmt->result= mysql_store_result(&stmt->dbc->mysql);
    if ( !stmt->result )
    {
        set_error(stmt,MYERR_S1000,mysql_error(&stmt->dbc->mysql),
                  mysql_errno(&stmt->dbc->mysql));
        pthread_mutex_unlock(&stmt->dbc->lock);
        return 1;
    }
    fix_result_types(stmt);
    set_current_cursor_data(stmt,0);
    pthread_mutex_unlock(&stmt->dbc->lock);
    return 0;
}


/*
  @type    : ODBC 1.0 API
  @purpose : retrieves data for a single column in the result set. It can
  be called multiple times to retrieve variable-length data
  in parts
*/

SQLRETURN SQL_API SQLGetData( SQLHSTMT      hstmt,
                              SQLUSMALLINT  icol,
                              SQLSMALLINT   fCType,
                              SQLPOINTER    rgbValue,
                              SQLLEN        cbValueMax, 
                              SQLLEN *      pcbValue )
{
    STMT FAR *stmt= (STMT FAR*) hstmt;
    SQLRETURN result;

    MYODBCDbgEnter;

    MYODBCDbgInfo( "icol:%d", icol );
    MYODBCDbgInfo( "ctype:%d", fCType );
    MYODBCDbgInfo( "rgb:0x%x", (uint)rgbValue );
    MYODBCDbgInfo( "len:%d", (int)cbValueMax );
    MYODBCDbgInfo( "pcb:0x%x", (uint)pcbValue );

    if ( !stmt->result || !stmt->current_values )
    {
        set_stmt_error(stmt,"24000","SQLGetData without a preceding SELECT",0);
        MYODBCDbgReturnReturn(SQL_ERROR);
    }
    if ( fCType == SQL_C_NUMERIC ) /* We don't support this */
    {
        set_error(stmt,MYERR_07006,
                  "Restricted data type attribute violation(SQL_C_NUMERIC)",0);
        MYODBCDbgReturnReturn(SQL_ERROR);
    }
    icol--;     /* Easier code if start from 0 */
    if ( icol != stmt->last_getdata_col )
    {   /* New column. Reset old offset */
        stmt->last_getdata_col= icol;
        stmt->getdata_offset= (ulong) ~0L;
    }

#ifdef LOG_ALL
    MYODBCDbgInfo( "icol: %d", icol+1 );
    MYODBCDbgInfo( "fCType: %d", fCType );
    MYODBCDbgInfo( "default: %d", stmt->odbc_types[icol] );
    MYODBCDbgInfo( "value: %.10s", (stmt->current_values[icol] ? stmt->current_values[icol] : "NULL") );
#endif
    if ( !(stmt->dbc->flag & FLAG_NO_LOCALE) )
        setlocale(LC_NUMERIC,"English");
    result= sql_get_data( stmt,
                          (SQLSMALLINT) (fCType == SQL_C_DEFAULT ? stmt->odbc_types[icol] : fCType),
                          stmt->result->fields+icol,
                          rgbValue,
                          cbValueMax,
                          pcbValue,
                          stmt->current_values[icol],
                          (stmt->result_lengths ? stmt->result_lengths[icol] : (stmt->current_values[icol] ? strlen( stmt->current_values[icol] ) : 0 ) ) );

    if ( !(stmt->dbc->flag & FLAG_NO_LOCALE) )
        setlocale(LC_NUMERIC,default_locale);

    MYODBCDbgInfo( "return:%d ", result );
    MYODBCDbgInfo( "rgb:0x%x", (uint)rgbValue );
    MYODBCDbgInfo( "pcb:0x%x", (uint)pcbValue );

    MYODBCDbgReturnReturn(result);
}


/*
  @type    : myodbc3 internal
  @purpose : returns the column data
*/

SQLRETURN SQL_API sql_get_data( STMT *          stmt,
                                SQLSMALLINT     fCType,
                                MYSQL_FIELD *   field,
                                SQLPOINTER      rgbValue,
                                SQLINTEGER      cbValueMax,
                                SQLLEN *        pcbValue,
                                char *          value,
                                uint            length )
{
    SQLLEN tmp;
    if ( !pcbValue )
        pcbValue= &tmp; /* Easier code */

    if ( !value )
        *pcbValue= SQL_NULL_DATA;
    else
    {
        switch ( fCType )
        {
            case SQL_C_CHAR:
                /* Handle BLOB -> CHAR conversion */
                if ( (field->flags & (BLOB_FLAG+BINARY_FLAG)) == (BLOB_FLAG+BINARY_FLAG) )
                    return copy_binary_result( SQL_HANDLE_STMT, 
                                               stmt,
                                               (SQLCHAR FAR*) rgbValue,
                                               cbValueMax,
                                               pcbValue,
                                               value, 
                                               length, 
                                               stmt->stmt_options.max_length,
                                               &stmt->getdata_offset );
                /* fall through */
            case SQL_C_BINARY:
                {
                    char buff[21];
                    if ( field->type == MYSQL_TYPE_TIMESTAMP && length != 19 )
                    {
                        /* MySQL doesn't have '-' in timestamp */
                        /* Convert timestamp to ANSI format */

                        char *pos;
                        uint i;
                        if ( length == 6 || length == 10 || length == 12 )
                        {
                            /* YYMMDD or YYMMDDHHMMSS timestamp */
                            if ( value[0] <= '6' )
                            {
                                buff[0]='2';
                                buff[1]='0';
                            }
                            else
                            {
                                buff[0]='1';
                                buff[1]='9';
                            }
                        }
                        else
                        {
                            buff[0]= value[0];
                            buff[1]= value[1];
                            value+= 2;
                            length-= 2;
                        }
                        buff[2]= *value++;
                        buff[3]= *value++;
                        buff[4]='-';
                        if ( value[0] == '0' && value[1] == '0' )
                        {
                            *pcbValue= SQL_NULL_DATA;  /* ODBC can't handle 0000-00-00 dates */
                            break;
                        }
                        pos= buff+5;
                        length&= 30;  /* Ensure that length is ok */
                        for ( i= 1, length-= 2 ; (int) length > 0 ; length-= 2,i++ )
                        {
                            *pos++= *value++;
                            *pos++= *value++;
                            *pos++= i < 2 ? '-' : (i == 2) ? ' ' : ':';
                        }
                        for ( ; pos != buff+20 ; i++ )
                        {
                            *pos++= '0';
                            *pos++= '0';
                            *pos++= i < 2 ? '-' : (i == 2) ? ' ' : ':';
                        }
                        value= buff;
                        length= 19;
                    }
                    return copy_lresult(SQL_HANDLE_STMT, stmt,
                                        (SQLCHAR FAR *) rgbValue,cbValueMax,pcbValue,value,
                                        length,stmt->stmt_options.max_length,
                                        (field->type == MYSQL_TYPE_STRING ? field->length :
                                         0L),
                                        &stmt->getdata_offset,
                                        (my_bool) (fCType == SQL_C_BINARY));
                }
            case SQL_C_BIT:
                if ( rgbValue )
                    *((char*) rgbValue)= (value[0] == 1 ? 1 : (atoi(value) == 0) ? 0 : 1);
                *pcbValue= 1;
                break;
            case SQL_C_TINYINT:
            case SQL_C_STINYINT:
                if ( rgbValue )
                    *((char*) rgbValue)= ((char) atoi(value));
                *pcbValue= 1;
                break;
            case SQL_C_UTINYINT:
                if ( rgbValue )
                    *((uchar*) rgbValue)= ((uchar) (uint) atoi(value));
                *pcbValue= 1;
                break;
            case SQL_C_SHORT:
            case SQL_C_SSHORT:
                if ( rgbValue )
                    *((short*) rgbValue)= (short) atoi(value);
                *pcbValue= sizeof(short);
                break;
            case SQL_C_USHORT:
                if ( rgbValue )
                    *((ushort*) rgbValue)= (ushort) (uint) atol(value);
                *pcbValue= sizeof(short);
                break;
            case SQL_C_LONG:
            case SQL_C_SLONG:
                if ( rgbValue )
                {
                    /*
                      Check if it could be a date...... :)
                    */
                    if ( length >= 10 && value[4] == '-' && value[7] == '-' &&
                         (!value[10] || value[10] == ' ') )
                    {
                        *((SQLINTEGER*) rgbValue)= ((SQLINTEGER) atol(value)*10000L+
                                              (SQLINTEGER) atol(value+5)*100L+
                                              (SQLINTEGER) atol(value+8));
                    }
                    else
                        *((SQLINTEGER*) rgbValue)= (SQLINTEGER) atol(value);
                }
                *pcbValue= sizeof(SQLINTEGER);
                break;
            case SQL_C_ULONG:
                if ( rgbValue )
                {
                    *((SQLUINTEGER*) rgbValue)= (SQLUINTEGER)atol(value);
                }
                *pcbValue= sizeof(SQLUINTEGER);
                break;
            case SQL_C_FLOAT:
                if ( rgbValue )
                    *((float*) rgbValue)= (float) atof(value);
                *pcbValue= sizeof(float);
                break;
            case SQL_C_DOUBLE:
                if ( rgbValue )
                    *((double*) rgbValue)= (double) atof(value);
                *pcbValue= sizeof(double);
                break;
            case SQL_C_DATE:
            case SQL_C_TYPE_DATE:
                {
                    SQL_DATE_STRUCT tmp_date;
                    if ( !rgbValue )
                        rgbValue=(char*) &tmp_date;
                    if ( !str_to_date((SQL_DATE_STRUCT *) rgbValue,value,length) )
                        *pcbValue=sizeof(SQL_DATE_STRUCT);
                    else
                    {
                        *pcbValue=SQL_NULL_DATA;  /* ODBC can't handle 0000-00-00 dates */
                    }
                    break;
                }
            case SQL_C_TIME:
            case SQL_C_TYPE_TIME:
                {
                    if (field->type == MYSQL_TYPE_TIMESTAMP ||
                        field->type == MYSQL_TYPE_DATETIME)
                    {
                      SQL_TIMESTAMP_STRUCT ts;
                      if ( str_to_ts(&ts,value) )
                          *pcbValue= SQL_NULL_DATA;
                      else
                      {
                          SQL_TIME_STRUCT *time_info= (SQL_TIME_STRUCT *)rgbValue;

                          if ( time_info )
                          {
                              time_info->hour= ts.hour;
                              time_info->minute= ts.minute;
                              time_info->second= ts.second;
                          }
                          *pcbValue=sizeof(TIME_STRUCT);
                      }
                    }
                    else if (field->type == MYSQL_TYPE_DATE)
                    {
                          SQL_TIME_STRUCT *time_info= (SQL_TIME_STRUCT *)rgbValue;

                          if ( time_info )
                          {
                              time_info->hour= 0;
                              time_info->minute= 0;
                              time_info->second= 0;
                          }
                          *pcbValue=sizeof(TIME_STRUCT);
                    }
                    else
                    {
                      SQL_TIME_STRUCT ts;
                      if ( str_to_time_st(&ts,value) )
                          *pcbValue= SQL_NULL_DATA;
                      else
                      {
                          SQL_TIME_STRUCT *time_info= (SQL_TIME_STRUCT *)rgbValue;

                          if ( time_info )
                          {
                              time_info->hour= ts.hour;
                              time_info->minute= ts.minute;
                              time_info->second= ts.second;          
                          }
                          *pcbValue=sizeof(TIME_STRUCT);
                      }
                    }
                    break;
                }
            case SQL_C_TIMESTAMP:
            case SQL_C_TYPE_TIMESTAMP:
                {
                    if (field->type == MYSQL_TYPE_TIME)
                    {
                        SQL_TIME_STRUCT ts;

                        if ( str_to_time_st(&ts,value) )
                            *pcbValue= SQL_NULL_DATA;
                        else
                        {
                            SQL_TIMESTAMP_STRUCT *timestamp_info= (SQL_TIMESTAMP_STRUCT *)rgbValue;
                            time_t sec_time= time(NULL);
                            struct tm cur_tm;
                            localtime_r(&sec_time, &cur_tm);

                            timestamp_info->year= 1900 + cur_tm.tm_year;
                            timestamp_info->month= 1 + cur_tm.tm_mon; /* January is 0 in tm */
                            timestamp_info->day= cur_tm.tm_mday;
                            timestamp_info->hour= ts.hour;
                            timestamp_info->minute= ts.minute;
                            timestamp_info->second= ts.second;
                            timestamp_info->fraction= 0;
                            *pcbValue= sizeof(SQL_TIMESTAMP_STRUCT);      
                        }
                    } 
                    else
                    {
                        if ( str_to_ts((SQL_TIMESTAMP_STRUCT *)rgbValue, value) )
                            *pcbValue= SQL_NULL_DATA;
                        else
                            *pcbValue= sizeof(SQL_TIMESTAMP_STRUCT);      
                    }
                    break;

                }

            case SQL_C_SBIGINT:
                {
                    if ( rgbValue )
                        *((longlong*) rgbValue)= (longlong) strtoll(value,NULL,10);
                    *pcbValue= sizeof(longlong);
                    break;
                }
            case SQL_C_UBIGINT:
                {
                    if ( rgbValue )
                        *((ulonglong*) rgbValue)= (ulonglong)strtoull(value,NULL,10);
                    *pcbValue= sizeof(ulonglong);
                    break;
                }
        }
    }

    if ( stmt->getdata_offset != (ulong) ~0L )  /* Second call to getdata */
        return SQL_NO_DATA_FOUND;
    stmt->getdata_offset= 0L;         /* All data is retrevied */
    return SQL_SUCCESS;
}


/*
  @type    : ODBC 1.0 API
  @purpose : determines whether more results are available on a statement
  containing SELECT, UPDATE, INSERT, or DELETE statements and,
  if so, initializes processing for those results
*/

SQLRETURN SQL_API SQLMoreResults( SQLHSTMT hStmt )
{
    STMT FAR *  pStmt   = (STMT FAR*)hStmt;
    int         nRetVal;
    SQLRETURN   nReturn = SQL_SUCCESS;

    MYODBCDbgEnter;

    pthread_mutex_lock( &pStmt->dbc->lock );

    CLEAR_STMT_ERROR( pStmt );

    /* SQLExecute or SQLExecDirect need to be called first */
    if ( pStmt->state != ST_EXECUTED )
    {
        nReturn = set_stmt_error( pStmt, "HY010", NULL, 0 );
        goto exitSQLMoreResults;
    }

    /* try to get next resultset */
    nRetVal = mysql_next_result( &pStmt->dbc->mysql );

    /* call to mysql_next_result() failed */
    if ( nRetVal > 0 )
    {
        nRetVal = mysql_errno( &pStmt->dbc->mysql );
        switch ( nRetVal )
        {
            case CR_SERVER_GONE_ERROR:
            case CR_SERVER_LOST:
                nReturn = set_stmt_error( pStmt, "08S01", mysql_error( &pStmt->dbc->mysql ), nRetVal );
                goto exitSQLMoreResults;
            case CR_COMMANDS_OUT_OF_SYNC:
            case CR_UNKNOWN_ERROR:
                nReturn = set_stmt_error( pStmt, "HY000", mysql_error( &pStmt->dbc->mysql ), nRetVal );
                goto exitSQLMoreResults;
            default:
                nReturn = set_stmt_error( pStmt, "HY000", "unhandled error from mysql_next_result()", nRetVal );
                goto exitSQLMoreResults;
        }
    }

    /* no more resultsets */
    if ( nRetVal < 0 )
    {
        nReturn = SQL_NO_DATA;
        goto exitSQLMoreResults;
    }

    /* cleanup existing resultset */
    nReturn = my_SQLFreeStmt( (SQLHSTMT)pStmt, SQL_CLOSE );
    if ( !SQL_SUCCEEDED( nReturn ) )
        goto exitSQLMoreResults;

    /* start using the new resultset */
    if ( if_forward_cache( pStmt ) )
        pStmt->result = mysql_use_result( &pStmt->dbc->mysql );
    else
        pStmt->result = mysql_store_result( &pStmt->dbc->mysql );

    if ( !pStmt->result )
    {
        /* no fields means; INSERT, UPDATE or DELETE so no resultset is fine */
        if ( !mysql_field_count( &pStmt->dbc->mysql ) )
        {
            pStmt->state = ST_EXECUTED;
            pStmt->affected_rows = mysql_affected_rows( &pStmt->dbc->mysql );
            MYODBCDbgInfo( "affected rows: %d", (int)pStmt->affected_rows );
            goto exitSQLMoreResults;
        }
        /* we have fields but no resultset (not even an empty one) - this is bad */
        MYODBCDbgError( "%s", "client failed to return resultset" );
        nReturn = set_stmt_error( pStmt, "HY000", mysql_error( &pStmt->dbc->mysql ), mysql_errno( &pStmt->dbc->mysql ) );
        goto exitSQLMoreResults;
    }

    MYODBCDbgInfo( "result set columns: %d", pStmt->result->field_count );
    MYODBCDbgInfo( "result set rows: %lld", pStmt->result->row_count );
    fix_result_types( pStmt );

exitSQLMoreResults:
    pthread_mutex_unlock( &pStmt->dbc->lock );
    MYODBCDbgReturnReturn( nReturn );
}


/*
  @type    : ODBC 1.0 API
  @purpose : returns the number of rows affected by an UPDATE, INSERT,
  or DELETE statement;an SQL_ADD, SQL_UPDATE_BY_BOOKMARK,
  or SQL_DELETE_BY_BOOKMARK operation in SQLBulkOperations;
  or an SQL_UPDATE or SQL_DELETE operation in SQLSetPos
*/

SQLRETURN SQL_API SQLRowCount( SQLHSTMT hstmt, 
                               SQLLEN * pcrow )
{
    STMT FAR *stmt= (STMT FAR*) hstmt;

    MYODBCDbgEnter;

    if ( stmt->result )
    {
        *pcrow= (SQLLEN) mysql_affected_rows(&stmt->dbc->mysql);
        MYODBCDbgInfo( "Rows in set: %ld", *pcrow );
    }
    else
    {
        *pcrow= (SQLLEN) stmt->affected_rows;
        MYODBCDbgInfo( "Affected rows: %ld", *pcrow );
    }
    MYODBCDbgReturnReturn(SQL_SUCCESS);
}


/*
  @type    : myodbc3 internal
  @purpose : fetches the specified rowset of data from the result set and
  returns data for all bound columns. Rowsets can be specified
  at an absolute or relative position
*/
SQLRETURN SQL_API my_SQLExtendedFetch( SQLHSTMT             hstmt,
                                       SQLUSMALLINT         fFetchType,
                                       SQLROWOFFSET         irow,
                                       SQLROWSETSIZE       *pcrow,
                                       SQLUSMALLINT FAR    *rgfRowStatus,
                                       bool                 upd_status )
{
    ulong rows_to_fetch;
    long cur_row, max_row;
    uint i;
    SQLRETURN res,tmp_res;
    STMT FAR *stmt= (STMT FAR*) hstmt;
    MYSQL_ROW values= 0;
    MYSQL_ROW_OFFSET save_position;
    SQLROWSETSIZE dummy_pcrow;

    MYODBCDbgEnter;

    LINT_INIT(save_position);

    if ( !stmt->result )
        MYODBCDbgReturnReturn(set_stmt_error(stmt,"24000","Fetch without a SELECT",
                                          0));
    MYODBCDbgInfo( "fetchtype: %d", fFetchType );
    MYODBCDbgInfo( "row: %ld", irow );
    MYODBCDbgInfo( "current top-row: %ld", stmt->current_row );
    MYODBCDbgInfo( "rows_found: %ld", stmt->rows_found_in_set );

    if ( stmt->stmt_options.cursor_type == SQL_CURSOR_FORWARD_ONLY )
    {
        if ( fFetchType != SQL_FETCH_NEXT && !(stmt->dbc->flag & FLAG_SAFE) )
            MYODBCDbgReturnReturn( set_error(stmt,MYERR_S1106,
                                         "Wrong fetchtype with FORWARD ONLY cursor",
                                         0));

        if ((stmt->dbc->flag & FLAG_NO_CACHE))
        {
          if (stmt->result_array)
            values= stmt->result_array + (cur_row * stmt->result->field_count);
          else
            values= mysql_fetch_row(stmt->result);
        }
    }

    if ( if_dynamic_cursor(stmt) && set_dynamic_result(stmt) )
        MYODBCDbgReturnReturn(set_error(stmt,MYERR_S1000,
                                     "Driver Failed to set the internal dynamic result",
                                     0));

    if ( !pcrow )
        pcrow= &dummy_pcrow;

    max_row= (long) mysql_num_rows(stmt->result);
    stmt->last_getdata_col= (uint)  ~0;
    stmt->current_values= 0;          /* For SQLGetData */

    switch ( fFetchType )
    {
        
        case SQL_FETCH_NEXT:
            cur_row= (stmt->current_row < 0 ? 0 :
                      stmt->current_row+stmt->rows_found_in_set);
            break;
        case SQL_FETCH_PRIOR:
            cur_row= (stmt->current_row <= 0 ? -1 :
                      (long) (stmt->current_row - stmt->stmt_options.rows_in_set));
            break;
        case SQL_FETCH_FIRST:
            cur_row= 0L;
            break;
        case SQL_FETCH_LAST:
            cur_row= max_row-stmt->stmt_options.rows_in_set;
            break;
        case SQL_FETCH_ABSOLUTE:
            if ( irow < 0 )
            {
                /* Fetch from end of result set */
                if ( max_row+irow < 0 && -irow <= (long) stmt->stmt_options.rows_in_set )
                {
                    /*
                      | FetchOffset | > LastResultRow AND
                      | FetchOffset | <= RowsetSize
                    */
                    cur_row= 0;     /* Return from beginning */
                }
                else
                    cur_row= max_row+irow;     /* Ok if max_row <= -irow */
            }
            else
                cur_row= (long) irow-1;
            break;

        case SQL_FETCH_RELATIVE:
            cur_row= stmt->current_row + irow;
            if ( stmt->current_row > 0 && cur_row < 0 &&
                 (long) -irow <= (long)stmt->stmt_options.rows_in_set )
                cur_row= 0;
            break;

        default:
            MYODBCDbgReturnReturn( set_error( stmt, MYERR_S1106, "Fetch type out of range", 0 ) );
    }

    if ( cur_row < 0 )
    {
        stmt->current_row= -1;  /* Before first row */
        stmt->rows_found_in_set= 0;
        mysql_data_seek(stmt->result,0L);
        MYODBCDbgReturnReturn(SQL_NO_DATA_FOUND);
    }
    if ( cur_row > max_row )
        cur_row= max_row;

    if ( !stmt->result_array && !if_forward_cache(stmt) )
    {
        /*
          If Dynamic, it looses the stmt->end_of_set, so
          seek to desired row, might have new data or
          might be deleted
        */
        if ( stmt->stmt_options.cursor_type != SQL_CURSOR_DYNAMIC &&
             cur_row && cur_row == (long)(stmt->current_row +
                                          stmt->rows_found_in_set) )
            mysql_row_seek(stmt->result,stmt->end_of_set);
        else
            mysql_data_seek(stmt->result,cur_row);
    }
    stmt->current_row= cur_row;

    rows_to_fetch= min(max_row-cur_row, (long)stmt->stmt_options.rows_in_set);
    if ( !rows_to_fetch )
    {
        *pcrow= 0;
        stmt->rows_found_in_set= 0;
        if ( upd_status && stmt->stmt_options.rowsFetchedPtr )
            *stmt->stmt_options.rowsFetchedPtr= 0;
        MYODBCDbgReturnReturn(SQL_NO_DATA_FOUND);
    }

    if ( !(stmt->dbc->flag & FLAG_NO_LOCALE) )
        setlocale(LC_NUMERIC,"English");
    res= SQL_SUCCESS;
    for ( i= 0 ; i < rows_to_fetch ; i++ )
    {
        if ( stmt->result_array )
        {
            values= stmt->result_array+cur_row*stmt->result->field_count;
            if ( i == 0 )
                stmt->current_values= values;
        }
        else
        {
            if ( !if_forward_cache(stmt) )
            {
                /* This code will ensure that values is alway set */
                if ( i == 0 )
                    save_position= mysql_row_tell(stmt->result);
                if ( !(values= mysql_fetch_row(stmt->result)) )
                    break;
            }
            if ( stmt->fix_fields )
                values= (*stmt->fix_fields)(stmt,values);
            else
                stmt->result_lengths= mysql_fetch_lengths(stmt->result);
            stmt->current_values= values;
        }

        if (rgfRowStatus)
          rgfRowStatus[i]= SQL_ROW_SUCCESS;
        /*
          No need to update rowStatusPtr_ex, it's the same as rgfRowStatus.
        */
        if (upd_status && stmt->stmt_options.rowStatusPtr)
          stmt->stmt_options.rowStatusPtr[i]= SQL_ROW_SUCCESS;

        if (stmt->bind)             /* Should always be true */
        {
            ulong *lengths= stmt->result_lengths;
            BIND *bind,*end;
            for ( bind= stmt->bind,end= bind + stmt->result->field_count ;
                bind < end ;
                bind++,values++ )
            {
                if ( bind->rgbValue || bind->pcbValue )
                {
                    ulong offset,pcb_offset;
                    if ( stmt->stmt_options.bind_type == SQL_BIND_BY_COLUMN )
                    {
                        offset= bind->cbValueMax*i;
                        pcb_offset= sizeof(SQLINTEGER)*i;
                    }
                    else
                        pcb_offset= offset= stmt->stmt_options.bind_type*i;
                    stmt->getdata_offset= (ulong) ~0L;
                    if ( (tmp_res= sql_get_data( stmt,
                                                 bind->fCType,
                                                 bind->field,
                                                 (bind->rgbValue ? (char*) bind->rgbValue + offset : 0),
                                                 bind->cbValueMax,
                                                 (bind->pcbValue ? (SQLLEN*) ((char*) bind->pcbValue + pcb_offset) : 0),
                                                 *values,
                                                 (lengths ? *lengths : *values ? strlen(*values) : 0) ) )
                         != SQL_SUCCESS )
                    {
                        if ( tmp_res == SQL_SUCCESS_WITH_INFO )
                        {
                            if ( res == SQL_SUCCESS )
                                res= tmp_res;
                        }
                        else
                            res= SQL_ERROR;
                    }
                }
                if ( lengths )
                    lengths++;
            }
        }
        cur_row++;
    }
    stmt->rows_found_in_set= i;
    *pcrow= i;

    if ( upd_status && stmt->stmt_options.rowsFetchedPtr )
        *stmt->stmt_options.rowsFetchedPtr= i;

    if ( rgfRowStatus )
        for ( ; i < stmt->stmt_options.rows_in_set ; i++ )
            rgfRowStatus[i]= SQL_ROW_NOROW;

    /*
      No need to update rowStatusPtr_ex, it's the same as rgfRowStatus.
    */
    if ( upd_status && stmt->stmt_options.rowStatusPtr )
        for ( ; i < stmt->stmt_options.rows_in_set ; i++ )
            stmt->stmt_options.rowStatusPtr[i]= SQL_ROW_NOROW;

    if ( !stmt->result_array && !if_forward_cache(stmt) )
    {
        /* read data from first row */
        stmt->end_of_set= mysql_row_seek(stmt->result,save_position);
        if ( i > 1 )
        {
            stmt->current_values= mysql_fetch_row(stmt->result);
            if ( stmt->fix_fields )
                stmt->current_values= (*stmt->fix_fields)(stmt,stmt->current_values);
            else
                stmt->result_lengths= mysql_fetch_lengths(stmt->result);
        }
    }
    if ( !(stmt->dbc->flag & FLAG_NO_LOCALE) )
        setlocale(LC_NUMERIC,default_locale);

    MYODBCDbgReturnReturn(res);
}


/*
  @type    : ODBC 1.0 API
  @purpose : fetches the specified rowset of data from the result set and
  returns data for all bound columns. Rowsets can be specified
  at an absolute or relative position
*/

SQLRETURN SQL_API SQLExtendedFetch( SQLHSTMT        hstmt,
                                    SQLUSMALLINT    fFetchType,
                                    SQLROWOFFSET    irow,
                                    SQLROWSETSIZE  *pcrow,
                                    SQLUSMALLINT FAR *rgfRowStatus )
{
    STMT_OPTIONS *options= &((STMT FAR *)hstmt)->stmt_options;

    MYODBCDbgEnter;

    options->rowStatusPtr_ex= rgfRowStatus;

    MYODBCDbgReturnReturn( my_SQLExtendedFetch( hstmt,
                                fFetchType,
                                irow,
                                pcrow,
                                rgfRowStatus,
                                1 ) );
}


/*
  @type    : ODBC 3.0 API
  @purpose : fetches the specified rowset of data from the result set and
  returns data for all bound columns. Rowsets can be specified
  at an absolute or relative position
*/

SQLRETURN SQL_API SQLFetchScroll( SQLHSTMT      StatementHandle,
                                  SQLSMALLINT   FetchOrientation,
                                  SQLLEN        FetchOffset )
{
    SQLRETURN result;
    STMT_OPTIONS *options= &((STMT FAR *)StatementHandle)->stmt_options;

    MYODBCDbgEnter;

    options->rowStatusPtr_ex= NULL;

    result= my_SQLExtendedFetch( StatementHandle, 
                                 FetchOrientation,
                                 FetchOffset,
                                 options->rowsFetchedPtr,
                                 options->rowStatusPtr, 
                                 0 );
    MYODBCDbgReturnReturn( result );
}

/*
  @type    : ODBC 1.0 API
  @purpose : fetches the next rowset of data from the result set and
  returns data for all bound columns
*/

SQLRETURN SQL_API SQLFetch(SQLHSTMT StatementHandle)
{
    SQLRETURN result;
    STMT_OPTIONS *options= &((STMT FAR *)StatementHandle)->stmt_options;

    MYODBCDbgEnter;

    options->rowStatusPtr_ex= NULL;

    result= my_SQLExtendedFetch(StatementHandle, SQL_FETCH_NEXT,
                                0,
                                options->rowsFetchedPtr,
                                options->rowStatusPtr, 0);

    MYODBCDbgReturnReturn(result);
}
