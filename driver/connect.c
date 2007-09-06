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

#include "myodbc3.h"

#ifndef _UNIX_
# ifdef USE_IODBC
#  include <iodbcinst.h>
# else
#  include <odbcinst.h>
# endif
#endif

#include "../util/MYODBCUtil.h"

#ifndef CLIENT_NO_SCHEMA
# define CLIENT_NO_SCHEMA      16
#endif


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
#if ((MYSQL_VERSION_ID >= 40113 && MYSQL_VERSION_ID < 50000) || \
     MYSQL_VERSION_ID >= 50006)
  if (charset && charset[0] && mysql_set_character_set(&dbc->mysql, charset))
    return set_dbc_error(dbc, "HY000", mysql_error(&dbc->mysql),
                         mysql_errno(&dbc->mysql));
#endif
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
SQLRETURN myodbc_do_connect(DBC *dbc, MYODBCUTIL_DATASOURCE *ds)
{
  SQLRETURN rc= SQL_SUCCESS;
  MYSQL *mysql= &dbc->mysql;
  unsigned long options, flags;
  unsigned int port;
  /* Use 'int' and fill all bits to avoid alignment Bug#25920 */
  unsigned int opt_ssl_verify_server_cert = ~0;

  /* Make sure default values are set on data source. */
  MYODBCUtilDefaultDataSource(ds);

  options= strtoul(ds->pszOPTION, NULL, 10);
  port= (unsigned int)atoi(ds->pszPORT);

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

  if (ds->pszSTMT && ds->pszSTMT[0])
    mysql_options(mysql, MYSQL_INIT_COMMAND, ds->pszSTMT);

  if (dbc->login_timeout)
    mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT,
                  (char *)&dbc->login_timeout);

  /* set SSL parameters */
  mysql_ssl_set(mysql, ds->pszSSLKEY, ds->pszSSLCERT, ds->pszSSLCA,
                ds->pszSSLCAPATH, ds->pszSSLCIPHER);
  mysql_options(mysql, MYSQL_OPT_SSL_VERIFY_SERVER_CERT,
                (const char *)&opt_ssl_verify_server_cert);

  if (!mysql_real_connect(mysql, ds->pszSERVER, ds->pszUSER, ds->pszPASSWORD,
                          ds->pszDATABASE, port, ds->pszSOCKET, flags))
  {
    set_dbc_error(dbc, "HY000", mysql_error(mysql), mysql_errno(mysql));
    translate_error(dbc->error.sqlstate, MYERR_S1000, mysql_errno(mysql));
    return SQL_ERROR;
  }

  if (!SQL_SUCCEEDED(myodbc_set_initial_character_set(dbc, ds->pszCHARSET)))
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

  if (ds->pszDSN)
    dbc->dsn= my_strdup(ds->pszDSN, MYF(MY_WME));
  if (ds->pszSERVER)
    dbc->server= my_strdup(ds->pszSERVER, MYF(MY_WME));
  if (ds->pszUSER)
    dbc->user= my_strdup(ds->pszUSER, MYF(MY_WME));
  if (ds->pszPASSWORD)
    dbc->password= my_strdup(ds->pszPASSWORD, MYF(MY_WME));
  if (ds->pszDATABASE)
    dbc->database= my_strdup(ds->pszDATABASE, MYF(MY_WME));

  dbc->port= port;
  dbc->flag= options;

  if (options & FLAG_LOG_QUERY && !dbc->query_log)
    dbc->query_log= init_query_log();

  /* Set the statement error prefix based on the server version. */
  strxmov(dbc->st_error_prefix, MYODBC3_ERROR_PREFIX, "[mysqld-",
          mysql->server_version, "]", NullS);

  /* This needs to be set after connection, or it doesn't stick.  */
  if (options & FLAG_AUTO_RECONNECT)
  {
    my_bool reconnect= 1;
    mysql_options(mysql, MYSQL_OPT_RECONNECT, (char *)&reconnect);
  }

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
  @param[in]  szDSN   Data source name
  @param[in]  cbDSN   Length of data source name or @c SQL_NTS
  @param[in]  szUID   User identifier
  @param[in]  cbUID   Length of user identifier or @c SQL_NTS
  @param[in]  szAuth  Authentication string (password)
  @param[in]  cbAuth  Length of authentication string or @c SQL_NTS

  @return  Standard ODBC success codes

  @since ODBC 1.0
  @since ISO SQL 92
