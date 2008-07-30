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
  @file  connect.c
  @brief Connection functions.
*/

#include "driver.h"
#include "installer.h"
#include "stringutil.h"
#include "MYODBCUtil.h"

#ifndef CLIENT_NO_SCHEMA
# define CLIENT_NO_SCHEMA      16
#endif

#ifndef USE_LEGACY_ODBC_GUI
typedef BOOL (*PromptFunc)(SQLHWND, SQLWCHAR *, SQLUSMALLINT,
                           SQLWCHAR *, SQLSMALLINT, SQLSMALLINT *);
#else /* USE_LEGACY_ODBC_GUI */
typedef BOOL (*PromptFunc)(SQLHDBC, SQLHWND, MYODBCUTIL_DATASOURCE *);
#endif /* USE_LEGACY_ODBC_GUI */

/**
  Get the connection flags based on the driver options.

  @param[in]  options  Options flags

  @return Client flags suitable for @c mysql_real_connect().
*/
unsigned long get_client_flags(unsigned long options)
{
  unsigned long flags= CLIENT_MULTI_RESULTS;

  if (options & (FLAG_FOUND_ROWS | FLAG_SAFE))
    flags|= CLIENT_FOUND_ROWS;
  if (options & FLAG_NO_SCHEMA)
    flags|= CLIENT_NO_SCHEMA;
  if (options & FLAG_COMPRESSED_PROTO)
    flags|= CLIENT_COMPRESS;
  if (options & FLAG_IGNORE_SPACE)
    flags|= CLIENT_IGNORE_SPACE;
  if (options & FLAG_MULTI_STATEMENTS)
    flags|= CLIENT_MULTI_STATEMENTS;

  return flags;
}


/**
 If it was specified, set the character set for the connection.

 @param[in]  dbc      Database connection
 @param[in]  charset  Character set name
*/
SQLRETURN myodbc_set_initial_character_set(DBC *dbc, const char *charset)
{
  if (dbc->unicode)
  {
    if (charset && charset[0])
    {
      dbc->ansi_charset_info= get_charset_by_csname(charset,
                                                    MYF(MY_CS_PRIMARY),
                                                    MYF(0));
    }

    charset= "utf8";
  }

  if (charset && charset[0])
  {
    if (mysql_set_character_set(&dbc->mysql, charset))
    {
      set_dbc_error(dbc, "HY000", mysql_error(&dbc->mysql),
                    mysql_errno(&dbc->mysql));
      return SQL_ERROR;
    }
  }
  else
  {
    if (mysql_set_character_set(&dbc->mysql, dbc->ansi_charset_info->csname))
    {
      set_dbc_error(dbc, "HY000", mysql_error(&dbc->mysql),
                    mysql_errno(&dbc->mysql));
      return SQL_ERROR;
    }
  }

  {
    MY_CHARSET_INFO my_charset;
    mysql_get_character_set_info(&dbc->mysql, &my_charset);
    dbc->cxn_charset_info= get_charset(my_charset.number, MYF(0));
  }

  if (!dbc->unicode)
    dbc->ansi_charset_info= dbc->cxn_charset_info;

  /*
    We always set character_set_results to NULL so we can do our own
    conversion to the ANSI character set or Unicode.
  */
  if (odbc_stmt(dbc, "SET character_set_results = NULL") != SQL_SUCCESS)
    return SQL_ERROR;

  return SQL_SUCCESS;
}


