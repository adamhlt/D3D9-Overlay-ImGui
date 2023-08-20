#include "Drawing.h"

LPCSTR Drawing::lpWindowName = "D3D9 Overlay ImGui";
ImVec2 Drawing::vWindowSize = { 350, 75 };
ImGuiWindowFlags Drawing::WindowFlags = 0;
bool Drawing::bDraw = true;
UI::WindowItem Drawing::lpSelectedWindow = {nullptr, "", ""};
LPDIRECT3DDEVICE9 Drawing::pD3DDevice = nullptr;

/**
    @brief : function that check if the menu is drawed.
    @retval : true if the function is drawed else false.
**/
bool Drawing::isActive()
{
	return bDraw == true;
}

/**
    @brief : Function that draw the ImGui menu.
**/
void Drawing::Draw()
{
	if (GetAsyncKeyState(VK_INSERT) & 1)
		bDraw = !bDraw;

	if (isActive())
	{
		// Draw the window picker.
		// Should be drawed only when the overlay is build as an EXE.
		if (!UI::IsWindowTargeted())
		{
			std::vector<UI::WindowItem> WindowList;
			UI::GetAllWindow(&WindowList);

			if (WindowList.empty())
				return;

			ImGui::SetNextWindowSize({400, 100}, ImGuiCond_Once);
			ImGui::SetNextWindowBgAlpha(1.0f);

			ImGui::Begin("Overlay Target Chooser", &bDraw, WindowFlags);
			{
				if(ImGui::BeginCombo("##combo", lpSelectedWindow.CurrentWindowTitle))
				{
					for (const auto& item : WindowList)
					{
						const bool is_selected = (strcmp(lpSelectedWindow.CurrentWindowTitle, item.CurrentWindowTitle) == 0);
						if (ImGui::Selectable(item.CurrentWindowTitle, is_selected))
							lpSelectedWindow = item;
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				ImGui::NewLine();
				if(ImGui::Button("Start Overlay"))
				{
					UI::SetTargetWindow(lpSelectedWindow.CurrentWindow);
				}
			}
			ImGui::End();

			return;
		}

		// Basic ImGui menu.
		ImGui::SetNextWindowSize(vWindowSize, ImGuiCond_Once);
		ImGui::SetNextWindowBgAlpha(1.0f);
		ImGui::Begin(lpWindowName, &bDraw, WindowFlags);
		{

			ImGui::Text("Create your own menu.");

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}
		ImGui::End();
	}
}

/**
    @brief : Function that set the D3D9 device and execute user D3D9 rendering.
    @param pCurrentD3DDevice : current D3D9 device.
**/
void Drawing::DXDraw(const LPDIRECT3DDEVICE9 pCurrentD3DDevice)
{
	pD3DDevice = pCurrentD3DDevice;
	DrawFilledRectangle(100, 100, 500, 500, 255, 255, 255);
}

/**
    @brief : Function that draw a filed rectangle.
**/
void Drawing::DrawFilledRectangle(const int x, const int y, const int w, const int h, const unsigned char r, const unsigned char g, const unsigned char b)
{
	const D3DCOLOR rectColor = D3DCOLOR_XRGB(r, g, b);	//No point in using alpha because clear & alpha dont work!
	const D3DRECT BarRect = { x, y, x + w, y + h };

	pD3DDevice->Clear(1, &BarRect, D3DCLEAR_TARGET | D3DCLEAR_TARGET, rectColor, 0, 0);
}