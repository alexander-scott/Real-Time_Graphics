#include "ShaderController.h"

ShaderController::ShaderController()
{
	mCurrentSceneRenderProcess = nullptr;
}

ShaderController::~ShaderController()
{
}

HRESULT ShaderController::SetupShader(string shaderName, string rtvName, float renderWidth, float renderHeight, WCHAR* szFileName, vector<D3D11_INPUT_ELEMENT_DESC> layoutDescVec, ID3D11Device* d3dDevice)
{
	auto renderProcess = std::make_unique<RenderToTextureProcess>(renderWidth, renderHeight);

	if (FAILED(renderProcess->LoadShaderFilesAndInputLayouts(d3dDevice, szFileName, szFileName, layoutDescVec)))
	{
		return E_FAIL;
	}

	if (FAILED(renderProcess->SetupRTVAndSRV(d3dDevice, rtvName)))
	{
		return E_FAIL;
	}

	mShaderList[shaderName] = std::move(renderProcess);

	return S_OK;
}

HRESULT ShaderController::AddDepthBufferShader(string shaderName, float renderWidth, float renderHeight, WCHAR* szFileName, vector<D3D11_INPUT_ELEMENT_DESC> layoutDescVec, ID3D11Device* d3dDevice)
{
	if (FAILED(SetupShader(shaderName, "DepthMap", renderWidth, renderHeight, szFileName, layoutDescVec, d3dDevice)))
	{
		return E_FAIL;
	}

	mShaderList[shaderName].get()->SetClearColour(1.0f, 1.0f, 1.0f, 1.0f);

	return S_OK;
}

HRESULT ShaderController::AddRenderFromQuadShader(string shaderName, float renderWidth, float renderHeight, WCHAR* szFileName, vector<D3D11_INPUT_ELEMENT_DESC> layoutDescVec, ID3D11Device* d3dDevice)
{
	if (FAILED(SetupShader(shaderName, "OutputText", renderWidth, renderHeight, szFileName, layoutDescVec, d3dDevice)))
	{
		return E_FAIL;
	}

	if (mShaderList[shaderName].get()->SetupRenderQuad(d3dDevice))
	{
		return E_FAIL;
	}

	mShaderList[shaderName].get()->SetClearColour(0.0f, 0.0f, 0.0f, 1.0f);

	return S_OK;
}

HRESULT ShaderController::AddRenderToBackBufferShader(string shaderName, float renderWidth, float renderHeight, WCHAR* szFileName, vector<D3D11_INPUT_ELEMENT_DESC> layoutDescVec, ID3D11Device* d3dDevice)
{
	auto renderProcess = std::make_unique<RenderToTextureProcess>(renderWidth, renderHeight);

	if (FAILED(renderProcess->LoadShaderFilesAndInputLayouts(d3dDevice, szFileName, szFileName, layoutDescVec)))
	{
		return E_FAIL;
	}

	if (renderProcess->SetupRenderQuad(d3dDevice))
	{
		return E_FAIL;
	}

	mShaderList[shaderName] = std::move(renderProcess);

	return S_OK;
}

void ShaderController::AddSceneShader(string shaderName, float renderWidth, float renderHeight)
{
	auto renderProcess = std::make_unique<RenderToTextureProcess>(renderWidth, renderHeight);
	renderProcess->SetClearColour(0.0f, 0.0f, 0.0f, 1.0f);
	mShaderList[shaderName] = std::move(renderProcess);
}

