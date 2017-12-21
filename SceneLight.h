#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "DDSTextureLoader.h"
#include "resource.h"
#include "CameraMouse.h"

#include <vector>

#include "GameObject.h"

class SceneLight
{

	struct BoundingSphere
	{
		BoundingSphere() : Center(0.0f, 0.0f, 0.0f), Radius(0.0f) {}
		XMFLOAT3 Center;
		float Radius;
	};

public:
	SceneLight(string type, ID3D11ShaderResourceView* texture, Geometry geometry, Material material);
	~SceneLight();

	void UpdateLight(float renderWidth, float renderHeight);
	void UpdateLightCube(float timeSinceStart, float deltaTime);
	void ToggleLightOn();

#pragma region Move Light

	void HandleLightControls(float deltaTime);

#pragma endregion


#pragma region Get/Set Functions

	string GetLightName() { return _pLightName; }
	void SetLightName(string lightName) { _pLightName = lightName; }

	GameObject* GetLightCubeGO() { return _pLightCube; }
	void SetLightCubeGO(GameObject* newLightCube) { _pLightCube = newLightCube; }

	XMFLOAT4X4 GetView() { return _pView; }
	void SetView(XMFLOAT4X4 newView) { _pView = newView; }

	XMFLOAT4X4 GetProjection() { return _pProjection; }
	void SetProjection(XMFLOAT4X4 newProjection) { _pProjection = newProjection; }

	XMFLOAT4X4 GetShadowTransform() { return _pShadowTransform; }
	void SetShadowTransform(XMFLOAT4X4 newShadowTransform) { _pShadowTransform = newShadowTransform; }

	XMFLOAT4 GetAmbientLight() { return _pAmbientLight; }
	void SetAmbientLight(XMFLOAT4 newAmbientLight) { _pAmbientLight = newAmbientLight; }

	XMFLOAT4 GetDiffuseLight() { return _pDiffuseLight; }
	void SetDiffuseLight(XMFLOAT4 newDiffuseLight) { _pDiffuseLight = newDiffuseLight; }

	XMFLOAT4 GetSpecularLight() { return _pSpecularLight; }
	void SetSpecularLight(XMFLOAT4 newSpecularLight) { _pSpecularLight = newSpecularLight; }

	float GetSpecularPower() { return _pSpecularPower; }
	void SetSpecularPower(float newSpecularPower) { _pSpecularPower = newSpecularPower; }

	XMFLOAT3 GetLightVecW() { return _pLightVecW; }
	void SetLightVecW(XMFLOAT3 newLightVecW) { _pLightVecW = newLightVecW; }

	XMFLOAT3 GetPaddingLightAmount() { return _pPaddingLightAmount; }
	void SetPaddingLightAmount(XMFLOAT3 newPaddingLightAmount) { _pPaddingLightAmount = newPaddingLightAmount; }

	float GetLightOn() { return _pLightOn; }
	void SetLightOn(float newLightOn) { _pLightOn = newLightOn; }

#pragma endregion


private:

	string _pLightName;

	GameObject* _pLightCube;

	XMFLOAT4X4 _pView;
	XMFLOAT4X4 _pProjection;
	XMFLOAT4X4 _pShadowTransform;

	XMFLOAT4 _pAmbientLight;
	XMFLOAT4 _pDiffuseLight;
	XMFLOAT4 _pSpecularLight;

	float _pSpecularPower;
	XMFLOAT3 _pLightVecW;

	XMFLOAT3 _pPaddingLightAmount;
	float _pLightOn;

	BoundingSphere _pSceneBounds;
};

