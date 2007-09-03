#include "stdafx.h"
#include "resource.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

HINSTANCE ghInstance;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	if ( ul_reason_for_call == DLL_PROCESS_ATTACH )
	{
		InitCommonControls();
		ghInstance = hModule;
		WNDCLASSEX wcx;
		// Get system dialog information.
		wcx.cbSize = sizeof(wcx);
		if (!GetClassInfoEx(NULL, MAKEINTRESOURCE(32770), &wcx))
			return 0;

		// Add our own stuff.
		wcx.hInstance = hModule;
	//    wcx.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDR_ICO_MAIN));
		wcx.lpszClassName = _T("TabCtrlDmoClass");
		if (!RegisterClassEx(&wcx) )
			return 0;
	}
	
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif