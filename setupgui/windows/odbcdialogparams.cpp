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

/**
 @file  odbcdialogparams.cpp
 @brief Defines the entry point for the DLL application.
*/

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include "resource.h"
#include "TabCtrl.h"
#include <assert.h>
#include <commdlg.h>
#include <shlobj.h>
#include <xstring>
#include <shellapi.h>
#include <winsock2.h>

#include "../setupgui.h"

#include "odbcdialogparams.h"

#include "stringutil.h"

extern HINSTANCE ghInstance;

static DataSource* pParams= NULL;
static PWCHAR      pCaption= NULL;
static int         OkPressed= 0;

static int         mod= 1;
static bool        flag= false;
static bool        BusyIndicator= false;

static TABCTRL     TabCtrl_1;
/* Whether we are in SQLDriverConnect() prompt mode (used to disable fields) */
static BOOL        g_isPrompt;
/* Variable to keep IDC of control where default value were put. It's reset if
   user changes value. Used to verify if we can reset that control's value.
   It won't work if for more than 1 field, but we have only one in visible future. */
static long        controlWithDefValue= 0;

HelpButtonPressedCallbackType* gHelpButtonPressedCallback = NULL;


/* Function that enables/disables groups of controls */
#define SWITCHED_GROUPS 2
#define MAX_GROUP_COTROLS 2
void SwitchTcpOrPipe(HWND hwnd, BOOL usePipe)
{
  /* groups of fields to enable/disable*/
  const int switchedFields[SWITCHED_GROUPS][MAX_GROUP_COTROLS]=
                                    {{IDC_EDIT_server, IDC_EDIT_port},
                                     {IDC_EDIT_socket, 0}};

  /* Default value for enabled empty field */
  const LPCWSTR defaultValues[SWITCHED_GROUPS][MAX_GROUP_COTROLS]=
                                    {{NULL, NULL}, {L"MySQL", NULL}};
  /* Can't be sure that usePipe contains 1 as TRUE*/
  long activeIndex= usePipe ? 1L : 0L;

  for (long i= 0; i < SWITCHED_GROUPS; ++i)
    for (long j= 0; j < MAX_GROUP_COTROLS && switchedFields[i][j] != 0; ++j)
    {
      HWND control= GetDlgItem(hwnd, switchedFields[i][j]);
      EnableWindow(control, i == activeIndex);

      if (defaultValues[i][j] != NULL)
      {
        if (i == activeIndex)
        {
          if (Edit_GetTextLength(control) == 0)
          {
            /* remember that we set that value */
            Edit_SetText(control, defaultValues[i][j]);
            controlWithDefValue= switchedFields[i][j];
          }
        }
        else if (controlWithDefValue == switchedFields[i][j])
        {
          /* we don't want to store the value we set instead of user */
          Edit_SetText(control,L"");
        }
      }
    }
}
#undef SWITCHED_GROUPS
#undef MAX_GROUP_COTROLS


