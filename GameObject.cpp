#include "GameObject.h"

GameObject::GameObject(string type, Geometry geometry, Material material) : _geometry(geometry), _type(type), _material(material)
{
	_parent = nullptr;
	_position = XMFLOAT3();
	_rotation = XMFLOAT3();
	_scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	_textureRV = nullptr;
	_normalMap = nullptr;
	_heightMap = nullptr;

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
	if (_type != "WhiteLightPosCube" && _type != "RedLightPosCube" && _type != "GreenLightPosCube" && _type != "BlueLightPosCube")
	{
		UpdateRotation(deltaTime);
	}

	// Calculate world matrix
	XMMATRIX scale = XMMatrixScaling(_scale.x, _scale.y, _scale.z);
	XMMATRIX rotation = XMMatrixRotationX(_rotation.x) * XMMatrixRotationY(_rotation.y) * XMMatrixRotationZ(_rotation.z);
	XMMATRIX translation = XMMatrixTranslation(_position.x, _position.y, _position.z);

	XMStoreFloat4x4(&_world, scale * rotation * translation);

	if (_parent != nullptr)
	{
		XMStoreFloat4x4(&_world, this->GetWorldMatrix() * _parent->GetWorldMatrix());
	}
}

void GameObject::UpdateRotation(float deltaTime)
{
	XMFLOAT3 origRot = _rotation;
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

	_rotation = newRotation;
}

void GameObject::Draw(ID3D11DeviceContext* pImmediateContext)
{
	// NOTE: We are assuming that the constant buffers and all other draw setup has already taken place

	// Set vertex and index buffers
	pImmediateContext->IASetVertexBuffers(0, 1, &_geometry.vertexBuffer, &_geometry.vertexBufferStride, &_geometry.vertexBufferOffset);
	pImmediateContext->IASetIndexBuffer(_geometry.indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	pImmediateContext->DrawIndexed(_geometry.numberOfIndices, 0, 0);
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