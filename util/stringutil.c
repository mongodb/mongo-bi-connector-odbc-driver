/*
  Copyright (c) 2007, 2012, Oracle and/or its affiliates. All rights reserved.

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
  @file  stringutil.c
  @brief String utility functions, mostly focused on SQLWCHAR and charset
         manipulations.
*/


#include "stringutil.h"


CHARSET_INFO *utf8_charset_info= NULL;


/**
  Duplicate a SQLCHAR in the specified character set as a SQLWCHAR.

  @param[in]      charset_info  Character set to convert into
  @param[in]      str           String to convert
  @param[in,out]  len           Pointer to length of source (in bytes) or
                                destination string (in chars)
  @param[out]     errors        Pointer to count of errors in conversion

  @return  Pointer to a newly allocated SQLWCHAR, or @c NULL
*/
SQLWCHAR *sqlchar_as_sqlwchar(CHARSET_INFO *charset_info, SQLCHAR *str,
                              SQLINTEGER *len, uint *errors)
{
  SQLCHAR *pos, *str_end;
  SQLWCHAR *out;
  SQLINTEGER i, out_bytes;
  my_bool free_str= FALSE;

  if (str && *len == SQL_NTS)
    *len= strlen((char *)str);

  if (!str || *len == 0)
  {
    *len= 0;
    return NULL;
  }

  if (!is_utf8_charset(charset_info->number))
  {
    uint32 used_bytes, used_chars;
    size_t u8_max= (*len / charset_info->mbminlen *
                    utf8_charset_info->mbmaxlen + 1);
    SQLCHAR *u8= (SQLCHAR *)my_malloc(u8_max, MYF(0));

    if (!u8)
    {
      *len= -1;
      return NULL;
    }

    *len= copy_and_convert((char *)u8, u8_max, utf8_charset_info,
                           (char *)str, *len, charset_info,
                           &used_bytes, &used_chars, errors);
    str= u8;
    free_str= TRUE;
  }

  str_end= str + *len;

  out_bytes= (*len + 1) * sizeof(SQLWCHAR);

  out= (SQLWCHAR *)my_malloc(out_bytes, MYF(0));
  if (!out)
  {
    *len= -1;
    return NULL;
  }

  for (pos= str, i= 0; *pos && pos < str_end; )
  {
    if (sizeof(SQLWCHAR) == 4)
    {
      int consumed= utf8toutf32(pos, (UTF32 *)(out + i++));
      pos+= consumed;
      if (!consumed)
      {
        *errors+= 1;
        break;
      }
    }
    else
    {
      UTF32 u32;
      int consumed= utf8toutf32(pos, &u32);
      pos+= consumed;
      if (!consumed)
      {
        *errors+= 1;
        break;
      }
      i+= utf32toutf16(u32, (UTF16 *)(out + i));
    }
  }

  *len= i;
  out[i]= 0;

  if (free_str)
    x_free(str);

  return out;
}


/**
  Duplicate a SQLWCHAR as a SQLCHAR in the specified character set.

  @param[in]      charset_info  Character set to convert into
  @param[in]      str           String to convert
  @param[in,out]  len           Pointer to length of source (in chars) or
                                destination string (in bytes)
  @param[out]     errors        Pointer to count of errors in conversion

  @return  Pointer to a newly allocated SQLCHAR, or @c NULL
*/
SQLCHAR *sqlwchar_as_sqlchar(CHARSET_INFO *charset_info, SQLWCHAR *str,
                             SQLINTEGER *len, uint *errors)
{
  SQLWCHAR *str_end;
  SQLCHAR *out;
  SQLINTEGER i, u8_len, out_bytes;
  UTF8 u8[MAX_BYTES_PER_UTF8_CP + 1];
  uint32 used_bytes, used_chars;

  *errors= 0;

  if (is_utf8_charset(charset_info->number))
    return sqlwchar_as_utf8(str, len);

  if (*len == SQL_NTS)
    *len= sqlwcharlen(str);
  if (!str || *len == 0)
  {
    *len= 0;
    return NULL;
  }

  out_bytes= *len * charset_info->mbmaxlen * sizeof(SQLCHAR) + 1;
  out= (SQLCHAR *)my_malloc(out_bytes, MYF(0));
  if (!out)
  {
    *len= -1;
    return NULL;
  }

  str_end= str + *len;

  for (i= 0; str < str_end; )
  {
    if (sizeof(SQLWCHAR) == 4)
    {
      u8_len= utf32toutf8((UTF32)*str++, u8);
    }
    else
    {
      UTF32 u32;
      int consumed= utf16toutf32((UTF16 *)str, &u32);
      str+= consumed;
      if (!consumed)
      {
        *errors+= 1;
        break;
      }
      u8_len= utf32toutf8(u32, u8);
    }

    i+= copy_and_convert((char *)out + i, out_bytes - i, charset_info,
                         (char *)u8, u8_len, utf8_charset_info, &used_bytes,
                         &used_chars, errors);
  }

  *len= i;
  out[i]= '\0';
  return out;
}


