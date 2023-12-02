#version 460 core
#extension GL_EXT_nonuniform_qualifier : require

/* ------ consts ------ */
const float PI = 3.14159265359;
const float TWO_PI = PI*2;
const float SQRT_2 = 1.41421356237;

/* ------ data structures ------ */
struct PointLightUniforms
{
	vec3  pos;
	float intensity;
	vec3  color;
	bool  castShadows;
	uint  textureIndex;
	float maxTexelSizeAtUnitDistance;
};

struct DirectionalLightUniforms
{
	vec3  fragToLightDirection;
	float intensity;
	vec3  color;
	bool  castShadows;
	mat4  lightViewProjection;
	uint  textureIndex;
	float shadowMapTexelWorldSize;
};

struct ObjectUniforms
{
	mat4 normalMatrix;
	mat4 model;
	mat4 mvp;
	int albedoIndex;
	int normalIndex;
	int roughnessIndex;
	int metalnessIndex;
	int displacementIndex;
	int emissiveIndex;
	vec3 albedoValue;
	float roughnessValue;
	float metalnessValue;
	float displacementScale;
	float emissiveScale;
};

/* ------ inputs ------ */
layout(location = 0) in V2F
{
	vec3 i_fragPos;
	vec2 i_texCoords;
	vec3 i_T;
	vec3 i_N;
};

/* ------ uniforms ------ */
layout(set = 0, binding = 0) uniform sampler2D u_textures[];

layout(std430, set = 1, binding = 0) readonly buffer UselessNameBecauseItIsNeverUsedAnywhere1
{
	DirectionalLightUniforms u_directionalLightUniforms[];
};
layout(set = 1, binding = 1) uniform sampler2D u_directionalLightTextures[];

layout(std430, set = 2, binding = 0) readonly buffer UselessNameBecauseItIsNeverUsedAnywhere2
{
	PointLightUniforms u_pointLightUniforms[];
};
layout(set = 2, binding = 1) uniform samplerCube u_pointLightTextures[];

layout(std430, set = 3, binding = 0) readonly buffer UselessNameBecauseItIsNeverUsedAnywhere3
{
	ObjectUniforms u_objectUniforms;
};

layout(push_constant) uniform constants
{
	vec3 u_viewPos;
	uint u_frameIndex;
};

/* ------ outputs ------ */
layout(location = 0) out vec4 o_color;

/* ------ code ------ */

float getDepth(vec2 texCoords)
{
	return u_objectUniforms.displacementIndex >= 0 ? 1.0 - texture(u_textures[u_objectUniforms.displacementIndex], texCoords).r : 0.0;
}

vec2 POM(vec2 texCoords, vec3 viewDir)
{
	const int   linearSamples  = 8;
	const int   binarySamples  = 6;

	// Initial sampling pass
	vec2 currentTexCoords = texCoords;

	float currentTexDepth  = getDepth(currentTexCoords);
	float previousTexDepth;

	if (currentTexDepth == 0 || linearSamples == 0) return texCoords;

	if (viewDir.z <= 0) return texCoords;

	// Offsets applied at each steps
	vec2  texCoordsStepOffset = -(viewDir.xy / viewDir.z) / linearSamples * u_objectUniforms.displacementScale;
	float depthStepOffset     = 1.0 / linearSamples;

	float currentDepth = 0;

	while (currentDepth < currentTexDepth)
	{
		currentTexCoords += texCoordsStepOffset;

		previousTexDepth = currentTexDepth;
		currentTexDepth = getDepth(currentTexCoords);

		currentDepth += depthStepOffset;
	}

	vec2 previousTexCoords = currentTexCoords - texCoordsStepOffset;
	float previousDepth = currentDepth - depthStepOffset;

	// Resampling pass

	for (int i = 0; i < binarySamples; i++)
	{
		texCoordsStepOffset *= 0.5;
		depthStepOffset *= 0.5;

		vec2  halfwayTexCoords = previousTexCoords + texCoordsStepOffset;
		float halfwayTexDepth  = getDepth(halfwayTexCoords);
		float halfwayDepth     = previousDepth + depthStepOffset;

		// If we are still above the surface
		if (halfwayDepth < halfwayTexDepth)
		{
			previousTexCoords = halfwayTexCoords;
			previousTexDepth  = halfwayTexDepth;
			previousDepth     = halfwayDepth;
		}
		else
		{
			currentTexCoords = halfwayTexCoords;
			currentTexDepth  = halfwayTexDepth;
			currentDepth     = halfwayDepth;
		}
	}

	// Interpolation
	float afterDepth  = currentTexDepth - currentDepth;
	float beforeDepth = previousTexDepth - currentDepth + depthStepOffset;

	float weight = afterDepth / (afterDepth - beforeDepth);
	texCoords = previousTexCoords * weight + currentTexCoords * (1.0 - weight);

	return texCoords;
}

