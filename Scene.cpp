#include "Scene.h"



Scene::Scene(string name) : mSceneName(name)
{
	XMFLOAT3 eye = XMFLOAT3(35.0f, 15.0f, -35.0f);
	mSceneCamera = new SceneCamera(0.01f, 200.0f, DX11AppHelper::_pRenderWidth, DX11AppHelper::_pRenderHeight);
	mSceneCamera->SetPosition(eye);

	mOctree = new Octree(100, XMFLOAT3(0, 0, 0), 25);
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

	if (GUIHandler::_pTestButton)
	{
		GUIHandler::_pTestButton = false;

		/*Bounds b(XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
		std::vector<GameObject*> gameobjs = mOctree->GetGameObjectsInBounds(b);
		int count = gameobjs.size();*/

		XMFLOAT3 dir, pos;
		XMStoreFloat3(&pos, mSceneCamera->GetPosition());
		XMStoreFloat3(&dir, mSceneCamera->GetForwardDirection());
		std::vector<GameObject*> gameobjs = mOctree->GetGameObjectsInRay(pos, dir);
		int count = gameobjs.size();
	}
}

void Scene::AddGameObjects(vector<GameObject*> gos)
{
	for (auto go : gos)
	{
		OctreeItem obj;
		obj.GameObject = go;
		obj.Bounds = Bounds(go->GetPosition(), XMFLOAT3(10 * go->GetScale().x, 10 * go->GetScale().y, 10 * go->GetScale().z));

		mOctree->Add(obj);

		mGameObjects.push_back(go);
	}
}

void Scene::AddGameObject(GameObject * go)
{
	OctreeItem obj;
	obj.GameObject = go;
	obj.Bounds = Bounds(go->GetPosition(), XMFLOAT3(10 * go->GetScale().x, 10 * go->GetScale().y, 10 * go->GetScale().z));

	mOctree->Add(obj);

	mGameObjects.push_back(go);
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