/**
  A bit extended version of sqlwchar_as_utf8 to be used by it and in other
  places where sqlwchar_as_utf8 could not be used

  @param[in]      str           String to convert
  @param[in,out]  len           Pointer to length of source (in chars) or
                                destination string (in bytes). 
  @param[in]      buff          Buffer to put the result string if it fits
  @param[in]      buff_max      max size(in bytes) of the buff.
  @param[out]     utf8mb4_used  has 4 bytes utf8 characters been used

  @return  Pointer to a newly allocated SQLCHAR, or @c NULL
*/
SQLCHAR *sqlwchar_as_utf8_ext(const SQLWCHAR *str, SQLINTEGER *len,
                              SQLCHAR *buff, uint buff_max, int *utf8mb4_used)
{
  const SQLWCHAR *str_end;
  UTF8 *u8;
  int utf8len, dummy;
  SQLINTEGER i;

  if (!str || *len <= 0)
  {
    *len= 0;
    return buff;
  }

  if (utf8mb4_used == NULL)
  {
    utf8mb4_used= &dummy;
  }

  if (buff == NULL || buff_max < (uint)(*len * MAX_BYTES_PER_UTF8_CP))
  {
    u8= (UTF8 *)my_malloc(sizeof(UTF8) * MAX_BYTES_PER_UTF8_CP * *len + 1,
                        MYF(0));
  }
  else
  {
    u8= buff;
  }

  if (!u8)
  {
    *len= -1;
    return NULL;
  }

  str_end= str + *len;

  if (sizeof(SQLWCHAR) == 4)
  {
    for (i= 0; str < str_end; )
    {
      i+= (utf8len= utf32toutf8((UTF32)*str++, u8 + i));

      if (utf8len)
      {
        *utf8mb4_used= 1;
      }
    }
  }
  else
  {
    for (i= 0; str < str_end; )
    {
      UTF32 u32;
      int consumed= utf16toutf32((UTF16 *)str, &u32);
      if (!consumed)
      {
        break;
      }

      str+= consumed;

      i+= (utf8len= utf32toutf8(u32, u8 + i));
      if (utf8len)
      {
        *utf8mb4_used= 1;
      }
    }
  }

  *len= i;
  return u8;
}


/**
  Duplicate a SQLWCHAR as a SQLCHAR encoded as UTF-8.

  @param[in]      str           String to convert
  @param[in,out]  len           Pointer to length of source (in chars) or
                                destination string (in bytes)

  @return  Pointer to a newly allocated SQLCHAR, or @c NULL
*/
SQLCHAR *sqlwchar_as_utf8(const SQLWCHAR *str, SQLINTEGER *len)
{
  SQLCHAR *res;

  if (*len == SQL_NTS)
  {
    *len= sqlwcharlen(str);
  }

  if (!str || *len <= 0)
  {
    *len= 0;
    return NULL;
  }

  res= sqlwchar_as_utf8_ext(str, len, NULL, 0, NULL);

  /* If we could not allocate memory */
  if (res != NULL)
  {
    res[*len]= '\0';
  }

  return res;
}


