/* Copyright (C) 1995-2006 MySQL AB

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   There are special exceptions to the terms and conditions of the GPL as it
   is applied to this software. View the full text of the exception in file
   EXCEPTIONS in the directory of this software distribution.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/***************************************************************************
 * UTILITY.C                                   *
 *                                	   *
 * @description: Utility functions                	   *
 *                                	   *
 * @author     : MySQL AB(monty@mysql.com, venu@mysql.com)		   *
 * @date       : 2001-Aug-15                		   *
 * @product    : myodbc3                		   *
 *                                	   *
 ****************************************************************************/

#include "myodbc3.h"
#include "errmsg.h"
#include "m_ctype.h"

#if MYSQL_VERSION_ID >= 40100
    #undef USE_MB
#endif


/*
  @type    : myodbc internal
  @purpose : executes the specified sql statement
*/

SQLRETURN odbc_stmt(DBC FAR *dbc, const char *query)
{
    SQLRETURN result= SQL_SUCCESS;

    MYODBCDbgEnter;
    MYODBCDbgInfo( "stmt: %s", query );

    pthread_mutex_lock(&dbc->lock);
    if ( check_if_server_is_alive(dbc) ||
         mysql_real_query(&dbc->mysql,query,strlen(query)) )
    {
        result= set_conn_error(dbc,MYERR_S1000,mysql_error(&dbc->mysql),
                               mysql_errno(&dbc->mysql));
    }
    pthread_mutex_unlock(&dbc->lock);
    MYODBCDbgReturnReturn(result);
}

/*
  @type    : myodbc internal
  @purpose : use own fields instead of sql fields
*/

void mysql_link_fields(STMT *stmt,MYSQL_FIELD *fields,uint field_count)
{
    MYSQL_RES *result;
    pthread_mutex_lock(&stmt->dbc->lock);
    result= stmt->result;
    result->fields= fields;
    result->field_count= field_count;
    result->current_field= 0;
    fix_result_types(stmt);
    pthread_mutex_unlock(&stmt->dbc->lock);
}


/*
  @type    : myodbc internal
  @purpose : fixes the result types
*/

void fix_result_types(STMT *stmt)
{
    uint i;
    MYSQL_RES *result= stmt->result;

    MYODBCDbgEnter;

    stmt->state= ST_EXECUTED;  /* Mark set found */
    if ( (stmt->odbc_types= (SQLSMALLINT*)
          my_malloc(sizeof(SQLSMALLINT)*result->field_count, MYF(0))) )
    {
        for ( i= 0 ; i < result->field_count ; i++ )
        {
            MYSQL_FIELD *field= result->fields+i;
            stmt->odbc_types[i]= (SQLSMALLINT) unireg_to_c_datatype(field);
        }
    }
    /*                              
      Fix default values for bound columns
      Normally there isn't any bound columns at this stage !
    */
    if ( stmt->bind )
    {
        if ( stmt->bound_columns < result->field_count )
        {
            if ( !(stmt->bind= (BIND*) my_realloc((char*) stmt->bind,
                                                  sizeof(BIND) * result->field_count,
                                                  MYF(MY_FREE_ON_ERROR))) )
            {
                /* We should in principle give an error here */
                stmt->bound_columns= 0;
                MYODBCDbgReturnVoid;
            }
            bzero((gptr) (stmt->bind+stmt->bound_columns),
                  (result->field_count -stmt->bound_columns)*sizeof(BIND));
            stmt->bound_columns= result->field_count;
        }
        /* Fix default types and pointers to fields */

        mysql_field_seek(result,0);
        for ( i= 0; i < result->field_count ; i++ )
        {
            if ( stmt->bind[i].fCType == SQL_C_DEFAULT )
                stmt->bind[i].fCType= stmt->odbc_types[i];
            stmt->bind[i].field= mysql_fetch_field(result);
        }
    }
    MYODBCDbgReturnVoid;
}


/*
  @type    : myodbc internal
  @purpose : change a string + length to a zero terminated string
*/

char *fix_str(char *to,char *from,int length)
{
    if ( !from )
        return "";
    if ( length == SQL_NTS )
        return from;
    strmake(to,from,length);
    return to;
}


/*
  @type    : myodbc internal
  @purpose : duplicate the string
*/

char *dupp_str(char *from,int length)
{
    char *to;
    if ( !from )
        return my_strdup("",MYF(MY_WME));
    if ( length == SQL_NTS )
        length= strlen(from);
    if ( (to= my_malloc(length+1,MYF(MY_WME))) )
    {
        memcpy(to,from,length);
        to[length]= 0;
    }
    return to;
}


/*
  @type    : myodbc internal
  @purpose : returns 1 if from is a null pointer or a empty string
*/

