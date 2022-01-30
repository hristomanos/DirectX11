#pragma once

#include <d3d11_1.h>
#include <string>
#include <windows.h>
#include <windowsx.h>

#include "DDSTextureLoader.h"
#include "Camera.h"
#include "DirectXColors.h"
#include "DrawableGameObject.h"
#include "Graphics.h"
#include "resource.h"

#include "ImGui\\imgui.h"
#include "ImGui\\imgui_impl_dx11.h"
#include "ImGui\\imgui_impl_win32.h"

#include "structures.h"
#include "Utilities/Helper.h"
#include "DirectInput.h"
#include "UserInterface.h"
#include "OBJLoader.h"
#include "Terrain.h"



#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define DEFAULT_WINDOW_TITLE L"Framework";
#define DEFAULT_WINDOW_CLASS L"FrameworkClass";

#define DEFAULT_WINDOW_WIDTH 1280;
#define DEFAULT_WINDOW_HEIGHT 720;

class Application
{
public:
	Application(wstring title, UINT width, UINT height) : m_windowTitle(title.c_str()), m_windowClass((title + L"Class").c_str()), m_windowWidth(width), m_windowHeight(height) {  };
	~Application() { Release(); };

	HRESULT Init(HINSTANCE hInstance, int nCmdShow);

	void Draw();
	void Update(float deltaTime);
	void Release();
	

private:

	HWND m_hWnd = nullptr;
	HINSTANCE m_hInst = nullptr;
	LPCWSTR m_windowTitle = DEFAULT_WINDOW_CLASS;
	LPCWSTR m_windowClass = DEFAULT_WINDOW_TITLE;
	UINT m_windowWidth = DEFAULT_WINDOW_WIDTH;
	UINT m_windowHeight = DEFAULT_WINDOW_HEIGHT;

	//Initilization functions
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);

	//ImGui
	void InitImGui(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	void RenderUI();

	// Camera
	Camera* m_pCamera = nullptr;

	// Graphics
	Graphics* m_pGraphics = nullptr;

	// GameObject
	DrawableGameObject* m_pGameObject = nullptr;

	DrawableGameObject* m_pGameObject2 = nullptr;

	// DirectInput
	DirectInput* m_pDirectInput = nullptr;

	//User interface
	UserInterface* m_pUserInterface = nullptr;

	//Obj loader
	MeshData m_CubeMesh;


	ConstantBuffer cb1;

	//Terrain
	Terrain* m_pTerrain = nullptr;
	

};
