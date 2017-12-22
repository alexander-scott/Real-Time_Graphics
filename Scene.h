#pragma once

#include "GameObject.h"

using namespace std;

class Scene
{
public:
	Scene(string name);
	~Scene();

	string GetSceneName() { return mSceneName; }

	void AddGameObject(GameObject* go) { mGameObjects.push_back(go); }
	vector<GameObject*> GetGameObjects() { return mGameObjects; }

private:
	string						mSceneName;
	vector<GameObject*>			mGameObjects;
};