/**
  Try to establish a connection to a MySQL server based on the data source
  configuration.

  @param[in]  dbc  Database connection
  @param[in]  ds   Data source information

  @return Standard SQLRETURN code. If it is @c SQL_SUCCESS or @c
  SQL_SUCCESS_WITH_INFO, a connection has been established.
*/
SQLRETURN myodbc_do_connect(DBC *dbc, DataSource *ds)
{
  SQLRETURN rc= SQL_SUCCESS;
  MYSQL *mysql= &dbc->mysql;
  unsigned long options, flags;
  /* Use 'int' and fill all bits to avoid alignment Bug#25920 */
  unsigned int opt_ssl_verify_server_cert = ~0;

  options= sqlwchartoul(ds->option, NULL);

#ifdef WIN32
  /*
   Detect if we are running with ADO present, and force on the
   FLAG_COLUMN_SIZE_S32 option if we are.
  */
  if (GetModuleHandle("msado15.dll") != NULL)
    options|= FLAG_COLUMN_SIZE_S32;

  /* Detect another problem specific to MS Access */
  if (GetModuleHandle("msaccess.exe") != NULL)
    options|= FLAG_DFLT_BIGINT_BIND_STR;
#endif

  mysql_init(mysql);

  flags= get_client_flags(options);

  /* Set other connection options */

  if (options & (FLAG_BIG_PACKETS | FLAG_SAFE))
    /* max_allowed_packet is a magical mysql macro. */
    max_allowed_packet= ~0L;

  if (options & FLAG_NAMED_PIPE)
    mysql_options(mysql, MYSQL_OPT_NAMED_PIPE, NullS);

  if (options & FLAG_USE_MYCNF)
    mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "odbc");

  if (ds->initstmt && ds->initstmt[0])
  {
    /* Check for SET NAMES */
    if (is_set_names_statement((SQLCHAR *)ds_get_utf8attr(ds->initstmt,
                                                          &ds->initstmt8)))
    {
      return set_dbc_error(dbc, "HY000",
                           "SET NAMES not allowed by driver", 0);
    }
    mysql_options(mysql, MYSQL_INIT_COMMAND, ds->initstmt8);
  }

  if (dbc->login_timeout)
    mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT,
                  (char *)&dbc->login_timeout);

  /* set SSL parameters */
  mysql_ssl_set(mysql,
                ds_get_utf8attr(ds->sslkey,    &ds->sslkey8),
                ds_get_utf8attr(ds->sslcert,   &ds->sslcert8),
                ds_get_utf8attr(ds->sslca,     &ds->sslca8),
                ds_get_utf8attr(ds->sslcapath, &ds->sslcapath8),
                ds_get_utf8attr(ds->sslcipher, &ds->sslcipher8));

#ifdef MYSQL_OPT_SSL_VERIFY_SERVER_CERT /* disabled if not in build headers */
  if (ds->sslverify)
    mysql_options(mysql, MYSQL_OPT_SSL_VERIFY_SERVER_CERT,
                (const char *)&opt_ssl_verify_server_cert);
#endif /* MYSQL_OPT_SSL_VERIFY_SERVER_CERT */


  {
    /*
      Get the ANSI charset info before we change connection to UTF-8.
    */
    MY_CHARSET_INFO my_charset;
    mysql_get_character_set_info(&dbc->mysql, &my_charset);
    dbc->ansi_charset_info= get_charset(my_charset.number, MYF(0));

  }

  /*
    We always use utf8 for the connection, and change it afterwards if needed.
  */
  mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "utf8");

  if (!mysql_real_connect(mysql,
                          ds_get_utf8attr(ds->server,   &ds->server8),
                          ds_get_utf8attr(ds->uid,      &ds->uid8),
                          ds_get_utf8attr(ds->pwd,      &ds->pwd8),
                          ds_get_utf8attr(ds->database, &ds->database8),
                          ds->port,
                          ds_get_utf8attr(ds->socket,   &ds->socket8),
                          flags))
  {
    set_dbc_error(dbc, "HY000", mysql_error(mysql), mysql_errno(mysql));
    translate_error(dbc->error.sqlstate, MYERR_S1000, mysql_errno(mysql));
    return SQL_ERROR;
  }

  rc= myodbc_set_initial_character_set(dbc, ds_get_utf8attr(ds->charset,
                                                            &ds->charset8));
  if (!SQL_SUCCEEDED(rc))
  {
    /** @todo set failure reason */
    goto error;
  }

  /*
    The MySQL server has a workaround for old versions of Microsoft Access
    (and possibly other products) that is no longer necessary, but is
    unfortunately enabled by default. We have to turn it off, or it causes
    other problems.
  */
  if (!(options & FLAG_AUTO_IS_NULL) &&
      odbc_stmt(dbc, "SET SQL_AUTO_IS_NULL = 0") != SQL_SUCCESS)
  {
    /** @todo set error reason */
    goto error;
  }

  /* TODO why cant the dbc just keep a copy of the DataSource */
  if (ds->name)
    dbc->dsn= my_strdup(ds_get_utf8attr(ds->name, &ds->name8), MYF(MY_WME));
  if (ds->server)
    dbc->server= my_strdup(ds_get_utf8attr(ds->server, &ds->server8),
                           MYF(MY_WME));
  if (ds->uid)
    dbc->user= my_strdup(ds_get_utf8attr(ds->uid, &ds->uid8), MYF(MY_WME));
  if (ds->pwd)
    dbc->password= my_strdup(ds_get_utf8attr(ds->pwd, &ds->pwd8), MYF(MY_WME));
  if (ds->database)
    dbc->database= my_strdup(ds_get_utf8attr(ds->database, &ds->database8),
                             MYF(MY_WME));
  if (ds->socket)
    dbc->socket= my_strdup(ds_get_utf8attr(ds->socket, &ds->socket8),
                           MYF(MY_WME));

  dbc->port= ds->port;
  dbc->flag= options;

  if (options & FLAG_LOG_QUERY && !dbc->query_log)
    dbc->query_log= init_query_log();

  /* Set the statement error prefix based on the server version. */
  strxmov(dbc->st_error_prefix, MYODBC3_ERROR_PREFIX, "[mysqld-",
          mysql->server_version, "]", NullS);

