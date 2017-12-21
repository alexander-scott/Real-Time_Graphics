#pragma once

#include <d3d11_1.h>
#include <unordered_map>
#include <memory>

#include "DX11AppHelper.h"
#include "RenderToTextureProcess.h"
#include "GUIHandler.h"

using namespace std;
using namespace DirectX;

static class ShaderManager
{
public:
	ShaderManager();
	~ShaderManager();

	static HRESULT AddDepthBufferShader(string shaderName, float renderWidth, float renderHeight, WCHAR* szFileName, vector<D3D11_INPUT_ELEMENT_DESC> layoutDescVec, ID3D11Device* d3dDevice);
	static HRESULT AddCommonShader(string shaderName, float renderWidth, float renderHeight, WCHAR* szFileName, vector<D3D11_INPUT_ELEMENT_DESC> layoutDescVec, ID3D11Device* d3dDevice);
	static HRESULT AddRenderFromQuadShader(string shaderName, float renderWidth, float renderHeight, WCHAR* szFileName, vector<D3D11_INPUT_ELEMENT_DESC> layoutDescVec, ID3D11Device* d3dDevice);
	static HRESULT AddRenderToBackBufferShader(string shaderName, float renderWidth, float renderHeight, WCHAR* szFileName, vector<D3D11_INPUT_ELEMENT_DESC> layoutDescVec, ID3D11Device* d3dDevice);
	static void AddCustomShader(string shaderName, float renderWidth, float renderHeight);

	static void HandleShaderControls(float deltaTime, int selectedShaderOption);

	static void ExecuteShadersInOrder(ConstantBuffer* cb, vector<SceneLight*> lights, vector<GameObject*> gameObjects);

	static void TurnOffDOF();

private: 
	static HRESULT SetupShader(string shaderName, string rtvName, float renderWidth, float renderHeight, WCHAR* szFileName, vector<D3D11_INPUT_ELEMENT_DESC> layoutDescVec, ID3D11Device* d3dDevice);
	static void HandleToggleShaderControls(int selectedShaderOption);

public:
	static unordered_map<string, unique_ptr<RenderToTextureProcess>> _pShaderList;
	static RenderToTextureProcess* _pCurrentSceneRenderProcess;

private:
	static int _pCurrentShaderOptionSelected;
	static bool _pDeferred;
	static bool _pDOFWasOn;
};