void InitStaticValues()
{
	BusyIndicator= true;
	pParams      = NULL;
	pCaption     = NULL;
	OkPressed    = 0;

	mod          = 1;
	flag         = false;
	BusyIndicator= false;

	gHelpButtonPressedCallback= NULL;
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


void getStrFieldData(HWND hwnd, SQLWCHAR **param, int idc)
{
  x_free(*param);
  *param= NULL;

  int len = Edit_GetTextLength(GetDlgItem(hwnd,idc));

  if (len>0)
  {
    *param= (SQLWCHAR *)my_malloc((len + 1) * sizeof(SQLWCHAR), MYF(0));
    if (*param)
      Edit_GetText(GetDlgItem(hwnd,idc), *param, len+1);
  }
}


void getStrFieldDataTab(SQLWCHAR **param, unsigned int framenum, int idc)
{
  assert(TabCtrl_1.hTabPages);
  HWND tab = TabCtrl_1.hTabPages[framenum-1];

  assert(tab);

  getStrFieldData(tab, param, idc);
}

void setComboFieldDataTab(SQLWCHAR *param, unsigned int framenum, int idc)
{
  if ( TabCtrl_1.hTabPages[framenum-1])
  {
    HWND tabHwndMisc = TabCtrl_1.hTabPages[framenum-1];
    HWND charsetCtrl = GetDlgItem(tabHwndMisc, idc);
    ComboBox_SetText(charsetCtrl, param);
  }
}


void setStrFieldData(HWND hwnd, SQLWCHAR *param, int idc)
{
  Edit_SetText(GetDlgItem(hwnd, idc), param);
}


void setStrFieldDataTab(SQLWCHAR *param, unsigned int framenum, int idc)
{
  assert(TabCtrl_1.hTabPages);
  HWND tab = TabCtrl_1.hTabPages[framenum-1];

  assert(tab);

  setStrFieldData(tab, param, idc);
}


void getUnsignedFieldDataTab(unsigned int framenum, unsigned int *param, int idc )
{
  getUnsignedFieldData(TabCtrl_1.hTabPages[framenum-1], param, idc);
}


void getUnsignedFieldData(HWND hwnd, unsigned int *param, int idc)
{
  *param = 0U;
  int len = Edit_GetTextLength(GetDlgItem(hwnd,idc));

  if(len>0)
  {
    SQLWCHAR *tmp1= (SQLWCHAR *)my_malloc((len + 1) * sizeof(SQLWCHAR),
                                         MYF(0));
    if (tmp1)
    {
      Edit_GetText(GetDlgItem(hwnd,idc), tmp1, len+1);
      *param = _wtol(tmp1);
      x_free(tmp1);
    }
  }
}


void setUnsignedFieldData(HWND hwnd, const unsigned int param, int idc)
{
  wchar_t buf[20];
  _itow( param, (wchar_t*)buf, 10 );
  Edit_SetText(GetDlgItem(hwnd,idc), buf);
}


void setUnsignedFieldDataTab(unsigned int framenum, const unsigned int param, int idc)
{
  setUnsignedFieldData(TabCtrl_1.hTabPages[framenum-1], param, idc);
}


HWND getTabCtrlTab(void)
{
  return TabCtrl_1.hTab;
}


HWND getTabCtrlTabPages(unsigned int framenum)
{
  return TabCtrl_1.hTabPages[framenum];
}


my_bool getBoolFieldDataTab(unsigned int framenum, int idc)
{
  assert(TabCtrl_1.hTabPages);
  HWND checkbox = GetDlgItem(TabCtrl_1.hTabPages[framenum-1], idc);

  assert(checkbox);
  if (checkbox)
      return !!Button_GetCheck(checkbox);

  return false;
}


/* this reads non-DSN bool data */
my_bool getBoolFieldData(HWND hwnd, int idc)
{
  HWND checkbox = GetDlgItem(hwnd, idc);

  assert(checkbox);
  if (checkbox)
      return !!Button_GetCheck(checkbox);

  return false;
}


void setBoolFieldData(HWND hwnd, int idc, my_bool state)
{
  HWND checkbox = GetDlgItem(hwnd, idc);
  assert(checkbox);
  if (checkbox)
  Button_SetCheck(checkbox, state);
}


void setBoolFieldDataTab(unsigned int framenum, int idc, my_bool state)
{
  assert(TabCtrl_1.hTabPages);
  Button_SetCheck(GetDlgItem(TabCtrl_1.hTabPages[framenum-1],idc), state);

}


void setControlEnabled(unsigned int framenum, int idc, my_bool state)
{
  HWND cursorTab= TabCtrl_1.hTabPages[framenum-1];
  assert(cursorTab);

  if (cursorTab)
  {
    EnableWindow(GetDlgItem(cursorTab, idc), state);
  }
}


void OnDialogClose();


void FormMain_OnClose(HWND hwnd)
{
	//PostQuitMessage(0);// turn off message loop
    //Unhooks hook(s) :)
  OnDialogClose();

	TabControl_Destroy(&TabCtrl_1);
  EndDialog(hwnd, NULL);
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
    static PWSTR tabnames[]= {L"Connection", L"Metadata", L"Cursors/Results", L"Debug", L"SSL", L"Misc", 0};
		static PWSTR dlgnames[]= {MAKEINTRESOURCE(IDD_TAB1),
							  	  MAKEINTRESOURCE(IDD_TAB2),
							  	  MAKEINTRESOURCE(IDD_TAB3),
							  	  MAKEINTRESOURCE(IDD_TAB4),
								    MAKEINTRESOURCE(IDD_TAB5),
                    MAKEINTRESOURCE(IDD_TAB6),0};

		New_TabControl( &TabCtrl_1,                 // address of TabControl struct
					          GetDlgItem(hwnd, IDC_TAB1), // handle to tab control
					          tabnames,                   // text for each tab
					          dlgnames,                   // dialog id's of each tab page dialog
					          &FormMain_DlgProc,          // address of main windows proc
					          NULL,                       // address of size function
					          TRUE);                      // stretch tab page to fit tab ctrl
		flag = true;		

		syncTabs(hwnd, pParams);
	}
	MoveWindow( hwnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top + 280*mod, TRUE );
}


void btnOk_Click (HWND hwnd)
{
  FillParameters(hwnd, pParams);

  /* if DS params are valid, close dialog */
  if (mytestaccept(hwnd, pParams))
  {
    OkPressed= 1;
    PostMessage(hwnd, WM_CLOSE, NULL, NULL);
  }
}