/**
  Convert a SQLCHAR encoded as UTF-8 into a SQLWCHAR.

  @param[out]     out           Pointer to SQLWCHAR buffer
  @param[in]      out_max       Length of @c out buffer
  @param[in]      in            Pointer to SQLCHAR string (utf-8 encoded)
  @param[in]      in_len        Length of @c in (in bytes)

  @return  Number of characters stored in the @c out buffer
*/
SQLSMALLINT utf8_as_sqlwchar(SQLWCHAR *out, SQLINTEGER out_max, SQLCHAR *in,
                             SQLINTEGER in_len)
{
  SQLINTEGER i;
  SQLWCHAR *pos, *out_end;

  for (i= 0, pos= out, out_end= out + out_max; i < in_len && pos < out_end; )
  {
    if (sizeof(SQLWCHAR) == 4)
    {
      int consumed= utf8toutf32(in + i, (UTF32 *)pos++);
      i+= consumed;
      if (!consumed)
        break;
    }
    else
    {
      UTF32 u32;
      int consumed= utf8toutf32(in + i, &u32);
      i+= consumed;
      if (!consumed)
        break;
      pos+= utf32toutf16(u32, (UTF16 *)pos);
    }
  }

  if (pos)
    *pos= 0;
  return pos - out;
}


/**
  Duplicate a SQLCHAR as a SQLCHAR in the specified character set.

  @param[in]      from_charset  Character set to convert from
  @param[in]      to_charset    Character set to convert into
  @param[in]      str           String to convert
  @param[in,out]  len           Pointer to length of source (in chars) or
                                destination string (in bytes)
  @param[out]     errors        Pointer to count of errors in conversion

  @return  Pointer to a newly allocated SQLCHAR, or @c NULL
*/
SQLCHAR *sqlchar_as_sqlchar(CHARSET_INFO *from_charset,
                            CHARSET_INFO *to_charset,
                            SQLCHAR *str, SQLINTEGER *len, uint *errors)
{
  uint32 used_bytes, used_chars, bytes;
  SQLCHAR *conv;

  if (*len == SQL_NTS)
    *len= strlen((char *)str);

  bytes= (*len / from_charset->mbminlen * to_charset->mbmaxlen);
  conv= (SQLCHAR *)my_malloc(bytes + 1, MYF(0));
  if (!conv)
  {
    *len= -1;
    return NULL;
  }

  *len= copy_and_convert((char *)conv, bytes, to_charset,
                         (char *)str, *len,
                         from_charset, &used_bytes,
                         &used_chars, errors);

  conv[*len]= '\0';

  return conv;
}


/**
  Convert a SQLWCHAR to a SQLCHAR in the specified character set. This
  variation uses a pre-allocated buffer.

  @param[in]      charset_info  Character set to convert into
  @param[out]     out           Pointer to SQLWCHAR buffer
  @param[in]      out_bytes     Length of @c out buffer
  @param[in]      str           String to convert
  @param[in]      len           Length of @c in (in SQLWCHAR's)
  @param[out]     errors        Pointer to count of errors in conversion

  @return  Number of characters stored in the @c out buffer
*/
SQLINTEGER sqlwchar_as_sqlchar_buf(CHARSET_INFO *charset_info,
                                   SQLCHAR *out, SQLINTEGER out_bytes,
                                   SQLWCHAR *str, SQLINTEGER len, uint *errors)
{
  SQLWCHAR *str_end;
  SQLINTEGER i, u8_len;
  UTF8 u8[MAX_BYTES_PER_UTF8_CP + 1];
  uint32 used_bytes, used_chars;

  *errors= 0;

  if (len == SQL_NTS)
    len= sqlwcharlen(str);
  if (!str || len == 0)
    return 0;

  str_end= str + len;

  for (i= 0; str < str_end; )
  {
    if (sizeof(SQLWCHAR) == 4)
    {
      u8_len= utf32toutf8((UTF32)*str++, u8);
    }
    else
    {
      UTF32 u32;
      int consumed= utf16toutf32((UTF16 *)str, &u32);
      str+= consumed;
      if (!consumed)
      {
        *errors+= 1;
        break;
      }
      u8_len= utf32toutf8(u32, u8);
    }

    i+= copy_and_convert((char *)out + i, out_bytes - i, charset_info,
                         (char *)u8, u8_len, utf8_charset_info, &used_bytes,
                         &used_chars, errors);
  }

  out[i]= '\0';

  return i;
}


