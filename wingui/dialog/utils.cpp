/****************************************************************************
 *                                                                          *
 * File    : 						                               *
 *                                                                          *
 * Purpose : utilities for callbacks.							*
 *                                                                          *
 * History : Date      Reason                                               *
 *           00/00/00  Created                                              *
 *                                                                          *
 ****************************************************************************/

#include "stdafx.h"

/** couple of temporary hacks to make it compile here*/
typedef char	byte;
#include <winsock2.h>
/** end hacks */

#include <sql.h>

#include "utils.h"
#include "../odbcdialogparams/myString.h"

extern "C"
{
#include "../driver/driver.h"
#include "../util/stringutil.h"
#include "../util/MYODBCUtil.h"
};

extern	SQLHDBC			hDBC;
extern	myString		stringConnectIn;
extern	WCHAR **		errorMsgs;
extern	myString		popupMsg;

void DecompileOptions(DataSource &params)
{
	ulong nOptions = sqlwchartoul( params.option );

    params.dont_optimize_column_width=				(nOptions & FLAG_FIELD_LENGTH) > 0;
    params.return_matching_rows=                    (nOptions & FLAG_FOUND_ROWS) > 0;  /* 2 */
	params.allow_big_results=						(nOptions & FLAG_BIG_PACKETS) > 0;
	params.dont_prompt_upon_connect=				(nOptions & FLAG_NO_PROMPT) > 0;
	params.enable_dynamic_cursor=					(nOptions & FLAG_DYNAMIC_CURSOR) > 0;
	params.ignore_N_in_name_table=					(nOptions & FLAG_NO_SCHEMA) > 0;
	params.user_manager_cursor=						(nOptions & FLAG_NO_DEFAULT_CURSOR) > 0;
	params.dont_use_set_locale=						(nOptions & FLAG_NO_LOCALE) > 0;
	params.pad_char_to_full_length=					(nOptions & FLAG_PAD_SPACE) > 0;
	params.return_table_names_for_SqlDesribeCol=    (nOptions & FLAG_FULL_COLUMN_NAMES) > 0;
	params.use_compressed_protocol=					(nOptions & FLAG_COMPRESSED_PROTO) > 0;
	params.ignore_space_after_function_names=		(nOptions & FLAG_IGNORE_SPACE) > 0; 
	params.force_use_of_named_pipes=				(nOptions & FLAG_NAMED_PIPE) > 0;          
	params.change_bigint_columns_to_int=			(nOptions & FLAG_NO_BIGINT) > 0;
	params.no_catalog=								(nOptions & FLAG_NO_CATALOG) > 0;
	params.read_options_from_mycnf=					(nOptions & FLAG_USE_MYCNF) > 0;          
	params.safe=									(nOptions & FLAG_SAFE) > 0;
	params.disable_transactions=					(nOptions & FLAG_NO_TRANSACTIONS) > 0;           
	params.save_queries=							(nOptions & FLAG_LOG_QUERY) > 0;
	params.dont_cache_result=						(nOptions & FLAG_NO_CACHE) > 0;
	params.force_use_of_forward_only_cursors=		(nOptions & FLAG_FORWARD_CURSOR) > 0;  
	params.enable_auto_reconnect=					(nOptions & FLAG_AUTO_RECONNECT) > 0;
	params.enable_auto_increment_null_search=		(nOptions & FLAG_AUTO_IS_NULL ) > 0;
}

void FreeEnvHandle( SQLHENV &hEnv )
{
	if ( hDBC == SQL_NULL_HDBC )
		SQLFreeHandle( SQL_HANDLE_ENV, hEnv );
}


void Disconnect( SQLHDBC &hDbc, SQLHENV &hEnv  )
{
	SQLDisconnect( hDbc );

	if ( hDBC == SQL_NULL_HDBC )
		SQLFreeHandle( SQL_HANDLE_DBC, hDbc );

	FreeEnvHandle( hEnv );
}


void Disconnect( SQLHSTMT &hStmt, SQLHDBC &hDbc, SQLHENV &hEnv  )
{
	SQLFreeHandle( SQL_HANDLE_STMT, hStmt );

	Disconnect( hDbc, hEnv );
}


