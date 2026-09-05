#include "../common/vertex types.slang"

__generic<T : __BuiltinFloatingPointType, let N : int>
[require(glsl, GLSL_400)]
[require(spirv)]
public matrix<T,N,N> inverse(matrix<T,N,N> m)
{
	return spirv_asm {
		OpExtInst $$matrix<T,N,N> result glsl450 MatrixInverse $m
	};
}

static const float PI = 3.14159265359;
static const float TWO_PI = PI*2;
static const float SQRT_2 = 1.41421356237;

struct GPUDirectionalLight
{
	float3 fragToLightDirection;
	float intensity;
	float3 color;
	bool castShadows;
	float4x4 vpMatrix;
	Texture2D<float>::Handle image;
	float shadowMapTexelWorldSize;
};

struct GPUPointLight
{
	float3 pos;
	float intensity;
	float3 color;
	bool castShadows;
	TextureCube<float>::Handle image;
	float maxTexelSizeAtUnitDistance;
};

cbuffer Parameters
{
	GPUDirectionalLight* u_directionalLightList;
	GPUPointLight* u_pointLightList;
	SamplerState::Handle u_materialSampler;
	SamplerComparisonState::Handle u_directionalLightSampler;
	SamplerComparisonState::Handle u_pointLightSampler;
	float3 u_viewPos;
	uint32_t u_frameIndex;
	uint32_t u_directionalLightCount;
	uint32_t u_pointLightCount;
	float u_pointLightMaxDistance;

	float3x3 u_normalMatrix;
	float4x4 u_mMatrix;
	float4x4 u_mvpMatrix;
	PositionVertexData* u_positionVertexList;
	MaterialVertexData* u_materialVertexList;
	Texture2D<float3>::Handle u_albedoImage;
	Texture2D<float2>::Handle u_normalImage;
	Texture2D<float>::Handle u_roughnessImage;
	Texture2D<float>::Handle u_metalnessImage;
	Texture2D<float>::Handle u_displacementImage;
	Texture2D<float>::Handle u_emissiveImage;
	float3 u_albedoValue;
	float u_roughnessValue;
	float u_metalnessValue;
	float u_displacementScale;
	float u_emissiveScale;
};

struct VIn
{
	uint vertex_index : SV_VulkanVertexID;
};

struct V2F
{
	float4 rasterPosition : SV_Position;
	float3 pos;
	float2 texCoord;
	float3 normal;
	float3 tangent;
	float3 bitangent;
};

struct FOut
{
	float4 color : SV_Target0;
};

[[shader("vertex")]]
V2F main(VIn input)
{
	PositionVertexData positionVertex = u_positionVertexList[input.vertex_index];
	MaterialVertexData materialVertex = u_materialVertexList[input.vertex_index];

	float3 normal = normalize(mul(u_normalMatrix, materialVertex.normal));
	float3 tangent = normalize(mul(u_normalMatrix, materialVertex.tangent.xyz));
	float3 bitangent = cross(normal, tangent) * materialVertex.tangent.w;

	V2F output;
	output.rasterPosition = mul(u_mvpMatrix, float4(positionVertex.pos, 1.0));
	output.pos = mul(u_mMatrix, float4(positionVertex.pos, 1.0)).xyz;
	output.texCoord = materialVertex.texCoord;
	output.normal = normal;
	output.tangent = tangent;
	output.bitangent = bitangent;
	return output;
}

float getDepth(float2 texCoord)
{
	return 1.0 - u_displacementImage.Sample(u_materialSampler, texCoord);
}

uint2 getExtent<T:ITexelElement>(Texture2D<T>::Handle image)
{
	uint2 extent;
	image.GetDimensions(extent.x, extent.y);
	return extent;
}

