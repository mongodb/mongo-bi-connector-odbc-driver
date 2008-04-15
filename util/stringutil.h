/*
  Copyright (C) 2007 MySQL AB

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
  @file  stringutil.c
  @brief Function prototypes from stringutil.c.
*/

#ifndef _STRINGUTIL_H
#define _STRINGUTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../MYODBC_MYSQL.h"

#include <string.h>
#include <sql.h>
#include <sqlext.h>

#ifndef x_free
#define x_free(A) { void *tmp= (A); if (tmp) my_free((char *) tmp,MYF(MY_WME+MY_FAE)); }
#endif

#define myodbc_min(a, b) ((a) < (b) ? (a) : (b))
#define myodbc_max(a, b) ((a) > (b) ? (a) : (b))

#define MAX_BYTES_PER_UTF8_CP 4 /* max 4 bytes per utf8 codepoint */

/* Unicode transcoding */
typedef unsigned int UTF32;
typedef unsigned short UTF16;
typedef unsigned char UTF8;

extern CHARSET_INFO *utf8_charset_info;


/**
 Determine whether a charset number represents a UTF-8 collation.
*/
#define is_utf8_charset(number) \
  (number == 33 || number == 83 || \
   (number >= 192 && number <= 211) || number == 253 || \
   number == 45 || number == 46 || \
   (number >= 224 && number <= 243))


int utf16toutf32(UTF16 *i, UTF32 *u);
int utf32toutf16(UTF32 i, UTF16 *u);
int utf8toutf32(UTF8 *i, UTF32 *u);
int utf32toutf8(UTF32 i, UTF8 *c);


/* Conversions */
SQLWCHAR *sqlchar_as_sqlwchar(CHARSET_INFO *charset_info, SQLCHAR *str,
                              SQLINTEGER *len, uint *errors);
SQLCHAR *sqlwchar_as_sqlchar(CHARSET_INFO *charset_info, SQLWCHAR *str,
                             SQLINTEGER *len, uint *errors);
SQLCHAR *sqlwchar_as_utf8(SQLWCHAR *str, SQLINTEGER *len);
SQLSMALLINT utf8_as_sqlwchar(SQLWCHAR *out, SQLINTEGER out_max, SQLCHAR *in,
                             SQLINTEGER in_len);

SQLCHAR *sqlchar_as_sqlchar(CHARSET_INFO *from_charset,
                            CHARSET_INFO *to_charset,
                            SQLCHAR *str, SQLINTEGER *len, uint *errors);
SQLINTEGER sqlwchar_as_sqlchar_buf(CHARSET_INFO *charset_info,
                                   SQLCHAR *out, SQLINTEGER out_bytes,
                                   SQLWCHAR *str, SQLINTEGER len, uint *errors);
uint32
copy_and_convert(char *to, uint32 to_length, CHARSET_INFO *to_cs,
                 const char *from, uint32 from_length, CHARSET_INFO *from_cs,
                 uint32 *used_bytes, uint32 *used_chars, uint *errors);


/* wcs* replacements */
int sqlwcharcasecmp(const SQLWCHAR *s1, const SQLWCHAR *s2);
const SQLWCHAR *sqlwcharchr(const SQLWCHAR *wstr, SQLWCHAR wchr);
size_t sqlwcharlen(const SQLWCHAR *wstr);
SQLWCHAR *sqlwchardup(const SQLWCHAR *wstr, size_t charlen);
unsigned long sqlwchartoul(const SQLWCHAR *wstr, const SQLWCHAR **endptr);
void sqlwcharfromul(SQLWCHAR *wstr, unsigned long v);
size_t sqlwcharncat2(SQLWCHAR *dest, const SQLWCHAR *src, size_t *n);
SQLWCHAR *sqlwcharncpy(SQLWCHAR *dest, const SQLWCHAR *src, size_t n);


#ifdef __cplusplus
}
#endif

#endif /* _STRINGUTIL_H */

