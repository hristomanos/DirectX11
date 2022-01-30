#include "Graphics.h"
#include "Graphics.h"

HRESULT Graphics::Init(HWND hWnd)
{
	HRESULT hr;

	//Initialise the direct 3D device
	hr = InitDevice(hWnd);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"Failed to init device", L"Error", MB_OK);
		Release();
		return hr;
	}
	
	// Create the constant buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = m_pd3dDevice->CreateBuffer(&bd, nullptr, &m_pConstantBuffer);
	if (FAILED(hr))
		return hr;

	 //Create the material constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(MaterialPropertiesConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = m_pd3dDevice->CreateBuffer(&bd, nullptr, &m_pMaterialConstantBuffer);
	if (FAILED(hr))
		return hr;

	// Create the light constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(LightPropertiesConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = m_pd3dDevice->CreateBuffer(&bd, nullptr, &m_pLightConstantBuffer);
	if (FAILED(hr))
		return hr;

	// Create the post process constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(PostProcessConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = m_pd3dDevice->CreateBuffer(&bd, nullptr, &m_pPostProcessConstantBuffer);
	if (FAILED(hr))
		return hr;


	//Instancing
	ConstantBufferPerScene constantBufferPerInst;
	//Create constant buffer per scene
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBufferPerScene);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = m_pd3dDevice->CreateBuffer(&bd, nullptr, &m_ConstantPerInstanceBuffer);
	if (FAILED(hr))
		return hr;
	m_pImmediateContext->UpdateSubresource(m_ConstantPerInstanceBuffer, 0, nullptr, &constantBufferPerInst, 0, 0);

	

	return hr;

}