#ifdef MYSQL_OPT_RECONNECT /* disabled if not included in build header */
  /* This needs to be set after connection, or it doesn't stick.  */
  if (options & FLAG_AUTO_RECONNECT)
  {
    my_bool reconnect= 1;
    mysql_options(mysql, MYSQL_OPT_RECONNECT, (char *)&reconnect);
  }
#endif /* MYSQL_OPT_RECONNECT */

  /* Make sure autocommit is set as configured. */
  if (dbc->commit_flag == CHECK_AUTOCOMMIT_OFF)
  {
    if (!trans_supported(dbc) || (options & FLAG_NO_TRANSACTIONS))
    {
      rc= SQL_SUCCESS_WITH_INFO;
      dbc->commit_flag= CHECK_AUTOCOMMIT_ON;
      set_conn_error(dbc, MYERR_01S02,
                     "Transactions are not enabled, option value "
                     "SQL_AUTOCOMMIT_OFF changed to SQL_AUTOCOMMIT_ON", 0);
    }
    else if (autocommit_on(dbc) && mysql_autocommit(mysql, FALSE))
    {
      /** @todo set error */
      goto error;
    }
  }
  else if ((dbc->commit_flag == CHECK_AUTOCOMMIT_ON) &&
           trans_supported(dbc) && !autocommit_on(dbc))
  {
    if (mysql_autocommit(mysql, TRUE))
    {
      /** @todo set error */
      goto error;
    }
  }

  /* Set transaction isolation as configured. */
  if (dbc->txn_isolation != DEFAULT_TXN_ISOLATION)
  {
    char buff[80];
    const char *level;

    if (dbc->txn_isolation & SQL_TXN_SERIALIZABLE)
      level= "SERIALIZABLE";
    else if (dbc->txn_isolation & SQL_TXN_REPEATABLE_READ)
      level= "REPEATABLE READ";
    else if (dbc->txn_isolation & SQL_TXN_READ_COMMITTED)
      level= "READ COMMITTED";
    else
      level= "READ UNCOMMITTED";

    if (trans_supported(dbc))
    {
      sprintf(buff, "SET SESSION TRANSACTION ISOLATION LEVEL %s", level);
      if (odbc_stmt(dbc, buff) != SQL_SUCCESS)
      {
        /** @todo set error reason */
        goto error;
      }
    }
    else
    {
      dbc->txn_isolation= SQL_TXN_READ_UNCOMMITTED;
      rc= SQL_SUCCESS_WITH_INFO;
      set_conn_error(dbc, MYERR_01S02,
                     "Transactions are not enabled, so transaction isolation "
                     "was ignored.", 0);
    }
  }

  return rc;

error:
  mysql_close(mysql);
  return SQL_ERROR;
}


/**
  Establish a connection to a data source.

  @param[in]  hdbc    Connection handle
  @param[in]  szDSN   Data source name (in connection charset)
  @param[in]  cbDSN   Length of data source name in bytes or @c SQL_NTS
  @param[in]  szUID   User identifier (in connection charset)
  @param[in]  cbUID   Length of user identifier in bytes or @c SQL_NTS
  @param[in]  szAuth  Authentication string (password) (in connection charset)
  @param[in]  cbAuth  Length of authentication string in bytes or @c SQL_NTS

  @return  Standard ODBC success codes

  @since ODBC 1.0
  @since ISO SQL 92
*/
SQLRETURN SQL_API MySQLConnect(SQLHDBC   hdbc,
                               SQLWCHAR *szDSN, SQLSMALLINT cbDSN,
                               SQLWCHAR *szUID, SQLSMALLINT cbUID,
                               SQLWCHAR *szAuth, SQLSMALLINT cbAuth)
{
  SQLRETURN rc;
  DBC *dbc= (DBC *)hdbc;
  DataSource *ds;

#ifdef NO_DRIVERMANAGER
  return set_dbc_error(dbc, "HY000",
                       "SQLConnect requires DSN and driver manager", 0);
#else

  /* Can't connect if we're already connected. */
  if (is_connected(dbc))
    return set_conn_error(hdbc, MYERR_08002, NULL, 0);

  /* Reset error state */
  CLEAR_DBC_ERROR(dbc);

  if (szDSN && !szDSN[0])
  {
    return set_conn_error(hdbc, MYERR_S1000,
                          "Invalid connection parameters", 0);
  }

  ds= ds_new();

  ds_set_strnattr(&ds->name, szDSN, cbDSN);
  ds_set_strnattr(&ds->uid, szUID, cbUID);
  ds_set_strnattr(&ds->pwd, szAuth, cbAuth);

  ds_lookup(ds);

  rc= myodbc_do_connect(dbc, ds);

  ds_delete(ds);
  return rc;
#endif
}


