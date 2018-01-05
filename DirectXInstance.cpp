#include "DirectXInstance.h"



DirectXInstance::DirectXInstance()
{
	HINSTANCE _hInst = nullptr;
	HWND _hWnd = nullptr;
	D3D_DRIVER_TYPE _pDriverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL _pFeatureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device* _pd3dDevice = nullptr;
	ID3D11DeviceContext* _pImmediateContext = nullptr;
	IDXGISwapChain* _pSwapChain = nullptr;

	ID3D11Buffer* _pVertexBuffer = nullptr;
	ID3D11Buffer* _pIndexBuffer = nullptr;
	ID3D11Buffer* _pPlaneVertexBuffer = nullptr;
	ID3D11Buffer* _pPlaneIndexBuffer = nullptr;
	ID3D11Buffer* _pConstantBuffer = nullptr;
	ID3D11Buffer* _pSMConstantBuffer = nullptr;

	ID3D11DepthStencilState* _pDSLessEqual = nullptr;
	ID3D11RasterizerState* _pRSCullNone = nullptr;
	ID3D11RasterizerState* _pCCWcullMode = nullptr;
	ID3D11RasterizerState* _pCWcullMode = nullptr;

	UINT _pWindowHeight;
	UINT _pWindowWidth;

	UINT _pRenderHeight = 1080;
	UINT _pRenderWidth = 1920;
}

DirectXInstance::~DirectXInstance()
{
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

HRESULT DirectXInstance::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	_pRenderWidth = desktop.right;
	_pRenderHeight = desktop.bottom;

	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"TutorialWindowClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	_hInst = hInstance;
	RECT rc = { 0, 0, _pRenderWidth, _pRenderHeight };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	_hWnd = CreateWindow(L"TutorialWindowClass", L"Real-Time Graphics for Games", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
		nullptr);
	if (!_hWnd)
		return E_FAIL;

	ShowWindow(_hWnd, nCmdShow);

	GetClientRect(_hWnd, &rc);
	_pWindowWidth = rc.right - rc.left;
	_pWindowHeight = rc.bottom - rc.top;

	return S_OK;
}

HRESULT DirectXInstance::InitDevice()
{
	HRESULT hr = S_OK;

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
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	UINT sampleCount = 4;

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = _pRenderWidth;
	sd.BufferDesc.Height = _pRenderHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = _hWnd;
	sd.SampleDesc.Count = sampleCount;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		_pDriverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(nullptr, _pDriverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_pFeatureLevel, &_pImmediateContext);
		if (SUCCEEDED(hr))
			break;
	}

	_pSwapChain->SetFullscreenState(false, NULL);

	if (FAILED(hr))
		return hr;

	hr = InitConstantBuffers();

	if (FAILED(hr))
		return hr;

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)_pRenderWidth;
	vp.Height = (FLOAT)_pRenderHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	_pImmediateContext->RSSetViewports(1, &vp);

	InitVertexBuffer();
	InitIndexBuffer();
	InitRasterizerState();

	// Set primitive topology
	_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	hr = InitConstantBuffers();

	return hr;
}


