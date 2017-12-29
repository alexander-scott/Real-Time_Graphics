#include "ShaderManager.h"

unordered_map<string, unique_ptr<RenderToTextureProcess>> ShaderManager::_pShaderList;
RenderToTextureProcess* ShaderManager::_pCurrentSceneRenderProcess = nullptr;

int ShaderManager::_pCurrentShaderOptionSelected = 1;
bool ShaderManager::_pDeferred = false;
bool ShaderManager::_pDOFWasOn = false;

ShaderManager::ShaderManager()
{
}

ShaderManager::~ShaderManager()
{
}

HRESULT ShaderManager::SetupShader(string shaderName, string rtvName, float renderWidth, float renderHeight, WCHAR* szFileName, vector<D3D11_INPUT_ELEMENT_DESC> layoutDescVec, ID3D11Device* d3dDevice)
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

	_pShaderList[shaderName] = std::move(renderProcess);
}

HRESULT ShaderManager::AddDepthBufferShader(string shaderName, float renderWidth, float renderHeight, WCHAR* szFileName, vector<D3D11_INPUT_ELEMENT_DESC> layoutDescVec, ID3D11Device* d3dDevice)
{
	if (FAILED(SetupShader(shaderName, "DepthMap", renderWidth, renderHeight, szFileName, layoutDescVec, d3dDevice)))
	{
		return E_FAIL;
	}

	_pShaderList[shaderName].get()->SetClearColour(1.0f, 1.0f, 1.0f, 1.0f);

	return S_OK;
}

HRESULT ShaderManager::AddCommonShader(string shaderName, float renderWidth, float renderHeight, WCHAR* szFileName, vector<D3D11_INPUT_ELEMENT_DESC> layoutDescVec, ID3D11Device* d3dDevice)
{
	if (FAILED(SetupShader(shaderName, "OutputText", renderWidth, renderHeight, szFileName, layoutDescVec, d3dDevice)))
	{
		return E_FAIL;
	}

	_pShaderList[shaderName].get()->SetClearColour(0.0f, 0.0f, 0.0f, 1.0f);

	return S_OK;
}

HRESULT ShaderManager::AddRenderFromQuadShader(string shaderName, float renderWidth, float renderHeight, WCHAR* szFileName, vector<D3D11_INPUT_ELEMENT_DESC> layoutDescVec, ID3D11Device* d3dDevice)
{
	if (FAILED(SetupShader(shaderName, "OutputText", renderWidth, renderHeight, szFileName, layoutDescVec, d3dDevice)))
	{
		return E_FAIL;
	}

	if (_pShaderList[shaderName].get()->SetupRenderQuad(d3dDevice))
	{
		return E_FAIL;
	}

	_pShaderList[shaderName].get()->SetClearColour(0.0f, 0.0f, 0.0f, 1.0f);

	return S_OK;
}

HRESULT ShaderManager::AddRenderToBackBufferShader(string shaderName, float renderWidth, float renderHeight, WCHAR* szFileName, vector<D3D11_INPUT_ELEMENT_DESC> layoutDescVec, ID3D11Device* d3dDevice)
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

	_pShaderList[shaderName] = std::move(renderProcess);

	return S_OK;
}

void ShaderManager::AddCustomShader(string shaderName, float renderWidth, float renderHeight)
{
	auto renderProcess = std::make_unique<RenderToTextureProcess>(renderWidth, renderHeight);
	renderProcess->SetClearColour(0.0f, 0.0f, 0.0f, 1.0f);
	_pShaderList[shaderName] = std::move(renderProcess);
}