/**
  An alternative to SQLConnect that allows specifying more of the connection
  parameters, and whether or not to prompt the user for more information
  using the setup library.

  NOTE: The prompting done in this function using MYODBCUTIL_DATASOURCE has
        been observed to cause an access violation outside of our code
        (specifically in Access after prompting, before table link list).
        This has only been seen when setting a breakpoint on this function.

  @param[in]  hdbc  Handle of database connection
  @param[in]  hwnd  Window handle. May be @c NULL if no prompting will be done.
  @param[in]  szConnStrIn  The connection string
  @param[in]  cbConnStrIn  The length of the connection string in characters
  @param[out] szConnStrOut Pointer to a buffer for the completed connection
                           string. Must be at least 1024 characters.
  @param[in]  cbConnStrOutMax Length of @c szConnStrOut buffer in characters
  @param[out] pcbConnStrOut Pointer to buffer for returning length of
                            completed connection string, in characters
  @param[in]  fDriverCompletion Complete mode, one of @cSQL_DRIVER_PROMPT,
              @c SQL_DRIVER_COMPLETE, @c SQL_DRIVER_COMPLETE_REQUIRED, or
              @cSQL_DRIVER_NOPROMPT

  @return Standard ODBC return codes

  @since ODBC 1.0
  @since ISO SQL 92
*/
SQLRETURN SQL_API MySQLDriverConnect(SQLHDBC hdbc, SQLHWND hwnd,
                                     SQLWCHAR *szConnStrIn,
                                     SQLSMALLINT cbConnStrIn,
                                     SQLWCHAR *szConnStrOut,
                                     SQLSMALLINT cbConnStrOutMax,
                                     SQLSMALLINT *pcbConnStrOut,
                                     SQLUSMALLINT fDriverCompletion)
{
  SQLRETURN rc= SQL_SUCCESS;
  DBC *dbc= (DBC *)hdbc;
  DataSource *ds= ds_new();
  /* We may have to read driver info to find the setup library. */
  Driver *pDriver= driver_new();
#ifndef USE_LEGACY_ODBC_GUI
  SQLWCHAR *prompt_instr= NULL;
  size_t prompt_inlen;
#else /* USE_LEGACY_ODBC_GUI */
  /* Legacy setup lib, used for prompting, will be deprecated by native guis */
  MYODBCUTIL_DATASOURCE *oldds= MYODBCUtilAllocDataSource(MYODBCUTIL_DATASOURCE_MODE_DRIVER_CONNECT);
#endif /* USE_LEGACY_ODBC_GUI */
  BOOL bPrompt= FALSE;
  HMODULE hModule= NULL;
  unsigned long options;

  if (cbConnStrIn != SQL_NTS)
    szConnStrIn= sqlwchardup(szConnStrIn, cbConnStrIn);

  /* Parse the incoming string */
  if (ds_from_kvpair(ds, szConnStrIn, (SQLWCHAR)';'))
  {
    rc= set_dbc_error(dbc, "HY000",
                      "Failed to parse the incoming connect string.", 0);
    goto error;
  }

#ifndef NO_DRIVERMANAGER
  /*
   If the connection string contains the DSN keyword, the driver retrieves
   the information for the specified data source (and merges it into the
   connection info with the provided connection info having precedence).

   This also allows us to get pszDRIVER (if not already given).
  */
  if (ds->name)
    ds_lookup(ds);
#endif

  /* If FLAG_NO_PROMPT is no set, force prompting off. */
  if (ds->option)
  {
    options= sqlwchartoul(ds->option, NULL);
    if (options & FLAG_NO_PROMPT)
      fDriverCompletion= SQL_DRIVER_NOPROMPT;
  }

  /*
    We only prompt if we need to.

    A not-so-obvious gray area is when SQL_DRIVER_COMPLETE or
    SQL_DRIVER_COMPLETE_REQUIRED was specified along without a server or
    password - particularly password. These can work with defaults/blank but
    many callers expect prompting when these are blank. So we compromise; we
    try to connect and if it works we say its ok otherwise we go to
    a prompt.
  */
  switch (fDriverCompletion)
  {
  case SQL_DRIVER_PROMPT:
    bPrompt= TRUE;
    break;

  case SQL_DRIVER_COMPLETE:
  case SQL_DRIVER_COMPLETE_REQUIRED:
    if (myodbc_do_connect(dbc, ds) == SQL_SUCCESS)
      goto connected;
    bPrompt= TRUE;
    break;

  case SQL_DRIVER_NOPROMPT:
    bPrompt= FALSE;
    break;

  default:
    rc= set_dbc_error(hdbc, "HY110", "Invalid driver completion.", 0);
    goto error;
  }

#ifdef USE_LEGACY_ODBC_GUI
  /* set prompt type for legacy GUI code */
  switch (fDriverCompletion)
  {
  case SQL_DRIVER_PROMPT:
    oldds->nPrompt= MYODBCUTIL_DATASOURCE_PROMPT_PROMPT;
    break;
  case SQL_DRIVER_COMPLETE:
    oldds->nPrompt= MYODBCUTIL_DATASOURCE_PROMPT_COMPLETE;
    break;
  case SQL_DRIVER_COMPLETE_REQUIRED:
    oldds->nPrompt= MYODBCUTIL_DATASOURCE_PROMPT_REQUIRED;
    break;
  case SQL_DRIVER_NOPROMPT:
    oldds->nPrompt= MYODBCUTIL_DATASOURCE_PROMPT_NOPROMPT;
    break;
  }
#endif /* USE_LEGACY_ODBC_GUI */

#ifdef __APPLE__
  /*
    We don't support prompting on Mac OS X, because Qt requires that
    the calling application be a GUI application, and we don't currently
    have a way to guarantee that.
  */
  if (bPrompt)
  {
    rc= set_dbc_error(hdbc, "HY000",
                           "Prompting is not supported on this platform. "
                           "Please provide all required connect information.",
                           0);
    goto error;
  }
#endif

  if (bPrompt)
  {
    PromptFunc pFunc;

    /*
     We can not present a prompt unless we can lookup the name of the setup
     library file name. This means we need a DRIVER. We try to look it up
     using a DSN (above) or, hopefully, get one in the connection string.

     This will, when we need to prompt, trump the ODBC rule where we can
     connect with a DSN which does not exist. A possible solution would be to
     hard-code some fall-back value for ds->pszDRIVER.
    */
    if (!ds->driver)
    {
      char szError[1024];
      sprintf(szError,
              "Could not determine the driver name; "
              "could not lookup setup library. DSN=(%s)\n",
              ds_get_utf8attr(ds->name, &ds->name8));
      rc= set_dbc_error(hdbc, "HY000", szError, 0);
      goto error;
    }

#ifndef NO_DRIVERMANAGER
    /* We can not present a prompt if we have a null window handle. */
    if (!hwnd)
    {
      rc= set_dbc_error(hdbc, "IM008", "Invalid window handle", 0);
      goto error;
    }

    /* if given a named DSN, we will have the path in the DRIVER field */
    if (ds->name)
        sqlwcharncpy(pDriver->lib, ds->driver, ODBCDRIVER_STRLEN);
    /* otherwise, it's the driver name */
    else
        sqlwcharncpy(pDriver->name, ds->driver, ODBCDRIVER_STRLEN);

    if (driver_lookup(pDriver))
    {
      char sz[1024];
      sprintf(sz, "Could not find driver '%s' in system information.",
              ds_get_utf8attr(ds->driver, &ds->driver8));

      rc= set_dbc_error(hdbc, "IM003", sz, 0);
      goto error;
    }

    if (!*pDriver->setup_lib)
#endif
    {
      rc= set_dbc_error(hdbc, "HY000",
                        "Could not determine the file name of setup library.",
                        0);
      goto error;
    }

    /*
     We dynamically load the setup library so the driver itself does not
     depend on GUI libraries.
    */
#ifndef WIN32
    lt_dlinit();
#endif

    if (!(hModule= LoadLibrary(ds_get_utf8attr(pDriver->setup_lib,
                                               &pDriver->setup_lib8))))
    {
      char sz[1024];
      sprintf(sz, "Could not load the setup library '%s'.",
              ds_get_utf8attr(pDriver->setup_lib, &pDriver->setup_lib8));
      rc= set_dbc_error(hdbc, "HY000", sz, 0);
      goto error;
    }

#ifndef USE_LEGACY_ODBC_GUI
    pFunc= (PromptFunc)GetProcAddress(hModule, "Driver_Prompt");
#else /* USE_LEGACY_ODBC_GUI */
    /*
       The setup library should expose a MYODBCSetupDriverConnect() C entry point
       for us to call.
    */
    pFunc= (PromptFunc)GetProcAddress(hModule, "MYODBCSetupDriverConnect");
#endif /* USE_LEGACY_ODBC_GUI */

    if (pFunc == NULL)
    {
#ifdef WIN32
      LPVOID pszMsg;

      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL, GetLastError(),
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR)&pszMsg, 0, NULL);
      rc= set_dbc_error(hdbc, "HY000", (char *)pszMsg, 0);
      LocalFree(pszMsg);
#else
      rc= set_dbc_error(hdbc, "HY000", lt_dlerror(), 0);
#endif
      goto error;
    }

