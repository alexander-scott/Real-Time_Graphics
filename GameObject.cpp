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

	_pRotationSpeed = 0.0005f;
	_pRotateCubesXAxis = false;
	_pRotateCubesYAxis = false; // Note: demo rotation
	_pRotateCubesZAxis = false;
	_pToggleXAxisPressed = false;
	_pToggleYAxisPressed = false;
	_pToggleZAxisPressed = false;
}

GameObject::~GameObject()
{
}

void GameObject::Update(float t, float deltaTime)
{
	if (mType != "WhiteLightPosCube" && mType != "RedLightPosCube" && mType != "GreenLightPosCube" && mType != "BlueLightPosCube")
	{
		UpdateRotation(deltaTime);
	}

	// Calculate world matrix
	XMMATRIX scale = XMMatrixScaling(mScale.x, mScale.y, mScale.z);
	XMMATRIX rotation = XMMatrixRotationX(mRotation.x) * XMMatrixRotationY(mRotation.y) * XMMatrixRotationZ(mRotation.z);
	XMMATRIX translation = XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);

	XMStoreFloat4x4(&mWorld, scale * rotation * translation);

	if (mParent != nullptr)
	{
		XMStoreFloat4x4(&mWorld, this->GetWorldMatrix() * mParent->GetWorldMatrix());
	}
}

void GameObject::UpdateRotation(float deltaTime)
{
	XMFLOAT3 origRot = mRotation;
	XMFLOAT3 newRotation;

	// Update Rotation Axis
	if (_pRotateCubesXAxis)
	{
		newRotation.x = origRot.x + (_pRotationSpeed * deltaTime);
	}
	else
	{
		newRotation.x = origRot.x;
	}

	if (_pRotateCubesYAxis)
	{
		newRotation.y = origRot.y + (_pRotationSpeed * deltaTime);
	}
	else
	{
		newRotation.y = origRot.y;
	}

	if (_pRotateCubesZAxis)
	{
		newRotation.z = origRot.z + (_pRotationSpeed * deltaTime);
	}
	else
	{
		newRotation.z = origRot.z;
	}

	mRotation = newRotation;
}

void GameObject::Draw(ID3D11DeviceContext* pImmediateContext)
{
	// NOTE: We are assuming that the constant buffers and all other draw setup has already taken place

	// Set vertex and index buffers
	pImmediateContext->IASetVertexBuffers(0, 1, &mGeometry.vertexBuffer, &mGeometry.vertexBufferStride, &mGeometry.vertexBufferOffset);
	pImmediateContext->IASetIndexBuffer(mGeometry.indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	pImmediateContext->DrawIndexed(mGeometry.numberOfIndices, 0, 0);
}

void GameObject::HandleControls()
{

#pragma region Cube Rotation Control

	// Toggle Cube Rotation Axis
	if (GetAsyncKeyState(VK_CONTROL))
	{
		if (GetAsyncKeyState('X') && !_pToggleXAxisPressed)
		{
			_pRotateCubesXAxis = !_pRotateCubesXAxis;
			_pToggleXAxisPressed = !_pToggleXAxisPressed;
		}
		else if (GetAsyncKeyState('Y') && !_pToggleYAxisPressed)
		{
			_pRotateCubesYAxis = !_pRotateCubesYAxis;
			_pToggleYAxisPressed = !_pToggleYAxisPressed;
		}
		else if (GetAsyncKeyState('Z') && !_pToggleZAxisPressed)
		{
			_pRotateCubesZAxis = !_pRotateCubesZAxis;
			_pToggleZAxisPressed = !_pToggleZAxisPressed;
		}
	}

	if (!GetAsyncKeyState('X') && _pToggleXAxisPressed)
	{
		_pToggleXAxisPressed = !_pToggleXAxisPressed;
	}
	if (!GetAsyncKeyState('Y') && _pToggleYAxisPressed)
	{
		_pToggleYAxisPressed = !_pToggleYAxisPressed;
	}
	if (!GetAsyncKeyState('Z') && _pToggleZAxisPressed)
	{
		_pToggleZAxisPressed = !_pToggleZAxisPressed;
	}

	// Rotate the Cubes toggle
	if (GetAsyncKeyState(VK_LSHIFT) && GetAsyncKeyState('R'))
	{
		_pRotationSpeed -= 0.0001f;
	}
	else if (GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState('R'))
	{
		_pRotationSpeed = 0.0f;
	}
	else if (GetAsyncKeyState('R'))
	{
		_pRotationSpeed += 0.0001f;
	}

#pragma endregion

}

void GameObject::SetTextures(TextureSet* textureSet)
{
	SetTextureRV(textureSet->texture);
	SetNormalMap(textureSet->normalMap);
	SetHeightMap(textureSet->heightMap);
}