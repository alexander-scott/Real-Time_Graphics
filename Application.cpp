#include "Application.h"

Application::Application()
{
	mShaderController = new ShaderController;
	mTextureController = new TextureController;
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    if (FAILED(DirectXInstance::Instance().InitWindow(hInstance, nCmdShow)))
	{
        return E_FAIL;
	}

    if (FAILED(DirectXInstance::Instance().InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }

	InitInputLayouts();

	if (FAILED(InitShaderResources()))
	{
		return E_FAIL;
	}

	mShaderController->SetShaderResources();

	GUIController::SetupGUI();

	// Setup Render Target Views
	// Create a render target view for the back buffer
	ID3D11Texture2D* pBackBuffer = nullptr;
	if (FAILED(DirectXInstance::Instance()._pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer)))
	{
		return E_FAIL;
	}

	if (FAILED(mShaderController->GetRTTProcess("Final Pass")->SetupBackBufferRTV(DirectXInstance::Instance()._pd3dDevice, pBackBuffer)))
	{
		return E_FAIL;
	}

	InitScene();

	return S_OK;
}

void Application::InitScene()
{
	// Add all textures to a texture manager
	mTextureController->AddTexture(DirectXInstance::Instance()._pd3dDevice, "Brick Texture", L"Resources\\brickTexture.dds", L"Resources\\brick_NRM.dds", L"Resources\\brick_DISP.dds");
	mTextureController->AddTexture(DirectXInstance::Instance()._pd3dDevice, "Stone Texture", L"Resources\\stoneTexture.dds", L"Resources\\stone_NRM.dds", L"Resources\\stone_DISP.dds");
	mTextureController->AddTexture(DirectXInstance::Instance()._pd3dDevice, "Crate Texture", L"Resources\\Crate_COLOR.dds", L"Resources\\Crate_NRM.dds", L"Resources\\Crate_DISP.dds");
	mTextureController->AddTexture(DirectXInstance::Instance()._pd3dDevice, "Floor Texture", L"Resources\\floorTexture.dds", L"Resources\\floor_NRM.dds", L"Resources\\floor_DISP.dds");
	mTextureController->AddTexture(DirectXInstance::Instance()._pd3dDevice, "Cracked Rock Texture", L"Resources\\crackedRockTexture.dds", L"Resources\\crackedRock_NRM.dds", L"Resources\\crackedRock_DISP.dds");
	mTextureController->AddTexture(DirectXInstance::Instance()._pd3dDevice, "White Texture", L"Resources\\whiteColourTexture.dds", nullptr, nullptr);
	mTextureController->AddTexture(DirectXInstance::Instance()._pd3dDevice, "Red Texture", L"Resources\\redColourTexture.dds", nullptr, nullptr);
	mTextureController->AddTexture(DirectXInstance::Instance()._pd3dDevice, "Green Texture", L"Resources\\greenColourTexture.dds", nullptr, nullptr);
	mTextureController->AddTexture(DirectXInstance::Instance()._pd3dDevice, "Blue Texture", L"Resources\\blueColourTexture.dds", nullptr, nullptr);

	// Prebuild the geometrys and materials to pass to the scene builder
	Geometry cubeGeometry;
	cubeGeometry.indexBuffer = DirectXInstance::Instance()._pIndexBuffer;
	cubeGeometry.vertexBuffer = DirectXInstance::Instance()._pVertexBuffer;
	cubeGeometry.numberOfIndices = 36;
	cubeGeometry.vertexBufferOffset = 0;
	cubeGeometry.vertexBufferStride = sizeof(SimpleVertex);

	Geometry planeGeometry;
	planeGeometry.indexBuffer = DirectXInstance::Instance()._pPlaneIndexBuffer;
	planeGeometry.vertexBuffer = DirectXInstance::Instance()._pPlaneVertexBuffer;
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

	mScene = SceneBuilder::BuildScene("Scene.xml", cubeGeometry, shinyMaterial, mTextureController);
}

void Application::InitInputLayouts()
{
	mInputLayoutBuilder.BuildInputLayout("Layout 1", { "POSITION", "TEXCOORD" }, DirectXInstance::Instance()._pd3dDevice);
	mInputLayoutBuilder.BuildInputLayout("Layout 2", { "POSITION", "NORMAL", "TEXCOORD", "TANGENT" }, DirectXInstance::Instance()._pd3dDevice);
	mInputLayoutBuilder.BuildInputLayout("Layout 3", { "POSITION", "NORMAL", "TEXCOORD" }, DirectXInstance::Instance()._pd3dDevice);
}

