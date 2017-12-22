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
#include "TextureManager.h"
#include "DX11AppHelper.h"
#include "ShaderManager.h"
#include "InputLayoutManager.h"
#include "GUIHandler.h"

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

	void InitCamera();
	void InitTextures();
	void InitScene(Geometry cubeGeometry, Material noSpecMaterial, Material shinyMaterial);
	
	Light GetLightFromSceneLight(SceneLight* light);

	Scene*						mScene;
};

