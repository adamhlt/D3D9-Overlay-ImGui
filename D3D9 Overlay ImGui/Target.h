#pragma once

#include "pch.h"

/**
    Target window handling: pick the window the overlay is drawn on top of,
    follow it and detect when it is closed.
**/
namespace Target
{
	struct WindowItem
	{
		HWND window = nullptr;
		std::string title;
		std::string processName;
	};

	// Define the target window (used by the window picker in EXE builds).
	// Returns false if the window is not valid.
	bool Set(HWND window);

	// Returns true if a target window has been defined.
	bool IsSet();

	// Returns true if the target window still exists and belongs to the same process.
	bool IsAlive();

	// Use the main window of the current process as the target (DLL builds).
	bool FindCurrentProcessWindow();

	// Enumerate all the windows that can be targeted.
	std::vector<WindowItem> GetAllWindows();

	// Returns true if the overlay or the target window (or its process) has the focus.
	bool IsFocused(HWND overlayWindow);

	// Move and resize the overlay on top of the target window.
	void MoveOverlay(HWND overlayWindow);
}
