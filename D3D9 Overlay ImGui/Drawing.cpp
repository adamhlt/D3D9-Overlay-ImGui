#include "pch.h"
#include "Drawing.h"
#include "Target.h"

namespace
{
	constexpr const char* MenuName = "D3D9 Overlay ImGui";
	constexpr ULONGLONG WindowListRefreshMs = 500;

	bool visible = true;
	Target::WindowItem selectedWindow;
	std::vector<Target::WindowItem> windowList;
	ULONGLONG lastWindowListRefresh = 0;

	/**
	    @brief : Draw the window picker.
	             Only shown when the overlay is built as an EXE and no target has been chosen.
	**/
	void DrawWindowPicker()
	{
		// Refreshing the list is expensive, do it a few times per second only.
		const ULONGLONG now = GetTickCount64();
		if (windowList.empty() || now - lastWindowListRefresh > WindowListRefreshMs)
		{
			windowList = Target::GetAllWindows();
			lastWindowListRefresh = now;
		}

		if (windowList.empty())
			return;

		ImGui::SetNextWindowSize({ 400, 100 }, ImGuiCond_Once);
		ImGui::SetNextWindowBgAlpha(1.0f);

		ImGui::Begin("Overlay Target Chooser", &visible);
		{
			if (ImGui::BeginCombo("##combo", selectedWindow.title.c_str()))
			{
				for (const auto& item : windowList)
				{
					const bool isSelected = selectedWindow.window == item.window;
					if (ImGui::Selectable(item.title.c_str(), isSelected))
						selectedWindow = item;
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::NewLine();

			if (ImGui::Button("Start Overlay"))
				Target::Set(selectedWindow.window);
		}
		ImGui::End();
	}
}

bool Drawing::IsVisible()
{
	return visible;
}

/**
    @brief : Draw the ImGui menu.
**/
void Drawing::Draw()
{
	if (GetAsyncKeyState(VK_INSERT) & 1)
		visible = !visible;

	if (!visible)
		return;

	if (!Target::IsSet())
	{
		DrawWindowPicker();
		return;
	}

	// Basic ImGui menu.
	ImGui::SetNextWindowSize({ 350, 75 }, ImGuiCond_Once);
	ImGui::SetNextWindowBgAlpha(1.0f);

	ImGui::Begin(MenuName, &visible);
	{
		ImGui::Text("Create your own menu.");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}
	ImGui::End();
}

/**
    @brief : Execute your own D3D9 rendering.
    @param device : current D3D9 device.
**/
void Drawing::DXDraw(const LPDIRECT3DDEVICE9 device)
{
	// Example: DrawFilledRectangle(device, 100, 100, 500, 500, 255, 255, 255);
	(void)device;
}

/**
    @brief : Draw a filled rectangle.
**/
void Drawing::DrawFilledRectangle(const LPDIRECT3DDEVICE9 device, const int x, const int y, const int w, const int h, const unsigned char r, const unsigned char g, const unsigned char b)
{
	const D3DCOLOR color = D3DCOLOR_XRGB(r, g, b); // No point in using alpha because clear & alpha don't work!
	const D3DRECT rect = { x, y, x + w, y + h };

	device->Clear(1, &rect, D3DCLEAR_TARGET, color, 0, 0);
}
