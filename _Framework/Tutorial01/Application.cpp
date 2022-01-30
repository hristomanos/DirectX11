#include "Application.h"

HRESULT Application::Init(HINSTANCE hInstance, int nCmdShow)
{
	HRESULT hr;

	//Initialize the application window
	hr = InitWindow(hInstance, nCmdShow);
	if (FAILED(hr))
		return hr;


	// Initialize core graphics
	m_pGraphics = new Graphics();
	hr = m_pGraphics->Init(m_hWnd);
	if (FAILED(hr))
		return hr;

	RECT rc;
	GetClientRect(m_hWnd, &rc);
	m_windowWidth = rc.right - rc.left;
	m_windowHeight = rc.bottom - rc.top;

	//Initialise camera
	m_pCamera = new Camera(m_windowWidth, m_windowHeight);

	//Initialise direct input
	m_pDirectInput = new DirectInput();
	if (!m_pDirectInput->InitDirectInput(hInstance, m_hWnd))
	{
		MessageBox(0, L"Direct Input Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}

	
	//Initialise object
	m_pGameObject = new DrawableGameObject();
	hr = m_pGameObject->Init(m_pGraphics->Getd3dDevice(), m_pGraphics->GetImmediateContext());
	if (FAILED(hr))
		return hr;

	m_pGameObject2 = new DrawableGameObject();
	hr = m_pGameObject2->Init(m_pGraphics->Getd3dDevice(), m_pGraphics->GetImmediateContext());
	if (FAILED(hr))
		return hr;


	//Initialise user interface
	//m_pUserInterface = new UserInterface(m_pGraphics,m_pGameObject);
	//m_pUserInterface->Init(m_hWnd,m_pGraphics->Getd3dDevice(),m_pGraphics->GetImmediateContext());

	m_pTerrain = new Terrain();
	m_pTerrain->Init(m_pGraphics->Getd3dDevice(), m_pGraphics->GetImmediateContext(), 513,513);

	InitImGui(m_hWnd, m_pGraphics->Getd3dDevice(), m_pGraphics->GetImmediateContext());

	//const aiScene* pScene = aiImportFile("Resources\\Models\\Walking.fbx", aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	

	return S_OK;
}

void Application::Update(float deltaTime)
{
	static float t = 0.0f;
	static ULONGLONG timeStart = 0;
	ULONGLONG timeCur = GetTickCount64();
	if (timeStart == 0)
		timeStart = timeCur;
	t = (timeCur - timeStart) / 1000.0f;

	m_pDirectInput->DetectInput(deltaTime,m_pCamera, m_hWnd,m_pTerrain);
	

	m_pTerrain->Update(t);

	//Object Update
	//m_pGameObject->enableSpin(true);
	m_pGameObject->Update(t);

	XMFLOAT4 lightPosition = m_pGraphics->GetLightPosition();
	m_pGameObject2->setPosition(XMFLOAT3(lightPosition.x, lightPosition.y, lightPosition.z));
	m_pGameObject2->Update(t);

	if (GetAsyncKeyState(VK_ESCAPE))
	{
		exit(0);
	}
}

void Application::Draw()
{
	m_pGraphics->Draw(m_pCamera->GetCamPosition());

	//Set the texture of the cube to the render target
	//m_pGameObject->SetDiffuseTexture(m_pGraphics->GetScreenQuadShaderResourceView());

	//Draw terrain
	XMMATRIX mGO = XMLoadFloat4x4(m_pTerrain->GetTransform());
	cb1.mWorld = XMMatrixTranspose(mGO);
	cb1.mView = XMMatrixTranspose(m_pCamera->GetViewMatrix());
	cb1.mProjection = XMMatrixTranspose(m_pCamera->GetProjectionMatrix());
	m_pGraphics->GetImmediateContext()->UpdateSubresource(m_pGraphics->GetConstantBuffer(), 0, nullptr, &cb1, 0, 0);
	m_pTerrain->Draw(m_pGraphics->GetImmediateContext());

	//Draw cube1
	mGO = XMLoadFloat4x4(m_pGameObject->getTransform());
	cb1.mWorld = XMMatrixTranspose(mGO);
	cb1.mView = XMMatrixTranspose(m_pCamera->GetViewMatrix());
	cb1.mProjection = XMMatrixTranspose(m_pCamera->GetProjectionMatrix()); 
	m_pGraphics->GetImmediateContext()->UpdateSubresource(m_pGraphics->GetConstantBuffer(), 0, nullptr, &cb1, 0, 0);

	m_pGameObject->Draw(m_pGraphics->GetImmediateContext());

	//light object
	mGO = XMLoadFloat4x4(m_pGameObject2->getTransform());
	cb1.mWorld = XMMatrixTranspose(mGO);
	cb1.mView = XMMatrixTranspose(m_pCamera->GetViewMatrix());
	cb1.mProjection = XMMatrixTranspose(m_pCamera->GetProjectionMatrix());
	m_pGraphics->GetImmediateContext()->UpdateSubresource(m_pGraphics->GetConstantBuffer(), 0, nullptr, &cb1, 0, 0);

	m_pGameObject2->Draw(m_pGraphics->GetImmediateContext());

	//Draw ImGui
	//m_pUserInterface->Render();
	RenderUI();

	//Switch render target to the backbuffer but disable depth buffer
	m_pGraphics->GetImmediateContext()->OMSetRenderTargets(1, m_pGraphics->GetRenderTargetView(), m_pGraphics->GetDepthStencilView());
	m_pGraphics->GetImmediateContext()->ClearRenderTargetView(m_pGraphics->GetRenderTargetView2(), Colors::MidnightBlue);
	m_pGraphics->GetImmediateContext()->ClearDepthStencilView(m_pGraphics->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	//Draw terrain
	mGO = XMLoadFloat4x4(m_pTerrain->GetTransform());
	cb1.mWorld = XMMatrixTranspose(mGO);
	cb1.mView = XMMatrixTranspose(m_pCamera->GetViewMatrix());
	cb1.mProjection = XMMatrixTranspose(m_pCamera->GetProjectionMatrix());
	m_pGraphics->GetImmediateContext()->UpdateSubresource(m_pGraphics->GetConstantBuffer(), 0, nullptr, &cb1, 0, 0);

	m_pTerrain->Draw(m_pGraphics->GetImmediateContext());

	//Draw cube
	 mGO = XMLoadFloat4x4(m_pGameObject->getTransform());
	cb1.mWorld = XMMatrixTranspose(mGO);
	cb1.mView = XMMatrixTranspose(m_pCamera->GetViewMatrix());
	cb1.mProjection = XMMatrixTranspose(m_pCamera->GetProjectionMatrix());
	m_pGraphics->GetImmediateContext()->UpdateSubresource(m_pGraphics->GetConstantBuffer(), 0, nullptr, &cb1, 0, 0);

	m_pGameObject->Draw(m_pGraphics->GetImmediateContext());

	//light object
	mGO = XMLoadFloat4x4(m_pGameObject2->getTransform());
	cb1.mWorld = XMMatrixTranspose(mGO);
	cb1.mView = XMMatrixTranspose(m_pCamera->GetViewMatrix());
	cb1.mProjection = XMMatrixTranspose(m_pCamera->GetProjectionMatrix());
	m_pGraphics->GetImmediateContext()->UpdateSubresource(m_pGraphics->GetConstantBuffer(), 0, nullptr, &cb1, 0, 0);

	m_pGameObject2->Draw(m_pGraphics->GetImmediateContext());

	//Draw ImGui
	//m_pUserInterface->Render();
	RenderUI();

	if(m_pGraphics->GetEnablePostProcess())
	{
		m_pGraphics->RenderToTexture();
	}

	//Draw on screen
	m_pGraphics->GetSwapChain()->Present(0, 0);
}

void Application::Release()
{
	if (m_pGraphics)
		m_pGraphics->Release();

	if (m_pGameObject)
			m_pGameObject->Release();

	if (m_pDirectInput)
		m_pDirectInput->~DirectInput();

	if (m_pUserInterface)
			m_pUserInterface->~UserInterface();
	

}

void Application::InitImGui(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	//Setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); // get input output
	ImGui_ImplWin32_Init(hWnd); // init win 32
	ImGui_ImplDX11_Init(device, deviceContext); //Init directx11
	ImGui::StyleColorsDark(); // style of gui
}

void Application::RenderUI()
{
	//Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//Create ImGui window
	ImGui::Begin("Inspector");
	ImGui::Text("Mouse camera movement");
	ImGui::Text("Enable : I");
	ImGui::Text("Disable : O");
	ImGui::Text("FPS : %.1f", ImGui::GetIO().Framerate);

	bool enableWireFrame = m_pGraphics->GetEnableWireFrame();
	if (ImGui::Checkbox("Wireframe", (bool*)&enableWireFrame))
	{
		m_pGraphics->SetEnableWireFrame(enableWireFrame);
	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Camera"))
	{
		XMFLOAT3* camPos = new XMFLOAT3(0,0,0);
		XMStoreFloat3(camPos, m_pCamera->GetCamPosition());

		ImGui::PushID("Camera");
		ImGui::Text("Position");
		ImGui::DragFloat("X", &camPos->x, 0.1f, -50000.0f, 50000.0f, "%.2f");
		ImGui::DragFloat("Y", &camPos->y, 0.1f, -50000.0f, 50000.0f, "%.2f");
		ImGui::DragFloat("Z", &camPos->z, 0.1f, -50000.0f, 50000.0f, "%.2f");

		ImGui::Separator();

		ImGui::Checkbox("Walk mode", &m_pCamera->m_WalkMode);
		ImGui::DragFloat("Height", &m_pCamera->m_WalkingHeight,0.1f,0.0f,300.0f,"%.2f");

		ImGui::Separator();
		ImGui::DragFloat("Speed", &m_pCamera->m_MovementSpeed, 0.1f, 0.0f, 100.0f, "%.2f");
		ImGui::NewLine();
		ImGui::PopID();
	}

	//UI
	m_pGraphics->UI();
	m_pGameObject->UI();
	m_pTerrain->UI();

	ImGui::End();
	//Assemble together draw data
	ImGui::Render();
	//Render draw data
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{

	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = Helper::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"FrameworkWindowClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	m_hInst = hInstance;
	RECT rc = { 0, 0, 1280, 720 };


	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	m_hWnd = CreateWindow(L"FrameworkWindowClass", 
						  L"Direct3D 11 Framework",
						  WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
						  CW_USEDEFAULT, 
						  CW_USEDEFAULT, 
						  rc.right - rc.left, 
						  rc.bottom - rc.top, 
						  nullptr, 
						  nullptr, 
						  hInstance,
						  nullptr);

	if (!m_hWnd)
		return E_FAIL;

	ShowWindow(m_hWnd, nCmdShow);

	return S_OK;
}

