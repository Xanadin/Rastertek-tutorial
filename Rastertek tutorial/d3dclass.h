#pragma once
#include "StdInclude.h"

#include <d3d11.h>
#include <DirectXColors.h>

class D3DClass
{
public:
	D3DClass();
	D3DClass(const D3DClass&);
	~D3DClass();

	bool Initialize(int clientWidth, int clientHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear);
	void Shutdown();

	void BeginScene();
	void EndScene();

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

	void GetProjectionMatrix(DirectX::XMMATRIX& projectionMatrix);
	void GetWorldMatrix(DirectX::XMMATRIX& worldMatrix);
	void GetOrthoMatrix(DirectX::XMMATRIX& orthoMatrix);

	void GetVideoCardInfo(LPTSTR name, int& memory);

private:
	void EnumAdapters();
	bool CheckAdapter();
	bool CreateDevice(ID3D11Device* device);
	bool CreateSwapChain(IDXGISwapChain* swapChain);
	bool CreateDepthStencilBuffer(ID3D11Texture2D* depthStencilBuffer);
	void SetRenderTargets();
	void SetDepthStencilState();
	void SetRasterState();
	void SetViewPorts();
	void BuildMatrices();

	IDXGISwapChain* m_swapChain;

	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;

	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11DepthStencilView* m_depthStencilView;

	ID3D11RasterizerState* m_rasterState;
	ID3D11DepthStencilState* m_depthStencilState;

	bool m_vsync_enabled;
	int m_videoCardMemory;	// In MegaBytes
	TCHAR videoCardDescription[MAXNAMELEN];

	DirectX::XMFLOAT4X4 m_projectionMatrix;
	DirectX::XMFLOAT4X4 m_worldMatrix;
	DirectX::XMFLOAT4X4 m_orthoMatrix;

	DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

};
