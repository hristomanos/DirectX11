#include "Terrain.h"

//Responsible for generating a terrain mesh

Terrain::Terrain()
{
	XMMATRIX world = XMMatrixIdentity();
	XMStoreFloat4x4(&m_WorldMatrix, world);

}

void Terrain::Release()
{
	if (m_pInputLayout) { m_pInputLayout->Release(); }
	if (m_pVertexBuffer){ m_pVertexBuffer->Release();}
	if (m_pIndexBuffer) { m_pIndexBuffer->Release(); }
	if (m_pVertexShader) { m_pVertexShader->Release();}
	if (m_pPixelShader) { m_pPixelShader->Release(); }
	for (ID3D11ShaderResourceView*& texture : m_pTextures)
		texture->Release();
	if (m_pSamplerLinear) {	m_pSamplerLinear->Release(); }
}


HRESULT Terrain::Init(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, int rows, int columns)
{
	HRESULT hr;

	// - Vertex shader
	hr =  CreateVertexShader(pDevice, pContext);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Creating terrain vertex shader - FAILED", L"Error", MB_OK);
		return hr;
	}
	// - Pixel shader
	hr = CreatePixelShader(pDevice);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Creating terrain pixel shader - FAILED", L"Error", MB_OK);
		return hr;
	}

	// CreateVerticies
	hr = GenerateMesh(pDevice, pContext, rows, columns);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Creating terrain mesh - FAILED", L"Error", MB_OK);
		return hr;
	}


	return hr;
}


HRESULT Terrain::CreateVertexShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	ID3DBlob* pVSBlob = nullptr;
	HRESULT hr = Helper::CompileShaderFromFile(L"TerrainShader.fx", "TerrainVS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Compiling terrain shader - FAILED", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	hr = CreateInputLayout(pDevice, pContext, pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"CreateInputLayout() (terrain) - FAILED", L"Error", MB_OK);
		return hr;
	}

	return hr;
}

HRESULT Terrain::CreateInputLayout(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,ID3DBlob*& pVertexShaderBlob)
{
	HRESULT hr;

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = pDevice->CreateInputLayout(layout, numElements, pVertexShaderBlob->GetBufferPointer(),
		pVertexShaderBlob->GetBufferSize(), &m_pInputLayout);

	pVertexShaderBlob->Release();
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Creating terrain input layout - FAILED", L"Error", MB_OK);
		return hr;
	}

	// Set the input layout
	pContext->IASetInputLayout(m_pInputLayout);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Setting terrain input layout - FAILED", L"Error", MB_OK);
		return hr;
	}

	return hr;
}

HRESULT Terrain::CreatePixelShader(ID3D11Device* pDevice)
{
	HRESULT hr;

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	hr = Helper::CompileShaderFromFile(L"TerrainShader.fx", "TerrainPS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Compiling terrain pixel shader - FAILED", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPixelShader);
	
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

	return hr;
}