HRESULT Graphics::InitDevice(HWND hWnd)
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		m_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);
		}

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = m_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	if (FAILED(hr))
		return hr;

	// Create swap chain
	IDXGIFactory2* dxgiFactory2 = nullptr;
	hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		hr = m_pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&m_pd3dDevice1));
		if (SUCCEEDED(hr))
		{
			(void)m_pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&m_pImmediateContext1));
		}

		DXGI_SWAP_CHAIN_DESC1 sd = {};
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; //  DXGI_FORMAT_R16G16B16A16_FLOAT;////DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;

		hr = dxgiFactory2->CreateSwapChainForHwnd(m_pd3dDevice, hWnd, &sd, nullptr, nullptr, &m_pSwapChain1);
		if (SUCCEEDED(hr))
		{
			hr = m_pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&m_pSwapChain));
		}

		dxgiFactory2->Release();
	}
	else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		hr = dxgiFactory->CreateSwapChain(m_pd3dDevice, &sd, &m_pSwapChain);
	}

	// Note this framework doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
	dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

	dxgiFactory->Release();

	if (FAILED(hr))
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
		return hr;

	hr = m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 2;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = m_pd3dDevice->CreateTexture2D(&descDepth, nullptr, &m_pDepthStencil);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Creating depth stencil - FAILED", L"Error", MB_OK);
		return hr;
	}

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = m_pd3dDevice->CreateDepthStencilView(m_pDepthStencil, &descDSV, &m_pDepthStencilView);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Creating depth stencil view - FAILED", L"Error", MB_OK);
		return hr;
	}

	//Set render target
	//m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	//Render to target texture description
	D3D11_TEXTURE2D_DESC renderToTextureDesc;
	ZeroMemory(&renderToTextureDesc,sizeof(renderToTextureDesc));
	renderToTextureDesc.Width = width;
	renderToTextureDesc.Height = height;
	renderToTextureDesc.MipLevels = 1;
	renderToTextureDesc.ArraySize = 1;
	renderToTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	renderToTextureDesc.SampleDesc.Count = 1;
	renderToTextureDesc.SampleDesc.Quality = 0;
	renderToTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	renderToTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	renderToTextureDesc.CPUAccessFlags = 0;
	renderToTextureDesc.MiscFlags = 0;

	//Create render to target texture
	hr = m_pd3dDevice->CreateTexture2D(&renderToTextureDesc, nullptr, &m_pRenderTargetTexture);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Creating render to texture texture - FAILED", L"Error", MB_OK);
		return hr;
	}

	//Create render to target view
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	ZeroMemory(&renderTargetViewDesc, sizeof(renderTargetViewDesc));
	renderTargetViewDesc.Format = renderToTextureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	hr = m_pd3dDevice->CreateRenderTargetView(m_pRenderTargetTexture, &renderTargetViewDesc, &m_pRenderToTextureView);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Creating render to texture view - FAILED", L"Error", MB_OK);
		return hr;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = renderToTextureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	hr = m_pd3dDevice->CreateShaderResourceView(m_pRenderTargetTexture, &shaderResourceViewDesc, &m_ScreenQuadShaderResourceView);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Creating screen quad shader resource view - FAILED", L"Error", MB_OK);
		return hr;
	}

	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	 hr = Helper::CompileShaderFromFile(L"ScreenQuadShader.fx", "QuadVS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Compiling screen quad vertex shader - FAILED", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = m_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pScreenQuadVS);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Creating screen quad vertex shader - FAILED", L"Error", MB_OK);
		pVSBlob->Release();
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = m_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
										 pVSBlob->GetBufferSize(), &m_pScreenQuadInputLayout);
	pVSBlob->Release();
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Creating screen quad input layout - FAILED", L"Error", MB_OK);
		return hr;
	}

	//// Set the input layout
	m_pImmediateContext->IASetInputLayout(m_pScreenQuadInputLayout);

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	hr = Helper::CompileShaderFromFile(L"ScreenQuadShader.fx", "QuadPS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Compiling screen quad pixel shader - FAILED", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = m_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pScreenQuadPS);
	pPSBlob->Release();
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Creating pixel shader - FAILED", L"Error", MB_OK);
		return hr;
	}


	//Vertices of a 2D screen quad
	SCREEN_VERTEX svQuad[4] =
	{
		{ XMFLOAT3(-1.0f, -1.0f,  0.0f), XMFLOAT3(0.0f, 1.0f,  0.0f), XMFLOAT2(0.0f,1.0f)}, //0 left bottom
		{ XMFLOAT3(-1.0f, 1.0f,   0.0f), XMFLOAT3(0.0f, 1.0f,  0.0f), XMFLOAT2(0.0f,0.0f)}, //1 left top
		{ XMFLOAT3( 1.0f, 1.0f,   0.0f), XMFLOAT3(0.0f, 1.0f,  0.0f), XMFLOAT2(1.0f,0.0f)}, //2 right top
		{ XMFLOAT3( 1.0f, -1.0f,  0.0f), XMFLOAT3(0.0f, 1.0f,  0.0f), XMFLOAT2(1.0f,1.0f)}, //3 right bottom
	};

	//Vertex buffer description
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SCREEN_VERTEX) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	//Vertex buffer subresource data
	D3D11_SUBRESOURCE_DATA InitData = {};
	ZeroMemory(&InitData,sizeof(InitData));
	InitData.pSysMem = svQuad;
	hr = m_pd3dDevice->CreateBuffer(&bd, &InitData, &m_pScreenQuadVB);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Creating screen quad vertex buffer - FAILED", L"Error", MB_OK);
		return hr;
	}

	// Set vertex buffer
	UINT stride = sizeof(SCREEN_VERTEX);
	UINT offset = 0;
	m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pScreenQuadVB, &stride, &offset);


	WORD svQuadIndices[] =
	{
		0,1,2,
		0,2,3,
	};

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 2 * 3;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = svQuadIndices;
	hr = m_pd3dDevice->CreateBuffer(&bd, &InitData, &m_pScreenQuadIB);
	if (FAILED(hr))
		return hr;

	// Set index buffer
	m_pImmediateContext->IASetIndexBuffer(m_pScreenQuadIB, DXGI_FORMAT_R16_UINT, 0);

	//Primitive topology
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	hr = CreateDDSTextureFromFile(m_pd3dDevice, L"Resources\\color.dds", nullptr, &m_ScreenQuadTexTemp);


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
	hr = m_pd3dDevice->CreateSamplerState(&sampDesc, &m_pSamplerLinear);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	ZeroMemory(&vp, sizeof(D3D11_VIEWPORT));
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_pImmediateContext->RSSetViewports(1, &vp);

	//Wireframe description
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc,sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	hr = m_pd3dDevice->CreateRasterizerState(&rasterizerDesc,&m_pWireFrame);

	//Wireframe description
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	hr = m_pd3dDevice->CreateRasterizerState(&rasterizerDesc, &m_pSolid);

	return S_OK;
}

