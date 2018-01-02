#include "SceneBuilder.h"

Scene* SceneBuilder::BuildScene(string fileName, Geometry cubeGeometry, Material noSpecMaterial, TextureController* textureManager)
{
	//Load the file
	std::ifstream inFile(fileName);

	if (!inFile)
		throw "Could not load level: " + fileName;

	//Dump contents of file into a string
	std::string xmlContents;

	//Blocked out of preference
	{
		std::string line;
		while (std::getline(inFile, line))
			xmlContents += line;
	}

	//Convert string to rapidxml readable char*
	std::vector<char> xmlData = std::vector<char>(xmlContents.begin(), xmlContents.end());
	xmlData.push_back('\0');

	//Create a parsed document with &xmlData[0] which is the char*
	xml_document<> doc;
	doc.parse<parse_no_data_nodes>(&xmlData[0]);

	//Get the root node
	xml_node<>* root = doc.first_node();

	// Initalise the scene with the scene name
	Scene* scene = new Scene(string(root->first_attribute("name")->value()), cubeGeometry, noSpecMaterial);

	////////////////////////////////////////////////////
	// Build the camera
	////////////////////////////////////////////////////
	xml_node<>* gameCameraNode = root->first_node("CameraLight");
	BuildCamera(gameCameraNode, cubeGeometry, noSpecMaterial, textureManager, scene);

	////////////////////////////////////////////////////
	// Extract all the GameObjects
	////////////////////////////////////////////////////
	xml_node<>* gameObjectNode = root->first_node("GameObject");
	while (gameObjectNode)
	{
		// Build and add the gameobject to the scene
		scene->AddGameObject(BuildGameObject(gameObjectNode, cubeGeometry, noSpecMaterial, textureManager));

		// Move to the next gameobject in the XML file
		gameObjectNode = gameObjectNode->next_sibling("GameObject");
	}

	////////////////////////////////////////////////////
	// Extract all the SceneLights
	////////////////////////////////////////////////////
	xml_node<>* sceneLightNode = root->first_node("SceneLight");
	while (sceneLightNode)
	{
		scene->AddSceneLight(BuildSceneLight(sceneLightNode, cubeGeometry, noSpecMaterial, textureManager));

		sceneLightNode = sceneLightNode->next_sibling("SceneLight");
	}

	////////////////////////////////////////////////////
	// Extract all the SceneLights
	////////////////////////////////////////////////////
	xml_node<>* planeNode = root->first_node("Plane");
	while (planeNode)
	{
		scene->AddGameObjects(BuildPlane(planeNode, cubeGeometry, noSpecMaterial, textureManager));

		planeNode = planeNode->next_sibling("Plane");
	}

	return scene;
}

void SceneBuilder::BuildCamera(xml_node<>* node, Geometry cubeGeometry, Material material, TextureController* textureManager, Scene* scene)
{
	SceneCamera* sceneCameraWalk = new SceneCamera(0.01f, 2000.0f, (float)DX11AppHelper::_pRenderWidth, (float)DX11AppHelper::_pRenderHeight, false, "Walking Camera", cubeGeometry, material);

	// Set position
	xml_node<>* positionNode = node->first_node("Position");
	sceneCameraWalk->SetWorldPosition(XMFLOAT3(
		(float)atof(positionNode->first_attribute("x")->value()),
		(float)atof(positionNode->first_attribute("y")->value()),
		(float)atof(positionNode->first_attribute("z")->value())));

	SceneLight* sceneLight = new SceneLight(string(node->first_attribute("type")->value()),
		textureManager->GetTextureSet(string(node->first_attribute("texture")->value()))->texture,
		cubeGeometry, material);

	// Set ambient light
	xml_node<>* ambientLightNode = node->first_node("AmbientLight");
	sceneLight->SetAmbientLight(XMFLOAT4(
		(float)atof(ambientLightNode->first_attribute("x")->value()),
		(float)atof(ambientLightNode->first_attribute("y")->value()),
		(float)atof(ambientLightNode->first_attribute("z")->value()),
		(float)atof(ambientLightNode->first_attribute("w")->value())));

	// Set diffuse light
	xml_node<>* diffuseLightNode = node->first_node("DiffuseLight");
	sceneLight->SetDiffuseLight(XMFLOAT4(
		(float)atof(diffuseLightNode->first_attribute("x")->value()),
		(float)atof(diffuseLightNode->first_attribute("y")->value()),
		(float)atof(diffuseLightNode->first_attribute("z")->value()),
		(float)atof(diffuseLightNode->first_attribute("w")->value())));

	// Set specular light
	xml_node<>* specularLightNode = node->first_node("SpecularLight");
	sceneLight->SetSpecularLight(XMFLOAT4(
		(float)atof(specularLightNode->first_attribute("x")->value()),
		(float)atof(specularLightNode->first_attribute("y")->value()),
		(float)atof(specularLightNode->first_attribute("z")->value()),
		(float)atof(specularLightNode->first_attribute("w")->value())));

	// Set specular power
	xml_node<>* specularPowerNode = node->first_node("SpecularPower");
	sceneLight->SetSpecularPower((float)atof(specularPowerNode->first_attribute("value")->value()));

	// Set light vec w
	xml_node<>* lightVecWNode = node->first_node("LightVecW");
	sceneLight->SetWorldPosition(XMFLOAT3(
		(float)atof(lightVecWNode->first_attribute("x")->value()),
		(float)atof(lightVecWNode->first_attribute("y")->value()),
		(float)atof(lightVecWNode->first_attribute("z")->value())));

	// Set range
	xml_node<>* rangeNode = node->first_node("Range");
	sceneLight->SetRange((float)atof(rangeNode->first_attribute("value")->value()));

	// Set attenuation
	xml_node<>* attenuationNode = node->first_node("Attenuation");
	sceneLight->SetAttenuation(XMFLOAT3(
		(float)atof(attenuationNode->first_attribute("x")->value()),
		(float)atof(attenuationNode->first_attribute("y")->value()),
		(float)atof(attenuationNode->first_attribute("z")->value())));

	// Set cone
	xml_node<>* coneNode = node->first_node("Cone");
	sceneLight->SetCone((float)atof(coneNode->first_attribute("value")->value()));

	// Set direction
	xml_node<>* directionNode = node->first_node("Direction");
	sceneLight->SetWorldRotation(
		(float)atof(directionNode->first_attribute("x")->value()),
		(float)atof(directionNode->first_attribute("y")->value()),
		(float)atof(directionNode->first_attribute("z")->value()));

	// Set padding light amount
	xml_node<>* paddingLightAmountNode = node->first_node("PaddingLightAmount");
	sceneLight->SetPaddingLightAmount(XMFLOAT3(
		(float)atof(paddingLightAmountNode->first_attribute("x")->value()),
		(float)atof(paddingLightAmountNode->first_attribute("y")->value()),
		(float)atof(paddingLightAmountNode->first_attribute("z")->value())));

	// Set light on
	xml_node<>* lightOnNode = node->first_node("LightOn");
	sceneLight->SetLightOn((float)atof(lightOnNode->first_attribute("value")->value()));

	sceneLight->SetWorldPosition(sceneCameraWalk->GetWorldPosition());

	sceneLight->SetParent(sceneCameraWalk);
	sceneCameraWalk->AddChild(sceneLight);

	scene->SetWalkingCamera(sceneCameraWalk, sceneLight);
}

