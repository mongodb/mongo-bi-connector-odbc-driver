/*
  Copyright (c) 2007, 2014, Oracle and/or its affiliates. All rights reserved.

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


/* TODO no L"" */
#include "setupgui.h"
#include "stringutil.h"
#include "windows/resource.h"

SQLWCHAR **errorMsgs= NULL;

SQLHDBC hDBC= SQL_NULL_HDBC;


/*
  Tests if it is possible to establish connection using given DS
  returns message text. user is responsible to free that text after use.
*/
SQLWCHAR *mytest(HWND hwnd, DataSource *params)
{
  SQLHDBC hDbc= hDBC;
  SQLHENV hEnv= SQL_NULL_HENV;
  SQLWCHAR *msg;

  /* 
    In case of file data source we do not want it to be created
    when clicking the Test button
  */
  SQLWCHAR *preservedSavefile= params->savefile;
  params->savefile= 0;

  if (SQL_SUCCEEDED(Connect(&hDbc, &hEnv, params)))
  {
    msg= sqlwchardup(_W(L"Connection Successful"), SQL_NTS);
  }
  else
  {
    SQLWCHAR state[10];
    SQLINTEGER native;
    SQLSMALLINT len;
    SQLWCHAR *ptr;

    msg= (SQLWCHAR *) myodbc_malloc(512 * sizeof(SQLWCHAR), MYF(0));
    *msg= 0;

    sqlwcharncpy(msg, _W(L"Connection Failed\n"), SQL_NTS);
    len= (SQLSMALLINT)sqlwcharlen(msg);

    ptr= msg + len;

    if (SQL_SUCCEEDED(SQLGetDiagRecW(SQL_HANDLE_DBC, hDbc, 1, state,
                                     &native, ptr,
                                     512 - len, &len)))
    {
      ptr= sqlwcharncpy(ptr + len, _W(L": ["), 3);
      ptr= sqlwcharncpy(ptr, state, 6);
      sqlwcharncpy(ptr, _W(L" ]"), 2);
    }
  }

  /* Restore savefile parameter */
  params->savefile= preservedSavefile;

  Disconnect(hDbc, hEnv);
  /* assuming that SQLWCHAR and wchar_t have the same size */
  return msg;
}


BOOL mytestaccept(HWND hwnd, DataSource* params)
{
  /* TODO validation */
  return TRUE;
}


LIST *mygetdatabases(HWND hwnd, DataSource* params)
{
  SQLHENV     hEnv= SQL_NULL_HENV;
  SQLHDBC     hDbc= hDBC;
  SQLHSTMT    hStmt;
  SQLRETURN   nReturn;
  SQLWCHAR    szCatalog[MYODBC_DB_NAME_MAX];
  SQLLEN      nCatalog;
  LIST        *dbs= NULL;
  SQLWCHAR    *preservedDatabase= params->database;
  BOOL        preservedNoCatalog= params->no_catalog;

  /* 
    In case of file data source we do not want it to be created
    when clicking the Test button
  */
  SQLWCHAR *preservedSavefile= params->savefile;
  params->savefile= NULL;

  params->database= NULL;
  params->no_catalog= FALSE;

  nReturn= Connect(&hDbc, &hEnv, params);

  params->savefile= preservedSavefile;
  params->database= preservedDatabase;
  params->no_catalog= preservedNoCatalog;

  if (nReturn != SQL_SUCCESS)
    ShowDiagnostics(nReturn, SQL_HANDLE_DBC, hDbc);
  if (!SQL_SUCCEEDED(nReturn))
  {
    Disconnect(hDbc,hEnv);
    return NULL;
  }

  nReturn= SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
  if (nReturn != SQL_SUCCESS)
    ShowDiagnostics(nReturn, SQL_HANDLE_DBC, hDbc);
  if (!SQL_SUCCEEDED(nReturn))
  {
    Disconnect(hDbc,hEnv);
    return NULL;
  }

  nReturn= SQLTablesW(hStmt, (SQLWCHAR*)SQL_ALL_CATALOGS, SQL_NTS,
                      (SQLWCHAR*)L"", SQL_NTS, (SQLWCHAR*)L"", 0,
                      (SQLWCHAR*)L"", 0);

  if (nReturn != SQL_SUCCESS)
    ShowDiagnostics(nReturn, SQL_HANDLE_STMT, hStmt);
  if (!SQL_SUCCEEDED(nReturn))
  {
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    Disconnect(hDbc, hEnv);
    return NULL;
  }

  nReturn= SQLBindCol(hStmt, 1, SQL_C_WCHAR, szCatalog, MYODBC_DB_NAME_MAX,
                      &nCatalog);
  while (TRUE)
  {
    nReturn= SQLFetch(hStmt);

    if (nReturn == SQL_NO_DATA)
      break;
    else if (nReturn != SQL_SUCCESS)
      ShowDiagnostics(nReturn, SQL_HANDLE_STMT, hStmt);
    if (SQL_SUCCEEDED(nReturn))
      dbs= list_cons(sqlwchardup(szCatalog, SQL_NTS), dbs);
    else
      break;
  }

  SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
  Disconnect(hDbc, hEnv);

  return list_reverse(dbs);
}


