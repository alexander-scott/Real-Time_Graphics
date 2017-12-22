#pragma once

#include "GameObject.h"

using namespace std;

class Scene
{
public:
	Scene(string name);
	~Scene();

	void Update(float timeSinceStart, float deltaTime);

	string GetSceneName() { return mSceneName; }

	void AddGameObject(GameObject* go) { mGameObjects.push_back(go); }
	vector<GameObject*> GetGameObjects() { return mGameObjects; }
	GameObject* GetGameObject(int index) { return mGameObjects.at(index); }

private:
	string						mSceneName;
	vector<GameObject*>			mGameObjects;
};