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
	auto frustums = mSceneCamera->GetFrustumPlanes();
	std::vector<GameObject*> returnObjects;

	bool cull = false;

	for (int i = 0; i < mGameObjects2.size(); i++)
	{
		cull = false;
		// Loop through each frustum plane
		for (int planeID = 0; planeID < 6; ++planeID)
		{
			XMVECTOR planeNormal = XMVectorSet(frustums[planeID].x, frustums[planeID].y, frustums[planeID].z, 0.0f);
			float planeConstant = frustums[planeID].w;

			// Check each axis (x,y,z) to get the AABB vertex furthest away from the direction the plane is facing (plane normal)
			XMFLOAT3 axisVert;

			// x-axis
			if (frustums[planeID].x < 0.0f)    // Which AABB vertex is furthest down (plane normals direction) the x axis
				axisVert.x = mGameObjects2[i].Bounds.Center.x - mGameObjects2[i].Bounds.Extents.x; // min x plus tree positions x
			else
				axisVert.x = mGameObjects2[i].Bounds.Center.x + mGameObjects2[i].Bounds.Extents.x; // max x plus tree positions x

																		   // y-axis
			if (frustums[planeID].y < 0.0f)    // Which AABB vertex is furthest down (plane normals direction) the y axis
				axisVert.y = mGameObjects2[i].Bounds.Center.y - mGameObjects2[i].Bounds.Extents.y; // min y plus tree positions y
			else
				axisVert.y = mGameObjects2[i].Bounds.Center.y + mGameObjects2[i].Bounds.Extents.y; // max y plus tree positions y

																		   // z-axis
			if (frustums[planeID].z < 0.0f)    // Which AABB vertex is furthest down (plane normals direction) the z axis
				axisVert.z = mGameObjects2[i].Bounds.Center.z - mGameObjects2[i].Bounds.Extents.z; // min z plus tree positions z
			else
				axisVert.z = mGameObjects2[i].Bounds.Center.z + mGameObjects2[i].Bounds.Extents.z; // max z plus tree positions z

												// Now we get the signed distance from the AABB vertex that's furthest down the frustum planes normal,
												// and if the signed distance is negative, then the entire bounding box is behind the frustum plane, which means
												// that it should be culled
			if (XMVectorGetX(XMVector3Dot(planeNormal, XMLoadFloat3(&axisVert))) + planeConstant < 0.0f)
			{
				cull = true;
				// Skip remaining planes to check and move on to next tree
				break;
			}
		}

		if (!cull)
		{
			returnObjects.push_back(mGameObjects2[i].GameObject);
		}
	}

	return returnObjects;
}