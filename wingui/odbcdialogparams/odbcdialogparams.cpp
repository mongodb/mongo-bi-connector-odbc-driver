// odbcdialogparams.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN
/* #define NOCRYPT */
/* #define NOSERVICE */
/* #define NOMCX */
/* #define NOIME */

#define DEBUG_TAB   4
#define SSL_TAB     5

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
/* #include <tchar.h> */
#include <stdio.h>
#include "resource.h"
#include "TabCtrl.h"
#include <assert.h>
#include <commdlg.h>
#include <shlobj.h>
#include <xstring>

#include <winsock2.h>

#include "odbcdialogparams.h"
#include "utils.h"

#include "../MYODBC_MYSQL.h"

extern HINSTANCE ghInstance;

DataSource* pParams = NULL;
PWCHAR pCaption = NULL;
bool OkPressed = false;

static int mod = 1;
static bool flag = false;
bool  BusyIndicator = false;

static TABCTRL TabCtrl_1;

TestButtonPressedCallbackType* gTestButtonPressedCallback = NULL;
HelpButtonPressedCallbackType* gHelpButtonPressedCallback = NULL;
AcceptParamsCallbackType* gAcceptParamsCallback = NULL;
DatabaseNamesCallbackType* gDatabaseNamesCallback = NULL;

void InitStaticValues()
{
	BusyIndicator	= true;
	pParams			= NULL;
	pCaption		= NULL;
	OkPressed		= false;

	mod				= 1;
	flag			= false;
	BusyIndicator	= false;

	gTestButtonPressedCallback	= NULL;
	gHelpButtonPressedCallback	= NULL;
	gAcceptParamsCallback		= NULL;
	gDatabaseNamesCallback		= NULL;
}

#define Refresh(A) RedrawWindow(A,NULL,NULL,RDW_ERASE|RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_UPDATENOW);

BOOL FormMain_DlgProc (HWND, UINT, WPARAM, LPARAM);

void DoEvents (void)
{
	MSG Msg;
	while (PeekMessage(&Msg,NULL,0,0,PM_REMOVE))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
}

VOID OnWMNotify(WPARAM wParam, LPARAM lParam);

static BOOL FormMain_OnNotify (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	OnWMNotify(wParam, lParam);
	int id = (int)wParam;

    switch(id)
	{
		case IDC_TAB1:
		{
			TabControl_Select(&TabCtrl_1); //update internal "this" pointer
			LPNMHDR nm = (LPNMHDR)lParam;
			switch (nm->code)
			{
				case TCN_KEYDOWN:
					TabCtrl_1.OnKeyDown(lParam);

				case TCN_SELCHANGE:
					TabCtrl_1.OnSelChanged();
			}
		}
		break;
	}
	

	return FALSE;
}

void getStrFieldData(HWND hwnd, myString & param, int idc )
{
    if (param)
        *(param) = NULL;

    int len = Edit_GetTextLength(GetDlgItem(hwnd,idc));

    if (len>0)
    {
        if ( myReserveMemory( param, len ) > 0 )
            Edit_GetText(GetDlgItem(hwnd,idc), param, len+1);
    }
}

void getStrFieldData(myString & param, unsigned int framenum, int idc )
{
    assert(TabCtrl_1.hTabPages);
    HWND tab = TabCtrl_1.hTabPages[framenum-1];

    assert(tab);

    getStrFieldData(tab, param, idc );
}

void setUnsignedFieldData(HWND hwnd, unsigned int & param, int idc )
{
    wchar_t buf[1024];
    _itow( param, (wchar_t*)buf, 10 );
    Edit_SetText(GetDlgItem(hwnd,idc), buf);
}

void getUnsignedFieldData( HWND hwnd, unsigned int & param, int idc )
{
    param = 0U;
    int len = Edit_GetTextLength(GetDlgItem(hwnd,idc));

    if(len>0)
    {
        myString tmp = NULL;

        if ( myReserveMemory( tmp, len ) > 0 )
        {
            Edit_GetText(GetDlgItem(hwnd,idc), tmp, len+1);
            param = _wtol(tmp);
        }
    }
}

bool getBoolFieldData(unsigned int framenum, int idc)
{
    assert(TabCtrl_1.hTabPages);
    HWND checkbox = GetDlgItem(TabCtrl_1.hTabPages[framenum-1], idc);

    assert(checkbox);
    if (checkbox)
        return !!Button_GetCheck(checkbox);

    return false;
}

