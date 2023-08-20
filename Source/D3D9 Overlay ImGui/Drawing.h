#ifndef DRAWING_H
#define DRAWING_H

#include "pch.h"
#include "UI.h"

class Drawing
{
private:
	static LPCSTR lpWindowName;
	static ImVec2 vWindowSize;
	static ImGuiWindowFlags WindowFlags;
	static bool bDraw;
	static UI::WindowItem lpSelectedWindow;
	static LPDIRECT3DDEVICE9 pD3DDevice;

	static void DrawFilledRectangle(int x, int y, int w, int h, unsigned char r, unsigned char g, unsigned char b);

public:
	static bool isActive();
	static void Draw();
	static void DXDraw(LPDIRECT3DDEVICE9 pCurrentD3DDevice);
};

#endif