bool empty_str(char *from,int length)
{
    if ( !from )
        return 1;
    if ( length == SQL_NTS )
        return from[0] == 0;
    return !length;
}


/*
  @type    : myodbc internal
  @purpose : copies the string data to rgbValue buffer. If rgbValue
  is NULL, then returns warning with full length, else
  copies the cbValueMax length from 'src' and returns it.
*/

SQLRETURN copy_str_data(SQLSMALLINT HandleType, SQLHANDLE Handle,
                        SQLCHAR FAR *rgbValue,
                        SQLSMALLINT cbValueMax,
                        SQLSMALLINT FAR *pcbValue,char FAR *src)
{
    SQLSMALLINT dummy;

    if ( !pcbValue )
        pcbValue= &dummy;

    if ( cbValueMax == SQL_NTS )
        cbValueMax= *pcbValue= strlen(src);

    else if ( cbValueMax < 0 )
        return set_handle_error(HandleType,Handle,MYERR_S1090,NULL,0);
    else
    {
        cbValueMax= cbValueMax ? cbValueMax - 1 : 0;
        *pcbValue= strlen(src);
    }

    if ( rgbValue )
        strmake((char*) rgbValue, src, cbValueMax);

    if ( min(*pcbValue , cbValueMax) != *pcbValue )
        return SQL_SUCCESS_WITH_INFO;
    return SQL_SUCCESS;
}


/*
  @type    : myodbc internal
  @purpose : returns (possibly truncated) results
  if result is truncated the result length contains
  length of the truncted result
*/

SQLRETURN
copy_lresult(SQLSMALLINT HandleType, SQLHANDLE Handle,
             SQLCHAR FAR *rgbValue, SQLINTEGER cbValueMax,
             SQLLEN *pcbValue,char *src,long src_length,
             long max_length,long fill_length,ulong *offset,
             my_bool binary_data)
{
    char *dst= (char*) rgbValue;
    ulong length;
    SQLINTEGER arg_length;

    if ( src && src_length == SQL_NTS )
        src_length= strlen(src);

    arg_length= cbValueMax;
    if ( cbValueMax && !binary_data )   /* If not length check */
        cbValueMax--;   /* Room for end null */
    else if ( !cbValueMax )
        dst= 0;     /* Don't copy anything! */
    if ( max_length )   /* If limit on char lengths */
    {
        set_if_smaller(cbValueMax,(long) max_length);
        set_if_smaller(src_length,max_length);
        set_if_smaller(fill_length,max_length);
    }
    if ( HandleType == SQL_HANDLE_DBC )
    {
        if ( fill_length < src_length || !Handle ||
             !(((DBC FAR*)Handle)->flag & FLAG_PAD_SPACE) )
            fill_length= src_length;
    }
    else
    {
        if ( fill_length < src_length || !Handle ||
             !(((STMT FAR*)Handle)->dbc->flag & FLAG_PAD_SPACE) )
            fill_length= src_length;
    }
    if ( *offset == (ulong) ~0L )
        *offset= 0;         /* First call */
    else if ( arg_length && *offset >= (ulong) fill_length )
        return SQL_NO_DATA_FOUND;

    src+= *offset;
    src_length-= (long) *offset;
    fill_length-= *offset;

    length= min(fill_length, cbValueMax);
    (*offset)+= length;        /* Fix for next call */
    if ( pcbValue )
        *pcbValue= fill_length;
    if ( dst )      /* Bind allows null pointers */
    {
        ulong copy_length= ((long) src_length >= (long) length ? length :
                            ((long) src_length >= 0 ? src_length : 0L));
        memcpy(dst,src,copy_length);
        bfill(dst+copy_length,length-copy_length,' ');
        if ( !binary_data || length != (ulong) cbValueMax )
            dst[length]= 0;
    }
    if ( arg_length && cbValueMax >= fill_length )
        return SQL_SUCCESS;
    MYODBCDbgInfo( "Returned %ld characters from", length );
    MYODBCDbgInfo( "offset: %lu", *offset - length );
    set_handle_error(HandleType,Handle,MYERR_01004,NULL,0);
    return SQL_SUCCESS_WITH_INFO;
}


/*
  @type    : myodbc internal
  @purpose : is used when converting a binary string to a SQL_C_CHAR
*/

