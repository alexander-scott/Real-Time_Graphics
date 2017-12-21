#pragma once

#include <d3d11_1.h>
#include <DirectXMath.h>
#include "resource.h"

#include "Structures.h"

using namespace DirectX;

static class DX11AppHelper
{
public:
	DX11AppHelper();
	~DX11AppHelper();

	static HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	static HRESULT InitDevice();
	static void Cleanup();

	static HWND                    _hWnd;
	static ID3D11Device*           _pd3dDevice;
	static ID3D11DeviceContext*    _pImmediateContext;
	static IDXGISwapChain*         _pSwapChain;

#pragma region Buffer Variables

	static ID3D11Buffer*           _pVertexBuffer;
	static ID3D11Buffer*           _pIndexBuffer;

	static ID3D11Buffer*           _pPlaneVertexBuffer;
	static ID3D11Buffer*           _pPlaneIndexBuffer;

	static ID3D11Buffer*           _pConstantBuffer;
	static ID3D11Buffer*			_pSMConstantBuffer;

#pragma endregion

private:
	static HRESULT InitConstantBuffers();
	static HRESULT InitVertexBuffer();
	static HRESULT InitIndexBuffer();
	static HRESULT InitRasterizerState();

#pragma region DirectX Initialisation Variables

	static HINSTANCE               _hInst;
	static D3D_DRIVER_TYPE         _pDriverType;
	static D3D_FEATURE_LEVEL       _pFeatureLevel;

#pragma endregion

#pragma region Windows Decriptive Variables

	static UINT _pWindowHeight;
	static UINT _pWindowWidth;

public:
	// Render dimensions - Change here to alter screen resolution
	static UINT _pRenderHeight;
	static UINT _pRenderWidth;

#pragma endregion

private:
#pragma region Rasterizer States

	static ID3D11DepthStencilState* _pDSLessEqual;
	static ID3D11RasterizerState* _pRSCullNone;

	static ID3D11RasterizerState* _pCCWcullMode;
	static ID3D11RasterizerState* _pCWcullMode;

#pragma endregion

};