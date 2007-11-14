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
