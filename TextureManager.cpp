#include "TextureManager.h"

unordered_map<string, unique_ptr<TextureSet>> TextureManager::_pTextureList;

TextureManager::TextureManager()
{
}


TextureManager::~TextureManager()
{
}

void TextureManager::AddTexture(ID3D11Device* d3dDevice, string textureSetName, LPCWSTR texturePath, LPCWSTR normalMapPath, LPCWSTR heightMapPath)
{
	ID3D11ShaderResourceView* texture = nullptr;
	ID3D11ShaderResourceView* normalMap = nullptr;
	ID3D11ShaderResourceView* heightMap = nullptr;

	if (texturePath != nullptr)
	{
		CreateDDSTextureFromFile(d3dDevice, texturePath, nullptr, &texture);
	}
	
	if (normalMapPath != nullptr)
	{
		CreateDDSTextureFromFile(d3dDevice, normalMapPath, nullptr, &normalMap);
	}

	if (heightMapPath != nullptr)
	{
		CreateDDSTextureFromFile(d3dDevice, heightMapPath, nullptr, &heightMap);
	}

	auto textureSet = std::make_unique<TextureSet>(textureSetName, texture, normalMap, heightMap);

	_pTextureList[textureSetName] = std::move(textureSet);
}