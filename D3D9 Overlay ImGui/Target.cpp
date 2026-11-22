#include "pch.h"
#include "Target.h"

namespace
{
	HWND targetWindow = nullptr;
	DWORD targetPID = 0;
	bool targetSet = false;

	std::string ToUtf8(const wchar_t* text)
	{
		const int size = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
		if (size <= 1)
			return {};

		std::string result(static_cast<size_t>(size) - 1, '\0');
		WideCharToMultiByte(CP_UTF8, 0, text, -1, result.data(), size, nullptr, nullptr);
		return result;
	}

	bool IsWindowCloaked(const HWND window)
	{
		DWORD cloaked = 0;
		const HRESULT hr = DwmGetWindowAttribute(window, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
		return SUCCEEDED(hr) && cloaked;
	}

	// A window is valid when it is a visible top level window with a client area.
	bool IsWindowValid(const HWND window)
	{
		if (!IsWindowVisible(window) || IsIconic(window) || IsWindowCloaked(window))
			return false;

		RECT rect;
		GetClientRect(window, &rect);

		const auto styles = static_cast<DWORD>(GetWindowLongPtr(window, GWL_STYLE));
		const auto exStyles = static_cast<DWORD>(GetWindowLongPtr(window, GWL_EXSTYLE));

		if (exStyles & WS_EX_TOOLWINDOW)
			return false;
		if (styles & WS_CHILD)
			return false;
		if (rect.bottom == 0 || rect.right == 0)
			return false;

		return true;
	}

	// Map every running process ID to its executable name with a single snapshot.
	std::unordered_map<DWORD, std::string> GetProcessNames()
	{
		std::unordered_map<DWORD, std::string> names;

		const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (snapshot == INVALID_HANDLE_VALUE)
			return names;

		PROCESSENTRY32W entry{};
		entry.dwSize = sizeof(entry);

		if (Process32FirstW(snapshot, &entry))
		{
			do
			{
				names[entry.th32ProcessID] = ToUtf8(entry.szExeFile);
			} while (Process32NextW(snapshot, &entry));
		}

		CloseHandle(snapshot);
		return names;
	}

	BOOL CALLBACK EnumCurrentProcessWindow(const HWND window, const LPARAM result)
	{
		DWORD pid = 0;
		GetWindowThreadProcessId(window, &pid);
		if (pid != GetCurrentProcessId() || !IsWindowValid(window))
			return TRUE;

		*reinterpret_cast<HWND*>(result) = window;
		return FALSE;
	}

	BOOL CALLBACK EnumAllWindows(const HWND window, const LPARAM result)
	{
		if (!IsWindowValid(window))
			return TRUE;

		wchar_t title[256];
		if (GetWindowTextW(window, title, static_cast<int>(std::size(title))) == 0)
			return TRUE;

		auto* list = reinterpret_cast<std::vector<Target::WindowItem>*>(result);
		list->push_back({ window, ToUtf8(title), {} });
		return TRUE;
	}
}

/**
    @brief : Define the target window.
    @param window : target window.
    @retval : true if the window is valid and has been set else false.
**/
bool Target::Set(const HWND window)
{
	if (window == nullptr || !IsWindow(window))
		return false;

	targetWindow = window;
	GetWindowThreadProcessId(window, &targetPID);
	targetSet = true;

	SetForegroundWindow(window);
	return true;
}

bool Target::IsSet()
{
	return targetSet;
}

/**
    @brief : Check if the target window has been closed.
    @retval : true if the target window is still up else false.
**/
bool Target::IsAlive()
{
	if (!targetSet || !IsWindow(targetWindow))
		return false;

	DWORD currentPID = 0;
	GetWindowThreadProcessId(targetWindow, &currentPID);

	return currentPID == targetPID;
}

/**
    @brief : Retrieve the main window of the current process and use it as target.
             Only used when the overlay is built as a DLL.
**/
bool Target::FindCurrentProcessWindow()
{
	HWND window = nullptr;
	EnumWindows(EnumCurrentProcessWindow, reinterpret_cast<LPARAM>(&window));

	return window != nullptr && Set(window);
}

/**
    @brief : Enumerate all the valid windows with their title and process name.
**/
std::vector<Target::WindowItem> Target::GetAllWindows()
{
	std::vector<WindowItem> windows;
	EnumWindows(EnumAllWindows, reinterpret_cast<LPARAM>(&windows));

	const auto processNames = GetProcessNames();
	for (auto& item : windows)
	{
		DWORD pid = 0;
		GetWindowThreadProcessId(item.window, &pid);

		if (const auto it = processNames.find(pid); it != processNames.end())
			item.processName = it->second;
	}

	return windows;
}

/**
    @brief : Check if the overlay or the target process is the foreground window.
    @param overlayWindow : window of the overlay.
**/
bool Target::IsFocused(const HWND overlayWindow)
{
	const HWND foreground = GetForegroundWindow();
	if (foreground == nullptr)
		return false;

	if (foreground == overlayWindow || foreground == targetWindow)
		return true;

	DWORD foregroundPID = 0;
	GetWindowThreadProcessId(foreground, &foregroundPID);

	return targetSet && foregroundPID == targetPID;
}

/**
    @brief : Move the overlay on top of the target window.
    @param overlayWindow : window of the overlay.
**/
void Target::MoveOverlay(const HWND overlayWindow)
{
	if (!targetSet)
		return;

	RECT rect;
	if (!GetWindowRect(targetWindow, &rect))
		return;

	// Remove the window borders and the title bar.
	constexpr int borderWidth = 5;
	constexpr int titleBarHeight = 29;

	SetWindowPos(overlayWindow, nullptr, rect.left, rect.top,
		rect.right - rect.left - borderWidth, rect.bottom - rect.top - titleBarHeight, SWP_SHOWWINDOW);
}
