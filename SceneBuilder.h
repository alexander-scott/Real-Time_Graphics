#pragma once

#include <fstream>

#include "Scene.h"

#include "rapidxml.hpp"

using namespace std;
using namespace rapidxml;

namespace SceneBuilder
{
	Scene* LoadScene(string fileName, Geometry cubeGeometry, Material noSpecMaterial);
};

