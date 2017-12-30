#include "Scene.h"

Scene::Scene(string name, Geometry cubeGeometry, Material cubeMat) : mSceneName(name), mCubeGeometry(cubeGeometry), mNoSpecMaterial(cubeMat)
{
	// Setup the walking camera
	XMFLOAT3 eyeWalking = XMFLOAT3(0.0f, 5.0f, 0.0f);
	mSceneCameraWalk = new SceneCamera(0.01f, 2000.0f, (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, false, "Walking Camera", mCubeGeometry, mNoSpecMaterial);
	mSceneCameraWalk->SetPosition(eyeWalking);

	// Setup the flying camera
	XMFLOAT3 eyeFly = XMFLOAT3(35.0f, 15.0f, -35.0f);
	mSceneCameraFly = new SceneCamera(0.01f, 2000.0f, (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, true, "Flying Camera", mCubeGeometry, mNoSpecMaterial);
	mSceneCameraFly->SetPosition(eyeFly);

	mOctree = new Octree(200, XMFLOAT3(0, 0, 0), 25);

	OctreeItem obj;
	obj.GameObject = mSceneCameraWalk;
	obj.Bounds = BoundingBox(mSceneCameraWalk->GetPosition(), XMFLOAT3(2 * mSceneCameraWalk->GetScale().x, 2 * mSceneCameraWalk->GetScale().y, 2 * mSceneCameraWalk->GetScale().z));
	mOctree->Add(obj);
	mOctreeGameObjects.push_back(obj);

	mFlyCameraActive = false;
	mSwitchCameraPressed = false;
}


Scene::~Scene()
{
	for (auto& go : mOctreeGameObjects)
	{
		if (go.GameObject)
		{
			delete go.GameObject;
			go.GameObject = nullptr;
		}
	}

	/*if (mSceneCameraWalk)
	{
		delete mSceneCameraWalk;
		mSceneCameraWalk = nullptr;
	}*/

	if (mSceneCameraFly)
	{
		delete mSceneCameraFly;
		mSceneCameraFly = nullptr;
	}
}

void Scene::Update(float timeSinceStart, float deltaTime)
{
	UpdateLightControls(deltaTime);

	// Update the view matrix of the current render camera
	if (mFlyCameraActive)
		mSceneCameraFly->UpdateCameraViewMatrix();
	else
		mSceneCameraWalk->UpdateCameraViewMatrix();

	for (auto& go : mOctreeGameObjects)
	{
		// If this returns true it means the object's position/rotation/scale has changed so we need to update it in the octree
		if (go.GameObject->Update(timeSinceStart, deltaTime))
		{
			mOctree->Remove(go);
			go.Bounds = BoundingBox(go.GameObject->GetPosition(), XMFLOAT3(2 * go.GameObject->GetScale().x, 2 * go.GameObject->GetScale().y, 2 * go.GameObject->GetScale().z));
			mOctree->Add(go);
		}
	}

	for (auto& sl : mSceneLights)
	{
		sl->UpdateLight((float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight);
	}

	// This if statement can be used to test functionality through a GUI click event
	if (GUIController::_pTestButton)
	{
		GUIController::_pTestButton = false;
	}
}

void Scene::AddGameObjects(vector<GameObject*> gos)
{
	for (auto& go : gos)
	{
		OctreeItem obj;
		obj.GameObject = go;
		obj.Bounds = BoundingBox(go->GetPosition(), XMFLOAT3(2 * go->GetScale().x, 2 * go->GetScale().y, 2 * go->GetScale().z));

		mOctree->Add(obj);

		mOctreeGameObjects.push_back(obj);
	}
}

void Scene::AddGameObject(GameObject * go)
{
	OctreeItem obj;
	obj.GameObject = go;
	obj.Bounds = BoundingBox(go->GetPosition(), XMFLOAT3(2 * go->GetScale().x, 2 * go->GetScale().y, 2 * go->GetScale().z));

	mOctree->Add(obj);

	mOctreeGameObjects.push_back(obj);
}

void Scene::AddSceneLight(SceneLight* sl)
{
	OctreeItem obj;
	obj.GameObject = sl;
	obj.Bounds = BoundingBox(sl->GetPosition(), XMFLOAT3(2 * sl->GetScale().x, 2 * sl->GetScale().y, 2 * sl->GetScale().z));

	mOctree->Add(obj);

	mSceneLights.push_back(sl); 

	mOctreeGameObjects.push_back(obj);
}

void Scene::UpdateLightControls(float deltaTime) 
{
	mSceneLights.at(GUIController::_pControlledLight)->HandleLightControls(deltaTime);

	// Toggle Lights ON/OFF
	if (GUIController::_pWhiteLightOn)
		mSceneLights.at(0)->SetLightOn(true);
	else
		mSceneLights.at(0)->SetLightOn(false);

	if (GUIController::_pRedLightOn)
		mSceneLights.at(1)->SetLightOn(true);
	else
		mSceneLights.at(1)->SetLightOn(false);

	if (GUIController::_pGreenLightOn)
		mSceneLights.at(2)->SetLightOn(true);
	else
		mSceneLights.at(2)->SetLightOn(false);

	if (GUIController::_pBlueLightOn)
		mSceneLights.at(3)->SetLightOn(true);
	else
		mSceneLights.at(3)->SetLightOn(false);

	// If C is pressed swap the render camera
	if (GetAsyncKeyState('C') && !mSwitchCameraPressed)
	{
		mSwitchCameraPressed = true;
		mFlyCameraActive = !mFlyCameraActive;
	}
	else if (!GetAsyncKeyState('C'))
	{
		mSwitchCameraPressed = false;
	}
}

void Scene::OnMouseMove(float x, float y)
{
	if (mFlyCameraActive)
		mSceneCameraFly->OnMouseMove(x, y);
	else
		mSceneCameraWalk->OnMouseMove(x, y);
}

SceneCamera * Scene::GetRenderCamera()
{
	if (mFlyCameraActive)
		return mSceneCameraFly;
	else
		return mSceneCameraWalk;
}

std::vector<GameObject*> Scene::GetGameObjectsInFrustumOctree()
{
	return mOctree->GetGameObjectsInBoundingFrustum(mSceneCameraWalk->GetBoundingFrustum());
}

std::vector<GameObject*> Scene::GetGameObjectsInFrustum()
{
	std::vector<GameObject*> renderedObjs;

	// Get the walking cameras frustum
	BoundingFrustum cameraFrustum = mSceneCameraWalk->GetBoundingFrustum();

	for (int i = 0; i < mOctreeGameObjects.size(); i++)
	{
		// If the gameobject intersects the gameobjects bounds the it can be seen
		if (cameraFrustum.Intersects(mOctreeGameObjects[i].Bounds))
		{
			// Add it to the renderedObjects vector
			renderedObjs.push_back(mOctreeGameObjects[i].GameObject);
		}
	}

	return renderedObjs;
}
