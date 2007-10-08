#pragma once

#ifndef _CALLBACKS_H

#define _CALLBACKS_H

#include <windef.h>
#include "utils.h"

// Not a callback though
		void		cleanUp();

// Callbacks indeed
const	wchar_t *	mytest			(HWND hwnd, OdbcDialogParams* params);
		BOOL		mytestaccept	(HWND hwnd, OdbcDialogParams* params);
		/*BOOL		mytestaccept	(HWND hwnd, OdbcDialogParams* params);*/
const	WCHAR**		mygetdatabases	(HWND hwnd, OdbcDialogParams* params);
		void		myhelp			(HWND hwnd);


#endif