GameObject * SceneBuilder::BuildGameObject(xml_node<>* node, Geometry cubeGeometry, Material noSpecMaterial, TextureController* textureManager)
{
	// Initalise the new gameobject
	GameObject* gameObject = new GameObject(string(node->first_attribute("type")->value()), cubeGeometry, noSpecMaterial);

	// Set the gameobjects position
	xml_node<>* positionNode = node->first_node("Position");
	gameObject->SetWorldPosition((float)atof(positionNode->first_attribute("x")->value()),
		(float)atof(positionNode->first_attribute("y")->value()),
		(float)atof(positionNode->first_attribute("z")->value()));

	// Set the gameobjects scale
	xml_node<>* scaleNode = node->first_node("Scale");
	gameObject->SetWorldScale((float)atof(scaleNode->first_attribute("x")->value()),
		(float)atof(scaleNode->first_attribute("y")->value()),
		(float)atof(scaleNode->first_attribute("z")->value()));

	// Set the gameobjects rotation
	xml_node<>* rotationNode = node->first_node("Rotation");
	gameObject->SetWorldRotation((float)atof(rotationNode->first_attribute("x")->value()),
		(float)atof(rotationNode->first_attribute("y")->value()),
		(float)atof(rotationNode->first_attribute("z")->value()));

	// Set the gameobjects textures
	gameObject->SetTextures(textureManager->GetTextureSet(string(node->first_attribute("texture")->value())));

	return gameObject;
}

