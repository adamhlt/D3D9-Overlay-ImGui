#include "pch.h"
#include "Overlay.h"

#ifdef _WINDLL

namespace
{
	HMODULE currentModule = nullptr;
	HANDLE overlayThread = nullptr;
	DWORD overlayThreadId = 0;

	DWORD WINAPI OverlayThread(LPVOID)
	{
		Overlay::Run();

		// The overlay is closed, unload the DLL.
		FreeLibraryAndExitThread(currentModule, 0);
	}
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hinstDLL);
		currentModule = hinstDLL;
		overlayThread = CreateThread(nullptr, 0, OverlayThread, nullptr, 0, &overlayThreadId);
	}
	else if (fdwReason == DLL_PROCESS_DETACH && overlayThread != nullptr)
	{
		// lpReserved is not null when the process is terminating, the thread is already gone.
		if (lpReserved == nullptr && GetCurrentThreadId() != overlayThreadId)
		{
			// The DLL is unloaded by someone else: stop the overlay and give it time to clean up.
			Overlay::RequestStop();
			WaitForSingleObject(overlayThread, 1000);
		}

		CloseHandle(overlayThread);
		overlayThread = nullptr;
	}

	return TRUE;
}

#else

int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
	Overlay::Run();

	return 0;
}

#endif