LIST *mygetcharsets(HWND hwnd, DataSource* params)
{
  SQLHENV     hEnv= SQL_NULL_HENV;
  SQLHDBC     hDbc= hDBC;
  SQLHSTMT    hStmt;
  SQLRETURN   nReturn;
  SQLWCHAR    szCharset[MYODBC_DB_NAME_MAX];
  SQLLEN      nCharset;
  LIST        *csl= NULL;
  SQLWCHAR    *preservedDatabase= params->database;
  BOOL        preservedNoCatalog= params->no_catalog;

  /* 
    In case of file data source we do not want it to be created
    when clicking the Test button
  */
  SQLWCHAR *preservedSavefile= params->savefile;
  params->savefile= NULL;

  params->database= NULL;
  params->no_catalog= FALSE;

  nReturn= Connect(&hDbc, &hEnv, params);

  params->savefile= preservedSavefile;
  params->database= preservedDatabase;
  params->no_catalog= preservedNoCatalog;

  if (nReturn != SQL_SUCCESS)
    ShowDiagnostics(nReturn, SQL_HANDLE_DBC, hDbc);
  if (!SQL_SUCCEEDED(nReturn))
  {
    Disconnect(hDbc,hEnv);
    return NULL;
  }

  nReturn= SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
  if (nReturn != SQL_SUCCESS)
    ShowDiagnostics(nReturn, SQL_HANDLE_DBC, hDbc);
  if (!SQL_SUCCEEDED(nReturn))
  {
    Disconnect(hDbc,hEnv);
    return NULL;
  }


#ifdef DRIVER_ANSI
  /* Skip undesired charsets */
  nReturn = SQLExecDirectW( hStmt, _W(L"SHOW CHARACTER SET WHERE " \
                                       "charset <> 'utf16' AND " \
                                       "charset <> 'utf32' AND " \
                                       "charset <> 'ucs2'" ), SQL_NTS);
#else
  nReturn = SQLExecDirectW( hStmt, _W(L"SHOW CHARACTER SET"), SQL_NTS);
#endif

  if (nReturn != SQL_SUCCESS)
    ShowDiagnostics(nReturn, SQL_HANDLE_STMT, hStmt);
  if (!SQL_SUCCEEDED(nReturn))
  {
    SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    Disconnect(hDbc, hEnv);
    return NULL;
  }

  nReturn= SQLBindCol(hStmt, 1, SQL_C_WCHAR, szCharset, MYODBC_DB_NAME_MAX,
                      &nCharset);
  while (TRUE)
  {
    nReturn= SQLFetch(hStmt);

    if (nReturn == SQL_NO_DATA)
      break;
    else if (nReturn != SQL_SUCCESS)
      ShowDiagnostics(nReturn, SQL_HANDLE_STMT, hStmt);
    if (SQL_SUCCEEDED(nReturn))
      csl= list_cons(sqlwchardup(szCharset, SQL_NTS), csl);
    else
      break;
  }

  SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
  Disconnect(hDbc, hEnv);

  return list_reverse(csl);
}


