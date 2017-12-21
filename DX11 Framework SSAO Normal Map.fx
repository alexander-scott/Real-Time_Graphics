//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//--------------------------------------------------------------------------------------

Texture2D txNormalMap : register(t0);
SamplerState samLinear : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

cbuffer ConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;

	matrix ViewToTex;
	matrix WorldInvTransposeView;

	float4 OffsetVectors[14];
	float4 FrustumCorners[4];

	float HasNormalMap;
}

struct VS_INPUT
{
	float3 PosL : POSITION;
	float3 NormL : NORMAL;
	float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
	float4 PosH : SV_POSITION;
	float4 PosV : POSITION;
	float3 NormalV : NORMAL;
	float2 Tex : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	//output.NormW = input.NormL;

	// Transform vertex position from model coordinates to device coordinates
	float4 tempPosV = mul(float4(input.PosL, 1.0f), World);
	output.PosV = mul(tempPosV, View);

	output.NormalV = mul(input.NormL, (float3x3)WorldInvTransposeView);

	output.PosH = mul(float4(input.PosL, 1.0f), World);
	output.PosH = mul(output.PosH, View);
	output.PosH = mul(output.PosH, Projection);

	output.Tex = input.Tex;

	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_Target
{
	input.NormalV = normalize(input.NormalV);

	float4 normalMap;
	normalMap.rg = input.NormalV.xy * 0.5f + 0.5f;

	normalMap = float4(normalMap.x, normalMap.y, normalMap.z, input.PosH.z/input.PosH.w);

	return normalMap;
}
