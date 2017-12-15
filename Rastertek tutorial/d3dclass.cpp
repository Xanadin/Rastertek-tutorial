#include "d3dclass.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

D3DClass::D3DClass()
{
	m_swapChain = NULL;
	m_device = NULL;
	m_deviceContext = NULL;
	m_depthStencilBuffer = NULL;
	m_renderTargetView = NULL;
	m_depthStencilView = NULL;
	m_rasterState = NULL;
	m_depthStencilState = NULL;
}

D3DClass::D3DClass(const D3DClass &)
{
}

D3DClass::~D3DClass()
{
}
// TODO splittare la funzione tra una parte che fa le query con DXGI e una parte che crea le interfacce D3D11
bool D3DClass::Initialize(int clientWidth, int clientHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear)
{
	m_vsync_enabled = vsync;

	///////////////////////////////////////////////////////
	//				ADAPTER CAPS     				     //
	///////////////////////////////////////////////////////

	IDXGIFactory* factory = NULL;
	IDXGIAdapter* adapter = NULL;		// Rappresenta una scheda video
	IDXGIOutput* adapterOutput = NULL;	// Rappresenta un monitor di una scheda video

	HR(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory));
	// Query primary adapter only
	HR(factory->EnumAdapters(0, &adapter));
	// Query monitor 1 only
	HR(adapter->EnumOutputs(0, &adapterOutput));
	// Query displaymodes for given pixel format

	unsigned int numModes;
	HR(adapterOutput->GetDisplayModeList(backBufferFormat, 0, &numModes, NULL));
	DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}
	HR(adapterOutput->GetDisplayModeList(backBufferFormat, 0, &numModes, displayModeList));

	// Seek matching display format
	unsigned int numerator, denominator;
	for (unsigned int i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)clientWidth)
		{
			if (displayModeList[i].Height == (unsigned int)clientHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	DXGI_ADAPTER_DESC adapterDesc;
	HR(adapter->GetDesc(&adapterDesc));
	m_videoCardMemory = static_cast<int>(adapterDesc.DedicatedVideoMemory / 1024 / 1024);
	if (_tcscpy_s(videoCardDescription, MAXNAMELEN, adapterDesc.Description) != 0)
	{
		return false;
	}

	delete[] displayModeList;
	displayModeList = 0;
	ReleaseCOM(adapterOutput);
	ReleaseCOM(adapter);
	ReleaseCOM(factory);
	// TODO create device without swap chain to test multisample levels
	// Do this in an appropriate function
	// Anche una volta enumerati gli adapters è sempre bene create il device e scartarli se non supportano D3D11

	///////////////////////////////////////////////////////
	//				SWAP CHAIN	+ BACK BUFFER		     //
	///////////////////////////////////////////////////////

	// Initialize swap chain (back - front buffer format, refresh and raster)
	// Single back buffer
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = clientWidth;
	swapChainDesc.BufferDesc.Height = clientHeight;
	swapChainDesc.BufferDesc.Format = backBufferFormat;
	if (m_vsync_enabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	if (fullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;
	// NB: No multisampling
	// Per fare il check sul multisampling usare D3D11CreateDevice function e non D3D11CreateDeviceAndSwapChain
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	///////////////////////////////////////////////////////
	//				DEVICE CREATION					     //
	///////////////////////////////////////////////////////

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	UINT createDeviceFlags = 0;
	#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif
	HR(D3D11CreateDeviceAndSwapChain(
									NULL,
									D3D_DRIVER_TYPE_HARDWARE,
									NULL,
									createDeviceFlags,
									&featureLevel,
									1, 
									D3D11_SDK_VERSION,
									&swapChainDesc,
									&m_swapChain,
									&m_device,
									NULL,
									&m_deviceContext));
	// Now that we have a swap chain we need to get a pointer to the back buffer and then attach it to the swap chain
	// We'll use the CreateRenderTargetView function to attach the back buffer to our swap chain.
	// Initialize depth and back buffer and attach resources (views) to swap chain

	///////////////////////////////////////////////////////
	//				BACK BUFFER	VIEW					 //
	///////////////////////////////////////////////////////
	
	ID3D11Texture2D* backBufferPtr;
	HR(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr));
	HR(m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView));
	ReleaseCOM(backBufferPtr);

	///////////////////////////////////////////////////////
	//				DEPTH STENCIL BUFFER				 //
	///////////////////////////////////////////////////////

	// Durante un resize della finestra o il passaggio tra fullscreen e windowed, bisogna ricreare queste risorse
	// Per questo è meglio mettere anche queste cose in una sua funzione autonoma

	// Depth stencil buffer
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));
	depthBufferDesc.Width = clientWidth;
	depthBufferDesc.Height = clientHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = depthStencilFormat;
	// Anche questi campi dipendono dall'uso del multisampling. Vale quanto sopra
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	//
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;
	HR(m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer));

	///////////////////////////////////////////////////////
	//				DEPTH STENCIL STATE					 //
	///////////////////////////////////////////////////////
	// This allows us to control what type of depth test Direct3D will do for each pixel
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	HR(m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState));
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

	///////////////////////////////////////////////////////
	//				RASTER STATE						 //
	///////////////////////////////////////////////////////

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	HR(m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView));

	//Bind the buffer and depth stencil view to the render pipeline

	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	///////////////////////////////////////////////////////
	//				RASTER STATE						 //
	///////////////////////////////////////////////////////

	// By default DirectX already has a rasterizer state set up and working the exact same as the one below 
	// but you have no control to change it unless you set up one yourself
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;	// Cambiare quando aggiungo il supporto al multisampling
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	HR(m_device->CreateRasterizerState(&rasterDesc, &m_rasterState));
	m_deviceContext->RSSetState(m_rasterState);

	///////////////////////////////////////////////////////
	//				VIEWPORTS							 //
	///////////////////////////////////////////////////////

	D3D11_VIEWPORT viewport;
	viewport.Width = static_cast<float>(clientWidth);
	viewport.Height = static_cast<float>(clientHeight);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	m_deviceContext->RSSetViewports(1, &viewport);

	///////////////////////////////////////////////////////
	//				WORLD PROJECTION MATRICES			 //
	///////////////////////////////////////////////////////

	float fieldOfView = DirectX::XM_PIDIV4;
	float AspectRatio = static_cast<float> (clientWidth) / clientHeight;

	DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(fieldOfView, fieldOfView, screenNear, screenDepth);
	DirectX::XMStoreFloat4x4(&m_projectionMatrix, P);

	DirectX::XMMATRIX I = DirectX::XMMatrixIdentity();
	DirectX::XMStoreFloat4x4(&m_worldMatrix, I);

	DirectX::XMMATRIX O = DirectX::XMMatrixOrthographicLH(static_cast<float> (clientWidth), static_cast<float> (clientHeight), screenNear, screenDepth);
	DirectX::XMStoreFloat4x4(&m_orthoMatrix, O);

	return true;
}

