//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//--------------------------------------------------------------------------------------
//
Texture2D txWhiteLightDepthMap : register(t0);
Texture2D txRedLightDepthMap : register(t1);
Texture2D txGreenLightDepthMap : register(t2);
Texture2D txBlueLightDepthMap : register(t3);
Texture2D txColourMap : register(t4);
Texture2D txNormalMap : register(t5);
Texture2D txTexCoordMap : register(t6);
Texture2D txPositionMap : register(t7);
Texture2D txTangentMap : register(t8);
Texture2D txWorldNormalMap : register(t9);
Texture2D txBiNormalMap : register(t10);

SamplerState samLinear : register(s0);
SamplerState samClamp : register(s1);

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
	float3 PosW : POSITION;
	float2 Tex : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	// Transform vertex position from model coordinates to device coordinates
	output.PosH = float4(input.PosL, 1.0f);
	output.Tex = input.Tex;

	return output;
}

// Calculate Specular Lighting on each Pixel taking into account each light
float3 CalculateSpecularLight(Light light, float3 lightVec, float3 normalMap, float diffuseAmount, float3 eyeVec)
{
	float3 specularLight = float3(0.0f, 0.0f, 0.0f);

	// Compute the reflection vector.
	float3 reflectionVector = reflect(-lightVec, normalMap);

	// Determine how much specular light makes it into the eye.
	float specularAmount;

	// Only display specular when there is diffuse
	if (diffuseAmount <= 0.0f)
	{
		specularAmount = 0.0f;
	}
	else
	{
		specularAmount = pow(max(dot(reflectionVector, eyeVec), 0.0f), light.SpecularPower);
	}

	specularLight = specularAmount * (surface.SpecularMtrl * light.SpecularLight).rgb;

	return specularLight;
}

// Calculate Diffuse Lighting on each Pixel taking into account each light
float3 CalculateDiffuseLight(Light light, float diffuseAmount)
{
	float3 diffuseLight = float3(0.0f, 0.0f, 0.0f);

	diffuseLight = diffuseAmount * (surface.DiffuseMtrl * light.DiffuseLight).rgb;

	return diffuseLight;
}

// Calculate Ambient Lighting on each Pixel taking into account each light
float3 CalculateAmbientLight(Light light, float4 textureColour)
{
	float3 ambientLight = float3(0.0f, 0.0f, 0.0f);

	ambientLight = (surface.AmbientMtrl * light.AmbientLight).rgb;

	return ambientLight;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 finalColour;

	float3 ambient = float3(0.0f, 0.0f, 0.0f);
	float3 diffuse = float3(0.0f, 0.0f, 0.0f);
	float3 specular = float3(0.0f, 0.0f, 0.0f);

	float4 colour = txColourMap.Sample(samLinear, input.Tex);
	float3 posW = txPositionMap.Sample(samLinear, input.Tex).xyz;
	float4 normalMap = txNormalMap.Sample(samLinear, input.Tex);
	float2 texCoordOffset = txTexCoordMap.Sample(samLinear, input.Tex).xy;
	float3 wTangent = txTangentMap.Sample(samLinear, input.Tex).xyz;
	float3 wBiNormal = txBiNormalMap.Sample(samLinear, input.Tex).xyz;
	float3 wNormal = txWorldNormalMap.Sample(samLinear, input.Tex).xyz;

	// Compute the 3x3 TEN matrix
	float3x3 wtMat; // World-to-tangent space transformation matrix
	wtMat[0] = wTangent.xyz;
	wtMat[1] = wBiNormal.xyz;
	wtMat[2] = wNormal.xyz;

	float3 wEyeVect = EyePosW - posW; // Eye vector in world space
	float3 eyeVec = normalize(mul(wEyeVect, wtMat));

	float3 lightVecWorld[4];
	float4 lightViewPositions[4];

	for (int i = 0; i < 4; i++)
	{
		// Compute light and eye vectors in world space
		float3 tmpLightVec = (lights[i].LightVecW).xyz; // Light vector in world space 
															  // Transform light and eye vectors from world space into tangent space 
		lightVecWorld[i] = normalize(mul(tmpLightVec, wtMat));

		// Calculate the position of the vertice as viewed by the light source.
		lightViewPositions[i] = mul(float4(posW, 1.0f), lights[i].View);
		lightViewPositions[i] = mul(lightViewPositions[i], lights[i].Projection);
	}

	float4 textureColour = float4(0.0f, 0.0f, 0.0f, 0.0f); // ??

	float depthValue;
	float lightDepthValue;

	for (int i = 0; i < 4; i++)
	{
		if (lights[i].lightOn == 1.0f)
		{
			// Determine the diffuse light intensity that strikes the vertex.
			float diffuseAmount = max(0.0f, dot(lightVecWorld[i], normalMap.xyz));

			if (lightVecWorld[i].z <= 0.0000001f || dot(normalMap, lightVecWorld[i]) <= 0.0000001f)
			{
				diffuseAmount = 0.0f;
			}
			else
			{
				float inShadow = 0.0f;

				if (shadowsOn == 1.0f)
				{
					float4 currentLightViewPosition = float4(0.0f, 0.0f, 0.0f, 1.0f);
					float newShadowsOn = 1.0f;

					currentLightViewPosition = lightViewPositions[i];

					if (newShadowsOn == 1.0f)
					{
						float2 projectedTexCoord;
						projectedTexCoord.x = ((currentLightViewPosition.x / currentLightViewPosition.w) / 2.0f + 0.5f) + texCoordOffset.x;
						projectedTexCoord.y = ((-currentLightViewPosition.y / currentLightViewPosition.w) / 2.0f + 0.5f) + texCoordOffset.y;

						if (i == 0)
						{
							depthValue = txWhiteLightDepthMap.Sample(samClamp, projectedTexCoord).r;
						}
						else if (i == 1)
						{
							depthValue = txRedLightDepthMap.Sample(samClamp, projectedTexCoord).r;
						}
						else if (i == 2)
						{
							depthValue = txGreenLightDepthMap.Sample(samClamp, projectedTexCoord).r;
						}
						else if (i == 3)
						{
							depthValue = txBlueLightDepthMap.Sample(samClamp, projectedTexCoord).r;
						}

						lightDepthValue = currentLightViewPosition.z / currentLightViewPosition.w;
						lightDepthValue = lightDepthValue - 0.000001f;

						if ((saturate(projectedTexCoord.x) == projectedTexCoord.x) && (saturate(projectedTexCoord.y) == projectedTexCoord.y))
						{
							if ((depthValue < lightDepthValue))
							{
								inShadow = 1.0f;
							}
						}
					}
				}

				if (inShadow == 1.0f)
				{
					diffuseAmount = 0.0f;
				}
			}

			// Compute the ambient, diffuse, and specular terms separately.
			specular += CalculateSpecularLight(lights[i], lightVecWorld[i], normalMap, diffuseAmount, eyeVec);
			diffuse += CalculateDiffuseLight(lights[i], diffuseAmount);
			ambient += CalculateAmbientLight(lights[i], textureColour);
		}
	}

	finalColour.rgb = (colour.rgb * (ambient + diffuse)) + specular;
	finalColour.a = colour.a;

	return finalColour;
}