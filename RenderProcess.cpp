#include "RenderProcess.h"


RenderProcess::RenderProcess(float renderWidth, float renderHeight)
{
	ZeroMemory(&_pRenderToTextureDesc, sizeof(_pRenderToTextureDesc));
	_pRenderToTextureDesc.Width = renderWidth;
	_pRenderToTextureDesc.Height = renderHeight;
	_pRenderToTextureDesc.MipLevels = 1;
	_pRenderToTextureDesc.ArraySize = 1;
	_pRenderToTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	_pRenderToTextureDesc.SampleDesc.Count = 1;
	_pRenderToTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	_pRenderToTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	_pRenderToTextureDesc.CPUAccessFlags = 0;
	_pRenderToTextureDesc.MiscFlags = 0;

	ZeroMemory(&_pRenderTargetViewDesc, sizeof(_pRenderTargetViewDesc));
	_pRenderTargetViewDesc.Format = _pRenderToTextureDesc.Format;
	_pRenderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	_pRenderTargetViewDesc.Texture2D.MipSlice = 0;

	nullShaderResourceView = nullptr;

	float ClearWhiteColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	currentShaderIndex = 0;
	renderTargetViewCount = 0;
	renderTargetViewLimit = 8;
}


RenderProcess::~RenderProcess()
{
}


HRESULT RenderProcess::LoadShaderFilesAndInputLayouts(ID3D11Device* d3dDevice, WCHAR* szVSFileName, WCHAR* szPSFileName, vector<D3D11_INPUT_ELEMENT_DESC> layoutDescVec)
{
	HRESULT hr;

	ID3DBlob* pVSBlob = nullptr;
	ID3DBlob* pPSBlob = nullptr;

	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11PixelShader* pixelShader = nullptr;

	ID3D11InputLayout* inputLayout;

	// Compile the vertex shader
	hr = CompileShaderFromFile(szVSFileName, "VS", "vs_4_0", &pVSBlob);

	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = d3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &vertexShader);

	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	_pVertexShaders.push_back(vertexShader);

	// Compile the pixel shader
	hr = CompileShaderFromFile(szVSFileName, "PS", "ps_4_0", &pPSBlob);

	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = d3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &pixelShader);
	pPSBlob->Release();

	if (FAILED(hr))
		return hr;

	_pPixelShaders.push_back(pixelShader);

#pragma region Define Input Layout

	ZeroMemory(&inputLayout, sizeof(inputLayout));

	UINT numElements = layoutDescVec.size();

	D3D11_INPUT_ELEMENT_DESC* layout = layoutDescVec.data();

	hr = d3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &inputLayout);
	pVSBlob->Release();

	_pInputLayouts.push_back(inputLayout);

#pragma endregion

	if (FAILED(hr))
		return hr;

	return hr;
}

HRESULT RenderProcess::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

	if (FAILED(hr))
	{
		if (pErrorBlob != nullptr)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

		if (pErrorBlob) pErrorBlob->Release();

		return hr;
	}

	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

HRESULT RenderProcess::SetupRTV(ID3D11Device* d3dDevice)
{
	HRESULT hr;

	if (renderTargetViewCount < renderTargetViewLimit)
	{
		ID3D11Texture2D* _pDepthStencilBuffer = nullptr;

		ZeroMemory(&_pDepthStencilTextDesc, sizeof(_pDepthStencilTextDesc));
		_pDepthStencilTextDesc.Width = _pRenderToTextureDesc.Width;
		_pDepthStencilTextDesc.Height = _pRenderToTextureDesc.Height;
		_pDepthStencilTextDesc.MipLevels = 1;
		_pDepthStencilTextDesc.ArraySize = 1;
		_pDepthStencilTextDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		_pDepthStencilTextDesc.SampleDesc.Count = 1;
		_pDepthStencilTextDesc.SampleDesc.Quality = 0;
		_pDepthStencilTextDesc.Usage = D3D11_USAGE_DEFAULT;
		_pDepthStencilTextDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		_pDepthStencilTextDesc.CPUAccessFlags = 0;
		_pDepthStencilTextDesc.MiscFlags = 0;

		hr = d3dDevice->CreateTexture2D(&_pDepthStencilTextDesc, nullptr, &_pDepthStencilBuffer);

		if (FAILED(hr))
			return hr;

		// Create the depth stencil view desc
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;

		ZeroMemory(&_pDepthStencilView, sizeof(_pDepthStencilView));

		hr = d3dDevice->CreateDepthStencilView(_pDepthStencilBuffer, &descDSV, &_pDepthStencilView);

		if (FAILED(hr))
			return hr;

		// Create shader resource view desc
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = _pDepthStencilTextDesc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;

		hr = d3dDevice->CreateShaderResourceView(_pDepthStencilBuffer, &srvDesc, &_pDepthMapResourceView);

		ZeroMemory(&_pRenderToTexture, sizeof(_pRenderToTexture));

		hr = d3dDevice->CreateTexture2D(&_pRenderToTextureDesc, NULL, &_pRenderToTexture);

		if (FAILED(hr))
			return hr;

		ID3D11RenderTargetView* renderTargetView;

		hr = d3dDevice->CreateRenderTargetView(_pRenderToTexture, &_pRenderTargetViewDesc, &renderTargetView);

		_pRenderTargetViews[renderTargetViewCount] = renderTargetView;

		if (FAILED(hr))
			return hr;

		renderTargetViewCount += 1;
	}

	return hr;
}

