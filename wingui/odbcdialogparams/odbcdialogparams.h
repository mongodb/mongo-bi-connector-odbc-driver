#ifndef __ODBCPARAMS_H__
#define __ODBCPARAMS_H__

#ifdef DataSource_EXPORTS
#define _ODBCPARAMDLL __declspec(dllexport)
#else
#define _ODBCPARAMDLL __declspec(dllimport)
#endif

#include "myString.h"
#include "../util/installer.h"

// procedure describes all items shown on the dialog


// four callbacks:
// called when [Help] pressed
typedef void HelpButtonPressedCallbackType(HWND dialog);
// called when [Test] pressed - show any messages to user here
typedef const wchar_t * TestButtonPressedCallbackType(HWND dialog, DataSource* params);
// called when [OK] pressed - show errors here (if any) and return false to prevent dialog close
typedef BOOL AcceptParamsCallbackType(HWND dialog, DataSource* params);
// called when DataBase combobox Drops Down
typedef const WCHAR** DatabaseNamesCallbackType(HWND dialog, DataSource* params);


// exported procedure (the one) - not any more. statically linked.
// returns TRUE if user pressed OK, FALSE - otherwise
int /*_ODBCPARAMDLL*/ ShowOdbcParamsDialog(
    PWCHAR caption,                 /*[in] Dialog caption*/
	DataSource* params,                  /*[inout] params */
	HWND ParentWnd = NULL,                     /* [in] could be NULL */
	HelpButtonPressedCallbackType* hcallback = NULL, /* [in] could be NULL */
	TestButtonPressedCallbackType* tcallback = NULL, /* [in] could be NULL */
	AcceptParamsCallbackType* acallback = NULL, /* [in] could be NULL */
	DatabaseNamesCallbackType* dcallback = NULL) /* [in] could be NULL */;



#endif