float2 POM(float2 texCoord, float3 viewDir)
{
	if (uint2(u_displacementImage).x == 0)
		return texCoord;

	const int linearSamples = 8;
	const int binarySamples = 6;

	// Initial sampling pass
	float2 currentTexCoord = texCoord;

	float currentTexDepth = getDepth(currentTexCoord);
	float previousTexDepth;

	if (currentTexDepth == 0 || linearSamples == 0) return texCoord;

	if (viewDir.z <= 0) return texCoord;

	// Offsets applied at each steps
	float2 texCoordStepOffset = -(viewDir.xy / viewDir.z) / linearSamples * u_displacementScale;
	float  depthStepOffset    = 1.0 / linearSamples;

	float currentDepth = 0;

	while (currentDepth < currentTexDepth)
	{
		currentTexCoord += texCoordStepOffset;

		previousTexDepth = currentTexDepth;
		currentTexDepth = getDepth(currentTexCoord);

		currentDepth += depthStepOffset;
	}

	float2 previousTexCoord = currentTexCoord - texCoordStepOffset;
	float previousDepth = currentDepth - depthStepOffset;

	// Resampling pass

	for (int i = 0; i < binarySamples; i++)
	{
		texCoordStepOffset *= 0.5;
		depthStepOffset *= 0.5;

		float2 halfwayTexCoord = previousTexCoord + texCoordStepOffset;
		float  halfwayTexDepth = getDepth(halfwayTexCoord);
		float  halfwayDepth    = previousDepth + depthStepOffset;

		// If we are still above the surface
		if (halfwayDepth < halfwayTexDepth)
		{
			previousTexCoord = halfwayTexCoord;
			previousTexDepth = halfwayTexDepth;
			previousDepth    = halfwayDepth;
		}
		else
		{
			currentTexCoord = halfwayTexCoord;
			currentTexDepth = halfwayTexDepth;
			currentDepth    = halfwayDepth;
		}
	}

	// Interpolation
	float afterDepth  = currentTexDepth - currentDepth;
	float beforeDepth = previousTexDepth - currentDepth + depthStepOffset;

	float weight = afterDepth / (afterDepth - beforeDepth);
	texCoord = previousTexCoord * weight + currentTexCoord * (1.0 - weight);

	return texCoord;
}

// Based on the code at https://www.gamedev.net/tutorials/programming/graphics/contact-hardening-soft-shadows-made-fast-r4906/
float2 VogelDiskSample(int sampleIndex, int samplesCount, float phi)
{
	float GoldenAngle = 2.39996322973;

	float r = sqrt(sampleIndex + 0.5) / sqrt(samplesCount);
	float theta = sampleIndex * GoldenAngle + phi;

	float sine = sin(theta);
	float cosine = cos(theta);

	return float2(r * cosine, r * sine);
}

float4 getRandom(uint2 texel)
{
	uint4 v = uint4(texel, u_frameIndex, 0);

	v = v * 1664525u + 1013904223u;
	v.x += v.y*v.w; v.y += v.z*v.x; v.z += v.x*v.y; v.w += v.y*v.z;
	v ^= v >> 16u;
	v.x += v.y*v.w; v.y += v.z*v.x; v.z += v.x*v.y; v.w += v.y*v.z;

	return clamp(v * (1.0 / float(0xffffffffu)), 0.0, 1.0);
}

float3 calculateNormalBias(float3 fragNormal, float3 lightDir, float texelSize_WS, float samplingRadius)
{
	float angle = acos(min(dot(fragNormal, lightDir), 1.0));
	float biasScale = sin(angle);

	float worstCastFilterRadius_WS = (samplingRadius + SQRT_2 * 0.5) * texelSize_WS;

	return fragNormal * worstCastFilterRadius_WS * biasScale;
}

float isInDirectionalShadow(uint2 texel, int lightIndex, float3 fragPos, float3 geometryNormal)
{
	float texelSize = 1.0 / getExtent(u_directionalLightList[lightIndex].image).x;
	float texelSize_WS = u_directionalLightList[lightIndex].shadowMapTexelWorldSize;

	float samplingRadius = 3;

	fragPos += calculateNormalBias(geometryNormal, u_directionalLightList[lightIndex].fragToLightDirection, texelSize_WS, 0);

	float4 shadowMapSpacePos = mul(u_directionalLightList[lightIndex].vpMatrix, float4(fragPos, 1));
	float3 projCoords = shadowMapSpacePos.xyz / shadowMapSpacePos.w;
	projCoords.xy = projCoords.xy * 0.5 + 0.5;

	if (projCoords.z > 1) return 0.0;

	float2 fragUV_SMV    = projCoords.xy;
	float  fragDepth_SMV = projCoords.z;

	// black magic trickery for per-texel depth bias from https://learn.microsoft.com/en-us/windows/win32/dxtecharts/cascaded-shadow-maps#filtering-shadow-maps
	// this allows to have a mush smaller normal bias which is no longer dependant on the sampling radius

	float3 vShadowTexDDX = ddx(projCoords);
	float3 vShadowTexDDY = ddy(projCoords);

	float2x2 matScreenToShadow = transpose(float2x2(vShadowTexDDX.xy, vShadowTexDDY.xy));
	float2x2 matShadowToScreen = inverse(matScreenToShadow);

	float2 vRightShadowTexelLocation = float2(texelSize, 0.0);
	float2 vUpShadowTexelLocation = float2(0.0, texelSize);
	float2 vRightTexelDepthRatio = mul(matShadowToScreen, vRightShadowTexelLocation);
	float2 vUpTexelDepthRatio = mul(matShadowToScreen, vUpShadowTexelLocation);

	float fUpTexelDepthDelta = vUpTexelDepthRatio.x * vShadowTexDDX.z + vUpTexelDepthRatio.y * vShadowTexDDY.z;
	float fRightTexelDepthDelta = vRightTexelDepthRatio.x * vShadowTexDDX.z + vRightTexelDepthRatio.y * vShadowTexDDY.z;

	float phi = getRandom(texel).x * TWO_PI;

	const float bias = 0.0002;
	const int sampleCount = 8;
	float shadow = 0.0;
	for (int i = 0; i < sampleCount; i++)
	{
		float2 sampleOffset = VogelDiskSample(i, sampleCount, phi) * samplingRadius;
		float2 uvOffset = sampleOffset * texelSize;
		float expectedDepth = fragDepth_SMV + fRightTexelDepthDelta * sampleOffset.x + fUpTexelDepthDelta * sampleOffset.y;
		shadow += u_directionalLightList[lightIndex].image.SampleCmpLevelZero(u_directionalLightSampler, fragUV_SMV + uvOffset, expectedDepth - bias);
	}
	shadow /= sampleCount;

	return shadow;
}

