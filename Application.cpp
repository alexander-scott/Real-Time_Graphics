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
	GUIController::SetupGUI();

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

	InitTextures();

	InitScene(cubeGeometry, noSpecMaterial, shinyMaterial);

	return S_OK;
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

void Application::InitScene(Geometry cubeGeometry, Material noSpecMaterial, Material shinyMaterial)
{
	mScene = SceneBuilder::BuildScene("Scene.xml", cubeGeometry, noSpecMaterial);
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

	if (FAILED(ShaderManager::AddCommonShader("Basic Scene", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Basic.fx", InputLayoutManager::_pInputLayoutList["Layout 3"], DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	ShaderManager::_pShaderList["Basic Scene"].get()->AddSamplerState(_pSamplerLinear);

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

	if (FAILED(ShaderManager::AddRenderFromQuadShader("Effect HBlur", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Horizontal Blur.fx", InputLayoutManager::_pInputLayoutList["Layout 3"], DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	ShaderManager::_pShaderList["Effect HBlur"].get()->AddSamplerState(_pSamplerLinear);
	ShaderManager::_pShaderList["Effect HBlur"].get()->AddShaderResource(ShaderManager::_pShaderList["Parrallax Scene"].get()->GetShaderTargetTexture("ColourMap"));

	if (FAILED(ShaderManager::AddRenderFromQuadShader("Effect VBlur", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Vertical Blur.fx", InputLayoutManager::_pInputLayoutList["Layout 3"], DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	ShaderManager::_pShaderList["Effect VBlur"].get()->AddSamplerState(_pSamplerLinear);
	ShaderManager::_pShaderList["Effect VBlur"].get()->AddShaderResource(ShaderManager::_pShaderList["Effect HBlur"].get()->GetShaderTargetTexture("OutputText"));

	if (FAILED(ShaderManager::AddDepthBufferShader("DOF Depth Map", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Basic Depth Mapping.fx", InputLayoutManager::_pInputLayoutList["Layout 1"], DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	ShaderManager::_pShaderList["DOF Depth Map"].get()->SetClearColour(1.0f, 1.0f, 1.0f, 1.0f);

	if (FAILED(ShaderManager::AddRenderFromQuadShader("Effect DOFHBlur", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework DOF Hor Blur.fx", InputLayoutManager::_pInputLayoutList["Layout 3"], DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	ShaderManager::_pShaderList["Effect DOFHBlur"].get()->AddSamplerState(_pSamplerLinear);
	ShaderManager::_pShaderList["Effect DOFHBlur"].get()->AddSamplerState(_pSamplerClamp);
	ShaderManager::_pShaderList["Effect DOFHBlur"].get()->AddShaderResource(ShaderManager::_pShaderList["Effect VBlur"].get()->GetShaderTargetTexture("OutputText"));
	ShaderManager::_pShaderList["Effect DOFHBlur"].get()->AddShaderResource(ShaderManager::_pShaderList["DOF Depth Map"].get()->GetShaderTargetTexture("DepthMap"));

	if (FAILED(ShaderManager::AddRenderFromQuadShader("Effect DOFVBlur", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework DOF Vert Blur.fx", InputLayoutManager::_pInputLayoutList["Layout 3"], DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	ShaderManager::_pShaderList["Effect DOFVBlur"].get()->AddSamplerState(_pSamplerLinear);
	ShaderManager::_pShaderList["Effect DOFVBlur"].get()->AddSamplerState(_pSamplerClamp);
	ShaderManager::_pShaderList["Effect DOFVBlur"].get()->AddShaderResource(ShaderManager::_pShaderList["Effect DOFHBlur"].get()->GetShaderTargetTexture("OutputText"));
	ShaderManager::_pShaderList["Effect DOFVBlur"].get()->AddShaderResource(ShaderManager::_pShaderList["DOF Depth Map"].get()->GetShaderTargetTexture("DepthMap"));

	if (FAILED(ShaderManager::AddRenderToBackBufferShader("Final Pass", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Render To Texture.fx", InputLayoutManager::_pInputLayoutList["Layout 3"], DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	ShaderManager::_pShaderList["Final Pass"].get()->AddSamplerState(_pSamplerLinear);
	ShaderManager::_pShaderList["Final Pass"].get()->AddShaderResource(ShaderManager::_pShaderList["Effect VBlur"].get()->GetShaderTargetTexture("OutputText"));

	ShaderManager::_pShaderList["Final Pass"].get()->SetClearColour(0.0f, 0.0f, 1.0f, 1.0f);

	return hr;
}

void Application::Cleanup()
{
	GUIController::ExitGUI();

	if (mScene)
	{
		delete mScene;
		mScene = nullptr;
	}	
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
	GUIController::UpdateGUI();
	ShaderManager::HandleShaderControls(deltaTime, GUIController::_pShaderControlOption);

	if (!GUIController::io.WantCaptureMouse)
	{
		POINT p;
		GetCursorPos(&p);

		// Send mouse move events to our camera so the camera's rotation gets updated
		mScene->OnMouseMove(p.x, p.y);
	}

	// Update our time
	static float timeSinceStart = 0.0f;
	static DWORD dwTimeStart = 0;

	DWORD dwTimeCur = GetTickCount();

	if (dwTimeStart == 0)
		dwTimeStart = dwTimeCur;

	timeSinceStart = (dwTimeCur - dwTimeStart) / 1000.0f;

	// Update scene
	mScene->Update(timeSinceStart, deltaTime);
}

void Application::Draw()
{
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;

	viewMatrix = mScene->GetRenderCamera()->GetViewMatrix();
	projectionMatrix = mScene->GetRenderCamera()->GetProjectionMatrix();

	ConstantBuffer cb;

	cb.World = XMMatrixTranspose(mScene->GetGameObject(0)->GetWorldMatrix());
	cb.View = XMMatrixTranspose(viewMatrix);
	cb.Projection = XMMatrixTranspose(projectionMatrix);

	cb.surface.AmbientMtrl = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	cb.surface.DiffuseMtrl = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	cb.surface.SpecularMtrl = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	cb.lights[0] = mScene->GetSceneLight(0)->GetLight();
	cb.lights[1] = mScene->GetSceneLight(1)->GetLight();
	cb.lights[2] = mScene->GetSceneLight(2)->GetLight();
	cb.lights[3] = mScene->GetSceneLight(3)->GetLight();

	cb.EyePosW = mScene->GetRenderCamera()->GetPosition();
	cb.HasTexture = 0.0f;
	cb.HasNormalMap = 0.0f;
	cb.HasHeightMap = 0.0f;
	cb.screenWidth = DX11AppHelper::_pRenderWidth;
	cb.screenHeight = DX11AppHelper::_pRenderHeight;

	if (GUIController::_pSelfShadingOn)
		cb.selfShadowOn = 1.0f;
	else
		cb.selfShadowOn = 0.0f;

	ShaderManager::ExecuteShadersInOrder(&cb, mScene->GetSceneLights(), mScene->GetGameObjectsInFrustumOctree());

	ImGui::Render();

    // Present our back buffer to our front buffer
	DX11AppHelper::_pSwapChain->Present(0, 0);
}