const myString & buildConnectString( DataSource* params )
{
	stringConnectIn = L"DRIVER=";

    if ( myStrlen(params->driver) > 0 )
        concat(stringConnectIn, params->driver);
    else
	    concat( stringConnectIn, MYODBCINST_DRIVER_NAME );

	wchar_t portstr[5];

#ifdef Q_WS_MACX
	/*
	The iODBC that ships with Mac OS X (10.4) must be given a filename for
	the driver library in SQLDriverConnect(), not just the driver name.  So
	we have to look it up using SQLGetPrivateProfileString() if we haven't
	already.
	*/
	{
		if (!params->drvname.empty())
		{
			/*
			SQLGetPrivateProfileString has bugs on iODBC, so we have to check
			both the SYSTEM and USER space explicitly.
			*/
			UWORD configMode;
			if (!SQLGetConfigMode(&configMode))
				return FALSE;
			if (!SQLSetConfigMode(ODBC_SYSTEM_DSN))
				return FALSE;

			char driver[PATH_MAX];
			if (!SQLGetPrivateProfileString(pDataSource->pszDRIVER,
				"DRIVER", pDataSource->pszDRIVER,
				driver, sizeof(driver),
				"ODBCINST.INI"))
				return FALSE;

			/* If we're creating a user DSN, make sure we really got a driver.  */
			if (configMode != ODBC_SYSTEM_DSN &&
				strcmp(driver, pDataSource->pszDRIVER) == 0)
			{
				if (configMode != ODBC_SYSTEM_DSN)
				{
					if (!SQLSetConfigMode(ODBC_USER_DSN))
						return FALSE;
					if (!SQLGetPrivateProfileString(pDataSource->pszDRIVER,
						"DRIVER", pDataSource->pszDRIVER,
						driver, sizeof(driver),
						"ODBCINST.INI"))
						return FALSE;
				}
			}

			pDataSource->pszDriverFileName= _global_strdup(driver);

			if (!SQLSetConfigMode(configMode))
				return FALSE;
		}

		stringConnectIn= concat( stringConnectIn, pDataSource->pszDriverFileName );
	}
	/*
	//#else
	concat(stringConnectIn, params->drvname );//pDataSource->pszDRIVER);*/
#endif

	concat( concat( stringConnectIn, L";UID=" ), params->uid );

	concat( concat( stringConnectIn, L";PWD=" ), params->pwd );

	concat( concat( stringConnectIn, L";SERVER=" ), params->server );

	if ( myStrlen( params->database ) )
		concat( concat( stringConnectIn, L";DATABASE="), params->database );

	if ( params->port > 0 )
	{
		wsprintfW( portstr, L"%d", params->port );
		concat( concat( stringConnectIn, L";PORT=" ), myString( portstr ) );
	}
	if ( myStrlen( params->socket) )
		concat( concat( stringConnectIn, L";SOCKET=" ), params->socket );
	//    if ( myStrlen( params->getOptions()) )
	//        stringConnectIn += ";OPTION=" ), params->getOptions );
	if ( myStrlen( params->initstmt))
		concat( concat( stringConnectIn, L";STMT=" ), params->initstmt );
	if ( myStrlen( params->charset ) )
		concat( concat( stringConnectIn, L";CHARSET=" ), params->charset );
	if ( myStrlen( params->sslkey) )
		concat( concat( stringConnectIn, L";SSLKEY=" ), params->sslkey );
	if ( myStrlen( params->sslcert ) )
		concat( concat( stringConnectIn, L";SSLERT=" ), params->sslcert );
	if ( myStrlen( params->sslca ) )
		concat( concat( stringConnectIn, L";SSLCA=" ), params->sslca);
	if ( myStrlen( params->sslcapath ) )
		concat( concat( stringConnectIn, L";SSLCAPATH=" ), params->sslcapath );
	if ( myStrlen( params->sslcipher ) )
		concat( concat( stringConnectIn, L";SSLCIPHER=" ), params->sslcipher );

	return stringConnectIn;
}


SQLRETURN Connect( SQLHDBC  &   hDbc, SQLHENV   &  hEnv, DataSource * params )
{
	SQLRETURN   nReturn;
	//			QStringList stringlistDatabases;
	myString    stringConnectIn= buildConnectString( params );


	if ( hDBC == SQL_NULL_HDBC )
	{
		nReturn = SQLAllocHandle( SQL_HANDLE_ENV, NULL, &hEnv );

		if ( nReturn != SQL_SUCCESS )
			ShowDiagnostics( nReturn, SQL_HANDLE_ENV, NULL );

		if ( !SQL_SUCCEEDED(nReturn) )
			return nReturn;

		nReturn = SQLSetEnvAttr( hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0 );

		if ( nReturn != SQL_SUCCESS )
			ShowDiagnostics( nReturn, SQL_HANDLE_ENV, NULL );

		if ( !SQL_SUCCEEDED(nReturn) )
		{
			return nReturn;
		}

		nReturn = SQLAllocHandle( SQL_HANDLE_DBC, hEnv, &hDbc );
		if ( nReturn != SQL_SUCCESS )
			ShowDiagnostics( nReturn, SQL_HANDLE_ENV, hEnv );
		if ( !SQL_SUCCEEDED(nReturn) )
		{
			return nReturn;
		}
	}

	nReturn = SQLDriverConnectW( hDbc, NULL, (SQLWCHAR*)( stringConnectIn ), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT );

	if ( nReturn != SQL_SUCCESS )
		ShowDiagnostics( nReturn, SQL_HANDLE_DBC, hDbc );

	return nReturn;
}


