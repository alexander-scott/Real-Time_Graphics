//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//--------------------------------------------------------------------------------------

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

struct SurfaceInfo
{
	float4 AmbientMtrl;
	float4 DiffuseMtrl;
	float4 SpecularMtrl;
};

struct Light
{
	matrix View;
	matrix Projection;

	float4 AmbientLight;
	float4 DiffuseLight;
	float4 SpecularLight;

	float SpecularPower;
	float3 LightVecW;

	float3 paddingLightAmount;
	float lightOn;
};

cbuffer ConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;

	SurfaceInfo surface;
	Light lights[4];

	float3 EyePosW;
	float HasTexture;

	float HasNormalMap;
	float HasHeightMap;
	float shadowsOn;
	float screenWidth;
	float screenHeight;
	float blurIntensity;
	float selfShadowOn;
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
	float2 Tex  : TEXCOORD0;
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.PosH = float4(input.PosL, 1.0f);
	output.Tex = input.Tex;

	return output;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 textureColour = float4(0.0f, 0.0f, 0.0f, 0.0f);

	if (blurIntensity > 0)
	{
		float texelSize = 1.0f / 1920;

		float2 Tex1 = input.Tex + float2(texelSize * -4.0f, 0.0f);
		float2 Tex2 = input.Tex + float2(texelSize * -3.0f, 0.0f);
		float2 Tex3 = input.Tex + float2(texelSize * -2.0f, 0.0f);
		float2 Tex4 = input.Tex + float2(texelSize * -1.0f, 0.0f);
		float2 Tex5 = input.Tex + float2(texelSize *  0.0f, 0.0f);
		float2 Tex6 = input.Tex + float2(texelSize *  1.0f, 0.0f);
		float2 Tex7 = input.Tex + float2(texelSize *  2.0f, 0.0f);
		float2 Tex8 = input.Tex + float2(texelSize *  3.0f, 0.0f);
		float2 Tex9 = input.Tex + float2(texelSize *  4.0f, 0.0f);

		float weight0 = 1.0f;
		float weight1 = 1.0f * blurIntensity;
		float weight2 = 1.0f * blurIntensity;
		float weight3 = 1.0f * blurIntensity;
		float weight4 = 1.0f * blurIntensity;

		// Create a normalized value to average the weights out a bit.
		float weightNormalisation;
		weightNormalisation = (weight0 + 2.0f * (weight1 + weight2 + weight3 + weight4));

		// Normalize the weights.
		weight0 = weight0 / weightNormalisation;
		weight1 = weight1 / weightNormalisation;
		weight2 = weight2 / weightNormalisation;
		weight3 = weight3 / weightNormalisation;
		weight4 = weight4 / weightNormalisation;

		textureColour += txDiffuse.Sample(samLinear, Tex1) * weight4;
		textureColour += txDiffuse.Sample(samLinear, Tex2) * weight3;
		textureColour += txDiffuse.Sample(samLinear, Tex3) * weight2;
		textureColour += txDiffuse.Sample(samLinear, Tex4) * weight1;
		textureColour += txDiffuse.Sample(samLinear, Tex5) * weight0;
		textureColour += txDiffuse.Sample(samLinear, Tex6) * weight1;
		textureColour += txDiffuse.Sample(samLinear, Tex7) * weight2;
		textureColour += txDiffuse.Sample(samLinear, Tex8) * weight3;
		textureColour += txDiffuse.Sample(samLinear, Tex9) * weight4;
	}
	else
	{
		textureColour = txDiffuse.Sample(samLinear, input.Tex);
	}

	//float4 colour = float4(0.0f, 0.5f, 0.0f, 1.0f);

	return textureColour;
}