// Based on the code at https://www.gamedev.net/tutorials/programming/graphics/contact-hardening-soft-shadows-made-fast-r4906/
vec2 VogelDiskSample(int sampleIndex, int samplesCount, float phi)
{
	float GoldenAngle = 2.39996322973;

	float r = sqrt(sampleIndex + 0.5) / sqrt(samplesCount);
	float theta = sampleIndex * GoldenAngle + phi;

	float sine = sin(theta);
	float cosine = cos(theta);

	return vec2(r * cosine, r * sine);
}

vec4 getRandom()
{
	uvec4 v = uvec4(uvec2(gl_FragCoord.xy), u_frameIndex, 0);

	v = v * 1664525u + 1013904223u;
	v.x += v.y*v.w; v.y += v.z*v.x; v.z += v.x*v.y; v.w += v.y*v.z;
	v ^= v >> 16u;
	v.x += v.y*v.w; v.y += v.z*v.x; v.z += v.x*v.y; v.w += v.y*v.z;

	return clamp(v * (1.0 / float(0xffffffffu)), 0.0, 1.0);
}

vec3 calculateNormalBias(vec3 fragNormal, vec3 lightDir, float texelSize_WS, float samplingRadius)
{
	float angle = acos(min(dot(fragNormal, lightDir), 1.0));
	float biasScale = sin(angle);

	float worstCastFilterRadius_WS = (samplingRadius + SQRT_2 * 0.5) * texelSize_WS;

	return fragNormal * worstCastFilterRadius_WS * biasScale;
}

float isInDirectionalShadow(int lightIndex, vec3 fragPos, vec3 geometryNormal)
{
	float texelSize = 1.0 / textureSize(u_directionalLightTextures[u_directionalLightUniforms[lightIndex].textureIndex], 0).x;
	float texelSize_WS = u_directionalLightUniforms[lightIndex].shadowMapTexelWorldSize;

	float samplingRadius = 3;

	fragPos += calculateNormalBias(geometryNormal, u_directionalLightUniforms[lightIndex].fragToLightDirection, texelSize_WS, 0);

	vec4 shadowMapSpacePos = u_directionalLightUniforms[lightIndex].lightViewProjection * vec4(fragPos, 1);
	vec3 projCoords = shadowMapSpacePos.xyz / shadowMapSpacePos.w;
	projCoords.xy = projCoords.xy * 0.5 + 0.5;

	if (projCoords.z > 1) return 0.0;

	vec2  fragUV_SMV    = projCoords.xy;
	float fragDepth_SMV = projCoords.z;

	// black magic trickery for per-texel depth bias from https://learn.microsoft.com/en-us/windows/win32/dxtecharts/cascaded-shadow-maps#filtering-shadow-maps
	// this allows to have a mush smaller normal bias which is no longer dependant on the sampling radius

	vec3 vShadowTexDDX = dFdx(projCoords);
	vec3 vShadowTexDDY = dFdy(projCoords);

	mat2 matScreentoShadow = mat2(vShadowTexDDX.xy, vShadowTexDDY.xy);
	mat2 matShadowToScreen = inverse(matScreentoShadow);

	vec2 vRightShadowTexelLocation = vec2(texelSize, 0.0);
	vec2 vUpShadowTexelLocation = vec2(0.0, texelSize);
	vec2 vRightTexelDepthRatio = matShadowToScreen * vRightShadowTexelLocation;
	vec2 vUpTexelDepthRatio = matShadowToScreen * vUpShadowTexelLocation;

	float fUpTexelDepthDelta = vUpTexelDepthRatio.x * vShadowTexDDX.z + vUpTexelDepthRatio.y * vShadowTexDDY.z;
	float fRightTexelDepthDelta = vRightTexelDepthRatio.x * vShadowTexDDX.z + vRightTexelDepthRatio.y * vShadowTexDDY.z;

	float phi = getRandom().x * TWO_PI;

	const float bias = 0.0002;
	const int sampleCount = 16;
	float shadow = 0.0;
	for (int i = 0; i < sampleCount; i++)
	{
		vec2 sampleOffset = VogelDiskSample(i, sampleCount, phi) * samplingRadius;
		vec2 uvOffset = sampleOffset * texelSize;
		float sampleDepth = texture(u_directionalLightTextures[u_directionalLightUniforms[lightIndex].textureIndex], fragUV_SMV + uvOffset).r;

		float expectedDepth = fragDepth_SMV + fRightTexelDepthDelta * sampleOffset.x + fUpTexelDepthDelta * sampleOffset.y;

		if (expectedDepth - bias > sampleDepth)
		{
			shadow++;
		}
	}
	shadow /= sampleCount;

	return shadow;
}

