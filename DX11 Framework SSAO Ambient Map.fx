//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//--------------------------------------------------------------------------------------

Texture2D txNormalMap : register(t0);
Texture2D txDepthMap : register(t1);
Texture2D txRandomVecMap : register(t2);

SamplerState samLinear : register(s0);
SamplerState samNormalDepth : register(s1);
SamplerState samClamp : register(s2);

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
	float3 Tangent : TANGENT;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
	float4 PosH : SV_POSITION;
	float3 ToFarPlane : TEXCOORD0;
	float2 Tex : TEXCOORD1;
	float3 Tangent : TEXCOORD2;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	// Transform vertex position from model coordinates to device coordinates
	output.PosH = float4(input.PosL, 1.0f);

	output.ToFarPlane = FrustumCorners[input.NormL.x].xyz;

	output.Tex = input.Tex;
	output.Tangent = input.Tangent;

	return output;
}

//float CalculateOcclusion(float distanceZ)
//{
//	float occlusion = 0.0f;
//
//	float OcclusionFadeStart = 0.2f;
//	float OcclusionFadeEnd = 2.0f;
//	float SurfaceEpsilon = 0.05f;
//
//	if (distanceZ < SurfaceEpsilon)
//	{
//		float fadeLength = OcclusionFadeEnd - OcclusionFadeStart;
//
//		occlusion = saturate((OcclusionFadeEnd - distanceZ) / fadeLength);
//	}
//
//	return occlusion;
//}
//
////--------------------------------------------------------------------------------------
//// Pixel Shader
////--------------------------------------------------------------------------------------
//float4 PS(VS_OUTPUT input) : SV_Target
//{
//	// Coordinates given in view space.
//	float OcclusionRadius = 0.2f;
//
//	float3 normals = txNormalMap.SampleLevel(samNormalDepth, input.Tex, 0.0f).xyz;
//	float pn = txNormalMap.SampleLevel(samNormalDepth, input.Tex, 0.0f).w;
//
//	//return float4(pn, pn, pn, 1.0f);
//
//	float pz = txDepthMap.Sample(samClamp, input.Tex).r;
//
//	// Reconstruct full view space position
//	float3 p = (pz / input.ToFarPlane.z) * input.ToFarPlane;
//
//	// Extract random vector and map
//	float3 randVec = 2.0f*txRandomVecMap.SampleLevel(samLinear, 4.0f*input.Tex, 0.0f).rgb - 1.0f;
//
//	float occlusionSum = 0.0f;
//
//	[unroll]
//	for (int i = 0; i < 14; ++i)
//	{
//		float3 offset = randVec;
//
//		//return float4(offset, 1.0f);
//
//		// Flip the offset vector if it is behind the plane
//		float flip = sign(dot(offset, normals));
//
//		// Sample a point near p within the occlusion radius
//		float3 q = p + flip * OcclusionRadius * offset;
//
//		float3x3 wtMat; // World-to-tangent space transformation matrix
//		wtMat[0] = normalize(mul(float4(input.Tangent, 0.0f), World).xyz); // Tangent basis vector
//		wtMat[1] = normalize(mul(float4(cross(normals, input.Tangent), 0.0f), World).xyz); // Binormal basis vector
//		wtMat[2] = normalize(mul(float4(normals, 0.0f), World).xyz); // Normal basis vector
//
//		float4 projectedSamplePoint = mul(float4(q, 1.0f), ViewToTex);
//		projectedSamplePoint /= projectedSamplePoint.w;
//
//		// Find the nearest depth value along the ray from the eye to the sample point
//		float rz = txNormalMap.Sample(samClamp, projectedSamplePoint.xy).w;
//
//		// Reconstruct full view space position
//		float3 r = (rz / q.z) * q;
//
//		float distanceZ = (p.z - r.z);
//
//		float dp = max(dot(normals, normalize(r - p)), 0.0f);
//		float occlusion = dp * CalculateOcclusion(distanceZ);
//
//		occlusionSum += occlusion;
//	}
//
//	occlusionSum /= 14;
//
//	float access = 1.0f - occlusionSum;
//
//	// Sharpen the contrast of the SSAO map to make the SSAO affect more dramatic.
//	return saturate(pow(access, 4.0f));
//}

float3 reconstructPosition(float depth, float2 texCoord)
{
	float4 pos = float4((texCoord.x - 0.5) * 2, (texCoord.y - 0.5) * 2.0f, 1.0f, 1.0f);
	float3 ray = mul(pos, WorldInvTransposeView).xyz;

	return ray.xyz * depth;
}

/// Sample the ambient occlusion at the following UV coordinate.
float3 SamplePixels(float3 srcPosition, float3 srcNormal, float2 texCoord)
{
	float occluderBias = 0.5f;

	float dstDepth = txNormalMap.SampleLevel(samNormalDepth, texCoord, 0.0f).w * 500.0f;
	float3 dstPosition = reconstructPosition(dstDepth, texCoord);

	float3 positionVec = dstPosition - srcPosition;
	float intensity = max(dot(normalize(positionVec), srcNormal) - occluderBias, 0.0);

	float dist = length(positionVec);

	return intensity * dist;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_Target
{
	float samplingRadius = 30.0f;
	float2 texelSize;
	texelSize.x = 1.0f / 1920.0f;
	texelSize.y = 1.0f / 1080.0f;

	texelSize.x = 1.0f;
	texelSize.y = 1.0f;

	float2 randVec = normalize(txRandomVecMap.Sample(samLinear, input.Tex).rgb).xy * 2.0f - 1.0f;

	float3 srcNormal = txNormalMap.Sample(samLinear, input.Tex).xyz;

	float srcDepth = txNormalMap.Sample(samLinear, input.Tex).w;

	float3 srcPosition = reconstructPosition(srcDepth, input.Tex);

	float kernelRadius = samplingRadius * (1.0f - srcDepth);

	// Sample neighbouring pixels
	float2 kernel[4];
	kernel[0] = float2(0.0f, 1.0f);  // top
	kernel[1] = float2(1.0f, 0.0f);  // right
	kernel[2] = float2(0.0f, -1.0f); // bottom
	kernel[3] = float2(-1.0f, 0.0f); // left

	float Sin45 = 0.707107f;
	
	float occlusion = 0.0f;

	for (int i = 0; i < 4; ++i)
	{
		float2 k1 = reflect(kernel[i], randVec);

		float2 k2 = float2(k1.x * Sin45 - k1.y * Sin45, k1.x * Sin45 + k1.y * Sin45);

		k1 *= texelSize;
		k2 *= texelSize;

		occlusion += SamplePixels(srcPosition, srcNormal, input.Tex + k1 * kernelRadius);
		occlusion += SamplePixels(srcPosition, srcNormal, input.Tex + k2 * kernelRadius * 0.75f);
		occlusion += SamplePixels(srcPosition, srcNormal, input.Tex + k1 * kernelRadius * 0.5f);
		occlusion += SamplePixels(srcPosition, srcNormal, input.Tex + k2 * kernelRadius * 0.25f);
	}

	// Average and clamp ambient occlusion
	occlusion /= 16;
	occlusion = 1.0 - clamp(occlusion, 0.0, 1.0);

	float4 colour = float4(occlusion, occlusion, occlusion, 1.0f);

	return colour;
}