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

#include <sql.h>

#include "utils.h"
#include "../odbcdialogparams/myString.h"
#include "../../util/MYODBCUtil.h"

extern	SQLHDBC			hDBC;
extern	myString		stringConnectIn;
extern	WCHAR **		errorMsgs;
extern	myString		popupMsg;

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

const myString & buildConnectString( OdbcDialogParams* params )
{
	stringConnectIn = L"DRIVER=";

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


	concat( concat( stringConnectIn, L";UID=" ), params->username );

	concat( concat( stringConnectIn, L";PWD=" ), params->password );

	concat( concat( stringConnectIn, L";SERVER=" ), params->srvname );

	if ( myStrlen( params->dbname ) )
		concat( concat( stringConnectIn, L";DATABASE="), params->dbname );

	if ( params->port > 0 )
	{
		wsprintf( portstr, L"%d", params->port );
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

SQLRETURN Connect( SQLHDBC  &   hDbc, SQLHENV   &  hEnv, OdbcDialogParams * params )
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

	nReturn = SQLDriverConnectW( hDbc, NULL, (SQLWCHAR*)( stringConnectIn.c_str() ), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT );

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

		while ( SQL_SUCCEEDED( SQLGetDiagRec( nHandleType,
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

long CompileOptions( OdbcDialogParams * params )
{
	long result = 0;

	return result;
}