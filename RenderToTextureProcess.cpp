#include "RenderToTextureProcess.h"

RenderToTextureProcess::RenderToTextureProcess(float renderWidth, float renderHeight)
					   :RenderProcess(renderWidth, renderHeight)
{
	ZeroMemory(&_pShaderResourceViewDesc, sizeof(_pShaderResourceViewDesc));
	_pShaderResourceViewDesc.Format = _pRenderToTextureDesc.Format;
	_pShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	_pShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	_pShaderResourceViewDesc.Texture2D.MipLevels = 1;
}

RenderToTextureProcess::~RenderToTextureProcess()
{
}

HRESULT RenderToTextureProcess::SetupRenderQuad(ID3D11Device* d3dDevice)
{
	HRESULT hr;

	// Setup Quad Vertex Buffer
	QuadVertex quadVertices[] =
	{
		{ XMFLOAT3(-1.0f,	-1.0f, 0.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f,	1.0f, 0.0f),	XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f,	1.0f, 0.0f),	XMFLOAT3(2.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f,	-1.0f, 0.0f),	XMFLOAT3(3.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) }
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = quadVertices;

	hr = d3dDevice->CreateBuffer(&bd, &InitData, &_pQuadVertexBuffer);

	if (FAILED(hr))
		return hr;

	// Setup Quad Indices Buffer
	WORD indices[] =
	{
		0, 1, 2,
		0, 2, 3
	};

	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 6;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = indices;

	hr = d3dDevice->CreateBuffer(&bd, &InitData, &_pQuadIndexBuffer);

	if (FAILED(hr))
		return hr;

	_pQuadIndexCount = ARRAYSIZE(indices);

	_pQuadStride = sizeof(QuadVertex);
	_pQuadOffset = 0;

	return hr;
}

HRESULT RenderToTextureProcess::SetupRTVAndSRV(ID3D11Device* d3dDevice, string resourceName)
{
	HRESULT hr;

	hr = RenderProcess::SetupRTV(d3dDevice);

	if (FAILED(hr))
		return hr;

	ResourceView* newResourceView = new ResourceView(resourceName, nullptr);

	hr = d3dDevice->CreateShaderResourceView(_pRenderToTexture, &_pShaderResourceViewDesc, &newResourceView->ShaderResourceView);
	_pRenderToTexture->Release();

	_pShaderResourceViews.push_back(newResourceView);

	if (FAILED(hr))
		return hr;

	return hr;
}

ID3D11ShaderResourceView* RenderToTextureProcess::GetShaderTargetTexture(string resourceName)
{
	for (int i = 0; i <_pShaderResourceViews.size(); i++)
	{
		if (_pShaderResourceViews.at(i)->Name == resourceName)
		{
			return _pShaderResourceViews.at(i)->ShaderResourceView;
		}
	}

	//return nullptr;
}

void RenderToTextureProcess::RenderToTexture(ID3D11DeviceContext* immediateContext, ID3D11Buffer* constantBuffer, ConstantBuffer* cb)
{
	immediateContext->IASetVertexBuffers(0, 1, &_pQuadVertexBuffer, &_pQuadStride, &_pQuadOffset);
	immediateContext->IASetIndexBuffer(_pQuadIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	immediateContext->UpdateSubresource(constantBuffer, 0, nullptr, cb, 0, 0);

	immediateContext->DrawIndexed(6, 0, 0);

	RenderProcess::NullifyShaderResources(immediateContext);
}

void RenderToTextureProcess::RenderSceneDepthMap(ID3D11DeviceContext* immediateContext, vector<GameObject*> gameObjects, ID3D11Buffer* constantBuffer, SMConstantBuffer* sMCB)
{
	// Switch Render Target back to the H Blur Render Target View
	ID3D11RenderTargetView* renderTargets[1] = { 0 };
	immediateContext->OMSetRenderTargets(1, renderTargets, _pDepthStencilView);
	immediateContext->ClearDepthStencilView(_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	immediateContext->IASetInputLayout(_pInputLayouts.at(currentShaderIndex));

	immediateContext->VSSetShader(_pVertexShaders.at(currentShaderIndex), nullptr, 0);
	immediateContext->PSSetShader(_pPixelShaders.at(currentShaderIndex), nullptr, 0);

	immediateContext->VSSetConstantBuffers(0, 1, &constantBuffer);
	immediateContext->PSSetConstantBuffers(0, 1, &constantBuffer);

	SetShaderResources(immediateContext);
	SetSamplerStates(immediateContext);

	// Render all scene objects
	for (auto gameObject : gameObjects)
	{
		// Set world matrix
		sMCB->World = XMMatrixTranspose(gameObject->GetWorldMatrix());

		// Update constant buffer
		immediateContext->UpdateSubresource(constantBuffer, 0, nullptr, sMCB, 0, 0);

		// Draw object
		gameObject->Draw(immediateContext);
	}

	NullifyShaderResources(immediateContext);
}

void RenderToTextureProcess::RemoveShaderResources()
{
	_pShaderResources.clear();
}