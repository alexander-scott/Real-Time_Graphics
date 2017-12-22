#include "Scene.h"



Scene::Scene(string name) : mSceneName(name)
{
	XMFLOAT3 eye = XMFLOAT3(35.0f, 15.0f, -35.0f);
	mSceneCamera = new SceneCamera(0.01f, 200.0f, DX11AppHelper::_pRenderWidth, DX11AppHelper::_pRenderHeight);
	mSceneCamera->SetPosition(eye);
}


Scene::~Scene()
{
	for (auto go : mGameObjects)
	{
		if (go)
		{
			delete go;
			go = nullptr;
		}
	}

	if (mSceneCamera)
	{
		delete mSceneCamera;
		mSceneCamera = nullptr;
	}
}

void Scene::Update(float timeSinceStart, float deltaTime)
{
	mSceneCamera->UpdateCameraView();

	for (auto go : mGameObjects)
	{
		go->Update(timeSinceStart, deltaTime);
	}

	UpdateLightControls(deltaTime);

	for (auto sl : mSceneLights)
	{
		sl->UpdateLightCube(timeSinceStart, deltaTime);
		sl->UpdateLight((float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight);
	}
}

void Scene::UpdateLightControls(float deltaTime) 
{
	mSceneLights.at(GUIHandler::_pControlledLight)->HandleLightControls(deltaTime);

	// Toggle Lights ON/OFF
	if (GUIHandler::_pWhiteLightOn)
	{
		mSceneLights.at(0)->SetLightOn(true);
	}
	else
	{
		mSceneLights.at(0)->SetLightOn(false);
	}

	if (GUIHandler::_pRedLightOn)
	{
		mSceneLights.at(1)->SetLightOn(true);
	}
	else
	{
		mSceneLights.at(1)->SetLightOn(false);
	}

	if (GUIHandler::_pGreenLightOn)
	{
		mSceneLights.at(2)->SetLightOn(true);
	}
	else
	{
		mSceneLights.at(2)->SetLightOn(false);
	}

	if (GUIHandler::_pBlueLightOn)
	{
		mSceneLights.at(3)->SetLightOn(true);
	}
	else
	{
		mSceneLights.at(3)->SetLightOn(false);
	}
}

void Scene::OnMouseMove(float x, float y)
{
	mSceneCamera->OnMouseMove(x, y);
}