#define GET_STRING(name)    getStrFieldData(hwnd,params.name,IDC_EDIT_##name)

#define SET_STRING(name) \
    Edit_SetText(GetDlgItem(hwnd,IDC_EDIT_##name), params.name)

#define GET_UNSIGNED(name)  getUnsignedFieldData(hwnd,params.name,IDC_EDIT_##name)
#define SET_UNSIGNED(name)  setUnsignedFieldData(hwnd,params.name,IDC_EDIT_##name)

#define GET_BOOL(framenum,name) \
    params.name = getBoolFieldData(framenum,IDC_CHECK_##name)

#define SET_BOOL(framenum,name) \
    Button_SetCheck(GetDlgItem(TabCtrl_1.hTabPages[framenum-1],IDC_CHECK_##name), params.name)

void syncData(HWND hwnd, DataSource &params)
{
    GET_STRING(name);
    GET_STRING(description);
    GET_STRING(server);
    GET_UNSIGNED(port);
    GET_STRING(uid);
    GET_STRING(pwd);
    GET_STRING(database);
}

void syncForm(HWND hwnd, DataSource &params)
{
    SET_STRING(name);
    SET_STRING(description);
    SET_STRING(server);
    SET_UNSIGNED(port);
    SET_STRING(uid);
    SET_STRING(pwd);
    SET_STRING(database);
}
void syncTabsData(HWND hwnd, DataSource &params)
{
    GET_BOOL(1,dont_optimize_column_width);
    GET_BOOL(1,return_matching_rows);
    GET_BOOL(1,allow_big_results);
    GET_BOOL(1,use_compressed_protocol);
    GET_BOOL(1,change_bigint_columns_to_int);
    GET_BOOL(1,safe);
    GET_BOOL(1,enable_auto_reconnect);
    GET_BOOL(1,enable_auto_increment_null_search);
    /* flags 2*/
    GET_BOOL(2,dont_prompt_upon_connect);
    GET_BOOL(2,enable_dynamic_cursor);
    GET_BOOL(2,ignore_N_in_name_table);
    GET_BOOL(2,user_manager_cursor);
    GET_BOOL(2,dont_use_set_locale);
    GET_BOOL(2,pad_char_to_full_length);
    GET_BOOL(2,dont_cache_result);
    /* flags 3 */
    GET_BOOL(3,return_table_names_for_SqlDesribeCol);
    GET_BOOL(3,ignore_space_after_function_names);
    GET_BOOL(3,force_use_of_named_pipes);
    GET_BOOL(3,no_catalog);
    GET_BOOL(3,read_options_from_mycnf);
    GET_BOOL(3,disable_transactions);
    GET_BOOL(3,force_use_of_forward_only_cursors);
    /* debug*/
    GET_BOOL(4,save_queries);

    /* ssl settings */
    getStrFieldData( params.sslkey      , 5, IDC_EDIT_sslkey);
    getStrFieldData( params.sslcert     , 5, IDC_EDIT_sslcert);
    getStrFieldData( params.sslca       , 5, IDC_EDIT_sslca);
    getStrFieldData( params.sslcapath   , 5, IDC_EDIT_sslcapath);
    getStrFieldData( params.sslcipher   , 5, IDC_EDIT_sslkey);
}

void syncTabs(HWND hwnd, DataSource &params)
{
    SET_BOOL(1,dont_optimize_column_width);
    SET_BOOL(1,return_matching_rows);
    SET_BOOL(1,allow_big_results);
    SET_BOOL(1,use_compressed_protocol);
    SET_BOOL(1,change_bigint_columns_to_int);
    SET_BOOL(1,safe);
    SET_BOOL(1,enable_auto_reconnect);
    SET_BOOL(1,enable_auto_increment_null_search);
    /* flags 2*/
    SET_BOOL(2,dont_prompt_upon_connect);
    SET_BOOL(2,enable_dynamic_cursor);
    SET_BOOL(2,ignore_N_in_name_table);
    SET_BOOL(2,user_manager_cursor);
    SET_BOOL(2,dont_use_set_locale);
    SET_BOOL(2,pad_char_to_full_length);
    SET_BOOL(2,dont_cache_result);
    /* flags 3 */
    SET_BOOL(3,return_table_names_for_SqlDesribeCol);
    SET_BOOL(3,ignore_space_after_function_names);
    SET_BOOL(3,force_use_of_named_pipes);
    SET_BOOL(3,no_catalog);
    SET_BOOL(3,read_options_from_mycnf);
    SET_BOOL(3,disable_transactions);
    SET_BOOL(3,force_use_of_forward_only_cursors);
    /* debug*/
    SET_BOOL(DEBUG_TAB,save_queries);

    /* ssl related */
    if ( TabCtrl_1.hTabPages[SSL_TAB-1])
    {
        HWND tabHwnd = TabCtrl_1.hTabPages[SSL_TAB-1];

        Edit_SetText( GetDlgItem( tabHwnd, IDC_EDIT_sslkey)     , params.sslkey);
        Edit_SetText( GetDlgItem( tabHwnd, IDC_EDIT_sslcert)    , params.sslcert);
        Edit_SetText( GetDlgItem( tabHwnd, IDC_EDIT_sslca)      , params.sslca);
        Edit_SetText( GetDlgItem( tabHwnd, IDC_EDIT_sslcapath)  , params.sslcapath);
        Edit_SetText( GetDlgItem( tabHwnd, IDC_EDIT_sslcipher)  , params.sslcipher);
    }

}

void FillParameters(HWND hwnd, DataSource & params)
{
	syncData(hwnd, params );

	if( TabCtrl_1.hTab )
		syncTabsData(hwnd, params);
}

void OnDialogClose();

void FormMain_OnClose(HWND hwnd)
{
	//PostQuitMessage(0);// turn off message loop
    //Unhooks hook(s) :)
    OnDialogClose();

	TabControl_Destroy(&TabCtrl_1);

    DWORD err;
	if (EndDialog(hwnd, 0) == 0)
        err = GetLastError();
}

/****************************************************************************
 *                                                                          *
 * Functions: FormMain_OnCommand related event code                         *
 *                                                                          *
 * Purpose : Handle WM_COMMAND messages: this is the heart of the app.		*
 *                                                                          *
 * History : Date      Reason                                               *
 *           00/00/00  Created                                              *
 *                                                                          *
 ****************************************************************************/

void btnDetails_Click (HWND hwnd)
{
	RECT rect;
	GetWindowRect( hwnd, &rect );
	mod *= -1;
	ShowWindow( GetDlgItem(hwnd,IDC_TAB1), mod > 0? SW_SHOW: SW_HIDE );

	if(!flag && mod==1)
	{
		static PWSTR tabnames[]= {L"Flags 1", L"Flags 2", L"Flags 3", L"Debug", L"SSL Settings", 0};
		static PWSTR dlgnames[]= {MAKEINTRESOURCE(IDD_TAB1),
							  	  MAKEINTRESOURCE(IDD_TAB2),
							  	  MAKEINTRESOURCE(IDD_TAB3),
							  	  MAKEINTRESOURCE(IDD_TAB4),
								  MAKEINTRESOURCE(IDD_TAB5),0};

		New_TabControl( &TabCtrl_1, // address of TabControl struct
					GetDlgItem(hwnd, IDC_TAB1), // handle to tab control
					tabnames, // text for each tab
					dlgnames, // dialog id's of each tab page dialog
					&FormMain_DlgProc, // address of main windows proc
					NULL, // address of size function
					TRUE); // stretch tab page to fit tab ctrl
		flag = true;		

		syncTabs(hwnd, *pParams);
	}
	MoveWindow( hwnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top + 280*mod, TRUE );
}

void btnOk_Click (HWND hwnd)
{
	if ( gAcceptParamsCallback ) 
	{
		/*DataSource params;*/
		FillParameters(hwnd, *pParams);
		if( (*gAcceptParamsCallback)( hwnd, pParams ) )
		{
			OkPressed = true;
            CompileOptions( pParams );
			PostMessage(hwnd, WM_CLOSE, NULL, NULL);
		}
	}
}

void btnCancel_Click (HWND hwnd)
{
    /* OK pressed by default. And only can be false if ok really pressed. so, no need to set it to "false" here*/
	/*OkPressed = false;*/
	PostMessage(hwnd, WM_CLOSE, NULL, NULL);
}

void btnTest_Click (HWND hwnd)
{
	if(gTestButtonPressedCallback)
	{
		/*OdbcDialogParams params;*/
		FillParameters(hwnd, *pParams);

        /*if ( pParams )
            params.driver= pParams->driver;*/

		const wchar_t * testResultMsg = (*gTestButtonPressedCallback)( hwnd, pParams );

		MessageBoxW( hwnd, testResultMsg, pParams->name, MB_OK );
	}
}


void btnHelp_Click (HWND hwnd)
{
	if(gHelpButtonPressedCallback)
	{
		(*gHelpButtonPressedCallback)( hwnd );
	}
}

void chooseFile( HWND parent, int hostCtlId )
{
	OPENFILENAMEW	dialog;

	HWND			hostControl = GetDlgItem( parent, hostCtlId );

	wchar_t			szFile[MAX_PATH];    // buffer for file name

	Edit_GetText( hostControl, szFile, sizeof(szFile) );
	// Initialize OPENFILENAME
	ZeroMemory(&dialog, sizeof(dialog));

	dialog.lStructSize			= sizeof(dialog);
	dialog.lpstrFile			= szFile;

	dialog.lpstrTitle			= L"Select File";
	dialog.nMaxFile				= sizeof(szFile);
	dialog.lpstrFileTitle		= NULL;
	dialog.nMaxFileTitle		= 0;
	dialog.lpstrInitialDir		= NULL;
	dialog.Flags				= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST ;
	dialog.hwndOwner			= parent;
	dialog.lpstrCustomFilter	= L"All Files\0*.*\0PEM\0*.pem\0";
	dialog.nFilterIndex			= 2;

	if ( GetOpenFileNameW( &dialog ) )
	{
		Edit_SetText( hostControl, dialog.lpstrFile );
	}
}

void choosePath( HWND parent, int hostCtlId )
{
	HWND			hostControl = GetDlgItem( parent, hostCtlId );

	BROWSEINFOW		dialog;
	wchar_t			path[MAX_PATH];    // buffer for file name

	Edit_GetText( hostControl, path, sizeof(path) );

	ZeroMemory(&dialog,sizeof(dialog));

	dialog.lpszTitle		= _T("Pick a CA Path");
	dialog.hwndOwner		= parent;
	dialog.pszDisplayName	= path;

	LPITEMIDLIST pidl = SHBrowseForFolder ( &dialog );

	if ( pidl )
	{
		SHGetPathFromIDList ( pidl, path );

		Edit_SetText( hostControl, path );

		IMalloc * imalloc = 0;
		if ( SUCCEEDED( SHGetMalloc ( &imalloc )) )
		{
			imalloc->Free ( pidl );
			imalloc->Release ( );
		}
	}
}
void FormMain_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
		case IDOK:
			btnOk_Click (hwnd); break;
		case IDCANCEL:
			btnCancel_Click (hwnd); break;
		case IDC_BUTTON_DETAILS:
			btnDetails_Click (hwnd); break;
		case IDC_BUTTON_HELP:
			btnHelp_Click (hwnd); break;
		case IDC_BUTTON_TEST:
			btnTest_Click (hwnd); break;
		case IDC_SSLKEYCHOOSER:
			chooseFile( hwnd, IDC_EDIT_sslkey ); break;
		case IDC_SSLCERTCHOOSER:
			chooseFile( hwnd, IDC_EDIT_sslcert ); break;
		case IDC_SSLCACHOOSER:
			chooseFile( hwnd, IDC_EDIT_sslca ); break;
		case IDC_SSLCAPATHCHOOSER:
			choosePath( hwnd, IDC_EDIT_sslcapath ); break;
		case IDC_EDIT_name:
		{
			if (codeNotify==EN_CHANGE) {
				int len = Edit_GetTextLength(GetDlgItem(hwnd,IDC_EDIT_name));
				Button_Enable(GetDlgItem(hwnd,IDOK), len > 0);
				Button_Enable(GetDlgItem(hwnd,IDC_BUTTON_TEST), len > 0);
				RedrawWindow(hwnd,NULL,NULL,RDW_INVALIDATE);	
			}
			break;
		}

		case IDC_EDIT_dbname:
		{
			if(codeNotify==CBN_DROPDOWN && gDatabaseNamesCallback) {
				/*OdbcDialogParams params;*/
				FillParameters(hwnd, *pParams);
				const WCHAR** items = gDatabaseNamesCallback( hwnd, pParams );
				if( items )
				{
					ComboBox_ResetContent(hwndCtl);
					while (*items)
					{
						ComboBox_AddString(hwndCtl, *items);
						items++;
					}
				}
			}
		}
	}

	return;
}

void AlignWindowToBottom(HWND hwnd, int dY);
void AdjustLayout(HWND hwnd);

void FormMain_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	AdjustLayout(hwnd);
}

void AdjustLayout(HWND hwnd)
{
	RECT  rc;
   	GetClientRect(hwnd,&rc);

	BOOL Visible = (mod==-1)?0:1;

	if(TabCtrl_1.hTab)
	{
		EnableWindow( TabCtrl_1.hTab, Visible );
		ShowWindow( TabCtrl_1.hTab, Visible );
	}

	PWSTR pButtonCaption = Visible? L"Details <<" : L"Details >>";
	SetWindowText( GetDlgItem(hwnd,IDC_BUTTON_DETAILS), pButtonCaption );
	const int dY = 20;
	AlignWindowToBottom( GetDlgItem(hwnd,IDC_BUTTON_DETAILS), dY);
	AlignWindowToBottom( GetDlgItem(hwnd,IDOK), dY);
	AlignWindowToBottom( GetDlgItem(hwnd,IDCANCEL), dY);
	AlignWindowToBottom( GetDlgItem(hwnd,IDC_BUTTON_HELP), dY);

	Refresh(hwnd);
}

void AlignWindowToBottom(HWND hwnd, int dY)
{
	if(!hwnd)
		return;
	RECT rect;
	GetWindowRect( hwnd, &rect );
	int h, w;
	RECT rc;
	GetWindowRect(GetParent(hwnd), &rc);

	h=rect.bottom-rect.top;
	w=rect.right-rect.left;

	rc.top = rc.bottom;
	MapWindowPoints(HWND_DESKTOP, GetParent(hwnd), (LPPOINT)&rect, 2);
	MapWindowPoints(HWND_DESKTOP, GetParent(hwnd), (LPPOINT)&rc, 2);

	MoveWindow(hwnd, rect.left, rc.top -dY-h,w,h,FALSE);
}

HWND g_hwndDlg;
BOOL DoCreateDialogTooltip(void);

BOOL FormMain_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	g_hwndDlg = hwnd;
	SetWindowText(hwnd, pCaption);
	//----Everything else must follow the above----//
	btnDetails_Click(hwnd);
	AdjustLayout(hwnd);
	//Get the initial Width and height of the dialog
	//in order to fix the minimum size of dialog

    DecompileOptions( pParams );
	syncForm(hwnd,*pParams);

	BOOL b = DoCreateDialogTooltip();
	return 0;
}


BOOL FormMain_DlgProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		HANDLE_MSG (hwndDlg, WM_CLOSE, FormMain_OnClose);
		HANDLE_MSG (hwndDlg, WM_COMMAND, FormMain_OnCommand);
		HANDLE_MSG (hwndDlg, WM_INITDIALOG, FormMain_OnInitDialog);
		HANDLE_MSG (hwndDlg, WM_SIZE, FormMain_OnSize);
	// There is no message cracker for WM_NOTIFY so redirect manually
	case WM_NOTIFY:
		return FormMain_OnNotify (hwndDlg,wParam,lParam);

	default: return FALSE;
	}
}