SQLRETURN copy_binary_result( SQLSMALLINT   HandleType, 
                              SQLHANDLE     Handle,
                              SQLCHAR FAR * rgbValue,
                              SQLINTEGER    cbValueMax,
                              SQLLEN *      pcbValue,
                              char *        src,
                              ulong         src_length,
                              ulong         max_length,
                              ulong *       offset )
{
    char *dst= (char*) rgbValue;
    ulong length;
#if MYSQL_VERSION_ID >= 40100
    char NEAR _dig_vec[] =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
#endif

    if ( !cbValueMax )
        dst= 0;  /* Don't copy anything! */
    if ( max_length ) /* If limit on char lengths */
    {
        set_if_smaller(cbValueMax,(long) max_length+1);
        set_if_smaller(src_length,(max_length+1)/2);
    }
    if ( *offset == (ulong) ~0L )
        *offset= 0;   /* First call */
    else if ( *offset >= src_length )
        return SQL_NO_DATA_FOUND;
    src+= *offset;
    src_length-= *offset;
    length= cbValueMax ? (ulong)(cbValueMax-1)/2 : 0;
    length= min(src_length,length);
    (*offset)+= length;     /* Fix for next call */
    if ( pcbValue )
        *pcbValue= src_length*2;
    if ( dst )  /* Bind allows null pointers */
    {
        ulong i;
        for ( i= 0 ; i < length ; i++ )
        {
            *dst++= _dig_vec[(uchar) *src >> 4];
            *dst++= _dig_vec[(uchar) *src++ & 15];
        }
        *dst= 0;
    }
    if ( (ulong) cbValueMax > length*2 )
        return SQL_SUCCESS;
    MYODBCDbgInfo( "Returned %ld characters from", length );
    MYODBCDbgInfo( "offset: %ld", *offset - length );

    set_handle_error(HandleType,Handle,MYERR_01004,NULL,0);
    return SQL_SUCCESS_WITH_INFO;
}


/*
  @type    : myodbc internal
  @purpose : get type, transfer length and precision for a unireg column
  note that timestamp is changed to YYYY-MM-DD HH:MM:SS type

  SQLUINTEGER

*/