HRESULT RenderProcess::SetupBackBufferRTV(ID3D11Device* d3dDevice, ID3D11Texture2D* backBuffer)
{
	HRESULT hr;

	ID3D11Texture2D* _pDepthStencilBuffer = nullptr;

	ZeroMemory(&_pDepthStencilTextDesc, sizeof(_pDepthStencilTextDesc));
	_pDepthStencilTextDesc.Width = _pRenderToTextureDesc.Width;
	_pDepthStencilTextDesc.Height = _pRenderToTextureDesc.Height;
	_pDepthStencilTextDesc.MipLevels = 1;
	_pDepthStencilTextDesc.ArraySize = 1;
	_pDepthStencilTextDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	_pDepthStencilTextDesc.SampleDesc.Count = 1;
	_pDepthStencilTextDesc.SampleDesc.Quality = 0;
	_pDepthStencilTextDesc.Usage = D3D11_USAGE_DEFAULT;
	_pDepthStencilTextDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	_pDepthStencilTextDesc.CPUAccessFlags = 0;
	_pDepthStencilTextDesc.MiscFlags = 0;

	hr = d3dDevice->CreateTexture2D(&_pDepthStencilTextDesc, nullptr, &_pDepthStencilBuffer);

	if (FAILED(hr))
		return hr;

	hr = d3dDevice->CreateDepthStencilView(_pDepthStencilBuffer, nullptr, &_pDepthStencilView);

	if (FAILED(hr))
		return hr;

	ID3D11RenderTargetView* renderTargetView;

	hr = d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);

	_pRenderTargetViews[renderTargetViewCount] = renderTargetView;

	renderTargetViewCount += 1;

	if (FAILED(hr))
		return hr;

	return hr;
}

void RenderProcess::SetClearColour(float r, float g, float b, float a)
{
	_pClearColour[0] = r;
	_pClearColour[1] = g;
	_pClearColour[2] = b;
	_pClearColour[3] = a;
}

void RenderProcess::AddSamplerState(ID3D11SamplerState* samplerState)
{
	_pSamplerStates.push_back(samplerState);
}

void RenderProcess::AddShaderResource(ID3D11ShaderResourceView* shaderResource)
{
	_pShaderResources.push_back(shaderResource);
}

void RenderProcess::SetSamplerStates(ID3D11DeviceContext* immediateContext)
{
	for (int i = 0; i < _pSamplerStates.size(); i++)
	{
		immediateContext->PSSetSamplers(i, 1, &_pSamplerStates.at(i));
	}
}

void RenderProcess::SetShaderResources(ID3D11DeviceContext* immediateContext)
{
	for (int i = 0; i < _pShaderResources.size(); i++)
	{
		immediateContext->PSSetShaderResources(i, 1, &_pShaderResources.at(i));
	}
}

void RenderProcess::NullifyShaderResources(ID3D11DeviceContext* immediateContext)
{
	for (int i = 0; i < _pShaderResources.size(); i++)
	{
		immediateContext->PSSetShaderResources(i, 1, &nullShaderResourceView);
	}
}

void RenderProcess::SetupRenderProcess(ID3D11DeviceContext* immediateContext, ID3D11Buffer* constantBuffer, bool needDepthView)
{
	// Switch Render Target back to the H Blur Render Target View
	if (renderTargetViewCount == 1)
	{
		if (needDepthView)
		{
			immediateContext->OMSetRenderTargets(1, &_pRenderTargetViews[0], _pDepthStencilView);
			immediateContext->ClearRenderTargetView(_pRenderTargetViews[0], _pClearColour);
			immediateContext->ClearDepthStencilView(_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		}
		else
		{
			immediateContext->OMSetRenderTargets(1, &_pRenderTargetViews[0], nullptr);
			immediateContext->ClearRenderTargetView(_pRenderTargetViews[0], _pClearColour);
		}
	}
	else
	{
		if (needDepthView)
		{
			immediateContext->OMSetRenderTargets(renderTargetViewCount, _pRenderTargetViews, _pDepthStencilView);

			for (int i = 0; i < renderTargetViewCount; i++)
			{
				immediateContext->ClearRenderTargetView(_pRenderTargetViews[i], _pClearColour);
			}

			immediateContext->ClearDepthStencilView(_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		}
		else
		{
			immediateContext->OMSetRenderTargets(renderTargetViewCount, _pRenderTargetViews, nullptr);

			for (int i = 0; i < renderTargetViewCount; i++)
			{
				immediateContext->ClearRenderTargetView(_pRenderTargetViews[i], _pClearColour);
			}
		}
	}

	immediateContext->IASetInputLayout(_pInputLayouts.at(currentShaderIndex));

	immediateContext->VSSetShader(_pVertexShaders.at(currentShaderIndex), nullptr, 0);
	immediateContext->PSSetShader(_pPixelShaders.at(currentShaderIndex), nullptr, 0);

	immediateContext->VSSetConstantBuffers(0, 1, &constantBuffer);
	immediateContext->PSSetConstantBuffers(0, 1, &constantBuffer);

	SetShaderResources(immediateContext);
	SetSamplerStates(immediateContext);
}

void RenderProcess::RenderGameObjects(ID3D11DeviceContext* immediateContext, vector<GameObject*> gameObjects, ID3D11Buffer* constantBuffer, ConstantBuffer* cb)
{
	int shaderResourceIndex = _pShaderResources.size();

	// Render all scene objects
	for (auto gameObject : gameObjects)
	{
		// Get render material
		Material material = gameObject->GetMaterial();

		// Copy material to shader
		cb->surface.AmbientMtrl = material.ambient;
		cb->surface.DiffuseMtrl = material.diffuse;
		cb->surface.SpecularMtrl = material.specular;

		// Set world matrix
		cb->World = XMMatrixTranspose(gameObject->GetWorldMatrix());

		// Set texture
		if (gameObject->HasTexture())
		{
			cb->HasTexture = 1.0f;

			ID3D11ShaderResourceView* textureRV = gameObject->GetTextureRV();
			immediateContext->PSSetShaderResources(shaderResourceIndex, 1, &textureRV);
			shaderResourceIndex += 1;

			if (gameObject->HasNormalMap())
			{
				cb->HasNormalMap = 1.0f;

				ID3D11ShaderResourceView* normalMap = gameObject->GetNormalMap();
				immediateContext->PSSetShaderResources(shaderResourceIndex, 1, &normalMap);
				shaderResourceIndex += 1;

				if (gameObject->HasHeightMap())
				{
					cb->HasHeightMap = 1.0f;

					ID3D11ShaderResourceView* heightMap = gameObject->GetHeightMap();
					immediateContext->PSSetShaderResources(shaderResourceIndex, 1, &heightMap);
					shaderResourceIndex += 1;
				}
				else
				{
					cb->HasHeightMap = 0.0f;
				}
			}
			else
			{
				cb->HasHeightMap = 0.0f;
				cb->HasNormalMap = 0.0f;
			}
		}
		else
		{
			cb->HasTexture = 0.0f;
		}

		// Update constant buffer
		immediateContext->UpdateSubresource(constantBuffer, 0, nullptr, cb, 0, 0);

		// Draw object
		gameObject->Draw(immediateContext);

		shaderResourceIndex = _pShaderResources.size();
	}

	NullifyShaderResources(immediateContext);
}