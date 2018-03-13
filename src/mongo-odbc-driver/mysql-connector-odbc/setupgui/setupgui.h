/*
  Copyright (c) 2007, 2013, Oracle and/or its affiliates. All rights reserved.

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

#ifndef _SETUPGUI_H
#define _SETUPGUI_H

#ifdef _WIN32
# include <winsock2.h>
# include <windows.h>

#define CONNECTION_TAB  1
#define METADATA_TAB    2
#define CURSORS_TAB     3
#define DEBUG_TAB       4
#define SSL_TAB         5
#define MISC_TAB        6

#else
# include <gtk/gtk.h>
# include <gdk/gdkkeysyms.h>

/* Older versions of GDK have different macros for keys */

# ifndef GDK_KEY_Up
#   define GDK_KEY_Up GDK_Up
# endif

# ifndef GDK_KEY_Down
#   define GDK_KEY_Down GDK_Down
# endif

# ifndef GDK_KEY_Tab
#   define GDK_KEY_Tab GDK_Tab
# endif

# ifndef GDK_KEY_ISO_Left_Tab
#   define GDK_KEY_ISO_Left_Tab GDK_ISO_Left_Tab
# endif

#endif

#include "MYODBC_MYSQL.h"
#include "installer.h"

#include <sql.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Utility functions. */
void ShowDiagnostics(SQLRETURN nReturn, SQLSMALLINT nHandleType, SQLHANDLE h);
void FreeEnvHandle(SQLHENV hEnv);
SQLRETURN Connect(SQLHDBC *hDbc, SQLHENV *hEnv, DataSource *params);
void Disconnect(SQLHDBC hDbc, SQLHENV hEnv);

/* Max DB name len, used when retrieving database list */
#define MYODBC_DB_NAME_MAX 255

/* Callbacks */
SQLWCHAR *mytest(HWND hwnd, DataSource *params);
BOOL mytestaccept(HWND hwnd, DataSource *params);
LIST *mygetdatabases(HWND hwnd, DataSource *params);
LIST *mygetcharsets(HWND hwnd, DataSource *params);

/* Sync data functions */
void syncData(HWND hwnd, DataSource *params);
void syncForm(HWND hwnd, DataSource *params);
void syncTabsData(HWND hwnd, DataSource *params);
void syncTabs(HWND hwnd, DataSource *params);
void FillParameters(HWND hwnd, DataSource *params);
  
/* Temporary buffer used in conversion macro */
SQLWCHAR tmpbuf[1024];

#define _W(string) wchar_t_as_sqlwchar((wchar_t*)string, (SQLWCHAR*)tmpbuf, \
                                          sizeof(string) / sizeof(wchar_t))

 
/**
  This is the function implemented by the platform-specific code.

  @return TRUE if user pressed OK, FALSE if cancelled or closed
*/
int ShowOdbcParamsDialog(DataSource *params, HWND ParentWnd, BOOL isPrompt);

#ifdef _WIN32

void getStrFieldData(HWND hwnd, SQLWCHAR **param, int idc);
void getStrFieldDataTab(SQLWCHAR **param, unsigned int framenum, int idc );
void setStrFieldData(HWND hwnd, SQLWCHAR *param, int idc);
void setStrFieldDataTab(SQLWCHAR *param, unsigned int framenum, int idc );

void setComboFieldDataTab(SQLWCHAR *param, unsigned int framenum, int idc);

my_bool getBoolFieldData(HWND hwnd, int idc);
my_bool getBoolFieldDataTab(unsigned int framenum, int idc);
void setBoolFieldDataTab(unsigned int framenum, int idc, my_bool state);
void setBoolFieldData(HWND hwnd, int idc, my_bool state);

void getUnsignedFieldData(HWND hwnd, unsigned int *param, int idc );
void getUnsignedFieldDataTab(unsigned int tab_num, unsigned int *param, int idc );
void setUnsignedFieldData(HWND hwnd, unsigned int param, int idc );
void setUnsignedFieldDataTab(unsigned int framenum, const unsigned int param, int idc);

HWND getTabCtrlTab(void);
HWND getTabCtrlTabPages(unsigned int framenum);
void SwitchTcpOrPipe(HWND hwnd, BOOL usePipe);
void setControlEnabled(unsigned int framenum, int idc, my_bool state);

