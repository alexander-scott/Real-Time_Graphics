#pragma once

#include <directxmath.h>
#include <d3d11_1.h>
#include <string>

#include <vector>

#include "TextureManager.h"

using namespace DirectX;
using namespace std;

struct Geometry
{
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	int numberOfIndices;

	UINT vertexBufferStride;
	UINT vertexBufferOffset;
};

struct Material
{
	XMFLOAT4 diffuse;
	XMFLOAT4 ambient;
	XMFLOAT4 specular;
	float specularPower;
};

class GameObject
{
public:
	GameObject(string type, Geometry geometry, Material material);
	~GameObject();

	// Setters and Getters for position/rotation/scale
	void SetPosition(XMFLOAT3 position) { _position = position; }
	void SetPosition(float x, float y, float z) { _position.x = x; _position.y = y; _position.z = z; }

	XMFLOAT3 GetPosition() const { return _position; }

	void SetScale(XMFLOAT3 scale) { _scale = scale; }
	void SetScale(float x, float y, float z) { _scale.x = x; _scale.y = y; _scale.z = z; }

	XMFLOAT3 GetScale() const { return _scale; }

	void SetRotation(XMFLOAT3 rotation) { _rotation = rotation; }
	void SetRotation(float x, float y, float z) { _rotation.x = x; _rotation.y = y; _rotation.z = z; }

	XMFLOAT3 GetRotation() const { return _rotation; }

	string GetType() const { return _type; }

	Geometry GetGeometryData() const { return _geometry; }

	Material GetMaterial() const { return _material; }

	XMMATRIX GetWorldMatrix() const { return XMLoadFloat4x4(&_world); }

	void SetTextures(TextureSet* textureSet);

	void SetTextureRV(ID3D11ShaderResourceView* textureRV) { _textureRV = textureRV; }
	ID3D11ShaderResourceView* GetTextureRV() const { return _textureRV; }
	bool HasTexture() const { return _textureRV ? true : false; }

	void SetNormalMap(ID3D11ShaderResourceView* normalMap) { _normalMap = normalMap; }
	ID3D11ShaderResourceView* GetNormalMap() const { return _normalMap; }
	bool HasNormalMap() const { return _normalMap ? true : false; }

	void SetHeightMap(ID3D11ShaderResourceView* heightMap) { _heightMap = heightMap; }
	ID3D11ShaderResourceView* GetHeightMap() const { return _heightMap; }
	bool HasHeightMap() const { return _heightMap ? true : false; }

	void SetParent(GameObject* parent) { _parent = parent; }

	void Update(float t, float deltaTime);
	void Draw(ID3D11DeviceContext* pImmediateContext);

	void HandleControls();
	void UpdateRotation(float deltaTime);

private:
	XMFLOAT3 _position;
	XMFLOAT3 _rotation;
	XMFLOAT3 _scale;

	string _type;

	XMFLOAT4X4 _world;

	Geometry _geometry;
	Material _material;

	ID3D11ShaderResourceView* _textureRV;
	ID3D11ShaderResourceView* _normalMap;
	ID3D11ShaderResourceView* _heightMap;

	// Children in Scene Graph
	vector < GameObject* > children;

	GameObject * _parent;

	float _pRotationSpeed;
	bool _pRotateCubesXAxis;
	bool _pRotateCubesYAxis;
	bool _pRotateCubesZAxis;
	bool _pToggleXAxisPressed;
	bool _pToggleYAxisPressed;
	bool _pToggleZAxisPressed;
};

