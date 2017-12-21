//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//--------------------------------------------------------------------------------------

Texture2D txWhiteLightDepthMap : register(t0);
Texture2D txRedLightDepthMap : register(t1);
Texture2D txDiffuse : register(t2);
Texture2D txNormalMap : register(t3);
Texture2D txHeightMap : register(t4);

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
	float3 NormW : NORMAL;

	float3 PosW : POSITION;
	float2 Tex : TEXCOORD0;

	float3 Tangent : TANGENT;

	float3 lightVec : TEXCOORD1;
	float3 eyeVec : TEXCOORD2;

	float4 lightViewPosition : TEXCOORD3;
	float4 lightViewPositionRed : TEXCOORD4;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.Tangent = input.Tangent;
	output.NormW = input.NormL;

	output.PosW = input.PosL;

	// Transform vertex position from model coordinates to device coordinates
	output.PosH = mul(float4(input.PosL, 1.0f), World);
	output.PosH = mul(output.PosH, View);
	output.PosH = mul(output.PosH, Projection);

	// Calculate the position of the vertice as viewed by the light source.
	output.lightViewPosition = mul(float4(input.PosL, 1.0f), World);
	output.lightViewPosition = mul(output.lightViewPosition, lights[0].View);
	output.lightViewPosition = mul(output.lightViewPosition, lights[0].Projection);

	// Calculate the position of the vertice as viewed by the light source.
	output.lightViewPositionRed = mul(float4(input.PosL, 1.0f), World);
	output.lightViewPositionRed = mul(output.lightViewPositionRed, lights[1].View);
	output.lightViewPositionRed = mul(output.lightViewPositionRed, lights[1].Projection);

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

float3 CalculateTextureCoordOffset(VS_OUTPUT input, float3 eyeVec, float3 lightVec)
{
	// Determine the depth of the parallax mapping
	float fParallaxLimit = -length(eyeVec.xy) / eyeVec.z;
	float fHeightMapScale = 0.1f;
	fParallaxLimit *= fHeightMapScale;

	float2 vOffsetDir = normalize(eyeVec.xy);
	float2 vMaxOffset = vOffsetDir * fParallaxLimit;

	// Max and min amount of ray samples
	int nMinSamples = 4;
	int nMaxSamples = 40;

	// Determine amount of smapling needed from the angle of the light vector to the eyeVec
	//int nNumSamples = (int)lerp(nMaxSamples, nMinSamples, dot(eyeVec, lightVec));
	int nNumSamples = 200;

	// Step size difference dependant on sampling frequency
	float fStepSize = 1.0f / (float)nNumSamples;

	float2 dx = ddx(input.Tex);
	float2 dy = ddy(input.Tex);

	float fCurrRayHeight = 1.0f;

	float2 vCurrOffset = float2(0.0f, 0.0f);
	float2 vLastOffset = float2(0.0f, 0.0f);

	float fLastSampledHeight = 1.0f;
	float fCurrSampledHeight = 1.0f;

	int nCurrSample = 0;

	while (nCurrSample < nNumSamples)
	{
		fCurrSampledHeight = txHeightMap.SampleGrad(samLinear, (input.Tex + vCurrOffset), dx, dy).r;

		if (fCurrSampledHeight > fCurrRayHeight)
		{
			float delta1 = fCurrSampledHeight - fCurrRayHeight;
			float delta2 = (fCurrRayHeight + fStepSize) - fLastSampledHeight;

			float ratio = delta1 / (delta1 + delta2);
			vCurrOffset = (ratio)* vLastOffset + (1.0f - ratio) * vCurrOffset;

			nCurrSample = nNumSamples + 1;
		}
		else
		{
			nCurrSample++;

			fCurrRayHeight -= fStepSize;

			vLastOffset = vCurrOffset;
			vCurrOffset += fStepSize * vMaxOffset;

			fLastSampledHeight = fCurrSampledHeight;
		}
	}

	float3 parrallaxDetails = float3(vCurrOffset.x, vCurrOffset.y, fCurrRayHeight);

	return parrallaxDetails;
}

