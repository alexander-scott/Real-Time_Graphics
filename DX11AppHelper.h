#pragma once

#include <d3d11_1.h>
#include <DirectXMath.h>
#include "resource.h"

#include "Structures.h"

using namespace DirectX;

class DX11AppHelper
{
public:
	DX11AppHelper();
	~DX11AppHelper();

	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();

	HWND						_hWnd;
	ID3D11Device*			_pd3dDevice;
	ID3D11DeviceContext*		_pImmediateContext;
	IDXGISwapChain*			_pSwapChain;

	ID3D11Buffer*			_pVertexBuffer;
	ID3D11Buffer*			_pIndexBuffer;

	ID3D11Buffer*			_pPlaneVertexBuffer;
	ID3D11Buffer*			_pPlaneIndexBuffer;

	ID3D11Buffer*			_pConstantBuffer;
	ID3D11Buffer*			_pSMConstantBuffer;

	// Render dimensions - Change here to alter screen resolution
	UINT						_pRenderHeight;
	UINT						_pRenderWidth;

	static DX11AppHelper& Instance()
	{
		static DX11AppHelper Instance;
		return Instance;
	}

private:
	HRESULT InitConstantBuffers();
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();
	HRESULT InitRasterizerState();

	HINSTANCE				_hInst;
	D3D_DRIVER_TYPE			_pDriverType;
	D3D_FEATURE_LEVEL		_pFeatureLevel;

	UINT						_pWindowHeight;
	UINT						_pWindowWidth;

	ID3D11DepthStencilState* _pDSLessEqual;
	ID3D11RasterizerState*	_pRSCullNone;

	ID3D11RasterizerState*	_pCCWcullMode;
	ID3D11RasterizerState*	_pCWcullMode;
};