int unireg_to_sql_datatype(STMT FAR *stmt, MYSQL_FIELD *field, char *buff,
                           ulong *transfer_length, ulong *precision,
                           ulong *display_size)
{
    char *pos;
    my_bool field_is_binary= binary_field(field);
/* PAH - SESSION 01
    if ( stmt->dbc->flag & (FLAG_FIELD_LENGTH | FLAG_SAFE) )
*/        *transfer_length= *precision= *display_size= max(field->length,
                                                         field->max_length);
/* PAH - SESSION 01
    else

        *transfer_length= *precision= *display_size= field->max_length;
*/

/* PAH - SESSION 01
printf( "[PAH][%s][%d][%s] field->type=%d field_is_binary=%d\n", __FILE__, __LINE__, __FUNCTION__, field->type, field_is_binary );
*/
    switch ( field->type )
    {
        case FIELD_TYPE_BIT:
            if ( buff )
            {
                pos= strmov(buff,"bit");
            }
            *transfer_length= 1;
            return SQL_BIT;  

        case FIELD_TYPE_DECIMAL:
        case FIELD_TYPE_NEWDECIMAL:
            *display_size= max(field->length,field->max_length) -
                           test(!(field->flags & UNSIGNED_FLAG)) -
                           test(field->decimals);
            *precision= *display_size;
            if ( buff ) strmov(buff,"decimal");
            return SQL_DECIMAL;

        case FIELD_TYPE_CHAR:
            if ( num_field(field) )
            {
                if ( buff )
                {
                    pos= strmov(buff,"tinyint");
                    if ( field->flags & UNSIGNED_FLAG )
                        strmov(pos," unsigned");
                }
                *transfer_length= 1;
                return SQL_TINYINT;
            }
            if ( buff )
            {
                pos= strmov(buff,"char");
                if ( field->flags & UNSIGNED_FLAG )
                    strmov(pos," unsigned");
            }
            *transfer_length= 1;
            return SQL_CHAR;

        case FIELD_TYPE_SHORT:
            if ( buff )
            {
                pos= strmov(buff,"smallint");
                if ( field->flags & UNSIGNED_FLAG )
                    strmov(pos," unsigned");
            }
            *transfer_length= 2;
            return SQL_SMALLINT;

        case FIELD_TYPE_INT24:
            if ( buff )
            {
                pos= strmov(buff,"mediumint");
                if ( field->flags & UNSIGNED_FLAG )
                    strmov(pos," unsigned");
            }
            *transfer_length= 4;
            return SQL_INTEGER;

        case FIELD_TYPE_LONG:
            if ( buff )
            {
                pos= strmov(buff,"integer");
                if ( field->flags & UNSIGNED_FLAG )
                    strmov(pos," unsigned");
            }
            *transfer_length= 4;
            return SQL_INTEGER;

        case FIELD_TYPE_LONGLONG:
            if ( buff )
            {
                pos= strmov(buff,"bigint");
                if ( field->flags & UNSIGNED_FLAG )
                    strmov(pos," unsigned");
            }
            *transfer_length= 20;
            if ( stmt->dbc->flag & FLAG_NO_BIGINT )
                return SQL_INTEGER;
            if ( field->flags & UNSIGNED_FLAG )
                *transfer_length= *precision= 20;
            else
                *transfer_length= *precision= 19;
            return SQL_BIGINT;

        case FIELD_TYPE_FLOAT:
            if ( buff )
            {
                pos= strmov(buff,"float");
                if ( field->flags & UNSIGNED_FLAG )
                    strmov(pos," unsigned");
            }
            *transfer_length= 4;
            return SQL_REAL;
        case FIELD_TYPE_DOUBLE:
            if ( buff )
            {
                pos= strmov(buff,"double");
                if ( field->flags & UNSIGNED_FLAG )
                    strmov(pos," unsigned");
            }
            *transfer_length= 8;
            return SQL_DOUBLE;

        case FIELD_TYPE_NULL:
            if ( buff ) strmov(buff,"null");
            return SQL_VARCHAR;

        case FIELD_TYPE_YEAR:
            if ( buff )
                pos= strmov(buff,"year");
            *transfer_length= 2;
            return SQL_SMALLINT;

        case FIELD_TYPE_TIMESTAMP:
            if ( buff ) strmov(buff,"timestamp");
            *transfer_length= 16;      /* size of timestamp_struct */
            *precision= *display_size= 19;
            if ( stmt->dbc->env->odbc_ver == SQL_OV_ODBC3 )
                return SQL_TYPE_TIMESTAMP;
            return SQL_TIMESTAMP;

        case FIELD_TYPE_DATETIME:
            if ( buff ) strmov(buff,"datetime");
            *transfer_length= 16;      /* size of timestamp_struct */
            *precision= *display_size= 19;
            if ( stmt->dbc->env->odbc_ver == SQL_OV_ODBC3 )
                return SQL_TYPE_TIMESTAMP;
            return SQL_TIMESTAMP;

        case FIELD_TYPE_NEWDATE:
        case FIELD_TYPE_DATE:
            if ( buff ) strmov(buff,"date");
            *transfer_length= 6;       /* size of date struct */
            *precision= *display_size= 10;
            if ( stmt->dbc->env->odbc_ver == SQL_OV_ODBC3 )
                return SQL_TYPE_DATE;
            return SQL_DATE;

        case FIELD_TYPE_TIME:
            if ( buff ) strmov(buff,"time");
            *transfer_length= 6;       /* size of time struct */
            *precision= *display_size= 8;
            if ( stmt->dbc->env->odbc_ver == SQL_OV_ODBC3 )
                return SQL_TYPE_TIME;
            return SQL_TIME;

        case FIELD_TYPE_STRING:
            /* Binary flag is for handling "VARCHAR() BINARY" but is unreliable (see BUG-4578) - PAH */
            if (field_is_binary)
            {
              if (buff) strmov(buff,"binary");
              return SQL_BINARY;
            }

            *transfer_length= *precision= *display_size= field->length ? 
                (stmt->dbc->mysql.charset ? 
                field->length/stmt->dbc->mysql.charset->mbmaxlen: field->length): 255;
            if ( buff ) strmov(buff,"char");
            return SQL_CHAR;

        case FIELD_TYPE_VAR_STRING:
            
            if(field->table && field->org_table && field->table[0] && field->org_table[0])
            {
                *transfer_length= *precision= *display_size= field->length ? 
                (stmt->dbc->mysql.charset ? 
                field->length/stmt->dbc->mysql.charset->mbmaxlen: field->length): 255;

                /* Binary flag is for handling "VARCHAR() BINARY" but is unreliable (see BUG-4578) - PAH */
                if (field_is_binary)
                {
                    if (buff) strmov(buff,"varbinary");
                        return SQL_VARBINARY;
                }
                if ( buff ) strmov(buff,"varchar");
                    return SQL_VARCHAR;
            }
            else
            {
                *transfer_length= *precision= *display_size= 16777216L;
                if (field_is_binary)
                {
                    if (buff) strmov(buff,"varbinary");
                    return SQL_LONGVARBINARY;
                }

                if ( buff ) strmov(buff,"varchar");
                    return SQL_LONGVARCHAR;
            }

        case FIELD_TYPE_TINY_BLOB:
            if ( buff )
                strmov(buff,(field_is_binary) ? "tinyblob" : "tinytext");
/* PAH - SESSION 01
            if ( stmt->dbc->flag & (FLAG_FIELD_LENGTH | FLAG_SAFE) )
*/
                *transfer_length= *precision= *display_size= field->length ? 
                (stmt->dbc->mysql.charset ?
                field->length/stmt->dbc->mysql.charset->mbmaxlen: field->length): 255;
            return(field_is_binary) ? SQL_LONGVARBINARY : SQL_LONGVARCHAR;

        case FIELD_TYPE_BLOB:
            if ( buff )
                strmov( buff, (field_is_binary) ? "blob" : "text" );

/* PAH - SESSION 01
            if ( stmt->dbc->flag & (FLAG_FIELD_LENGTH | FLAG_SAFE) ) 
*/
                *transfer_length= *precision= *display_size= field->length ? (stmt->dbc->mysql.charset ? field->length/stmt->dbc->mysql.charset->mbmaxlen: field->length): 65535;

            return ( field_is_binary ) ? SQL_LONGVARBINARY : SQL_LONGVARCHAR;

        case FIELD_TYPE_MEDIUM_BLOB:
            if ( buff )
                strmov(buff,((field_is_binary) ? "mediumblob" :
                             "mediumtext"));
/* PAH - SESSION 01
            if ( stmt->dbc->flag & (FLAG_FIELD_LENGTH | FLAG_SAFE) )
*/
                *transfer_length= *precision= *display_size= field->length ? 
                (stmt->dbc->mysql.charset ?
                field->length/stmt->dbc->mysql.charset->mbmaxlen: field->length): (1L << 24)-1L;
            return(field_is_binary) ? SQL_LONGVARBINARY : SQL_LONGVARCHAR;

        case FIELD_TYPE_LONG_BLOB:
            if ( buff )
                strmov(buff,((field_is_binary) ? "longblob": "longtext"));
/* PAH - SESSION 01
            if ( stmt->dbc->flag & (FLAG_FIELD_LENGTH | FLAG_SAFE) )
*/
                *transfer_length= *precision= *display_size= field->length ? 
                (stmt->dbc->mysql.charset ?
                field->length/stmt->dbc->mysql.charset->mbmaxlen: field->length): INT_MAX32;
            return(field_is_binary) ? SQL_LONGVARBINARY : SQL_LONGVARCHAR;

        case FIELD_TYPE_ENUM:
            if ( buff )
                strmov(buff,"enum");
            return SQL_CHAR;

        case FIELD_TYPE_SET:
            if ( buff )
                strmov(buff,"set");
            return SQL_CHAR;

        case FIELD_TYPE_GEOMETRY:
            if ( buff )
                strmov(buff,"blob");
            return SQL_LONGVARBINARY;
    }
    return 0; /* Impossible */
}


