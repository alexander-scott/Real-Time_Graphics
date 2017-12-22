#include "SceneBuilder.h"

Scene* SceneBuilder::BuildScene(string fileName, Geometry cubeGeometry, Material noSpecMaterial)
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
	Scene* scene = new Scene(string(root->first_attribute("name")->value()));

	////////////////////////////////////////////////////
	// Extract all the GameObjects
	////////////////////////////////////////////////////
	xml_node<>* gameObjectNode = root->first_node("GameObject");
	while (gameObjectNode)
	{
		// Initalise the new gameobject
		GameObject* gameObject = new GameObject(string(gameObjectNode->first_attribute("type")->value()), cubeGeometry, noSpecMaterial);

		// Set the gameobjects position
		xml_node<>* positionNode = gameObjectNode->first_node("Position");
		gameObject->SetPosition((float)atof(positionNode->first_attribute("x")->value()), 
								(float)atof(positionNode->first_attribute("y")->value()), 
								(float)atof(positionNode->first_attribute("z")->value()));

		// Set the gameobjects scale
		xml_node<>* scaleNode = gameObjectNode->first_node("Scale");
		gameObject->SetScale((float)atof(scaleNode->first_attribute("x")->value()),
								(float)atof(scaleNode->first_attribute("y")->value()),
								(float)atof(scaleNode->first_attribute("z")->value()));

		// Set the gameobjects rotation
		xml_node<>* rotationNode = gameObjectNode->first_node("Rotation");
		gameObject->SetRotation((float)atof(rotationNode->first_attribute("x")->value()),
								(float)atof(rotationNode->first_attribute("y")->value()),
								(float)atof(rotationNode->first_attribute("z")->value()));

		// Set the gameobjects textures
		gameObject->SetTextures(TextureManager::_pTextureList[string(gameObjectNode->first_attribute("texture")->value())].get());

		// Add the gameobject to the scene
		scene->AddGameObject(gameObject);

		// Move to the next gameobject in the XML file
		gameObjectNode = gameObjectNode->next_sibling("GameObject");
	}

	////////////////////////////////////////////////////
	// Extract all the SceneLights
	////////////////////////////////////////////////////
	xml_node<>* sceneLightNode = root->first_node("SceneLight");
	while (sceneLightNode)
	{
		SceneLight* sceneLight = new SceneLight(string(sceneLightNode->first_attribute("type")->value()),
			TextureManager::_pTextureList[string(sceneLightNode->first_attribute("texture")->value())].get()->texture,
			cubeGeometry, noSpecMaterial);

		// Set ambient light
		xml_node<>* ambientLightNode = sceneLightNode->first_node("AmbientLight");
		sceneLight->SetAmbientLight(XMFLOAT4(
			(float)atof(ambientLightNode->first_attribute("x")->value()), 
			(float)atof(ambientLightNode->first_attribute("y")->value()), 
			(float)atof(ambientLightNode->first_attribute("z")->value()), 
			(float)atof(ambientLightNode->first_attribute("w")->value())));

		// Set diffuse light
		xml_node<>* diffuseLightNode = sceneLightNode->first_node("DiffuseLight");
		sceneLight->SetDiffuseLight(XMFLOAT4(
			(float)atof(diffuseLightNode->first_attribute("x")->value()),
			(float)atof(diffuseLightNode->first_attribute("y")->value()),
			(float)atof(diffuseLightNode->first_attribute("z")->value()),
			(float)atof(diffuseLightNode->first_attribute("w")->value())));

		// Set specular light
		xml_node<>* specularLightNode = sceneLightNode->first_node("SpecularLight");
		sceneLight->SetSpecularLight(XMFLOAT4(
			(float)atof(specularLightNode->first_attribute("x")->value()),
			(float)atof(specularLightNode->first_attribute("y")->value()),
			(float)atof(specularLightNode->first_attribute("z")->value()),
			(float)atof(specularLightNode->first_attribute("w")->value())));

		// Set specular power
		xml_node<>* specularPowerNode = sceneLightNode->first_node("SpecularPower");
		sceneLight->SetSpecularPower((float)atof(specularPowerNode->first_attribute("value")->value()));

		// Set light vec w
		xml_node<>* lightVecWNode = sceneLightNode->first_node("LightVecW");
		sceneLight->SetLightVecW(XMFLOAT3(
			(float)atof(lightVecWNode->first_attribute("x")->value()),
			(float)atof(lightVecWNode->first_attribute("y")->value()),
			(float)atof(lightVecWNode->first_attribute("z")->value())));

		// Set padding light amount
		xml_node<>* paddingLightAmountNode = sceneLightNode->first_node("PaddingLightAmount");
		sceneLight->SetPaddingLightAmount(XMFLOAT3(
			(float)atof(paddingLightAmountNode->first_attribute("x")->value()),
			(float)atof(paddingLightAmountNode->first_attribute("y")->value()),
			(float)atof(paddingLightAmountNode->first_attribute("z")->value())));

		// Set light on
		xml_node<>* lightOnNode = sceneLightNode->first_node("LightOn");
		sceneLight->SetLightOn((float)atof(lightOnNode->first_attribute("value")->value()));

		sceneLight->SetPosition(sceneLight->GetLightVecW());

		scene->AddSceneLight(sceneLight);

		sceneLightNode = sceneLightNode->next_sibling("SceneLight");
	}


	return scene;
}
