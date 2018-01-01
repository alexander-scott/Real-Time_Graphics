#pragma once

#include <fstream>

#include "Scene.h"

#include "rapidxml.hpp"

using namespace std;
using namespace rapidxml;

namespace SceneBuilder
{
	Scene* BuildScene(string fileName, Geometry cubeGeometry, Material noSpecMaterial, TextureController* textureManager);

	inline void BuildCamera(xml_node<>* node, Geometry cubeGeometry, Material noSpecMaterial, TextureController* textureManager, Scene* scene);
	inline GameObject* BuildGameObject(xml_node<>* node, Geometry cubeGeometry, Material noSpecMaterial, TextureController* textureManager);
	inline SceneLight* BuildSceneLight(xml_node<>* node, Geometry cubeGeometry, Material noSpecMaterial, TextureController* textureManager);
	inline vector<GameObject*> BuildPlane(xml_node<>* node, Geometry cubeGeometry, Material noSpecMaterial, TextureController* textureManager);
};