HRESULT Terrain::GenerateMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,int rows,int columns)
{
	HRESULT hr;

	m_TerrainWidth = rows;
	m_TerrainHeight = columns;

	m_TotalCellRows = rows - 1;
	m_TotalCellColumns = columns - 1;

	//Width of a cell and total width
	 m_Dx = 1.0f; //X Terrain cell size
	m_TotalWidth = m_TotalCellColumns * m_Dx;

	//Depth of a cell and total depth
	m_Dz = 1.0f; //Z terrain cell z
	m_TotalDepth = m_TotalCellRows * m_Dz;

	m_Du = 1.0f / m_TotalCellColumns;
	m_Dv = 1.0f / m_TotalCellRows;


	//X and Z offsets
	m_XOffset = -m_TotalWidth * 0.5f;
	m_ZOffset = m_TotalDepth * 0.5;

	m_TotalVertices = rows * columns;
	m_TotalTriangles = (m_TotalCellRows * m_TotalCellColumns) * 2;
	m_TotalIndices = m_TotalTriangles * 3;

	SimpleTerrainVertex* vertices = new SimpleTerrainVertex[m_TotalVertices];

	//LoadHeightmap(513,513,100);

	GenerateDiamondSquare();



	int k = 0;
	for (int r = 0; r < rows; r++) //Z
	{
		for (int c = 0; c < columns; c++) //X
		{
			//m_Heightmap[r * columns + c] = m_GeneratedHeightMap[r][c];
			vertices[k].Pos = XMFLOAT3(c * m_Dx + m_XOffset, m_Heightmap[k], -(r * m_Dz) + m_ZOffset);
			vertices[k].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			vertices[k].TexCoord = XMFLOAT2(c * m_Du, r * m_Dv);
			k++;
		}
	}


	//Buffer description
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleTerrainVertex) * m_TotalVertices;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	//Buffer initilisation
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &vertices[0];

	hr = pDevice->CreateBuffer(&bd, &InitData, &m_pVertexBuffer);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Creating terrain vertex buffer - FAILED", L"Error", MB_OK);
		return hr;
	}


	// Set vertex buffer
	UINT stride = sizeof(SimpleTerrainVertex);
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	GenerateIndices(pDevice,pContext,rows,columns);


	hr = CreateDDSTextureFromFile(pDevice, L"Resources\\Terrain_Textures\\513\\lightdirt.dds", nullptr, &m_pTextures[0]);
	hr = CreateDDSTextureFromFile(pDevice, L"Resources\\Terrain_Textures\\513\\darkdirt.dds", nullptr, &m_pTextures[1]);
	hr = CreateDDSTextureFromFile(pDevice, L"Resources\\Terrain_Textures\\513\\grass.dds", nullptr, &m_pTextures[2]);
	hr = CreateDDSTextureFromFile(pDevice, L"Resources\\Terrain_Textures\\513\\stone.dds", nullptr, &m_pTextures[3]);
	hr = CreateDDSTextureFromFile(pDevice, L"Resources\\Terrain_Textures\\513\\snow.dds", nullptr, &m_pTextures[4]);
	hr = CreateDDSTextureFromFile(pDevice, L"Resources\\Terrain_Textures\\513\\blend.dds", nullptr, &m_pTextures[5]);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"setting terrain texture - FAILED", L"Error", MB_OK);
		return hr;
	}

	//Sampler description
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = pDevice->CreateSamplerState(&sampDesc, &m_pSamplerLinear);



	return hr;
}

HRESULT Terrain::GenerateIndices(ID3D11Device* pDevice, ID3D11DeviceContext* pContext,int rows, int columns)
{
	HRESULT hr;

	vector<DWORD> indices;

	indices.reserve(m_TotalIndices);
	for (DWORD i = 0; i < (DWORD)rows - 1; i++)
	{
		for (DWORD j = 0; j < (DWORD)columns - 1; j++)
		{
			indices.push_back(i * columns + j);
			indices.push_back(i * columns + (j + 1));
			indices.push_back((i + 1) * columns + j);

			indices.push_back((i + 1) * columns + j);
			indices.push_back((i * columns) + (j + 1));
			indices.push_back((i + 1) * columns + (j + 1));
		}
	}

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(DWORD) * m_TotalIndices;
	bd.CPUAccessFlags = 0;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &indices[0];

	hr = pDevice->CreateBuffer(&bd, &InitData, &m_pIndexBuffer);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Creating terrain index buffer - FAILED", L"Error", MB_OK);
		return hr;
	}

	// Set index buffer
	pContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set primitive topology
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	return hr;
}

void Terrain::Update(float t)
{
	XMMATRIX mTranslate = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	XMMATRIX world = mTranslate; //* mSpin;
	XMStoreFloat4x4(&m_WorldMatrix, world);
}


void Terrain::Draw(ID3D11DeviceContext* pContext)
{
	if (m_IsEnabled)
	{
		pContext->VSSetShader(m_pVertexShader, nullptr, 0);
		pContext->PSSetShader(m_pPixelShader, nullptr, 0);

		pContext->IASetInputLayout(m_pInputLayout);

		UINT stride = sizeof(SimpleTerrainVertex);
		UINT offset = 0;
		pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
		pContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
		pContext->PSSetShaderResources(0, 6, m_pTextures);
		pContext->PSSetSamplers(0, 1, &m_pSamplerLinear);
		pContext->DrawIndexed(m_TotalIndices,0,0);
	}
}


void Terrain::LoadHeightmap(int width,int height,int scale)
{
	m_HeightmapWidth = width;
	m_HeightmapHeight = height;
	m_HeightScale = scale;

	// A height for each vertex 
	std::vector<unsigned char> in(m_HeightmapWidth * m_HeightmapHeight);

	// Open the file.
	std::ifstream inFile;
	//inFile.open(heightMapFilename.c_str(), std::ios_base::binary);
	inFile.open("Resources\\Terrain_Textures\\513\\coastMountain513.raw", std::ios_base::binary);
	if (inFile)
	{
		// Read the RAW bytes.
		inFile.read((char*)&in[0], (std::streamsize)in.size());
		// Done with file.
		inFile.close();
	}

	// Copy the array data into a float array and scale it. mHeightmap.resize(heightmapHeight * heightmapWidth, 0);
	m_Heightmap.resize(m_HeightmapHeight * m_HeightmapWidth, 0);
	for (UINT i = 0; i < m_HeightmapHeight * m_HeightmapWidth; ++i)
	{
		m_Heightmap[i] = (in[i] / 255.0f) * m_HeightScale;
	}
}


