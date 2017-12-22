#include "Scene.h"



Scene::Scene(string name) : mSceneName(name)
{
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
}

void Scene::Update(float timeSinceStart, float deltaTime)
{
	for (auto go : mGameObjects)
	{
		go->Update(timeSinceStart, deltaTime);
	}
}
