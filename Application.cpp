#include "Application.h"

Application::Application()
{
	mShaderController = new ShaderController;
	mInputLayoutBuilder = new InputLayoutBuilder;
	mTextureManager = new TextureController;
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

	if (FAILED(mShaderController->GetRenderToTextureProcess("Final Pass")->SetupBackBufferRTV(DX11AppHelper::_pd3dDevice, pBackBuffer)))
	{
		return E_FAIL;
	}

	InitScene();

	return S_OK;
}

void Application::InitScene()
{
	// Add all textures to a texture manager
	mTextureManager->AddTexture(DX11AppHelper::_pd3dDevice, "Brick Texture", L"Resources\\brickTexture.dds", L"Resources\\brick_NRM.dds", L"Resources\\brick_DISP.dds");
	mTextureManager->AddTexture(DX11AppHelper::_pd3dDevice, "Stone Texture", L"Resources\\stoneTexture.dds", L"Resources\\stone_NRM.dds", L"Resources\\stone_DISP.dds");
	mTextureManager->AddTexture(DX11AppHelper::_pd3dDevice, "Crate Texture", L"Resources\\Crate_COLOR.dds", L"Resources\\Crate_NRM.dds", L"Resources\\Crate_DISP.dds");
	mTextureManager->AddTexture(DX11AppHelper::_pd3dDevice, "Floor Texture", L"Resources\\floorTexture.dds", L"Resources\\floor_NRM.dds", L"Resources\\floor_DISP.dds");
	mTextureManager->AddTexture(DX11AppHelper::_pd3dDevice, "Cracked Rock Texture", L"Resources\\crackedRockTexture.dds", L"Resources\\crackedRock_NRM.dds", L"Resources\\crackedRock_DISP.dds");
	mTextureManager->AddTexture(DX11AppHelper::_pd3dDevice, "White Texture", L"Resources\\whiteColourTexture.dds", nullptr, nullptr);
	mTextureManager->AddTexture(DX11AppHelper::_pd3dDevice, "Red Texture", L"Resources\\redColourTexture.dds", nullptr, nullptr);
	mTextureManager->AddTexture(DX11AppHelper::_pd3dDevice, "Green Texture", L"Resources\\greenColourTexture.dds", nullptr, nullptr);
	mTextureManager->AddTexture(DX11AppHelper::_pd3dDevice, "Blue Texture", L"Resources\\blueColourTexture.dds", nullptr, nullptr);

	// Prebuild the geometrys and materials to pass to the scene builder
	Geometry cubeGeometry;
	cubeGeometry.indexBuffer = DX11AppHelper::_pIndexBuffer;
	cubeGeometry.vertexBuffer = DX11AppHelper::_pVertexBuffer;
	cubeGeometry.numberOfIndices = 36;
	cubeGeometry.vertexBufferOffset = 0;
	cubeGeometry.vertexBufferStride = sizeof(SimpleVertex);

	Geometry planeGeometry;
	planeGeometry.indexBuffer = DX11AppHelper::_pPlaneIndexBuffer;
	planeGeometry.vertexBuffer = DX11AppHelper::_pPlaneVertexBuffer;
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

	mScene = SceneBuilder::BuildScene("Scene.xml", cubeGeometry, noSpecMaterial, mTextureManager);
}

