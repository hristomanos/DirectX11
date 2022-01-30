#pragma once

#include <d3d11_1.h>
#include "Utilities/Helper.h"
#include "Resource.h"
#include <directxmath.h>
#include "structures.h"
#include <directxcolors.h>
#include "DDSTextureLoader.h"

#define DEFAULT_WINDOW_TITLE L"Framework"
#define DEFAULT_WINDOW_CLASS L"FrameworkClass"

#define DEFAULT_WINDOW_WIDTH 1280;
#define DEFAULT_WINDOW_HEIGHT 720;


class Graphics
{
public:
	~Graphics() { Release(); };

	HRESULT Init(HWND hWnd);
	void Draw(XMVECTOR camPos);
	void Release();

	ID3D11Device*						Getd3dDevice()									{ return m_pd3dDevice; };
	ID3D11Device1*						Getd3dDevice1()									{ return m_pd3dDevice1; };
	ID3D11DeviceContext*				GetImmediateContext()							{ return m_pImmediateContext; };
	ID3D11DeviceContext1*				GetImmediateContext1()							{ return m_pImmediateContext1; };
	IDXGISwapChain*						GetSwapChain()									{ return m_pSwapChain; };
	IDXGISwapChain1*					GetSwapChain1()									{ return m_pSwapChain1; };
	ID3D11RenderTargetView**			GetRenderTargetView()							{ return &m_pRenderTargetView; };
	ID3D11RenderTargetView*				GetRenderTargetView2()							{ return m_pRenderTargetView; };
	ID3D11Texture2D*					GetDepthStencil()								{ return m_pDepthStencil; };
	ID3D11DepthStencilView*				GetDepthStencilView()							{ return m_pDepthStencilView; };

	
	ID3D11Buffer*						GetConstantBuffer()								{ return m_pConstantBuffer; };
	ID3D11Buffer*						GetMaterialConstantBuffer()						{ return m_pMaterialConstantBuffer; };
	ID3D11Buffer*						GetLightConstantBuffer()						{ return m_pLightConstantBuffer; };


	ID3D11ShaderResourceView**			GetScreenQuadShaderResourceView()				{ return &m_ScreenQuadShaderResourceView; }
	ID3D11ShaderResourceView**			GetScreenQuadTexTemp()							{ return &m_ScreenQuadTexTemp; }
	ID3D11SamplerState**				GetScreenQuadSamplerState()						{ return &m_pSamplerLinear; }

	ID3D11Buffer*						GetScreenQuadVB()								{ return m_pScreenQuadVB; };
	ID3D11Buffer**						GetScreenQuadVB2()								{ return &m_pScreenQuadVB; };
	ID3D11Buffer*						GetScreenQuadIB()								{ return m_pScreenQuadIB; };

	ID3D11VertexShader*					GetScreenQuadVS()								{ return m_pScreenQuadVS; }
	ID3D11PixelShader*					GetScreenQuadPS()								{ return m_pScreenQuadPS; }

	ID3D11InputLayout*					GetScreenQuadInputLayout()						{ return m_pScreenQuadInputLayout; }

	
	void Setd3dDevice(ID3D11Device *pd3dDevice)								{ m_pd3dDevice = pd3dDevice; };
	void Setd3dDevice1(ID3D11Device1 *pd3dDevice1)							{ m_pd3dDevice1 = pd3dDevice1; };
	void SetImmediateContext(ID3D11DeviceContext *pImmediateContext)		{ m_pImmediateContext = pImmediateContext; };
	void SetImmediateContext1(ID3D11DeviceContext1 *pImmediateContext1)		{ m_pImmediateContext1 = pImmediateContext1; };
	void SetSwapChain(IDXGISwapChain *pSwapChain)							{ m_pSwapChain = pSwapChain; };
	void SetSwapChain1(IDXGISwapChain1 *pSwapChain1)						{ m_pSwapChain1 = pSwapChain1; };
	void SetRenderTargetView(ID3D11RenderTargetView *pRenderTargetView)		{ m_pRenderTargetView = pRenderTargetView; };
	void SetDepthStencil(ID3D11Texture2D *pDepthStencil)					{ m_pDepthStencil = pDepthStencil; };
	void SetDepthStencilView(ID3D11DepthStencilView *pDepthStencilView)		{ m_pDepthStencilView = pDepthStencilView; };


