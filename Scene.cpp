#include "Scene.h"



Scene::Scene(string name) : mSceneName(name)
{
	XMFLOAT3 eye = XMFLOAT3(35.0f, 15.0f, -35.0f);
	mSceneCamera = new SceneCamera(0.01f, 2000.0f, DX11AppHelper::_pRenderWidth, DX11AppHelper::_pRenderHeight);
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

		/*XMFLOAT3 dir, pos;
		XMStoreFloat3(&pos, mSceneCamera->GetPosition());
		XMStoreFloat3(&dir, mSceneCamera->GetForwardDirection());
		std::vector<GameObject*> gameobjs = mOctree->GetGameObjectsInRay(pos, dir);
		int count = gameobjs.size();*/

		/*std::vector<GameObject*> gameobjs = mOctree->GetGameObjectsInFrustums(mSceneCamera->GetFrustumPlanes());
		int count = gameobjs.size();*/
	}
}

void Scene::AddGameObjects(vector<GameObject*> gos)
{
	for (auto go : gos)
	{
		OctreeItem obj;
		obj.GameObject = go;
		obj.Bounds = BoundingBox(go->GetPosition(), XMFLOAT3(2 * go->GetScale().x, 2 * go->GetScale().y, 2 * go->GetScale().z));

		mOctree->Add(obj);

		mGameObjects.push_back(go);
		mGameObjects2.push_back(obj);
	}
}

void Scene::AddGameObject(GameObject * go)
{
	OctreeItem obj;
	obj.GameObject = go;
	obj.Bounds = BoundingBox(go->GetPosition(), XMFLOAT3(2 * go->GetScale().x, 2 * go->GetScale().y, 2 * go->GetScale().z));

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

std::vector<GameObject*> Scene::GetGameObjectsInFrustumOctree()
{
	return mOctree->GetGameObjectsInFrustums(mSceneCamera->GetFrustumPlanes());
}

std::vector<GameObject*> Scene::GetGameObjectsInFrustum()
{
	std::vector<GameObject*> returnObjs;

	XMVECTOR detView = XMMatrixDeterminant(mSceneCamera->GetViewMatrix());
	XMMATRIX invView = XMMatrixInverse(&detView, mSceneCamera->GetViewMatrix());

	for (int i = 0; i < mGameObjects2.size(); i++)
	{
		XMMATRIX W = mGameObjects2[i].GameObject->GetWorldMatrix();
		XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

		// View space to the object's local space.
		XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

		BoundingFrustum localSpaceFrustum;
		mSceneCamera->GetBoundingFrustum().Transform(localSpaceFrustum, toLocal);

		if (localSpaceFrustum.Intersects(mGameObjects2[i].Bounds))
		{
			returnObjs.push_back(mGameObjects2[i].GameObject);
		}
	}

	return returnObjs;
}