/**
  Copy a string from one character set to another. Taken from sql_string.cc
  in the MySQL Server source code, since we don't export this functionality
  in libmysql yet.

  @c to must be at least as big as @c from_length * @c to_cs->mbmaxlen

  @param[in,out] to           Store result here
  @param[in]     to_cs        Character set of result string
  @param[in]     from         Copy from here
  @param[in]     from_length  Length of string in @c from (in bytes)
  @param[in]     from_cs      Character set of string in @c from
  @param[out]    used_bytes   Buffer for returning number of bytes consumed
                              from @c from
  @param[out]    used_chars   Buffer for returning number of chars consumed
                              from @c from
  @param[in,out] errors       Pointer to value where number of errors
                              encountered is added.

  @retval Length of bytes copied to @c to
*/
uint32
copy_and_convert(char *to, uint32 to_length, CHARSET_INFO *to_cs,
                 const char *from, uint32 from_length, CHARSET_INFO *from_cs,
                 uint32 *used_bytes, uint32 *used_chars, uint *errors)
{
  int         from_cnvres, to_cnvres;
  my_wc_t     wc;
  const uchar *from_end= (const uchar*) from+from_length;
  char *to_start= to;
  uchar *to_end= (uchar*) to+to_length;
  int (*mb_wc)(struct charset_info_st *, my_wc_t *, const uchar *,
               const uchar *)= from_cs->cset->mb_wc;
  int (*wc_mb)(struct charset_info_st *, my_wc_t, uchar *s, uchar *e)=
    to_cs->cset->wc_mb;
  uint error_count= 0;

  *used_bytes= *used_chars= 0;

  while (1)
  {
    if ((from_cnvres= (*mb_wc)(from_cs, &wc, (uchar*) from, from_end)) > 0)
      from+= from_cnvres;
    else if (from_cnvres == MY_CS_ILSEQ)
    {
      ++error_count;
      ++from;
      wc= '?';
    }
    else if (from_cnvres > MY_CS_TOOSMALL)
    {
      /*
        A correct multibyte sequence detected
        But it doesn't have Unicode mapping.
      */
      ++error_count;
      from+= (-from_cnvres);
      wc= '?';
    }
    else
      break; /* Not enough characters */

outp:
    if ((to_cnvres= (*wc_mb)(to_cs, wc, (uchar*) to, to_end)) > 0)
      to+= to_cnvres;
    else if (to_cnvres == MY_CS_ILUNI && wc != '?')
    {
      ++error_count;
      wc= '?';
      goto outp;
    }
    else
      break;

    *used_bytes+= from_cnvres;
    *used_chars+= 1;
  }
  if (errors)
    *errors+= error_count;

  return (uint32) (to - to_start);
}




/*
 * Compare two SQLWCHAR strings ignoring case. This is only
 * case-insensitive over the ASCII range of characters.
 *
 * @return 0 if the strings are the same, 1 if they are not.
 */
int sqlwcharcasecmp(const SQLWCHAR *s1, const SQLWCHAR *s2)
{
  SQLWCHAR c1, c2;
  while (*s1 && *s2)
  {
    c1= *s1;
    c2= *s2;
    /* capitalize both strings */
    if (c1 >= 'a')
      c1 -= ('a' - 'A');
    if (c2 >= 'a')
      c2 -= ('a' - 'A');
    if (c1 != c2)
      return 1;
    ++s1;
    ++s2;
  }

  /* one will be null, so both must be */
  return *s1 != *s2;
}


/*
 * Locate a SQLWCHAR in a SQLWCHAR string.
 *
 * @return Position of char if found, otherwise NULL.
 */
const SQLWCHAR *sqlwcharchr(const SQLWCHAR *wstr, SQLWCHAR wchr)
{
  while (*wstr)
  {
    if (*wstr == wchr)
    {
      return wstr;
    }

    ++wstr;
  }

  return NULL;
}


