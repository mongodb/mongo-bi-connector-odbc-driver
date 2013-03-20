/*
  Copyright (c) 2007, 2013, Oracle and/or its affiliates. All rights reserved.

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

/*
 * Installer wrapper implementations.
 *
 * How to add a data-source parameter:
 *    Search for DS_PARAM, and follow the code around it
 *    Add an extra field to the DataSource struct (installer.h)
 *    Add a default value in ds_new()
 *    Initialize/destroy them in ds_new/ds_delete
 *    Print the field in myodbc3i.c
 *    Add to the configuration GUIs
 *
 */
#include "stringutil.h"
#include "installer.h"


/*
   SQLGetPrivateProfileStringW is buggy in all releases of unixODBC
   as of 2007-12-03, so always use our replacement.
*/
#if USE_UNIXODBC
# define SQLGetPrivateProfileStringW MySQLGetPrivateProfileStringW
#endif


/*
   Most of the installer API functions in iODBC incorrectly reset the
   config mode, so we need to save and restore it whenever we call those
   functions. These macros reduce the clutter a little bit.
*/
#if USE_IODBC
# define SAVE_MODE() UWORD config_mode= config_get()
# define RESTORE_MODE() config_set(config_mode)
#else
# define SAVE_MODE()
# define RESTORE_MODE()
#endif


/* ODBC Installer Config Wrapper */

/* a few constants */
static SQLWCHAR W_EMPTY[]= {0};
static SQLWCHAR W_ODBCINST_INI[]=
  {'O', 'D', 'B', 'C', 'I', 'N', 'S', 'T', '.', 'I', 'N', 'I', 0};
static SQLWCHAR W_ODBC_INI[]= {'O', 'D', 'B', 'C', '.', 'I', 'N', 'I', 0};
static SQLWCHAR W_CANNOT_FIND_DRIVER[]= {'C', 'a', 'n', 'n', 'o', 't', ' ',
                                         'f', 'i', 'n', 'd', ' ',
                                         'd', 'r', 'i', 'v', 'e', 'r', 0};

static SQLWCHAR W_DSN[]= {'D', 'S', 'N', 0};
static SQLWCHAR W_DRIVER[]= {'D', 'r', 'i', 'v', 'e', 'r', 0};
static SQLWCHAR W_DESCRIPTION[]=
  {'D', 'E', 'S', 'C', 'R', 'I', 'P', 'T', 'I', 'O', 'N', 0};
static SQLWCHAR W_SERVER[]= {'S', 'E', 'R', 'V', 'E', 'R', 0};
static SQLWCHAR W_UID[]= {'U', 'I', 'D', 0};
static SQLWCHAR W_USER[]= {'U', 'S', 'E', 'R', 0};
static SQLWCHAR W_PWD[]= {'P', 'W', 'D', 0};
static SQLWCHAR W_PASSWORD[]= {'P', 'A', 'S', 'S', 'W', 'O', 'R', 'D', 0};
static SQLWCHAR W_DB[]= {'D', 'B', 0};
static SQLWCHAR W_DATABASE[]= {'D', 'A', 'T', 'A', 'B', 'A', 'S', 'E', 0};
static SQLWCHAR W_SOCKET[]= {'S', 'O', 'C', 'K', 'E', 'T', 0};
static SQLWCHAR W_INITSTMT[]= {'I', 'N', 'I', 'T', 'S', 'T', 'M', 'T', 0};
static SQLWCHAR W_OPTION[]= {'O', 'P', 'T', 'I', 'O', 'N', 0};
static SQLWCHAR W_CHARSET[]= {'C', 'H', 'A', 'R', 'S', 'E', 'T', 0};
static SQLWCHAR W_SSLKEY[]= {'S', 'S', 'L', 'K', 'E', 'Y', 0};
static SQLWCHAR W_SSLCERT[]= {'S', 'S', 'L', 'C', 'E', 'R', 'T', 0};
static SQLWCHAR W_SSLCA[]= {'S', 'S', 'L', 'C', 'A', 0};
static SQLWCHAR W_SSLCAPATH[]=
  {'S', 'S', 'L', 'C', 'A', 'P', 'A', 'T', 'H', 0};
static SQLWCHAR W_SSLCIPHER[]=
  {'S', 'S', 'L', 'C', 'I', 'P', 'H', 'E', 'R', 0};
static SQLWCHAR W_SSLVERIFY[]=
  {'S', 'S', 'L', 'V', 'E', 'R', 'I', 'F', 'Y', 0};
static SQLWCHAR W_PORT[]= {'P', 'O', 'R', 'T', 0};
static SQLWCHAR W_SETUP[]= {'S', 'E', 'T', 'U', 'P', 0};
static SQLWCHAR W_READTIMEOUT[]=
  {'R','E','A','D','T','I','M','E','O','U','T',0};
static SQLWCHAR W_WRITETIMEOUT[]=
  {'W','R','I','T','E','T','I','M','E','O','U','T',0};
static SQLWCHAR W_FOUND_ROWS[]=
  {'F','O','U','N','D','_','R','O','W','S',0};
static SQLWCHAR W_BIG_PACKETS[]=
  {'B','I','G','_','P','A','C','K','E','T','S',0};
static SQLWCHAR W_NO_PROMPT[]=
  {'N','O','_','P','R','O','M','P','T',0};
static SQLWCHAR W_DYNAMIC_CURSOR[]=
  {'D','Y','N','A','M','I','C','_','C','U','R','S','O','R',0};
static SQLWCHAR W_NO_SCHEMA[]=
  {'N','O','_','S','C','H','E','M','A',0};
static SQLWCHAR W_NO_DEFAULT_CURSOR[]=
  {'N','O','_','D','E','F','A','U','L','T','_','C','U','R','S','O','R',0};
static SQLWCHAR W_NO_LOCALE[]=
  {'N','O','_','L','O','C','A','L','E',0};
static SQLWCHAR W_PAD_SPACE[]=
  {'P','A','D','_','S','P','A','C','E',0};
static SQLWCHAR W_FULL_COLUMN_NAMES[]=
  {'F','U','L','L','_','C','O','L','U','M','N','_','N','A','M','E','S',0};
static SQLWCHAR W_COMPRESSED_PROTO[]=
  {'C','O','M','P','R','E','S','S','E','D','_','P','R','O','T','O',0};
static SQLWCHAR W_IGNORE_SPACE[]=
  {'I','G','N','O','R','E','_','S','P','A','C','E',0};
static SQLWCHAR W_NAMED_PIPE[]=
  {'N','A','M','E','D','_','P','I','P','E',0};
static SQLWCHAR W_NO_BIGINT[]=
  {'N','O','_','B','I','G','I','N','T',0};
static SQLWCHAR W_NO_CATALOG[]=
  {'N','O','_','C','A','T','A','L','O','G',0};
static SQLWCHAR W_USE_MYCNF[]=
  {'U','S','E','_','M','Y','C','N','F',0};
static SQLWCHAR W_SAFE[]=
  {'S','A','F','E',0};
static SQLWCHAR W_NO_TRANSACTIONS[]=
  {'N','O','_','T','R','A','N','S','A','C','T','I','O','N','S',0};
static SQLWCHAR W_LOG_QUERY[]=
  {'L','O','G','_','Q','U','E','R','Y',0};
static SQLWCHAR W_NO_CACHE[]=
  {'N','O','_','C','A','C','H','E',0};
