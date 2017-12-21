#include "Application.h"

Application::Application()
{
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    if (FAILED(DX11AppHelper::InitWindow(hInstance, nCmdShow)))
	{
        return E_FAIL;
	}

    if (FAILED(DX11AppHelper::InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }

	InitInputLayouts();
	InitRenderProcesses();
	GUIHandler::SetupGUI();

	// Setup Render Target Views
	// Create a render target view for the back buffer
	ID3D11Texture2D* pBackBuffer = nullptr;
	if (FAILED(DX11AppHelper::_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer)))
	{
		return E_FAIL;
	}

	if (FAILED(ShaderManager::_pShaderList["Final Pass"].get()->SetupBackBufferRTV(DX11AppHelper::_pd3dDevice, pBackBuffer)))
	{
		return E_FAIL;
	}

	Geometry cubeGeometry;
	cubeGeometry.indexBuffer =  DX11AppHelper::_pIndexBuffer;
	cubeGeometry.vertexBuffer =  DX11AppHelper::_pVertexBuffer;
	cubeGeometry.numberOfIndices = 36;
	cubeGeometry.vertexBufferOffset = 0;
	cubeGeometry.vertexBufferStride = sizeof(SimpleVertex);

	Geometry planeGeometry;
	planeGeometry.indexBuffer =  DX11AppHelper::_pPlaneIndexBuffer;
	planeGeometry.vertexBuffer =  DX11AppHelper::_pPlaneVertexBuffer;
	planeGeometry.numberOfIndices = 6;
	planeGeometry.vertexBufferOffset = 0;
	planeGeometry.vertexBufferStride = sizeof(SimpleVertex);

	Material shinyMaterial;
	shinyMaterial.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	shinyMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	shinyMaterial.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	shinyMaterial.specularPower = 10.0f;

	Material noSpecMaterial;
	noSpecMaterial.ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	noSpecMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	noSpecMaterial.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	noSpecMaterial.specularPower = 0.0f;

	InitCamera();
	InitTextures();
	InitLights(cubeGeometry, shinyMaterial);
	InitGameObjectCubes(cubeGeometry, noSpecMaterial, shinyMaterial);

	return S_OK;
}

void Application::InitCamera()
{
	// Setup Camera
	XMFLOAT3 eye = XMFLOAT3(35.0f, 15.0f, -35.0f);
	XMFLOAT3 at = XMFLOAT3(0.0f, 2.0f, 0.0f);
	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	_pCamera = new CameraMouse(0.01f, 200.0f, DX11AppHelper::_pRenderWidth, DX11AppHelper::_pRenderHeight);

	_pCamera->SetPosition(eye);
	_pCamera->LookAt(eye, at, _pCamera->GetUp3f());
}

void Application::InitTextures()
{
	TextureManager::AddTexture(  DX11AppHelper::_pd3dDevice, "Brick Texture", L"Resources\\brickTexture.dds", L"Resources\\brick_NRM.dds", L"Resources\\brick_DISP.dds");
	TextureManager::AddTexture(  DX11AppHelper::_pd3dDevice, "Stone Texture", L"Resources\\stoneTexture.dds", L"Resources\\stone_NRM.dds", L"Resources\\stone_DISP.dds");
	TextureManager::AddTexture(  DX11AppHelper::_pd3dDevice, "Crate Texture", L"Resources\\Crate_COLOR.dds", L"Resources\\Crate_NRM.dds", L"Resources\\Crate_DISP.dds");
	TextureManager::AddTexture(  DX11AppHelper::_pd3dDevice, "Floor Texture", L"Resources\\floorTexture.dds", L"Resources\\floor_NRM.dds", L"Resources\\floor_DISP.dds");
	TextureManager::AddTexture(  DX11AppHelper::_pd3dDevice, "Cracked Rock Texture", L"Resources\\crackedRockTexture.dds", L"Resources\\crackedRock_NRM.dds", L"Resources\\crackedRock_DISP.dds");
	TextureManager::AddTexture(  DX11AppHelper::_pd3dDevice, "White Texture", L"Resources\\whiteColourTexture.dds", nullptr, nullptr);
	TextureManager::AddTexture(  DX11AppHelper::_pd3dDevice, "Red Texture", L"Resources\\redColourTexture.dds", nullptr, nullptr);
	TextureManager::AddTexture(  DX11AppHelper::_pd3dDevice, "Green Texture", L"Resources\\greenColourTexture.dds", nullptr, nullptr);
	TextureManager::AddTexture(  DX11AppHelper::_pd3dDevice, "Blue Texture", L"Resources\\blueColourTexture.dds", nullptr, nullptr);
}