void Graphics::Draw(XMVECTOR camPos)
{

	ID3D11ShaderResourceView* null[] = { nullptr, nullptr };
	m_pImmediateContext->PSSetShaderResources(0, 2, null);

	//Set render to texture as a render target
	m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderToTextureView, nullptr);

	// Clear render to texture buffer
	//m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, Colors::MidnightBlue);
	m_pImmediateContext->ClearRenderTargetView(m_pRenderToTextureView, Colors::MidnightBlue);

	// Clear the depth buffer to 1.0 (max depth)
	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


	if (m_EnableWireFrame) { m_pImmediateContext->RSSetState(m_pWireFrame); } else { m_pImmediateContext->RSSetState(m_pSolid); }

	ConstantBuffer cb1;
	cb1.vOutputColor = XMFLOAT4(1, 0, 0, 0);
	m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, nullptr, &cb1, 0, 0);

	Light light;
	light.Enabled = static_cast<int>(true);
	light.LightType = PointLight;
	light.Color = XMFLOAT4(Colors::White);
	light.SpotAngle = XMConvertToRadians(45.0f);
	light.ConstantAttenuation = 1.0f;
	light.LinearAttenuation = 1;
	light.QuadraticAttenuation = 1;
	
	light.Position = m_LightPosition;


	LightPropertiesConstantBuffer lightProperties;
	XMStoreFloat4(&lightProperties.EyePosition,camPos);
	lightProperties.Lights[0] = light;
	m_pImmediateContext->UpdateSubresource(m_pLightConstantBuffer, 0, nullptr, &lightProperties, 0, 0);

	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	m_pImmediateContext->VSSetConstantBuffers(2, 1, &m_pLightConstantBuffer);
	m_pImmediateContext->PSSetConstantBuffers(2, 1, &m_pLightConstantBuffer);

}

void Graphics::RenderToTexture()
{
	//Render full screen Quad
	//4)Set shaders
	m_pImmediateContext->VSSetShader(m_pScreenQuadVS, nullptr, 0);
	m_pImmediateContext->PSSetShader(m_pScreenQuadPS, nullptr, 0);

	//5)Set input layout
	m_pImmediateContext->IASetInputLayout(m_pScreenQuadInputLayout);

	//postProcCB.colour = XMFLOAT4(0.0f,1.0f,0.0f,1.0f);
	postProcCB.colour = XMFLOAT4(m_ColourTint);
	m_pImmediateContext->UpdateSubresource(m_pPostProcessConstantBuffer, 0, nullptr, &postProcCB, 0, 0);

	//6)Set buffers
	//// Set vertex buffer
	UINT stride = sizeof(SCREEN_VERTEX);
	UINT offset = 0;
	m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pScreenQuadVB, &stride, &offset);
	m_pImmediateContext->IASetIndexBuffer(m_pScreenQuadIB, DXGI_FORMAT_R16_UINT, 0);
	//m_pGraphics->GetImmediateContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_pImmediateContext->PSSetConstantBuffers(0, 1, &m_pPostProcessConstantBuffer);
	m_pImmediateContext->PSSetShaderResources(0, 1, &m_ScreenQuadShaderResourceView);
	m_pImmediateContext->PSSetSamplers(0, 1, &m_pSamplerLinear);
	m_pImmediateContext->DrawIndexed(6, 0, 0);
	//m_pImmediateContext->Draw(4, 0);
}

void Graphics::UI()
{
	//ImGui::Checkbox("Wireframe", (bool*)&m_EnableWireFrame);
	if (ImGui::CollapsingHeader("Post-processing"))
	{
		ImGui::Checkbox("Colout tint", (bool*)&m_EnablePostProcess);
		ImGui::ColorEdit4("Colour tint", m_ColourTint);
	}

	if (ImGui::CollapsingHeader("Light"))
	{
		ImGui::PushID("Light");
		ImGui::Text("Position");
		ImGui::SliderFloat("X", (float*)&m_LightPosition.x, -10, 10) || ImGui::SliderFloat("Y", (float*)&m_LightPosition.y, -10, 10) || ImGui::SliderFloat("Z", (float*)&m_LightPosition.z, -10, 10);
		ImGui::PopID();
	}
}


void Graphics::Release()
{

	if (m_pImmediateContext) m_pImmediateContext->ClearState();	
	if (m_pConstantBuffer)	m_pConstantBuffer->Release();
	if (m_pDepthStencil) m_pDepthStencil->Release();
	if (m_pDepthStencilView) m_pDepthStencilView->Release();
	if (m_pRenderTargetView) m_pRenderTargetView->Release();
	if (m_pSwapChain1) m_pSwapChain1->Release();
	if (m_pSwapChain) m_pSwapChain->Release();
	if (m_pImmediateContext1) m_pImmediateContext1->Release();
	if (m_pImmediateContext) m_pImmediateContext->Release();
	if (m_pd3dDevice) m_pd3dDevice->Release();
	if (m_pd3dDevice1) m_pd3dDevice1->Release();

	if (m_pRenderToTextureView)m_pRenderToTextureView->Release();
	if (m_pRenderTargetTexture)	m_pRenderTargetTexture->Release();

	if (m_pScreenQuadVB)	m_pScreenQuadVB->Release();
	if (m_pScreenQuadIB)	m_pScreenQuadIB->Release();
	if (m_pScreenQuadVS)	m_pScreenQuadVS->Release();
	if (m_pScreenQuadPS)	m_pScreenQuadPS->Release();
	if (m_pScreenQuadInputLayout)	m_pScreenQuadInputLayout->Release();

	if (m_pWireFrame) {	m_pWireFrame->Release(); }

}



