#include "Camera.h"



Camera::Camera(UINT WindowWidth,UINT WindowHeight)
{
	m_CamPosition = XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f);
	m_CamTarget = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	m_CamUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	//Set the View matrix
	m_CamView = XMMatrixLookAtLH(m_CamPosition, m_CamTarget, m_CamUp);

	//Set the Projection matrix
	m_CamProjection = XMMatrixPerspectiveFovLH(0.4f * 3.14f, (float)WindowWidth / WindowHeight, 0.01f, 1000.0f);
}


Camera::~Camera()
{
}


void Camera::MoveForward(float speed)
{
	m_MoveBackForward += speed;
}

void Camera::MoveBackward(float speed)
{
	m_MoveBackForward -= speed;
}

void Camera::StrafeRight(float speed)
{
	m_MoveLeftRight += speed;
}

void Camera::StrafeLeft(float speed)
{
	m_MoveLeftRight -= speed;
}

void Camera::RotateLeftRight(float value)
{
	m_CamYaw += value;
}

void Camera::RotateUpDown(float value)
{
	m_CamPitch += value;
}

void Camera::UpdateCamera()
{
	m_CamRotationMatrix = XMMatrixRotationRollPitchYaw(m_CamPitch, m_CamYaw, 0);
	m_CamTarget = XMVector3TransformCoord(m_DefaultForward, m_CamRotationMatrix);
	m_CamTarget = XMVector3Normalize(m_CamTarget);

	XMMATRIX RotateYTemptMatrix;
	RotateYTemptMatrix = XMMatrixRotationY(m_CamYaw);

	m_CamRight = XMVector3TransformCoord(m_DefaultRight, m_CamRotationMatrix);
	m_CamForward = XMVector3TransformCoord(m_DefaultForward, m_CamRotationMatrix);
	m_CamUp = XMVector3Cross(m_CamForward, m_CamRight);

	m_CamPosition += m_MoveLeftRight * m_CamRight;
	m_CamPosition += m_MoveBackForward * m_CamForward;

	m_MoveLeftRight = 0.0f;
	m_MoveBackForward = 0.0f;
	XMFLOAT3 camPos;
	XMStoreFloat3(&camPos, m_CamPosition);

	m_CamTarget = m_CamPosition + m_CamTarget;

	m_CamView = XMMatrixLookAtLH(m_CamPosition, m_CamTarget, m_CamUp);

}

