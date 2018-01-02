#pragma once

#include "TextureController.h"
#include "Structures.h"

using namespace DirectX;
using namespace std;

class GameObject
{
public:
	GameObject(string type, Geometry geometry, Material material);
	~GameObject();

	// Setters for position/rotation/scale
	void SetWorldPosition(XMFLOAT3 position);
	void SetWorldPosition(float x, float y, float z);
	void SetWorldScale(float x, float y, float z);
	void SetWorldRotation(float x, float y, float z);

	// Setters for position/rotation/scale
	void SetLocalPosition(XMFLOAT3 position);
	void SetLocalPosition(float x, float y, float z);
	void SetLocalScale(float x, float y, float z);
	void SetLocalRotation(float x, float y, float z);

	XMFLOAT3 GetWorldPosition() { return mWorldPosition; }
	XMFLOAT3 GetWorldScale() { return mWorldScale; }
	XMFLOAT3 GetWorldRotation() { return mWorldRotation; }

	// Getters for object details
	string GetType() const { return mType; }
	Geometry GetGeometryData() const { return mGeometry; }
	Material GetMaterial() const { return mMaterial; }
	XMMATRIX GetWorldMatrix() const { return XMLoadFloat4x4(&mWorld); }

	void SetTextures(TextureSet* textureSet);

	void SetTextureRV(ID3D11ShaderResourceView* textureRV) { mTextureRV = textureRV; }
	ID3D11ShaderResourceView* GetTextureRV() const { return mTextureRV; }
	bool HasTexture() const { return mTextureRV ? true : false; }

	void SetNormalMap(ID3D11ShaderResourceView* normalMap) { mNormalMap = normalMap; }
	ID3D11ShaderResourceView* GetNormalMap() const { return mNormalMap; }
	bool HasNormalMap() const { return mNormalMap ? true : false; }

	void SetHeightMap(ID3D11ShaderResourceView* heightMap) { mHeightMap = heightMap; }
	ID3D11ShaderResourceView* GetHeightMap() const { return mHeightMap; }
	bool HasHeightMap() const { return mHeightMap ? true : false; }

	void SetParent(GameObject* parent) { mParent = parent; }
	void AddChild(GameObject* child) { mChildren.push_back(child); }
	GameObject* GetChild(int index) { return mChildren[index]; }

	bool Update(float t, float deltaTime);
	void Draw(ID3D11DeviceContext* pImmediateContext);
	
protected:
	string						mType;

	Geometry					mGeometry;
	Material					mMaterial;

private:
	XMFLOAT3					mLocalPosition;
	XMFLOAT3					mLocalRotation;
	XMFLOAT3					mLocalScale;

	XMFLOAT3					mWorldPosition;
	XMFLOAT3					mWorldRotation;
	XMFLOAT3					mWorldScale;

	XMFLOAT4X4					mWorld;

	ID3D11ShaderResourceView*	mTextureRV;
	ID3D11ShaderResourceView*	mNormalMap;
	ID3D11ShaderResourceView*	mHeightMap;

	// Children in Scene Graph
	vector<GameObject*>			mChildren;

	GameObject *				mParent;

	bool						mHasChanged;
};