void Application::InitInputLayouts()
{
	mInputLayoutBuilder->BuildInputLayout("Layout 1", { "POSITION", "TEXCOORD" }, DX11AppHelper::_pd3dDevice);
	mInputLayoutBuilder->BuildInputLayout("Layout 2", { "POSITION", "NORMAL", "TEXCOORD", "TANGENT" }, DX11AppHelper::_pd3dDevice);
	mInputLayoutBuilder->BuildInputLayout("Layout 3", { "POSITION", "NORMAL", "TEXCOORD" }, DX11AppHelper::_pd3dDevice);
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

	///////////////////////////////////////////////////////////////////
	// Light Depth Maps
	///////////////////////////////////////////////////////////////////
	if (FAILED(mShaderController->AddDepthBufferShader("White Light Depth Map", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Depth Mapping.fx", mInputLayoutBuilder->GetD3D11InputDescs("Layout 1"), DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	if (FAILED(mShaderController->AddDepthBufferShader("Red Light Depth Map", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Depth Mapping.fx", mInputLayoutBuilder->GetD3D11InputDescs("Layout 1"), DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	if (FAILED(mShaderController->AddDepthBufferShader("Green Light Depth Map", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Depth Mapping.fx", mInputLayoutBuilder->GetD3D11InputDescs("Layout 1"), DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	if (FAILED(mShaderController->AddDepthBufferShader("Blue Light Depth Map", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Depth Mapping.fx", mInputLayoutBuilder->GetD3D11InputDescs("Layout 1"), DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	///////////////////////////////////////////////////////////////////
	// Build parallax scene
	///////////////////////////////////////////////////////////////////
	mShaderController->AddCustomShader("Parallax Scene", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight);

	mShaderController->GetRenderToTextureProcess("Parallax Scene")->LoadShaderFilesAndInputLayouts(  DX11AppHelper::_pd3dDevice, L"DX11 Framework Parallax SS Diffuse Mapping.fx", L"DX11 Framework Parallax SS Diffuse Mapping.fx", mInputLayoutBuilder->GetD3D11InputDescs("Layout 2"));
	mShaderController->GetRenderToTextureProcess("Parallax Scene")->LoadShaderFilesAndInputLayouts(  DX11AppHelper::_pd3dDevice, L"DX11 Framework Parallax SS Deferred.fx", L"DX11 Framework Parallax SS Deferred.fx", mInputLayoutBuilder->GetD3D11InputDescs("Layout 2"));

	if (FAILED(hr))
		return hr;

	hr = mShaderController->GetRenderToTextureProcess("Parallax Scene")->SetupRTVAndSRV(  DX11AppHelper::_pd3dDevice, "ColourMap");
	hr = mShaderController->GetRenderToTextureProcess("Parallax Scene")->SetupRTVAndSRV(  DX11AppHelper::_pd3dDevice, "NormalMap");
	hr = mShaderController->GetRenderToTextureProcess("Parallax Scene")->SetupRTVAndSRV(  DX11AppHelper::_pd3dDevice, "TexCoordOffsetMap");
	hr = mShaderController->GetRenderToTextureProcess("Parallax Scene")->SetupRTVAndSRV(  DX11AppHelper::_pd3dDevice, "PositionMap");
	hr = mShaderController->GetRenderToTextureProcess("Parallax Scene")->SetupRTVAndSRV(  DX11AppHelper::_pd3dDevice, "TangentMap");
	hr = mShaderController->GetRenderToTextureProcess("Parallax Scene")->SetupRTVAndSRV(  DX11AppHelper::_pd3dDevice, "BiNormalMap");
	hr = mShaderController->GetRenderToTextureProcess("Parallax Scene")->SetupRTVAndSRV(  DX11AppHelper::_pd3dDevice, "WorldNormalMap");

	if (FAILED(hr))
		return hr;

	mShaderController->GetRenderToTextureProcess("Parallax Scene")->AddSamplerState(_pSamplerLinear);
	mShaderController->GetRenderToTextureProcess("Parallax Scene")->AddSamplerState(_pSamplerClamp);

	mShaderController->GetRenderToTextureProcess("Parallax Scene")->AddShaderResource(mShaderController->GetRenderToTextureProcess("White Light Depth Map")->GetDepthMapResourceView());
	mShaderController->GetRenderToTextureProcess("Parallax Scene")->AddShaderResource(mShaderController->GetRenderToTextureProcess("Red Light Depth Map")->GetDepthMapResourceView());
	mShaderController->GetRenderToTextureProcess("Parallax Scene")->AddShaderResource(mShaderController->GetRenderToTextureProcess("Green Light Depth Map")->GetDepthMapResourceView());
	mShaderController->GetRenderToTextureProcess("Parallax Scene")->AddShaderResource(mShaderController->GetRenderToTextureProcess("Blue Light Depth Map")->GetDepthMapResourceView());

	mShaderController->SetCurrentRenderToTexutreProcess(mShaderController->GetRenderToTextureProcess("Parallax Scene"));

	///////////////////////////////////////////////////////////////////
	// Build basic scene
	///////////////////////////////////////////////////////////////////
	if (FAILED(mShaderController->AddCommonShader("Basic Scene", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Basic.fx", mInputLayoutBuilder->GetD3D11InputDescs("Layout 3"), DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	mShaderController->GetRenderToTextureProcess("Basic Scene")->AddSamplerState(_pSamplerLinear);

	///////////////////////////////////////////////////////////////////
	// Build Pixel Scene
	///////////////////////////////////////////////////////////////////
	if (FAILED(mShaderController->AddCommonShader("Pixel Scene", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Pixel SS.fx", mInputLayoutBuilder->GetD3D11InputDescs("Layout 2"), DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	mShaderController->GetRenderToTextureProcess("Pixel Scene")->AddSamplerState(_pSamplerLinear);

	mShaderController->GetRenderToTextureProcess("Pixel Scene")->AddSamplerState(_pSamplerLinear);

	mShaderController->GetRenderToTextureProcess("Pixel Scene")->AddShaderResource(mShaderController->GetRenderToTextureProcess("White Light Depth Map")->GetDepthMapResourceView());
	mShaderController->GetRenderToTextureProcess("Pixel Scene")->AddShaderResource(mShaderController->GetRenderToTextureProcess("Red Light Depth Map")->GetDepthMapResourceView());
	mShaderController->GetRenderToTextureProcess("Pixel Scene")->AddShaderResource(mShaderController->GetRenderToTextureProcess("Green Light Depth Map")->GetDepthMapResourceView());
	mShaderController->GetRenderToTextureProcess("Pixel Scene")->AddShaderResource(mShaderController->GetRenderToTextureProcess("Blue Light Depth Map")->GetDepthMapResourceView());

	if (FAILED(mShaderController->AddRenderFromQuadShader("Effect HBlur", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Horizontal Blur.fx", mInputLayoutBuilder->GetD3D11InputDescs("Layout 3"), DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	mShaderController->GetRenderToTextureProcess("Effect HBlur")->AddSamplerState(_pSamplerLinear);
	mShaderController->GetRenderToTextureProcess("Effect HBlur")->AddShaderResource(mShaderController->GetRenderToTextureProcess("Parallax Scene")->GetShaderTargetTexture("ColourMap"));

	if (FAILED(mShaderController->AddRenderFromQuadShader("Effect VBlur", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Vertical Blur.fx", mInputLayoutBuilder->GetD3D11InputDescs("Layout 3"), DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	mShaderController->GetRenderToTextureProcess("Effect VBlur")->AddSamplerState(_pSamplerLinear);
	mShaderController->GetRenderToTextureProcess("Effect VBlur")->AddShaderResource(mShaderController->GetRenderToTextureProcess("Effect HBlur")->GetShaderTargetTexture("OutputText"));

	if (FAILED(mShaderController->AddRenderToBackBufferShader("Final Pass", (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, L"DX11 Framework Render To Texture.fx", mInputLayoutBuilder->GetD3D11InputDescs("Layout 3"), DX11AppHelper::_pd3dDevice)))
	{
		return E_FAIL;
	}

	mShaderController->GetRenderToTextureProcess("Final Pass")->AddSamplerState(_pSamplerLinear);
	mShaderController->GetRenderToTextureProcess("Final Pass")->AddShaderResource(mShaderController->GetRenderToTextureProcess("Effect VBlur")->GetShaderTargetTexture("OutputText"));

	mShaderController->GetRenderToTextureProcess("Final Pass")->SetClearColour(0.0f, 0.0f, 1.0f, 1.0f);

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
	mShaderController->UpdateShaderSelection(GUIController::_pShaderControlOption);

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

	mShaderController->ExecuteShadersInOrder(&cb, mScene->GetSceneLights(), mScene->GetGameObjectsInFrustumOctree());

	ImGui::Render();

    // Present our back buffer to our front buffer
	DX11AppHelper::_pSwapChain->Present(0, 0);
}