void ShaderManager::ExecuteShadersInOrder(ConstantBuffer* cb, vector<SceneLight*> lights, vector<GameObject*> gameObjects)
{
	cb->shadowsOn = GUIHandler::_pShadowMappingOn;
	cb->blurIntensity = GUIHandler::_pBlurIntensity;

	SMConstantBuffer smCB;

	XMFLOAT4X4 viewAsFloats;
	XMFLOAT4X4 projectionAsFloats;
	XMFLOAT4X4 shadowTransformAsFloats;
	XMMATRIX view;
	XMMATRIX projection;
	XMMATRIX shadowTransform;

	if (GUIHandler::_pShadowMappingOn)
	{
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

				_pShaderList[shaderName].get()->RenderSceneDepthMap(DX11AppHelper::_pImmediateContext, gameObjects, DX11AppHelper::_pSMConstantBuffer, &smCB);
			}
		}
	}

	_pCurrentSceneRenderProcess->SetupRenderProcess(DX11AppHelper::_pImmediateContext, DX11AppHelper::_pConstantBuffer, true);
	_pCurrentSceneRenderProcess->RenderGameObjects(DX11AppHelper::_pImmediateContext, gameObjects, DX11AppHelper::_pConstantBuffer, cb);

	if (_pDeferred)
	{
		_pShaderList["Deferred Parrallax Scene"].get()->SetupRenderProcess(DX11AppHelper::_pImmediateContext, DX11AppHelper::_pConstantBuffer, false);
		_pShaderList["Deferred Parrallax Scene"].get()->RenderToTexture(DX11AppHelper::_pImmediateContext, DX11AppHelper::_pConstantBuffer, cb);
	}

	if (GUIHandler::_pBlurEffectPasses != 0)
	{
		for (int i = 0; i < GUIHandler::_pBlurEffectPasses; i++)
		{
			_pShaderList["Effect HBlur"].get()->SetupRenderProcess(DX11AppHelper::_pImmediateContext, DX11AppHelper::_pConstantBuffer, false);
			_pShaderList["Effect HBlur"].get()->RenderToTexture(DX11AppHelper::_pImmediateContext, DX11AppHelper::_pConstantBuffer, cb);

			_pShaderList["Effect VBlur"].get()->SetupRenderProcess(DX11AppHelper::_pImmediateContext, DX11AppHelper::_pConstantBuffer, false);
			_pShaderList["Effect VBlur"].get()->RenderToTexture(DX11AppHelper::_pImmediateContext, DX11AppHelper::_pConstantBuffer, cb);

			if (i == 0)
			{
				_pShaderList["Effect HBlur"].get()->RemoveShaderResources();
				_pShaderList["Effect HBlur"].get()->AddShaderResource(_pShaderList["Effect VBlur"].get()->GetShaderTargetTexture("OutputText"));
			}
		}

		_pShaderList["Effect HBlur"].get()->RemoveShaderResources();
		_pShaderList["Effect HBlur"].get()->AddShaderResource(_pCurrentSceneRenderProcess->GetShaderTargetTexture("ColourMap"));
	}
	else
	{
		_pShaderList["Effect HBlur"].get()->SetupRenderProcess(DX11AppHelper::_pImmediateContext, DX11AppHelper::_pConstantBuffer, false);
		_pShaderList["Effect HBlur"].get()->RenderToTexture(DX11AppHelper::_pImmediateContext, DX11AppHelper::_pConstantBuffer, cb);

		_pShaderList["Effect VBlur"].get()->SetupRenderProcess(DX11AppHelper::_pImmediateContext, DX11AppHelper::_pConstantBuffer, false);
		_pShaderList["Effect VBlur"].get()->RenderToTexture(DX11AppHelper::_pImmediateContext, DX11AppHelper::_pConstantBuffer, cb);
	}

	_pShaderList["DOF Depth Map"].get()->SetupRenderProcess(DX11AppHelper::_pImmediateContext, DX11AppHelper::_pConstantBuffer, true);
	_pShaderList["DOF Depth Map"].get()->RenderGameObjects(DX11AppHelper::_pImmediateContext, gameObjects, DX11AppHelper::_pConstantBuffer, cb);

	if (GUIHandler::_pDOFActive)
	{
		_pShaderList["Effect DOFHBlur"].get()->SetupRenderProcess(DX11AppHelper::_pImmediateContext, DX11AppHelper::_pConstantBuffer, false);
		_pShaderList["Effect DOFHBlur"].get()->RenderToTexture(DX11AppHelper::_pImmediateContext, DX11AppHelper::_pConstantBuffer, cb);

		_pShaderList["Effect DOFVBlur"].get()->SetupRenderProcess(DX11AppHelper::_pImmediateContext, DX11AppHelper::_pConstantBuffer, false);
		_pShaderList["Effect DOFVBlur"].get()->RenderToTexture(DX11AppHelper::_pImmediateContext, DX11AppHelper::_pConstantBuffer, cb);
	}

	_pShaderList["Final Pass"].get()->SetupRenderProcess(DX11AppHelper::_pImmediateContext, DX11AppHelper::_pConstantBuffer, false);
	_pShaderList["Final Pass"].get()->RenderToTexture(DX11AppHelper::_pImmediateContext, DX11AppHelper::_pConstantBuffer, cb);
}