/*
  @type    : myodbc internal
  @purpose : returns internal type to C type
*/

int unireg_to_c_datatype(MYSQL_FIELD *field)
{
    switch ( field->type )
    {
        case FIELD_TYPE_LONGLONG: /* Must be returned as char */
        default:
            return SQL_C_CHAR;
        case FIELD_TYPE_BIT:
            return SQL_C_BIT;
        case FIELD_TYPE_CHAR:
            return SQL_C_TINYINT;
        case FIELD_TYPE_YEAR:
        case FIELD_TYPE_SHORT:
            return SQL_C_SHORT;
        case FIELD_TYPE_INT24:
        case FIELD_TYPE_LONG:
            return SQL_C_LONG;
        case FIELD_TYPE_FLOAT:
            return SQL_C_FLOAT;
        case FIELD_TYPE_DOUBLE:
            return SQL_C_DOUBLE;
        case FIELD_TYPE_TIMESTAMP:
        case FIELD_TYPE_DATETIME:
            return SQL_C_TIMESTAMP;
        case FIELD_TYPE_NEWDATE:
        case FIELD_TYPE_DATE:
            return SQL_C_DATE;
        case FIELD_TYPE_TIME:
            return SQL_C_TIME;
        case FIELD_TYPE_BLOB:
        case FIELD_TYPE_TINY_BLOB:
        case FIELD_TYPE_MEDIUM_BLOB:
        case FIELD_TYPE_LONG_BLOB:
            return SQL_C_BINARY;
    }
}


/*
  @type    : myodbc internal
  @purpose : returns default C type for a given SQL type
*/

