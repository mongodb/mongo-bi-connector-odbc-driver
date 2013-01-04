/*
  Copyright (c) 2000, 2013, Oracle and/or its affiliates. All rights reserved.

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


/* Verifies if C type is suitable for copying SQL_BINARY data
   http://msdn.microsoft.com/en-us/library/ms713559%28VS.85%29.aspx */
my_bool is_binary_ctype( SQLSMALLINT cType)
{
  return (cType == SQL_C_CHAR
       || cType == SQL_C_BINARY
       || cType == SQL_C_WCHAR);
}


/* Converts binary(currently used for bit field only) to long long number.*/
long long binary2numeric(long long *dst, char *src, uint srcLen)
{
  *dst= 0;

  while (srcLen)
  {
    /* if source binary data is longer than 8 bytes(size of long long)
       we consider only minor 8 bytes */
    if (srcLen > sizeof(long long))
          continue;
    *dst+= (0xff & *src) << (--srcLen)*8;
    ++src;
  }

  return *dst;
}


/* Function that verifies if conversion from given sql type to c type supported.
   Based on http://msdn.microsoft.com/en-us/library/ms709280%28VS.85%29.aspx
   and underlying pages.
   Currently checks conversions for MySQL BIT(n) field(SQL_BIT or SQL_BINARY)
*/
my_bool odbc_supported_conversion(SQLSMALLINT sqlType, SQLSMALLINT cType)
{
  switch (sqlType)
  {
  case SQL_BIT:
    {
      switch (cType)
      {
      case SQL_C_DATE:
      case SQL_C_TYPE_DATE:
      case SQL_C_TIME:
      case SQL_C_TYPE_TIME:
      case SQL_C_TIMESTAMP:
      case SQL_C_TYPE_TIMESTAMP:
        return FALSE;
      }
    }
  case SQL_BINARY:
    {
      return is_binary_ctype(cType);
    }
  }

  return TRUE;
}


 /* Conversion supported by driver as exception to odbc specs
    (i.e. to odbc_supported_conversion() results).
    e.g. we map bit(n>1) to SQL_BINARY, but provide its conversion to numeric
    types */
 my_bool driver_supported_conversion(MYSQL_FIELD * field, SQLSMALLINT cType)
 {
   switch(field->type)
   {
   case MYSQL_TYPE_BIT:
     {
       switch (cType)
       {
       case SQL_C_BIT:
       case SQL_C_TINYINT:
       case SQL_C_STINYINT:
       case SQL_C_UTINYINT:
       case SQL_C_SHORT:
       case SQL_C_SSHORT:
       case SQL_C_USHORT:
       case SQL_C_LONG:
       case SQL_C_SLONG:
       case SQL_C_ULONG:
       case SQL_C_FLOAT:
       case SQL_C_DOUBLE:
       case SQL_C_SBIGINT:
       case SQL_C_UBIGINT:
         return TRUE;

       /* SQL_BIT should be converted SQL_C_NUMERIC, while SQL_BINARY should not
        */
       case SQL_C_NUMERIC:
         return TRUE;
       }
     }
   case MYSQL_TYPE_STRING:
     {
       switch (cType)
       {
       /* Date string is(often) identified as binary data by the driver.
          Have to either add exception here, or to change the way we detect if
          field is binary.
       */
       case SQL_C_TIMESTAMP:
       case SQL_C_DATE:
       case SQL_C_TIME:
       case SQL_C_TYPE_TIMESTAMP:
       case SQL_C_TYPE_DATE:
       case SQL_C_TYPE_TIME:

         return TRUE;
       }
     }
   }

   return FALSE;
 }


 SQLUSMALLINT sqlreturn2row_status(SQLRETURN res)
 {
   switch (res)
   {
     case SQL_SUCCESS:            return SQL_ROW_SUCCESS;
     case SQL_SUCCESS_WITH_INFO:  return SQL_ROW_SUCCESS_WITH_INFO;
   }

   return SQL_ROW_ERROR;
 }

