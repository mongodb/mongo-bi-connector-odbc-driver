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

//#include <stdio.h>
#include "resource.h"

#include "../odbcdialogparams/odbcdialogparams.h"
#include "callbacks.h"

// this is client application
int APIENTRY wWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	OdbcDialogParams Params;
	Params.drvname	= L"MySQL-ODBC";
	Params.drvdesc	= L"";
	Params.srvname	= L"";
	Params.port		= 0;
	Params.username = L"root";
	Params.password = L"";
	Params.dbname	= L"";
	// flags 1
	Params.dont_optimize_column_width	= false;
	Params.return_matching_rows			= true;
	Params.allow_big_results			= false;
	Params.use_compressed_protocol		= false;
	Params.change_bigint_columns_to_int	= false;
	Params.safe							= false;
	Params.enable_auto_reconnect		= false;
	Params.enable_auto_increment_null_search = false;

    // Initialize common controls. Also needed for MANIFEST's.
    InitCommonControls();
	
	int				res = ShowOdbcParamsDialog(L"Demo ODBC Dialog", &Params, NULL, myhelp, mytest, mytestaccept, mygetdatabases);
	const WCHAR*	tmp = Params.drvname.c_str();

	cleanUp();

	return res;
}