void ShaderManager::HandleShaderControls(float deltaTime, int selectedShaderOption)
{
	HandleToggleShaderControls(selectedShaderOption);
}

void ShaderManager::HandleToggleShaderControls(int selectedShaderOption)
{
	if (GUIHandler::_pDOFActive && !_pDOFWasOn)
	{
		if (_pDeferred)
		{
			_pCurrentSceneRenderProcess = _pShaderList["Parrallax Scene"].get();
			_pShaderList["Effect HBlur"].get()->RemoveShaderResources();
			_pShaderList["Effect HBlur"].get()->AddShaderResource(_pShaderList["Deferred Parrallax Scene"].get()->GetShaderTargetTexture("OutputText"));
			_pShaderList["Final Pass"].get()->RemoveShaderResources();
			_pShaderList["Final Pass"].get()->AddShaderResource(_pShaderList["Effect DOFVBlur"].get()->GetShaderTargetTexture("OutputText"));
		}
		else
		{
			_pCurrentSceneRenderProcess = _pShaderList["Parrallax Scene"].get();
			_pShaderList["Effect HBlur"].get()->RemoveShaderResources();
			_pShaderList["Effect HBlur"].get()->AddShaderResource(_pCurrentSceneRenderProcess->GetShaderTargetTexture("ColourMap"));
			_pShaderList["Final Pass"].get()->RemoveShaderResources();
			_pShaderList["Final Pass"].get()->AddShaderResource(_pShaderList["Effect DOFVBlur"].get()->GetShaderTargetTexture("OutputText"));
		}

		_pDOFWasOn = true;
	}
	else if (!GUIHandler::_pDOFActive && _pDOFWasOn)
	{
		TurnOffDOF();
	}

	if (_pCurrentShaderOptionSelected != selectedShaderOption)
	{
		_pCurrentShaderOptionSelected = selectedShaderOption;

		switch (selectedShaderOption)
		{
		case 0:
			TurnOffDOF();
			_pCurrentSceneRenderProcess = _pShaderList["Basic Scene"].get();
			_pShaderList["Effect HBlur"].get()->RemoveShaderResources();
			_pShaderList["Effect HBlur"].get()->AddShaderResource(_pShaderList["Basic Scene"].get()->GetShaderTargetTexture("OutputText"));
			GUIHandler::ResetBlurOptions();
			break;

		case 1:
			TurnOffDOF();
			_pCurrentSceneRenderProcess = _pShaderList["Pixel Scene"].get();
			_pShaderList["Effect HBlur"].get()->RemoveShaderResources();
			_pShaderList["Effect HBlur"].get()->AddShaderResource(_pShaderList["Pixel Scene"].get()->GetShaderTargetTexture("OutputText"));
			GUIHandler::ResetBlurOptions();
			break;

		case 2:
			TurnOffDOF();
			_pShaderList["Parrallax Scene"].get()->RemoveShaderResources();
			_pShaderList["Parrallax Scene"].get()->AddShaderResource(_pShaderList["White Light Depth Map"].get()->GetDepthMapResourceView());
			_pShaderList["Parrallax Scene"].get()->AddShaderResource(_pShaderList["Red Light Depth Map"].get()->GetDepthMapResourceView());
			_pShaderList["Parrallax Scene"].get()->AddShaderResource(_pShaderList["Green Light Depth Map"].get()->GetDepthMapResourceView());
			_pShaderList["Parrallax Scene"].get()->AddShaderResource(_pShaderList["Blue Light Depth Map"].get()->GetDepthMapResourceView());
			_pShaderList["Parrallax Scene"].get()->SetCurrentShaderIndex(0);
			_pCurrentSceneRenderProcess = _pShaderList["Parrallax Scene"].get();
			_pShaderList["Effect HBlur"].get()->RemoveShaderResources();
			_pShaderList["Effect HBlur"].get()->AddShaderResource(_pCurrentSceneRenderProcess->GetShaderTargetTexture("ColourMap"));
			_pShaderList["Final Pass"].get()->RemoveShaderResources();
			_pShaderList["Final Pass"].get()->AddShaderResource(_pShaderList["Effect VBlur"].get()->GetShaderTargetTexture("OutputText"));
			GUIHandler::ResetBlurOptions();
			_pDeferred = false;
			break;

		case 3:
			TurnOffDOF();
			_pShaderList["Parrallax Scene"].get()->RemoveShaderResources();
			_pShaderList["Parrallax Scene"].get()->SetCurrentShaderIndex(1);
			_pCurrentSceneRenderProcess = _pShaderList["Parrallax Scene"].get();
			_pShaderList["Effect HBlur"].get()->RemoveShaderResources();
			_pCurrentSceneRenderProcess = _pShaderList["Parrallax Scene"].get();
			_pShaderList["Effect HBlur"].get()->RemoveShaderResources();
			_pShaderList["Effect HBlur"].get()->AddShaderResource(_pCurrentSceneRenderProcess->GetShaderTargetTexture("ColourMap"));
			GUIHandler::ResetBlurOptions();
			_pDeferred = true;
			break;

		case 4:
			TurnOffDOF();
			_pShaderList["Parrallax Scene"].get()->RemoveShaderResources();
			_pShaderList["Parrallax Scene"].get()->SetCurrentShaderIndex(1);
			_pCurrentSceneRenderProcess = _pShaderList["Parrallax Scene"].get();
			_pShaderList["Effect HBlur"].get()->RemoveShaderResources();
			_pShaderList["Effect HBlur"].get()->AddShaderResource(_pCurrentSceneRenderProcess->GetShaderTargetTexture("NormalMap"));
			GUIHandler::ResetBlurOptions();
			_pDeferred = true;
			break;

		case 5:
			TurnOffDOF();
			_pShaderList["Parrallax Scene"].get()->SetCurrentShaderIndex(1);
			_pCurrentSceneRenderProcess = _pShaderList["Parrallax Scene"].get();
			_pShaderList["Effect HBlur"].get()->RemoveShaderResources();
			_pShaderList["Effect HBlur"].get()->AddShaderResource(_pCurrentSceneRenderProcess->GetShaderTargetTexture("PositionMap"));
			GUIHandler::ResetBlurOptions();
			_pDeferred = true;
			break;

		case 6:
			TurnOffDOF();
			_pShaderList["Parrallax Scene"].get()->RemoveShaderResources();
			_pShaderList["Parrallax Scene"].get()->SetCurrentShaderIndex(1);
			_pCurrentSceneRenderProcess = _pShaderList["Parrallax Scene"].get();
			_pShaderList["Effect HBlur"].get()->RemoveShaderResources();
			_pShaderList["Effect HBlur"].get()->AddShaderResource(_pShaderList["Deferred Parrallax Scene"].get()->GetShaderTargetTexture("OutputText"));
			_pShaderList["Final Pass"].get()->RemoveShaderResources();
			_pShaderList["Final Pass"].get()->AddShaderResource(_pShaderList["Effect VBlur"].get()->GetShaderTargetTexture("OutputText"));
			GUIHandler::ResetBlurOptions();
			_pDeferred = true;
			break;
		}
	}
}

