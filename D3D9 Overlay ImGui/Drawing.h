#pragma once

#include "pch.h"

/**
    Your drawing code: the ImGui menu (Draw) and the raw D3D9 rendering (DXDraw).
**/
namespace Drawing
{
	// Returns true if the menu is showed (toggled with the INSERT key).
	bool IsVisible();

	// Draw the ImGui menu. Called every frame between NewFrame and Render.
	void Draw();

	// Draw with D3D9 directly. Called every frame before the ImGui rendering.
	void DXDraw(LPDIRECT3DDEVICE9 device);

	// Example of D3D9 drawing: fill a rectangle with a solid color.
	void DrawFilledRectangle(LPDIRECT3DDEVICE9 device, int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b);
}