static SQLWCHAR W_FORWARD_CURSOR[]=
  {'F','O','R','W','A','R','D','_','C','U','R','S','O','R',0};
static SQLWCHAR W_AUTO_RECONNECT[]=
  {'A','U','T','O','_','R','E','C','O','N','N','E','C','T',0};
static SQLWCHAR W_AUTO_IS_NULL[]=
  {'A','U','T','O','_','I','S','_','N','U','L','L',0};
static SQLWCHAR W_ZERO_DATE_TO_MIN[]=
  {'Z','E','R','O','_','D','A','T','E','_','T','O','_','M','I','N',0};
static SQLWCHAR W_MIN_DATE_TO_ZERO[]=
  {'M','I','N','_','D','A','T','E','_','T','O','_','Z','E','R','O',0};
static SQLWCHAR W_MULTI_STATEMENTS[]=
  {'M','U','L','T','I','_','S','T','A','T','E','M','E','N','T','S',0};
static SQLWCHAR W_COLUMN_SIZE_S32[]=
  {'C','O','L','U','M','N','_','S','I','Z','E','_','S','3','2',0};
static SQLWCHAR W_NO_BINARY_RESULT[]=
  {'N','O','_','B','I','N','A','R','Y','_','R','E','S','U','L','T',0};
static SQLWCHAR W_DFLT_BIGINT_BIND_STR[]=
  {'D','F','L','T','_','B','I','G','I','N','T','_','B','I','N','D','_','S','T','R',0};
static SQLWCHAR W_CLIENT_INTERACTIVE[]=
  {'I','N','T','E','R','A','C','T','I','V','E',0};
static SQLWCHAR W_NO_I_S[]= {'N','O','_','I','_','S',0};
static SQLWCHAR W_PREFETCH[]= {'P','R','E','F','E','T','C','H',0};
static SQLWCHAR W_NO_SSPS[]= {'N','O','_','S','S','P','S',0};
static SQLWCHAR W_CAN_HANDLE_EXP_PWD[]=
  {'C','A','N','_','H','A','N','D','L','E','_','E','X','P','_','P','W','D',0};
static SQLWCHAR W_ENABLE_CLEARTEXT_PLUGIN[]=
  {'E','N','A','B','L','E','_','C','L','E','A', 'R', 'T', 'E', 'X', 'T', '_','P','L','U','G','I','N',0};

/* DS_PARAM */
/* externally used strings */
const SQLWCHAR W_DRIVER_PARAM[]= {';', 'D', 'R', 'I', 'V', 'E', 'R', '=', 0};
const SQLWCHAR W_DRIVER_NAME[]= {'M', 'y', 'S', 'Q', 'L', ' ',
                                 'O', 'D', 'B', 'C', ' ', '5', '.', '2', ' ',
                                 'D', 'r', 'i', 'v', 'e', 'r', 0};
const SQLWCHAR W_INVALID_ATTR_STR[]= {'I', 'n', 'v', 'a', 'l', 'i', 'd', ' ',
                                      'a', 't', 't', 'r', 'i', 'b', 'u', 't', 'e', ' ',
                                      's', 't', 'r', 'i', 'n', 'g', 0};

/* List of all DSN params, used when serializing to string */
static const
SQLWCHAR *dsnparams[]= {W_DSN, W_DRIVER, W_DESCRIPTION, W_SERVER,
                        W_UID, W_PWD, W_DATABASE, W_SOCKET, W_INITSTMT,
                        W_PORT, W_OPTION, W_CHARSET, W_SSLKEY,
                        W_SSLCERT, W_SSLCA, W_SSLCAPATH, W_SSLCIPHER,
                        W_SSLVERIFY, W_READTIMEOUT, W_WRITETIMEOUT,
                        W_FOUND_ROWS, W_BIG_PACKETS, W_NO_PROMPT,
                        W_DYNAMIC_CURSOR, W_NO_SCHEMA, W_NO_DEFAULT_CURSOR,
                        W_NO_LOCALE, W_PAD_SPACE, W_FULL_COLUMN_NAMES,
                        W_COMPRESSED_PROTO, W_IGNORE_SPACE, W_NAMED_PIPE,
                        W_NO_BIGINT, W_NO_CATALOG, W_USE_MYCNF, W_SAFE,
                        W_NO_TRANSACTIONS, W_LOG_QUERY, W_NO_CACHE,
                        W_FORWARD_CURSOR, W_AUTO_RECONNECT, W_AUTO_IS_NULL,
                        W_ZERO_DATE_TO_MIN, W_MIN_DATE_TO_ZERO,
                        W_MULTI_STATEMENTS, W_COLUMN_SIZE_S32,
                        W_NO_BINARY_RESULT, W_DFLT_BIGINT_BIND_STR,
                        W_CLIENT_INTERACTIVE, W_NO_I_S, W_PREFETCH, W_NO_SSPS,
                        W_CAN_HANDLE_EXP_PWD, W_ENABLE_CLEARTEXT_PLUGIN};
static const
int dsnparamcnt= sizeof(dsnparams) / sizeof(SQLWCHAR *);
/* DS_PARAM */

const unsigned int default_cursor_prefetch= 100;

/* convenience macro to append a single character */
#define APPEND_SQLWCHAR(buf, ctr, c) {\
  if (ctr) { \
    *((buf)++)= (c); \
    if (--(ctr)) \
        *(buf)= 0; \
  } \
}


/*
 * Check whether a parameter value needs escaping.
 */
static int value_needs_escaped(SQLWCHAR *str)
{
  SQLWCHAR c;
  while (str && (c= *str++))
  {
    if (c >= '0' && c <= '9')
      continue;
    else if (c >= 'a' && c <= 'z')
      continue;
    else if (c >= 'A' && c <= 'Z')
      continue;
    /* other non-alphanumeric characters that don't need escaping */
    switch (c)
    {
    case '_':
    case ' ':
    case '.':
      continue;
    }
    return 1;
  }
  return 0;
}


/*
 * Convenience function to get the current config mode.
 */
UWORD config_get()
{
  UWORD mode;
  SQLGetConfigMode(&mode);
  return mode;
}


/*
 * Convenience function to set the current config mode. Returns the
 * mode in use when the function was called.
 */
UWORD config_set(UWORD mode)
{
  UWORD current= config_get();
  SQLSetConfigMode(mode);
  return current;
}


/* ODBC Installer Driver Wrapper */


/*
 * Create a new driver object. All string data is pre-allocated.
 */
Driver *driver_new()
{
  Driver *driver= (Driver *)my_malloc(sizeof(Driver), MYF(0));
  if (!driver)
    return NULL;
  driver->name= (SQLWCHAR *)my_malloc(ODBCDRIVER_STRLEN * sizeof(SQLWCHAR),
                                      MYF(0));
  if (!driver->name)
  {
    x_free(driver);
    return NULL;
  }
  driver->lib= (SQLWCHAR *)my_malloc(ODBCDRIVER_STRLEN * sizeof(SQLWCHAR),
                                     MYF(0));
  if (!driver->lib)
  {
    x_free(driver);
    x_free(driver->name);
    return NULL;
  }
  driver->setup_lib= (SQLWCHAR *)my_malloc(ODBCDRIVER_STRLEN *
                                           sizeof(SQLWCHAR), MYF(0));
  if (!driver->setup_lib)
  {
    x_free(driver);
    x_free(driver->name);
    x_free(driver->lib);
    return NULL;
  }
  /* init to empty strings */
  driver->name[0]= 0;
  driver->lib[0]= 0;
  driver->setup_lib[0]= 0;
  driver->name8= NULL;
  driver->lib8= NULL;
  driver->setup_lib8= NULL;
  return driver;
}