/*
 * Calculate the length of a SQLWCHAR string.
 *
 * @return The number of SQLWCHAR units in the string.
 */
size_t sqlwcharlen(const SQLWCHAR *wstr)
{
  size_t len= 0;
  while (wstr && *wstr++)
    ++len;
  return len;
}


/*
 * Duplicate a SQLWCHAR string. Memory is allocated with my_malloc()
 * and should be freed with my_free() or the x_free() macro.
 *
 * @return A pointer to a new string.
 */
SQLWCHAR *sqlwchardup(const SQLWCHAR *wstr, size_t charlen)
{
  size_t chars= charlen == SQL_NTS ? sqlwcharlen(wstr) : charlen;
  SQLWCHAR *res= (SQLWCHAR *)my_malloc((chars + 1) * sizeof(SQLWCHAR), MYF(0));
  if (!res)
    return NULL;
  memcpy(res, wstr, chars * sizeof(SQLWCHAR));
  res[chars]= 0;
  return res;
}


/*
 * Convert a SQLWCHAR string to a long integer.
 *
 * @return The integer result of the conversion or 0 if the
 *         string could not be parsed.
 */
unsigned long sqlwchartoul(const SQLWCHAR *wstr, const SQLWCHAR **endptr){
  unsigned long res= 0;
  SQLWCHAR c;

  if (!wstr)
    return 0;

  while (c= *wstr)
  {
    if (c < '0' || c > '9')
      break;
    res*= 10;
    res+= c - '0';
    ++wstr;
  }

  if (endptr)
    *endptr= wstr;

  return res;
}


/*
 * Convert a long integer to a SQLWCHAR string.
 */
void sqlwcharfromul(SQLWCHAR *wstr, unsigned long v)
{
  int chars;
  unsigned long v1;
  for(chars= 0, v1= v; v1 > 0; ++chars, v1 /= 10);
  wstr[chars]= (SQLWCHAR)0;
  for(v1= v; v1 > 0; v1 /= 10)
    wstr[--chars]= (SQLWCHAR)('0' + (v1 % 10));
}


/*
 * Concatenate two strings. This differs from the convential
 * strncat() in that the parameter n is reduced by the number
 * of characters used.
 *
 * Returns the number of characters copied.
 */
size_t sqlwcharncat2(SQLWCHAR *dest, const SQLWCHAR *src, size_t *n)
{
  SQLWCHAR *orig_dest;
  if (!n || !*n)
    return 0;
  orig_dest= (dest += sqlwcharlen(dest));
  while (*src && *n && (*n)--)
    *dest++= *src++;
  if (*n)
    *dest= 0;
  else
    *(dest - 1)= 0;
  return dest - orig_dest;
}


/*
 * Copy up to 'n' characters (including NULL) from src to dest.
 */
SQLWCHAR *sqlwcharncpy(SQLWCHAR *dest, const SQLWCHAR *src, size_t n)
{
  if (!dest || !src)
    return NULL;
  while (*src && n--)
    *dest++= *src++;
  if (n)
    *dest= 0;
  else
    *(dest - 1)= 0;
  return dest;
}


/* Converts all characters in string to lowercase */
char * myodbc_strlwr(char *target, size_t len)
{
  unsigned char *c= (unsigned char *)target;
  if (len <= 0)
    len= strlen(target);

  while (len-- > 0)
    *c++= tolower(*c);

  return target;
}


/********************************
This charset mapping function is cut out from server repo: sql_common/client.c
*********************************/

typedef enum my_cs_match_type_enum
{
  /* MySQL and OS charsets are fully compatible */
  my_cs_exact,
  /* MySQL charset is very close to OS charset  */
  my_cs_approx,
  /*
    MySQL knows this charset, but it is not supported as client character set.
  */
  my_cs_unsupp
} my_cs_match_type;


typedef struct str2str_st
{
  const char        *os_name;
  const char        *my_name;
  my_cs_match_type  param;
} MY_CSET_OS_NAME;