void ShaderController::Draw(ConstantBuffer* cb, vector<SceneLight*> lights, vector<GameObject*> gameObjects)
{
	cb->shadowsOn = true;
	cb->blurIntensity = GUIController::_pBlurIntensity;

	SMConstantBuffer smCB;

	XMFLOAT4X4 viewAsFloats;
	XMFLOAT4X4 projectionAsFloats;
	XMFLOAT4X4 shadowTransformAsFloats;
	XMMATRIX view;
	XMMATRIX projection;
	XMMATRIX shadowTransform;

	// Create shadow maps
	for (int i = 0; i < lights.size(); i++)
	{
		if (lights.at(i)->GetLightOn())
		{
			viewAsFloats = lights.at(i)->GetView();
			projectionAsFloats = lights.at(i)->GetProjection();
			shadowTransformAsFloats = lights.at(i)->GetShadowTransform();

			view = XMLoadFloat4x4(&viewAsFloats);
			projection = XMLoadFloat4x4(&projectionAsFloats);
			shadowTransform = XMLoadFloat4x4(&projectionAsFloats);

			smCB.View = XMMatrixTranspose(view);
			smCB.Projection = XMMatrixTranspose(projection);

			string shaderName = lights.at(i)->GetLightName() + " Depth Map";

			mShaderList[shaderName].get()->RenderSceneDepthMap(DirectXInstance::Instance()._pImmediateContext, gameObjects, DirectXInstance::Instance()._pSMConstantBuffer, &smCB);
		}
	}

	mCurrentSceneRenderProcess->SetupRenderProcess(DirectXInstance::Instance()._pImmediateContext, DirectXInstance::Instance()._pConstantBuffer, true);
	mCurrentSceneRenderProcess->RenderGameObjects(DirectXInstance::Instance()._pImmediateContext, gameObjects, DirectXInstance::Instance()._pConstantBuffer, cb);

	if (GUIController::_pBlurEffectPasses != 0)
	{
		for (int i = 0; i < GUIController::_pBlurEffectPasses; i++)
		{
			mShaderList["Effect HBlur"].get()->SetupRenderProcess(DirectXInstance::Instance()._pImmediateContext, DirectXInstance::Instance()._pConstantBuffer, false);
			mShaderList["Effect HBlur"].get()->RenderToTexture(DirectXInstance::Instance()._pImmediateContext, DirectXInstance::Instance()._pConstantBuffer, cb);

			mShaderList["Effect VBlur"].get()->SetupRenderProcess(DirectXInstance::Instance()._pImmediateContext, DirectXInstance::Instance()._pConstantBuffer, false);
			mShaderList["Effect VBlur"].get()->RenderToTexture(DirectXInstance::Instance()._pImmediateContext, DirectXInstance::Instance()._pConstantBuffer, cb);

			if (i == 0)
			{
				mShaderList["Effect HBlur"].get()->RemoveShaderResources();
				mShaderList["Effect HBlur"].get()->AddShaderResource(mShaderList["Effect VBlur"].get()->GetShaderTargetTexture("OutputText"));
			}
		}

		mShaderList["Effect HBlur"].get()->RemoveShaderResources();
		mShaderList["Effect HBlur"].get()->AddShaderResource(mCurrentSceneRenderProcess->GetShaderTargetTexture("ColourMap"));
	}
	else
	{
		mShaderList["Effect HBlur"].get()->SetupRenderProcess(DirectXInstance::Instance()._pImmediateContext, DirectXInstance::Instance()._pConstantBuffer, false);
		mShaderList["Effect HBlur"].get()->RenderToTexture(DirectXInstance::Instance()._pImmediateContext, DirectXInstance::Instance()._pConstantBuffer, cb);

		mShaderList["Effect VBlur"].get()->SetupRenderProcess(DirectXInstance::Instance()._pImmediateContext, DirectXInstance::Instance()._pConstantBuffer, false);
		mShaderList["Effect VBlur"].get()->RenderToTexture(DirectXInstance::Instance()._pImmediateContext, DirectXInstance::Instance()._pConstantBuffer, cb);
	}

	mShaderList["Final Pass"].get()->SetupRenderProcess(DirectXInstance::Instance()._pImmediateContext, DirectXInstance::Instance()._pConstantBuffer, false);
	mShaderList["Final Pass"].get()->RenderToTexture(DirectXInstance::Instance()._pImmediateContext, DirectXInstance::Instance()._pConstantBuffer, cb);
}

void ShaderController::SetShaderResources()
{
	mShaderList["Parallax Scene"].get()->RemoveShaderResources();
	mShaderList["Parallax Scene"].get()->AddShaderResource(mShaderList["White Light Depth Map"].get()->GetDepthMapResourceView());
	mShaderList["Parallax Scene"].get()->AddShaderResource(mShaderList["Red Light Depth Map"].get()->GetDepthMapResourceView());
	mShaderList["Parallax Scene"].get()->AddShaderResource(mShaderList["Green Light Depth Map"].get()->GetDepthMapResourceView());
	mShaderList["Parallax Scene"].get()->AddShaderResource(mShaderList["Blue Light Depth Map"].get()->GetDepthMapResourceView());
	mShaderList["Parallax Scene"].get()->SetCurrentShaderIndex(0);
	mCurrentSceneRenderProcess = mShaderList["Parallax Scene"].get();
	mShaderList["Effect HBlur"].get()->RemoveShaderResources();
	mShaderList["Effect HBlur"].get()->AddShaderResource(mCurrentSceneRenderProcess->GetShaderTargetTexture("ColourMap"));
	mShaderList["Final Pass"].get()->RemoveShaderResources();
	mShaderList["Final Pass"].get()->AddShaderResource(mShaderList["Effect VBlur"].get()->GetShaderTargetTexture("OutputText"));
	GUIController::ResetBlurOptions();
}