/*
 * Delete an existing driver object.
 */
void driver_delete(Driver *driver)
{
  x_free(driver->name);
  x_free(driver->lib);
  x_free(driver->setup_lib);
  x_free(driver->name8);
  x_free(driver->lib8);
  x_free(driver->setup_lib8);
  x_free(driver);
}


#ifdef _WIN32
/*
 * Utility function to duplicate path and remove "(x86)" chars.
 */
SQLWCHAR *remove_x86(SQLWCHAR *path, SQLWCHAR *loc)
{
  /* Program Files (x86)
   *           13^   19^
   */
  size_t chars = sqlwcharlen(loc) - 18 /* need +1 for sqlwcharncat2() */;
  SQLWCHAR *news= sqlwchardup(path, SQL_NTS);
  news[(loc-path)+13] = 0;
  sqlwcharncat2(news, loc + 19, &chars);
  return news;
}


/*
 * Compare two library paths taking into account different
 * locations of "Program Files" for 32 and 64 bit applications
 * on Windows. This is done by removing the "(x86)" from "Program
 * Files" if present in either path.
 *
 * Note: wcs* functions are used as this only needs to support
 *       Windows where SQLWCHAR is wchar_t.
 */
int Win64CompareLibs(SQLWCHAR *lib1, SQLWCHAR *lib2)
{
  int free1= 0, free2= 0;
  int rc;
  SQLWCHAR *llib1, *llib2;

  /* perform necessary transformations */
  if (llib1= wcsstr(lib1, L"Program Files (x86)"))
  {
    llib1= remove_x86(lib1, llib1);
    free1= 1;
  }
  else
    llib1= lib1;
  if (llib2= wcsstr(lib2, L"Program Files (x86)"))
  {
    llib2= remove_x86(lib2, llib2);
    free2= 1;
  }
  else
    llib2= lib2;

  /* perform the comparison */
  rc= sqlwcharcasecmp(llib1, llib2);

  if (free1)
    free(llib1);
  if (free2)
    free(llib2);
  return rc;
}
#endif /* _WIN32 */


/*
 * Lookup a driver given only the filename of the driver. This is used:
 *
 * 1. When prompting for additional DSN info upon connect when the
 *    driver uses an external setup library.
 *
 * 2. When testing a connection when adding/editing a DSN.
 */
int driver_lookup_name(Driver *driver)
{
  SQLWCHAR drivers[16384];
  SQLWCHAR *pdrv= drivers;
  SQLWCHAR driverinfo[1024];
  int len;
  short slen; /* WORD needed for windows */
  SAVE_MODE();

  /* get list of drivers */
#ifdef _WIN32
  if (!SQLGetInstalledDriversW(pdrv, 16383, &slen) || !(len= slen))
#else
  if (!(len = SQLGetPrivateProfileStringW(NULL, NULL, W_EMPTY, pdrv, 16383,
                                          W_ODBCINST_INI)))
#endif
    return -1;

  RESTORE_MODE();

  /* check the lib of each driver for one that matches the given lib name */
  while (len > 0)
  {
    if (SQLGetPrivateProfileStringW(pdrv, W_DRIVER, W_EMPTY, driverinfo,
                                    1023, W_ODBCINST_INI))
    {
      RESTORE_MODE();

#ifdef _WIN32
      if (!Win64CompareLibs(driverinfo, driver->lib))
#else
      if (!sqlwcharcasecmp(driverinfo, driver->lib))
#endif
      {
        sqlwcharncpy(driver->name, pdrv, ODBCDRIVER_STRLEN);
        return 0;
      }
    }

    RESTORE_MODE();

    len -= sqlwcharlen(pdrv) + 1;
    pdrv += sqlwcharlen(pdrv) + 1;
  }

  return -1;
}


/*
 * Lookup a driver in the system. The driver name is read from the given
 * object. If greater-than zero is returned, additional information
 * can be obtained from SQLInstallerError(). A less-than zero return code
 * indicates that the driver could not be found.
 */
int driver_lookup(Driver *driver)
{
  SQLWCHAR buf[4096];
  SQLWCHAR *entries= buf;
  SQLWCHAR *dest;
  SAVE_MODE();

  /* if only the filename is given, we must get the driver's name */
  if (!*driver->name && *driver->lib)
  {
    if (driver_lookup_name(driver))
      return -1;
  }

  /* get entries and make sure the driver exists */
  if (SQLGetPrivateProfileStringW(driver->name, NULL, W_EMPTY, buf, 4096,
                                  W_ODBCINST_INI) < 1)
  {
    SQLPostInstallerErrorW(ODBC_ERROR_INVALID_NAME, W_CANNOT_FIND_DRIVER);
    return -1;
  }

  RESTORE_MODE();

  /* read the needed driver attributes */
  while (*entries)
  {
    dest= NULL;
    if (!sqlwcharcasecmp(W_DRIVER, entries))
      dest= driver->lib;
    else if (!sqlwcharcasecmp(W_SETUP, entries))
      dest= driver->setup_lib;
    else
      { /* unknown/unused entry */ }

    /* get the value if it's one we're looking for */
    if (dest && SQLGetPrivateProfileStringW(driver->name, entries, W_EMPTY,
                                            dest, ODBCDRIVER_STRLEN,
                                            W_ODBCINST_INI) < 1)
    {
      RESTORE_MODE();
      return 1;
    }

    RESTORE_MODE();

    entries += sqlwcharlen(entries) + 1;
  }

  return 0;
}


/*
 * Read the semi-colon delimited key-value pairs the attributes
 * necessary to popular the driver object.
 */
int driver_from_kvpair_semicolon(Driver *driver, const SQLWCHAR *attrs)
{
  const SQLWCHAR *split;
  const SQLWCHAR *end;
  SQLWCHAR attribute[100];
  SQLWCHAR *dest;

  while (*attrs)
  {
    dest= NULL;

    /* invalid key-value pair if no equals */
    if ((split= sqlwcharchr(attrs, '=')) == NULL)
      return 1;

    /* get end of key-value pair */
    if ((end= sqlwcharchr(attrs, ';')) == NULL)
      end= attrs + sqlwcharlen(attrs);

    /* pull out the attribute name */
    memcpy(attribute, attrs, (split - attrs) * sizeof(SQLWCHAR));
    attribute[split - attrs]= 0; /* add null term */
    ++split;

    /* if its one we want, copy it over */
    if (!sqlwcharcasecmp(W_DRIVER, attribute))
      dest= driver->lib;
    else if (!sqlwcharcasecmp(W_SETUP, attribute))
      dest= driver->setup_lib;
    else
      { /* unknown/unused attribute */ }

    if (dest)
    {
      memcpy(dest, split, (end - split) * sizeof(SQLWCHAR));
      dest[end - split]= 0; /* add null term */
    }

    /* advanced to next attribute */
    attrs= end;
    if (*end)
      ++attrs;
  }

  return 0;
}


/*
 * Write the attributes of the driver object into key-value pairs
 * separated by single NULL chars.
 */