int default_c_type(int sql_data_type)
{
    switch ( sql_data_type )
    {
        case SQL_CHAR:
        case SQL_VARCHAR:
        case SQL_LONGVARCHAR:
        case SQL_DECIMAL:
        case SQL_NUMERIC:
        default:
            return SQL_C_CHAR;
        case SQL_BIGINT:
            return SQL_C_SBIGINT;
        case SQL_BIT:
            return SQL_C_BIT;
        case SQL_TINYINT:
            return SQL_C_TINYINT;
        case SQL_SMALLINT:
            return SQL_C_SHORT;
        case SQL_INTEGER:
            return SQL_C_LONG;
        case SQL_REAL:
        case SQL_FLOAT:
            return SQL_C_FLOAT;
        case SQL_DOUBLE:
            return SQL_C_DOUBLE;
        case SQL_BINARY:
        case SQL_VARBINARY:
        case SQL_LONGVARBINARY:
            return SQL_C_BINARY;
        case SQL_DATE:
        case SQL_TYPE_DATE:
            return SQL_C_DATE;
        case SQL_TIME:
        case SQL_TYPE_TIME:
            return SQL_C_TIME;
        case SQL_TIMESTAMP:
        case SQL_TYPE_TIMESTAMP:
            return SQL_C_TIMESTAMP;
    }
}


/*
  @type    : myodbc internal
  @purpose : returns bind length
*/

ulong bind_length(int sql_data_type,ulong length)
{
    switch ( sql_data_type )
    {
        default:                  /* For CHAR, VARCHAR, BLOB...*/
            return length;
        case SQL_C_BIT:
        case SQL_C_TINYINT:
        case SQL_C_STINYINT:
        case SQL_C_UTINYINT:
            return 1;
        case SQL_C_SHORT:
        case SQL_C_SSHORT:
        case SQL_C_USHORT:
            return 2;
        case SQL_C_LONG:
        case SQL_C_SLONG:
        case SQL_C_ULONG:
            return sizeof(SQLINTEGER);
        case SQL_C_FLOAT:
            return sizeof(float);
        case SQL_C_DOUBLE:
            return sizeof(double);
        case SQL_C_DATE:
        case SQL_C_TYPE_DATE:
            return sizeof(DATE_STRUCT);
        case SQL_C_TIME:
        case SQL_C_TYPE_TIME:
            return sizeof(TIME_STRUCT);
        case SQL_C_TIMESTAMP:
        case SQL_C_TYPE_TIMESTAMP:
            return sizeof(TIMESTAMP_STRUCT);
        case SQL_C_SBIGINT:
        case SQL_C_UBIGINT:
            return sizeof(longlong);
    }
}

/*
  @type    : myodbc internal
  @purpose : convert a possible string to a timestamp value
*/

my_bool str_to_ts(SQL_TIMESTAMP_STRUCT *ts, const char *str)
{ 
    uint year, length;
    char buff[15],*to;
    SQL_TIMESTAMP_STRUCT tmp_timestamp;

    if ( !ts )
        ts= (SQL_TIMESTAMP_STRUCT *) &tmp_timestamp;

    for ( to= buff ; *str && to < buff+sizeof(buff)-1 ; str++ )
    {
        if ( isdigit(*str) )
            *to++= *str;
    }

    length= (uint) (to-buff);

    if ( length == 6 || length == 12 )  /* YYMMDD or YYMMDDHHMMSS */
    {
        bmove_upp(to+2,to,length);
        if ( buff[0] <= '6' )
        {
            buff[0]='2';
            buff[1]='0';
        }
        else
        {
            buff[0]='1';
            buff[1]='9';
        }
        length+= 2;
        to+= 2;
    }

    if ( length < 14 )
        strfill(to,14 - length,'0');
    else
        *to= 0;

    year= (digit(buff[0])*1000+digit(buff[1])*100+digit(buff[2])*10+digit(buff[3]));

    if ( buff[4] == '0' && buff[5] == '0' )
        return 1;

    ts->year=   year;
    ts->month=  digit(buff[4])*10+digit(buff[5]);
    ts->day=    digit(buff[6])*10+digit(buff[7]);
    ts->hour=   digit(buff[8])*10+digit(buff[9]);
    ts->minute= digit(buff[10])*10+digit(buff[11]);
    ts->second= digit(buff[12])*10+digit(buff[13]);
    ts->fraction= 0;
    return 0;
}

/*
  @type    : myodbc internal
  @purpose : convert a possible string to a time value
*/

my_bool str_to_time_st(SQL_TIME_STRUCT *ts, const char *str)
{ 
    char buff[12],*to;
    SQL_TIME_STRUCT tmp_time;

    if ( !ts )
        ts= (SQL_TIME_STRUCT *) &tmp_time;

    for ( to= buff ; *str && to < buff+sizeof(buff)-1 ; str++ )
    {
        if ( isdigit(*str) )
            *to++= *str;
    }

    ts->hour=   digit(buff[0])*10+digit(buff[1]);
    ts->minute= digit(buff[2])*10+digit(buff[3]);
    ts->second= digit(buff[4])*10+digit(buff[5]);
    return 0;
}