/* Init DataSource from the main dialog controls */
void syncData(HWND hwnd, DataSource *params)
{
  GET_STRING(name);
  GET_STRING(description);
  GET_STRING(server);
  GET_STRING(socket);
  GET_UNSIGNED(port);
  GET_STRING(uid);
  GET_STRING(pwd);
  GET_COMBO(database);

#ifdef _WIN32
  /* use this flag exclusively for Windows */
  params->force_use_of_named_pipes = READ_BOOL(hwnd, IDC_RADIO_pipe);
#endif
}


/* Set the main dialog controls using DataSource */
void syncForm(HWND hwnd, DataSource *params)
{
  SET_STRING(name);
  SET_STRING(description);
  SET_STRING(server);
  SET_UNSIGNED(port);
  SET_STRING(uid);
  SET_STRING(pwd);
  SET_STRING(socket);
  SET_COMBO(database);

#ifdef _WIN32
  if (params->force_use_of_named_pipes)
  {
    SET_RADIO(hwnd, IDC_RADIO_pipe, TRUE);
  }
  else
  {
    SET_RADIO(hwnd, IDC_RADIO_tcp, TRUE);
  }
  SwitchTcpOrPipe(hwnd, params->force_use_of_named_pipes);
#else
  if (params->socket)
  {
    /* this flag means the socket file in Linux */
    SET_CHECKED(__UNUSED, use_socket_file, TRUE);
    SET_SENSITIVE(server, FALSE);
    SET_SENSITIVE(socket, TRUE);
  }
  else
  {
    SET_CHECKED(__UNUSED, use_tcp_ip_server, TRUE);
    SET_SENSITIVE(server, TRUE);
    SET_SENSITIVE(socket, FALSE);
  }
#endif
}