void Terrain::GenerateDiamondSquare()
{
	//Initialise heightmap
	for (int i = 0; i < WIDTH_Z; i++)
	{
		for (int j = 0; j < WIDTH_X; j++)
		{
			m_GeneratedHeightMap[j][i] = 0;
		}
	}

	int randomHeight = random(5);
	m_GeneratedHeightMap[0][0] = randomHeight; //Top left
	m_GeneratedHeightMap[WIDTH_X - 1][0] = randomHeight; //top right
	m_GeneratedHeightMap[0][WIDTH_Z - 1] = randomHeight; //bottom right
	m_GeneratedHeightMap[WIDTH_X - 1][WIDTH_Z - 1] = randomHeight; //bottom left

	m_Heightmap.resize(WIDTH_X * WIDTH_Z, 0);

	DiamondSquare(m_GeneratedHeightMap,WIDTH_X - 1);

	for (int r = 0; r < m_TerrainHeight; r++) //Z
	{
		for (int c = 0; c < m_TerrainWidth; c++) //X
		{
			m_Heightmap[r * m_TerrainWidth + c] = m_GeneratedHeightMap[r][c];
		}
	}

	SmoothHeights(5);
	
}


void Terrain::DiamondSquare(int Array[WIDTH_X][WIDTH_Z], int size)
{
	int half = size / 2;
	if (half < 1) 
		return;

	//square steps
	for (int z = half; z < WIDTH_Z; z += size) 
	{
		for (int x = half; x < WIDTH_X; x += size)
		{
			SquareStep(Array, x % WIDTH_X, z % WIDTH_Z, half);
		}
	}


	//diamond steps
	int col = 0;
	for (int x = 0; x < WIDTH_X; x += half)
	{
		col++;
		//if this is an odd column.
		if (col % 2 == 1)
		{
			for (int z = half; z < WIDTH_Z; z += size)
			{
				DiamondStep(Array, x % WIDTH_X, z % WIDTH_Z, half);
			}
		}
		else
		{
			for (int z = 0; z < WIDTH_Z; z += size)
			{
				DiamondStep(Array, x % WIDTH_X, z % WIDTH_Z, half);
			}
		}
	}

	DiamondSquare(Array, size / 2);
}


void Terrain::SquareStep(int Array[WIDTH_X][WIDTH_Z], int x, int z, int reach)
{
	int count = 0;
	float avg = 0.0f;
	if (x - reach >= 0 && z - reach >= 0) 
	{
		avg += Array[x - reach][z - reach];
		count++;
	}

	if (x - reach >= 0 && z + reach < WIDTH_Z)
	{
		avg += Array[x - reach][z + reach];
		count++;
	}

	if (x + reach < WIDTH_X && z - reach >= 0)
	{
		avg += Array[x + reach][z - reach];
		count++;
	}

	if (x + reach < WIDTH_X && z + reach < WIDTH_Z)
	{
		avg += Array[x + reach][z + reach];
		count++;
	}

	avg += random(reach);
	avg /= count;
	Array[x][z] = round(avg);
}

void Terrain::DiamondStep(int Array[WIDTH_X][WIDTH_Z], int x, int z, int reach)
{
	int count = 0;
	float avg = 0.0f;

	if (x - reach >= 0)
	{
		avg += Array[x - reach][z];
		count++;
	}

	if (x + reach < WIDTH_X)
	{
		avg += Array[x + reach][z];
		count++;
	}

	if (z - reach >= 0)
	{
		avg += Array[x][z - reach];
		count++;
	}

	if (z + reach < WIDTH_Z)
	{
		avg += Array[x][z + reach];
		count++;
	}


	avg += random(reach);
	avg /= count;
	Array[x][z] = (int)avg;
}

