#pragma once

#include "pch.h"

/**
    Overlay window: creates the transparent window, the D3D9 device and the
    ImGui context, then runs the render loop.
**/
namespace Overlay
{
	// Create the overlay and run the render loop until the overlay is closed
	// (END key, window closed or target window closed). Blocking.
	void Run();

	// Ask the render loop to stop. Can be called from any thread.
	void RequestStop();
}