int driver_to_kvpair_null(Driver *driver, SQLWCHAR *attrs, size_t attrslen)
{
  *attrs= 0;
  attrs+= sqlwcharncat2(attrs, driver->name, &attrslen);

  /* append NULL-separator */
  APPEND_SQLWCHAR(attrs, attrslen, 0);

  attrs+= sqlwcharncat2(attrs, W_DRIVER, &attrslen);
  APPEND_SQLWCHAR(attrs, attrslen, '=');
  attrs+= sqlwcharncat2(attrs, driver->lib, &attrslen);

  /* append NULL-separator */
  APPEND_SQLWCHAR(attrs, attrslen, 0);

  if (*driver->setup_lib)
  {
    attrs+= sqlwcharncat2(attrs, W_SETUP, &attrslen);
    APPEND_SQLWCHAR(attrs, attrslen, '=');
    attrs+= sqlwcharncat2(attrs, driver->setup_lib, &attrslen);

    /* append NULL-separator */
    APPEND_SQLWCHAR(attrs, attrslen, 0);
  }
  if (attrslen--)
    *attrs= 0;
  return !(attrslen > 0);
}


/* ODBC Installer Data Source Wrapper */


/*
 * Create a new data source object.
 */
DataSource *ds_new()
{
  DataSource *ds= (DataSource *)my_malloc(sizeof(DataSource), MYF(0));
  if (!ds)
    return NULL;
  memset(ds, 0, sizeof(DataSource));

  /* non-zero DataSource defaults here */
  ds->port=                   3306;
  /* DS_PARAM */

  return ds;
}


/*
 * Delete an existing data source object.
 */
void ds_delete(DataSource *ds)
{
  x_free(ds->name);
  x_free(ds->driver);
  x_free(ds->description);
  x_free(ds->server);
  x_free(ds->uid);
  x_free(ds->pwd);
  x_free(ds->database);
  x_free(ds->socket);
  x_free(ds->initstmt);
  x_free(ds->charset);
  x_free(ds->sslkey);
  x_free(ds->sslcert);
  x_free(ds->sslca);
  x_free(ds->sslcapath);
  x_free(ds->sslcipher);
  
  x_free(ds->name8);
  x_free(ds->driver8);
  x_free(ds->description8);
  x_free(ds->server8);
  x_free(ds->uid8);
  x_free(ds->pwd8);
  x_free(ds->database8);
  x_free(ds->socket8);
  x_free(ds->initstmt8);
  x_free(ds->charset8);
  x_free(ds->sslkey8);
  x_free(ds->sslcert8);
  x_free(ds->sslca8);
  x_free(ds->sslcapath8);
  x_free(ds->sslcipher8);

  x_free(ds);
}


/*
 * Set a string attribute of a given data source object. The string
 * will be copied into the object.
 */
int ds_set_strattr(SQLWCHAR **attr, const SQLWCHAR *val)
{
  x_free(*attr);
  if (val && *val)
    *attr= sqlwchardup(val, SQL_NTS);
  else
    *attr= NULL;
  return *attr || 0;
}


/*
 * Same as ds_set_strattr, but allows truncating the given string. If
 * charcount is 0 or SQL_NTS, it will act the same as ds_set_strattr.
 */
int ds_set_strnattr(SQLWCHAR **attr, const SQLWCHAR *val, size_t charcount)
{
  x_free(*attr);

  if (charcount == SQL_NTS)
    charcount= sqlwcharlen(val);

  if (!charcount)
  {
    *attr= NULL;
    return 1;
  }

  if (val && *val)
    *attr= sqlwchardup(val, charcount);
  else
    *attr= NULL;
  return *attr || 0;
}


/*
 * Internal function to map a parameter name of the data source object
 * to the pointer needed to set the parameter. Only one of strdest or
 * intdest will be set. strdest and intdest will be set for populating
 * string (SQLWCHAR *) or int parameters.
 */
void ds_map_param(DataSource *ds, const SQLWCHAR *param,
                  SQLWCHAR ***strdest, unsigned int **intdest,
                  BOOL **booldest)
{
  *strdest= NULL;
  *intdest= NULL;
  *booldest= NULL;
  /* parameter aliases can be used here, see W_UID, W_USER */
  if (!sqlwcharcasecmp(W_DSN, param))
    *strdest= &ds->name;
  else if (!sqlwcharcasecmp(W_DRIVER, param))
    *strdest= &ds->driver;
  else if (!sqlwcharcasecmp(W_DESCRIPTION, param))
    *strdest= &ds->description;
  else if (!sqlwcharcasecmp(W_SERVER, param))
    *strdest= &ds->server;
  else if (!sqlwcharcasecmp(W_UID, param))
    *strdest= &ds->uid;
  else if (!sqlwcharcasecmp(W_USER, param))
    *strdest= &ds->uid;
  else if (!sqlwcharcasecmp(W_PWD, param))
    *strdest= &ds->pwd;
  else if (!sqlwcharcasecmp(W_PASSWORD, param))
    *strdest= &ds->pwd;
  else if (!sqlwcharcasecmp(W_DB, param))
    *strdest= &ds->database;
  else if (!sqlwcharcasecmp(W_DATABASE, param))
    *strdest= &ds->database;
  else if (!sqlwcharcasecmp(W_SOCKET, param))
    *strdest= &ds->socket;
  else if (!sqlwcharcasecmp(W_INITSTMT, param))
    *strdest= &ds->initstmt;
  else if (!sqlwcharcasecmp(W_CHARSET, param))
    *strdest= &ds->charset;
  else if (!sqlwcharcasecmp(W_SSLKEY, param))
    *strdest= &ds->sslkey;
  else if (!sqlwcharcasecmp(W_SSLCERT, param))
    *strdest= &ds->sslcert;
  else if (!sqlwcharcasecmp(W_SSLCA, param))
    *strdest= &ds->sslca;
  else if (!sqlwcharcasecmp(W_SSLCAPATH, param))
    *strdest= &ds->sslcapath;
  else if (!sqlwcharcasecmp(W_SSLCIPHER, param))
    *strdest= &ds->sslcipher;

  else if (!sqlwcharcasecmp(W_PORT, param))
    *intdest= &ds->port;
  else if (!sqlwcharcasecmp(W_SSLVERIFY, param))
    *intdest= &ds->sslverify;
  else if (!sqlwcharcasecmp(W_READTIMEOUT, param))
    *intdest= &ds->readtimeout;
  else if (!sqlwcharcasecmp(W_WRITETIMEOUT, param))
    *intdest= &ds->writetimeout;
  else if (!sqlwcharcasecmp(W_CLIENT_INTERACTIVE, param))
    *intdest= &ds->clientinteractive;
  else if (!sqlwcharcasecmp(W_PREFETCH, param))
    *intdest= &ds->cursor_prefetch_number;
  else if (!sqlwcharcasecmp(W_FOUND_ROWS, param))
    *booldest= &ds->return_matching_rows;
  else if (!sqlwcharcasecmp(W_BIG_PACKETS, param))
    *booldest= &ds->allow_big_results;
  else if (!sqlwcharcasecmp(W_NO_PROMPT, param))
    *booldest= &ds->dont_prompt_upon_connect;
  else if (!sqlwcharcasecmp(W_DYNAMIC_CURSOR, param))
    *booldest= &ds->dynamic_cursor;
  else if (!sqlwcharcasecmp(W_NO_SCHEMA, param))
    *booldest= &ds->ignore_N_in_name_table;
  else if (!sqlwcharcasecmp(W_NO_DEFAULT_CURSOR, param))
    *booldest= &ds->user_manager_cursor;
  else if (!sqlwcharcasecmp(W_NO_LOCALE, param))
    *booldest= &ds->dont_use_set_locale;
  else if (!sqlwcharcasecmp(W_PAD_SPACE, param))
    *booldest= &ds->pad_char_to_full_length;
  else if (!sqlwcharcasecmp(W_FULL_COLUMN_NAMES, param))
    *booldest= &ds->return_table_names_for_SqlDescribeCol;
  else if (!sqlwcharcasecmp(W_COMPRESSED_PROTO, param))
    *booldest= &ds->use_compressed_protocol;
  else if (!sqlwcharcasecmp(W_IGNORE_SPACE, param))
    *booldest= &ds->ignore_space_after_function_names;
  else if (!sqlwcharcasecmp(W_NAMED_PIPE, param))
    *booldest= &ds->force_use_of_named_pipes;
  else if (!sqlwcharcasecmp(W_NO_BIGINT, param))
    *booldest= &ds->change_bigint_columns_to_int;
  else if (!sqlwcharcasecmp(W_NO_CATALOG, param))
    *booldest= &ds->no_catalog;
  else if (!sqlwcharcasecmp(W_USE_MYCNF, param))
    *booldest= &ds->read_options_from_mycnf;
  else if (!sqlwcharcasecmp(W_SAFE, param))
    *booldest= &ds->safe;
  else if (!sqlwcharcasecmp(W_NO_TRANSACTIONS, param))
    *booldest= &ds->disable_transactions;
  else if (!sqlwcharcasecmp(W_LOG_QUERY, param))
    *booldest= &ds->save_queries;
  else if (!sqlwcharcasecmp(W_NO_CACHE, param))
    *booldest= &ds->dont_cache_result;
  else if (!sqlwcharcasecmp(W_FORWARD_CURSOR, param))
    *booldest= &ds->force_use_of_forward_only_cursors;
  else if (!sqlwcharcasecmp(W_AUTO_RECONNECT, param))
    *booldest= &ds->auto_reconnect;
  else if (!sqlwcharcasecmp(W_AUTO_IS_NULL, param))
    *booldest= &ds->auto_increment_null_search;
  else if (!sqlwcharcasecmp(W_ZERO_DATE_TO_MIN, param))
    *booldest= &ds->zero_date_to_min;
  else if (!sqlwcharcasecmp(W_MIN_DATE_TO_ZERO, param))
    *booldest= &ds->min_date_to_zero;
  else if (!sqlwcharcasecmp(W_MULTI_STATEMENTS, param))
    *booldest= &ds->allow_multiple_statements;
  else if (!sqlwcharcasecmp(W_COLUMN_SIZE_S32, param))
    *booldest= &ds->limit_column_size;
  else if (!sqlwcharcasecmp(W_NO_BINARY_RESULT, param))
    *booldest= &ds->handle_binary_as_char;
  else if (!sqlwcharcasecmp(W_DFLT_BIGINT_BIND_STR, param))
    *booldest= &ds->default_bigint_bind_str;
  else if (!sqlwcharcasecmp(W_NO_I_S, param))
    *booldest= &ds->no_information_schema;
  else if (!sqlwcharcasecmp(W_NO_SSPS, param))
    *booldest= &ds->no_ssps;
  else if (!sqlwcharcasecmp(W_CAN_HANDLE_EXP_PWD, param))
    *booldest= &ds->can_handle_exp_pwd;
  else if (!sqlwcharcasecmp(W_ENABLE_CLEARTEXT_PLUGIN, param))
    *booldest= &ds->enable_cleartext_plugin;

  /* DS_PARAM */
}


