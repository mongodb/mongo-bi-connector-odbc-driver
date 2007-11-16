/*
  Copyright (C) 2007 MySQL AB

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  There are special exceptions to the terms and conditions of the GPL
  as it is applied to this software. View the full text of the exception
  in file LICENSE.exceptions in the top-level directory of this software
  distribution.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
