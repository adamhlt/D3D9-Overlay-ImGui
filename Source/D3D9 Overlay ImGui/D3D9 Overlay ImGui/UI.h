#ifndef UI_H
#define UI_H

#include "pch.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class UI
{
private:
	

public:
	static HMODULE hCurrentModule;

	static void Render();

};

#endif