/*
 * Lookup a data source in the system. The name will be read from
 * the object and the rest of the details will be populated.
 *
 * If greater-than zero is returned, additional information
 * can be obtained from SQLInstallerError(). A less-than zero return code
 * indicates that the driver could not be found.
 */
int ds_lookup(DataSource *ds)
{
  SQLWCHAR buf[8192];
  SQLWCHAR *entries= buf;
  SQLWCHAR **dest;
  SQLWCHAR val[256];
  int size, used;
  int rc= 0;
  UWORD config_mode= config_get();
  unsigned int *intdest;
  BOOL *booldest;
  /* No need for SAVE_MODE() because we always call config_get() above. */

#ifdef _WIN32
  /* We must do this to detect the WinXP bug mentioned below */
  memset(buf, 0xff, sizeof(buf));
#endif

  /* get entries and check if data source exists */
  if ((size= SQLGetPrivateProfileStringW(ds->name, NULL, W_EMPTY,
                                         buf, 8192, W_ODBC_INI)) < 1)
  {
    rc= -1;
    goto end;
  }

  RESTORE_MODE();

  /* Debug code to print the entries returned, char by char */
#ifdef DEBUG_MYODBC_DS_LOOKUP
  {
  int i;
  char dbuf[100];
  OutputDebugString("Dumping SQLGetPrivateProfileStringW result");
  for (i= 0; i < size; ++i)
  {
    sprintf(dbuf, "[%d] = %wc - 0x%x\n", i,
            (entries[i] < 0x7f && isalpha(entries[i]) ? entries[i] : 'X'),
            entries[i]);
    OutputDebugString(dbuf);
  }
  }
#endif

#ifdef _WIN32
  /*
   * In Windows XP, there is a bug in SQLGetPrivateProfileString
   * when mode is ODBC_BOTH_DSN and we are looking for a system
   * DSN. In this case SQLGetPrivateProfileString will find the
   * system dsn but return a corrupt list of attributes. 
   *
   * See debug code above to print the exact data returned.
   * See also: http://support.microsoft.com/kb/909122/
   */
  if (config_mode == ODBC_BOTH_DSN &&
      /* two null chars or a null and some bogus character */
      *(entries + sqlwcharlen(entries)) == 0 &&
      (*(entries + sqlwcharlen(entries) + 1) > 0x7f ||
       *(entries + sqlwcharlen(entries) + 1) == 0))
  {
    /* revert to system mode and try again */
    config_set(ODBC_SYSTEM_DSN);
    if ((size= SQLGetPrivateProfileStringW(ds->name, NULL, W_EMPTY,
                                           buf, 8192, W_ODBC_INI)) < 1)
    {
      rc= -1;
      goto end;
    }
  }
#endif

  for (used= 0; used < size; used += sqlwcharlen(entries) + 1,
                             entries += sqlwcharlen(entries) + 1)
  {
    int valsize;
    ds_map_param(ds, entries, &dest, &intdest, &booldest);

    if ((valsize= SQLGetPrivateProfileStringW(ds->name, entries, W_EMPTY,
                                              val, ODBCDATASOURCE_STRLEN,
                                              W_ODBC_INI)) < 0)
    {
      rc= 1;
      goto end;
    }
    else if (!valsize)
      /* skip blanks */;
    else if (dest && !*dest)
      ds_set_strnattr(dest, val, valsize);
    else if (intdest)
      *intdest= sqlwchartoul(val, NULL);
    else if (booldest)
      *booldest= sqlwchartoul(val, NULL) > 0;
    else if (!sqlwcharcasecmp(W_OPTION, entries))
      ds_set_options(ds, ds_get_options(ds) | sqlwchartoul(val, NULL));

    RESTORE_MODE();
  }

end:
  config_set(config_mode);
  return rc;
}


