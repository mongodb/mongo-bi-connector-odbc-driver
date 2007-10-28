#ifndef _CALLBACKS_H
#define _CALLBACKS_H

#ifdef _WIN32
#include <windows.h>
#endif

#include "utils.h"


// Not a callback though
		void        cleanUp();

// Callbacks indeed
const	wchar_t *	mytest			(HWND hwnd, DataSource* params);
		BOOL		mytestaccept	(HWND hwnd, DataSource* params);
		/*BOOL		mytestaccept	(HWND hwnd, DataSource* params);*/
const	WCHAR**		mygetdatabases	(HWND hwnd, DataSource* params);
		void		myhelp			(HWND hwnd);


#endif