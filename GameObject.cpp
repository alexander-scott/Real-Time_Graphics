#include "GameObject.h"

GameObject::GameObject(string type, Geometry geometry, Material material) : mGeometry(geometry), mType(type), mMaterial(material)
{
	mParent = nullptr;
	mPosition = XMFLOAT3();
	mRotation = XMFLOAT3();
	mScale = XMFLOAT3(1.0f, 1.0f, 1.0f);

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
	if (!mHasChanged)
		return false;

	mHasChanged = false;

	// Calculate world matrix
	XMMATRIX scale = XMMatrixScaling(mScale.x, mScale.y, mScale.z);
	XMMATRIX rotation = XMMatrixRotationX(mRotation.x) * XMMatrixRotationY(mRotation.y) * XMMatrixRotationZ(mRotation.z);
	XMMATRIX translation = XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);

	XMStoreFloat4x4(&mWorld, scale * rotation * translation);

	if (mParent != nullptr)
	{
		XMStoreFloat4x4(&mWorld, this->GetWorldMatrix() * mParent->GetWorldMatrix());
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