#ifndef USE_LEGACY_ODBC_GUI
    /* create a string for prompting, and add driver manually */
    prompt_inlen= ds_to_kvpair_len(ds) + sqlwcharlen(W_DRIVER_PARAM) +
                  sqlwcharlen(ds->driver) + 1;
    prompt_instr= (SQLWCHAR *) my_malloc(prompt_inlen * sizeof(SQLWCHAR),
                                         MYF(0));
    if (ds_to_kvpair(ds, prompt_instr, prompt_inlen, ';') == -1)
    {
      rc= set_dbc_error(dbc, "HY000",
                        "Failed to prepare prompt input.", 0);
      goto error;
    }
    prompt_inlen-= sqlwcharlen(prompt_instr);
    sqlwcharncat2(prompt_instr, W_DRIVER_PARAM, &prompt_inlen);
    sqlwcharncat2(prompt_instr, ds->driver, &prompt_inlen);

    if (!pFunc(hwnd, prompt_instr, fDriverCompletion,
               szConnStrOut, cbConnStrOutMax, pcbConnStrOut))
    {
      set_dbc_error(hdbc, "HY000", "User cancelled.", 0);
      rc= SQL_NO_DATA;
      goto error;
    }

    /* refresh our DataSource */
    ds_delete(ds);
    ds= ds_new();
    if (ds_from_kvpair(ds, szConnStrOut, ';'))
    {
      rc= set_dbc_error(dbc, "HY000",
                        "Failed to parse the prompt output string.", 0);
      goto error;
    }
