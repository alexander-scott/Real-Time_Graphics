#include "Scene.h"



Scene::Scene(string name) : mSceneName(name)
{
}


Scene::~Scene()
{
	for (auto go : mGameObjects)
	{
		delete go;
		go = nullptr;
	}
}
