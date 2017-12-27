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
	void UpdateLightCube(float timeSinceStart, float deltaTime);
	void ToggleLightOn();

	void HandleLightControls(float deltaTime);

	Light BuildCBLight();

	SceneLightData GetSceneLightData() { return mSceneLightData; }
	void SetSceneLightData(SceneLightData data) { mSceneLightData = data; }

	void SetLightOn(bool on) { mSceneLightData.LightOn = on; }

private:
	SceneLightData					mSceneLightData;
};