*/
SQLRETURN SQL_API SQLConnect(SQLHDBC  hdbc,
                             SQLCHAR *szDSN,  SQLSMALLINT cbDSN,
                             SQLCHAR *szUID,
                             SQLSMALLINT cbUID __attribute__((unused)),
                             SQLCHAR *szAuth,
                             SQLSMALLINT cbAuth __attribute__((unused)))
{
  SQLRETURN rc;
  DBC *dbc= (DBC *)hdbc;
  char dsn[SQL_MAX_DSN_LENGTH], *dsn_ptr;
  MYODBCUTIL_DATASOURCE *ds;

#ifdef NO_DRIVERMANAGER
  return set_dbc_error(dbc, "HY000",
                       "SQLConnect requires DSN and driver manager", 0);
#else

  /* Can't connect if we're already connected. */
  if (is_connected(dbc))
    return set_conn_error(hdbc, MYERR_08002, NULL, 0);

  /* Reset error state */
  CLEAR_DBC_ERROR(dbc);

  dsn_ptr= fix_str(dsn, (char *)szDSN, cbDSN);

  if (dsn_ptr && !dsn_ptr[0])
    return set_conn_error(hdbc, MYERR_S1000,
                          "Invalid connection parameters", 0);

  ds= MYODBCUtilAllocDataSource(MYODBCUTIL_DATASOURCE_MODE_DRIVER_CONNECT);

  /* Set username and password if they were provided. */
  if (szUID && szUID[0])
  {
    if (cbUID == SQL_NTS)
      cbUID= strlen((char *)szUID);
    ds->pszUSER= _global_strndup((char *)szUID, cbUID);
  }
  if (szAuth && szAuth[0])
  {
    if (cbAuth == SQL_NTS)
      cbAuth= strlen((char *)szAuth);
    ds->pszPASSWORD= _global_strndup((char *)szAuth, cbAuth);
  }

  /*
    We don't care if this fails, because we might be able to get away
    with the defaults. We probably should warn about this, though.
  */
  (void)MYODBCUtilReadDataSource(ds, dsn_ptr);

  rc= myodbc_do_connect(dbc, ds);

  MYODBCUtilFreeDataSource(ds);
  return rc;
#endif
}


/**
  An alternative to SQLDriverConnect that allows specifying more of the
  connection parameters, and whether or not to prompt the user for more
  information using the setup library.

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
SQLRETURN SQL_API SQLDriverConnect(SQLHDBC hdbc, SQLHWND hwnd,
                                   SQLCHAR *szConnStrIn,
                                   SQLSMALLINT cbConnStrIn
                                     __attribute__((unused)),
                                   SQLCHAR * szConnStrOut,
                                   SQLSMALLINT cbConnStrOutMax,
                                   SQLSMALLINT *pcbConnStrOut,
                                   SQLUSMALLINT fDriverCompletion)
{
  SQLRETURN rc= SQL_SUCCESS;
  DBC *dbc= (DBC *)hdbc;
  MYODBCUTIL_DATASOURCE *ds=
    MYODBCUtilAllocDataSource(MYODBCUTIL_DATASOURCE_MODE_DRIVER_CONNECT);
  /* We may have to read driver info to find the setup library. */
  MYODBCUTIL_DRIVER *pDriver= MYODBCUtilAllocDriver();
  BOOL bPrompt= FALSE;
  HMODULE hModule= NULL;

  /* Parse the incoming string */
  if (!MYODBCUtilReadConnectStr(ds, (LPCSTR)szConnStrIn))
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
  if (ds->pszDSN)
    (void)MYODBCUtilReadDataSource(ds, ds->pszDSN);
