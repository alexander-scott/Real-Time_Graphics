#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <DirectXCollision.h>
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

	float Range;
	XMFLOAT3 Attenuation;

	float Cone;
	XMFLOAT3 Direction;

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

struct HeightMapInfo 
{
	int terrainWidth;		// Width of heightmap
	int terrainHeight;		// Height (Length) of heightmap
	XMFLOAT3 *heightMap;	// Array to store terrain's vertex positions
};

#pragma endregion