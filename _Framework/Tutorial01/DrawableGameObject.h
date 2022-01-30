#pragma once

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxcolors.h>
#include <DirectXCollision.h>
#include "DDSTextureLoader.h"
#include "resource.h"
#include <iostream>
#include "structures.h"

#include "Utilities/Helper.h"

#include <vector>

using namespace DirectX;
using namespace std;

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
	XMFLOAT3 tangent;
	XMFLOAT3 biTangent;
};

class DrawableGameObject
{
public:
	DrawableGameObject();
	~DrawableGameObject() { Release(); };

	HRESULT Init(ID3D11Device *pd3dDevice, ID3D11DeviceContext *pContext);
	HRESULT initMesh(ID3D11Device *pd3dDevice, ID3D11DeviceContext *pContext);

	void Update(float t);
	void Draw(ID3D11DeviceContext *pContext);
	void Release();

	ID3D11Buffer *getVertexBuffer() { return m_pVertexBuffer; }
	ID3D11Buffer *getIndexBuffer() { return m_pIndexBuffer; }
	ID3D11ShaderResourceView **getTextureResourceView() { return &m_pTextureResourceView; }
	XMFLOAT4X4 *getTransform() { return &m_World; }
	ID3D11SamplerState **getTextureSamplerState() { return &m_pSamplerLinear; }
	MaterialPropertiesConstantBuffer getMaterial() { return m_material; }
	void setPosition(XMFLOAT3 position);
	XMFLOAT3 GetPosition() { return m_position; }

	void enableSpin(bool enable) { m_bSpin = enable; }

	ID3D11ShaderResourceView* GetTextureArr(int val) { return g_pTextureArr[val]; }

	void SetDiffuseTexture(ID3D11ShaderResourceView* tex) { g_pTextureArr[0] = tex; }

	ID3D11ShaderResourceView* g_pTextureArr[3]{ nullptr, nullptr,nullptr };

	MaterialPropertiesConstantBuffer GetRedPlasticMaterial() { return m_RedPlasticMaterial; }
	void SetRedPlasticMaterial(MaterialPropertiesConstantBuffer redPlasticMaterial) { m_RedPlasticMaterial = redPlasticMaterial; }

	void UI();

private:
	bool m_bSpin = false;
	bool m_bEnabled = false;

	//Instancing
	ID3D11Buffer* m_CubeInstanceBuffer = nullptr;
	const int m_NumCubes = 32;

	XMFLOAT4X4 m_World;
	ID3D11InputLayout*					m_pVertexLayout = nullptr;
	ID3D11Buffer*						m_pVertexBuffer = nullptr;
	ID3D11Buffer*						m_pIndexBuffer = nullptr;

	//Textures
	ID3D11ShaderResourceView*			m_pTextureResourceView = nullptr;
	ID3D11SamplerState *				m_pSamplerLinear = nullptr;

	ID3D11Buffer* m_pMaterialConstantBuffer = nullptr;
	MaterialPropertiesConstantBuffer    m_material;
	XMFLOAT3							m_position = XMFLOAT3(0,0,0);


	void CalculateModelVectors(SimpleVertex *vertices, int vertexCount);
	void CalculateTangentBinormal3(SimpleVertex v0, SimpleVertex v1, SimpleVertex v2, XMFLOAT3 &normal, XMFLOAT3 &tangent, XMFLOAT3 &binormal);


	ID3D11VertexShader*					m_pVertexShader = nullptr;
    ID3D11PixelShader*					m_pPixelShader = nullptr;

	ID3D11PixelShader*				    m_pPixelShaderSolid = nullptr;
	
	MaterialPropertiesConstantBuffer m_RedPlasticMaterial;


};