/*
  @type    : myodbc internal
  @purpose : convert a possible string to a data value
*/

my_bool str_to_date(SQL_DATE_STRUCT *rgbValue, const char *str,uint length)
{
    uint field_length,year_length,digits,i,date[3];
    const char *pos;
    const char *end= str+length;
    for ( ; !isdigit(*str) && str != end ; str++ ) ;
    /*
      Calculate first number of digits.
      If length= 4, 8 or >= 14 then year is of format YYYY
      (YYYY-MM-DD,  YYYYMMDD)
    */
    for ( pos= str; pos != end && isdigit(*pos) ; pos++ ) ;
    digits= (uint) (pos-str);
    year_length= (digits == 4 || digits == 8 || digits >= 14) ? 4 : 2;
    field_length= year_length-1;

    for ( i= 0 ; i < 3 && str != end; i++ )
    {
        uint tmp_value= (uint) (uchar) (*str++ - '0');
        while ( str != end && isdigit(str[0]) && field_length-- )
        {
            tmp_value= tmp_value*10 + (uint) (uchar) (*str - '0');
            str++;
        }
        date[i]= tmp_value;
        while ( str != end && !isdigit(*str) )
            str++;
        field_length= 1;   /* Rest fields can only be 2 */
    }
    if ( i <= 1 || date[1] == 0 )   /* Wrong date */
        return 1;
    while ( i < 3 )
        date[i++]= 1;

    rgbValue->year=  date[0];
    rgbValue->month= date[1];
    rgbValue->day=   date[2];
    return 0;
}


/*
  @type    : myodbc internal
  @purpose : convert a time string to a (ulong) value.
  At least following formats are recogniced
  HHMMSS HHMM HH HH.MM.SS	 {t HH:MM:SS }
  @return  : HHMMSS
*/

ulong str_to_time_as_long(const char *str,uint length)
{
    uint i,date[3];
    const char *end= str+length;

    if ( length == 0 )
        return 0;

    for ( ; !isdigit(*str) && str != end ; str++ ) length--;

    for ( i= 0 ; i < 3 && str != end; i++ )
    {
        uint tmp_value= (uint) (uchar) (*str++ - '0');
        length--;

        while ( str != end && isdigit(str[0]) )
        {
            tmp_value= tmp_value*10 + (uint) (uchar) (*str - '0');
            str++; 
            length--;
        }
        date[i]= tmp_value;
        while ( str != end && !isdigit(*str) )
        {
            str++;
            length--;
        }
    }
    if ( length && str != end )
        return str_to_time_as_long(str, length);/* timestamp format */

    if ( date[0] > 10000L || i < 3 )    /* Properly handle HHMMSS format */
        return(ulong) date[0];

    return(ulong) date[0] * 10000L + (ulong) (date[1]*100L+date[2]);
}


/*
  @type    : myodbc internal
  @purpose : if there was a long time since last question, check that
  the server is up with mysql_ping (to force a reconnect)
*/

int check_if_server_is_alive( DBC FAR *dbc )
{
    time_t seconds= (time_t) time( (time_t*)0 );
    int result= 0;

    if ( (ulong)(seconds - dbc->last_query_time) >= CHECK_IF_ALIVE )
    {
        if ( mysql_ping( &dbc->mysql ) )
        {
            /*  BUG: 14639

                A. The 4.1 doc says when mysql_ping() fails we can get one
                of the following errors from mysql_errno();

                    CR_COMMANDS_OUT_OF_SYNC
                    CR_SERVER_GONE_ERROR
                    CR_UNKNOWN_ERROR   

                But if you do a mysql_ping() after bringing down the server
                you get CR_SERVER_LOST.

                PAH - 9.MAR.06
            */
            
            if ( mysql_errno( &dbc->mysql ) == CR_SERVER_LOST )
                result = 1;
        }
    }
    dbc->last_query_time = seconds;

    return result;
}


/*
  @type    : myodbc3 internal
  @purpose : appends quoted string to dynamic string
*/

my_bool dynstr_append_quoted_name(DYNAMIC_STRING *str, const char *name)
{
    uint tmp= strlen(name);
    char *pos;
    if ( dynstr_realloc(str,tmp+3) )
        return 1;
    pos= str->str+str->length;
    *pos='`';
    memcpy(pos+1,name,tmp);
    pos[tmp+1]='`';
    pos[tmp+2]= 0;        /* Safety */
    str->length+= tmp+2;
    return 0;
}


/*
  @type    : myodbc3 internal
  @purpose : reset the db name to current_database()
*/

