/*
  Copyright (c) 2007 MySQL AB, 2010 Sun Microsystems, Inc.
  Use is subject to license terms.

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
 @file TabCtrl.h
 @brief Tab Control Enhanced.

 Make Creating and modifying Tab Control Property pages a snap.

 (c) 2006 David MacDermot

 This module is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/** Structs *****************************************************************/

typedef struct TabControl
{
	HWND hTab;
	HWND* hTabPages;
	PWSTR *tabNames;
	PWSTR *dlgNames;
	int tabPageCount;
	BOOL blStretchTabs;

	// Function pointer to Parent Dialog Proc
	BOOL (*ParentProc)(HWND, UINT, WPARAM, LPARAM);

	// Pointers to shared functions
	BOOL (*OnSelChanged)(VOID);
	BOOL (*OnKeyDown)(LPARAM);
	BOOL (*StretchTabPage) (HWND);
	BOOL (*CenterTabPage) (HWND);

} TABCTRL, *LPTABCTRL;

/** Prototypes **************************************************************/


//typedef BOOL CALLBACK (ParentProcType)(HWND, UINT, WPARAM, LPARAM);

void New_TabControl(LPTABCTRL,
					HWND,
					PWSTR*,
					PWSTR*,
					BOOL (*ParentProc)(HWND, UINT, WPARAM, LPARAM),
					VOID (*TabPage_OnSize)(HWND, UINT, int, int),
					BOOL fStretch);

void TabControl_Select(LPTABCTRL);
void TabControl_Destroy(LPTABCTRL);