float isInPointShadow(uint2 texel, int lightIndex, float3 fragPos, float3 geometryNormal)
{
	// get vector between fragment position and light position
	float3 lightToFrag = fragPos - u_pointLightList[lightIndex].pos;
	float fragDist = length(lightToFrag);

	float texelSize_WS = u_pointLightList[lightIndex].maxTexelSizeAtUnitDistance * fragDist;

	float samplingRadius = 3;

	fragPos += calculateNormalBias(geometryNormal, -lightToFrag / fragDist, texelSize_WS, samplingRadius);

	// recalculate with normal-biased frag position
	lightToFrag = fragPos - u_pointLightList[lightIndex].pos;
	fragDist = length(lightToFrag);

	float fragDepth_SMV = fragDist;

	float3 forward = lightToFrag / fragDist;
	forward.z = -forward.z;
	float3 up = abs(dot(forward, float3(0, 1, 0))) > 0.9 ? float3(1, 0, 0) : float3(0, 1, 0);
	float3 left = normalize(cross(forward, up));
	up = cross(left, forward);

	const float bias = 0.0002;
	float samplingRadiusNormalized = samplingRadius * u_pointLightList[lightIndex].maxTexelSizeAtUnitDistance;
	float phi = getRandom(texel).x * TWO_PI;

	float shadow = 0.0;

	const int sampleCount = 8;
	for (int i = 0; i < sampleCount; i++)
	{
		float2 uvOffset = VogelDiskSample(i, sampleCount, phi) * samplingRadiusNormalized;
		float3 posOffset = (left * uvOffset.x) + (up * uvOffset.y);
		float expectedDepth = fragDepth_SMV;
		shadow += u_pointLightList[lightIndex].image.SampleCmpLevelZero(u_pointLightSampler, forward + posOffset, (expectedDepth - bias) / u_pointLightMaxDistance);
	}
	shadow /= sampleCount;

	return shadow;
}