HRESULT Application::InitShaderResources()
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
	hr =   DirectXInstance::Instance()._pd3dDevice->CreateSamplerState(&samplerDesc, &_pSamplerLinear);

	if (FAILED(hr))
		return hr;

	// Create a clamp texture sampler state description.
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	hr =   DirectXInstance::Instance()._pd3dDevice->CreateSamplerState(&samplerDesc, &_pSamplerClamp);

	if (FAILED(hr))
		return hr;

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 0.0f;
	samplerDesc.BorderColor[1] = 0.0f;
	samplerDesc.BorderColor[2] = 0.0f;
	samplerDesc.BorderColor[3] = 1e5f;

	hr =   DirectXInstance::Instance()._pd3dDevice->CreateSamplerState(&samplerDesc, &_pSamplerNormalDepth);

	if (FAILED(hr))
		return hr;

	///////////////////////////////////////////////////////////////////
	// Light Depth Maps
	///////////////////////////////////////////////////////////////////
	if (FAILED(mShaderController->AddDepthBufferShader("White Light Depth Map", (float)DirectXInstance::Instance()._pRenderWidth, (float)DirectXInstance::Instance()._pRenderHeight, L"DX11 Framework Depth Mapping.fx", mInputLayoutBuilder.GetD3D11InputDescs("Layout 1"), DirectXInstance::Instance()._pd3dDevice)))
	{
		return E_FAIL;
	}

	if (FAILED(mShaderController->AddDepthBufferShader("Red Light Depth Map", (float)DirectXInstance::Instance()._pRenderWidth, (float)DirectXInstance::Instance()._pRenderHeight, L"DX11 Framework Depth Mapping.fx", mInputLayoutBuilder.GetD3D11InputDescs("Layout 1"), DirectXInstance::Instance()._pd3dDevice)))
	{
		return E_FAIL;
	}

	if (FAILED(mShaderController->AddDepthBufferShader("Green Light Depth Map", (float)DirectXInstance::Instance()._pRenderWidth, (float)DirectXInstance::Instance()._pRenderHeight, L"DX11 Framework Depth Mapping.fx", mInputLayoutBuilder.GetD3D11InputDescs("Layout 1"), DirectXInstance::Instance()._pd3dDevice)))
	{
		return E_FAIL;
	}

	if (FAILED(mShaderController->AddDepthBufferShader("Blue Light Depth Map", (float)DirectXInstance::Instance()._pRenderWidth, (float)DirectXInstance::Instance()._pRenderHeight, L"DX11 Framework Depth Mapping.fx", mInputLayoutBuilder.GetD3D11InputDescs("Layout 1"), DirectXInstance::Instance()._pd3dDevice)))
	{
		return E_FAIL;
	}

	///////////////////////////////////////////////////////////////////
	// Build parallax scene
	///////////////////////////////////////////////////////////////////
	mShaderController->AddSceneShader("Parallax Scene", (float)DirectXInstance::Instance()._pRenderWidth, (float)DirectXInstance::Instance()._pRenderHeight);

	mShaderController->GetRTTProcess("Parallax Scene")->LoadShaderFilesAndInputLayouts(  DirectXInstance::Instance()._pd3dDevice, L"DX11 Framework Parallax SS Diffuse Mapping.fx", L"DX11 Framework Parallax SS Diffuse Mapping.fx", mInputLayoutBuilder.GetD3D11InputDescs("Layout 2"));

	if (FAILED(hr))
		return hr;

	hr = mShaderController->GetRTTProcess("Parallax Scene")->SetupRTVAndSRV(  DirectXInstance::Instance()._pd3dDevice, "ColourMap");
	hr = mShaderController->GetRTTProcess("Parallax Scene")->SetupRTVAndSRV(  DirectXInstance::Instance()._pd3dDevice, "NormalMap");
	hr = mShaderController->GetRTTProcess("Parallax Scene")->SetupRTVAndSRV(  DirectXInstance::Instance()._pd3dDevice, "TexCoordOffsetMap");
	hr = mShaderController->GetRTTProcess("Parallax Scene")->SetupRTVAndSRV(  DirectXInstance::Instance()._pd3dDevice, "PositionMap");
	hr = mShaderController->GetRTTProcess("Parallax Scene")->SetupRTVAndSRV(  DirectXInstance::Instance()._pd3dDevice, "TangentMap");
	hr = mShaderController->GetRTTProcess("Parallax Scene")->SetupRTVAndSRV(  DirectXInstance::Instance()._pd3dDevice, "BiNormalMap");
	hr = mShaderController->GetRTTProcess("Parallax Scene")->SetupRTVAndSRV(  DirectXInstance::Instance()._pd3dDevice, "WorldNormalMap");

	if (FAILED(hr))
		return hr;

	mShaderController->GetRTTProcess("Parallax Scene")->AddSamplerState(_pSamplerLinear);
	mShaderController->GetRTTProcess("Parallax Scene")->AddSamplerState(_pSamplerClamp);

	mShaderController->GetRTTProcess("Parallax Scene")->AddShaderResource(mShaderController->GetRTTProcess("White Light Depth Map")->GetDepthMapResourceView());
	mShaderController->GetRTTProcess("Parallax Scene")->AddShaderResource(mShaderController->GetRTTProcess("Red Light Depth Map")->GetDepthMapResourceView());
	mShaderController->GetRTTProcess("Parallax Scene")->AddShaderResource(mShaderController->GetRTTProcess("Green Light Depth Map")->GetDepthMapResourceView());
	mShaderController->GetRTTProcess("Parallax Scene")->AddShaderResource(mShaderController->GetRTTProcess("Blue Light Depth Map")->GetDepthMapResourceView());

	///////////////////////////////////////////////////////////////////
	// Build Blur Special Effect
	///////////////////////////////////////////////////////////////////
	if (FAILED(mShaderController->AddRenderFromQuadShader("Effect HBlur", (float)DirectXInstance::Instance()._pRenderWidth, (float)DirectXInstance::Instance()._pRenderHeight, L"DX11 Framework Horizontal Blur.fx", mInputLayoutBuilder.GetD3D11InputDescs("Layout 3"), DirectXInstance::Instance()._pd3dDevice)))
	{
		return E_FAIL;
	}

	mShaderController->GetRTTProcess("Effect HBlur")->AddSamplerState(_pSamplerLinear);
	mShaderController->GetRTTProcess("Effect HBlur")->AddShaderResource(mShaderController->GetRTTProcess("Parallax Scene")->GetShaderTargetTexture("ColourMap"));

	if (FAILED(mShaderController->AddRenderFromQuadShader("Effect VBlur", (float)DirectXInstance::Instance()._pRenderWidth, (float)DirectXInstance::Instance()._pRenderHeight, L"DX11 Framework Vertical Blur.fx", mInputLayoutBuilder.GetD3D11InputDescs("Layout 3"), DirectXInstance::Instance()._pd3dDevice)))
	{
		return E_FAIL;
	}

	mShaderController->GetRTTProcess("Effect VBlur")->AddSamplerState(_pSamplerLinear);
	mShaderController->GetRTTProcess("Effect VBlur")->AddShaderResource(mShaderController->GetRTTProcess("Effect HBlur")->GetShaderTargetTexture("OutputText"));

	///////////////////////////////////////////////////////////////////
	// Setup Final Pass
	///////////////////////////////////////////////////////////////////
	if (FAILED(mShaderController->AddRenderToBackBufferShader("Final Pass", (float)DirectXInstance::Instance()._pRenderWidth, (float)DirectXInstance::Instance()._pRenderHeight, L"DX11 Framework Render To Texture.fx", mInputLayoutBuilder.GetD3D11InputDescs("Layout 3"), DirectXInstance::Instance()._pd3dDevice)))
	{
		return E_FAIL;
	}

	mShaderController->GetRTTProcess("Final Pass")->AddSamplerState(_pSamplerLinear);
	mShaderController->GetRTTProcess("Final Pass")->AddShaderResource(mShaderController->GetRTTProcess("Effect VBlur")->GetShaderTargetTexture("OutputText"));

	mShaderController->GetRTTProcess("Final Pass")->SetClearColour(0.0f, 0.0f, 1.0f, 1.0f);

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

	std::vector<SceneLight*> sceneLights = mScene->GetSceneLights();

	int count = 0;
	for (auto& sl : sceneLights) // Load all the scene lights into the constant buffer
	{
		cb.lights[count] = sl->GetLight();
		count++;
	}

	cb.EyePosW = mScene->GetRenderCamera()->GetWorldPosition();
	cb.HasTexture = 0.0f;
	cb.HasNormalMap = 0.0f;
	cb.HasHeightMap = 0.0f;
	cb.screenWidth = DirectXInstance::Instance()._pRenderWidth;
	cb.screenHeight = DirectXInstance::Instance()._pRenderHeight;

	if (GUIController::_pSelfShadingOn)
		cb.selfShadowOn = 1.0f;
	else
		cb.selfShadowOn = 0.0f;

	mShaderController->Draw(&cb, sceneLights, mScene->GetGameObjectsInFrustumOctree());

	ImGui::Render();

    // Present our back buffer to our front buffer
	DirectXInstance::Instance()._pSwapChain->Present(0, 0);
}