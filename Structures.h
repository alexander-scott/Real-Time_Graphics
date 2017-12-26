#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <string>

using namespace std;
using namespace DirectX;

#pragma region Consts

static constexpr float kCameraLookSpeed = 0.25f; // Default 0.25f
static constexpr float kCameraMoveSpeed = 0.5f; // Default 0.5f

static constexpr float kPI = 3.1415926535f;
static constexpr float kCameraRadius = 45.0f;
static constexpr float kCameraMaxPitch = 1.2566370616f;

static constexpr float kOctreeNodeMaxObjects = 10;

#pragma endregion

#pragma region Useful Functions

inline XMVECTOR XMFLOAT3ToXMVECTOR(XMFLOAT3& val)
{
	return XMLoadFloat3(&val);
}

inline XMFLOAT3 XMVECTORToXMFLOAT3(XMVECTOR& vec)
{
	XMFLOAT3 val;
	XMStoreFloat3(&val, vec);
	return val;
}

inline XMMATRIX XMFLOAT4X4ToXMMATRIX(XMFLOAT4X4& val)
{
	return XMLoadFloat4x4(&val);
}

inline XMFLOAT4X4 XMMATRIXToXMFLOAT4X4(XMMATRIX& matrix)
{
	XMFLOAT4X4 val;
	XMStoreFloat4x4(&val, matrix);
	return val;
}

#pragma endregion

#pragma region Structs

struct SimpleVertex
{
	XMFLOAT3 PosL;
	XMFLOAT3 NormL;
	XMFLOAT2 Tex;
	XMFLOAT3 Tangent;
};

struct QuadVertex
{
	XMFLOAT3 PosL;
	XMFLOAT3 NormL;
	XMFLOAT2 Tex;
};

struct SurfaceInfo
{
	XMFLOAT4 AmbientMtrl;
	XMFLOAT4 DiffuseMtrl;
	XMFLOAT4 SpecularMtrl;
};

struct Light
{
	XMMATRIX View;
	XMMATRIX Projection;

	XMFLOAT4 AmbientLight;
	XMFLOAT4 DiffuseLight;
	XMFLOAT4 SpecularLight;

	float SpecularPower;
	XMFLOAT3 LightVecW;

	XMFLOAT3 paddingLightAmount;
	float lightOn;
};

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

struct ConstantBuffer
{
	XMMATRIX World;
	XMMATRIX View;
	XMMATRIX Projection;

	SurfaceInfo surface;
	Light lights[4];

	XMFLOAT3 EyePosW;
	float HasTexture;

	float HasNormalMap;
	float HasHeightMap;
	float shadowsOn;
	float screenWidth;
	float screenHeight;
	float blurIntensity;
	float selfShadowOn;
};

struct SMConstantBuffer
{
	XMMATRIX World;
	XMMATRIX View;
	XMMATRIX Projection;
};

struct Bounds
{
public:
	XMFLOAT3 Centre; // The center of the bounding box.
	XMFLOAT3 Extents; // The extents of the Bounding Box. This is always half of the size of the Bounds.
	XMFLOAT3 Max; // The maximal point of the box. This is always equal to center+extents.
	XMFLOAT3 Min; // The minimal point of the box. This is always equal to center-extents.
	XMFLOAT3 Size; // The total size of the box. This is always twice as large as the extents.

	Bounds()
	{
		Centre = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Extents = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Max = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Min = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Size = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

	Bounds(XMFLOAT3 origin, XMFLOAT3 actualBoundsSize)
	{
		Centre = origin;
		Size = actualBoundsSize;
		Extents = XMFLOAT3(Size.x / 2, Size.y / 2, Size.z / 2);
		Max = XMFLOAT3(Centre.x + Extents.x, Centre.y + Extents.y, Centre.z + Extents.z);
		Min = XMFLOAT3(Centre.x - Extents.x, Centre.y - Extents.y, Centre.z - Extents.z);
	}
};

#pragma endregion