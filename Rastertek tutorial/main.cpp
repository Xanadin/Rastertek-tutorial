#include "systemclass.h"
#include <crtdbg.h>

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR pszCmdLine, int nCmdShow)
{
	// Enable run-time memory check for debug builds.
	#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif
	SystemClass* System;
	bool result;

	System = new SystemClass;
	if (!System)
	{
		return 0;
	}
	result = System->Initialize(TEXT("Tutorial 2"));
	if (result)
	{
		System->Run();
	}
	System->Shutdown();
	SafeDelete(System);
	return 0;
}