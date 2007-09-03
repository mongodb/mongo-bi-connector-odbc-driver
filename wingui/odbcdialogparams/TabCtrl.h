/****************************************************************************
 *                                                                          *
 * File    : TabCtrl.h                                                      *
 *                                                                          *
 * Purpose : Tab Control Enhanced.					                        *
 *			 Make Creating and modifying Tab Control Property pages a snap  *
 *			 (c) 2006 David MacDermot										*
 *			 This module is distributed in the hope that it will be useful, *
 *			 but WITHOUT ANY WARRANTY; without even the implied warranty of *
 *			 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.			*
 *                                                                          *
 * History : Date      Reason                                               *
 *           06/22/06  Created                                              *
 *                                                                          *
 ****************************************************************************/

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