	void SetConstantBuffer(ID3D11Buffer *pConstantBuffer)					{ m_pConstantBuffer = pConstantBuffer; };
	void SetMaterialConstantBuffer(ID3D11Buffer *pMaterialConstantBuffer)	{ m_pMaterialConstantBuffer = pMaterialConstantBuffer; };
	void SetLightConstantBuffer(ID3D11Buffer *pLightConstantBuffer)			{ m_pLightConstantBuffer = pLightConstantBuffer; };

	ID3D11RasterizerState* GetWireFrameState() { return m_pWireFrame; }

	HRESULT InitDevice(HWND hwnd);
	
	XMFLOAT4 GetLightPosition()												{ return m_LightPosition; }
	void	 SetLightPosition(XMFLOAT4 newLightPos)							{ m_LightPosition = newLightPos; }

	void RenderToTexture();

	bool GetEnablePostProcess() { return m_EnablePostProcess; }
	void SetEnablePostProcess(bool enable) { m_EnablePostProcess = enable; }
	
	bool GetEnableWireFrame() { return m_EnableWireFrame; }
	void SetEnableWireFrame(bool enable) { m_EnableWireFrame = enable; }

	float* GetColourTint() { return m_ColourTint; }
	void SetColourTint(float* newColour) { for (int i = 0; i < 4; i++)	m_ColourTint[i] = newColour[i]; }

	void UI();
private:
	D3D_DRIVER_TYPE		    m_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL		m_featureLevel = D3D_FEATURE_LEVEL_11_0;

	ID3D11Device*			m_pd3dDevice = nullptr;
	ID3D11Device1*			m_pd3dDevice1 = nullptr;
	ID3D11DeviceContext*	m_pImmediateContext = nullptr;
	ID3D11DeviceContext1*	m_pImmediateContext1 = nullptr;
	IDXGISwapChain*			m_pSwapChain = nullptr;
	IDXGISwapChain1*		m_pSwapChain1 = nullptr;
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;

	//Render to texture
	ID3D11Texture2D*		m_pRenderTargetTexture = nullptr; //Texture to render to
	ID3D11RenderTargetView* m_pRenderToTextureView = nullptr; // Render target

	ID3D11ShaderResourceView* m_ScreenQuadShaderResourceView = nullptr; //Shader resource view for the texture
	ID3D11SamplerState*		m_pSamplerLinear = nullptr;

	//Test textures
	ID3D11ShaderResourceView* m_ScreenQuadTexTemp = nullptr; //render the scene

	bool m_EnablePostProcess = false;

	//Screen quad
	ID3D11Buffer*			m_pScreenQuadVB = nullptr;
	ID3D11Buffer*			m_pScreenQuadIB = nullptr;
	ID3D11VertexShader*		m_pScreenQuadVS = nullptr;
	ID3D11PixelShader*		m_pScreenQuadPS = nullptr;
	ID3D11InputLayout*		m_pScreenQuadInputLayout = nullptr;

	ID3D11Texture2D*		m_pDepthStencil = nullptr;
	ID3D11DepthStencilView* m_pDepthStencilView = nullptr;

	ID3D11Buffer*			m_pPostProcessConstantBuffer = nullptr;
	PostProcessConstantBuffer postProcCB;
	float m_ColourTint[4] = { 0.0f, 0.5f, 0.0f, 1.0f };

	ID3D11Buffer*			m_pConstantBuffer = nullptr;
	ID3D11Buffer*			m_pMaterialConstantBuffer = nullptr;

	//Light
	ID3D11Buffer*			m_pLightConstantBuffer = nullptr;
	XMFLOAT4				m_LightPosition = XMFLOAT4(0.0f, 0.0f, -5.0f, 1.0f);

	ID3D11Buffer*			m_ConstantPerInstanceBuffer = nullptr;

	D3D11_VIEWPORT vpOld[D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX];
	UINT nViewPorts = 1;

	bool m_EnableWireFrame = false;
	ID3D11RasterizerState* m_pWireFrame;
	ID3D11RasterizerState* m_pSolid;
};


