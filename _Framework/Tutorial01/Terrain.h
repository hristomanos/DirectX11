#pragma once

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxcolors.h>
#include "structures.h"
#include "Utilities/Helper.h"
#include "DDSTextureLoader.h"

#include <vector>
#include <fstream>

#include <random>

struct SimpleTerrainVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
};

inline static float random(int range)
{
	return (rand() % (range * 2) - range);
}

class Terrain
{
public:
	Terrain();
	~Terrain() { Release(); }

	//Compileshaders
	HRESULT Init(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, int rows, int columns);
	void Update(float deltaTime);
	void Draw(ID3D11DeviceContext* pContext);
	void Release();


	XMFLOAT4X4* GetTransform() { return &m_WorldMatrix; }

	XMFLOAT3 GetPosition() { return m_position; }
	void SetPosition(XMFLOAT3 position) { m_position = position; }

	float GetHeight(float x,float z);

	void UI();


private:

	bool m_IsEnabled = true;

	//World matrix
	XMFLOAT4X4 m_WorldMatrix;

	//Vertices
	ID3D11Buffer* m_pVertexBuffer = nullptr;

	//Indices
	ID3D11Buffer* m_pIndexBuffer = nullptr;

	//Input layout
	ID3D11InputLayout* m_pInputLayout = nullptr;

	//Shaders
	ID3D11VertexShader* m_pVertexShader = nullptr;
	ID3D11PixelShader* m_pPixelShader = nullptr;

	HRESULT CreateVertexShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	HRESULT CreateInputLayout(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,ID3DBlob*& pVertexShaderBlob);
	HRESULT CreatePixelShader(ID3D11Device* pDevice);

	HRESULT GenerateMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,int rows, int columns);
	HRESULT GenerateIndices(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,int rows, int columns);

	//Grid
	int m_TotalIndices = 0;

	//Textures
	ID3D11ShaderResourceView* m_pTextures[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	ID3D11SamplerState* m_pSamplerLinear = nullptr;

	XMFLOAT3			m_position = XMFLOAT3(0, -1.1f, 0);


	void LoadHeightmap(int width,int height,int scale);
	vector<float> m_Heightmap;

	float m_TotalWidth = 0;
	float m_TotalDepth = 0;
	float m_Dx = 0;
	float m_Dz = 0;
	int m_HeightmapWidth = 0;
	int m_HeightmapHeight = 0;
	int m_HeightScale = 0;
	int m_TerrainWidth = 0;
	int m_TerrainHeight = 0;
	int m_TotalCellRows = 0;
	int m_TotalCellColumns = 0;
	int m_TotalVertices = 0;
	int m_TotalTriangles = 0;
	float m_Du = 0;
	float m_Dv = 0;

	float m_XOffset = 0;
	float m_ZOffset = 0;

	const static int WIDTH_X = 513;
	const static int WIDTH_Z = 513;

	int m_GeneratedHeightMap[WIDTH_X][WIDTH_Z];
	void GenerateDiamondSquare();
	void DiamondSquare(int Array[WIDTH_X][WIDTH_Z], int size);
	void SquareStep(int Array[WIDTH_X][WIDTH_Z], int x, int z, int reach);
	void DiamondStep(int Array[WIDTH_X][WIDTH_Z], int x, int z, int reach);


	void SmoothHeights(int strength);

};