void Application::InitLights(Geometry geometry, Material material)
{
	SceneLight* whiteLight = new SceneLight("White Light", TextureManager::_pTextureList["White Texture"].get()->texture, geometry, material);
	SceneLight* redLight = new SceneLight("Red Light", TextureManager::_pTextureList["Red Texture"].get()->texture, geometry, material);
	SceneLight* greenLight = new SceneLight("Green Light", TextureManager::_pTextureList["Green Texture"].get()->texture, geometry, material);
	SceneLight* blueLight = new SceneLight("Blue Light", TextureManager::_pTextureList["Blue Texture"].get()->texture, geometry, material);

	// Setup the scene's light
	whiteLight->SetAmbientLight(XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f));
	whiteLight->SetDiffuseLight(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	whiteLight->SetSpecularLight(XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));
	whiteLight->SetSpecularPower(20.0f);
	whiteLight->SetLightVecW(XMFLOAT3(-5.05f, 10.0f, 14.5f));
	whiteLight->SetPaddingLightAmount(XMFLOAT3(0.0f, 0.0f, 0.0f));
	whiteLight->SetLightOn(0.0f);
	whiteLight->GetLightCubeGO()->SetPosition(whiteLight->GetLightVecW());

	// Setup the scene's RED light
	redLight->SetAmbientLight(XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f));
	redLight->SetDiffuseLight(XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
	redLight->SetSpecularLight(XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));
	redLight->SetSpecularPower(20.0f);
	redLight->SetLightVecW(XMFLOAT3(-0.45f, 10.0f, 12.7f));
	redLight->SetPaddingLightAmount(XMFLOAT3(0.0f, 0.0f, 0.0f));
	redLight->SetLightOn(1.0f);
	redLight->GetLightCubeGO()->SetPosition(redLight->GetLightVecW());

	// Setup the scene's GREEN light
	greenLight->SetAmbientLight(XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f));
	greenLight->SetDiffuseLight(XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
	greenLight->SetSpecularLight(XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));
	greenLight->SetSpecularPower(20.0f);
	greenLight->SetLightVecW(XMFLOAT3(4.0f, 10.0f, 10.0f));
	greenLight->SetPaddingLightAmount(XMFLOAT3(0.0f, 0.0f, 0.0f));
	greenLight->SetLightOn(0.0f);
	greenLight->GetLightCubeGO()->SetPosition(greenLight->GetLightVecW());

	// Setup the scene's BLUE light
	blueLight->SetAmbientLight(XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f));
	blueLight->SetDiffuseLight(XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));
	blueLight->SetSpecularLight(XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));
	blueLight->SetSpecularPower(20.0f);
	blueLight->SetLightVecW(XMFLOAT3(8.0f, 10.0f, 10.0f));
	blueLight->SetPaddingLightAmount(XMFLOAT3(0.0f, 0.0f, 0.0f));
	blueLight->SetLightOn(0.0f);
	blueLight->GetLightCubeGO()->SetPosition(blueLight->GetLightVecW());

	_pLights.push_back(whiteLight);
	_pLights.push_back(redLight);
	_pLights.push_back(greenLight);
	_pLights.push_back(blueLight);

	_pGameObjects.push_back(whiteLight->GetLightCubeGO());
	_pGameObjects.push_back(redLight->GetLightCubeGO());
	_pGameObjects.push_back(greenLight->GetLightCubeGO());
	_pGameObjects.push_back(blueLight->GetLightCubeGO());
}

void Application::InitGameObjectCubes(Geometry cubeGeometry, Material noSpecMaterial, Material shinyMaterial)
{
	GameObject* gameObject = new GameObject("Floor", cubeGeometry, noSpecMaterial);
	gameObject->SetPosition(0.0f, 0.0f, 0.0f);
	gameObject->SetScale(5.0f, 5.0f, 5.0f);
	gameObject->SetRotation(0.0f, 0.0f, 0.0f);
	gameObject->SetTextures(TextureManager::_pTextureList["Floor Texture"].get());

	_pGameObjects.push_back(gameObject);
}