/**
  Retrieve the data from a field as a specified ODBC C type.

  TODO arrec->indicator_ptr could be different than pcbValue
  ideally, two separate pointers would be passed here

  @param[in]  stmt        Handle of statement
  @param[in]  fCType      ODBC C type to return data as
  @param[in]  field       Field describing the type of the data
  @param[out] rgbValue    Pointer to buffer for returning data
  @param[in]  cbValueMax  Length of buffer
  @param[out] pcbValue    Bytes used in the buffer, or SQL_NULL_DATA
  @param[out] value       The field data to be converted and returned
  @param[in]  length      Length of value
  @param[in]  arrec       ARD record for this column (can be NULL)
*/
SQLRETURN SQL_API
sql_get_data(STMT *stmt, SQLSMALLINT fCType, uint column_number,
             SQLPOINTER rgbValue, SQLLEN cbValueMax, SQLLEN *pcbValue,
             char *value, ulong length, DESCREC *arrec)
{
  MYSQL_FIELD *field= mysql_fetch_field_direct(stmt->result, column_number);
  SQLLEN    tmp;
  long long numericValue;
  my_bool   convert= 1;
  SQLRETURN result= SQL_SUCCESS;
  char      as_string[50]; /* Buffer that might be required to convert other
                              types data to its string representation */

  /* get the exact type if we don't already have it */
  if (fCType == SQL_C_DEFAULT)
  {
    fCType= unireg_to_c_datatype(field);

    if (!cbValueMax)
    {
      cbValueMax= bind_length(fCType, 0);
    }
  }
  else if (fCType == SQL_ARD_TYPE)
  {
    if (!arrec)
    {
      return set_stmt_error(stmt, "07009", "Invalid descriptor index", 0);
    }

    fCType= arrec->concise_type;
  }

  /* set prec and scale for numeric */
  if (fCType == SQL_C_NUMERIC && rgbValue)
  {
    SQL_NUMERIC_STRUCT *sqlnum= (SQL_NUMERIC_STRUCT *) rgbValue;

    if (arrec) /* normally set via ard */
    {
      sqlnum->precision= (SQLSCHAR) arrec->precision;
      sqlnum->scale= (SQLCHAR) arrec->scale;
    }
    else /* just take the defaults */
    {
      sqlnum->precision= 38;
      sqlnum->scale= 0;
    }
  }

  if (is_null(stmt, column_number, value))
  {
    /* pcbValue must be available if its NULL */
    if (!pcbValue)
    {
      return set_stmt_error(stmt,"22002",
                            "Indicator variable required but not supplied",0);
    }

    *pcbValue= SQL_NULL_DATA;
  }
  else
  {
    if (!odbc_supported_conversion(get_sql_data_type(stmt, field, 0), fCType)
     && !driver_supported_conversion(field,fCType))
    {
      /*The state 07009 was incorrect
      (http://msdn.microsoft.com/en-us/library/ms715441%28v=VS.85%29.aspx)
      */
      return set_stmt_error(stmt, "07006", "Conversion is not possible", 0);
    }

    if (!pcbValue)
    {
      pcbValue= &tmp; /* Easier code */
    }

    if (field->type == MYSQL_TYPE_BIT)
    {
      if (is_binary_ctype(fCType))
      {
        return copy_binary_result(stmt, (SQLCHAR *)rgbValue, cbValueMax,
                                  pcbValue, field, value, length);
      }
      else
      {
        binary2numeric(&numericValue, value, length);
        convert= 0;
      }
    }

    switch (fCType)
    {
    case SQL_C_CHAR:
      /*
        Handle BLOB -> CHAR conversion 
        Conversion only for field which is having binary character set (63)
      */
      if (((field->flags & (BLOB_FLAG|BINARY_FLAG)) 
                == (BLOB_FLAG|BINARY_FLAG) )
                && field->charsetnr == BINARY_CHARSET_NUMBER )
      {
        return copy_binhex_result(stmt,
                                  (SQLCHAR *)rgbValue, cbValueMax, pcbValue,
                                  field, value, length);
      }
      /* fall through */

    case SQL_C_BINARY:
      {
        char buff[21];
        /* It seems that server version needs to be checked along with length,
           as fractions can mess everything up. but on other hand i do not know
           how to get MYSQL_TYPE_TIMESTAMP with short format date in modern
           servers. That also makes me think we do not need to consider
           possibility of having fractional part in complete_timestamp() */
        if (field->type == MYSQL_TYPE_TIMESTAMP && length < 19)
        {
          /* Convert MySQL timestamp to full ANSI timestamp format. */
          /*TODO if stlen_ind_ptr was not passed - error has to be returned */
          if (complete_timestamp(value, length, buff) == NULL)
          {
            *pcbValue= SQL_NULL_DATA;
            break;
          }

          value= buff;
          length= 19;
        }

        if (fCType == SQL_C_BINARY)
        {
          return copy_binary_result(stmt, (SQLCHAR *)rgbValue, cbValueMax,
                                    pcbValue, field, value, length);
        }
        else
        {
          char *tmp= get_string(stmt, column_number, value, &length, as_string);
          return copy_ansi_result(stmt,(SQLCHAR*)rgbValue, cbValueMax, pcbValue,
                          field, tmp,length);
        }
      }

    case SQL_C_WCHAR:
      {
        char *tmp= get_string(stmt, column_number, value, &length, as_string);

        return copy_wchar_result(stmt, (SQLWCHAR *)rgbValue,
                        (SQLINTEGER)(cbValueMax / sizeof(SQLWCHAR)), pcbValue,
                        field, tmp, length);
      }

    case SQL_C_BIT:
      if (rgbValue)
      {
        /* for MySQL bit(n>1) 1st byte may be '\0'. So testing already converted
           to a number value or atoi for other types. */
        if (!convert)
        {
          *((char *)rgbValue)= numericValue > 0 ? '\1' : '\0';
        }
        else
        {
          *((char *)rgbValue)= get_int(stmt, column_number, value, length) > 0 ?
                               '\1' : '\0';
        }
      }

      *pcbValue= 1;
      break;

    case SQL_C_TINYINT:
    case SQL_C_STINYINT:
      if (rgbValue)
        *((SQLSCHAR *)rgbValue)= (SQLSCHAR)(convert
                                 ? get_int(stmt, column_number, value, length)
                                 : (numericValue & (SQLSCHAR)(-1)));
      *pcbValue= 1;
      break;

    case SQL_C_UTINYINT:
      if (rgbValue)
        *((SQLCHAR *)rgbValue)= (SQLCHAR)(unsigned int)(convert
                                ? get_int(stmt, column_number, value, length)
                                : (numericValue & (SQLCHAR)(-1)));
      *pcbValue= 1;
      break;

    case SQL_C_SHORT:
    case SQL_C_SSHORT:
      if (rgbValue)
        *((SQLSMALLINT *)rgbValue)= (SQLSMALLINT)(convert
                                ? get_int(stmt, column_number, value, length)
                                : (numericValue & (SQLUSMALLINT)(-1)));
      *pcbValue= sizeof(SQLSMALLINT);
      break;

    case SQL_C_USHORT:
      if (rgbValue)
        *((SQLUSMALLINT *)rgbValue)= (SQLUSMALLINT)(uint)(convert
                                ? get_int64(stmt, column_number, value, length)
                                : (numericValue & (SQLUSMALLINT)(-1)));
      *pcbValue= sizeof(SQLUSMALLINT);
      break;

    case SQL_C_LONG:
    case SQL_C_SLONG:
      if (rgbValue)
      {
        /* Check if it could be a date...... :) */
        if (convert)
        {
          if (length >= 10 && value[4] == '-' && value[7] == '-' &&
               (!value[10] || value[10] == ' '))
          {
            *((SQLINTEGER *)rgbValue)= ((SQLINTEGER) atol(value) * 10000L +
                                        (SQLINTEGER) atol(value + 5) * 100L +
                                        (SQLINTEGER) atol(value + 8));
          }
          else
            *((SQLINTEGER *)rgbValue)= (SQLINTEGER) get_int64(stmt,
                                                  column_number, value, length);
        }
        else
          *((SQLINTEGER *)rgbValue)= (SQLINTEGER)(numericValue
                                                  & (SQLUINTEGER)(-1));
      }
      *pcbValue= sizeof(SQLINTEGER);
      break;

    case SQL_C_ULONG:
      if (rgbValue)
        *((SQLUINTEGER *)rgbValue)= (SQLUINTEGER)(convert ?
                                get_int64(stmt, column_number, value, length) :
                                numericValue & (SQLUINTEGER)(-1));
      *pcbValue= sizeof(SQLUINTEGER);
      break;

    case SQL_C_FLOAT:
      if (rgbValue)
        *((float *)rgbValue)= (float)(convert ? get_double(stmt, column_number,
                                    value, length) : numericValue & (int)(-1));
      *pcbValue= sizeof(float);
      break;

    case SQL_C_DOUBLE:
      if (rgbValue)
        *((double *)rgbValue)= (double)(convert ? get_double(stmt, column_number,
                                    value, length) : numericValue);
      *pcbValue= sizeof(double);
      break;

    case SQL_C_DATE:
    case SQL_C_TYPE_DATE:
      {
        SQL_DATE_STRUCT tmp_date;
        char *tmp= get_string(stmt, column_number, value, &length, as_string);

        if (!rgbValue)
        {
          rgbValue= (char *)&tmp_date;
        }

        if (!str_to_date((SQL_DATE_STRUCT *)rgbValue, tmp, length,
                          stmt->dbc->ds->zero_date_to_min))
        {
          *pcbValue= sizeof(SQL_DATE_STRUCT);
        }
        else
        {
          /*TODO - again, error if pcbValue was originally NULL */
          *pcbValue= SQL_NULL_DATA;  /* ODBC can't handle 0000-00-00 dates */
        }

        break;
      }

    case SQL_C_TIME:
    case SQL_C_TYPE_TIME:
      if (field->type == MYSQL_TYPE_TIMESTAMP ||
          field->type == MYSQL_TYPE_DATETIME)
      {
        SQL_TIMESTAMP_STRUCT ts;

        switch (str_to_ts(&ts, get_string(stmt, column_number, value, &length,
                          as_string), SQL_NTS, stmt->dbc->ds->zero_date_to_min,
                          TRUE))
        {
        case SQLTS_BAD_DATE:
          return set_stmt_error(stmt, "22018", "Data value is not a valid time(stamp) value", 0);
        case SQLTS_NULL_DATE:
          *pcbValue= SQL_NULL_DATA;
          break;
        default:
          {
            SQL_TIME_STRUCT *time_info= (SQL_TIME_STRUCT *)rgbValue;

            if (time_info)
            {
              time_info->hour=   ts.hour;
              time_info->minute= ts.minute;
              time_info->second= ts.second;

              if (ts.fraction > 0)
              {
                set_stmt_error(stmt, "01S07", NULL, 0);
                result= SQL_SUCCESS_WITH_INFO;
              }
            }
            *pcbValue= sizeof(TIME_STRUCT);
          }
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
        char *tmp= get_string(stmt,
                            column_number, value, &length, as_string);
        if (str_to_time_st(&ts, tmp))
        {
          *pcbValue= SQL_NULL_DATA;
        }
        else
        {
          SQL_TIME_STRUCT *time_info= (SQL_TIME_STRUCT *)rgbValue;
          SQLUINTEGER fraction;

          if (time_info)
          {
            time_info->hour=   ts.hour;
            time_info->minute= ts.minute;
            time_info->second= ts.second;
          }

          *pcbValue= sizeof(TIME_STRUCT);

          get_fractional_part(tmp, SQL_NTS, TRUE, &fraction);

          if (fraction)
          {
            /* http://msdn.microsoft.com/en-us/library/ms713346%28v=VS.85%29.aspx
               We are loosing fractional part - thus we have to set correct sqlstate
               and return SQL_SUCCESS_WITH_INFO */
            set_stmt_error(stmt, "01S07", NULL, 0);
            result= SQL_SUCCESS_WITH_INFO;
          }
        }
      }
      break;

    case SQL_C_TIMESTAMP:
    case SQL_C_TYPE_TIMESTAMP:
      {
      char *tmp= get_string(stmt, column_number, value, &length, as_string);

      if (field->type == MYSQL_TYPE_TIME)
      {
        SQL_TIME_STRUCT ts;
        
        if (str_to_time_st(&ts, tmp))
        {
          *pcbValue= SQL_NULL_DATA;
        }
        else
        {
          SQL_TIMESTAMP_STRUCT *timestamp_info=
            (SQL_TIMESTAMP_STRUCT *)rgbValue;
          time_t sec_time= time(NULL);
          struct tm cur_tm;
          localtime_r(&sec_time, &cur_tm);

          /* I wornder if that hasn't to be server current date*/
          timestamp_info->year=   1900 + cur_tm.tm_year;
          timestamp_info->month=  1 + cur_tm.tm_mon; /* January is 0 in tm */
          timestamp_info->day=    cur_tm.tm_mday;
          timestamp_info->hour=   ts.hour;
          timestamp_info->minute= ts.minute;
          timestamp_info->second= ts.second;
          get_fractional_part(tmp, SQL_NTS, TRUE, &timestamp_info->fraction);
          *pcbValue= sizeof(SQL_TIMESTAMP_STRUCT);
        }
      }
      else
      {
        switch (str_to_ts((SQL_TIMESTAMP_STRUCT *)rgbValue, tmp, SQL_NTS,
                      stmt->dbc->ds->zero_date_to_min, TRUE))
        {
        case SQLTS_BAD_DATE:
          return set_stmt_error(stmt, "22018", "Data value is not a valid date/time(stamp) value", 0);

        case SQLTS_NULL_DATE:
          *pcbValue= SQL_NULL_DATA;
          break;

        default:
          *pcbValue= sizeof(SQL_TIMESTAMP_STRUCT);
        }
      }
      break;
      }

    case SQL_C_SBIGINT:
      /** @todo This is not right. SQLBIGINT is not always longlong. */
      if (rgbValue)
        *((longlong *)rgbValue)= (longlong)(convert ? get_int64(stmt,
                        column_number, value, length) : numericValue);

      *pcbValue= sizeof(longlong);
      break;

    case SQL_C_UBIGINT:
      /** @todo This is not right. SQLUBIGINT is not always ulonglong.  */
      if (rgbValue)
          *((ulonglong *)rgbValue)= (ulonglong)(convert ? get_int64(stmt,
                        column_number, value, length) : numericValue);

      *pcbValue= sizeof(ulonglong);
      break;

    case SQL_C_NUMERIC:
      {
        int overflow= 0;
        SQL_NUMERIC_STRUCT *sqlnum= (SQL_NUMERIC_STRUCT *) rgbValue;

        if (rgbValue)
        {
          if (convert)
            sqlnum_from_str(get_string(stmt, column_number, value, &length, as_string), sqlnum, &overflow);
          else /* bit field */
          {
            /* Lazy way - converting number we have to a string.
               If it couldn't happen we have to scale/unscale number - we would
               just reverse binary data */
            char _value[21]; /* max string length of 64bit number */
            sprintf(_value, "%llu", numericValue);

            sqlnum_from_str(_value, sqlnum, &overflow);
          }

        }
        *pcbValue= sizeof(ulonglong);

        if (overflow)
          return set_stmt_error(stmt, "22003",
                                "Numeric value out of range", 0);
      }
      break;

    default:
      return set_error(stmt,MYERR_07006,
                       "Restricted data type attribute violation",0);
      break;
    }
  }

  if (stmt->getdata.source)  /* Second call to getdata */
    return SQL_NO_DATA_FOUND;

  return result;
}


/*
  @type    : myodbc3 internal
  @purpose : execute the query if it is only prepared. This is needed
  because the ODBC standard allows calling some functions
  before SQLExecute().
*/

static SQLRETURN check_result(STMT FAR *stmt )
{
  SQLRETURN error= 0;

  switch (stmt->state)
  {
    case ST_UNKNOWN:
      error= set_stmt_error(stmt,"24000","Invalid cursor state",0);
      break;
    case ST_PREPARED:
      /*TODO: introduce state for statements prepared on the server side */
      if (!ssps_used(stmt) && stmt_returns_result(&stmt->query))
      {
        SQLULEN real_max_rows= stmt->stmt_options.max_rows;
        stmt->stmt_options.max_rows= 1;
        /* select limit will be restored back to max_rows before real execution */
        if ( (error= my_SQLExecute(stmt)) == SQL_SUCCESS )
        {
          stmt->state= ST_PRE_EXECUTED;  /* mark for execute */
        }
        else
        {
          set_sql_select_limit(stmt->dbc, real_max_rows);
        }
        stmt->stmt_options.max_rows= real_max_rows;
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
    SQLRETURN rc;
    STMT FAR *stmt= (STMT FAR *)hstmt;
    uint     nparam;

    for ( nparam= 0; nparam < stmt->param_count; ++nparam )
    {
        DESCREC *aprec= desc_get_rec(stmt->apd, nparam, TRUE);
        if (!aprec->par.real_param_done)
        {
            /* do the dummy bind temporarily to get the result set
               and once everything is done, remove it */
            if (!SQL_SUCCEEDED(rc= my_SQLBindParameter(hstmt, nparam+1,
                                                       SQL_PARAM_INPUT,
                                                       SQL_C_CHAR,
                                                       SQL_VARCHAR, 0, 0,
                                                       "NULL", SQL_NTS, NULL)))
                return rc;
            /* reset back to false (this is the *dummy* param bind) */
            aprec->par.real_param_done= FALSE;
        }
    }
    stmt->dummy_state= ST_DUMMY_PREPARED;
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

  if (!ssps_used(stmt))
  {
    if (stmt->param_count > 0 && stmt->dummy_state == ST_DUMMY_UNKNOWN &&
      (stmt->state != ST_PRE_EXECUTED || stmt->state != ST_EXECUTED))
    {
      if ( do_dummy_parambind(hstmt) != SQL_SUCCESS )
      {
        return SQL_ERROR;
      }
    }
    if ((error= check_result(stmt)) != SQL_SUCCESS)
    {
      return error;
    }
  }

  *pccol= (SQLSMALLINT) stmt->ird->count;

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
  STMT *stmt= (STMT *)hstmt;
  DESCREC* irrec;

  /* SQLDescribeCol can be called before SQLExecute. Thus we need make sure that
     all parameters have been bound */
  if (!ssps_used(stmt))
  {
    if ( stmt->param_count > 0 && stmt->dummy_state == ST_DUMMY_UNKNOWN &&
      (stmt->state != ST_PRE_EXECUTED || stmt->state != ST_EXECUTED) )
    {
      if ( do_dummy_parambind(hstmt) != SQL_SUCCESS )
        return SQL_ERROR;
    }

    if ((error= check_result(stmt)) != SQL_SUCCESS)
      return error;
    if (!stmt->result)
      return set_stmt_error(stmt, "07005", "No result set", 0);
    if (column == 0 || column > stmt->ird->count)
      return set_stmt_error(stmt, "07009", "Invalid descriptor index", 0);
  }

  irrec= desc_get_rec(stmt->ird, column - 1, FALSE);
  assert(irrec);

  if (type)
    *type= irrec->concise_type;
  if (size)
    *size= irrec->length;
  if (scale)
    *scale= irrec->scale;
  if (nullable)
    *nullable= irrec->nullable;

  *need_free= 0;

  if (stmt->dbc->ds->return_table_names_for_SqlDescribeCol && irrec->table_name)
  {
    char *tmp= my_malloc(strlen((char *)irrec->name) +
                         strlen((char *)irrec->table_name) + 2,
                         MYF(0));
    if (!tmp)
    {
      *need_free= -1;
      *name= NULL;
    }
    else
    {
      strxmov(tmp, (char *)irrec->table_name, ".", (char *)irrec->name, NullS);
      *name= (SQLCHAR *)tmp;
      *need_free= 1;
    }
  }
  else
    *name= (SQLCHAR *)irrec->name;

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
  STMT *stmt= (STMT *)hstmt;
  SQLLEN nparam= 0;
  SQLRETURN error= SQL_SUCCESS;
  DESCREC *irrec;

  if (!ssps_used(stmt))
  {
    /* MySQLColAttribute can be called before SQLExecute. Thus we need make sure that
    all parameters have been bound */
    if ( stmt->param_count > 0 && stmt->dummy_state == ST_DUMMY_UNKNOWN &&
      (stmt->state != ST_PRE_EXECUTED || stmt->state != ST_EXECUTED) )
    {
      if ( do_dummy_parambind(hstmt) != SQL_SUCCESS )
        return SQL_ERROR;
    }

    if (check_result(stmt) != SQL_SUCCESS)
      return SQL_ERROR;
  }

  if (!stmt->result)
  {
    return set_stmt_error(stmt, "07005", "No result set", 0);
  }

  /* we report bookmark type if requested, nothing else */
  if (attrib == SQL_DESC_TYPE && column == 0)
  {
    *(SQLINTEGER *)num_attr= SQL_INTEGER;
    return SQL_SUCCESS;
  }

  if (column == 0 || column > stmt->ird->count)
    return set_error(hstmt,  MYERR_07009, NULL, 0);

  if (!num_attr)
    num_attr= &nparam;

  if ((error= check_result(stmt)) != SQL_SUCCESS)
    return error;

  if (attrib == SQL_DESC_COUNT || attrib == SQL_COLUMN_COUNT)
  {
    *num_attr= stmt->ird->count;
    return SQL_SUCCESS;
  }

  irrec= desc_get_rec(stmt->ird, column - 1, FALSE);
  assert(irrec);

  /*
     Map to descriptor fields. This approach is only valid
     for ODBC 3.0 API applications.

     @todo Add additional logic to properly handle these fields
           for ODBC 2.0 API applications.
  */
  switch (attrib)
  {
  case SQL_COLUMN_SCALE:
    attrib= SQL_DESC_SCALE;
    break;
  case SQL_COLUMN_PRECISION:
    attrib= SQL_DESC_PRECISION;
    break;
  case SQL_COLUMN_NULLABLE:
    attrib= SQL_DESC_NULLABLE;
    break;
  case SQL_COLUMN_LENGTH:
    attrib= SQL_DESC_OCTET_LENGTH;
    break;
  case SQL_COLUMN_NAME:
    attrib= SQL_DESC_NAME;
    break;
  }

  switch (attrib)
  {
  case SQL_DESC_AUTO_UNIQUE_VALUE:
  case SQL_DESC_CASE_SENSITIVE:
  case SQL_DESC_FIXED_PREC_SCALE:
  case SQL_DESC_NULLABLE:
  case SQL_DESC_NUM_PREC_RADIX:
  case SQL_DESC_PRECISION:
  case SQL_DESC_SCALE:
  case SQL_DESC_SEARCHABLE:
  case SQL_DESC_TYPE:
  case SQL_DESC_CONCISE_TYPE:
  case SQL_DESC_UNNAMED:
  case SQL_DESC_UNSIGNED:
  case SQL_DESC_UPDATABLE:
    error= stmt_SQLGetDescField(stmt, stmt->ird, column, attrib,
#ifdef USE_SQLCOLATTRIBUTE_SQLLEN_PTR
                                (SQLPOINTER)num_attr,SQL_IS_LEN,
#else
                                num_attr, SQL_IS_INTEGER,
#endif
               NULL);
    break;

  case SQL_DESC_DISPLAY_SIZE:
  case SQL_DESC_LENGTH:
  case SQL_DESC_OCTET_LENGTH:
    error= stmt_SQLGetDescField(stmt, stmt->ird, column, attrib,
                                num_attr, SQL_IS_LEN, NULL);
    break;

  /* We need support from server, when aliasing is there */
  case SQL_DESC_BASE_COLUMN_NAME:
    *char_attr= irrec->base_column_name ? irrec->base_column_name :
                                          (SQLCHAR *) "";
    break;

  case SQL_DESC_LABEL:
  case SQL_DESC_NAME:
    *char_attr= irrec->name;
    break;

  case SQL_DESC_BASE_TABLE_NAME:
    *char_attr= irrec->base_table_name ? irrec->base_table_name :
                                         (SQLCHAR *) "";
    break;

  case SQL_DESC_TABLE_NAME:
    *char_attr= irrec->table_name ? irrec->table_name : (SQLCHAR *) "";
    break;

  case SQL_DESC_CATALOG_NAME:
    *char_attr= irrec->catalog_name;
    break;

  case SQL_DESC_LITERAL_PREFIX:
    *char_attr= irrec->literal_prefix;
    break;

  case SQL_DESC_LITERAL_SUFFIX:
    *char_attr= irrec->literal_suffix;
    break;

  case SQL_DESC_SCHEMA_NAME:
    *char_attr= irrec->schema_name;
    break;

  case SQL_DESC_TYPE_NAME:
    *char_attr= irrec->type_name;
    break;

  /*
    Hack : Fix for the error from ADO 'rs.resync' "Key value for this row
    was changed or deleted at the data store.  The local row is now deleted.
    This should also fix some Multi-step generated error cases from ADO
  */
  case SQL_MY_PRIMARY_KEY: /* MSSQL extension !! */
    *(SQLINTEGER *)num_attr= ((irrec->row.field->flags & PRI_KEY_FLAG) ?
                              SQL_TRUE : SQL_FALSE);
    break;

  default:
    return set_stmt_error(stmt, "HY091",
                          "Invalid descriptor field identifier",0);
  }

  return error;
}


/*
  @type    : ODBC 1.0 API
  @purpose : binds application data buffers to columns in the result set
*/

SQLRETURN SQL_API SQLBindCol(SQLHSTMT      StatementHandle, 
                             SQLUSMALLINT  ColumnNumber,
                             SQLSMALLINT   TargetType, 
                             SQLPOINTER    TargetValuePtr,
                             SQLLEN        BufferLength, 
                             SQLLEN *      StrLen_or_IndPtr)
{
  SQLRETURN rc;
  STMT FAR *stmt= (STMT FAR*) StatementHandle;
  DESCREC *arrec;
  /* TODO if this function fails, the SQL_DESC_COUNT should be unchanged in ard */

  CLEAR_STMT_ERROR(stmt);

  if (!TargetValuePtr && !StrLen_or_IndPtr) /* Handling unbinding */
  {
    /*
       If unbinding the last bound column, we reduce the
       ARD records until the highest remaining bound column.
    */
    if (ColumnNumber == stmt->ard->count)
    {
      int i;
      --stmt->ard->count;
      for (i= (int)stmt->ard->count - 1; i >= 0; --i)
      {
        arrec= desc_get_rec(stmt->ard, i, FALSE);
        if (ARD_IS_BOUND(arrec))
          break;
        else
          --stmt->ard->count;
      }
    }
    else
    {
      arrec= desc_get_rec(stmt->ard, ColumnNumber - 1, FALSE);
      if (arrec)
      {
        arrec->data_ptr= NULL;
        arrec->octet_length_ptr= NULL;
      }
    }
    return SQL_SUCCESS;
  }

  if (ColumnNumber == 0 || (stmt->state == ST_EXECUTED &&
                            ColumnNumber > stmt->ird->count))
  {
    return set_stmt_error(stmt, "07009", "Invalid descriptor index",
                          MYERR_07009);
  }

  arrec= desc_get_rec(stmt->ard, ColumnNumber - 1, TRUE);

  if ((rc= stmt_SQLSetDescField(stmt, stmt->ard, ColumnNumber,
                                SQL_DESC_CONCISE_TYPE,
                                (SQLPOINTER)(SQLINTEGER) TargetType,
                                SQL_IS_SMALLINT)) != SQL_SUCCESS)
    return rc;
  if ((rc= stmt_SQLSetDescField(stmt, stmt->ard, ColumnNumber,
                                SQL_DESC_OCTET_LENGTH,
                                (SQLPOINTER) bind_length(TargetType,
                                                         BufferLength),
                                SQL_IS_LEN)) != SQL_SUCCESS)
    return rc;
  if ((rc= stmt_SQLSetDescField(stmt, stmt->ard, ColumnNumber,
                                SQL_DESC_DATA_PTR, TargetValuePtr,
                                SQL_IS_POINTER)) != SQL_SUCCESS)
    return rc;
  if ((rc= stmt_SQLSetDescField(stmt, stmt->ard, ColumnNumber,
                                SQL_DESC_INDICATOR_PTR, StrLen_or_IndPtr,
                                SQL_IS_POINTER)) != SQL_SUCCESS)
    return rc;
  if ((rc= stmt_SQLSetDescField(stmt, stmt->ard, ColumnNumber,
                                SQL_DESC_OCTET_LENGTH_PTR, StrLen_or_IndPtr,
                                SQL_IS_POINTER)) != SQL_SUCCESS)
    return rc;

  return SQL_SUCCESS;
}


/*
  @type    : myodbc3 internal
  @purpose : returns the latest resultset(dynamic)
*/

my_bool set_dynamic_result(STMT FAR *stmt)
{
  SQLRETURN rc;
  long row= stmt->current_row;
  uint rows= stmt->rows_found_in_set;

  rc= my_SQLExecute(stmt);

  stmt->current_row= row;
  stmt->rows_found_in_set= rows;

  if (SQL_SUCCEEDED(rc))
    set_current_cursor_data(stmt,0);

  return rc != 0;
}


/*
  @type    : ODBC 1.0 API
  @purpose : retrieves data for a single column in the result set. It can
  be called multiple times to retrieve variable-length data
  in parts
*/

SQLRETURN SQL_API SQLGetData(SQLHSTMT      StatementHandle,
                             SQLUSMALLINT  ColumnNumber,
                             SQLSMALLINT   TargetType,
                             SQLPOINTER    TargetValuePtr,
                             SQLLEN        BufferLength,
                             SQLLEN *      StrLen_or_IndPtr)
{
    STMT *stmt= (STMT *) StatementHandle;
    SQLRETURN result;
    ulong length= 0;
    DESCREC *irrec;

    if (!stmt->result || !stmt->current_values)
    {
      set_stmt_error(stmt,"24000","SQLGetData without a preceding SELECT",0);
      return SQL_ERROR;
    }

    if (ColumnNumber < 1 || ColumnNumber > stmt->ird->count)
    {
      return set_stmt_error(stmt, "07009", "Invalid descriptor index",
                            MYERR_07009);
    }
    --ColumnNumber;     /* Easier code if start from 0 */
    if (ColumnNumber != stmt->getdata.column)
    {
      /* New column. Reset old offset */
      reset_getdata_position(stmt);
      stmt->getdata.column= ColumnNumber;
    }

    irrec= desc_get_rec(stmt->ird, ColumnNumber, FALSE);
    assert(irrec);

    /* catalog functions with "fake" results won't have lengths */
    length= irrec->row.datalen;
    if (!length && stmt->current_values[ColumnNumber])
      length= strlen(stmt->current_values[ColumnNumber]);

    if (!stmt->dbc->ds->dont_use_set_locale)
      setlocale(LC_NUMERIC, "C");

    result= sql_get_data(stmt, TargetType, ColumnNumber,
                         TargetValuePtr, BufferLength, StrLen_or_IndPtr,
                         stmt->current_values[ColumnNumber], length,
                         desc_get_rec(stmt->ard, ColumnNumber, FALSE));

    if (!stmt->dbc->ds->dont_use_set_locale)
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

  /* SQLExecute or SQLExecDirect need to be called first */
  if ( pStmt->state != ST_EXECUTED )
  {
    nReturn = set_stmt_error( pStmt, "HY010", NULL, 0 );
    goto exitSQLMoreResults;
  }

  /* try to get next resultset */
  nRetVal = next_result(pStmt);

  /* call to mysql_next_result() failed */
  if (nRetVal > 0)
  {
    nRetVal= mysql_errno(&pStmt->dbc->mysql);

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
  if (nRetVal < 0)
  {
    nReturn = SQL_NO_DATA;
    goto exitSQLMoreResults;
  }

  /* cleanup existing resultset */
  nReturn = my_SQLFreeStmtExtended((SQLHSTMT)pStmt,SQL_CLOSE,0);
  if (!SQL_SUCCEEDED( nReturn ))
  {
    goto exitSQLMoreResults;
  }

  /* start using the new resultset */
  pStmt->result = get_result_metadata(pStmt, FALSE);

  if (!pStmt->result)
  {
    /* no fields means; INSERT, UPDATE or DELETE so no resultset is fine */
    if (!field_count(pStmt))
    {
      pStmt->state = ST_EXECUTED;
      pStmt->affected_rows = affected_rows(pStmt);
      goto exitSQLMoreResults;
    }
    /* we have fields but no resultset (not even an empty one) - this is bad */
    nReturn = set_stmt_error(pStmt, "HY000", mysql_error( &pStmt->dbc->mysql ),
                              mysql_errno(&pStmt->dbc->mysql));
    goto exitSQLMoreResults;
  }
  
  /* checking if next result is SP OUT params and fetch them if needed */
  if (IS_PS_OUT_PARAMS(pStmt))
  {
    fix_result_types(pStmt);
    /* This server status(SERVER_PS_OUT_PARAMS) can be only if we used PS */
    ssps_get_out_params(pStmt);
  }
  else
  {
    free_result_bind(pStmt);
    if (bind_result(pStmt) || get_result(pStmt))
    {
      nReturn= set_stmt_error(pStmt, "HY000", mysql_error( &pStmt->dbc->mysql ),
                            mysql_errno(&pStmt->dbc->mysql));
    }

    fix_result_types(pStmt);
  }

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
      /* for SetPos operations result is defined and they use direct execution */
        *pcrow= (SQLLEN) affected_rows(stmt);
    }
    else
    {
        *pcrow= (SQLLEN) stmt->affected_rows;
    }
    return SQL_SUCCESS;
}


/**
  Populate the data lengths in the IRD for the current row

  @param[in]  ird         IRD to populate
  @param[in]  lengths     Data lengths from mysql_fetch_lengths()
  @param[in]  fields      Number of fields
*/
void fill_ird_data_lengths(DESC *ird, ulong *lengths, uint fields)
{
  uint i;
  DESCREC *irrec;

  assert(fields == ird->count);

  /* This will be NULL for catalog functions with "fake" results */
  if (!lengths)
  {	
    return;
  }

  for (i= 0; i < fields; ++i)
  {
    irrec= desc_get_rec(ird, i, FALSE);
    assert(irrec);

    irrec->row.datalen= lengths[i];
  }
}


/**
  Populate a single row of fetch buffers

  @param[in]  stmt        Handle of statement
  @param[in]  values      Row buffers from libmysql
  @param[in]  rownum      Row number of current fetch block
*/
static SQLRETURN
fill_fetch_buffers(STMT *stmt, MYSQL_ROW values, uint rownum)
{
  SQLRETURN res= SQL_SUCCESS, tmp_res;
  int i;
  ulong length= 0;
  DESCREC *irrec, *arrec;
 
  for (i= 0; i < myodbc_min(stmt->ird->count, stmt->ard->count); ++i, ++values)
  {
    irrec= desc_get_rec(stmt->ird, i, FALSE);
    arrec= desc_get_rec(stmt->ard, i, FALSE);
    assert(irrec && arrec);

    if (ARD_IS_BOUND(arrec))
    {
      SQLLEN *pcbValue= NULL;
      SQLPOINTER TargetValuePtr= NULL;

      reset_getdata_position(stmt);

      if (arrec->data_ptr)
      {
        TargetValuePtr= ptr_offset_adjust(arrec->data_ptr, 
                                          stmt->ard->bind_offset_ptr, 
                                          stmt->ard->bind_type, 
                                          arrec->octet_length, rownum);
      }

      /* catalog functions with "fake" results won't have lengths */
      length= irrec->row.datalen;

      if (!length && *values)
      {
        length= strlen(*values);
      }

      /* We need to pass that pointer to the sql_get_data so it could detect
         22002 error - for NULL values that pointer has to be supplied by user.
       */
      if (arrec->octet_length_ptr)
      {
        pcbValue= ptr_offset_adjust(arrec->octet_length_ptr, 
                                      stmt->ard->bind_offset_ptr, 
                                      stmt->ard->bind_type, 
                                      sizeof(SQLLEN), rownum);
      }

      tmp_res= sql_get_data(stmt, arrec->concise_type, (uint)i,
                            TargetValuePtr, arrec->octet_length, pcbValue,
                            *values, length, arrec);
      if (tmp_res != SQL_SUCCESS)
      {
        if (tmp_res == SQL_SUCCESS_WITH_INFO)
        {
          if (res == SQL_SUCCESS)
            res= tmp_res;
        }
        else
        {
          res= SQL_ERROR;
        }
      }
    }
  }

  return res;
}


/*
  @type    : myodbc3 internal
  @purpose : fetches the specified rowset of data from the result set and
  returns data for all bound columns. Rowsets can be specified
  at an absolute or relative position

  This function is way to long and needs to be structured.
*/
SQLRETURN SQL_API my_SQLExtendedFetch( SQLHSTMT             hstmt,
                                       SQLUSMALLINT         fFetchType,
                                       SQLLEN               irow,
                                       SQLULEN             *pcrow,
                                       SQLUSMALLINT FAR    *rgfRowStatus,
                                       my_bool              upd_status )
{
    SQLULEN           rows_to_fetch;
    long              cur_row, max_row;
    SQLULEN           i;
    SQLRETURN         row_res, res;
    STMT FAR          *stmt= (STMT *) hstmt;
    MYSQL_ROW         values= 0;
    MYSQL_ROW_OFFSET  save_position;
    SQLULEN           dummy_pcrow;
    BOOL              disconnected= FALSE;

    LINT_INIT(save_position);

    if ( !stmt->result )
      return set_stmt_error(stmt, "24000", "Fetch without a SELECT", 0);

    if (stmt->out_params_state > 0)
    {
      switch(--stmt->out_params_state)
      {
      case 0:
        return SQL_NO_DATA_FOUND;
      default:
        /* TODO: Need to remember real fetch' result*/
        /* just in case...*/
        stmt->out_params_state= 1;
      }
    }

    cur_row = stmt->current_row;

    if ( stmt->stmt_options.cursor_type == SQL_CURSOR_FORWARD_ONLY )
    {
      if ( fFetchType != SQL_FETCH_NEXT && !stmt->dbc->ds->safe )
        return  set_error(stmt,MYERR_S1106,
                          "Wrong fetchtype with FORWARD ONLY cursor", 0);
    }

    if ( if_dynamic_cursor(stmt) && set_dynamic_result(stmt) )
      return set_error(stmt,MYERR_S1000,
                       "Driver Failed to set the internal dynamic result", 0);

    if ( !pcrow )
      pcrow= &dummy_pcrow;

    /* for scrollable cursor("scroller") max_row is max row for currently
       fetched part of resultset */
    max_row= (long) num_rows(stmt);
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
                  (long)(stmt->current_row - stmt->ard->array_size));
        break;
      case SQL_FETCH_FIRST:
        cur_row= 0L;
        break;
      case SQL_FETCH_LAST:
        cur_row= max_row - stmt->ard->array_size;
        break;
      case SQL_FETCH_ABSOLUTE:
        if (irow < 0)
        {
          /* Fetch from end of result set */
          if ( max_row+irow < 0 && -irow <= (long) stmt->ard->array_size )
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
        if (stmt->current_row > 0 && cur_row < 0 &&
           (long)-irow <= (long)stmt->ard->array_size)
        {
          cur_row= 0;
        }
        break;

      default:
          return set_error(stmt, MYERR_S1106, "Fetch type out of range", 0);
    }

    if ( cur_row < 0 )
    {
      stmt->current_row= -1;  /* Before first row */
      stmt->rows_found_in_set= 0;
      data_seek(stmt, 0L);
      return SQL_NO_DATA_FOUND;
    }
    if ( cur_row > max_row )
    {
      if (scroller_exists(stmt))
      {
        while (cur_row > (max_row= (long)scroller_move(stmt)));

        switch (scroller_prefetch(stmt))
        {
          case SQL_NO_DATA: return SQL_NO_DATA_FOUND;
          case SQL_ERROR:   return set_error(stmt,MYERR_S1000,
                                            mysql_error(&stmt->dbc->mysql), 0);
        }
      }
      else
      {
        cur_row= max_row;
      }
    }

    if ( !stmt->result_array && !if_forward_cache(stmt) )
    {
        /*
          If Dynamic, it loses the stmt->end_of_set, so
          seek to desired row, might have new data or
          might be deleted
        */
        if ( stmt->stmt_options.cursor_type != SQL_CURSOR_DYNAMIC &&
             cur_row && cur_row == (long)(stmt->current_row +
                                          stmt->rows_found_in_set) )
            row_seek(stmt, stmt->end_of_set);
        else
            data_seek(stmt, cur_row);
    }
    stmt->current_row= cur_row;

    if (scroller_exists(stmt)
      || (if_forward_cache(stmt) && !stmt->result_array))
    {
      rows_to_fetch= stmt->ard->array_size;
    }
    else
    {
      rows_to_fetch= myodbc_min(max_row-cur_row,
                                (long)stmt->ard->array_size);
    }

    /* out params has been silently fetched */
    if (rows_to_fetch == 0)
    {
      if (stmt->out_params_state > 0)
      {
        rows_to_fetch= 1;
      }
      else
      {
        *pcrow= 0;
        stmt->rows_found_in_set= 0;
        if ( upd_status && stmt->ird->rows_processed_ptr )
        {
          *stmt->ird->rows_processed_ptr= 0;
        }
        return SQL_NO_DATA_FOUND;
      }
    }

    if (!stmt->dbc->ds->dont_use_set_locale)
    {
      setlocale(LC_NUMERIC, "C");
    }

    res= SQL_SUCCESS;
    for (i= 0 ; i < rows_to_fetch ; ++i)
    {
      if ( stmt->result_array )
      {
        values= stmt->result_array + cur_row*stmt->result->field_count;
        if ( i == 0 )
        {
          stmt->current_values= values;
        }
      }
      else
      {
        /* This code will ensure that values is always set */
        if ( i == 0 )
        {
            save_position= row_tell(stmt);
        }
        /* - Actual fetching happens here - */
        if ( stmt->out_params_state == 0
          && !(values= fetch_row(stmt)) )
        {
          if (scroller_exists(stmt))
          {
            scroller_move(stmt);

            row_res= scroller_prefetch(stmt);

            if (row_res != SQL_SUCCESS)
            {
              break;
            }

            if ( !(values= fetch_row(stmt)) )
            {
              break;
            }

            /* Not sure that is right, but see it better than nothing */
            save_position= row_tell(stmt);
          }
          else
          {
            break;
          }
        }

        if (stmt->out_params_state)
        {
          values= stmt->array;
        }

        if ( stmt->fix_fields )
        {
            values= (*stmt->fix_fields)(stmt,values);
        }

        stmt->current_values= values;
      }

      if (!stmt->fix_fields)
      {
        /* lengths contains lengths for all rows. Alternate use could be
           filling ird buffers in the (fix_fields) function. In this case
           lengths could contain just one array with rules for lengths
           calculating(it can work out in many cases like in catalog functions
           there some fields from results of auxiliary query are simply mixed
           somehow and constant fields added ).
           Another approach could be using of "array" and "order" arrays
           and special fix_fields callback, that will fix array and set
           lengths in ird*/
        if (stmt->lengths)
        {
          fill_ird_data_lengths(stmt->ird, stmt->lengths + cur_row*stmt->result->field_count,
                                stmt->result->field_count);
        }
        else
        {
          fill_ird_data_lengths(stmt->ird, fetch_lengths(stmt),
                                stmt->result->field_count);
        }
      }

      row_res= fill_fetch_buffers(stmt, values, i);

      /* For SQL_SUCCESS we need all rows to be SQL_SUCCESS */
      if (res != row_res)
      {
        /* Any successful row makes overall result SQL_SUCCESS_WITH_INFO */
        if (SQL_SUCCEEDED(row_res))
        {
          res= SQL_SUCCESS_WITH_INFO;
        }
        /* Else error */
        else if (i == 0)
        {
          /* SQL_ERROR only if all rows fail */
          res= SQL_ERROR;
        }
        else
        {
          res= SQL_SUCCESS_WITH_INFO;
        }
      }

      /* "Fetching" includes buffers filling. I think errors in that 
         have to affect row status */

      if (rgfRowStatus)
      {
        rgfRowStatus[i]= sqlreturn2row_status(row_res);
      }
      /*
        No need to update rowStatusPtr_ex, it's the same as rgfRowStatus.
      */
      if (upd_status && stmt->ird->array_status_ptr)
      {
        stmt->ird->array_status_ptr[i]= sqlreturn2row_status(row_res);
      }

      ++cur_row;
    }   /* fetching cycle end*/

    stmt->rows_found_in_set= i;
    *pcrow= i;

    disconnected= is_connection_lost(mysql_errno(&stmt->dbc->mysql))
      && handle_connection_error(stmt);

    if ( upd_status && stmt->ird->rows_processed_ptr )
    {
      *stmt->ird->rows_processed_ptr= i;
    }

    /* It is possible that both rgfRowStatus and array_status_ptr are set
    (and upp_status is TRUE) */
    for ( ; i < stmt->ard->array_size ; ++i )
    {
      if ( rgfRowStatus )
      {
        rgfRowStatus[i]= disconnected ? SQL_ROW_ERROR : SQL_ROW_NOROW;
      }
      /*
        No need to update rowStatusPtr_ex, it's the same as rgfRowStatus.
      */
      if ( upd_status && stmt->ird->array_status_ptr )
      {
        stmt->ird->array_status_ptr[i]= disconnected? SQL_ROW_ERROR
                                                    : SQL_ROW_NOROW;
      }
    }

    if (SQL_SUCCEEDED(res) && !stmt->result_array && !if_forward_cache(stmt))
    {
      /* reset result position */
      stmt->end_of_set= row_seek(stmt, save_position);
    }

    if (!stmt->dbc->ds->dont_use_set_locale)
    {
      setlocale(LC_NUMERIC,default_locale);
    }

    if (SQL_SUCCEEDED(res)
      && stmt->rows_found_in_set < stmt->ard->array_size)
    {
      if (disconnected)
      {
        return SQL_ERROR;
      }
      else if (stmt->rows_found_in_set == 0)
      {
        return SQL_NO_DATA_FOUND;
      }
    }

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
                                    SQLLEN          irow,
                                    SQLULEN        *pcrow,
                                    SQLUSMALLINT FAR *rgfRowStatus )
{
    SQLRETURN rc;
    SQLULEN rows;
    STMT_OPTIONS *options= &((STMT FAR *)hstmt)->stmt_options;

    options->rowStatusPtr_ex= rgfRowStatus;

    rc= my_SQLExtendedFetch(hstmt, fFetchType, irow, &rows, rgfRowStatus, 1);
    if (pcrow)
      *pcrow= (SQLULEN) rows;

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
    STMT *stmt = (STMT *)StatementHandle;
    STMT_OPTIONS *options= &stmt->stmt_options;

    options->rowStatusPtr_ex= NULL;

    return my_SQLExtendedFetch(StatementHandle, FetchOrientation, FetchOffset,
                               stmt->ird->rows_processed_ptr, stmt->ird->array_status_ptr,
                               0);
}

/*
  @type    : ODBC 1.0 API
  @purpose : fetches the next rowset of data from the result set and
  returns data for all bound columns
*/

SQLRETURN SQL_API SQLFetch(SQLHSTMT StatementHandle)
{
    STMT *stmt = (STMT *)StatementHandle;
    STMT_OPTIONS *options= &stmt->stmt_options;

    options->rowStatusPtr_ex= NULL;

    return my_SQLExtendedFetch(StatementHandle, SQL_FETCH_NEXT, 0,
                               stmt->ird->rows_processed_ptr, stmt->ird->array_status_ptr,
                               0);
}