#endif

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
    ds->nPrompt= MYODBCUTIL_DATASOURCE_PROMPT_PROMPT;
    bPrompt= TRUE;
    break;

  case SQL_DRIVER_COMPLETE:
    ds->nPrompt= MYODBCUTIL_DATASOURCE_PROMPT_COMPLETE;
    if (myodbc_do_connect(dbc, ds) == SQL_SUCCESS)
      goto connected;
    bPrompt= TRUE;
    break;

  case SQL_DRIVER_COMPLETE_REQUIRED:
    ds->nPrompt= MYODBCUTIL_DATASOURCE_PROMPT_REQUIRED;
    if (myodbc_do_connect(dbc, ds) == SQL_SUCCESS)
      goto connected;
    bPrompt= TRUE;
    break;

  case SQL_DRIVER_NOPROMPT:
    ds->nPrompt= MYODBCUTIL_DATASOURCE_PROMPT_NOPROMPT;
    bPrompt= FALSE;
    break;

  default:
    rc= set_dbc_error(hdbc, "HY110", "Invalid driver completion.", 0);
    goto error;
  }

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
    BOOL (*pFunc)(SQLHDBC, SQLHWND, MYODBCUTIL_DATASOURCE *);

    /*
     We can not present a prompt unless we can lookup the name of the setup
     library file name. This means we need a DRIVER. We try to look it up
     using a DSN (above) or, hopefully, get one in the connection string.

     This will, when we need to prompt, trump the ODBC rule where we can
     connect with a DSN which does not exist. A possible solution would be to
     hard-code some fall-back value for ds->pszDRIVER.
    */
    if (!ds->pszDRIVER && !ds->pszDriverFileName)
    {
      char szError[1024];
      sprintf(szError,
              "Could not determine the driver name; "
              "could not lookup setup library. DSN=(%s)\n", ds->pszDSN);
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

    /*
     At this point we should have a driver name (friendly name) either
     loaded from DSN or provided in connection string. So lets determine
     the setup library file name (better to not assume name). We read from
     ODBC system info. This allows someone configure for a custom setup
     interface.
    */
    if (!MYODBCUtilReadDriver(pDriver, ds->pszDRIVER, ds->pszDriverFileName))
    {
      char sz[1024];
      if (ds->pszDRIVER && ds->pszDRIVER[0])
        sprintf(sz, "Could not find driver '%s' in system information.",
                ds->pszDRIVER);
      else
        sprintf(sz, "Could not find driver '%s' in system information.",
                ds->pszDriverFileName);

      rc= set_dbc_error(hdbc, "IM003", sz, 0);
      goto error;
    }

    if (!pDriver->pszSETUP )
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

    if (!(hModule= LoadLibrary(pDriver->pszSETUP)))
    {
      char sz[1024];
      sprintf(sz, "Could not load the setup library '%s'.", pDriver->pszSETUP);
      rc= set_dbc_error(hdbc, "HY000", sz, 0);
      goto error;
    }

    /*
       The setup library should expose a MYODBCSetupDriverConnect() C entry point
       for us to call.
    */
    pFunc= (BOOL (*)(SQLHDBC, SQLHWND, MYODBCUTIL_DATASOURCE *))
      GetProcAddress(hModule, "MYODBCSetupDriverConnect");

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

    /* Prompt. Function returns false if user cancels.  */
    if (!pFunc(hdbc, hwnd, ds))
    {
      set_dbc_error(hdbc, "HY000", "User cancelled.", 0);
      rc= ds->bSaveFileDSN ? SQL_NO_DATA : SQL_ERROR;
      goto error;
    }
  }

  if (myodbc_do_connect(dbc, ds) != SQL_SUCCESS)
  {
    if (!ds->bSaveFileDSN)
    {
      /** @todo error message? */
      rc= SQL_ERROR;
      goto error;
    }
    else
    {
      set_dbc_error(hdbc, "08001", "Client unable to establish connection.", 0);
      rc= SQL_SUCCESS_WITH_INFO;
    }
  }

connected:

  if (szConnStrOut)
  {
#ifdef WIN32
    /*
      Work around a bug in ADO/VB -- the intput and output strings are
      set tot he same address, but the output string is not guaranteed
      to have the recommended amount of space (1024 bytes).
    */
    if (szConnStrOut != szConnStrIn)
#endif
    {
      *szConnStrOut= '\0';
      if (!MYODBCUtilWriteConnectStr(ds, (char *)szConnStrOut, cbConnStrOutMax))
      {
        set_dbc_error(dbc, "01000",
                      "Something went wrong while building the "
                      "outgoing connect string.", 0);
        rc= SQL_SUCCESS_WITH_INFO;
      }
    }

    if (pcbConnStrOut)
      *pcbConnStrOut= strlen((char *)szConnStrOut);
  }

error:
  if (hModule)
    FreeLibrary(hModule);

  MYODBCUtilFreeDriver(pDriver);
  MYODBCUtilFreeDataSource(ds);

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
  my_free(dbc->dsn, MYF(0));
  my_free(dbc->database, MYF(0));
  my_free(dbc->server, MYF(0));
  my_free(dbc->user, MYF(0));
  my_free(dbc->password, MYF(0));
  dbc->dsn= dbc->database= dbc->server= dbc->user= dbc->password= 0;

  if (dbc->flag & FLAG_LOG_QUERY)
    end_query_log(dbc->query_log);

  return SQL_SUCCESS;
}