void btnCancel_Click (HWND hwnd)
{
  PostMessage(hwnd, WM_CLOSE, NULL, NULL);
}


void btnTest_Click (HWND hwnd)
{
  FillParameters(hwnd, pParams);
  wchar_t *testResultMsg= mytest(hwnd, pParams);
  MessageBoxW(hwnd, testResultMsg, L"Test Result", MB_OK);
  x_free(testResultMsg);
}


void btnHelp_Click (HWND hwnd)
{
  ShellExecute(NULL, L"open",
	       L"http://dev.mysql.com/doc/refman/5.1/en/connector-odbc-configuration-dsn-windows.html",
	       NULL, NULL, SW_SHOWNORMAL);
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

	dialog.lpszTitle		= L"Pick a CA Path";
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

#ifndef MAX_VISIBLE_CB_ITEMS
#define MAX_VISIBLE_CB_ITEMS 20
#endif


/**
   Adjusting height of dropped list of cbHwnd combobox to fit
   itemsCount items, but not more than MAX_VISIBLE_CB_ITEMS
   ComboBox_SetMinVisible not used because it was introduced in XP.
*/
int adjustDropdownHeight(HWND cbHwnd, unsigned int itemsCount)
{
  COMBOBOXINFO  dbcbinfo;
  RECT          ddRect;
  int           newHeight = 0;

  dbcbinfo.cbSize= sizeof(COMBOBOXINFO);
  ComboBox_GetDroppedControlRect(cbHwnd, &ddRect);
  newHeight= ddRect.bottom - ddRect.top;

  if ( GetComboBoxInfo(cbHwnd, &dbcbinfo) )
  {
    itemsCount= itemsCount < 1 ? 1 : (itemsCount > MAX_VISIBLE_CB_ITEMS
                                      ? MAX_VISIBLE_CB_ITEMS
                                      : itemsCount );

    /* + (itemsCount - 1) - 1 pixel spaces between list items */
    newHeight= itemsCount*ComboBox_GetItemHeight(cbHwnd) + (itemsCount - 1);
    MoveWindow(dbcbinfo.hwndList, ddRect.left, ddRect.top, ddRect.right-ddRect.left, newHeight, FALSE);
  }

  return newHeight;
}


/**
   Processing commands for dbname combobox (hwndCtl).
*/
void processDbCombobox(HWND hwnd, HWND hwndCtl, UINT codeNotify)
{
  switch(codeNotify)
  {
    /* Loading list and adjust its height if button clicked and on user input */
    case(CBN_DROPDOWN):
    {
      FillParameters(hwnd, pParams);
      LIST *dbs= mygetdatabases(hwnd, pParams);
      LIST *dbtmp= dbs;

      ComboBox_ResetContent(hwndCtl);

      adjustDropdownHeight(hwndCtl,list_length(dbs));

      for (; dbtmp; dbtmp= list_rest(dbtmp))
        ComboBox_AddString(hwndCtl, (SQLWCHAR *)dbtmp->data);

      list_free(dbs, 1);

      ComboBox_SetText(hwndCtl,pParams->database);

      break;
    }
  }
}


/**
Processing commands for charset combobox (hwndCtl).
*/
void processCharsetCombobox(HWND hwnd, HWND hwndCtl, UINT codeNotify)
{
  switch(codeNotify)
  {
    /* Loading list and adjust its height if button clicked and on user input */
    case(CBN_DROPDOWN):
    {
      //FillParameters(hwnd, *pParams);
      LIST *csl= mygetcharsets(hwnd, pParams);
      LIST *cstmp= csl;

      ComboBox_ResetContent(hwndCtl);

      adjustDropdownHeight(hwndCtl,list_length(csl));

      for (; cstmp; cstmp= list_rest(cstmp))
      ComboBox_AddString(hwndCtl, (SQLWCHAR *)cstmp->data);

      list_free(csl, 1);

      ComboBox_SetText(hwndCtl,pParams->charset);

      break;
    }
  }
}


void FormMain_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
  if (controlWithDefValue != 0 && id == controlWithDefValue
    && codeNotify==EN_CHANGE)
    controlWithDefValue= 0;

  switch (id)
  {
    case IDOK:
      btnOk_Click(hwnd); break;
    case IDCANCEL:
      btnCancel_Click(hwnd); break;
    case IDC_BUTTON_DETAILS:
      btnDetails_Click(hwnd); break;
    case IDC_BUTTON_HELP:
      btnHelp_Click(hwnd); break;
    case IDC_BUTTON_TEST:
      btnTest_Click(hwnd); break;
    case IDC_SSLKEYCHOOSER:
      chooseFile(hwnd, IDC_EDIT_sslkey); break;
    case IDC_SSLCERTCHOOSER:
      chooseFile(hwnd, IDC_EDIT_sslcert); break;
    case IDC_SSLCACHOOSER:
      chooseFile(hwnd, IDC_EDIT_sslca); break;
    case IDC_SSLCAPATHCHOOSER:
      choosePath(hwnd, IDC_EDIT_sslcapath); break;
    case IDC_RSAKEYCHOOSER:
      chooseFile(hwnd, IDC_EDIT_rsakey); break;
    case IDC_RADIO_tcp:
    case IDC_RADIO_pipe:
      SwitchTcpOrPipe(hwnd, !!Button_GetCheck(GetDlgItem(hwnd, IDC_RADIO_pipe)));
      break;
    case IDC_CHECK_cursor_prefetch_active:
      {
        HWND cursorTab= TabCtrl_1.hTabPages[CURSORS_TAB-1];
        assert(cursorTab);
        HWND prefetch= GetDlgItem(cursorTab, IDC_EDIT_cursor_prefetch_number);
        assert(prefetch);

        EnableWindow(prefetch, !!Button_GetCheck(GetDlgItem(cursorTab,
                                            IDC_CHECK_cursor_prefetch_active)));
                  
        if (Edit_GetTextLength(prefetch) == 0)
        {
          setUnsignedFieldData(cursorTab, default_cursor_prefetch,
                              IDC_EDIT_cursor_prefetch_number);
        }
      }
      break;
    case IDC_EDIT_name:
    {
      if (codeNotify==EN_CHANGE)
      {
        int len = Edit_GetTextLength(GetDlgItem(hwnd,IDC_EDIT_name));
        Button_Enable(GetDlgItem(hwnd,IDOK), len > 0);
        Button_Enable(GetDlgItem(hwnd,IDC_BUTTON_TEST), len > 0);
        RedrawWindow(hwnd,NULL,NULL,RDW_INVALIDATE);
      }
      break;
    }

    case IDC_EDIT_dbname:
      processDbCombobox(hwnd, hwndCtl, codeNotify);
    break;

    case IDC_EDIT_charset:
      processCharsetCombobox(hwnd, hwndCtl, codeNotify);
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

	syncForm(hwnd, pParams);

  /* Disable fields if in prompt mode */
  if (g_isPrompt)
  {
    EnableWindow(GetDlgItem(hwnd, IDC_EDIT_name), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_EDIT_description), FALSE);
  }

  /* if prompting without DSN, don't disable OK button */
  /* preserved here old logic + enabled OK if data source
     name is not NULL when not prompting. I don't know why it should be disabled
     when prompting and name is not NULL. */
  if (g_isPrompt == (pParams->name==NULL))
  {
    Button_Enable(GetDlgItem(hwnd,IDOK), 1);
    Button_Enable(GetDlgItem(hwnd,IDC_BUTTON_TEST), 1);
    RedrawWindow(hwnd,NULL,NULL,RDW_INVALIDATE);
  }

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


/*
   Display the DSN dialog

   @param params DataSource struct, should be pre-populated
   @param ParentWnd Parent window handle
   @return 1 if the params were correctly populated and OK was pressed
           0 if the dialog was closed or cancelled
*/
extern "C"
int ShowOdbcParamsDialog(DataSource* params, HWND ParentWnd, BOOL isPrompt)
{
	assert(!BusyIndicator);
	InitStaticValues();

	pParams= params;
	pCaption= L"MySQL Connector/ODBC Data Source Configuration";
  g_isPrompt= isPrompt;

  /*
     If prompting (with a DSN name), or not prompting (add/edit DSN),
     we translate the lib path to the actual driver name.
  */
  if (params->name || !isPrompt)
  {
    Driver *driver= driver_new();
    params->driver && memcpy(driver->lib, params->driver,
           (sqlwcharlen(params->driver) + 1) * sizeof(SQLWCHAR));
    /* TODO Driver lookup is done in driver too, do we really need it there? */
    if (!*driver->lib || driver_lookup_name(driver))
    {
      wchar_t msg[256];
      swprintf(msg, 256, L"Failure to lookup driver entry at path '%ls'",
               driver->lib);
      MessageBox(ParentWnd, msg, L"Cannot find driver entry", MB_OK);
      driver_delete(driver);
      return 0;
    }
    ds_set_strattr(&params->driver, driver->name);
    driver_delete(driver);
  }
  DialogBox(ghInstance, MAKEINTRESOURCE(IDD_DIALOG1), ParentWnd,
            (DLGPROC)FormMain_DlgProc);

	BusyIndicator= false;
	return OkPressed;
}
