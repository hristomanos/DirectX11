#pragma once
#pragma comment (lib,"dinput8.lib")
#pragma comment (lib,"dxguid.lib")
#include <dinput.h>
#include"Camera.h"
#include "Terrain.h"

class DirectInput
{
public:
	DirectInput();
	~DirectInput();


	bool InitDirectInput(HINSTANCE hInstance,HWND hWnd);
	void DetectInput(double time,Camera* camera, HWND hWnd,Terrain* terrain);

	bool is_left_Button_Pressed(DIMOUSESTATE mouseCurrState);

private:
	bool m_bIsMouseAcquired = false;

	//COM interface. Represent a physical object to get our input.
	IDirectInputDevice8* m_pDIKeyboard;
	IDirectInputDevice8* m_pDIMouse;

	//Holds the last state of the mouse
	DIMOUSESTATE m_MouseLastState;

	//Structure that gets input from keyboard
	LPDIRECTINPUT8 m_DirectInputObject;
};