/*
 Sets the DataSource fields from the dialog inputs
*/
void syncTabsData(HWND hwnd, DataSource *params)
{  /* 1 - Connection */
  GET_BOOL_TAB(CONNECTION_TAB, allow_big_results);
  GET_BOOL_TAB(CONNECTION_TAB, use_compressed_protocol);
  GET_BOOL_TAB(CONNECTION_TAB, dont_prompt_upon_connect);
  GET_BOOL_TAB(CONNECTION_TAB, auto_reconnect);
  GET_BOOL_TAB(CONNECTION_TAB, allow_multiple_statements);
  GET_BOOL_TAB(CONNECTION_TAB, clientinteractive);
  GET_BOOL_TAB(CONNECTION_TAB, can_handle_exp_pwd);
  GET_BOOL_TAB(CONNECTION_TAB, enable_cleartext_plugin);
  GET_BOOL_TAB(CONNECTION_TAB, disable_ssl_default);

  GET_COMBO_TAB(CONNECTION_TAB, charset);
  GET_STRING_TAB(CONNECTION_TAB, initstmt);
  GET_STRING_TAB(CONNECTION_TAB, plugin_dir);
  GET_STRING_TAB(CONNECTION_TAB, default_auth);

  /* 2 - Metadata*/
  GET_BOOL_TAB(METADATA_TAB, change_bigint_columns_to_int);
  GET_BOOL_TAB(METADATA_TAB, handle_binary_as_char);
  GET_BOOL_TAB(METADATA_TAB, ignore_N_in_name_table);
  GET_BOOL_TAB(METADATA_TAB, return_table_names_for_SqlDescribeCol);
  GET_BOOL_TAB(METADATA_TAB, no_catalog);
  GET_BOOL_TAB(METADATA_TAB, limit_column_size);
  GET_BOOL_TAB(METADATA_TAB, no_information_schema);

  /* 3 - Cursors/Results */
  GET_BOOL_TAB(CURSORS_TAB, return_matching_rows);
  GET_BOOL_TAB(CURSORS_TAB, auto_increment_null_search);
  GET_BOOL_TAB(CURSORS_TAB, dynamic_cursor);
  GET_BOOL_TAB(CURSORS_TAB, user_manager_cursor);
  GET_BOOL_TAB(CURSORS_TAB, pad_char_to_full_length);
  GET_BOOL_TAB(CURSORS_TAB, dont_cache_result);
  GET_BOOL_TAB(CURSORS_TAB, force_use_of_forward_only_cursors);
  GET_BOOL_TAB(CURSORS_TAB, zero_date_to_min);

  if (READ_BOOL_TAB(CURSORS_TAB, cursor_prefetch_active))
  {
    GET_UNSIGNED_TAB(CURSORS_TAB, cursor_prefetch_number);
  }
  else
  {
    params->cursor_prefetch_number= 0;
  }
  /* 4 - debug*/
  GET_BOOL_TAB(DEBUG_TAB,save_queries);

  /* 5 - ssl related */
  GET_STRING_TAB(SSL_TAB, sslkey);
  GET_STRING_TAB(SSL_TAB, sslcert);
  GET_STRING_TAB(SSL_TAB, sslca);
  GET_STRING_TAB(SSL_TAB, sslcapath);
  GET_STRING_TAB(SSL_TAB, sslcipher);
  GET_COMBO_TAB(SSL_TAB, sslmode);

  //GET_BOOL_TAB(SSL_TAB,sslverify);
  GET_STRING_TAB(SSL_TAB, rsakey);
  GET_BOOL_TAB(SSL_TAB, no_tls_1);
  GET_BOOL_TAB(SSL_TAB, no_tls_1_1);
  GET_BOOL_TAB(SSL_TAB, no_tls_1_2);

  /* 6 - Misc*/
  GET_BOOL_TAB(MISC_TAB, safe);
  GET_BOOL_TAB(MISC_TAB, dont_use_set_locale);
  GET_BOOL_TAB(MISC_TAB, ignore_space_after_function_names);
  GET_BOOL_TAB(MISC_TAB, read_options_from_mycnf);
  GET_BOOL_TAB(MISC_TAB, disable_transactions);
  GET_BOOL_TAB(MISC_TAB, min_date_to_zero);
  GET_BOOL_TAB(MISC_TAB, no_ssps);
  GET_BOOL_TAB(MISC_TAB, default_bigint_bind_str);
  GET_BOOL_TAB(MISC_TAB, no_date_overflow);
}

