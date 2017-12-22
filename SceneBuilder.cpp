#include "SceneBuilder.h"

Scene* SceneBuilder::LoadScene(string fileName, Geometry cubeGeometry, Material noSpecMaterial)
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

	xml_node<>* gameObjectNode = root->first_node("GameObject");
	while (gameObjectNode)
	{
		// Initalise the new gameobject
		GameObject* gameObject = new GameObject(string(root->first_attribute("type")->value()), cubeGeometry, noSpecMaterial);

		// Set the gameobjects position
		xml_node<>* positionNode = gameObjectNode->first_node("Position");
		gameObject->SetPosition((float)atof(positionNode->first_attribute("x")->value()), 
								(float)atof(positionNode->first_attribute("y")->value()), 
								(float)atof(positionNode->first_attribute("z")->value()));

		// Set the gameobjects scale
		xml_node<>* scaleNode = gameObjectNode->first_node("Scale");
		gameObject->SetPosition((float)atof(scaleNode->first_attribute("x")->value()),
								(float)atof(scaleNode->first_attribute("y")->value()),
								(float)atof(scaleNode->first_attribute("z")->value()));

		// Set the gameobjects rotation
		xml_node<>* rotationNode = gameObjectNode->first_node("Rotation");
		gameObject->SetPosition((float)atof(rotationNode->first_attribute("x")->value()),
								(float)atof(rotationNode->first_attribute("y")->value()),
								(float)atof(rotationNode->first_attribute("z")->value()));

		// Set the gameobjects textures
		gameObject->SetTextures(TextureManager::_pTextureList[string(root->first_attribute("texture")->value())].get());

		// Add the gameobject to the scene
		scene->AddGameObject(gameObject);

		// Move to the next gameobject in the XML file
		gameObjectNode = gameObjectNode->next_sibling("GameObject");
	}

	return scene;
}
