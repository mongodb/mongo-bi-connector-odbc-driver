#ifndef __ODBCPARAMS_H__
#define __ODBCPARAMS_H__

#include "installer.h"

#ifdef __cplusplus
extern "C" {
#endif

// four callbacks:
// called when [Help] pressed
typedef void HelpButtonPressedCallbackType(HWND dialog);
// called when [Test] pressed - show any messages to user here
//typedef const wchar_t * TestButtonPressedCallbackType(HWND dialog, DataSource* params);
// called when [OK] pressed - show errors here (if any) and return false to prevent dialog close
typedef BOOL AcceptParamsCallbackType(HWND dialog, DataSource* params);
// called when DataBase combobox Drops Down
//typedef const WCHAR** DatabaseNamesCallbackType(HWND dialog, DataSource* params);


// exported procedure (the one) - not any more. statically linked.
// returns TRUE if user pressed OK, FALSE - otherwise
int ShowOdbcParamsDialog(
	DataSource* params,                  /*[inout] params */
	HWND ParentWnd,                     /* [in] could be NULL */
	BOOL isPrompt);

#ifdef __cplusplus
}
#endif

#endif