void D3DClass::Shutdown()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if (m_swapChain)
	{
		m_swapChain->SetFullscreenState(false, NULL);
	}
	ReleaseCOM(m_rasterState);
	ReleaseCOM(m_depthStencilView);
	ReleaseCOM(m_depthStencilState);
	ReleaseCOM(m_depthStencilBuffer);
	ReleaseCOM(m_renderTargetView);
	ReleaseCOM(m_deviceContext);
	ReleaseCOM(m_device);
	ReleaseCOM(m_swapChain);
}

void D3DClass::BeginScene()
{
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::Aquamarine)); // TODO portare dentro la classe
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	return;
}

void D3DClass::EndScene()
{
	if (m_vsync_enabled)
	{
		m_swapChain->Present(1, 0);
	}
	else
	{
		m_swapChain->Present(0, 0);
	}
	return;
}

ID3D11Device * D3DClass::GetDevice()
{
	return m_device;
}

ID3D11DeviceContext * D3DClass::GetDeviceContext()
{
	return m_deviceContext;
}

void D3DClass::GetProjectionMatrix(DirectX::XMMATRIX & projectionMatrix)
{
	projectionMatrix = DirectX::XMLoadFloat4x4(&m_projectionMatrix);
	return;
}

void D3DClass::GetWorldMatrix(DirectX::XMMATRIX & worldMatrix)
{
	worldMatrix = DirectX::XMLoadFloat4x4(&m_worldMatrix);
	return;
}

void D3DClass::GetOrthoMatrix(DirectX::XMMATRIX & orthoMatrix)
{
	orthoMatrix = DirectX::XMLoadFloat4x4(&m_orthoMatrix);
	return;
}

void D3DClass::GetVideoCardInfo(LPTSTR name, int & memory)
{
	_tcscpy_s(name, MAXNAMELEN, videoCardDescription);
	memory = m_videoCardMemory;
	return;
}

void D3DClass::EnumAdapters()
{
}

bool D3DClass::CheckAdapter()
{
	return false;
}

bool D3DClass::CreateDevice(ID3D11Device * device)
{
	return false;
}

bool D3DClass::CreateSwapChain(IDXGISwapChain * swapChain)
{
	return false;
}

bool D3DClass::CreateDepthStencilBuffer(ID3D11Texture2D * depthStencilBuffer)
{
	return false;
}

void D3DClass::SetRenderTargets()
{
}

void D3DClass::SetDepthStencilState()
{
}

void D3DClass::SetRasterState()
{
}

void D3DClass::SetViewPorts()
{
}

void D3DClass::BuildMatrices()
{
}