static const MY_CSET_OS_NAME charsets[]=
{
#ifdef __WIN__
  {"cp437",          "cp850",    my_cs_approx},
  {"cp850",          "cp850",    my_cs_exact},
  {"cp852",          "cp852",    my_cs_exact},
  {"cp858",          "cp850",    my_cs_approx},
  {"cp866",          "cp866",    my_cs_exact},
  {"cp874",          "tis620",   my_cs_approx},
  {"cp932",          "cp932",    my_cs_exact},
  {"cp936",          "gbk",      my_cs_approx},
  {"cp949",          "euckr",    my_cs_approx},
  {"cp950",          "big5",     my_cs_exact},
  {"cp1200",         "utf16le",  my_cs_unsupp},
  {"cp1201",         "utf16",    my_cs_unsupp},
  {"cp1250",         "cp1250",   my_cs_exact},
  {"cp1251",         "cp1251",   my_cs_exact},
  {"cp1252",         "latin1",   my_cs_exact},
  {"cp1253",         "greek",    my_cs_exact},
  {"cp1254",         "latin5",   my_cs_exact},
  {"cp1255",         "hebrew",   my_cs_approx},
  {"cp1256",         "cp1256",   my_cs_exact},
  {"cp1257",         "cp1257",   my_cs_exact},
  {"cp10000",        "macroman", my_cs_exact},
  {"cp10001",        "sjis",     my_cs_approx},
  {"cp10002",        "big5",     my_cs_approx},
  {"cp10008",        "gb2312",   my_cs_approx},
  {"cp10021",        "tis620",   my_cs_approx},
  {"cp10029",        "macce",    my_cs_exact},
  {"cp12001",        "utf32",    my_cs_unsupp},
  {"cp20107",        "swe7",     my_cs_exact},
  {"cp20127",        "latin1",   my_cs_approx},
  {"cp20866",        "koi8r",    my_cs_exact},
  {"cp20932",        "ujis",     my_cs_exact},
  {"cp20936",        "gb2312",   my_cs_approx},
  {"cp20949",        "euckr",    my_cs_approx},
  {"cp21866",        "koi8u",    my_cs_exact},
  {"cp28591",        "latin1",   my_cs_approx},
  {"cp28592",        "latin2",   my_cs_exact},
  {"cp28597",        "greek",    my_cs_exact},
  {"cp28598",        "hebrew",   my_cs_exact},
  {"cp28599",        "latin5",   my_cs_exact},
  {"cp28603",        "latin7",   my_cs_exact},
#ifdef UNCOMMENT_THIS_WHEN_WL_4579_IS_DONE
  {"cp28605",        "latin9",   my_cs_exact},
#endif
  {"cp38598",        "hebrew",   my_cs_exact},
  {"cp51932",        "ujis",     my_cs_exact},
  {"cp51936",        "gb2312",   my_cs_exact},
  {"cp51949",        "euckr",    my_cs_exact},
  {"cp51950",        "big5",     my_cs_exact},
#ifdef UNCOMMENT_THIS_WHEN_WL_WL_4024_IS_DONE
  {"cp54936",        "gb18030",  my_cs_exact},
#endif
  {"cp65001",        "utf8",     my_cs_exact},

#else /* not Windows */

  {"646",            "latin1",   my_cs_approx}, /* Default on Solaris */
  {"ANSI_X3.4-1968", "latin1",   my_cs_approx},
  {"ansi1251",       "cp1251",   my_cs_exact},
  {"armscii8",       "armscii8", my_cs_exact},
  {"armscii-8",      "armscii8", my_cs_exact},
  {"ASCII",          "latin1",   my_cs_approx},
  {"Big5",           "big5",     my_cs_exact},
  {"cp1251",         "cp1251",   my_cs_exact},
  {"cp1255",         "hebrew",   my_cs_approx},
  {"CP866",          "cp866",    my_cs_exact},
  {"eucCN",          "gb2312",   my_cs_exact},
  {"euc-CN",         "gb2312",   my_cs_exact},
  {"eucJP",          "ujis",     my_cs_exact},
  {"euc-JP",         "ujis",     my_cs_exact},
  {"eucKR",          "euckr",    my_cs_exact},
  {"euc-KR",         "euckr",    my_cs_exact},
#ifdef UNCOMMENT_THIS_WHEN_WL_WL_4024_IS_DONE
  {"gb18030",        "gb18030",  my_cs_exact},
#endif
  {"gb2312",         "gb2312",   my_cs_exact},
  {"gbk",            "gbk",      my_cs_exact},
  {"georgianps",     "geostd8",  my_cs_exact},
  {"georgian-ps",    "geostd8",  my_cs_exact},
  {"IBM-1252",       "cp1252",   my_cs_exact},

  {"iso88591",       "latin1",   my_cs_approx},
  {"ISO_8859-1",     "latin1",   my_cs_approx},
  {"ISO8859-1",      "latin1",   my_cs_approx},
  {"ISO-8859-1",     "latin1",   my_cs_approx},

  {"iso885913",      "latin7",   my_cs_exact},
  {"ISO_8859-13",    "latin7",   my_cs_exact},
  {"ISO8859-13",     "latin7",   my_cs_exact},
  {"ISO-8859-13",    "latin7",   my_cs_exact},

#ifdef UNCOMMENT_THIS_WHEN_WL_4579_IS_DONE
  {"iso885915",      "latin9",   my_cs_exact},
  {"ISO_8859-15",    "latin9",   my_cs_exact},
  {"ISO8859-15",     "latin9",   my_cs_exact},
  {"ISO-8859-15",    "latin9",   my_cs_exact},
#endif

  {"iso88592",       "latin2",   my_cs_exact},
  {"ISO_8859-2",     "latin2",   my_cs_exact},
  {"ISO8859-2",      "latin2",   my_cs_exact},
  {"ISO-8859-2",     "latin2",   my_cs_exact},

  {"iso88597",       "greek",    my_cs_exact},
  {"ISO_8859-7",     "greek",    my_cs_exact},
  {"ISO8859-7",      "greek",    my_cs_exact},
  {"ISO-8859-7",     "greek",    my_cs_exact},

  {"iso88598",       "hebrew",   my_cs_exact},
  {"ISO_8859-8",     "hebrew",   my_cs_exact},
  {"ISO8859-8",      "hebrew",   my_cs_exact},
  {"ISO-8859-8",     "hebrew",   my_cs_exact},

  {"iso88599",       "latin5",   my_cs_exact},
  {"ISO_8859-9",     "latin5",   my_cs_exact},
  {"ISO8859-9",      "latin5",   my_cs_exact},
  {"ISO-8859-9",     "latin5",   my_cs_exact},

  {"koi8r",          "koi8r",    my_cs_exact},
  {"KOI8-R",         "koi8r",    my_cs_exact},
  {"koi8u",          "koi8u",    my_cs_exact},
  {"KOI8-U",         "koi8u",    my_cs_exact},

  {"roman8",         "hp8",      my_cs_exact}, /* Default on HP UX */

  {"Shift_JIS",      "sjis",     my_cs_exact},
  {"SJIS",           "sjis",     my_cs_exact},
  {"shiftjisx0213",  "sjis",     my_cs_exact},
  
  {"tis620",         "tis620",   my_cs_exact},
  {"tis-620",        "tis620",   my_cs_exact},

  {"ujis",           "ujis",     my_cs_exact},

  {"US-ASCII",       "latin1",   my_cs_approx},

  {"utf8",           "utf8",     my_cs_exact},
  {"utf-8",          "utf8",     my_cs_exact},
#endif
  {NULL,             NULL,       0}
};


const char *
my_os_charset_to_mysql_charset(const char *csname)
{
  const MY_CSET_OS_NAME *csp;
  for (csp= charsets; csp->os_name; ++csp)
  {
    if (!my_strcasecmp(&my_charset_latin1, csp->os_name, csname))
    {
      switch (csp->param)
      {
      case my_cs_exact:
        return csp->my_name;

      case my_cs_approx:
        /*
          Maybe we should print a warning eventually:
          character set correspondence is not exact.
        */
        return csp->my_name;

      default:
        goto def;
      }
    }
  }

def:
  csname= MYSQL_DEFAULT_CHARSET_NAME;

  return csname;
}