void ShaderManager::TurnOffDOF()
{
	_pShaderList["Parrallax Scene"].get()->RemoveShaderResources();
	_pShaderList["Parrallax Scene"].get()->AddShaderResource(_pShaderList["White Light Depth Map"].get()->GetDepthMapResourceView());
	_pShaderList["Parrallax Scene"].get()->AddShaderResource(_pShaderList["Red Light Depth Map"].get()->GetDepthMapResourceView());
	_pShaderList["Parrallax Scene"].get()->AddShaderResource(_pShaderList["Green Light Depth Map"].get()->GetDepthMapResourceView());
	_pShaderList["Parrallax Scene"].get()->AddShaderResource(_pShaderList["Blue Light Depth Map"].get()->GetDepthMapResourceView());
	_pShaderList["Parrallax Scene"].get()->SetCurrentShaderIndex(0);
	_pShaderList["Effect HBlur"].get()->RemoveShaderResources();
	_pShaderList["Effect HBlur"].get()->AddShaderResource(_pCurrentSceneRenderProcess->GetShaderTargetTexture("ColourMap"));
	_pShaderList["Final Pass"].get()->RemoveShaderResources();
	_pShaderList["Final Pass"].get()->AddShaderResource(_pShaderList["Effect VBlur"].get()->GetShaderTargetTexture("OutputText"));

	_pDeferred = false;
	_pDOFWasOn = false;
}