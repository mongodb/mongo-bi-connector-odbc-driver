/****************************************************************************
 *                                                                          *
 * File    : 						                                        *
 *                                                                          *
 * Purpose : GUI Callbacks                                                  *
 *                                                                          *
 * History : Date      Reason                                               *
 *           00/00/00  Created                                              *
 *                                                                          *
 ****************************************************************************/

#include "stdafx.h"

#include "callbacks.h"
/*#include "../odbcdialogparams/odbcdialogparams.h"*/
#include "../odbcdialogparams/myString.h"
#include "../../util/MYODBCUtil.h"
#include <WinSock2.h>
#include "../util/stringutil.h"

myString		stringConnectIn	= L"";
WCHAR **		errorMsgs		= NULL;
myString		popupMsg		= L"";

SQLHDBC			hDBC			= SQL_NULL_HDBC;

static WCHAR **	databases		= NULL;

void cleanUp()
{
	clearList(databases);
	clearList(errorMsgs);
}

const wchar_t * mytest(HWND hwnd, DataSource* params)
{
	SQLHDBC hDbc = hDBC;
	SQLHENV hEnv = SQL_NULL_HENV;

	if ( SQL_SUCCEEDED( Connect( hDbc, hEnv, params ) ) )
		return L"Connection successful";
	else
	{
        myString tmp = NULL;

        strAssign(tmp,myString(L"Connection Failed:"));

		strAssign(popupMsg, concat( tmp, popupMsg ) );

        x_free( tmp );

		return popupMsg;
	}

	Disconnect( hDbc, hEnv );
	//MessageBox(hwnd, params->dbname.c_str(), params->drvdesc.c_str(), MB_OK);
}

BOOL mytestaccept(HWND hwnd, DataSource* params)
{
	return true/*(IDYES == MessageBoxW(hwnd, params->dbname.c_str(), params->drvdesc.c_str(), MB_YESNO))*/;
}

const WCHAR** mygetdatabases(HWND hwnd, DataSource* params)
{
	// = { L"DB1", L"DB2", NULL };

	SQLHENV     hEnv        = SQL_NULL_HENV;
	SQLHDBC     hDbc        = hDBC;
	SQLHSTMT    hStmt;
	SQLRETURN   nReturn;
	//			QStringList stringlistDatabases;
	SQLWCHAR     szCatalog[MYODBC_DB_NAME_MAX];//MYODBC_DB_NAME_MAX]; 
	SQLLEN      nCatalog;
	myString    stringConnectIn= buildConnectString( params );

	clearList(databases);

	nReturn = Connect( hDbc, hEnv, params );

	if ( nReturn != SQL_SUCCESS )
		ShowDiagnostics( nReturn, SQL_HANDLE_DBC, hDbc );
	if ( !SQL_SUCCEEDED(nReturn) )
    {
		Disconnect( hDbc,hEnv );
        return NULL;
    }

	nReturn = SQLAllocHandle( SQL_HANDLE_STMT, hDbc, &hStmt );
	if ( nReturn != SQL_SUCCESS )
		ShowDiagnostics( nReturn, SQL_HANDLE_DBC, hDbc );
	if ( !SQL_SUCCEEDED(nReturn) )
    {
		Disconnect( hDbc,hEnv );
        return NULL;
    }

	nReturn = SQLTablesW( hStmt, (SQLWCHAR*)SQL_ALL_CATALOGS, SQL_NTS, (SQLWCHAR*)L"", SQL_NTS, (SQLWCHAR*)L"", 0, (SQLWCHAR*)L"", 0 );

	if ( nReturn != SQL_SUCCESS )
		ShowDiagnostics( nReturn, SQL_HANDLE_STMT, hStmt );
	if ( !SQL_SUCCEEDED(nReturn) )
    {
		Disconnect( hStmt, hDbc, hEnv );
        return NULL;
    }

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

	Disconnect( hStmt, hDbc, hEnv );

	return (const WCHAR**)databases;
}

void myhelp(HWND hwnd)
{
    /** TODO: Rewrite - Shouldn't be windows stuff here */
	MessageBoxW(hwnd, L"HELP", L"Sorry, Help is not Available", MB_OK);
}