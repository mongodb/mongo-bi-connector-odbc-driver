/****************************************************************************
 *                                                                          *
 * File    : main.c								                               *
 *                                                                          *
 * Purpose : Generic dialog based Win32 application.                        *
 *                                                                          *
 * History : Date      Reason                                               *
 *           00/00/00  Created                                              *
 *                                                                          *
 ****************************************************************************/

/* 
 * Either define WIN32_LEAN_AND_MEAN, or one or more of NOCRYPT,
 * NOSERVICE, NOMCX and NOIME, to decrease compile time (if you
 * don't need these defines -- see windows.h).
 */

#define WIN32_LEAN_AND_MEAN
/* #define NOCRYPT */
/* #define NOSERVICE */
/* #define NOMCX */
/* #define NOIME */

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <stdio.h>
#include "resource.h"

#include "../odbcdialogparams/odbcdialogparams.h"
#include "../odbcdialogparams/myString.h"
#include "../../util/MYODBCUtil.h"

myString		stringConnectIn = L"";
WCHAR **		errorMsgs		= NULL;
myString		popupMsg		= L"";

SQLHDBC			hDBC			= SQL_NULL_HDBC;

SQLRETURN Connect( SQLHDBC  &   hDbc, OdbcDialogParams * params );

const wchar_t * mytest(HWND hwnd, OdbcDialogParams* params)
{
	SQLHDBC hDbc = hDBC;

	if ( SQL_SUCCEEDED( Connect( hDbc, params ) ) )
		return L"Connection successful";
	else
	{
		strAssign(popupMsg, concat(myString(L"Connection Failed:"), popupMsg ) );
		return popupMsg.c_str();
	}

	//MessageBox(hwnd, params->dbname.c_str(), params->drvdesc.c_str(), MB_OK);
}

BOOL mytestaccept(HWND hwnd, OdbcDialogParams* params)
{
	return (IDYES == MessageBox(hwnd, params->dbname.c_str(), params->drvdesc.c_str(), MB_YESNO));
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

SQLRETURN Connect( SQLHDBC  &   hDbc, OdbcDialogParams * params )
{
	SQLHENV     hEnv        = SQL_NULL_HENV;
	SQLRETURN   nReturn, result = SQL_SUCCESS;
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
			result = nReturn;
			goto slotLoadDatabaseNamesExit1;
		}

		nReturn = SQLAllocHandle( SQL_HANDLE_DBC, hEnv, &hDbc );
		if ( nReturn != SQL_SUCCESS )
			ShowDiagnostics( nReturn, SQL_HANDLE_ENV, hEnv );
		if ( !SQL_SUCCEEDED(nReturn) )
		{
			result = nReturn;
			goto slotLoadDatabaseNamesExit1;
		}
	}

	nReturn = SQLDriverConnectW( hDbc, NULL, (SQLWCHAR*)( stringConnectIn.c_str() ), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT );

	if ( nReturn != SQL_SUCCESS )
		ShowDiagnostics( nReturn, SQL_HANDLE_DBC, hDbc );
	if ( !SQL_SUCCEEDED(nReturn) )
	{
		result = nReturn;
		goto slotLoadDatabaseNamesExit2;
	}

slotLoadDatabaseNamesExit2:
	nReturn = SQLDisconnect( hDbc );
	if ( hDBC == SQL_NULL_HDBC )
		nReturn = SQLFreeHandle( SQL_HANDLE_DBC, hDbc );
slotLoadDatabaseNamesExit1:
	if ( hDBC == SQL_NULL_HDBC )
		nReturn = SQLFreeHandle( SQL_HANDLE_ENV, hEnv );

	return result;
}

