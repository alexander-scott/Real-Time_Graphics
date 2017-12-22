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
static constexpr float kCameraMoveSpeed = 0.03f; // Default 0.03f

static constexpr float kPI = 3.1415926535f;
static constexpr float kCameraRadius = 45.0f;
static constexpr float kCameraMaxPitch = 1.570796327f;

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

#pragma endregion