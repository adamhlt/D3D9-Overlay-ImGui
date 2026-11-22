#include "pch.h"
#include "Overlay.h"
#include "Drawing.h"
#include "Target.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace
{
	constexpr TCHAR WindowTitle[] = _T("D3D9 Overlay ImGui");
	constexpr D3DCOLOR ClearColor = D3DCOLOR_RGBA(0, 0, 0, 0);

	LPDIRECT3D9 d3d = nullptr;
	LPDIRECT3DDEVICE9 device = nullptr;
	D3DPRESENT_PARAMETERS d3dpp = {};
	UINT resizeWidth = 0;
	UINT resizeHeight = 0;
	std::atomic<bool> stopRequested = false;

	/**
	    @brief : Create the D3D9 device.
	    @param window : window of the overlay.
	**/
	bool CreateDevice(const HWND window)
	{
		if ((d3d = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
			return false;

		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.Windowed = TRUE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
		d3dpp.EnableAutoDepthStencil = TRUE;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

		return SUCCEEDED(d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &device));
	}

	/**
	    @brief : Release the D3D9 device and object.
	**/
	void CleanupDevice()
	{
		if (device) { device->Release(); device = nullptr; }
		if (d3d) { d3d->Release(); d3d = nullptr; }
	}

	/**
	    @brief : Reset the current D3D9 device.
	**/
	void ResetDevice()
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
		const HRESULT hr = device->Reset(&d3dpp);
		if (hr == D3DERR_INVALIDCALL)
			IM_ASSERT(0);
		ImGui_ImplDX9_CreateDeviceObjects();
	}

	/**
	    @brief : Window message handler (https://learn.microsoft.com/en-us/windows/win32/api/winuser/nc-winuser-wndproc).
	**/
	LRESULT WINAPI WndProc(const HWND window, const UINT msg, const WPARAM wParam, const LPARAM lParam)
	{
		if (ImGui_ImplWin32_WndProcHandler(window, msg, wParam, lParam))
			return true;

		switch (msg)
		{
		case WM_SIZE:
			if (wParam == SIZE_MINIMIZED)
				return 0;
			// Queue the resize, it is handled in the render loop.
			resizeWidth = static_cast<UINT>(LOWORD(lParam));
			resizeHeight = static_cast<UINT>(HIWORD(lParam));
			return 0;

		case WM_SYSCOMMAND:
			if ((wParam & 0xfff0) == SC_KEYMENU)
				return 0;
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		default:
			break;
		}

		return DefWindowProc(window, msg, wParam, lParam);
	}

	/**
	    @brief : Make the overlay window transparent to the inputs or not.
	**/
	void SetClickThrough(const HWND window, const bool clickThrough)
	{
		static int current = -1;
		if (current == static_cast<int>(clickThrough))
			return;

		current = clickThrough;
		DWORD style = WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW;
		if (clickThrough)
			style |= WS_EX_TRANSPARENT;

		SetWindowLong(window, GWL_EXSTYLE, style);
	}

	/**
	    @brief : Create the transparent overlay window.
	**/
	HWND CreateOverlayWindow(const WNDCLASSEX& wc)
	{
		RegisterClassEx(&wc);

		const HWND window = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE, wc.lpszClassName, WindowTitle, WS_POPUP,
			0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), nullptr, nullptr, wc.hInstance, nullptr);

		if (window == nullptr)
			return nullptr;

		SetLayeredWindowAttributes(window, 0, 255, LWA_ALPHA);
		const MARGINS margin = { -1, 0, 0, 0 };
		DwmExtendFrameIntoClientArea(window, &margin);

		return window;
	}

	/**
	    @brief : Create the ImGui context and initialize the backends.
	**/
	void InitImGui(const HWND window)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.IniFilename = nullptr;

		ImGui::StyleColorsDark();

		// Scale the font size depending of the screen size.
		const HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
		MONITORINFO info = {};
		info.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(monitor, &info);

		if (info.rcMonitor.bottom - info.rcMonitor.top > 1080)
		{
			ImFontConfig config;
			config.SizePixels = 13 * 2.0f;
			io.Fonts->AddFontDefault(&config);
		}

		ImGui_ImplWin32_Init(window);
		ImGui_ImplDX9_Init(device);
	}

	void ShutdownImGui()
	{
		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	/**
	    @brief : Present the back buffer and handle the loss of the D3D9 device.
	**/
	void Present()
	{
		const HRESULT result = device->Present(nullptr, nullptr, nullptr, nullptr);

		if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
			ResetDevice();
	}

	void ClearBackBuffer()
	{
		device->SetRenderState(D3DRS_ZENABLE, FALSE);
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, ClearColor, 1.0f, 0);
	}

	/**
	    @brief : Draw one frame: the ImGui menu and the user D3D9 rendering.
	**/
	void RenderFrame(const HWND window)
	{
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		Drawing::Draw();

		// The overlay handles the inputs only when the menu is showed.
		SetClickThrough(window, !Drawing::IsVisible());

		ImGui::Render();

		ClearBackBuffer();

		if (Target::IsSet())
			Drawing::DXDraw(device);

		if (device->BeginScene() >= 0)
		{
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			device->EndScene();
		}

		Present();
	}

	/**
	    @brief : Handle the window messages.
	    @retval : false when the overlay has to be closed.
	**/
	bool PumpMessages()
	{
		bool running = true;

		MSG msg;
		while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				running = false;
		}

		return running;
	}
}

/**
    @brief : Create the overlay window and run the render loop.
**/
void Overlay::Run()
{
	stopRequested = false;

	ImGui_ImplWin32_EnableDpiAwareness();

	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
	wc.lpszClassName = WindowTitle;

	const HWND window = CreateOverlayWindow(wc);
	if (window == nullptr || !CreateDevice(window))
	{
		CleanupDevice();
		if (window != nullptr)
			DestroyWindow(window);
		UnregisterClass(wc.lpszClassName, wc.hInstance);
		DeleteObject(wc.hbrBackground);
		return;
	}

	ShowWindow(window, SW_SHOWDEFAULT);
	UpdateWindow(window);

	InitImGui(window);

	while (!stopRequested)
	{
		if (!PumpMessages())
			break;

		if (GetAsyncKeyState(VK_END) & 1)
			break;

		// Stop when the target window has been closed.
		if (Target::IsSet() && !Target::IsAlive())
			break;

		if (resizeWidth != 0 && resizeHeight != 0)
		{
			d3dpp.BackBufferWidth = resizeWidth;
			d3dpp.BackBufferHeight = resizeHeight;
			resizeWidth = resizeHeight = 0;
			ResetDevice();
		}

#ifdef _WINDLL
		// The target is the main window of the process, wait until it exists.
		if (!Target::IsSet() && !Target::FindCurrentProcessWindow())
		{
			Sleep(100);
			continue;
		}
#endif

		// Move the window on top of the target window and handle resize.
		if (Target::IsSet())
			Target::MoveOverlay(window);

		// Clear the overlay when the target window is not focused.
		if (Target::IsSet() && !Target::IsFocused(window))
		{
			SetClickThrough(window, true);
			ClearBackBuffer();

			if (device->BeginScene() >= 0)
				device->EndScene();

			Present();
			continue;
		}

		RenderFrame(window);
	}

	ShutdownImGui();
	CleanupDevice();
	DestroyWindow(window);
	UnregisterClass(wc.lpszClassName, wc.hInstance);
	DeleteObject(wc.hbrBackground);
}

void Overlay::RequestStop()
{
	stopRequested = true;
}
