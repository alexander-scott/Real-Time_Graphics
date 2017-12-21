#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "DDSTextureLoader.h"
#include "resource.h"
#include "CameraMouse.h"

#include <vector>

#include "Structures.h"
#include "RenderProcess.h"

class RenderToTextureProcess : public RenderProcess
{
	struct ResourceView
	{
		ResourceView(string resourceName, ID3D11ShaderResourceView* newShaderResourceView)
		{
			Name = resourceName;
			ShaderResourceView = newShaderResourceView;
		}

		string Name;
		ID3D11ShaderResourceView* ShaderResourceView;
	};

public:
	RenderToTextureProcess(float renderWidth, float renderHeight);
	~RenderToTextureProcess();

	void RemoveShaderResources();

	HRESULT SetupRTVAndSRV(ID3D11Device* d3dDevice, string resourceName);
	HRESULT SetupRenderQuad(ID3D11Device* d3dDevice);

	void RenderToTexture(ID3D11DeviceContext* immediateContext, ID3D11Buffer* constantBuffer, ConstantBuffer* cb);

	void RenderSceneDepthMap(ID3D11DeviceContext* immediateContext, vector<GameObject*> gameObjects, ID3D11Buffer* constantBuffer, SMConstantBuffer* sMCB);

#pragma region Get/Set Functions

	ID3D11ShaderResourceView* GetShaderTargetTexture(string resourceName);

#pragma endregion

protected:
	D3D11_SHADER_RESOURCE_VIEW_DESC _pShaderResourceViewDesc;

	ID3D11Buffer* _pQuadIndexBuffer;
	ID3D11Buffer* _pQuadVertexBuffer;

	UINT _pQuadStride;
	UINT _pQuadOffset;
	UINT _pQuadIndexCount;

private:
	vector<ResourceView*> _pShaderResourceViews;
};