/*
 * Read an attribute list (key/value pairs) into a data source
 * object. Delimiter should probably be 0 or ';'.
 */
int ds_from_kvpair(DataSource *ds, const SQLWCHAR *attrs, SQLWCHAR delim)
{
  const SQLWCHAR *split;
  const SQLWCHAR *end;
  SQLWCHAR **dest;
  SQLWCHAR attribute[100];
  int len;
  unsigned int *intdest;
  BOOL *booldest;

  while (*attrs)
  {
    if ((split= sqlwcharchr(attrs, (SQLWCHAR)'=')) == NULL)
      return 1;

    /* remove leading spaces on attribute */
    while (*attrs == ' ')
      ++attrs;
    len = split - attrs;
    memcpy(attribute, attrs, len * sizeof(SQLWCHAR));
    attribute[len]= 0;
    /* remove trailing spaces on attribute */
    --len;
    while (attribute[len] == ' ')
    {
      attribute[len] = 0;
      --len;
    }

    /* remove leading and trailing spaces on value */
    while (*(++split) == ' ');

    /* check for an "escaped" value */
    if ((*split == '{' && (end= sqlwcharchr(attrs, '}')) == NULL) ||
        /* or a delimited value */
        (*split != '{' && (end= sqlwcharchr(attrs, delim)) == NULL))
      /* otherwise, take the rest of the string */
      end= attrs + sqlwcharlen(attrs);

    /* remove trailing spaces on value (not escaped part) */
    len = end - split - 1;
    while (end > split && split[len] == ' ' && split[len+1] != '}')
    {
      --len;
      --end;
    }

    /* handle deprecated options as an exception */
    if (!sqlwcharcasecmp(W_OPTION, attribute))
    {
      ds_set_options(ds, sqlwchartoul(split, NULL));
    }
    else
    {
      ds_map_param(ds, attribute, &dest, &intdest, &booldest);

      if (dest)
      {
        if (*split == '{' && *end == '}')
        {
          ds_set_strnattr(dest, split + 1, end - split - 1);
          ++end;
        }
        else
          ds_set_strnattr(dest, split, end - split);
      }
      else if (intdest)
      {
        /* we know we have a ; or NULL at the end so we just let it go */
        *intdest= sqlwchartoul(split, NULL);
      }
      else if (booldest)
      {
        *booldest= sqlwchartoul(split, NULL) > 0;
      }
    }

    attrs= end;
    /* If delim is NULL then double-NULL is the end of key-value pairs list */
    while ((delim && *attrs == delim) || *attrs == ' ')
      ++attrs;
  }

  return 0;
}


/*
 * Copy data source details into an attribute string. Use attrslen
 * to limit the number of characters placed into the string.
 *
 * Return -1 for an error or truncation, otherwise the number of
 * characters written.
 */
int ds_to_kvpair(DataSource *ds, SQLWCHAR *attrs, size_t attrslen,
                 SQLWCHAR delim)
{
  int i;
  SQLWCHAR **strval;
  unsigned int *intval;
  BOOL *boolval;
  int origchars= attrslen;
  SQLWCHAR numbuf[21];

  if (!attrslen)
    return -1;

  *attrs= 0;

  for (i= 0; i < dsnparamcnt; ++i)
  {
    ds_map_param(ds, dsnparams[i], &strval, &intval, &boolval);

    /* We skip the driver if dsn name is given */
    if (!sqlwcharcasecmp(W_DRIVER, dsnparams[i]) && ds->name && *ds->name)
      continue;

    if (strval && *strval && **strval)
    {
      attrs+= sqlwcharncat2(attrs, dsnparams[i], &attrslen);
      APPEND_SQLWCHAR(attrs, attrslen, '=');
      if (value_needs_escaped(*strval))
      {
        APPEND_SQLWCHAR(attrs, attrslen, '{');
        attrs+= sqlwcharncat2(attrs, *strval, &attrslen);
        APPEND_SQLWCHAR(attrs, attrslen, '}');
      }
      else
        attrs+= sqlwcharncat2(attrs, *strval, &attrslen);
      APPEND_SQLWCHAR(attrs, attrslen, delim);
    }
    /* only write out int values if they're non-zero */
    else if (intval && *intval)
    {
      attrs+= sqlwcharncat2(attrs, dsnparams[i], &attrslen);
      APPEND_SQLWCHAR(attrs, attrslen, '=');
      sqlwcharfromul(numbuf, *intval);
      attrs+= sqlwcharncat2(attrs, numbuf, &attrslen);
      APPEND_SQLWCHAR(attrs, attrslen, delim);
    }
    else if (boolval && *boolval)
    {
      attrs+= sqlwcharncat2(attrs, dsnparams[i], &attrslen);
      APPEND_SQLWCHAR(attrs, attrslen, '=');
      APPEND_SQLWCHAR(attrs, attrslen, '1');
      APPEND_SQLWCHAR(attrs, attrslen, delim);
    }

    if (!attrslen)
      /* we don't have enough room */
      return -1;
  }

  /* always ends in delimiter, so overwrite it */
  *(attrs - 1)= 0;

  return origchars - attrslen;
}


/*
 * Calculate the length of serializing this DataSource to a string
 * including null terminator. Given in characters.
 */
size_t ds_to_kvpair_len(DataSource *ds)
{
  size_t len= 0;
  int i;
  SQLWCHAR **strval;
  unsigned int *intval;
  BOOL *boolval;
  SQLWCHAR numbuf[21];

  for (i= 0; i < dsnparamcnt; ++i)
  {
    ds_map_param(ds, dsnparams[i], &strval, &intval, &boolval);

    /* We skip the driver if dsn name is given */
    if (!sqlwcharcasecmp(W_DRIVER, dsnparams[i]) && ds->name && *ds->name)
      continue;

    if (strval && *strval && **strval)
    {
      len+= sqlwcharlen(dsnparams[i]);
      len+= sqlwcharlen(*strval);
      if (value_needs_escaped(*strval))
        len += 2; /* for escape braces */
      len+= 2; /* for = and delimiter */
    }
    else if (intval && *intval)
    {
      len+= sqlwcharlen(dsnparams[i]);
      sqlwcharfromul(numbuf, *intval);
      len+= sqlwcharlen(numbuf);
      len+= 2; /* for = and delimiter */
    }
    else if (boolval && *boolval)
    {
      len+= sqlwcharlen(dsnparams[i]);
      len+= 3; /* for = and delimiter and '1' */
    }
  }

  /*
     delimiter is always counted at the end, so we don't add one
     for the null terminator
   */

  return len;
}


/*
 * Utility method for ds_add() to add a single string
 * property via the installer api.
 */
int ds_add_strprop(const SQLWCHAR *name, const SQLWCHAR *propname,
                   const SQLWCHAR *propval)
{
  /* don't write if its null or empty string */
  if (propval && *propval)
  {
    BOOL rc;
    SAVE_MODE();
    rc= SQLWritePrivateProfileStringW(name, propname, propval, W_ODBC_INI);
    if (rc)
      RESTORE_MODE();
    return !rc;
  }

  return 0;
}


/*
 * Utility method for ds_add() to add a single integer
 * property via the installer api.
 */
int ds_add_intprop(const SQLWCHAR *name, const SQLWCHAR *propname, int propval)
{
  SQLWCHAR buf[21];
  sqlwcharfromul(buf, propval);
  return ds_add_strprop(name, propname, buf);
}