#else /* USE_LEGACY_ODBC_GUI */
    /* Copy to the legacy data source structure for prompting */
    if (ds->name)
      oldds->pszDSN=         _global_strdup(ds_get_utf8attr(ds->name, &ds->name8));
    if (ds->description)
      oldds->pszDESCRIPTION= _global_strdup(ds_get_utf8attr(ds->description, &ds->description8));
    if (ds->server)
      oldds->pszSERVER=      _global_strdup(ds_get_utf8attr(ds->server, &ds->server8));
    if (ds->uid)
      oldds->pszUSER=        _global_strdup(ds_get_utf8attr(ds->uid, &ds->uid8));
    if (ds->pwd)
      oldds->pszPASSWORD=    _global_strdup(ds_get_utf8attr(ds->pwd, &ds->pwd8));
    if (ds->database)
      oldds->pszDATABASE=    _global_strdup(ds_get_utf8attr(ds->database, &ds->database8));
    if (ds->socket)
      oldds->pszSOCKET=      _global_strdup(ds_get_utf8attr(ds->socket, &ds->socket8));
    if (ds->initstmt)
      oldds->pszSTMT=        _global_strdup(ds_get_utf8attr(ds->initstmt, &ds->initstmt8));
    if (ds->option)
      oldds->pszOPTION=      _global_strdup(ds_get_utf8attr(ds->option, &ds->option8));
    if (ds->sslkey)
      oldds->pszSSLKEY=      _global_strdup(ds_get_utf8attr(ds->sslkey, &ds->sslkey8));
    if (ds->sslcert)
      oldds->pszSSLCERT=     _global_strdup(ds_get_utf8attr(ds->sslcert, &ds->sslcert8));
    if (ds->sslca)
      oldds->pszSSLCA=       _global_strdup(ds_get_utf8attr(ds->sslca, &ds->sslca8));
    if (ds->sslcapath)
      oldds->pszSSLCAPATH=   _global_strdup(ds_get_utf8attr(ds->sslcapath, &ds->sslcapath8));
    if (ds->sslcipher)
      oldds->pszSSLCIPHER=   _global_strdup(ds_get_utf8attr(ds->sslcipher, &ds->sslcipher8));
    if (ds->charset)
      oldds->pszCHARSET=     _global_strdup(ds_get_utf8attr(ds->charset, &ds->charset8));

    oldds->pszPORT= _global_strdup("        ");
    sprintf(oldds->pszPORT, "%d", ds->port);

    oldds->pszSSL= _global_strdup(" ");
    sprintf(oldds->pszSSLVERIFY,"%d", ds->sslverify);

    /* Prompt. Function returns false if user cancels.  */
    if (!pFunc(hdbc, hwnd, oldds))
    {
      set_dbc_error(hdbc, "HY000", "User cancelled.", 0);
      rc= SQL_NO_DATA;
      goto error;
    }

    /* After prompting, copy data back from legacy datasource struct */
    if (oldds->pszSERVER)
      ds_setattr_from_utf8(&ds->server, oldds->pszSERVER);
    if (oldds->pszUSER)
      ds_setattr_from_utf8(&ds->uid, oldds->pszUSER);
    if (oldds->pszPASSWORD)
      ds_setattr_from_utf8(&ds->pwd, oldds->pszPASSWORD);
    if (oldds->pszDATABASE)
      ds_setattr_from_utf8(&ds->database, oldds->pszDATABASE);
    if (oldds->pszSOCKET)
      ds_setattr_from_utf8(&ds->socket, oldds->pszSOCKET);
    if (oldds->pszSTMT)
      ds_setattr_from_utf8(&ds->initstmt, oldds->pszSTMT);
    if (oldds->pszOPTION)
      ds_setattr_from_utf8(&ds->option, oldds->pszOPTION);
    if (oldds->pszSSLKEY)
      ds_setattr_from_utf8(&ds->sslkey, oldds->pszSSLKEY);
    if (oldds->pszSSLCERT)
      ds_setattr_from_utf8(&ds->sslcert, oldds->pszSSLCERT);
    if (oldds->pszSSLCA)
      ds_setattr_from_utf8(&ds->sslca, oldds->pszSSLCA);
    if (oldds->pszSSLCAPATH)
      ds_setattr_from_utf8(&ds->sslcapath, oldds->pszSSLCAPATH);
    if (oldds->pszSSLCIPHER)
      ds_setattr_from_utf8(&ds->sslcipher, oldds->pszSSLCIPHER);
    if (oldds->pszCHARSET)
      ds_setattr_from_utf8(&ds->charset, oldds->pszCHARSET);
    if (oldds->pszPORT)
      ds->port= strtoul(oldds->pszPORT, NULL, 10);
    if (oldds->pszSSLVERIFY)
      ds->sslverify= strtoul(oldds->pszSSLVERIFY, NULL, 10);

