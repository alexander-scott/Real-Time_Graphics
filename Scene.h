#pragma once

#include "GameObject.h"
#include "SceneLight.h"
#include "SceneCamera.h"
#include "GUIHandler.h"

using namespace std;

class Scene
{
public:
	Scene(string name);
	~Scene();

	void Update(float timeSinceStart, float deltaTime);

	string GetSceneName() { return mSceneName; }

	void AddGameObjects(vector<GameObject*> gos) { mGameObjects.insert(mGameObjects.end(), gos.begin(), gos.end()); }
	void AddGameObject(GameObject* go) { mGameObjects.push_back(go); }
	vector<GameObject*> GetGameObjects() { return mGameObjects; }
	GameObject* GetGameObject(int index) { return mGameObjects.at(index); }

	void AddSceneLight(SceneLight* sl) { mSceneLights.push_back(sl); mGameObjects.push_back(sl); }
	vector<SceneLight*> GetSceneLights() { return mSceneLights; }
	SceneLight* GetSceneLight(int index) { return mSceneLights.at(index); }

	void OnMouseMove(float x, float y);
	SceneCamera* GetCamera() { return mSceneCamera; }

private:
	void UpdateLightControls(float deltaTime);

	string						mSceneName;
	SceneCamera*				mSceneCamera;
	vector<GameObject*>			mGameObjects;
	vector<SceneLight*>			mSceneLights;
};