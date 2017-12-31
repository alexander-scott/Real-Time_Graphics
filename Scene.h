#pragma once

#include "GameObject.h"
#include "SceneLight.h"
#include "SceneCamera.h"
#include "GUIController.h"
#include "Octree.h"

using namespace std;

class Scene
{
public:
	Scene(string name, Geometry cubeGeometry, Material cubeMat);
	~Scene();

	void Update(float timeSinceStart, float deltaTime);

	// Add game objects to the scene from the scene builder
	void AddGameObjects(vector<GameObject*> gos);
	void AddGameObject(GameObject* go);

	// Get a gameobject at a specific index
	GameObject* GetGameObject(int index) { return mOctreeGameObjects.at(index).GameObject; }

	// Add a scene light to the sceen from the scene builder
	void AddSceneLight(SceneLight* sl);

	// Get scene lights
	vector<SceneLight*> GetSceneLights() { return mSceneLights; }
	SceneLight* GetSceneLight(int index) { return mSceneLights.at(index); }

	// Called from the main windows message loop and used to update camera rotation based on mouse movement
	void OnMouseMove(float x, float y);

	// Get the current active render camera
	SceneCamera* GetRenderCamera();

	void SetWalkingCamera(SceneCamera* cam, SceneLight* camLight);

	// Get all the game objects in a camera frustum
	std::vector<GameObject*> GetGameObjectsInFrustumOctree();
	std::vector<GameObject*> GetGameObjectsInFrustum();

private:
	void UpdateLightControls(float deltaTime);

	string						mSceneName;
	SceneCamera*				mSceneCameraWalk;
	SceneCamera*				mSceneCameraFly;
	Octree*						mOctree;

	Geometry					mCubeGeometry;
	Material					mNoSpecMaterial;

	vector<SceneLight*>			mSceneLights;

	vector<OctreeItem>			mOctreeGameObjects; // Objects that will exist in the octree
	vector<GameObject*>			mGameObjects; // Objects that need updating but aren't in the octree

	bool						mFlyCameraActive;
	bool						mSwitchCameraPressed;
};