SceneLight * SceneBuilder::BuildSceneLight(xml_node<>* node, Geometry cubeGeometry, Material noSpecMaterial, TextureController* textureManager)
{
	SceneLight* sceneLight = new SceneLight(string(node->first_attribute("type")->value()),
		textureManager->GetTextureSet(string(node->first_attribute("texture")->value()))->texture,
		cubeGeometry, noSpecMaterial);

	// Set ambient light
	xml_node<>* ambientLightNode = node->first_node("AmbientLight");
	sceneLight->SetAmbientLight(XMFLOAT4(
		(float)atof(ambientLightNode->first_attribute("x")->value()),
		(float)atof(ambientLightNode->first_attribute("y")->value()),
		(float)atof(ambientLightNode->first_attribute("z")->value()),
		(float)atof(ambientLightNode->first_attribute("w")->value())));

	// Set diffuse light
	xml_node<>* diffuseLightNode = node->first_node("DiffuseLight");
	sceneLight->SetDiffuseLight(XMFLOAT4(
		(float)atof(diffuseLightNode->first_attribute("x")->value()),
		(float)atof(diffuseLightNode->first_attribute("y")->value()),
		(float)atof(diffuseLightNode->first_attribute("z")->value()),
		(float)atof(diffuseLightNode->first_attribute("w")->value())));

	// Set specular light
	xml_node<>* specularLightNode = node->first_node("SpecularLight");
	sceneLight->SetSpecularLight(XMFLOAT4(
		(float)atof(specularLightNode->first_attribute("x")->value()),
		(float)atof(specularLightNode->first_attribute("y")->value()),
		(float)atof(specularLightNode->first_attribute("z")->value()),
		(float)atof(specularLightNode->first_attribute("w")->value())));

	// Set specular power
	xml_node<>* specularPowerNode = node->first_node("SpecularPower");
	sceneLight->SetSpecularPower((float)atof(specularPowerNode->first_attribute("value")->value()));

	// Set light vec w
	xml_node<>* lightVecWNode = node->first_node("LightVecW");
	sceneLight->SetWorldPosition(XMFLOAT3(
		(float)atof(lightVecWNode->first_attribute("x")->value()),
		(float)atof(lightVecWNode->first_attribute("y")->value()),
		(float)atof(lightVecWNode->first_attribute("z")->value())));

	// Set range
	xml_node<>* rangeNode = node->first_node("Range");
	sceneLight->SetRange((float)atof(rangeNode->first_attribute("value")->value()));

	// Set attenuation
	xml_node<>* attenuationNode = node->first_node("Attenuation");
	sceneLight->SetAttenuation(XMFLOAT3(
		(float)atof(attenuationNode->first_attribute("x")->value()),
		(float)atof(attenuationNode->first_attribute("y")->value()),
		(float)atof(attenuationNode->first_attribute("z")->value())));

	// Set cone
	xml_node<>* coneNode = node->first_node("Cone");
	sceneLight->SetCone((float)atof(coneNode->first_attribute("value")->value()));

	// Set direction
	xml_node<>* directionNode = node->first_node("Direction");
	sceneLight->SetWorldRotation(
		(float)atof(directionNode->first_attribute("x")->value()),
		(float)atof(directionNode->first_attribute("y")->value()),
		(float)atof(directionNode->first_attribute("z")->value()));

	// Set padding light amount
	xml_node<>* paddingLightAmountNode = node->first_node("PaddingLightAmount");
	sceneLight->SetPaddingLightAmount(XMFLOAT3(
		(float)atof(paddingLightAmountNode->first_attribute("x")->value()),
		(float)atof(paddingLightAmountNode->first_attribute("y")->value()),
		(float)atof(paddingLightAmountNode->first_attribute("z")->value())));

	// Set light on
	xml_node<>* lightOnNode = node->first_node("LightOn");
	sceneLight->SetLightOn((float)atof(lightOnNode->first_attribute("value")->value()));

	sceneLight->SetWorldPosition(sceneLight->GetWorldPosition());

	return sceneLight;
}

vector<GameObject*> SceneBuilder::BuildPlane(xml_node<>* node, Geometry cubeGeometry, Material noSpecMaterial, TextureController* textureManager)
{
	string type = string(node->first_attribute("type")->value());
	string textureName = string(node->first_attribute("texture")->value());
	int width = atoi(node->first_attribute("width")->value());
	int depth = atoi(node->first_attribute("depth")->value());
	int height = atoi(node->first_attribute("height")->value());
	int step = atoi(node->first_attribute("step")->value());

	int midWidthIndex = width / 2;
	int midDepthIndex = depth / 2;
	int midHeightIndex = height / 2;

	xml_node<>* positionNode = node->first_node("Position");
	XMFLOAT3 position = XMFLOAT3(
		(float)atof(positionNode->first_attribute("x")->value()),
		(float)atof(positionNode->first_attribute("y")->value()),
		(float)atof(positionNode->first_attribute("z")->value()));

	xml_node<>* baseScaleNode = node->first_node("BaseScale");
	XMFLOAT3 baseScale = XMFLOAT3(
		(float)atof(baseScaleNode->first_attribute("x")->value()),
		(float)atof(baseScaleNode->first_attribute("y")->value()),
		(float)atof(baseScaleNode->first_attribute("z")->value()));

	xml_node<>* baseRotationNode = node->first_node("BaseRotation");
	XMFLOAT3 baseRotation = XMFLOAT3(
		(float)atof(baseRotationNode->first_attribute("x")->value()),
		(float)atof(baseRotationNode->first_attribute("y")->value()),
		(float)atof(baseRotationNode->first_attribute("z")->value()));

	vector<GameObject*>	gameObjects;

	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < depth; j++)
		{
			for (int k = 0; k < height; k++)
			{
				GameObject* go = new GameObject(type, cubeGeometry, noSpecMaterial);
				go->SetTextures(textureManager->GetTextureSet(textureName));
				go->SetWorldRotation(baseRotation.x, baseRotation.y, baseRotation.z);
				go->SetWorldScale(baseScale.x, baseScale.y, baseScale.z);
				go->SetWorldPosition(position.x + ((i - midWidthIndex) * step), position.y + ((k - midHeightIndex) * step), position.z + ((j - midDepthIndex) * step));

				gameObjects.push_back(go);
			}
		}
	}

	return gameObjects;
}
