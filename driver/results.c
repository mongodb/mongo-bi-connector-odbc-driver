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

#include "driver.h"
#include <errmsg.h>
#include <ctype.h>
#include <locale.h>

#define SQL_MY_PRIMARY_KEY 1212


void reset_getdata_position(STMT *stmt)
{
  stmt->getdata.column= (uint) ~0L;
  stmt->getdata.source= NULL;
  stmt->getdata.dst_bytes= (ulong) ~0L;
  stmt->getdata.dst_offset= (ulong) ~0L;
  stmt->getdata.src_offset= (ulong) ~0L;
  stmt->getdata.latest_bytes= stmt->getdata.latest_used= 0;
}


/**
  Retrieve the data from a field as a specified ODBC C type.

  @param[in]  stmt        Handle of statement
  @param[in]  fCType      ODBC C type to return data as
  @param[in]  field       Field describing the type of the data
  @param[out] rgbValue    Pointer to buffer for returning data
  @param[in]  cbValueMax  Length of buffer
  @param[out] pcbValue    Bytes used in the buffer, or SQL_NULL_DATA
  @param[out] value       The field data to be converted and returned
  @param[in]  length      Length of value
*/
static SQLRETURN SQL_API
sql_get_data(STMT *stmt, SQLSMALLINT fCType, MYSQL_FIELD *field,
             SQLPOINTER rgbValue, SQLLEN cbValueMax, SQLLEN *pcbValue,
             char *value, uint length)
{
  SQLLEN tmp;

  if (!value)
  {
    /* pcbValue must be available if its NULL */
    if (!pcbValue)
      return set_stmt_error(stmt,"22002",
                            "Indicator variable required but not supplied",0);

    *pcbValue= SQL_NULL_DATA;
  }
  else
  {
    if (!pcbValue)
      pcbValue= &tmp; /* Easier code */

    switch (fCType) {
    case SQL_C_CHAR:
      /* Handle BLOB -> CHAR conversion */
      if ((field->flags & (BLOB_FLAG|BINARY_FLAG)) == (BLOB_FLAG|BINARY_FLAG))
        return copy_binhex_result(stmt,
                                  (SQLCHAR *)rgbValue, cbValueMax, pcbValue,
                                  field, value, length);
      /* fall through */

    case SQL_C_BINARY:
      {
        char buff[21];
        if (field->type == MYSQL_TYPE_TIMESTAMP && length != 19)
        {
          /* Convert MySQL timestamp to full ANSI timestamp format. */
          char *pos;
          uint i;
          if (length == 6 || length == 10 || length == 12)
          {
            /* For two-digit year, < 60 is considered after Y2K */
            if (value[0] <= '6')
            {
              buff[0]= '2';
              buff[1]= '0';
            }
            else
            {
              buff[0]= '1';
              buff[1]= '9';
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
          buff[4]= '-';
          if (value[0] == '0' && value[1] == '0')
          {
            /* Month was 0, which ODBC can't handle. */
            *pcbValue= SQL_NULL_DATA;
            break;
          }
          pos= buff+5;
          length&= 30;  /* Ensure that length is ok */
          for (i= 1, length-= 2; (int)length > 0; length-= 2, i++)
          {
            *pos++= *value++;
            *pos++= *value++;
            *pos++= i < 2 ? '-' : (i == 2) ? ' ' : ':';
          }
          for ( ; pos != buff + 20; i++)
          {
            *pos++= '0';
            *pos++= '0';
            *pos++= i < 2 ? '-' : (i == 2) ? ' ' : ':';
          }
          value= buff;
          length= 19;
        }

        if (fCType == SQL_C_BINARY)
          return copy_binary_result(stmt, (SQLCHAR *)rgbValue, cbValueMax,
                                    pcbValue, field, value, length);
        else
          return copy_ansi_result(stmt, (SQLCHAR *)rgbValue, cbValueMax,
                                  pcbValue, field, value, length);
      }

    case SQL_C_WCHAR:
      return copy_wchar_result(stmt,
                               (SQLWCHAR *)rgbValue, cbValueMax, pcbValue,
                               field, value, length);

    case SQL_C_BIT:
      if (rgbValue)
        *((char *)rgbValue)= (value[0] == 1 ? 1 : (atoi(value) == 0) ? 0 : 1);
      *pcbValue= 1;
      break;

    case SQL_C_TINYINT:
    case SQL_C_STINYINT:
      if (rgbValue)
        *((SQLSCHAR *)rgbValue)= (SQLSCHAR)atoi(value);
      *pcbValue= 1;
      break;

    case SQL_C_UTINYINT:
      if (rgbValue)
        *((SQLCHAR *)rgbValue)= (SQLCHAR)(unsigned int)atoi(value);
      *pcbValue= 1;
      break;

    case SQL_C_SHORT:
    case SQL_C_SSHORT:
      if (rgbValue)
        *((SQLSMALLINT *)rgbValue)= (SQLSMALLINT)atoi(value);
      *pcbValue= sizeof(SQLSMALLINT);
      break;

    case SQL_C_USHORT:
      if (rgbValue)
        *((SQLUSMALLINT *)rgbValue)= (SQLUSMALLINT)(uint)atol(value);
      *pcbValue= sizeof(SQLUSMALLINT);
      break;

    case SQL_C_LONG:
    case SQL_C_SLONG:
      if (rgbValue)
      {
        /* Check if it could be a date...... :) */
        if (length >= 10 && value[4] == '-' && value[7] == '-' &&
             (!value[10] || value[10] == ' '))
        {
          *((SQLINTEGER *)rgbValue)= ((SQLINTEGER) atol(value) * 10000L +
                                      (SQLINTEGER) atol(value + 5) * 100L +
                                      (SQLINTEGER) atol(value + 8));
        }
        else
          *((SQLINTEGER *)rgbValue)= (SQLINTEGER) atol(value);
      }
      *pcbValue= sizeof(SQLINTEGER);
      break;

    case SQL_C_ULONG:
      if (rgbValue)
        *((SQLUINTEGER *)rgbValue)= (SQLUINTEGER)atol(value);
      *pcbValue= sizeof(SQLUINTEGER);
      break;

    case SQL_C_FLOAT:
      if (rgbValue)
        *((float *)rgbValue)= (float)atof(value);
      *pcbValue= sizeof(float);
      break;

    case SQL_C_DOUBLE:
      if (rgbValue)
        *((double *)rgbValue)= (double)atof(value);
      *pcbValue= sizeof(double);
      break;

    case SQL_C_DATE:
    case SQL_C_TYPE_DATE:
      {
        SQL_DATE_STRUCT tmp_date;
        if (!rgbValue)
          rgbValue= (char *)&tmp_date;
        if (!str_to_date((SQL_DATE_STRUCT *)rgbValue, value,
                         length, stmt->dbc->flag & FLAG_ZERO_DATE_TO_MIN))
          *pcbValue= sizeof(SQL_DATE_STRUCT);
        else
          *pcbValue= SQL_NULL_DATA;  /* ODBC can't handle 0000-00-00 dates */
        break;
      }

    case SQL_C_TIME:
    case SQL_C_TYPE_TIME:
      if (field->type == MYSQL_TYPE_TIMESTAMP ||
          field->type == MYSQL_TYPE_DATETIME)
      {
        SQL_TIMESTAMP_STRUCT ts;
        if (str_to_ts(&ts, value, stmt->dbc->flag & FLAG_ZERO_DATE_TO_MIN))
          *pcbValue= SQL_NULL_DATA;
        else
        {
          SQL_TIME_STRUCT *time_info= (SQL_TIME_STRUCT *)rgbValue;

          if (time_info)
          {
            time_info->hour=   ts.hour;
            time_info->minute= ts.minute;
            time_info->second= ts.second;
          }
          *pcbValue= sizeof(TIME_STRUCT);
        }
      }
      else if (field->type == MYSQL_TYPE_DATE)
      {
        SQL_TIME_STRUCT *time_info= (SQL_TIME_STRUCT *)rgbValue;

        if (time_info)
        {
          time_info->hour=   0;
          time_info->minute= 0;
          time_info->second= 0;
        }
        *pcbValue= sizeof(TIME_STRUCT);
      }
      else
      {
        SQL_TIME_STRUCT ts;
        if (str_to_time_st(&ts, value))
          *pcbValue= SQL_NULL_DATA;
        else
        {
          SQL_TIME_STRUCT *time_info= (SQL_TIME_STRUCT *)rgbValue;

          if (time_info)
          {
            time_info->hour=   ts.hour;
            time_info->minute= ts.minute;
            time_info->second= ts.second;
          }
          *pcbValue= sizeof(TIME_STRUCT);
        }
      }
      break;

    case SQL_C_TIMESTAMP:
    case SQL_C_TYPE_TIMESTAMP:
      if (field->type == MYSQL_TYPE_TIME)
      {
        SQL_TIME_STRUCT ts;

        if (str_to_time_st(&ts, value))
          *pcbValue= SQL_NULL_DATA;
        else
        {
          SQL_TIMESTAMP_STRUCT *timestamp_info=
            (SQL_TIMESTAMP_STRUCT *)rgbValue;
          time_t sec_time= time(NULL);
          struct tm cur_tm;
          localtime_r(&sec_time, &cur_tm);

          timestamp_info->year=   1900 + cur_tm.tm_year;
          timestamp_info->month=  1 + cur_tm.tm_mon; /* January is 0 in tm */
          timestamp_info->day=    cur_tm.tm_mday;
          timestamp_info->hour=   ts.hour;
          timestamp_info->minute= ts.minute;
          timestamp_info->second= ts.second;
          timestamp_info->fraction= 0;
          *pcbValue= sizeof(SQL_TIMESTAMP_STRUCT);
        }
      }
      else
      {
        if (str_to_ts((SQL_TIMESTAMP_STRUCT *)rgbValue, value,
                      stmt->dbc->flag & FLAG_ZERO_DATE_TO_MIN))
          *pcbValue= SQL_NULL_DATA;
        else
          *pcbValue= sizeof(SQL_TIMESTAMP_STRUCT);
      }
      break;

    case SQL_C_SBIGINT:
      /** @todo This is not right. SQLBIGINT is not always longlong. */
      if (rgbValue)
        *((longlong *)rgbValue)= (longlong)strtoll(value, NULL, 10);
      *pcbValue= sizeof(longlong);
      break;

    case SQL_C_UBIGINT:
      /** @todo This is not right. SQLUBIGINT is not always ulonglong.  */
      if (rgbValue)
          *((ulonglong *)rgbValue)= (ulonglong)strtoull(value, NULL, 10);
      *pcbValue= sizeof(ulonglong);
      break;
    }
  }

  if (stmt->getdata.source)  /* Second call to getdata */
    return SQL_NO_DATA_FOUND;

  stmt->getdata.source= NULL;         /* All data is retrieved */

  return SQL_SUCCESS;
}


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

    if ( stmt->param_count > 0 && stmt->dummy_state == ST_DUMMY_UNKNOWN &&
         (stmt->state != ST_PRE_EXECUTED || stmt->state != ST_EXECUTED) )
    {
        if ( do_dummy_parambind(hstmt) != SQL_SUCCESS )
            return SQL_ERROR;
    }
    if ( (error= check_result(stmt)) != SQL_SUCCESS )
        return error;

    if ( !stmt->result )
        *pccol= 0;      /* Not a select */
    else
        *pccol= stmt->result->field_count;

    return SQL_SUCCESS;
}


/**
  Get some basic properties of a column.

  @param[in]  hstmt      Statement handle
  @param[in]  column     Column number (starting with 1)
  @param[out] name       Pointer to column name
  @param[out] need_free  Whether the column name needs to be freed.
  @param[out] type       Column SQL type
  @param[out] size       Column size
  @param[out] scale      Scale
  @param[out] nullable   Whether the column is nullable
*/
SQLRETURN SQL_API
MySQLDescribeCol(SQLHSTMT hstmt, SQLUSMALLINT column,
                 SQLCHAR **name, SQLSMALLINT *need_free, SQLSMALLINT *type,
                 SQLULEN *size, SQLSMALLINT *scale, SQLSMALLINT *nullable)
{
  SQLRETURN error;
  MYSQL_FIELD *field;
  STMT *stmt= (STMT *)hstmt;

  if ((error= check_result(stmt)) != SQL_SUCCESS)
    return error;
  if (!stmt->result)
    return set_stmt_error(stmt, "07005", "No result set", 0);

  mysql_field_seek(stmt->result, column - 1);
  if (!(field= mysql_fetch_field(stmt->result)))
    return set_error(stmt, MYERR_S1002, "Invalid column number", 0);

  if (type)
    *type= get_sql_data_type(stmt, field, NULL);
  if (size)
    *size= get_column_size(stmt, field, FALSE);
  if (scale)
    *scale= max(0, get_decimal_digits(stmt, field));
  if (nullable)
    *nullable= ((field->flags & NOT_NULL_FLAG) ? SQL_NO_NULLS : SQL_NULLABLE);

  *need_free= 0;

  if (stmt->dbc->flag & FLAG_FULL_COLUMN_NAMES && field->table)
  {
    char *tmp= my_malloc(strlen(field->name) + strlen(field->table) + 2,
                         MYF(0));
    if (!tmp)
    {
      *need_free= -1;
      *name= NULL;
    }
    else
    {
      strxmov(tmp, field->table, ".", field->name, NullS);
      *name= tmp;
      *need_free= 1;
    }
  }
  else
    *name= field->name;

  return SQL_SUCCESS;
}


/*
  Retrieve an attribute of a column in a result set.

  @param[in]  hstmt          Handle to statement
  @param[in]  column         The column to retrieve data for, indexed from 1
  @param[in]  attrib         The attribute to be retrieved
  @param[out] char_attr      Pointer to a string pointer for returning strings
                             (caller must make their own copy)
  @param[out] num_attr       Pointer to an integer to return the value if the
                             @a attrib corresponds to a numeric type

  @since ODBC 1.0
*/
SQLRETURN SQL_API
MySQLColAttribute(SQLHSTMT hstmt, SQLUSMALLINT column,
                  SQLUSMALLINT attrib, SQLCHAR **char_attr, SQLLEN *num_attr)
{
  MYSQL_FIELD *field;
  STMT *stmt= (STMT *)hstmt;
  SQLLEN nparam= 0;
  SQLRETURN error;

  if (check_result(stmt) != SQL_SUCCESS)
    return SQL_ERROR;

  if (!stmt->result)
    return set_stmt_error(stmt, "07005", "No result set", 0);

#ifdef CHECK_EXTRA_ARGUMENTS
  if (column > stmt->result->field_count)
    return set_error(hstmt,  MYERR_07009, NULL, 0);
#endif

  if (!num_attr)
    num_attr= &nparam;

  if ((error= check_result(stmt)) != SQL_SUCCESS)
    return error;

  if (attrib == SQL_DESC_COUNT || attrib == SQL_COLUMN_COUNT)
  {
    *num_attr= (SQLLEN)stmt->result->field_count;
    return SQL_SUCCESS;
  }

  if (attrib == SQL_DESC_TYPE && column == 0)
  {
    *(SQLINTEGER *)num_attr= SQL_INTEGER;
    return SQL_SUCCESS;
  }

  mysql_field_seek(stmt->result, column - 1);
  if (!(field= mysql_fetch_field(stmt->result)))
    return set_error(stmt, MYERR_S1002, "Invalid column number", 0);

  switch (attrib)
  {
  case SQL_DESC_AUTO_UNIQUE_VALUE:
    *(SQLINTEGER *)num_attr= (field->flags & AUTO_INCREMENT_FLAG ?
                              SQL_TRUE : SQL_FALSE);
    break;

  /* We need support from server, when aliasing is there */
  case SQL_DESC_BASE_COLUMN_NAME:
    *char_attr= (SQLCHAR *)(field->org_name ? field->org_name : "");
    break;

  case SQL_DESC_LABEL:
  case SQL_DESC_NAME:
  case SQL_COLUMN_NAME:
    *char_attr= (SQLCHAR *)field->name;
    break;

  case SQL_DESC_BASE_TABLE_NAME:
    *char_attr= (SQLCHAR *)(field->org_table ? field->org_table : "");
    break;

  case SQL_DESC_TABLE_NAME:
    *char_attr= (SQLCHAR *)(field->table ? field->table : "");
    break;

  case SQL_DESC_CASE_SENSITIVE:
    *(SQLINTEGER *)num_attr= (field->flags & BINARY_FLAG ?
                              SQL_FALSE : SQL_TRUE);
    break;

  case SQL_DESC_CATALOG_NAME:
    *char_attr= (SQLCHAR *)((field->db && field->db[0]) ?
                            field->db : stmt->dbc->database);
    break;

  case SQL_DESC_DISPLAY_SIZE:
    *num_attr= get_display_size(stmt, field);
    break;

  case SQL_DESC_FIXED_PREC_SCALE:
    if (field->type == MYSQL_TYPE_DECIMAL ||
        field->type == MYSQL_TYPE_NEWDECIMAL)
      *(SQLINTEGER *)num_attr= SQL_TRUE;
    else
      *(SQLINTEGER *)num_attr= SQL_FALSE;
    break;

  case SQL_DESC_LENGTH:
    *num_attr= get_column_size(stmt, field, TRUE);
    break;

  case SQL_COLUMN_LENGTH:
  case SQL_DESC_OCTET_LENGTH:
    /* Need to add 1 for \0 on character fields. */
    *num_attr= get_transfer_octet_length(stmt, field) +
      test(field->charsetnr != 63);
    break;

  case SQL_DESC_LITERAL_SUFFIX:
  case SQL_DESC_LITERAL_PREFIX:
    switch (field->type) {
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_NEWDATE:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_YEAR:
      if (field->charsetnr == 63)
      {
        if (attrib == SQL_DESC_LITERAL_PREFIX)
          *char_attr= (SQLCHAR *)"0x";
        else
          *char_attr= (SQLCHAR *)"";
      }
      else
        *char_attr= (SQLCHAR *)"'";

    default:
      *char_attr= (SQLCHAR *)"";
    }
    break;

  case SQL_DESC_NULLABLE:
  case SQL_COLUMN_NULLABLE:
    *(SQLINTEGER *)num_attr= ((field->flags & NOT_NULL_FLAG) ? SQL_NO_NULLS :
                              SQL_NULLABLE);
    break;

  case SQL_DESC_NUM_PREC_RADIX:
    switch (field->type) {
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_LONGLONG:
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_DECIMAL:
      *(SQLINTEGER *)num_attr= 10;
      break;

    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE:
      *(SQLINTEGER *) num_attr= 2;
      break;

    default:
      *(SQLINTEGER *)num_attr= 0;
      break;
    }
    break;

  case SQL_COLUMN_PRECISION:
  case SQL_DESC_PRECISION:
    *(SQLINTEGER *)num_attr= get_column_size(stmt, field, FALSE);
    break;

  case SQL_COLUMN_SCALE:
  case SQL_DESC_SCALE:
    *(SQLINTEGER *)num_attr= max(0, get_decimal_digits(stmt, field));
    break;

  case SQL_DESC_SCHEMA_NAME:
    *char_attr= (SQLCHAR *)"";
    break;

  case SQL_DESC_SEARCHABLE:
    *(SQLINTEGER *)num_attr= SQL_SEARCHABLE;
    break;

  case SQL_DESC_TYPE:
    {
      SQLSMALLINT type= get_sql_data_type(stmt, field, NULL);
      if (type == SQL_DATE || type == SQL_TYPE_DATE || type == SQL_TIME ||
          type == SQL_TYPE_TIME || type == SQL_TIMESTAMP ||
          type == SQL_TYPE_TIMESTAMP)
        type= SQL_DATETIME;
      *(SQLINTEGER *)num_attr= type;
      break;
    }

  case SQL_DESC_CONCISE_TYPE:
    *(SQLINTEGER *)num_attr= get_sql_data_type(stmt, field, NULL);
    break;

  case SQL_DESC_TYPE_NAME:
    {
      /* Sort of lame that we have to strdup this, but that's life. */
      char buff[40];
      (void)get_sql_data_type(stmt, field, buff);
      *char_attr= (SQLCHAR *)my_strdup(buff, MYF(0));
    }

  case SQL_DESC_UNNAMED:
    *(SQLINTEGER *)num_attr= SQL_NAMED;
    break;

  case SQL_DESC_UNSIGNED:
    *(SQLINTEGER *) num_attr= ((field->flags & UNSIGNED_FLAG) ?  SQL_TRUE :
                               SQL_FALSE);
    break;

  case SQL_DESC_UPDATABLE:
    *(SQLINTEGER *)num_attr= (field->table && field->table[0] ?
                              SQL_ATTR_READWRITE_UNKNOWN : SQL_ATTR_READONLY);
    break;

  /*
    Hack : Fix for the error from ADO 'rs.resync' "Key value for this row
    was changed or deleted at the data store.  The local row is now deleted.
    This should also fix some Multi-step generated error cases from ADO
  */
  case SQL_MY_PRIMARY_KEY: /* MSSQL extension !! */
    *(SQLINTEGER *)num_attr= ((field->flags & PRI_KEY_FLAG) ?
                              SQL_TRUE : SQL_FALSE);
    break;

  default:
    return set_stmt_error(stmt, "HY091",
                          "Invalid descriptor field identifier",0);
  }

  return SQL_SUCCESS;
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

    icol--;
    /*
      The next case if because of VB 5.0 that binds columns before preparing
      a statement
    */

    if ( stmt->state == ST_UNKNOWN )
    {
        if ( fCType == SQL_C_NUMERIC ) /* We don't support this */
        {
            set_error(stmt,MYERR_07006,
                      "Restricted data type attribute violation(SQL_C_NUMERIC)",0);
            return SQL_ERROR;
        }
        if ( icol >= stmt->bound_columns )
        {
            if ( !(stmt->bind= (BIND*) my_realloc((char*) stmt->bind,
                                                  (icol+1)*sizeof(BIND),
                                                  MYF(MY_ALLOW_ZERO_PTR |
                                                      MY_FREE_ON_ERROR))) )
            {
                stmt->bound_columns= 0;
                return set_error(stmt,MYERR_S1001,NULL,4001);
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
                return SQL_ERROR;
        }
        if ( fCType == SQL_C_NUMERIC ) /* We don't support this */
        {
            set_error(stmt,MYERR_07006,
                      "Restricted data type attribute violation(SQL_C_NUMERIC)",0);
            return SQL_ERROR;
        }
        if ( (error= check_result(stmt)) != SQL_SUCCESS )
            return error;

        if ( !stmt->result || (uint) icol >= stmt->result->field_count )
        {
            error= set_error(stmt,MYERR_S1002,"Invalid column number",0);
            return error;
        }
        if ( !stmt->bind )
        {
            if ( !(stmt->bind= (BIND*) my_malloc(sizeof(BIND)*
                                                 stmt->result->field_count,
                                                 MYF(MY_ZEROFILL))) )
                return set_error(stmt,MYERR_S1001,NULL,4001);
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
    return SQL_SUCCESS;
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

    if ( !stmt->result || !stmt->current_values )
    {
        set_stmt_error(stmt,"24000","SQLGetData without a preceding SELECT",0);
        return SQL_ERROR;
    }
    if ( fCType == SQL_C_NUMERIC ) /* We don't support this */
    {
        set_error(stmt,MYERR_07006,
                  "Restricted data type attribute violation(SQL_C_NUMERIC)",0);
        return SQL_ERROR;
    }
    icol--;     /* Easier code if start from 0 */
    if (icol != stmt->getdata.column)
    {
      /* New column. Reset old offset */
      reset_getdata_position(stmt);
      stmt->getdata.column= icol;
    }

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

    return result;
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

    pthread_mutex_lock( &pStmt->dbc->lock );

    CLEAR_STMT_ERROR( pStmt );

    if (!mysql_more_results(&pStmt->dbc->mysql))
    {
      nReturn= SQL_NO_DATA;
      goto exitSQLMoreResults;
    }

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
    nReturn = my_SQLFreeStmtExtended((SQLHSTMT)pStmt,SQL_CLOSE,0);
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
            goto exitSQLMoreResults;
        }
        /* we have fields but no resultset (not even an empty one) - this is bad */
        nReturn = set_stmt_error( pStmt, "HY000", mysql_error( &pStmt->dbc->mysql ), mysql_errno( &pStmt->dbc->mysql ) );
        goto exitSQLMoreResults;
    }
    fix_result_types( pStmt );

exitSQLMoreResults:
    pthread_mutex_unlock( &pStmt->dbc->lock );
    return nReturn;
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

    if ( stmt->result )
    {
        *pcrow= (SQLLEN) mysql_affected_rows(&stmt->dbc->mysql);
    }
    else
    {
        *pcrow= (SQLLEN) stmt->affected_rows;
    }
    return SQL_SUCCESS;
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
                                       SQLULEN             *pcrow,
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
    SQLULEN dummy_pcrow;

    LINT_INIT(save_position);

    if ( !stmt->result )
        return set_stmt_error(stmt, "24000", "Fetch without a SELECT", 0);

    cur_row = stmt->current_row;

    if ( stmt->stmt_options.cursor_type == SQL_CURSOR_FORWARD_ONLY )
    {
        if ( fFetchType != SQL_FETCH_NEXT && !(stmt->dbc->flag & FLAG_SAFE) )
            return  set_error(stmt,MYERR_S1106,
                              "Wrong fetchtype with FORWARD ONLY cursor", 0);

        if ((stmt->dbc->flag & FLAG_NO_CACHE))
        {
          if (stmt->result_array)
            values= stmt->result_array + (cur_row * stmt->result->field_count);
          else
            values= mysql_fetch_row(stmt->result);
        }
    }

    if ( if_dynamic_cursor(stmt) && set_dynamic_result(stmt) )
        return set_error(stmt,MYERR_S1000,
                         "Driver Failed to set the internal dynamic result", 0);

    if ( !pcrow )
        pcrow= &dummy_pcrow;

    max_row= (long) mysql_num_rows(stmt->result);
    reset_getdata_position(stmt);
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
            return set_error(stmt, MYERR_S1106, "Fetch type out of range", 0);
    }

    if ( cur_row < 0 )
    {
        stmt->current_row= -1;  /* Before first row */
        stmt->rows_found_in_set= 0;
        mysql_data_seek(stmt->result,0L);
        return SQL_NO_DATA_FOUND;
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
        return SQL_NO_DATA_FOUND;
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
                    SQLLEN offset,pcb_offset;
                    SQLLEN pcbValue;

                    if ( stmt->stmt_options.bind_type == SQL_BIND_BY_COLUMN )
                    {
                        offset= bind->cbValueMax*i;
                        pcb_offset= sizeof(SQLLEN)*i;
                    }
                    else
                        pcb_offset= offset= stmt->stmt_options.bind_type*i;

                    /* apply SQL_ATTR_ROW_BIND_OFFSET_PTR */
                    if (stmt->stmt_options.bind_offset)
                    {
                      offset     += *stmt->stmt_options.bind_offset;
                      pcb_offset += *stmt->stmt_options.bind_offset;
                    }

                    reset_getdata_position(stmt);

                    if ( (tmp_res= sql_get_data( stmt,
                                                 bind->fCType,
                                                 bind->field,
                                                 (bind->rgbValue ? (char*) bind->rgbValue + offset : 0),
                                                 bind->cbValueMax,
                                                 &pcbValue,
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
                    if (bind->pcbValue)
                      *(bind->pcbValue + (pcb_offset / sizeof(SQLLEN))) = pcbValue;
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

    return res;
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
    SQLRETURN rc;
    SQLULEN rows;
    STMT_OPTIONS *options= &((STMT FAR *)hstmt)->stmt_options;

    options->rowStatusPtr_ex= rgfRowStatus;

    rc= my_SQLExtendedFetch(hstmt, fFetchType, irow, &rows, rgfRowStatus, 1);
    if (pcrow)
      *pcrow= (SQLROWSETSIZE)rows;

    return rc;
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
    STMT_OPTIONS *options= &((STMT FAR *)StatementHandle)->stmt_options;

    options->rowStatusPtr_ex= NULL;

    return my_SQLExtendedFetch(StatementHandle, FetchOrientation, FetchOffset,
                               options->rowsFetchedPtr, options->rowStatusPtr,
                               0);
}

/*
  @type    : ODBC 1.0 API
  @purpose : fetches the next rowset of data from the result set and
  returns data for all bound columns
*/

SQLRETURN SQL_API SQLFetch(SQLHSTMT StatementHandle)
{
    STMT_OPTIONS *options= &((STMT FAR *)StatementHandle)->stmt_options;

    options->rowStatusPtr_ex= NULL;

    return my_SQLExtendedFetch(StatementHandle, SQL_FETCH_NEXT, 0,
                               options->rowsFetchedPtr, options->rowStatusPtr,
                               0);
}