/* 
 Sets the options in the dialog tabs using DataSource 
*/
void syncTabs(HWND hwnd, DataSource *params)
{
  /* 1 - Connection */
  SET_BOOL_TAB(CONNECTION_TAB, allow_big_results);
  SET_BOOL_TAB(CONNECTION_TAB, use_compressed_protocol);
  SET_BOOL_TAB(CONNECTION_TAB, dont_prompt_upon_connect);
  SET_BOOL_TAB(CONNECTION_TAB, auto_reconnect);
  SET_BOOL_TAB(CONNECTION_TAB, allow_multiple_statements);
  SET_BOOL_TAB(CONNECTION_TAB, clientinteractive);
  SET_BOOL_TAB(CONNECTION_TAB, can_handle_exp_pwd);
  SET_BOOL_TAB(CONNECTION_TAB, enable_cleartext_plugin);
  SET_BOOL_TAB(CONNECTION_TAB, disable_ssl_default);

#ifdef _WIN32
  if ( getTabCtrlTabPages(CONNECTION_TAB-1))
#endif
  {
    SET_COMBO_TAB(CONNECTION_TAB, charset);
    SET_STRING_TAB(CONNECTION_TAB,initstmt);
    SET_STRING_TAB(CONNECTION_TAB,plugin_dir);
    SET_STRING_TAB(CONNECTION_TAB,default_auth);
  }

  /* 2 - Metadata*/
  SET_BOOL_TAB(METADATA_TAB, change_bigint_columns_to_int);
  SET_BOOL_TAB(METADATA_TAB, handle_binary_as_char);
  SET_BOOL_TAB(METADATA_TAB, return_table_names_for_SqlDescribeCol);
  SET_BOOL_TAB(METADATA_TAB, ignore_N_in_name_table);
  SET_BOOL_TAB(METADATA_TAB, no_catalog);
  SET_BOOL_TAB(METADATA_TAB, limit_column_size);
  SET_BOOL_TAB(METADATA_TAB, no_information_schema);

  /* 3 - Cursors/Results */
  SET_BOOL_TAB(CURSORS_TAB, return_matching_rows);
  SET_BOOL_TAB(CURSORS_TAB, auto_increment_null_search);
  SET_BOOL_TAB(CURSORS_TAB, dynamic_cursor);
  SET_BOOL_TAB(CURSORS_TAB, user_manager_cursor);
  SET_BOOL_TAB(CURSORS_TAB, pad_char_to_full_length);
  SET_BOOL_TAB(CURSORS_TAB, dont_cache_result);
  SET_BOOL_TAB(CURSORS_TAB, force_use_of_forward_only_cursors);
  SET_BOOL_TAB(CURSORS_TAB, zero_date_to_min);

  if(params->cursor_prefetch_number > 0)
  {
#ifdef _WIN32
    SET_ENABLED(CURSORS_TAB, IDC_EDIT_cursor_prefetch_number, TRUE);
#endif
    SET_CHECKED_TAB(CURSORS_TAB, cursor_prefetch_active, TRUE);
    SET_UNSIGNED_TAB(CURSORS_TAB, cursor_prefetch_number);
  }

  /* 4 - debug*/
  SET_BOOL_TAB(DEBUG_TAB,save_queries); 

  /* 5 - ssl related */
#ifdef _WIN32
  if ( getTabCtrlTabPages(SSL_TAB-1) )
#endif
  {
    if(params->sslkey)
      SET_STRING_TAB(SSL_TAB, sslkey);
    
    if(params->sslcert)
      SET_STRING_TAB(SSL_TAB, sslcert);
    
    if(params->sslca)
      SET_STRING_TAB(SSL_TAB, sslca);
    
    if(params->sslcapath)
      SET_STRING_TAB(SSL_TAB, sslcapath);
    
    if(params->sslcipher)
      SET_STRING_TAB(SSL_TAB, sslcipher);

    if (params->sslmode)
      SET_COMBO_TAB(SSL_TAB, sslmode);

    //SET_BOOL_TAB(SSL_TAB, sslverify);

    if(params->rsakey)
      SET_STRING_TAB(SSL_TAB, rsakey);

    SET_BOOL_TAB(SSL_TAB, no_tls_1);
    SET_BOOL_TAB(SSL_TAB, no_tls_1_1);
    SET_BOOL_TAB(SSL_TAB, no_tls_1_2);
  }

  /* 6 - Misc*/
  SET_BOOL_TAB(MISC_TAB, safe);
  SET_BOOL_TAB(MISC_TAB, dont_use_set_locale);
  SET_BOOL_TAB(MISC_TAB, ignore_space_after_function_names);
  SET_BOOL_TAB(MISC_TAB, read_options_from_mycnf);
  SET_BOOL_TAB(MISC_TAB, disable_transactions);
  SET_BOOL_TAB(MISC_TAB, min_date_to_zero);
  SET_BOOL_TAB(MISC_TAB, no_ssps);
  SET_BOOL_TAB(MISC_TAB, default_bigint_bind_str);
  SET_BOOL_TAB(MISC_TAB, no_date_overflow);
}

void FillParameters(HWND hwnd, DataSource *params)
{
  syncData(hwnd, params );
#ifdef _WIN32
  if(getTabCtrlTab())
#endif
    syncTabsData(hwnd, params);
}