#define GET_STRING(name) \
  getStrFieldData(hwnd, &(params->name),IDC_EDIT_##name)

#define GET_STRING_TAB(framenum, name) \
  getStrFieldDataTab(&(params->name), framenum, IDC_EDIT_##name)

#define SET_STRING(name) \
  setStrFieldData(hwnd, params->name, IDC_EDIT_##name)

#define SET_STRING_TAB(framenum, name) \
  setStrFieldDataTab(params->name, framenum, IDC_EDIT_##name)

#define GET_COMBO(name) \
  GET_STRING(name)

#define GET_COMBO_TAB(framenum, name) \
  GET_STRING_TAB(framenum, name)

#define SET_COMBO(name) \
  SET_STRING(name)

#define SET_COMBO_TAB(framenum, name) \
  setComboFieldDataTab(params->name, framenum, IDC_EDIT_##name)

#define SET_CSTRING(name) \
    ComboBox_SetText(GetDlgItem(hwnd,IDC_EDIT_##name), params->name)

#define GET_UNSIGNED(name)  getUnsignedFieldData(hwnd, &(params->name), IDC_EDIT_##name)
#define GET_UNSIGNED_TAB(framenum, name)  getUnsignedFieldDataTab(framenum, &(params->name), IDC_EDIT_##name)

#define SET_UNSIGNED(name)  setUnsignedFieldData(hwnd, params->name, IDC_EDIT_##name)
#define SET_UNSIGNED_TAB(framenum, name)  setUnsignedFieldDataTab(framenum, params->name, IDC_EDIT_##name)


/* Definitions for BOOL inputs */
#define READ_BOOL(hwnd, name) \
    getBoolFieldData(hwnd, name)

#define READ_BOOL_TAB(framenum, name) \
    getBoolFieldDataTab(framenum, IDC_CHECK_##name)

#define SET_CHECKED(hwnd, name, state) \
  setBoolFieldData(hwnd, IDC_CHECK_##name, state)

#define SET_CHECKED_TAB(framenum, name, state) \
  setBoolFieldDataTab(framenum, IDC_CHECK_##name, state)

#define SET_RADIO(hwnd, name, state) \
  setBoolFieldData(hwnd, name, state)

#define SET_ENABLED(framenum, idc, state) \
  setControlEnabled(framenum, idc, state)


#else

void setSensitive(gchar *widget_name, gboolean state);
gboolean getBoolFieldData(gchar *widget_name);
void setBoolFieldData(gchar *widget_name, gboolean checked);
void getStrFieldData(gchar *widget_name, SQLWCHAR **param);

/* param8 is the output buffer in UTF8 for GTK */
void setStrFieldData(gchar *widget_name, SQLWCHAR *param, SQLCHAR **param8);

void getUnsignedFieldData(gchar *widget_name, unsigned int *param);
void setUnsignedFieldData(gchar *widget_name, unsigned int param);

#define READ_BOOL(UNUSED_PARAM, name) \
  getBoolFieldData(#name)

#define READ_BOOL_TAB(UNUSED_PARAM, name) \
  READ_BOOL(UNUSED_PARAM, name)

#define SET_CHECKED(UNUSED_PARAM, name, state) \
  setBoolFieldData(#name, state)

#define SET_CHECKED_TAB(UNUSED_PARAM, name, state) \
  SET_CHECKED(UNUSED_PARAM, name, state)

#define GET_STRING(name) \
  getStrFieldData(#name, &(params->name))

#define GET_STRING_TAB(UNUSED_PARAM, name) \
  GET_STRING(name)

#define SET_STRING(name) \
  setStrFieldData(#name, params->name, &(params->name ## 8))

#define SET_STRING_TAB(UNUSED_PARAM, name) \
  SET_STRING(name)

#define SET_COMBO(name) \
  setComboFieldData(#name, params->name, &(params->name ## 8))

#define SET_COMBO_TAB(UNUSED_PARAM, name) \
  SET_COMBO(name)

#define GET_COMBO(name) \
  getComboFieldData(#name, &(params->name))

#define GET_COMBO_TAB(UNUSED_PARAM, name) \
  GET_COMBO(name)
  
#define GET_UNSIGNED(name) \
  getUnsignedFieldData(#name, &(params->name))

#define GET_UNSIGNED_TAB(UNUSED_PARAM, name) \
  GET_UNSIGNED(name)

#define SET_UNSIGNED(name) \
  setUnsignedFieldData(#name, params->name)

#define SET_UNSIGNED_TAB(UNUSED_PARAM, name) \
  SET_UNSIGNED(name)
  
#define SET_SENSITIVE(name, state) \
  setSensitive((gchar*) #name, (gboolean)state)
  
#endif

#define GET_BOOL(hwnd, name) \
    params->name= READ_BOOL(hwnd, name)

#define GET_BOOL_TAB(framenum, name) \
    params->name = READ_BOOL_TAB(framenum, name)

#define SET_BOOL(hwnd, name) \
  SET_CHECKED(hwnd, name, params->name)

#define SET_BOOL_TAB(framenum, name) \
  SET_CHECKED_TAB(framenum, name, params->name)

#ifdef __cplusplus
}
#endif

#endif