void Application::InitInputLayouts()
{
	InputLayoutManager::GenerateInputLayout("Layout 1", { "POSITION", "TEXCOORD" }, DX11AppHelper::_pd3dDevice);
	InputLayoutManager::GenerateInputLayout("Layout 2", { "POSITION", "NORMAL", "TEXCOORD", "TANGENT" }, DX11AppHelper::_pd3dDevice);
	InputLayoutManager::GenerateInputLayout("Layout 3", { "POSITION", "NORMAL", "TEXCOORD" }, DX11AppHelper::_pd3dDevice);
}

HRESULT Application::InitRenderProcesses()
{
	HRESULT hr;

#pragma region Setup Sampler States

	ID3D11SamplerState* _pSamplerLinear;
	ID3D11SamplerState* _pSamplerClamp;
	ID3D11SamplerState* _pSamplerNormalDepth;

	D3D11_SAMPLER_DESC samplerDesc;

	// Create a wrap texture sampler state description.
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr =   DX11AppHelper::_pd3dDevice->CreateSamplerState(&samplerDesc, &_pSamplerLinear);

	if (FAILED(hr))
		return hr;

	// Create a clamp texture sampler state description.
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	hr =   DX11AppHelper::_pd3dDevice->CreateSamplerState(&samplerDesc, &_pSamplerClamp);

	if (FAILED(hr))
		return hr;

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 0.0f;
	samplerDesc.BorderColor[1] = 0.0f;
	samplerDesc.BorderColor[2] = 0.0f;
	samplerDesc.BorderColor[3] = 1e5f;

	hr =   DX11AppHelper::_pd3dDevice->CreateSamplerState(&samplerDesc, &_pSamplerNormalDepth);

	if (FAILED(hr))
		return hr;

#pragma endregion

#pragma region Setup Light Shadow Mapping Render Processes

	if (FAILED(ShaderManager::AddDepthBufferShader("White Light Depth Map", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Depth Mapping.fx", InputLayoutManager::_pInputLayoutList["Layout 1"], DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	if (FAILED(ShaderManager::AddDepthBufferShader("Red Light Depth Map", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Depth Mapping.fx", InputLayoutManager::_pInputLayoutList["Layout 1"], DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	if (FAILED(ShaderManager::AddDepthBufferShader("Green Light Depth Map", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Depth Mapping.fx", InputLayoutManager::_pInputLayoutList["Layout 1"], DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	if (FAILED(ShaderManager::AddDepthBufferShader("Blue Light Depth Map", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Depth Mapping.fx", InputLayoutManager::_pInputLayoutList["Layout 1"], DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

#pragma endregion

#pragma region Setup Base Scene Normal + Deferred Render Process

	ShaderManager::AddCustomShader("Parrallax Scene", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight);

	ShaderManager::_pShaderList["Parrallax Scene"].get()->LoadShaderFilesAndInputLayouts(  DX11AppHelper::_pd3dDevice, L"DX11 Framework Parallax SS Diffuse Mapping.fx", L"DX11 Framework Parallax SS Diffuse Mapping.fx", InputLayoutManager::_pInputLayoutList["Layout 2"]);
	ShaderManager::_pShaderList["Parrallax Scene"].get()->LoadShaderFilesAndInputLayouts(  DX11AppHelper::_pd3dDevice, L"DX11 Framework Parallax SS Deferred.fx", L"DX11 Framework Parallax SS Deferred.fx", InputLayoutManager::_pInputLayoutList["Layout 2"]);

	if (FAILED(hr))
		return hr;

	hr = ShaderManager::_pShaderList["Parrallax Scene"].get()->SetupRTVAndSRV(  DX11AppHelper::_pd3dDevice, "ColourMap");
	hr = ShaderManager::_pShaderList["Parrallax Scene"].get()->SetupRTVAndSRV(  DX11AppHelper::_pd3dDevice, "NormalMap");
	hr = ShaderManager::_pShaderList["Parrallax Scene"].get()->SetupRTVAndSRV(  DX11AppHelper::_pd3dDevice, "TexCoordOffsetMap");
	hr = ShaderManager::_pShaderList["Parrallax Scene"].get()->SetupRTVAndSRV(  DX11AppHelper::_pd3dDevice, "PositionMap");
	hr = ShaderManager::_pShaderList["Parrallax Scene"].get()->SetupRTVAndSRV(  DX11AppHelper::_pd3dDevice, "TangentMap");
	hr = ShaderManager::_pShaderList["Parrallax Scene"].get()->SetupRTVAndSRV(  DX11AppHelper::_pd3dDevice, "BiNormalMap");
	hr = ShaderManager::_pShaderList["Parrallax Scene"].get()->SetupRTVAndSRV(  DX11AppHelper::_pd3dDevice, "WorldNormalMap");

	if (FAILED(hr))
		return hr;

	ShaderManager::_pShaderList["Parrallax Scene"].get()->AddSamplerState(_pSamplerLinear);
	ShaderManager::_pShaderList["Parrallax Scene"].get()->AddSamplerState(_pSamplerClamp);

	ShaderManager::_pShaderList["Parrallax Scene"].get()->AddShaderResource(ShaderManager::_pShaderList["White Light Depth Map"].get()->GetDepthMapResourceView());
	ShaderManager::_pShaderList["Parrallax Scene"].get()->AddShaderResource(ShaderManager::_pShaderList["Red Light Depth Map"].get()->GetDepthMapResourceView());
	ShaderManager::_pShaderList["Parrallax Scene"].get()->AddShaderResource(ShaderManager::_pShaderList["Green Light Depth Map"].get()->GetDepthMapResourceView());
	ShaderManager::_pShaderList["Parrallax Scene"].get()->AddShaderResource(ShaderManager::_pShaderList["Blue Light Depth Map"].get()->GetDepthMapResourceView());

	ShaderManager::_pCurrentSceneRenderProcess = ShaderManager::_pShaderList["Parrallax Scene"].get();

#pragma endregion

#pragma region Setup Basic Scene Render Process

	if (FAILED(ShaderManager::AddCommonShader("Basic Scene", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Basic.fx", InputLayoutManager::_pInputLayoutList["Layout 3"], DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	ShaderManager::_pShaderList["Basic Scene"].get()->AddSamplerState(_pSamplerLinear);

#pragma endregion

#pragma region Setup Pixel Scene Render Process

	if (FAILED(ShaderManager::AddCommonShader("Pixel Scene", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Pixel SS.fx", InputLayoutManager::_pInputLayoutList["Layout 2"], DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	ShaderManager::_pShaderList["Pixel Scene"].get()->AddSamplerState(_pSamplerLinear);

	ShaderManager::_pShaderList["Pixel Scene"].get()->AddSamplerState(_pSamplerLinear);

	ShaderManager::_pShaderList["Pixel Scene"].get()->AddShaderResource(ShaderManager::_pShaderList["White Light Depth Map"].get()->GetDepthMapResourceView());
	ShaderManager::_pShaderList["Pixel Scene"].get()->AddShaderResource(ShaderManager::_pShaderList["Red Light Depth Map"].get()->GetDepthMapResourceView());
	ShaderManager::_pShaderList["Pixel Scene"].get()->AddShaderResource(ShaderManager::_pShaderList["Green Light Depth Map"].get()->GetDepthMapResourceView());
	ShaderManager::_pShaderList["Pixel Scene"].get()->AddShaderResource(ShaderManager::_pShaderList["Blue Light Depth Map"].get()->GetDepthMapResourceView());

#pragma endregion

#pragma region Setup Deferred Lighting Render Process

	ShaderManager::AddCustomShader("Deferred Parrallax Scene", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight);

	hr = ShaderManager::_pShaderList["Deferred Parrallax Scene"].get()->LoadShaderFilesAndInputLayouts(  DX11AppHelper::_pd3dDevice, L"DX11 Framework Deferred Lighting.fx", L"DX11 Framework Deferred Lighting.fx", InputLayoutManager::_pInputLayoutList["Layout 2"]);

	if (FAILED(hr))
		return hr;

	hr = ShaderManager::_pShaderList["Deferred Parrallax Scene"].get()->SetupRTVAndSRV(  DX11AppHelper::_pd3dDevice, "OutputText");

	if (FAILED(hr))
		return hr;

	hr = ShaderManager::_pShaderList["Deferred Parrallax Scene"].get()->SetupRenderQuad(  DX11AppHelper::_pd3dDevice);

	if (FAILED(hr))
		return hr;

	ShaderManager::_pShaderList["Deferred Parrallax Scene"].get()->AddSamplerState(_pSamplerLinear);
	ShaderManager::_pShaderList["Deferred Parrallax Scene"].get()->AddSamplerState(_pSamplerClamp);

	ShaderManager::_pShaderList["Deferred Parrallax Scene"].get()->AddShaderResource(ShaderManager::_pShaderList["White Light Depth Map"].get()->GetDepthMapResourceView());
	ShaderManager::_pShaderList["Deferred Parrallax Scene"].get()->AddShaderResource(ShaderManager::_pShaderList["Red Light Depth Map"].get()->GetDepthMapResourceView());
	ShaderManager::_pShaderList["Deferred Parrallax Scene"].get()->AddShaderResource(ShaderManager::_pShaderList["Green Light Depth Map"].get()->GetDepthMapResourceView());
	ShaderManager::_pShaderList["Deferred Parrallax Scene"].get()->AddShaderResource(ShaderManager::_pShaderList["Blue Light Depth Map"].get()->GetDepthMapResourceView());

	ShaderManager::_pShaderList["Deferred Parrallax Scene"].get()->AddShaderResource(ShaderManager::_pCurrentSceneRenderProcess->GetShaderTargetTexture("ColourMap"));
	ShaderManager::_pShaderList["Deferred Parrallax Scene"].get()->AddShaderResource(ShaderManager::_pCurrentSceneRenderProcess->GetShaderTargetTexture("NormalMap"));
	ShaderManager::_pShaderList["Deferred Parrallax Scene"].get()->AddShaderResource(ShaderManager::_pCurrentSceneRenderProcess->GetShaderTargetTexture("TexCoordOffsetMap"));
	ShaderManager::_pShaderList["Deferred Parrallax Scene"].get()->AddShaderResource(ShaderManager::_pCurrentSceneRenderProcess->GetShaderTargetTexture("PositionMap"));
	ShaderManager::_pShaderList["Deferred Parrallax Scene"].get()->AddShaderResource(ShaderManager::_pCurrentSceneRenderProcess->GetShaderTargetTexture("TangentMap"));
	ShaderManager::_pShaderList["Deferred Parrallax Scene"].get()->AddShaderResource(ShaderManager::_pCurrentSceneRenderProcess->GetShaderTargetTexture("WorldNormalMap"));
	ShaderManager::_pShaderList["Deferred Parrallax Scene"].get()->AddShaderResource(ShaderManager::_pCurrentSceneRenderProcess->GetShaderTargetTexture("BiNormalMap"));

#pragma endregion

#pragma region Setup Horizontal Blur Render Process

	if (FAILED(ShaderManager::AddRenderFromQuadShader("Effect HBlur", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Horizontal Blur.fx", InputLayoutManager::_pInputLayoutList["Layout 3"], DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	ShaderManager::_pShaderList["Effect HBlur"].get()->AddSamplerState(_pSamplerLinear);
	ShaderManager::_pShaderList["Effect HBlur"].get()->AddShaderResource(ShaderManager::_pShaderList["Parrallax Scene"].get()->GetShaderTargetTexture("ColourMap"));


#pragma endregion

#pragma region Setup Vertical Blur Render Process

	if (FAILED(ShaderManager::AddRenderFromQuadShader("Effect VBlur", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Vertical Blur.fx", InputLayoutManager::_pInputLayoutList["Layout 3"], DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	ShaderManager::_pShaderList["Effect VBlur"].get()->AddSamplerState(_pSamplerLinear);
	ShaderManager::_pShaderList["Effect VBlur"].get()->AddShaderResource(ShaderManager::_pShaderList["Effect HBlur"].get()->GetShaderTargetTexture("OutputText"));

#pragma endregion

#pragma region Setup DOF Depth Map Render Process

	if (FAILED(ShaderManager::AddDepthBufferShader("DOF Depth Map", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Basic Depth Mapping.fx", InputLayoutManager::_pInputLayoutList["Layout 1"], DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	ShaderManager::_pShaderList["DOF Depth Map"].get()->SetClearColour(1.0f, 1.0f, 1.0f, 1.0f);

#pragma endregion

#pragma region Setup DOF H Blur Render Process

	if (FAILED(ShaderManager::AddRenderFromQuadShader("Effect DOFHBlur", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework DOF Hor Blur.fx", InputLayoutManager::_pInputLayoutList["Layout 3"], DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	ShaderManager::_pShaderList["Effect DOFHBlur"].get()->AddSamplerState(_pSamplerLinear);
	ShaderManager::_pShaderList["Effect DOFHBlur"].get()->AddSamplerState(_pSamplerClamp);
	ShaderManager::_pShaderList["Effect DOFHBlur"].get()->AddShaderResource(ShaderManager::_pShaderList["Effect VBlur"].get()->GetShaderTargetTexture("OutputText"));
	ShaderManager::_pShaderList["Effect DOFHBlur"].get()->AddShaderResource(ShaderManager::_pShaderList["DOF Depth Map"].get()->GetShaderTargetTexture("DepthMap"));

#pragma endregion

#pragma region Setup DOF V Blur Render Process

	if (FAILED(ShaderManager::AddRenderFromQuadShader("Effect DOFVBlur", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework DOF Vert Blur.fx", InputLayoutManager::_pInputLayoutList["Layout 3"], DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	ShaderManager::_pShaderList["Effect DOFVBlur"].get()->AddSamplerState(_pSamplerLinear);
	ShaderManager::_pShaderList["Effect DOFVBlur"].get()->AddSamplerState(_pSamplerClamp);
	ShaderManager::_pShaderList["Effect DOFVBlur"].get()->AddShaderResource(ShaderManager::_pShaderList["Effect DOFHBlur"].get()->GetShaderTargetTexture("OutputText"));
	ShaderManager::_pShaderList["Effect DOFVBlur"].get()->AddShaderResource(ShaderManager::_pShaderList["DOF Depth Map"].get()->GetShaderTargetTexture("DepthMap"));

#pragma endregion

#pragma region Setup Final Pass Render Process

	if (FAILED(ShaderManager::AddRenderToBackBufferShader("Final Pass", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Render To Texture.fx", InputLayoutManager::_pInputLayoutList["Layout 3"], DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	ShaderManager::_pShaderList["Final Pass"].get()->AddSamplerState(_pSamplerLinear);
	ShaderManager::_pShaderList["Final Pass"].get()->AddShaderResource(ShaderManager::_pShaderList["Effect VBlur"].get()->GetShaderTargetTexture("OutputText"));

	ShaderManager::_pShaderList["Final Pass"].get()->SetClearColour(0.0f, 0.0f, 1.0f, 1.0f);

#pragma endregion

	return hr;
}

void Application::Cleanup()
{
	GUIHandler::ExitGUI();

#pragma region Light Variables

	vector<SceneLight*> _pLights;

	for (auto light : _pLights)
	{
		if (light)
		{
			delete light;
			light = nullptr;
		}
	}

#pragma endregion

#pragma region Game Objects

	for (auto gameObject : _pGameObjects)
	{
		if (gameObject)
		{
			delete gameObject;
			gameObject = nullptr;
		}
	}

#pragma endregion

#pragma region Camera

	if (_pCamera)
	{
		delete _pCamera;
		_pCamera = nullptr;
	}

#pragma endregion

}

Light Application::GetLightFromSceneLight(SceneLight* light)
{
	Light newLight;

	XMMATRIX lightView = XMLoadFloat4x4(&light->GetView());
	XMMATRIX lightProjection = XMLoadFloat4x4(&light->GetProjection());

	newLight.View =					XMMatrixTranspose(lightView);
	newLight.Projection =			XMMatrixTranspose(lightProjection);
	newLight.AmbientLight =			light->GetAmbientLight();
	newLight.DiffuseLight =			light->GetDiffuseLight();
	newLight.SpecularLight =		light->GetSpecularLight();
	newLight.SpecularPower =		light->GetSpecularPower();
	newLight.LightVecW =			light->GetLightVecW();
	newLight.paddingLightAmount =	light->GetPaddingLightAmount();
	newLight.lightOn =				light->GetLightOn();

	return newLight;
}

void Application::UpdateLightsControls(float deltaTime)
{

#pragma region Toggle Current Light

	_pLights.at(GUIHandler::_pControlledLight)->HandleLightControls(deltaTime);

#pragma endregion

#pragma region Turn Lights ON/OFF

	// Toggle Lights ON/OFF
	if (GUIHandler::_pWhiteLightOn)
	{
		_pLights.at(0)->SetLightOn(true);
	}
	else
	{
		_pLights.at(0)->SetLightOn(false);
	}

	if (GUIHandler::_pRedLightOn)
	{
		_pLights.at(1)->SetLightOn(true);
	}
	else
	{
		_pLights.at(1)->SetLightOn(false);
	}

	if (GUIHandler::_pGreenLightOn)
	{
		_pLights.at(2)->SetLightOn(true);
	}
	else
	{
		_pLights.at(2)->SetLightOn(false);
	}

	if (GUIHandler::_pBlueLightOn)
	{
		_pLights.at(3)->SetLightOn(true);
	}
	else
	{
		_pLights.at(3)->SetLightOn(false);
	}

#pragma endregion

}

bool Application::HandleKeyboard(MSG msg, float deltaTime)
{
	if (GetAsyncKeyState(VK_ESCAPE))
	{
		PostQuitMessage(WM_QUIT);
	}

	return false;
}

void Application::Update(float deltaTime)
{
	GUIHandler::UpdateGUI();
	ShaderManager::HandleShaderControls(deltaTime, GUIHandler::_pShaderControlOption);

	if (!GUIHandler::io.WantCaptureMouse)
	{
		POINT p;
		GetCursorPos(&p);

		_pCamera->OnMouseMove(p.x, p.y);
	}

	// Update our time
	static float timeSinceStart = 0.0f;
	static DWORD dwTimeStart = 0;

	DWORD dwTimeCur = GetTickCount();

	if (dwTimeStart == 0)
		dwTimeStart = dwTimeCur;

	timeSinceStart = (dwTimeCur - dwTimeStart) / 1000.0f;

#pragma region Update Camera

	_pCamera->UpdateCameraView();

#pragma endregion

#pragma region Update Game Objects

	// Update objects
	for (auto gameObject : _pGameObjects)
	{
		gameObject->Update(timeSinceStart, deltaTime);
	}

#pragma endregion

#pragma region Update Lights

	UpdateLightsControls(deltaTime);

	for (SceneLight* light : _pLights)
	{
		light->UpdateLightCube(timeSinceStart, deltaTime);
		light->UpdateLight((float)  DX11AppHelper::_pRenderWidth, (float)  DX11AppHelper::_pRenderHeight);
	}

#pragma endregion

}

void Application::Draw()
{

#pragma region Initialise Draw Variables

	SMConstantBuffer smCB;

	XMFLOAT4X4 viewAsFloats;
	XMFLOAT4X4 projectionAsFloats;
	XMFLOAT4X4 shadowTransformAsFloats;
	XMMATRIX view;
	XMMATRIX projection;
	XMMATRIX shadowTransform;

#pragma endregion

#pragma region Initialise Constant Buffer

	ConstantBuffer cb;

	cb.World = XMMatrixTranspose(_pGameObjects.at(0)->GetWorldMatrix());

	viewAsFloats = _pCamera->GetView4x4f();
	projectionAsFloats = _pCamera->GetProj4x4f();

	view = XMLoadFloat4x4(&viewAsFloats);
	projection = XMLoadFloat4x4(&projectionAsFloats);

	cb.View = XMMatrixTranspose(view);
	cb.Projection = XMMatrixTranspose(projection);

	cb.surface.AmbientMtrl = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	cb.surface.DiffuseMtrl = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	cb.surface.SpecularMtrl = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	cb.lights[0] = GetLightFromSceneLight(_pLights.at(0));
	cb.lights[1] = GetLightFromSceneLight(_pLights.at(1));
	cb.lights[2] = GetLightFromSceneLight(_pLights.at(2));
	cb.lights[3] = GetLightFromSceneLight(_pLights.at(3));

	cb.EyePosW = _pCamera->GetPosition3f();
	cb.HasTexture = 0.0f;
	cb.HasNormalMap = 0.0f;
	cb.HasHeightMap = 0.0f;
	cb.screenWidth = DX11AppHelper::_pRenderWidth;
	cb.screenHeight = DX11AppHelper::_pRenderHeight;

	if (GUIHandler::_pSelfShadingOn)
	{
		cb.selfShadowOn = 1.0f;
	}
	else
	{
		cb.selfShadowOn = 0.0f;
	}

#pragma endregion

	ShaderManager::ExecuteShadersInOrder(&cb, _pLights, _pGameObjects);

	ImGui::Render();

    // Present our back buffer to our front buffer
	DX11AppHelper::_pSwapChain->Present(0, 0);
}