void ShowDiagnostics( SQLRETURN nReturn, SQLSMALLINT nHandleType, SQLHANDLE h )
{
	BOOL        bDiagnostics = FALSE;
	SQLSMALLINT nRec = 1;
	SQLWCHAR     szSQLState[6];
	SQLINTEGER  nNative;
	SQLWCHAR     szMessage[SQL_MAX_MESSAGE_LENGTH];
	SQLSMALLINT nMessage;

	if ( h )
	{
		*szSQLState = '\0';
		*szMessage  = '\0';

		while ( SQL_SUCCEEDED( SQLGetDiagRecW( nHandleType,
			h,
			nRec,
			szSQLState,
			&nNative,
			szMessage,
			SQL_MAX_MESSAGE_LENGTH,
			&nMessage ) ) )
		{
			szSQLState[5]               = '\0';
			szMessage[SQL_MAX_MESSAGE_LENGTH - 1]  = '\0';


			add2list(errorMsgs, szMessage);

			bDiagnostics = TRUE;
			nRec++;

			*szSQLState = '\0';
			*szMessage  = '\0';
		}
	}

	switch ( nReturn )
	{
	case SQL_ERROR:
		strAssign( popupMsg, L"Request returned with SQL_ERROR." );//, L"MYODBCConfig" );
		break;
	case SQL_SUCCESS_WITH_INFO:
		strAssign( popupMsg, L"Request return with SQL_SUCCESS_WITH_INFO." );//, L"MYODBCConfig" );
		break;
	case SQL_INVALID_HANDLE:
		strAssign( popupMsg, L"Request returned with SQL_INVALID_HANDLE." );//, L"MYODBCConfig" );
		break;
	default:
		strAssign( popupMsg, L"Request did not return with SQL_SUCCESS." );//, L"MYODBCConfig" );
	}
}


unsigned long CompileOptions( DataSource * params )
{
	unsigned long nFlags = 0;

    if (params->dont_optimize_column_width)				nFlags |= FLAG_FIELD_LENGTH;
    if (params->return_matching_rows)                   nFlags |= FLAG_FOUND_ROWS;  /* 2 */
    if (params->allow_big_results)						nFlags |= FLAG_BIG_PACKETS;
    if (params->dont_prompt_upon_connect)				nFlags |= FLAG_NO_PROMPT;
    if (params->enable_dynamic_cursor)					nFlags |= FLAG_DYNAMIC_CURSOR;
    if (params->ignore_N_in_name_table)					nFlags |= FLAG_NO_SCHEMA;
    if (params->user_manager_cursor)					nFlags |= FLAG_NO_DEFAULT_CURSOR;
    if (params->dont_use_set_locale)					nFlags |= FLAG_NO_LOCALE;
    if (params->pad_char_to_full_length)				nFlags |= FLAG_PAD_SPACE;
    if (params->return_table_names_for_SqlDesribeCol)   nFlags |= FLAG_FULL_COLUMN_NAMES;
    if (params->use_compressed_protocol)				nFlags |= FLAG_COMPRESSED_PROTO;
    if (params->ignore_space_after_function_names)		nFlags |= FLAG_IGNORE_SPACE; 
    if (params->force_use_of_named_pipes)				nFlags |= FLAG_NAMED_PIPE;          
    if (params->change_bigint_columns_to_int)			nFlags |= FLAG_NO_BIGINT;
    if (params->no_catalog)								nFlags |= FLAG_NO_CATALOG;
    if (params->read_options_from_mycnf)				nFlags |= FLAG_USE_MYCNF;          
    if (params->safe)									nFlags |= FLAG_SAFE;
    if (params->disable_transactions)					nFlags |= FLAG_NO_TRANSACTIONS;
    if (params->save_queries)							nFlags |= FLAG_LOG_QUERY;
    if (params->dont_cache_result)						nFlags |= FLAG_NO_CACHE;
    if (params->force_use_of_forward_only_cursors)		nFlags |= FLAG_FORWARD_CURSOR;  
    if (params->enable_auto_reconnect)					nFlags |= FLAG_AUTO_RECONNECT;
    if (params->enable_auto_increment_null_search)		nFlags |= FLAG_AUTO_IS_NULL;

	return nFlags;
}