const WCHAR** mygetdatabases(HWND hwnd, OdbcDialogParams* params)
{
	static WCHAR ** databases = NULL;// = { L"DB1", L"DB2", NULL };

	SQLHENV     hEnv        = SQL_NULL_HENV;
	SQLHDBC     hDbc        = hDBC;
	SQLHSTMT    hStmt;
	SQLRETURN   nReturn;
	//			QStringList stringlistDatabases;
	SQLWCHAR     szCatalog[MYODBC_DB_NAME_MAX];//MYODBC_DB_NAME_MAX]; 
	SQLLEN      nCatalog;
	myString    stringConnectIn= buildConnectString( params );

	clearList(databases);

	if ( hDBC == SQL_NULL_HDBC )
	{
		nReturn = SQLAllocHandle( SQL_HANDLE_ENV, NULL, &hEnv );
		if ( nReturn != SQL_SUCCESS )
			ShowDiagnostics( nReturn, SQL_HANDLE_ENV, NULL );
		if ( !SQL_SUCCEEDED(nReturn) )
			return (const WCHAR**)databases;

		nReturn = SQLSetEnvAttr( hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0 );
		if ( nReturn != SQL_SUCCESS )
			ShowDiagnostics( nReturn, SQL_HANDLE_ENV, NULL );
		if ( !SQL_SUCCEEDED(nReturn) )
			goto slotLoadDatabaseNamesExit1;

		nReturn = SQLAllocHandle( SQL_HANDLE_DBC, hEnv, &hDbc );
		if ( nReturn != SQL_SUCCESS )
			ShowDiagnostics( nReturn, SQL_HANDLE_ENV, hEnv );
		if ( !SQL_SUCCEEDED(nReturn) )
			goto slotLoadDatabaseNamesExit1;
	}

	nReturn = SQLDriverConnectW( hDbc, NULL, (SQLWCHAR*)( stringConnectIn.c_str() ), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT );

	if ( nReturn != SQL_SUCCESS )
		ShowDiagnostics( nReturn, SQL_HANDLE_DBC, hDbc );
	if ( !SQL_SUCCEEDED(nReturn) )
		goto slotLoadDatabaseNamesExit2;

	nReturn = SQLAllocHandle( SQL_HANDLE_STMT, hDbc, &hStmt );
	if ( nReturn != SQL_SUCCESS )
		ShowDiagnostics( nReturn, SQL_HANDLE_DBC, hDbc );
	if ( !SQL_SUCCEEDED(nReturn) )
		goto slotLoadDatabaseNamesExit2;

	nReturn = SQLTablesW( hStmt, (SQLWCHAR*)SQL_ALL_CATALOGS, SQL_NTS, (SQLWCHAR*)L"", SQL_NTS, (SQLWCHAR*)L"", 0, (SQLWCHAR*)L"", 0 );
	if ( nReturn != SQL_SUCCESS )
		ShowDiagnostics( nReturn, SQL_HANDLE_STMT, hStmt );
	if ( !SQL_SUCCEEDED(nReturn) )
		goto slotLoadDatabaseNamesExit3;

	nReturn = SQLBindCol( hStmt, 1, SQL_C_WCHAR, szCatalog, MYODBC_DB_NAME_MAX, &nCatalog );
	while ( TRUE )
	{
		nReturn = SQLFetch( hStmt );
		if ( nReturn == SQL_NO_DATA )
			break;
		else if ( nReturn != SQL_SUCCESS )
			ShowDiagnostics( nReturn, SQL_HANDLE_STMT, hStmt );
		if ( SQL_SUCCEEDED(nReturn) )
			add2list( databases, (const wchar_t*)(szCatalog) );
		else
			break;
	}

slotLoadDatabaseNamesExit3:
	nReturn = SQLFreeHandle( SQL_HANDLE_STMT, hStmt );
slotLoadDatabaseNamesExit2:
	nReturn = SQLDisconnect( hDbc );
	if ( hDBC == SQL_NULL_HDBC )
		nReturn = SQLFreeHandle( SQL_HANDLE_DBC, hDbc );
slotLoadDatabaseNamesExit1:
	if ( hDBC == SQL_NULL_HDBC )
		nReturn = SQLFreeHandle( SQL_HANDLE_ENV, hEnv );

	return (const WCHAR**)databases;
}

void myhelp(HWND hwnd)
{
	MessageBox(hwnd, L"HELP", L"HELP", MB_OK);
}
// this is client application
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	OdbcDialogParams Params;
	Params.drvname = L"MySQL-ODBC";
	Params.drvdesc = L"Some Decription";
	Params.srvname = L"";
	Params.port    = 0;
	Params.username = L"root";
	Params.password = L"";
	Params.dbname   = L"";
	// flags 1
	Params.dont_optimize_column_width = false;
	Params.return_matching_rows = true;
	Params.allow_big_results = false;
	Params.use_compressed_protocol = false;
	Params.change_bigint_columns_to_int = false;
	Params.safe = false;
	Params.enable_auto_reconnect = false;
	Params.enable_auto_increment_null_search = false;

    // Initialize common controls. Also needed for MANIFEST's.
    InitCommonControls();
	
	int res = ShowOdbcParamsDialog(L"Demo ODBC Dialog", &Params, NULL, myhelp, mytest, mytestaccept, mygetdatabases);
	const WCHAR* tmp = Params.drvname.c_str(); 
	return res;
}