// Creates a local copy of the y (height) values then smoothes      //
// Calculates the average height considering up 8 neighbour heights //
void Terrain::SmoothHeights(int strength)
{
	// Number of passes = smoothness
	for (int i = 0; i < strength; i++) {
		// Local storage vector for heights 
		std::vector< float > heights;

		// Resize to fit incoming data
		heights.resize(m_TerrainHeight * m_TerrainWidth,0);

		// Initialize index counter
		int index = 0;

		// Fill local copy
		for (int j = 0; j < m_TerrainHeight; j++) {
			for (int i = 0; i < m_TerrainWidth; i++) {
				// Get current index
				index = (m_TerrainHeight * j) + i;

				// Set height
				heights[index] = (m_Heightmap[(m_TerrainHeight * j) + i]);
			}
		}

		// Reset index counter
		index = 0;

		// Loop through terrain
		for (int j = 0; j < m_TerrainHeight; j++) {
			for (int i = 0; i < m_TerrainWidth; i++) {
				// Get current index
				index = (m_TerrainHeight * j) + i;

				// Temp average variables
				float averageHeight = 0;
				int   numOfAverages = 0;

				// Check legal neighbours
				// North
				if ((j - 1) > 0) {
					averageHeight += heights[(m_TerrainHeight * (j - 1)) + i];
					numOfAverages++;
				}
				// North-East
				if (((j - 1) > 0) && ((i + 1) < m_TerrainWidth)) {
					averageHeight += heights[(m_TerrainHeight * (j - 1)) + (i + 1)];
					numOfAverages++;
				}
				// East
				if ((i + 1) < m_TerrainWidth) {
					averageHeight += heights[(m_TerrainHeight * j) + (i + 1)];
					numOfAverages++;
				}
				// South-East
				if (((j + 1) < m_TerrainHeight) && ((i + 1) < m_TerrainWidth)) {
					averageHeight += heights[(m_TerrainHeight * (j + 1)) + (i + 1)];
					numOfAverages++;
				}
				// South
				if ((j + 1) < m_TerrainHeight) {
					averageHeight += heights[(m_TerrainHeight * (j + 1)) + i];
					numOfAverages++;
				}
				// South-West
				if (((j + 1) < m_TerrainHeight) && ((i - 1) > 0)) {
					averageHeight += heights[(m_TerrainHeight * (j + 1)) + (i - 1)];
					numOfAverages++;
				}
				// West
				if ((i - 1) > 0) {
					averageHeight += heights[(m_TerrainHeight * j) + (i - 1)];
					numOfAverages++;
				}
				// North-West
				if (((j - 1) > 0) && ((i - 1) > 0)) {
					averageHeight += heights[(m_TerrainHeight * (j - 1)) + (i - 1)];
					numOfAverages++;
				}

				// Calculate and set average height
				if (numOfAverages > 0) {
					m_Heightmap[index] = (averageHeight / numOfAverages);
				}
				else {
					m_Heightmap[index] = 0.0f;
				}
			}
		}
	}
}


float Terrain::GetHeight(float x, float z)
{
	// Transform from terrain local space to "cell" space.
	float c = (x + 0.5f * m_TotalWidth) / m_Dx;
	float d = (z - 0.5f * m_TotalDepth) / -m_Dz;

	// Get the row and column we are in.
	int row = (int)floorf(d);
	int col = (int)floorf(c);

	// Grab the heights of the cell we are in.
	float A = m_Heightmap[row * m_TerrainWidth + col];
	float B = m_Heightmap[row * m_TerrainWidth + col + 1];
	float C = m_Heightmap[(row + 1) * m_TerrainWidth + col];
	float D = m_Heightmap[(row + 1) * m_TerrainWidth + col + 1];

	// Where we are relative to the cell.
	float s = c - (float)col;
	float t = d - (float)row;

	// If upper triangle ABC.
	if (s + t <= 1.0f)
	{
		float uy = B - A;
		float vy = C - A;
		return A + s * uy + t * vy;
	}
	else // lower triangle DCB.
	{
		float uy = C - D;
		float vy = B - D;
		return D + (1.0f - s) * uy + (1.0f - t) * vy;
	}
}

void Terrain::UI()
{
	if (ImGui::CollapsingHeader("Terrain"))
	{
		ImGui::PushID("Terrain");
		//if (ImGui::Button("Regenerate")

		ImGui::Checkbox("Activate", (bool*)&m_IsEnabled);
		ImGui::Separator();
		ImGui::Text("Position");
		ImGui::SliderFloat("X", (float*)&m_position.x, -10, 10) || ImGui::SliderFloat("Y", (float*)&m_position.y, -10, 10) || ImGui::SliderFloat("Z", (float*)&m_position.z, -10, 10);
		ImGui::Separator();
		ImGui::Text("Textures");
		for (ID3D11ShaderResourceView* texture : m_pTextures)
		{
			ImGui::Image((void*)texture, ImVec2(64, 64));
			ImGui::SameLine();
		}
		ImGui::NewLine();
		ImGui::PopID();
	}
}