HRESULT DirectXInstance::InitVertexBuffer()
{
	HRESULT hr;

	// Frank Luna Normals and Tangents
	SimpleVertex vertices[] =
	{
		// Top Face
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f),		XMFLOAT3(0.0f, 1.0f, 0.0f),		XMFLOAT2(1.0f, 0.0f),  XMFLOAT3(-1.0f, 0.0f, 0.0f) },	// 0
		{ XMFLOAT3(1.0f, 1.0f, -1.0f),		XMFLOAT3(0.0f, 1.0f, 0.0f),		XMFLOAT2(0.0f, 0.0f),  XMFLOAT3(-1.0f, 0.0f, 0.0f) },	// 1
		{ XMFLOAT3(1.0f, 1.0f, 1.0f),		XMFLOAT3(0.0f, 1.0f, 0.0f),		XMFLOAT2(0.0f, 1.0f),  XMFLOAT3(-1.0f, 0.0f, 0.0f) },	// 2
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f),		XMFLOAT3(0.0f, 1.0f, 0.0f),		XMFLOAT2(1.0f, 1.0f),  XMFLOAT3(-1.0f, 0.0f, 0.0f) },	// 3

																																	// Bottom Face
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f),	XMFLOAT3(0.0f, 1.0f, 0.0f),		XMFLOAT2(0.0f, 0.0f),  XMFLOAT3(1.0f, 0.0f, 0.0f) },	// 4
		{ XMFLOAT3(1.0f, -1.0f, -1.0f),		XMFLOAT3(0.0f, 1.0f, 0.0f),		XMFLOAT2(1.0f, 0.0f),  XMFLOAT3(1.0f, 0.0f, 0.0f) },	// 5
		{ XMFLOAT3(1.0f, -1.0f, 1.0f),		XMFLOAT3(0.0f, 1.0f, 0.0f),		XMFLOAT2(1.0f, 1.0f),  XMFLOAT3(1.0f, 0.0f, 0.0f) },	// 6
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f),		XMFLOAT3(0.0f, 1.0f, 0.0f),		XMFLOAT2(0.0f, 1.0f),  XMFLOAT3(1.0f, 0.0f, 0.0f) },	// 7

																																	// Left Face
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f),		XMFLOAT3(-1.0f, 0.0f, 0.0f),	XMFLOAT2(0.0f, 1.0f),  XMFLOAT3(0.0f, 0.0f, -1.0f) },	// 8
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f),	XMFLOAT3(-1.0f, 0.0f, 0.0f),	XMFLOAT2(1.0f, 1.0f),  XMFLOAT3(0.0f, 0.0f, -1.0f) },	// 9
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f),		XMFLOAT3(-1.0f, 0.0f, 0.0f),	XMFLOAT2(1.0f, 0.0f),  XMFLOAT3(0.0f, 0.0f, -1.0f) },	// 10
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f),		XMFLOAT3(-1.0f, 0.0f, 0.0f),	XMFLOAT2(0.0f, 0.0f),  XMFLOAT3(0.0f, 0.0f, -1.0f) },	// 11

																																	// Right Face
		{ XMFLOAT3(1.0f, -1.0f, 1.0f),		XMFLOAT3(1.0f, 0.0f, 0.0f),		XMFLOAT2(1.0f, 1.0f),  XMFLOAT3(0.0f, 0.0f, 1.0f) },	// 12
		{ XMFLOAT3(1.0f, -1.0f, -1.0f),		XMFLOAT3(1.0f, 0.0f, 0.0f),		XMFLOAT2(0.0f, 1.0f),  XMFLOAT3(0.0f, 0.0f, 1.0f) },	// 13
		{ XMFLOAT3(1.0f, 1.0f, -1.0f),		XMFLOAT3(1.0f, 0.0f, 0.0f),		XMFLOAT2(0.0f, 0.0f),  XMFLOAT3(0.0f, 0.0f, 1.0f) },	// 14
		{ XMFLOAT3(1.0f, 1.0f, 1.0f),		XMFLOAT3(1.0f, 0.0f, 0.0f),		XMFLOAT2(1.0f, 0.0f),  XMFLOAT3(0.0f, 0.0f, 1.0f) },	// 15

																																	// Front Face
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f),	XMFLOAT3(0.0f, 0.0f, -1.0f),	XMFLOAT2(0.0f, 1.0f),  XMFLOAT3(1.0f, 0.0f, 0.0f) },	// 16
		{ XMFLOAT3(1.0f, -1.0f, -1.0f),		XMFLOAT3(0.0f, 0.0f, -1.0f),	XMFLOAT2(1.0f, 1.0f),  XMFLOAT3(1.0f, 0.0f, 0.0f) },	// 17
		{ XMFLOAT3(1.0f, 1.0f, -1.0f),		XMFLOAT3(0.0f, 0.0f, -1.0f),	XMFLOAT2(1.0f, 0.0f),  XMFLOAT3(1.0f, 0.0f, 0.0f) },	// 18
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f),		XMFLOAT3(0.0f, 0.0f, -1.0f),	XMFLOAT2(0.0f, 0.0f),  XMFLOAT3(1.0f, 0.0f, 0.0f) },	// 19

																																	// Back Face
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f),		XMFLOAT3(0.0f, 0.0f, 1.0f),		XMFLOAT2(1.0f, 1.0f),  XMFLOAT3(-1.0f, 0.0f, 0.0f) },	// 20
		{ XMFLOAT3(1.0f, -1.0f, 1.0f),		XMFLOAT3(0.0f, 0.0f, 1.0f),		XMFLOAT2(0.0f, 1.0f),  XMFLOAT3(-1.0f, 0.0f, 0.0f) },	// 21
		{ XMFLOAT3(1.0f, 1.0f, 1.0f),		XMFLOAT3(0.0f, 0.0f, 1.0f),		XMFLOAT2(0.0f, 0.0f),  XMFLOAT3(-1.0f, 0.0f, 0.0f) },	// 22
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f),		XMFLOAT3(0.0f, 0.0f, 1.0f),		XMFLOAT2(1.0f, 0.0f),  XMFLOAT3(-1.0f, 0.0f, 0.0f) },	// 23
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 24;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;

	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBuffer);

	if (FAILED(hr))
		return hr;

	// Create Plane vertex buffer
	SimpleVertex planeVertices[] =
	{
		{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT3(-1.0f, -1.0f, 0.0f),	XMFLOAT2(0.0f, 5.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, 0.0f),	XMFLOAT3(1.0f, -1.0f, 0.0f),	XMFLOAT2(5.0f, 5.0f), XMFLOAT3(0.0f, -1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, 1.0f, 0.0f),	XMFLOAT3(1.0f, 1.0f, 0.0f),		XMFLOAT2(5.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f, 1.0f, 0.0f),	XMFLOAT3(-1.0f, 1.0f, 0.0f),	XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) }
	};

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = planeVertices;

	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pPlaneVertexBuffer);

	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT DirectXInstance::InitIndexBuffer()
{
	HRESULT hr;

	// Create index buffer
	WORD indices[] =
	{
		3, 1, 0,
		2, 1, 3,

		6, 4, 5,
		7, 4, 6,

		11, 9, 8,
		10, 9, 11,

		14, 12, 13,
		15, 12, 14,

		19, 17, 16,
		18, 17, 19,

		22, 20, 21,
		23, 20, 22
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 36;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = indices;
	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBuffer);

	if (FAILED(hr))
		return hr;

	// Create plane index buffer
	WORD planeIndices[] =
	{
		0, 3, 1,
		3, 2, 1,
	};

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 6;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = planeIndices;
	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pPlaneIndexBuffer);

	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT DirectXInstance::InitConstantBuffers()
{
	HRESULT hr;

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);

	if (FAILED(hr))
		return hr;

	// Create the shadow map constant buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SMConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pSMConstantBuffer);

	if (FAILED(hr))
		return hr;

	if (FAILED(hr))
		return hr;
}

