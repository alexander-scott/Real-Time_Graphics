#include "SceneLight.h"

SceneLight::SceneLight(string type, ID3D11ShaderResourceView* texture, Geometry geometry, Material material) : GameObject(type, geometry, material)
{
	mSceneLightData.LightName = type;

	SetScale(0.5f, 0.5f, 0.5f);
	SetRotation(0.0f, 0.0f, 0.0f);
	SetTextureRV(texture);

	mSceneLightData.LightVecW = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);
}

SceneLight::~SceneLight()
{
}

void SceneLight::UpdateLight(float renderWidth, float renderHeight)
{
	// Update the Main Light
	XMFLOAT4 lightEyePos = XMFLOAT4(mSceneLightData.LightVecW.x, mSceneLightData.LightVecW.y, mSceneLightData.LightVecW.z, 1.0f);
	XMFLOAT4 lightAtPos = XMFLOAT4(lightEyePos.x, lightEyePos.y - 1.0f, lightEyePos.z - 0.0001f, 1.0f);
	XMFLOAT4 lightUpPos = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

	XMVECTOR lightEyeVector = XMLoadFloat4(&lightEyePos);
	XMVECTOR lightAtVector = XMLoadFloat4(&lightAtPos);
	XMVECTOR lightUpVector = XMLoadFloat4(&lightUpPos);

	// Initialise Light Matrices
	XMStoreFloat4x4(&mSceneLightData.ViewMatrix, XMMatrixLookAtLH(lightEyeVector, lightAtVector, lightUpVector));
	XMStoreFloat4x4(&mSceneLightData.ProjectionMatrix, XMMatrixPerspectiveFovLH(0.6f * XM_PI, (renderWidth / renderHeight), 0.01f, 100.0f));
}


void SceneLight::UpdateLightCube(float timeSinceStart, float deltaTime)
{
	Update(timeSinceStart, deltaTime);
}

void SceneLight::HandleLightControls(float deltaTime)
{
	// Move Basic Light along Z-axis
	if (GetAsyncKeyState(VK_LSHIFT) && GetAsyncKeyState('W'))
	{
		mSceneLightData.LightVecW.y += 0.01f * deltaTime;
	}
	else if (GetAsyncKeyState(VK_LSHIFT) && GetAsyncKeyState('S'))
	{
		mSceneLightData.LightVecW.y -= 0.01f * deltaTime;
	}
	else if (GetAsyncKeyState('W'))
	{
		mSceneLightData.LightVecW.z += 0.01f * deltaTime;
	}
	else if (GetAsyncKeyState('S'))
	{
		mSceneLightData.LightVecW.z -= 0.01f * deltaTime;
	}
	else if (GetAsyncKeyState('A'))
	{
		mSceneLightData.LightVecW.x -= 0.01f * deltaTime;
	}
	else if (GetAsyncKeyState('D'))
	{
		mSceneLightData.LightVecW.x += 0.01f * deltaTime;
	}

	SetPosition(mSceneLightData.LightVecW);
}

void SceneLight::ToggleLightOn()
{
	if (mSceneLightData.LightOn == 1.0f)
	{
		mSceneLightData.LightOn = 0.0f;
	}
	else
	{
		mSceneLightData.LightOn = 1.0f;
	}
}

Light SceneLight::BuildCBLight()
{
	Light newLight;
	SceneLightData data = GetSceneLightData();

	XMMATRIX lightView = XMLoadFloat4x4(&data.ViewMatrix);
	XMMATRIX lightProjection = XMLoadFloat4x4(&data.ProjectionMatrix);

	newLight.View = XMMatrixTranspose(lightView);
	newLight.Projection = XMMatrixTranspose(lightProjection);
	newLight.AmbientLight = data.AmbientLight;
	newLight.DiffuseLight = data.DiffuseLight;
	newLight.SpecularLight = data.SpecularLight;
	newLight.SpecularPower = data.SpecularPower;
	newLight.LightVecW = data.LightVecW;
	newLight.paddingLightAmount = data.PaddingLightAmount;
	newLight.lightOn = data.LightOn;

	return newLight;
}