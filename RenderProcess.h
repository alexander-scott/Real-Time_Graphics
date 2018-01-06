#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"

#include <vector>

#include "Structures.h"
#include "DDSTextureLoader.h"
#include "GameObject.h"
#include "SceneLight.h"

class RenderProcess
{
public:
	RenderProcess(float renderWidth, float renderHeight);
	~RenderProcess();

	HRESULT LoadShader(ID3D11Device* d3dDevice, WCHAR* szVSFileName, WCHAR* szPSFileName, vector<D3D11_INPUT_ELEMENT_DESC> layoutDescVec);
	HRESULT SetupBackBufferRTV(ID3D11Device* d3dDevice, ID3D11Texture2D* backBuffer);

	void AddSamplerState(ID3D11SamplerState* samplerState);
	void AddShaderResource(ID3D11ShaderResourceView* shaderResource);

	void SetupRenderProcess(ID3D11DeviceContext* immediateContext, ID3D11Buffer* constantBufferbool, bool needDepthView);

	void RenderGameObjects(ID3D11DeviceContext* immediateContext, vector<GameObject*> gameObjects, ID3D11Buffer* constantBuffer, ConstantBuffer* cb);

	void SetClearColour(float r, float g, float b, float a);

	ID3D11ShaderResourceView* GetDepthMapResourceView() { return mDepthMapResourceView; }

protected:
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

	virtual HRESULT SetupRTV(ID3D11Device* d3dDevice);

	void SetSamplerStates(ID3D11DeviceContext* immediateContext);
	void SetShaderResources(ID3D11DeviceContext* immediateContext);

	void NullifyShaderResources(ID3D11DeviceContext* immediateContext);

	float									mClearColour[4];

	int										mCurrentShaderIndex;

	D3D11_TEXTURE2D_DESC					mRenderToTextureDesc;
	D3D11_RENDER_TARGET_VIEW_DESC			mRenderTargetViewDesc;
	D3D11_TEXTURE2D_DESC					mDepthStencilTextDesc;

	ID3D11Texture2D*						mRenderToTexture;
	ID3D11RenderTargetView*					mRenderTargetViews[8];
	int										mRenderTargetViewCount;
	int										mRenderTargetViewLimit;

	ID3D11DepthStencilView*					mDepthStencilView;

	vector<ID3D11VertexShader*>				mVertexShaders;
	vector<ID3D11PixelShader*>				mPixelShaders;

	vector<ID3D11InputLayout*>				mInputLayouts;

	vector<ID3D11SamplerState*>				mSamplerStates;
	vector<ID3D11ShaderResourceView*>		mShaderResources;

	ID3D11ShaderResourceView*				mDepthMapResourceView;

	ID3D11ShaderResourceView*				nullShaderResourceView;
};

