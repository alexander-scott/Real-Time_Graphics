#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <sstream>
#include "resource.h"
#include <iostream>

#include <vector>
#include <memory>

#include "Structures.h"
#include "DDSTextureLoader.h"
#include "SceneBuilder.h"
#include "RenderToTextureProcess.h"
#include "TextureController.h"
#include "DirectXInstance.h"
#include "ShaderController.h"
#include "InputLayoutBuilder.h"
#include "GUIController.h"

using namespace DirectX;

class Application
{
public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	bool HandleKeyboard(MSG msg, float timeStep);

	void Update(float deltaTime);
	void Draw();

private:
	void Cleanup();

	void InitInputLayouts();
	HRESULT InitRenderProcesses();

	void InitScene();
	
	Scene*						mScene;
	ShaderController*			mShaderController;
	InputLayoutBuilder*			mInputLayoutBuilder;
	TextureController*			mTextureController;
};