/*
 * Add the given datasource to system. Call SQLInstallerError() to get
 * further error details if non-zero is returned. ds->driver should be
 * the driver name.
 */
int ds_add(DataSource *ds)
{
  Driver *driver= NULL;
  int rc= 1;
  SAVE_MODE();

  /* Validate data source name */
  if (!SQLValidDSNW(ds->name))
    goto error;

  RESTORE_MODE();

  /* remove if exists, FYI SQLRemoveDSNFromIni returns true
   * even if the dsn isnt found, false only if there is a failure */
  if (!SQLRemoveDSNFromIniW(ds->name))
    goto error;

  RESTORE_MODE();

  /* Get the actual driver info (not just name) */
  driver= driver_new();
  memcpy(driver->name, ds->driver,
         (sqlwcharlen(ds->driver) + 1) * sizeof(SQLWCHAR));
  if (driver_lookup(driver))
  {
    SQLPostInstallerErrorW(ODBC_ERROR_INVALID_KEYWORD_VALUE,
                           W_CANNOT_FIND_DRIVER);
    goto error;
  }

  /* "Create" section for data source */
  if (!SQLWriteDSNToIniW(ds->name, driver->name))
    goto error;

  RESTORE_MODE();

  /* write all fields (util method takes care of skipping blank fields) */
  if (ds_add_strprop(ds->name, W_DRIVER     , driver->lib    )) goto error;
  if (ds_add_strprop(ds->name, W_DESCRIPTION, ds->description)) goto error;
  if (ds_add_strprop(ds->name, W_SERVER     , ds->server     )) goto error;
  if (ds_add_strprop(ds->name, W_UID        , ds->uid        )) goto error;
  if (ds_add_strprop(ds->name, W_PWD        , ds->pwd        )) goto error;
  if (ds_add_strprop(ds->name, W_DATABASE   , ds->database   )) goto error;
  if (ds_add_strprop(ds->name, W_SOCKET     , ds->socket     )) goto error;
  if (ds_add_strprop(ds->name, W_INITSTMT   , ds->initstmt   )) goto error;
  if (ds_add_strprop(ds->name, W_CHARSET    , ds->charset    )) goto error;
  if (ds_add_strprop(ds->name, W_SSLKEY     , ds->sslkey     )) goto error;
  if (ds_add_strprop(ds->name, W_SSLCERT    , ds->sslcert    )) goto error;
  if (ds_add_strprop(ds->name, W_SSLCA      , ds->sslca      )) goto error;
  if (ds_add_strprop(ds->name, W_SSLCAPATH  , ds->sslcapath  )) goto error;
  if (ds_add_strprop(ds->name, W_SSLCIPHER  , ds->sslcipher  )) goto error;

  if (ds_add_intprop(ds->name, W_SSLVERIFY  , ds->sslverify  )) goto error;
  if (ds_add_intprop(ds->name, W_PORT       , ds->port       )) goto error;
  if (ds_add_intprop(ds->name, W_READTIMEOUT, ds->readtimeout)) goto error;
  if (ds_add_intprop(ds->name, W_WRITETIMEOUT, ds->writetimeout)) goto error;
  if (ds_add_intprop(ds->name, W_CLIENT_INTERACTIVE, ds->clientinteractive)) goto error;
  if (ds_add_intprop(ds->name, W_PREFETCH   , ds->cursor_prefetch_number)) goto error;

  if (ds_add_intprop(ds->name, W_FOUND_ROWS, ds->return_matching_rows)) goto error;
  if (ds_add_intprop(ds->name, W_BIG_PACKETS, ds->allow_big_results)) goto error;
  if (ds_add_intprop(ds->name, W_NO_PROMPT, ds->dont_prompt_upon_connect)) goto error;
  if (ds_add_intprop(ds->name, W_DYNAMIC_CURSOR, ds->dynamic_cursor)) goto error;
  if (ds_add_intprop(ds->name, W_NO_SCHEMA, ds->ignore_N_in_name_table)) goto error;
  if (ds_add_intprop(ds->name, W_NO_DEFAULT_CURSOR, ds->user_manager_cursor)) goto error;
  if (ds_add_intprop(ds->name, W_NO_LOCALE, ds->dont_use_set_locale)) goto error;
  if (ds_add_intprop(ds->name, W_PAD_SPACE, ds->pad_char_to_full_length)) goto error;
  if (ds_add_intprop(ds->name, W_FULL_COLUMN_NAMES, ds->return_table_names_for_SqlDescribeCol)) goto error;
  if (ds_add_intprop(ds->name, W_COMPRESSED_PROTO, ds->use_compressed_protocol)) goto error;
  if (ds_add_intprop(ds->name, W_IGNORE_SPACE, ds->ignore_space_after_function_names)) goto error;
  if (ds_add_intprop(ds->name, W_NAMED_PIPE, ds->force_use_of_named_pipes)) goto error;
  if (ds_add_intprop(ds->name, W_NO_BIGINT, ds->change_bigint_columns_to_int)) goto error;
  if (ds_add_intprop(ds->name, W_NO_CATALOG, ds->no_catalog)) goto error;
  if (ds_add_intprop(ds->name, W_USE_MYCNF, ds->read_options_from_mycnf)) goto error;
  if (ds_add_intprop(ds->name, W_SAFE, ds->safe)) goto error;
  if (ds_add_intprop(ds->name, W_NO_TRANSACTIONS, ds->disable_transactions)) goto error;
  if (ds_add_intprop(ds->name, W_LOG_QUERY, ds->save_queries)) goto error;
  if (ds_add_intprop(ds->name, W_NO_CACHE, ds->dont_cache_result)) goto error;
  if (ds_add_intprop(ds->name, W_FORWARD_CURSOR, ds->force_use_of_forward_only_cursors)) goto error;
  if (ds_add_intprop(ds->name, W_AUTO_RECONNECT, ds->auto_reconnect)) goto error;
  if (ds_add_intprop(ds->name, W_AUTO_IS_NULL, ds->auto_increment_null_search)) goto error;
  if (ds_add_intprop(ds->name, W_ZERO_DATE_TO_MIN, ds->zero_date_to_min)) goto error;
  if (ds_add_intprop(ds->name, W_MIN_DATE_TO_ZERO, ds->min_date_to_zero)) goto error;
  if (ds_add_intprop(ds->name, W_MULTI_STATEMENTS, ds->allow_multiple_statements)) goto error;
  if (ds_add_intprop(ds->name, W_COLUMN_SIZE_S32, ds->limit_column_size)) goto error;
  if (ds_add_intprop(ds->name, W_NO_BINARY_RESULT, ds->handle_binary_as_char)) goto error;
  if (ds_add_intprop(ds->name, W_DFLT_BIGINT_BIND_STR, ds->default_bigint_bind_str)) goto error;
  if (ds_add_intprop(ds->name, W_NO_I_S, ds->no_information_schema)) goto error;
  if (ds_add_intprop(ds->name, W_NO_SSPS, ds->no_ssps)) goto error;
  if (ds_add_intprop(ds->name, W_CAN_HANDLE_EXP_PWD, ds->can_handle_exp_pwd)) goto error;
  if (ds_add_intprop(ds->name, W_ENABLE_CLEARTEXT_PLUGIN, ds->enable_cleartext_plugin)) goto error;
  /* DS_PARAM */

  rc= 0;

error:
  if (driver)
    driver_delete(driver);
  return rc;
}