HRESULT DirectXInstance::InitRasterizerState()
{
	HRESULT hr;

	// Rasterizer
	D3D11_RASTERIZER_DESC cmdesc;

	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_BACK;
	hr = _pd3dDevice->CreateRasterizerState(&cmdesc, &_pRSCullMode);

	if (FAILED(hr))
		return hr;

	_pImmediateContext->RSSetState(_pRSCullMode);

	D3D11_DEPTH_STENCIL_DESC dssDesc;
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	hr = _pd3dDevice->CreateDepthStencilState(&dssDesc, &_pDSLessEqual);

	return hr;
}

void DirectXInstance::Cleanup()
{
	if (_pSwapChain) _pSwapChain->Release();
	if (_pImmediateContext) _pImmediateContext->ClearState();
	if (_pImmediateContext) _pImmediateContext->Release();
	if (_pd3dDevice)  _pd3dDevice->Release();

	if (_pDSLessEqual) _pDSLessEqual->Release();
	if (_pRSCullMode) _pRSCullMode->Release();

	if (_pVertexBuffer) _pVertexBuffer->Release();
	if (_pIndexBuffer) _pIndexBuffer->Release();

	if (_pPlaneVertexBuffer) _pPlaneVertexBuffer->Release();
	if (_pPlaneIndexBuffer) _pPlaneIndexBuffer->Release();

	if (_pConstantBuffer) _pConstantBuffer->Release();
	if (_pSMConstantBuffer) _pSMConstantBuffer->Release();
}