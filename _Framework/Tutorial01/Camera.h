#pragma once

#include <DirectXMath.h>
#include "structures.h"
#include <windows.h>

class Camera
{
public:
	Camera(UINT WindowWidth, UINT WindowHeight);
	~Camera();

	void MoveForward(float speed);
	void MoveBackward(float speed);
	void StrafeRight(float speed);
	void StrafeLeft(float speed);

	void RotateLeftRight(float value);
	void RotateUpDown(float value);

	void UpdateCamera();

	XMMATRIX GetProjectionMatrix() { return m_CamProjection; }
	XMMATRIX GetViewMatrix() { return m_CamView; }

	XMVECTOR GetCamPosition() { return m_CamPosition; }
	void SetCamPosition(float x, float y, float z) { m_CamPosition = XMVectorSet(x, y, z, 0.0f); }

	bool m_WalkMode = false;
	float m_WalkingHeight = 10.0f;

	float m_MovementSpeed = 65.0f;

private:


	float m_MoveBackForward = 0.0f;
	float m_MoveLeftRight = 0.0f;

	//Rotation left/right. Y axis
	float m_CamYaw = 0.0f;

	//Rotation Up/Down. X axis.
	float m_CamPitch = 0.0f;

	XMVECTOR m_CamPosition = XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f);
	XMVECTOR m_CamTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR m_CamUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);


	XMMATRIX m_CamView;
	XMMATRIX m_CamProjection;

	XMVECTOR m_DefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR m_DefaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR m_CamForward = XMVectorSet(0.0f,0.0f,1.0f,0.0f);
	XMVECTOR m_CamRight = XMVectorSet(1.0f,0.0f,0.0f,0.0f);

	XMMATRIX m_CamRotationMatrix;

};

