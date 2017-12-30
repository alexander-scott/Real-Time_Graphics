#include "SceneLight.h"

SceneLight::SceneLight(string type, ID3D11ShaderResourceView* texture, Geometry geometry, Material material) : GameObject(type, geometry, material)
{
	_pLightName = type;

	SetScale(0.1f, 0.1f, 0.1f);
	SetRotation(0.0f, 0.0f, 0.0f);
	SetTextureRV(texture);

	_pLightVecW = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);
}

SceneLight::~SceneLight()
{
}

void SceneLight::UpdateLight(float renderWidth, float renderHeight)
{
	// Update the Main Light
	XMFLOAT4 lightEyePos = XMFLOAT4(_pLightVecW.x, _pLightVecW.y, _pLightVecW.z, 1.0f);
	XMFLOAT4 lightEyeDir = XMFLOAT4(lightEyePos.x, lightEyePos.y - 1.0f, lightEyePos.z - 0.0001f, 1.0f);
	XMFLOAT4 lightUpPos = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

	XMVECTOR lightEyeVector = XMLoadFloat4(&lightEyePos);
	XMVECTOR lightAtVector = XMLoadFloat4(&lightEyeDir);
	XMVECTOR lightUpVector = XMLoadFloat4(&lightUpPos);

	// Initialise Light Matrices
	XMStoreFloat4x4(&_pView, XMMatrixLookAtLH(lightEyeVector, lightAtVector, lightUpVector));
	XMStoreFloat4x4(&_pProjection, XMMatrixPerspectiveFovLH(0.9f * XM_PI, (renderWidth / renderHeight), 0.01f, 100.0f));
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
	else
	{
		return;
	}

	SetPosition(_pLightVecW);
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

Light SceneLight::GetLight()
{
	Light newLight;

	XMMATRIX lightView = XMLoadFloat4x4(&GetView());
	XMMATRIX lightProjection = XMLoadFloat4x4(&GetProjection());

	newLight.View = XMMatrixTranspose(lightView);
	newLight.Projection = XMMatrixTranspose(lightProjection);
	newLight.AmbientLight = GetAmbientLight();
	newLight.DiffuseLight = GetDiffuseLight();
	newLight.SpecularLight = GetSpecularLight();
	newLight.SpecularPower = GetSpecularPower();
	newLight.LightVecW = GetLightVecW();
	newLight.paddingLightAmount = GetPaddingLightAmount();
	newLight.lightOn = GetLightOn();

	return newLight;
}