float isInPointShadow(int lightIndex, vec3 fragPos, vec3 geometryNormal)
{
	// get vector between fragment position and light position
	vec3 lightToFrag = fragPos - u_pointLightUniforms[lightIndex].pos;
	float fragDist = length(lightToFrag);

	float texelSize_WS = u_pointLightUniforms[lightIndex].maxTexelSizeAtUnitDistance * fragDist;

	float samplingRadius = 3;

	fragPos += calculateNormalBias(geometryNormal, -lightToFrag / fragDist, texelSize_WS, samplingRadius);

	// recalculate with normal-biased frag position
	lightToFrag = fragPos - u_pointLightUniforms[lightIndex].pos;
	fragDist = length(lightToFrag);

	float fragDepth_SMV = fragDist;

	vec3 forward = lightToFrag / fragDist;
	forward.z = -forward.z;
	vec3 up = abs(dot(forward, vec3(0, 1, 0))) > 0.9 ? vec3(1, 0, 0) : vec3(0, 1, 0);
	vec3 left = normalize(cross(forward, up));
	up = cross(left, forward);

	const float bias = 0.0002;
	float samplingRadiusNormalized = samplingRadius * u_pointLightUniforms[lightIndex].maxTexelSizeAtUnitDistance;
	float phi = getRandom().x * TWO_PI;

	float shadow = 0.0;

	const int sampleCount = 16;
	for (int i = 0; i < sampleCount; i++)
	{
		vec2 uvOffset = VogelDiskSample(i, sampleCount, phi) * samplingRadiusNormalized;
		vec3 posOffset = (left * uvOffset.x) + (up * uvOffset.y);
		float sampleDepth = texture(u_pointLightTextures[u_pointLightUniforms[lightIndex].textureIndex], forward + posOffset).r;

		if (fragDepth_SMV - bias > sampleDepth)
		{
			shadow++;
		}
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
vec3 F_Schlick(float HdotV, vec3 F0, vec3 F90)
{
	return F0 + (F90 - F0) * pow(1.0 - HdotV, 5.0);
}

vec3 calculateLighting(vec3 radiance, vec3 lightDir, vec3 viewDir, vec3 albedo, vec3 normal, float roughness, float metalness)
{
	vec3 halfwayDir = normalize(viewDir + lightDir);

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

	vec3 diffuseBRDF = albedo / PI;

	// ---------- Dielectric and conductor terms ----------

	// dielectric terms
	vec3 dielectricSpecularWeight = F_Schlick(HdotV, vec3(0.04), vec3(1.0));
	vec3 dielectricDiffuseWeight  = vec3(1.0) - dielectricSpecularWeight;

	// conductor terms
	vec3 conductorSpecularWeight  = F_Schlick(HdotV, albedo, vec3(1.0));
	vec3 conductorDiffuseWeight   = vec3(0);

	// ---------- Result ----------

	// combined terms
	vec3 specularWeight = mix(dielectricSpecularWeight, conductorSpecularWeight, metalness);
	vec3 diffuseWeight  = mix(dielectricDiffuseWeight,  conductorDiffuseWeight,  metalness);

	vec3 brdf = (diffuseWeight * diffuseBRDF) + (specularWeight * specularBRDF);

	return brdf * radiance * NdotL;
}

// Based on the code at https://learnopengl.com/PBR/Lighting by Joey de Vries (https://twitter.com/JoeyDeVriez)
void main()
{
	vec3 T = normalize(i_T);
	vec3 N = normalize(i_N);
	vec3 B = normalize(cross(i_T, i_N));
	mat3 tangentToWorld = mat3(T, B, N);
	mat3 worldToTangent = transpose(tangentToWorld);
	vec3 viewDir = normalize(u_viewPos - i_fragPos);

	// ----------------- displacement -----------------

	vec2 texCoords = POM(i_texCoords, normalize(worldToTangent * viewDir));

	// ----------------- albedo -----------------

	vec3 albedo = u_objectUniforms.albedoIndex >= 0 ? texture(u_textures[u_objectUniforms.albedoIndex], texCoords).rgb : u_objectUniforms.albedoValue;

	// ----------------- normal -----------------

	vec3 normal = vec3(0);
	normal.xy = u_objectUniforms.normalIndex >= 0 ? texture(u_textures[u_objectUniforms.normalIndex], texCoords).rg * 2.0 - 1.0 : vec2(0.0, 0.0);
	normal.z = sqrt(1 - min(dot(normal.xy, normal.xy), 1));
	normal = tangentToWorld * normal;

	// ----------------- roughness -----------------

	float roughness = u_objectUniforms.roughnessIndex >= 0 ? texture(u_textures[u_objectUniforms.roughnessIndex], texCoords).r : u_objectUniforms.roughnessValue;

	// ----------------- metalness -----------------

	float metalness = u_objectUniforms.metalnessIndex >= 0 ? texture(u_textures[u_objectUniforms.metalnessIndex], texCoords).r : u_objectUniforms.metalnessValue;

	// ----------------- emissive -----------------

	float emissive = (u_objectUniforms.emissiveIndex >= 0 ? texture(u_textures[u_objectUniforms.emissiveIndex], texCoords).r : 1.0) * u_objectUniforms.emissiveScale;

	// ----------------- geometry normal -----------------

	vec3 geometryNormal = normalize(cross(dFdy(i_fragPos), dFdx(i_fragPos)));

	// ----------------- position -----------------

	vec3 fragPos = i_fragPos;

	// aka Lo
	vec3 finalColor = albedo * emissive;

	// Directional Light calculation
	for (int i = 0; i < u_directionalLightUniforms.length(); ++i)
	{
		float shadow = u_directionalLightUniforms[i].castShadows ? isInDirectionalShadow(i, fragPos, geometryNormal) : 0;

		// calculate light parameters
		vec3 lightDir    = u_directionalLightUniforms[i].fragToLightDirection;
		vec3 radiance    = u_directionalLightUniforms[i].color * u_directionalLightUniforms[i].intensity;

		finalColor += calculateLighting(radiance, lightDir, viewDir, albedo, normal, roughness, metalness) * (1 - shadow);
	}

	// Point Light calculation
	for (int i = 0; i < u_pointLightUniforms.length(); ++i)
	{
		float shadow = u_pointLightUniforms[i].castShadows ? isInPointShadow(i, fragPos, geometryNormal) : 0;

		// calculate light parameters
		vec3  lightDir    = normalize(u_pointLightUniforms[i].pos - fragPos);
		float distance    = length(u_pointLightUniforms[i].pos - fragPos);
		float attenuation = 1.0 / (1 + distance * distance);
		vec3  radiance    = u_pointLightUniforms[i].color * u_pointLightUniforms[i].intensity * attenuation;

		finalColor += calculateLighting(radiance, lightDir, viewDir, albedo, normal, roughness, metalness) * (1 - shadow);
	}

	o_color = vec4(finalColor, 1);
}