// returns FALSE if 'cancel' button pressed
int ShowOdbcParamsDialog(
    PWCHAR caption,
	DataSource* params,                  /*[inout]*/
	HWND ParentWnd,                     /* [in] could be NULL */
	HelpButtonPressedCallbackType* hcallback, /* [in] could be NULL */
	TestButtonPressedCallbackType* tcallback, /* [in] could be NULL */
	AcceptParamsCallbackType* acallback, /* [in] could be NULL */
	DatabaseNamesCallbackType* dcallback)
{
	assert(!BusyIndicator);
	InitStaticValues();

	pParams=                    params;
	pCaption=                   caption;
	gHelpButtonPressedCallback= hcallback;
	gTestButtonPressedCallback= tcallback;
	gAcceptParamsCallback=      acallback;
	gDatabaseNamesCallback=     dcallback;

    // The user interface is a modal dialog box.
    DWORD err;
    
    /*HINSTANCE my= ::GetModuleHandle(L"myodbc3s.dll");
    HANDLE hResInfo= FindResource( ghInstance,MAKEINTRESOURCE(IDD_DIALOG1),RT_DIALOG );
 
    if ( hResInfo)
        params->port = 1111;
    else
        params->port = 0;*/

    INT_PTR res = DialogBox(ghInstance, MAKEINTRESOURCE(IDD_DIALOG1), ParentWnd, (DLGPROC)FormMain_DlgProc);

    if ( res == -1 )
        err = GetLastError();

	BusyIndicator = false;
	return OkPressed;
}
