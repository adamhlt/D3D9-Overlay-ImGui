#ifndef UI_H
#define UI_H

#include "pch.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class UI
{
private:
	static LPDIRECT3D9 pD3D;
	static LPDIRECT3DDEVICE9 pD3DDevice;
	static D3DPRESENT_PARAMETERS D3Dpp;
	static bool bInit;
	static UINT g_ResizeWidth, g_ResizeHeight;

	static bool CreateDeviceD3D(HWND hWnd);
	static void CleanupDeviceD3D();
	static void ResetDevice();
	static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
	static HMODULE hCurrentModule;

	static void Render();

};

#endif