float CalculateIfInShadow(VS_OUTPUT input, float3 eyeVec, float3 lightVec, float2 vFinalCoords)
{
	float inShadow = 0.0f;
	
	// Determine the depth of the parallax mapping
	float fParallaxLimit = length(lightVec.xy) / lightVec.z;
	float fHeightMapScale = 0.1f;
	fParallaxLimit *= fHeightMapScale;

	float2 vOffsetDir = normalize(lightVec.xy);
	float2 vMaxOffset = vOffsetDir * fParallaxLimit;

	// Max and min amount of ray samples
	int nMinSamples = 4;
	int nMaxSamples = 40;

	// Determine amount of smapling needed from the angle of the light vector to the eyeVec
	//int nNumSamples = (int)lerp(nMaxSamples, nMinSamples, dot(eyeVec, lightVec));
	int nNumSamples = 200;

	// Step size difference dependant on sampling frequency
	float fStepSize = 1.0f / (float)nNumSamples;

	float2 vCurrOffset = fStepSize * vMaxOffset;

	float2 dx = ddx(vFinalCoords);
	float2 dy = ddy(vFinalCoords);

	float fCurrSampledHeight = txHeightMap.SampleGrad(samLinear, vFinalCoords + vCurrOffset, dx, dy).r;
	float fCurrRayHeight = fCurrSampledHeight += fStepSize;

	int nCurrSample = 0;

	while (nCurrSample < nNumSamples)
	{
		fCurrSampledHeight = txHeightMap.SampleGrad(samLinear, (vFinalCoords + vCurrOffset), dx, dy).r;

		if (fCurrSampledHeight > fCurrRayHeight)
		{
			inShadow = 1.0f;

			nCurrSample = nNumSamples + 1;
		}
		else
		{
			nCurrSample++;

			fCurrRayHeight += fStepSize;
			vCurrOffset += fStepSize * vMaxOffset;
		}
	}
	
	return inShadow;
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

	// Compute the 3x3 TEN matrix
	float3x3 wtMat; // World-to-tangent space transformation matrix
	wtMat[0] = normalize(mul(float4(input.Tangent, 0.0f), World).xyz); // Tangent basis vector
	wtMat[1] = normalize(mul(float4(cross(input.NormW, input.Tangent), 0.0f), World).xyz); // Binormal basis vector
	wtMat[2] = normalize(mul(float4(input.NormW, 0.0f), World).xyz); // Normal basis vector

	// Transform vertex position from model coordinates to world coordinates
	float3 wPos = mul(float4(input.PosW, 1.0f), World).xyz;

	float3 wLightVect[4];

	for (int i = 0; i < 4; i++)
	{
		wLightVect[i] = float3(0.0f, 0.0f, 0.0f);
	}

	float4 textureColour = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 wEyeVect = (EyePosW - wPos).xyz; // Eye vector in world space
	float3 eyeVec = normalize(mul(wEyeVect, wtMat));

	float parallaxHeight;
	float depthValue;
	float lightDepthValue;

	for (int i = 0; i < 4; i++)
	{
		if (lights[i].lightOn == 1.0f)
		{
			// Compute light and eye vectors in world space
			float3 tmpLightVec3 = mul(lights[i].LightVecW, World);
			float3 tmpLightVec2 = (tmpLightVec3 - wPos).xyz;
			float3 lightVec2 = normalize(mul(tmpLightVec2, wtMat));

			// Compute light and eye vectors in world space
			float3 tmpLightVec = (lights[i].LightVecW - wPos).xyz; // Light vector in world space 
			// Transform light and eye vectors from world space into tangent space 
			float3 lightVec = normalize(mul(tmpLightVec, wtMat));

			float2 vFinalCoords;
			float2 textureCoordOffset;

			if (HasHeightMap == 1.0f)
			{
				float3 parallaxDetails = CalculateTextureCoordOffset(input, eyeVec, lightVec);
				parallaxHeight = parallaxDetails.z;
				textureCoordOffset = float2(parallaxDetails.x, parallaxDetails.y);

				vFinalCoords = input.Tex + textureCoordOffset;
			}
			else
			{
				vFinalCoords = input.Tex;
				textureCoordOffset = float2(0.0f, 0.0f);
			}
			
			// Sample colour and normal maps
			textureColour = txDiffuse.Sample(samLinear, vFinalCoords);
			float4 normalMap = txNormalMap.Sample(samLinear, vFinalCoords);

			normalMap = (2.0f * normalMap) - 1.0f;

			// Determine the diffuse light intensity that strikes the vertex.
			float diffuseAmount = max(0.0f, dot(lightVec, normalMap.xyz));

			if (lightVec.z <= 0.00001 || dot(normalMap, lightVec) <= 0.00001)
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

					if (i == 0)
					{
						currentLightViewPosition = input.lightViewPosition;
					}
					else if (i == 1)
					{
						currentLightViewPosition = input.lightViewPositionRed;
					}
					else
					{
						newShadowsOn == 0.0f;
					}

					if (newShadowsOn == 1.0f)
					{
						float2 projectedTexCoord;
						projectedTexCoord.x = ((currentLightViewPosition.x / currentLightViewPosition.w) / 2.0f + 0.5f) + textureCoordOffset.x;
						projectedTexCoord.y = ((-currentLightViewPosition.y / currentLightViewPosition.w) / 2.0f + 0.5f) + textureCoordOffset.y;

						if (i == 0)
						{
							depthValue = txWhiteLightDepthMap.Sample(samClamp, projectedTexCoord).r;
						}
						else if (i == 1)
						{
							depthValue = txRedLightDepthMap.Sample(samClamp, projectedTexCoord).r;
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

				if (inShadow == 0.0f)
				{
					inShadow = CalculateIfInShadow(input, eyeVec, lightVec, vFinalCoords);
				}

				if (inShadow == 1.0f)
				{
					diffuseAmount = 0.0f;
				}
			}

			// Compute the ambient, diffuse, and specular terms separately.
			specular += CalculateSpecularLight(lights[i], lightVec, normalMap, diffuseAmount, eyeVec);
			diffuse += CalculateDiffuseLight(lights[i], diffuseAmount);
			ambient += CalculateAmbientLight(lights[i], textureColour);
		}
	}

	// Sum all the terms together and copy over the diffuse alpha.
	if (HasTexture == 1.0f)
	{
		finalColour.rgb = (textureColour.rgb * (ambient + diffuse)) + specular;
	}
	else
	{
		finalColour.rgb = ambient + diffuse + specular;
	}

	finalColour.a = surface.DiffuseMtrl.a;

	// Clear Colour (Blue)
	//finalColour = float4(0.0f, 0.0f, 0.5f, 1.0f);

	// Depth Map
	//float eyeDepthValue = (input.PosH.z + parallaxHeight) / input.PosH.w;
	//float4 colour = float4(depthValue, depthValue, depthValue, 1.0f);
	//float4 colour = float4(lightDepthValue, lightDepthValue, lightDepthValue, 1.0f);

	return finalColour;
}