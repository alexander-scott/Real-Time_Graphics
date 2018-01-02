#include "GameObject.h"

GameObject::GameObject(string type, Geometry geometry, Material material) : mGeometry(geometry), mType(type), mMaterial(material)
{
	mParent = nullptr;
	mWorldPosition = XMFLOAT3();
	mWorldRotation = XMFLOAT3();
	mWorldScale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	mLocalPosition = XMFLOAT3();
	mLocalRotation = XMFLOAT3();
	mLocalScale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	mTextureRV = nullptr;
	mNormalMap = nullptr;
	mHeightMap = nullptr;

	mHasChanged = true;
}

GameObject::~GameObject()
{
}

bool GameObject::Update(float t, float deltaTime)
{
	if (!mHasChanged && (mParent != nullptr && !mParent->mHasChanged)) // If object transform has changed or it's parents has
		return false;

	mHasChanged = false;

	// Calculate world matrix
	XMMATRIX scale = XMMatrixScaling(mWorldScale.x, mWorldScale.y, mWorldScale.z);
	XMMATRIX rotation = XMMatrixRotationX(mWorldRotation.x) * XMMatrixRotationY(mWorldRotation.y) * XMMatrixRotationZ(mWorldRotation.z);
	XMMATRIX translation = XMMatrixTranslation(mWorldPosition.x, mWorldPosition.y, mWorldPosition.z);

	XMStoreFloat4x4(&mWorld, scale * rotation * translation);

	if (mParent != nullptr)
	{
		XMStoreFloat4x4(&mWorld, this->GetWorldMatrix() * mParent->GetWorldMatrix());

		SetWorldRotation(mLocalRotation.x + mParent->GetWorldRotation().x, mLocalRotation.y + mParent->GetWorldRotation().y, mLocalRotation.z + mParent->GetWorldRotation().z);
		SetWorldPosition(mLocalPosition.x + mParent->GetWorldPosition().x, mLocalPosition.y + mParent->GetWorldPosition().y, mLocalPosition.z + mParent->GetWorldPosition().z);
		SetWorldScale(mLocalScale.x * mParent->GetWorldScale().x, mLocalScale.y * mParent->GetWorldScale().y, mLocalScale.z * mParent->GetWorldScale().z);
	}

	return true;
}

void GameObject::Draw(ID3D11DeviceContext* pImmediateContext)
{
	// NOTE: We are assuming that the constant buffers and all other draw setup has already taken place

	// Set vertex and index buffers
	pImmediateContext->IASetVertexBuffers(0, 1, &mGeometry.vertexBuffer, &mGeometry.vertexBufferStride, &mGeometry.vertexBufferOffset);
	pImmediateContext->IASetIndexBuffer(mGeometry.indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	pImmediateContext->DrawIndexed(mGeometry.numberOfIndices, 0, 0);
}

void GameObject::SetTextures(TextureSet* textureSet)
{
	SetTextureRV(textureSet->texture);
	SetNormalMap(textureSet->normalMap);
	SetHeightMap(textureSet->heightMap);
}

void GameObject::SetWorldPosition(XMFLOAT3 position)
{
	if (mWorldPosition.x == position.x && mWorldPosition.y == position.y && mWorldPosition.z == position.z)
		return;

	mHasChanged = true;
	mWorldPosition = position;
}

void GameObject::SetWorldPosition(float x, float y, float z)
{
	if (mWorldPosition.x == x && mWorldPosition.y == y && mWorldPosition.z == z)
		return;
		
	mHasChanged = true;
	mWorldPosition.x = x; mWorldPosition.y = y; mWorldPosition.z = z; 
}

void GameObject::SetWorldScale(float x, float y, float z)
{
	if (mWorldScale.x == x && mWorldScale.y == y && mWorldScale.z == z)
		return;
		
	mHasChanged = true;
	mWorldScale.x = x; mWorldScale.y = y; mWorldScale.z = z;
}

void GameObject::SetWorldRotation(float x, float y, float z)
{
	if (mWorldRotation.x == x && mWorldRotation.y == y && mWorldRotation.z == z)
		return;

	mHasChanged = true;
	mWorldRotation.x = x; mWorldRotation.y = y; mWorldRotation.z = z;
}

void GameObject::SetLocalPosition(XMFLOAT3 position)
{
	if (mLocalPosition.x == position.x && mLocalPosition.y == position.y && mLocalPosition.z == position.z)
		return;

	mHasChanged = true;
	mLocalPosition = position;
}

void GameObject::SetLocalPosition(float x, float y, float z)
{
	if (mLocalPosition.x == x && mLocalPosition.y == y && mLocalPosition.z == z)
		return;

	mHasChanged = true;
	mLocalPosition.x = x; mLocalPosition.y = y; mLocalPosition.z = z;
}

void GameObject::SetLocalScale(float x, float y, float z)
{
	if (mLocalScale.x == x && mLocalScale.y == y && mLocalScale.z == z)
		return;

	mHasChanged = true;
	mLocalScale.x = x; mLocalScale.y = y; mLocalScale.z = z;
}

void GameObject::SetLocalRotation(float x, float y, float z)
{
	if (mLocalRotation.x == x && mLocalRotation.y == y && mLocalRotation.z == z)
		return;

	mHasChanged = true;
	mLocalRotation.x = x; mLocalRotation.y = y; mLocalRotation.z = z;
}