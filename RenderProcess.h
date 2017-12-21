#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"

#include <vector>

#include "Structures.h"
#include "CameraMouse.h"
#include "DDSTextureLoader.h"
#include "GameObject.h"
#include "SceneLight.h"

class RenderProcess
{
public:
	RenderProcess(float renderWidth, float renderHeight);
	~RenderProcess();

	HRESULT LoadShaderFilesAndInputLayouts(ID3D11Device* d3dDevice, WCHAR* szVSFileName, WCHAR* szPSFileName, vector<D3D11_INPUT_ELEMENT_DESC> layoutDescVec);
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

	virtual HRESULT RenderProcess::SetupRTV(ID3D11Device* d3dDevice);
	HRESULT RenderProcess::SetupBackBufferRTV(ID3D11Device* d3dDevice, ID3D11Texture2D* backBuffer);

	void AddSamplerState(ID3D11SamplerState* samplerState);
	void AddShaderResource(ID3D11ShaderResourceView* shaderResource);

	void SetupRenderProcess(ID3D11DeviceContext* immediateContext, ID3D11Buffer* constantBufferbool, bool needDepthView);

	void RenderGameObjects(ID3D11DeviceContext* immediateContext, vector<GameObject*> gameObjects, ID3D11Buffer* constantBuffer, ConstantBuffer* cb);

#pragma region Get/Set Functions

	vector<ID3D11ShaderResourceView*> GetShaderResources() { return _pShaderResources; }
	void SetClearColour(float r, float g, float b, float a);
	void SetCurrentShaderIndex(int newShaderIndex) { currentShaderIndex = newShaderIndex; }

	D3D11_TEXTURE2D_DESC GetRenderToTextureDesc() { return _pRenderToTextureDesc; }

	ID3D11ShaderResourceView* GetDepthMapResourceView() { return _pDepthMapResourceView; }

#pragma endregion


protected:
	void SetSamplerStates(ID3D11DeviceContext* immediateContext);
	void SetShaderResources(ID3D11DeviceContext* immediateContext);

	void NullifyShaderResources(ID3D11DeviceContext* immediateContext);

protected:
	float _pClearColour[4];

	int currentShaderIndex;

	D3D11_TEXTURE2D_DESC _pRenderToTextureDesc;
	D3D11_RENDER_TARGET_VIEW_DESC _pRenderTargetViewDesc;
	D3D11_TEXTURE2D_DESC _pDepthStencilTextDesc;

	ID3D11Texture2D* _pRenderToTexture;
	ID3D11RenderTargetView* _pRenderTargetViews[8];
	int renderTargetViewCount;
	int renderTargetViewLimit;

	ID3D11DepthStencilView* _pDepthStencilView;

	vector<ID3D11VertexShader*> _pVertexShaders;
	vector<ID3D11PixelShader*> _pPixelShaders;

	vector<ID3D11InputLayout*> _pInputLayouts;

	vector<ID3D11SamplerState*> _pSamplerStates;
	vector<ID3D11ShaderResourceView*> _pShaderResources;

	ID3D11ShaderResourceView* _pDepthMapResourceView;

	ID3D11ShaderResourceView* nullShaderResourceView;
};

