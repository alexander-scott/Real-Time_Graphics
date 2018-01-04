#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "DDSTextureLoader.h"
#include "resource.h"

#include <vector>

#include "GameObject.h"

class SceneLight : public GameObject
{
public:
	SceneLight(string type, ID3D11ShaderResourceView* texture, Geometry geometry, Material material);
	~SceneLight();

	void UpdateLight(float renderWidth, float renderHeight);
	void ToggleLightOn();

	void HandleLightControls(float deltaTime);

	Light GetLight();

	string GetLightName() { return mLightName; }
	void SetLightName(string lightName) { mLightName = lightName; }

	XMFLOAT4X4 GetView() { return mViewMatrix; }
	void SetView(XMFLOAT4X4 newView) { mViewMatrix = newView; }

	XMFLOAT4X4 GetProjection() { return mProjectionMatrix; }
	void SetProjection(XMFLOAT4X4 newProjection) { mProjectionMatrix = newProjection; }

	XMFLOAT4X4 GetShadowTransform() { return mShadowTransMatrix; }
	void SetShadowTransform(XMFLOAT4X4 newShadowTransform) { mShadowTransMatrix = newShadowTransform; }

	XMFLOAT4 GetAmbientLight() { return mAmbientLight; }
	void SetAmbientLight(XMFLOAT4 newAmbientLight) { mAmbientLight = newAmbientLight; }

	XMFLOAT4 GetDiffuseLight() { return mDiffuseLight; }
	void SetDiffuseLight(XMFLOAT4 newDiffuseLight) { mDiffuseLight = newDiffuseLight; }

	XMFLOAT4 GetSpecularLight() { return mSpecularLight; }
	void SetSpecularLight(XMFLOAT4 newSpecularLight) { mSpecularLight = newSpecularLight; }

	float GetSpecularPower() { return mSpecularPower; }
	void SetSpecularPower(float newSpecularPower) { mSpecularPower = newSpecularPower; }

	float GetRange() { return mRange; }
	void SetRange(float range) { mRange = range; }

	XMFLOAT3 GetAttenuation() { return mAttenuation; }
	void SetAttenuation(XMFLOAT3 attenuation) { mAttenuation = attenuation; }

	float GetCone() { return mCone; }
	void SetCone(float cone) { mCone = cone; }

	XMFLOAT3 GetPaddingLightAmount() { return mPaddingLightAmount; }
	void SetPaddingLightAmount(XMFLOAT3 newPaddingLightAmount) { mPaddingLightAmount = newPaddingLightAmount; }

	float GetLightOn() { return mLightOn; }
	void SetLightOn(float newLightOn) { mLightOn = newLightOn; }

private:

	string							mLightName;

	XMFLOAT4X4						mViewMatrix;
	XMFLOAT4X4						mProjectionMatrix;
	XMFLOAT4X4						mShadowTransMatrix;

	XMFLOAT4						mAmbientLight;
	XMFLOAT4						mDiffuseLight;
	XMFLOAT4						mSpecularLight;

	float							mSpecularPower;

	float							mRange;
	XMFLOAT3						mAttenuation;

	float							mCone;

	XMFLOAT3						mPaddingLightAmount;
	float							mLightOn;
};