// Normal Distribution Function
float D_GGX(float NdotH, float alpha)
{
	float alpha2 = alpha * alpha;

	float num   = alpha2;
	float denom = (NdotH * NdotH * (alpha2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / max(denom, 0.0000001);
}

// Geometry Shadowing Function
float G_SmithGGX(float NdotV, float NdotL, float alpha)
{
	float alpha2 = alpha * alpha;

	float ggx1 = (2.0 * NdotV) / (NdotV + sqrt(alpha2 + (1 - alpha2) * NdotV * NdotV));
	float ggx2 = (2.0 * NdotL) / (NdotL + sqrt(alpha2 + (1 - alpha2) * NdotL * NdotL));

	return isnan(ggx1) || isnan(ggx2) ? 0.0 : ggx1 * ggx2;
}

// Fresnel Function
float3 F_Schlick(float HdotV, float3 F0, float3 F90)
{
	return F0 + (F90 - F0) * pow(1.0 - HdotV, 5.0);
}

float3 calculateLighting(float3 radiance, float3 lightDir, float3 viewDir, float3 albedo, float3 normal, float roughness, float metalness)
{
	float3 halfwayDir = normalize(viewDir + lightDir);

	float HdotV = clamp(dot(halfwayDir, viewDir), 0.0, 1.0);
	float NdotV = clamp(dot(normal, viewDir),     0.0, 1.0);
	float NdotL = clamp(dot(normal, lightDir),    0.0, 1.0);
	float NdotH = clamp(dot(normal, halfwayDir),  0.0, 1.0);

	// ---------- Specular BRDF (Cook-Torrance) ----------

	float alpha = roughness * roughness;

	float D = D_GGX(NdotH, alpha);
	float G = G_SmithGGX(NdotV, NdotL, alpha);

	float numerator   = D * G;
	float denominator = 4.0 * NdotV * NdotL;

	float specularBRDF = numerator / max(denominator, 0.001);

	// ---------- Diffuse BRDF (Lambertian) ----------

	float3 diffuseBRDF = albedo / PI;

	// ---------- Dielectric and conductor terms ----------

	// dielectric terms
	float3 dielectricSpecularWeight = F_Schlick(HdotV, float3(0.04), float3(1.0));
	float3 dielectricDiffuseWeight  = float3(1.0) - dielectricSpecularWeight;

	// conductor terms
	float3 conductorSpecularWeight  = F_Schlick(HdotV, albedo, float3(1.0));
	float3 conductorDiffuseWeight   = float3(0);

	// ---------- Result ----------

	// combined terms
	float3 specularWeight = lerp(dielectricSpecularWeight, conductorSpecularWeight, metalness);
	float3 diffuseWeight  = lerp(dielectricDiffuseWeight,  conductorDiffuseWeight,  metalness);

	float3 brdf = (diffuseWeight * diffuseBRDF) + (specularWeight * specularBRDF);

	return brdf * radiance * NdotL;
}

// Based on the code at https://learnopengl.com/PBR/Lighting by Joey de Vries (https://twitter.com/JoeyDeVriez)
[[shader("fragment")]]
FOut main(V2F input)
{
	float3x3 worldToTangent = float3x3(
		normalize(input.tangent), 
		normalize(input.bitangent), 
		normalize(input.normal)
	);
	float3x3 tangentToWorld = transpose(worldToTangent);
	float3 viewDir = normalize(u_viewPos - input.pos);

	// ----------------- displacement -----------------

	float2 texCoord = POM(input.texCoord, normalize(mul(worldToTangent, viewDir)));

	// ----------------- albedo -----------------

	float3 albedo = uint2(u_albedoImage).x > 0 ? u_albedoImage.Sample(u_materialSampler, texCoord) : u_albedoValue;

	// ----------------- normal -----------------

	float3 normal = float3(0);
	normal.xy = uint2(u_normalImage).x > 0 ? u_normalImage.Sample(u_materialSampler, texCoord) * 2.0 - 1.0 : float2(0.0, 0.0);
	normal.z = sqrt(1 - min(dot(normal.xy, normal.xy), 1));
	normal = mul(tangentToWorld, normal);

	// ----------------- roughness -----------------

	float roughness = uint2(u_roughnessImage).x > 0 ? u_roughnessImage.Sample(u_materialSampler, texCoord) : u_roughnessValue;

	// ----------------- metalness -----------------

	float metalness = uint2(u_metalnessImage).x > 0 ? u_metalnessImage.Sample(u_materialSampler, texCoord) : u_metalnessValue;

	// ----------------- emissive -----------------

	float emissive = (uint2(u_emissiveImage).x > 0 ? u_emissiveImage.Sample(u_materialSampler, texCoord) : 1.0) * u_emissiveScale;

	// ----------------- geometry normal -----------------

	float3 geometryNormal = normalize(cross(ddy(input.pos), ddx(input.pos)));

	uint2 texel = uint2(input.rasterPosition.xy);

	// aka Lo
	float3 finalColor = albedo * emissive;

	// Directional Light calculation
	for (int i = 0; i < u_directionalLightCount; ++i)
	{
		float shadow = u_directionalLightList[i].castShadows ? isInDirectionalShadow(texel, i, input.pos, geometryNormal) : 0;

		// calculate light parameters
		float3 lightDir = u_directionalLightList[i].fragToLightDirection;
		float3 radiance = u_directionalLightList[i].color * u_directionalLightList[i].intensity;

		finalColor += calculateLighting(radiance, lightDir, viewDir, albedo, normal, roughness, metalness) * (1 - shadow);
	}

	// Point Light calculation
	for (int i = 0; i < u_pointLightCount; ++i)
	{
		float shadow = u_pointLightList[i].castShadows ? isInPointShadow(texel, i, input.pos, geometryNormal) : 0;

		// calculate light parameters
		float3 lightDir    = normalize(u_pointLightList[i].pos - input.pos);
		float  distance    = length(u_pointLightList[i].pos - input.pos);
		float  attenuation = 1.0 / (1 + distance * distance);
		float3 radiance    = u_pointLightList[i].color * u_pointLightList[i].intensity * attenuation;

		finalColor += calculateLighting(radiance, lightDir, viewDir, albedo, normal, roughness, metalness) * (1 - shadow);
	}
	
	FOut output;
	output.color = float4(finalColor, 1);
	return output;
}