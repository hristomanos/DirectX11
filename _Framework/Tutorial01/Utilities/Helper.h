#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include "..\ImGui\imgui.h"
#include "..\ImGui\imgui_impl_win32.h"

class Helper
{
public:
	static LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
	static HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	
};

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