my_bool reget_current_catalog(DBC FAR *dbc)
{
    my_free((gptr) dbc->database,MYF(0));
    if ( odbc_stmt(dbc, "select database()") )
    {
        return 1;
    }
    else
    {
        MYSQL_RES *res;
        MYSQL_ROW row;

        if ( (res= mysql_store_result(&dbc->mysql)) &&
             (row= mysql_fetch_row(res)) )
        {
/*            if (cmp_database(row[0], dbc->database)) */
            {
                if ( row[0] )
                    dbc->database = my_strdup(row[0], MYF(MY_WME));
                else
                    dbc->database = strdup( "null" );
            }
        }
        mysql_free_result(res);
    }

    return 0;
}


/*
  @type    : myodbc internal
  @purpose : compare strings without regarding to case
*/

int myodbc_strcasecmp(const char *s, const char *t)
{
#ifdef USE_MB
    if ( use_mb(default_charset_info) )
    {
        register uint32 l;
        register const char *end= s+strlen(s);
        while ( s<end )
        {
            if ( (l= my_ismbchar(default_charset_info, s,end)) )
            {
                while ( l-- )
                    if ( *s++ != *t++ ) return 1;
            }
            else if ( my_ismbhead(default_charset_info, *t) ) return 1;
            else if ( toupper((uchar) *s++) != toupper((uchar) *t++) ) return 1;
        }
        return *t;
    }
    else
#endif
    {
        while ( toupper((uchar) *s) == toupper((uchar) *t++) )
            if ( !*s++ ) return 0;
        return((int) toupper((uchar) s[0]) - (int) toupper((uchar) t[-1]));
    }
}


/*
  @type    : myodbc internal
  @purpose : compare strings without regarding to case
*/

int myodbc_casecmp(const char *s, const char *t, uint len)
{
#ifdef USE_MB
    if ( use_mb(default_charset_info) )
    {
        register uint32 l;
        register const char *end= s+len;
        while ( s<end )
        {
            if ( (l= my_ismbchar(default_charset_info, s,end)) )
            {
                while ( l-- )
                    if ( *s++ != *t++ ) return 1;
            }
            else if ( my_ismbhead(default_charset_info, *t) ) return 1;
            else if ( toupper((uchar) *s++) != toupper((uchar) *t++) ) return 1;
        }
        return 0;
    }
    else
#endif /* USE_MB */
    {
        while ( len-- != 0 && toupper(*s++) == toupper(*t++) ) ;
        return(int) len+1;
    }
}


/*
  @type    : myodbc3 internal
  @purpose : logs the queries sent to server
*/

#ifdef MYODBC_DBG
void query_print(FILE *log_file,char *query)
{
    if ( log_file && query )
        fprintf(log_file, "%s;\n",query);
}


FILE *init_query_log(void)
{
    FILE *query_log;

    if ( query_log= fopen(DRIVER_QUERY_LOGFILE, "w") )
    {
        fprintf(query_log,"-- Query logging\n");
        fprintf(query_log,"--\n");
        fprintf(query_log,"--  Driver name: %s  Version: %s\n",DRIVER_NAME,
                DRIVER_VERSION);
#ifdef HAVE_LOCALTIME_R
        {
            time_t now= time(NULL);
            struct tm start;
            localtime_r(&now,&start);

            fprintf(query_log,"-- Timestamp: %02d%02d%02d %2d:%02d:%02d\n",
                    start.tm_year % 100,
                    start.tm_mon+1,
                    start.tm_mday,
                    start.tm_hour,
                    start.tm_min,
                    start.tm_sec);
#endif /* HAVE_LOCALTIME_R */
            fprintf(query_log,"\n");
        }
    }
    return query_log;
}


void end_query_log(FILE *query_log)
{
    if ( query_log )
    {
        fclose(query_log);
        query_log= 0;
    }
}

#else
void query_print(char *query __attribute__((unused)))
{
}
#endif /* !DBUG_OFF */


my_bool is_minimum_version(const char *server_version,const char *version,
                           uint length)
{
    if ( strncmp(server_version,version,length) >= 0 )
        return TRUE;
    return FALSE;
}

#ifndef _UNIX_


/*****************************************************************************
 Define functions that dosen't exist in a dll
*****************************************************************************/

/* _exit is called by safemalloc, mystatic & my_malloc */
    #ifndef __WIN__
void exit(int exit)
{
    abort();
}
    #endif /* !__WIN__ */

/* perror is called by dbug.c */
/*
void perror(const char *str)
{
}
*/
/* clock is called by dbug.c when profiling */
/*
long clock(void)
{
    return 0L;
}
*/

    #ifndef THREAD
long getpid()
{
    return 0;
}
    #else

int pthread_dummy(int return_value)
{
    return return_value;
}
    #endif /* !THREAD */

#endif /* !_UNIX_ */
