#include "UserInterface.h"



UserInterface::UserInterface(Graphics* graphics, DrawableGameObject* drawableGameObject)
{
	m_pGraphics = graphics;
	m_pDrawableGameObject = drawableGameObject;
}


UserInterface::~UserInterface()
{
}


void UserInterface::Init(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	//Setup ImGui
	IMGUI_CHECKVERSION();
    ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); // get input output
	ImGui_ImplWin32_Init(hWnd); // init win 32
	ImGui_ImplDX11_Init(device, deviceContext); //Init directx11
	ImGui::StyleColorsDark(); // style of gui
}

void UserInterface::Render()
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

	ImGui::Separator();

	bool enableWireFrame = m_pGraphics->GetEnableWireFrame();
	if (ImGui::Checkbox("Wireframe", (bool*)&enableWireFrame))
	{
		m_pGraphics->SetEnableWireFrame(enableWireFrame);
	}

	if (ImGui::CollapsingHeader("Post-processing"))
	{
		bool enablePostProcess = m_pGraphics->GetEnablePostProcess();
		if (ImGui::Checkbox("Colout tint", (bool*)&enablePostProcess))
		{
			m_pGraphics->SetEnablePostProcess(enablePostProcess);
		}
		float* colourTint = m_pGraphics->GetColourTint();
		if (ImGui::ColorEdit4("Colour tint", colourTint))
		{
			m_pGraphics->SetColourTint(colourTint);
		}
	}


	if (ImGui::CollapsingHeader("Light"))
	{
		ImGui::PushID("Light");
		//Light
		ImGui::Text("Position");
		XMFLOAT4 tempLightPos = m_pGraphics->GetLightPosition(); // Get current light position
			if (ImGui::SliderFloat("X", (float*)& tempLightPos.x, -10, 10) || ImGui::SliderFloat("Y", (float*)& tempLightPos.y, -10, 10) || ImGui::SliderFloat("Z", (float*)&tempLightPos.z, -10, 10))
				m_pGraphics->SetLightPosition(tempLightPos);
			ImGui::PopID();
	}


	if (ImGui::CollapsingHeader("Cube"))
	{

		ImGui::Text("Position");
		XMFLOAT3 tempCubePos = m_pDrawableGameObject->GetPosition();
		if (ImGui::SliderFloat("X", (float*)& tempCubePos.x, -10, 10) || ImGui::SliderFloat("Y", (float*)& tempCubePos.y, -10, 10) || ImGui::SliderFloat("Z", (float*)& tempCubePos.z, -10, 10))
			m_pDrawableGameObject->setPosition(tempCubePos);

		MaterialPropertiesConstantBuffer redPlasticMaterial = m_pDrawableGameObject->GetRedPlasticMaterial();
		if (ImGui::Checkbox("Parallax mapping",(bool*)&redPlasticMaterial.Material.UseDisplacementMap) || ImGui::Checkbox("Normal mapping", (bool*)&redPlasticMaterial.Material.UseBumpMap))
		{
			m_pDrawableGameObject->SetRedPlasticMaterial(redPlasticMaterial);
		}

	}


	ImGui::End();

	//Assemble together draw data
	ImGui::Render();
	//Render draw data
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void UserInterface::CubeWindow()
{
	ImGui::Begin("Cube");
	ImGui::Text("Position");
	XMFLOAT3 tempCubePos = m_pDrawableGameObject->GetPosition();
	if (ImGui::SliderFloat("X", (float*)&tempCubePos.x, -10, 10) || ImGui::SliderFloat("Y", (float*)&tempCubePos.y, -10, 10) || ImGui::SliderFloat("Z", (float*)&tempCubePos.z, -10, 10))
		m_pDrawableGameObject->setPosition(tempCubePos);

	ImGui::Text("Textures");
	for(ID3D11ShaderResourceView * texture : m_pDrawableGameObject->g_pTextureArr)
	{
		ImGui::Image((void*)texture, ImVec2(64, 64));
		ImGui::SameLine();
	}


	ImGui::End();
}