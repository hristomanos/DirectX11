#pragma once
using namespace std;
using namespace DirectX;

#include <iostream>
#include <sstream>

//DBOUT( "The value of x is " << x );
#define DBOUT( s )            \
{                             \
   wostringstream os_;    \
   os_ << s;                   \
   OutputDebugStringW( os_.str().c_str() );  \
}

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMFLOAT4 vOutputColor;
	bool isInstance;
};

struct SCREEN_VERTEX
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
};

struct InstanceData
{
	XMFLOAT3 pos;
};

struct ConstantBufferPerScene
{
	XMMATRIX cubes[];
};

struct PostProcessConstantBuffer
{
	XMFLOAT4 colour;
};

struct _Material
{
	_Material()
		: Emissive(0.0f, 0.0f, 0.0f, 1.0f), Ambient(0.1f, 0.1f, 0.1f, 1.0f), Diffuse(1.0f, 1.0f, 1.0f, 1.0f), 
		  Specular(1.0f, 1.0f, 1.0f, 1.0f), SpecularPower(128.0f), UseTexture(false), UseBumpMap(false), UseDisplacementMap(false)
	{
	}

	DirectX::XMFLOAT4 Emissive;
	//----------------------------------- (16 byte boundary)
	DirectX::XMFLOAT4 Ambient;
	//----------------------------------- (16 byte boundary)
	DirectX::XMFLOAT4 Diffuse;
	//----------------------------------- (16 byte boundary)
	DirectX::XMFLOAT4 Specular;
	//----------------------------------- (16 byte boundary)
	float SpecularPower;
	// Add some padding complete the 16 byte boundary.
	bool UseTexture;
	// Add some padding to complete the 16 byte boundary.
	int UseBumpMap;
	int UseDisplacementMap;
	//float Padding;
	//----------------------------------- (16 byte boundary)
}; // Total:                                80 bytes (5 * 16)

struct MaterialPropertiesConstantBuffer
{
	_Material Material;
};

enum LightType
{
	DirectionalLight = 0,
	PointLight = 1,
	SpotLight = 2
};

#define MAX_LIGHTS 1

struct Light
{
	Light()
		: Position(0.0f, 0.0f, 0.0f, 1.0f), Direction(0.0f, 0.0f, 1.0f, 0.0f), Color(1.0f, 1.0f, 1.0f, 1.0f), SpotAngle(DirectX::XM_PIDIV2), ConstantAttenuation(1.0f), LinearAttenuation(0.0f), QuadraticAttenuation(0.0f), LightType(DirectionalLight), Enabled(0)
	{
	}

	DirectX::XMFLOAT4 Position;
	//----------------------------------- (16 byte boundary)
	DirectX::XMFLOAT4 Direction;
	//----------------------------------- (16 byte boundary)
	DirectX::XMFLOAT4 Color;
	//----------------------------------- (16 byte boundary)
	float SpotAngle;
	float ConstantAttenuation;
	float LinearAttenuation;
	float QuadraticAttenuation;
	//----------------------------------- (16 byte boundary)
	int LightType;
	int Enabled;
	// Add some padding to make this struct size a multiple of 16 bytes.
	int Padding[2];
	//----------------------------------- (16 byte boundary)
}; // Total:                              80 bytes ( 5 * 16 )

struct LightPropertiesConstantBuffer
{
	LightPropertiesConstantBuffer()
		: EyePosition(0, 0, 0, 1), GlobalAmbient(0.2f, 0.2f, 0.8f, 1.0f)
	{
	}

	DirectX::XMFLOAT4 EyePosition;
	//----------------------------------- (16 byte boundary)
	DirectX::XMFLOAT4 GlobalAmbient;
	//----------------------------------- (16 byte boundary)
	Light Lights[MAX_LIGHTS]; // 80 * 8 bytes
};							  // Total:                                  672 bytes (42 * 16)