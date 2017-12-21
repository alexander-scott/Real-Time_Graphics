#include "SceneLight.h"

SceneLight::SceneLight(string type, ID3D11ShaderResourceView* texture, Geometry geometry, Material material)
{
	_pLightName = type;

	_pLightCube = new GameObject(type, geometry, material);
	_pLightCube->SetScale(0.5f, 0.5f, 0.5f);
	_pLightCube->SetRotation(0.0f, 0.0f, 0.0f);
	_pLightCube->SetTextureRV(texture);

	_pSceneBounds.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
	_pSceneBounds.Radius = sqrtf(10.0f*10.0f + 15.0f*15.0f);

	_pLightVecW = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);
}


SceneLight::~SceneLight()
{
}

void SceneLight::UpdateLight(float renderWidth, float renderHeight)
{
	// Update the Main Light
	XMFLOAT4 lightEyePos = XMFLOAT4(_pLightVecW.x, _pLightVecW.y, _pLightVecW.z, 1.0f);
	XMFLOAT4 lightAtPos = XMFLOAT4(lightEyePos.x, lightEyePos.y - 1.0f, lightEyePos.z - 0.0001f, 1.0f);
	XMFLOAT4 lightUpPos = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

	XMVECTOR lightEyeVector = XMLoadFloat4(&lightEyePos);
	XMVECTOR lightAtVector = XMLoadFloat4(&lightAtPos);
	XMVECTOR lightUpVector = XMLoadFloat4(&lightUpPos);

	// Initialise Light Matrices
	XMStoreFloat4x4(&_pView, XMMatrixLookAtLH(lightEyeVector, lightAtVector, lightUpVector));
	XMStoreFloat4x4(&_pProjection, XMMatrixPerspectiveFovLH(0.6f * XM_PI, (renderWidth / renderHeight), 0.01f, 100.0f));
}


void SceneLight::UpdateLightCube(float timeSinceStart, float deltaTime)
{
	_pLightCube->Update(timeSinceStart, deltaTime);
}

void SceneLight::HandleLightControls(float deltaTime)
{
	// Move Basic Light along Z-axis
	if (GetAsyncKeyState(VK_LSHIFT) && GetAsyncKeyState('W'))
	{
		_pLightVecW.y += 0.01f * deltaTime;
	}
	else if (GetAsyncKeyState(VK_LSHIFT) && GetAsyncKeyState('S'))
	{
		_pLightVecW.y -= 0.01f * deltaTime;
	}
	else if (GetAsyncKeyState('W'))
	{
		_pLightVecW.z += 0.01f * deltaTime;
	}
	else if (GetAsyncKeyState('S'))
	{
		_pLightVecW.z -= 0.01f * deltaTime;
	}
	else if (GetAsyncKeyState('A'))
	{
		_pLightVecW.x -= 0.01f * deltaTime;
	}
	else if (GetAsyncKeyState('D'))
	{
		_pLightVecW.x += 0.01f * deltaTime;
	}

	_pLightCube->SetPosition(_pLightVecW);
}

void SceneLight::ToggleLightOn()
{
	if (_pLightOn == 1.0f)
	{
		_pLightOn = 0.0f;
	}
	else
	{
		_pLightOn = 1.0f;
	}
}