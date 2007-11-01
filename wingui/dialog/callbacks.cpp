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

#include "callbacks.h"
#include "stringutil.h"

WCHAR **		errorMsgs		= NULL;

SQLHDBC			hDBC			= SQL_NULL_HDBC;

wchar_t * mytest(HWND hwnd, DataSource* params)
{
	SQLHDBC hDbc = hDBC;
	SQLHENV hEnv = SQL_NULL_HENV;

	if ( SQL_SUCCEEDED( Connect( hDbc, hEnv, params ) ) )
		return sqlwchardup(L"Connection successful", SQL_NTS);
	else
	{
        wchar_t *tmp= (wchar_t *) my_malloc(512 * sizeof(SQLWCHAR), MYF(0));
        SQLWCHAR state[10];
        SQLINTEGER native;
        SQLSMALLINT len;
        *tmp= 0;

        wcscat(tmp, L"Connection Failed: [");
        len= sqlwcharlen(tmp);
        SQLGetDiagRecW(SQL_HANDLE_DBC, hDbc, 1, state, &native,
                       tmp + len + 7, 512 - len - 8, &len);
        sqlwcharncpy(tmp + sqlwcharlen(tmp), state, 6);
        *(tmp + sqlwcharlen(tmp) + 1) = ' ';
        *(tmp + sqlwcharlen(tmp)) = ']';

        return tmp;
	}

	Disconnect( hDbc, hEnv );
}

BOOL mytestaccept(HWND hwnd, DataSource* params)
{
    /* TODO validation */
	return TRUE;
}

LIST *mygetdatabases(HWND hwnd, DataSource* params)
{
	SQLHENV     hEnv        = SQL_NULL_HENV;
	SQLHDBC     hDbc        = hDBC;
	SQLHSTMT    hStmt;
	SQLRETURN   nReturn;
	SQLWCHAR     szCatalog[MYODBC_DB_NAME_MAX];
	SQLLEN      nCatalog;
    LIST *dbs= NULL;

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
			dbs= list_cons(sqlwchardup(szCatalog, SQL_NTS), dbs);
		else
			break;
	}

	Disconnect( hStmt, hDbc, hEnv );

	return list_reverse(dbs);
}

void myhelp(HWND hwnd)
{
    /** TODO: Rewrite - Shouldn't be windows stuff here */
	MessageBoxW(hwnd, L"HELP", L"Sorry, Help is not Available", MB_OK);
}