#endif /* USE_LEGACY_ODBC_GUI */
  }

  if ((rc= myodbc_do_connect(dbc, ds)) != SQL_SUCCESS)
    goto error;

connected:

#ifndef USE_LEGACY_ODBC_GUI
  /* copy input to output if connected without prompting */
  if (!bPrompt)
  {
    size_t inlen= (sqlwcharlen(szConnStrIn) + 1) * sizeof(SQLWCHAR);
    size_t copylen= myodbc_min((size_t)cbConnStrOutMax, inlen);
    memcpy(szConnStrOut, szConnStrIn, copylen);
    if (pcbConnStrOut)
      *pcbConnStrOut= copylen / sizeof(SQLWCHAR);
  }

  /* return SQL_SUCCESS_WITH_INFO if truncated output string */
  if (pcbConnStrOut && cbConnStrOutMax < *pcbConnStrOut)
  {
    set_dbc_error(hdbc, "01004", "String data, right truncated.", 0);
    rc= SQL_SUCCESS_WITH_INFO;
  }
#else /* USE_LEGACY_ODBC_GUI */
  if (szConnStrOut /*&& cbConnStrOutMax*/)
  {
#ifdef WIN32
    /*
      Work around a bug in ADO/VB -- the input and output strings are
      set to the same address, but the output string is not guaranteed
      to have the recommended amount of space (1024 bytes).
    */
    if (szConnStrOut != szConnStrIn)
#endif
    {
      if (ds_to_kvpair(ds, szConnStrOut, cbConnStrOutMax, L';') < 0)
      {
        set_dbc_error(dbc, "01000",
                      "Something went wrong while building the "
                      "outgoing connect string.", 0);
        rc= SQL_SUCCESS_WITH_INFO;
      }
    }

    /* TODO set this even if cbConnStrOutMax == 0? */
    if (pcbConnStrOut)
      *pcbConnStrOut= sqlwcharlen(szConnStrOut);
  }
#endif /* USE_LEGACY_ODBC_GUI */

error:
  if (hModule)
    FreeLibrary(hModule);
  if (cbConnStrIn != SQL_NTS)
    x_free(szConnStrIn);

  driver_delete(pDriver);
  ds_delete(ds);
#ifndef USE_LEGACY_ODBC_GUI
  x_free(prompt_instr);
#else /* USE_LEGACY_ODBC_GUI */
  MYODBCUtilFreeDataSource(oldds);
#endif /* USE_LEGACY_ODBC_GUI */

  return rc;
}


/**
  Discover and enumerate the attributes and attribute values required to
  connect.

  @return Always returns @c SQL_ERROR, because the driver does not support this.

  @since ODBC 1.0
*/
SQLRETURN SQL_API
SQLBrowseConnect(SQLHDBC hdbc,
                 SQLCHAR FAR *szConnStrIn __attribute__((unused)),
                 SQLSMALLINT cbConnStrIn __attribute__((unused)),
                 SQLCHAR FAR *szConnStrOut __attribute__((unused)),
                 SQLSMALLINT cbConnStrOutMax __attribute__((unused)),
                 SQLSMALLINT FAR *pcbConnStrOut __attribute__((unused)))
{
  return set_conn_error(hdbc,MYERR_S1000,
                        "Driver does not support this API", 0);
}


/**
  Disconnect a connection.

  @param[in]  hdbc   Connection handle

  @return  Standard ODBC return codes

  @since ODBC 1.0
  @since ISO SQL 92
*/
SQLRETURN SQL_API SQLDisconnect(SQLHDBC hdbc)
{
  LIST *list_element, *next_element;
  DBC FAR *dbc= (DBC FAR*) hdbc;

  for (list_element= dbc->statements; list_element; list_element= next_element)
  {
     next_element= list_element->next;
     my_SQLFreeStmt((SQLHSTMT)list_element->data, SQL_DROP);
  }
  mysql_close(&dbc->mysql);
  x_free(dbc->dsn);
  x_free(dbc->database);
  x_free(dbc->server);
  x_free(dbc->user);
  x_free(dbc->password);
  dbc->dsn= dbc->database= dbc->server= dbc->user= dbc->password= 0;

  if (dbc->flag & FLAG_LOG_QUERY)
    end_query_log(dbc->query_log);

  return SQL_SUCCESS;
}

