#include "SceneLight.h"

SceneLight::SceneLight(string type, ID3D11ShaderResourceView* texture, Geometry geometry, Material material) : GameObject(type, geometry, material)
{
	mLightName = type;

	SetWorldScale(0.2f, 0.2f, 0.2f);
	SetWorldRotation(0.0f, 0.0f, 0.0f);
	SetTextureRV(texture);
}

SceneLight::~SceneLight()
{
}

void SceneLight::UpdateLight(float renderWidth, float renderHeight)
{
	// Update the Main Light
	XMFLOAT4 lightEyePos = XMFLOAT4(GetWorldPosition().x, GetWorldPosition().y, GetWorldPosition().z, 1.0f);
	XMFLOAT4 lightEyeDir = XMFLOAT4(lightEyePos.x, lightEyePos.y - 1.0f, lightEyePos.z - 0.0001f, 1.0f);
	XMFLOAT4 lightUpPos = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);

	XMVECTOR lightEyeVector = XMLoadFloat4(&lightEyePos);
	XMVECTOR lightAtVector = XMLoadFloat4(&lightEyeDir);
	XMVECTOR lightUpVector = XMLoadFloat4(&lightUpPos);

	// Initialise Light Matrices
	XMStoreFloat4x4(&mViewMatrix, XMMatrixLookAtLH(lightEyeVector, lightAtVector, lightUpVector));
	XMStoreFloat4x4(&mProjectionMatrix, XMMatrixPerspectiveFovLH(2 * XM_PI, (renderWidth / renderHeight), 0.01f, 100.0f));
}

void SceneLight::HandleLightControls(float deltaTime)
{
	// Move Basic Light along Z-axis
	if (GetAsyncKeyState(VK_LSHIFT) && GetAsyncKeyState('W'))
	{
		SetWorldPosition(GetWorldPosition().x, GetWorldPosition().y + 0.01f * deltaTime, GetWorldPosition().z);
	}
	else if (GetAsyncKeyState(VK_LSHIFT) && GetAsyncKeyState('S'))
	{
		SetWorldPosition(GetWorldPosition().x, GetWorldPosition().y - 0.01f * deltaTime, GetWorldPosition().z);
	}
	else if (GetAsyncKeyState('W'))
	{
		SetWorldPosition(GetWorldPosition().x, GetWorldPosition().y, GetWorldPosition().z + 0.01f * deltaTime);
	}
	else if (GetAsyncKeyState('S'))
	{
		SetWorldPosition(GetWorldPosition().x, GetWorldPosition().y, GetWorldPosition().z - 0.01f * deltaTime);
	}
	else if (GetAsyncKeyState('A'))
	{
		SetWorldPosition(GetWorldPosition().x - 0.01f * deltaTime, GetWorldPosition().y, GetWorldPosition().z);
	}
	else if (GetAsyncKeyState('D'))
	{
		SetWorldPosition(GetWorldPosition().x + 0.01f * deltaTime, GetWorldPosition().y, GetWorldPosition().z);
	}
	else
	{
		return;
	}

	//SetPosition(_pLightVecW);
}

void SceneLight::ToggleLightOn()
{
	if (mLightOn == 1.0f)
	{
		mLightOn = 0.0f;
	}
	else
	{
		mLightOn = 1.0f;
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
	newLight.LightVecW = GetWorldPosition();
	newLight.Range = GetRange();
	newLight.Attenuation = GetAttenuation();
	newLight.Cone = GetCone();
	newLight.Direction = GetWorldRotation();
	newLight.paddingLightAmount = GetPaddingLightAmount();
	newLight.lightOn = GetLightOn();

	return newLight;
}