/*
 * Convenience method to check if data source exists. Set the dsn
 * scope before calling this to narrow the check.
 */
int ds_exists(SQLWCHAR *name)
{
  SQLWCHAR buf[100];
  SAVE_MODE();

  /* get entries and check if data source exists */
  if (SQLGetPrivateProfileStringW(name, NULL, W_EMPTY, buf, 100, W_ODBC_INI))
    return 0;

  RESTORE_MODE();

  return 1;
}


/*
 * Get a copy of an attribute in UTF-8. You can use the attr8 style
 * pointer and it will be freed when the data source is deleted.
 *
 * ex. char *username= ds_get_utf8attr(ds->uid, &ds->uid8);
 */
char *ds_get_utf8attr(SQLWCHAR *attrw, SQLCHAR **attr8)
{
  SQLINTEGER len= SQL_NTS;
  x_free(*attr8);
  *attr8= sqlwchar_as_utf8(attrw, &len);
  return (char *)*attr8;
}


/*
 * Assign a data source attribute from a UTF-8 string.
 */
int ds_setattr_from_utf8(SQLWCHAR **attr, SQLCHAR *val8)
{
  size_t len= strlen((char *)val8);
  x_free(*attr);
  if (!(*attr= (SQLWCHAR *)my_malloc((len + 1) * sizeof(SQLWCHAR), MYF(0))))
    return -1;
  utf8_as_sqlwchar(*attr, len, val8, len);
  return 0;
}


/*
 * Set DataSource member flags from deprecated options value.
 */
void ds_set_options(DataSource *ds, ulong options)
{
  ds->return_matching_rows=                 (options & FLAG_FOUND_ROWS) > 0;
  ds->allow_big_results=                    (options & FLAG_BIG_PACKETS) > 0;
  ds->dont_prompt_upon_connect=             (options & FLAG_NO_PROMPT) > 0;
  ds->dynamic_cursor=                       (options & FLAG_DYNAMIC_CURSOR) > 0;
  ds->ignore_N_in_name_table=               (options & FLAG_NO_SCHEMA) > 0;
  ds->user_manager_cursor=                  (options & FLAG_NO_DEFAULT_CURSOR) > 0;
  ds->dont_use_set_locale=                  (options & FLAG_NO_LOCALE) > 0;
  ds->pad_char_to_full_length=              (options & FLAG_PAD_SPACE) > 0;
  ds->return_table_names_for_SqlDescribeCol=(options & FLAG_FULL_COLUMN_NAMES) > 0;
  ds->use_compressed_protocol=              (options & FLAG_COMPRESSED_PROTO) > 0;
  ds->ignore_space_after_function_names=    (options & FLAG_IGNORE_SPACE) > 0;
  ds->force_use_of_named_pipes=             (options & FLAG_NAMED_PIPE) > 0;
  ds->change_bigint_columns_to_int=         (options & FLAG_NO_BIGINT) > 0;
  ds->no_catalog=                           (options & FLAG_NO_CATALOG) > 0;
  ds->read_options_from_mycnf=              (options & FLAG_USE_MYCNF) > 0;
  ds->safe=                                 (options & FLAG_SAFE) > 0;
  ds->disable_transactions=                 (options & FLAG_NO_TRANSACTIONS) > 0;
  ds->save_queries=                         (options & FLAG_LOG_QUERY) > 0;
  ds->dont_cache_result=                    (options & FLAG_NO_CACHE) > 0;
  ds->force_use_of_forward_only_cursors=    (options & FLAG_FORWARD_CURSOR) > 0;
  ds->auto_reconnect=                       (options & FLAG_AUTO_RECONNECT) > 0;
  ds->auto_increment_null_search=           (options & FLAG_AUTO_IS_NULL) > 0;
  ds->zero_date_to_min=                     (options & FLAG_ZERO_DATE_TO_MIN) > 0;
  ds->min_date_to_zero=                     (options & FLAG_MIN_DATE_TO_ZERO) > 0;
  ds->allow_multiple_statements=            (options & FLAG_MULTI_STATEMENTS) > 0;
  ds->limit_column_size=                    (options & FLAG_COLUMN_SIZE_S32) > 0;
  ds->handle_binary_as_char=                (options & FLAG_NO_BINARY_RESULT) > 0;
  ds->no_information_schema=                (options & FLAG_NO_INFORMATION_SCHEMA) > 0;
  ds->default_bigint_bind_str=              (options & FLAG_DFLT_BIGINT_BIND_STR) > 0;
}


/*
 * Get deprecated options value from DataSource member flags.
 */
ulong ds_get_options(DataSource *ds)
{
  ulong options= 0;

  if (ds->return_matching_rows)
    options|= FLAG_FOUND_ROWS;
  if (ds->allow_big_results)
    options|= FLAG_BIG_PACKETS;
  if (ds->dont_prompt_upon_connect)
    options|= FLAG_NO_PROMPT;
  if (ds->dynamic_cursor)
    options|= FLAG_DYNAMIC_CURSOR;
  if (ds->ignore_N_in_name_table)
    options|= FLAG_NO_SCHEMA;
  if (ds->user_manager_cursor)
    options|= FLAG_NO_DEFAULT_CURSOR;
  if (ds->dont_use_set_locale)
    options|= FLAG_NO_LOCALE;
  if (ds->pad_char_to_full_length)
    options|= FLAG_PAD_SPACE;
  if (ds->return_table_names_for_SqlDescribeCol)
    options|= FLAG_FULL_COLUMN_NAMES;
  if (ds->use_compressed_protocol)
    options|= FLAG_COMPRESSED_PROTO;
  if (ds->ignore_space_after_function_names)
    options|= FLAG_IGNORE_SPACE;
  if (ds->force_use_of_named_pipes)
    options|= FLAG_NAMED_PIPE;
  if (ds->change_bigint_columns_to_int)
    options|= FLAG_NO_BIGINT;
  if (ds->no_catalog)
    options|= FLAG_NO_CATALOG;
  if (ds->read_options_from_mycnf)
    options|= FLAG_USE_MYCNF;
  if (ds->safe)
    options|= FLAG_SAFE;
  if (ds->disable_transactions)
    options|= FLAG_NO_TRANSACTIONS;
  if (ds->save_queries)
    options|= FLAG_LOG_QUERY;
  if (ds->dont_cache_result)
    options|= FLAG_NO_CACHE;
  if (ds->no_information_schema)
    options|= FLAG_NO_INFORMATION_SCHEMA;
  if (ds->force_use_of_forward_only_cursors)
    options|= FLAG_FORWARD_CURSOR;
  if (ds->auto_reconnect)
    options|= FLAG_AUTO_RECONNECT;
  if (ds->auto_increment_null_search)
    options|= FLAG_AUTO_IS_NULL;
  if (ds->zero_date_to_min)
    options|= FLAG_ZERO_DATE_TO_MIN;
  if (ds->min_date_to_zero)
    options|= FLAG_MIN_DATE_TO_ZERO;
  if (ds->allow_multiple_statements)
    options|= FLAG_MULTI_STATEMENTS;
  if (ds->limit_column_size)
    options|= FLAG_COLUMN_SIZE_S32;
  if (ds->handle_binary_as_char)
    options|= FLAG_NO_BINARY_RESULT;
  if (ds->default_bigint_bind_str)
    options|= FLAG_DFLT_BIGINT_BIND_STR;

  return options;
}
