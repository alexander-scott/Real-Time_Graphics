#include "SceneLight.h"

SceneLight::SceneLight(string type, ID3D11ShaderResourceView* texture, Geometry geometry, Material material) : GameObject(type, geometry, material)
{
	_pLightName = type;

	SetScale(0.5f, 0.5f, 0.5f);
	SetRotation(0.0f, 0.0f, 0.0f);
	SetTextureRV(texture);
}

SceneLight::~SceneLight()
{
}

void SceneLight::UpdateLight(float renderWidth, float renderHeight)
{
	// Update the Main Light
	XMFLOAT4 lightEyePos = XMFLOAT4(GetPosition().x, GetPosition().y, GetPosition().z, 1.0f);
	XMFLOAT4 lightEyeDir = XMFLOAT4(lightEyePos.x, lightEyePos.y - 1.0f, lightEyePos.z - 0.0001f, 1.0f);
	XMFLOAT4 lightUpPos = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

	XMVECTOR lightEyeVector = XMLoadFloat4(&lightEyePos);
	XMVECTOR lightAtVector = XMLoadFloat4(&lightEyeDir);
	XMVECTOR lightUpVector = XMLoadFloat4(&lightUpPos);

	// Initialise Light Matrices
	XMStoreFloat4x4(&_pView, XMMatrixLookAtLH(lightEyeVector, lightAtVector, lightUpVector));
	XMStoreFloat4x4(&_pProjection, XMMatrixPerspectiveFovLH(2 * XM_PI, (renderWidth / renderHeight), 0.01f, 100.0f));
}

void SceneLight::HandleLightControls(float deltaTime)
{
	// Move Basic Light along Z-axis
	if (GetAsyncKeyState(VK_LSHIFT) && GetAsyncKeyState('W'))
	{
		SetPosition(GetPosition().x, GetPosition().y + 0.01f * deltaTime, GetPosition().z);
	}
	else if (GetAsyncKeyState(VK_LSHIFT) && GetAsyncKeyState('S'))
	{
		SetPosition(GetPosition().x, GetPosition().y - 0.01f * deltaTime, GetPosition().z);
	}
	else if (GetAsyncKeyState('W'))
	{
		SetPosition(GetPosition().x, GetPosition().y, GetPosition().z + 0.01f * deltaTime);
	}
	else if (GetAsyncKeyState('S'))
	{
		SetPosition(GetPosition().x, GetPosition().y, GetPosition().z - 0.01f * deltaTime);
	}
	else if (GetAsyncKeyState('A'))
	{
		SetPosition(GetPosition().x - 0.01f * deltaTime, GetPosition().y, GetPosition().z);
	}
	else if (GetAsyncKeyState('D'))
	{
		SetPosition(GetPosition().x + 0.01f * deltaTime, GetPosition().y, GetPosition().z);
	}
	else
	{
		return;
	}

	//SetPosition(_pLightVecW);
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
	newLight.LightVecW = GetPosition();
	newLight.Range = GetRange();
	newLight.Attenuation = GetAttenuation();
	newLight.Cone = GetCone();
	newLight.Direction = GetRotation();
	newLight.paddingLightAmount = GetPaddingLightAmount();
	newLight.lightOn = GetLightOn();

	return newLight;
}