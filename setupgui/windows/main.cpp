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

#include "resource.h"
#include <winsock2.h>
#include <windows.h>
#include <commctrl.h>
#include <new>
#include "MYODBC_MYSQL.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

HINSTANCE ghInstance;
const LPCWSTR className = L"MySQLSetupLib";

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	if ( ul_reason_for_call == DLL_PROCESS_ATTACH )
	{
		my_init();
		InitCommonControls();
		ghInstance = hModule;
		WNDCLASSEX wcx;
		// Get system dialog information.
		wcx.cbSize = sizeof(wcx);
		if (!GetClassInfoEx(NULL, MAKEINTRESOURCE(32770), &wcx))
			return 0;

        //FindResource(hModule, RT_DIALOG );
		// Add our own stuff.
		wcx.hInstance = hModule;
	//    wcx.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDR_ICO_MAIN));
		wcx.lpszClassName = className;
		if (!RegisterClassEx(&wcx) )
			return 0;
	}
    else if ( ul_reason_for_call == DLL_PROCESS_DETACH )
    {
        my_end(0);
        UnregisterClass(className,ghInstance);
    }
	
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif
