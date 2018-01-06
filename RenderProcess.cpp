#include "RenderProcess.h"


RenderProcess::RenderProcess(float renderWidth, float renderHeight)
{
	ZeroMemory(&mRenderToTextureDesc, sizeof(mRenderToTextureDesc));
	mRenderToTextureDesc.Width = renderWidth;
	mRenderToTextureDesc.Height = renderHeight;
	mRenderToTextureDesc.MipLevels = 1;
	mRenderToTextureDesc.ArraySize = 1;
	mRenderToTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	mRenderToTextureDesc.SampleDesc.Count = 1;
	mRenderToTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	mRenderToTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	mRenderToTextureDesc.CPUAccessFlags = 0;
	mRenderToTextureDesc.MiscFlags = 0;

	ZeroMemory(&mRenderTargetViewDesc, sizeof(mRenderTargetViewDesc));
	mRenderTargetViewDesc.Format = mRenderToTextureDesc.Format;
	mRenderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	mRenderTargetViewDesc.Texture2D.MipSlice = 0;

	nullShaderResourceView = nullptr;

	float ClearWhiteColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	mCurrentShaderIndex = 0;
	mRenderTargetViewCount = 0;
	mRenderTargetViewLimit = 8;
}


RenderProcess::~RenderProcess()
{
}


HRESULT RenderProcess::LoadShader(ID3D11Device* d3dDevice, WCHAR* szVSFileName, WCHAR* szPSFileName, vector<D3D11_INPUT_ELEMENT_DESC> layoutDescVec)
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

	mVertexShaders.push_back(vertexShader);

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

	mPixelShaders.push_back(pixelShader);

	ZeroMemory(&inputLayout, sizeof(inputLayout));

	UINT numElements = layoutDescVec.size();

	D3D11_INPUT_ELEMENT_DESC* layout = layoutDescVec.data();

	hr = d3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &inputLayout);
	pVSBlob->Release();

	mInputLayouts.push_back(inputLayout);

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

	if (mRenderTargetViewCount < mRenderTargetViewLimit)
	{
		ID3D11Texture2D* _pDepthStencilBuffer = nullptr;

		ZeroMemory(&mDepthStencilTextDesc, sizeof(mDepthStencilTextDesc));
		mDepthStencilTextDesc.Width = mRenderToTextureDesc.Width;
		mDepthStencilTextDesc.Height = mRenderToTextureDesc.Height;
		mDepthStencilTextDesc.MipLevels = 1;
		mDepthStencilTextDesc.ArraySize = 1;
		mDepthStencilTextDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		mDepthStencilTextDesc.SampleDesc.Count = 1;
		mDepthStencilTextDesc.SampleDesc.Quality = 0;
		mDepthStencilTextDesc.Usage = D3D11_USAGE_DEFAULT;
		mDepthStencilTextDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		mDepthStencilTextDesc.CPUAccessFlags = 0;
		mDepthStencilTextDesc.MiscFlags = 0;

		hr = d3dDevice->CreateTexture2D(&mDepthStencilTextDesc, nullptr, &_pDepthStencilBuffer);

		if (FAILED(hr))
			return hr;

		// Create the depth stencil view desc
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;

		ZeroMemory(&mDepthStencilView, sizeof(mDepthStencilView));

		hr = d3dDevice->CreateDepthStencilView(_pDepthStencilBuffer, &descDSV, &mDepthStencilView);

		if (FAILED(hr))
			return hr;

		// Create shader resource view desc
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = mDepthStencilTextDesc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;

		hr = d3dDevice->CreateShaderResourceView(_pDepthStencilBuffer, &srvDesc, &mDepthMapResourceView);

		ZeroMemory(&mRenderToTexture, sizeof(mRenderToTexture));

		hr = d3dDevice->CreateTexture2D(&mRenderToTextureDesc, NULL, &mRenderToTexture);

		if (FAILED(hr))
			return hr;

		ID3D11RenderTargetView* renderTargetView;

		hr = d3dDevice->CreateRenderTargetView(mRenderToTexture, &mRenderTargetViewDesc, &renderTargetView);

		mRenderTargetViews[mRenderTargetViewCount] = renderTargetView;

		if (FAILED(hr))
			return hr;

		mRenderTargetViewCount += 1;
	}

	return hr;
}

HRESULT RenderProcess::SetupBackBufferRTV(ID3D11Device* d3dDevice, ID3D11Texture2D* backBuffer)
{
	HRESULT hr;

	ID3D11Texture2D* _pDepthStencilBuffer = nullptr;

	ZeroMemory(&mDepthStencilTextDesc, sizeof(mDepthStencilTextDesc));
	mDepthStencilTextDesc.Width = mRenderToTextureDesc.Width;
	mDepthStencilTextDesc.Height = mRenderToTextureDesc.Height;
	mDepthStencilTextDesc.MipLevels = 1;
	mDepthStencilTextDesc.ArraySize = 1;
	mDepthStencilTextDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	mDepthStencilTextDesc.SampleDesc.Count = 1;
	mDepthStencilTextDesc.SampleDesc.Quality = 0;
	mDepthStencilTextDesc.Usage = D3D11_USAGE_DEFAULT;
	mDepthStencilTextDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	mDepthStencilTextDesc.CPUAccessFlags = 0;
	mDepthStencilTextDesc.MiscFlags = 0;

	hr = d3dDevice->CreateTexture2D(&mDepthStencilTextDesc, nullptr, &_pDepthStencilBuffer);

	if (FAILED(hr))
		return hr;

	hr = d3dDevice->CreateDepthStencilView(_pDepthStencilBuffer, nullptr, &mDepthStencilView);

	if (FAILED(hr))
		return hr;

	ID3D11RenderTargetView* renderTargetView;

	hr = d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);

	mRenderTargetViews[mRenderTargetViewCount] = renderTargetView;

	mRenderTargetViewCount += 1;

	if (FAILED(hr))
		return hr;

	return hr;
}

void RenderProcess::SetClearColour(float r, float g, float b, float a)
{
	mClearColour[0] = r;
	mClearColour[1] = g;
	mClearColour[2] = b;
	mClearColour[3] = a;
}

void RenderProcess::AddSamplerState(ID3D11SamplerState* samplerState)
{
	mSamplerStates.push_back(samplerState);
}

void RenderProcess::AddShaderResource(ID3D11ShaderResourceView* shaderResource)
{
	mShaderResources.push_back(shaderResource);
}

void RenderProcess::SetSamplerStates(ID3D11DeviceContext* immediateContext)
{
	for (int i = 0; i < mSamplerStates.size(); i++)
	{
		immediateContext->PSSetSamplers(i, 1, &mSamplerStates.at(i));
	}
}

void RenderProcess::SetShaderResources(ID3D11DeviceContext* immediateContext)
{
	for (int i = 0; i < mShaderResources.size(); i++)
	{
		immediateContext->PSSetShaderResources(i, 1, &mShaderResources.at(i));
	}
}

void RenderProcess::NullifyShaderResources(ID3D11DeviceContext* immediateContext)
{
	for (int i = 0; i < mShaderResources.size(); i++)
	{
		immediateContext->PSSetShaderResources(i, 1, &nullShaderResourceView);
	}
}

void RenderProcess::SetupRenderProcess(ID3D11DeviceContext* immediateContext, ID3D11Buffer* constantBuffer, bool needDepthView)
{
	// Switch Render Target back to the H Blur Render Target View
	if (mRenderTargetViewCount == 1)
	{
		if (needDepthView)
		{
			immediateContext->OMSetRenderTargets(1, &mRenderTargetViews[0], mDepthStencilView);
			immediateContext->ClearRenderTargetView(mRenderTargetViews[0], mClearColour);
			immediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		}
		else
		{
			immediateContext->OMSetRenderTargets(1, &mRenderTargetViews[0], nullptr);
			immediateContext->ClearRenderTargetView(mRenderTargetViews[0], mClearColour);
		}
	}
	else
	{
		if (needDepthView)
		{
			immediateContext->OMSetRenderTargets(mRenderTargetViewCount, mRenderTargetViews, mDepthStencilView);

			for (int i = 0; i < mRenderTargetViewCount; i++)
			{
				immediateContext->ClearRenderTargetView(mRenderTargetViews[i], mClearColour);
			}

			immediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		}
		else
		{
			immediateContext->OMSetRenderTargets(mRenderTargetViewCount, mRenderTargetViews, nullptr);

			for (int i = 0; i < mRenderTargetViewCount; i++)
			{
				immediateContext->ClearRenderTargetView(mRenderTargetViews[i], mClearColour);
			}
		}
	}

	immediateContext->IASetInputLayout(mInputLayouts.at(mCurrentShaderIndex));

	immediateContext->VSSetShader(mVertexShaders.at(mCurrentShaderIndex), nullptr, 0);
	immediateContext->PSSetShader(mPixelShaders.at(mCurrentShaderIndex), nullptr, 0);

	immediateContext->VSSetConstantBuffers(0, 1, &constantBuffer);
	immediateContext->PSSetConstantBuffers(0, 1, &constantBuffer);

	SetShaderResources(immediateContext);
	SetSamplerStates(immediateContext);
}

void RenderProcess::RenderGameObjects(ID3D11DeviceContext* immediateContext, vector<GameObject*> gameObjects, ID3D11Buffer* constantBuffer, ConstantBuffer* cb)
{
	int shaderResourceIndex = mShaderResources.size();

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

		shaderResourceIndex = mShaderResources.size();
	}

	NullifyShaderResources(immediateContext);
}