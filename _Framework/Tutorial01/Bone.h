#pragma once

#include <directxcolors.h>
#include "structures.h"

class Bone
{
	Bone();
	~Bone() { Release(); }

public:
	void Init();
	void Release();
private:

	const char* m_Name;
	XMFLOAT4X4 m_Tranform;
	XMFLOAT4X4 m_Orientation;
	
	XMFLOAT3 m_Translation;
	XMFLOAT3 m_Scale;
	XMFLOAT4X4 m_Offset;
};

