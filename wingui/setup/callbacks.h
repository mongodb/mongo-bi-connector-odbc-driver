#pragma once

#ifndef _CALLBACKS_H

#define _CALLBACKS_H

#include <windef.h>
#include "utils.h"

// Not a callback though
		void		cleanUp();

// Callbacks indeed
const	wchar_t *	mytest			(HWND hwnd, DataSource* params);
		BOOL		mytestaccept	(HWND hwnd, DataSource* params);
		/*BOOL		mytestaccept	(HWND hwnd, DataSource* params);*/
const	WCHAR**		mygetdatabases	(HWND hwnd, DataSource* params);
		void		myhelp			(HWND hwnd);


#endif
