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
	float far;
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
	int  objectIndex;
	uint albedoIndex;
	uint normalIndex;
	uint roughnessIndex;
	uint metalnessIndex;
	uint displacementIndex;
	uint emissiveIndex;
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

layout(std430, set = 1, binding = 0) buffer UselessNameBecauseItIsNeverUsedAnywhere1
{
	DirectionalLightUniforms u_directionalLightUniforms[];
};
layout(set = 1, binding = 1) uniform sampler2D u_directionalLightTextures[];

layout(std430, set = 2, binding = 0) buffer UselessNameBecauseItIsNeverUsedAnywhere2
{
	PointLightUniforms u_pointLightUniforms[];
};
layout(set = 2, binding = 1) uniform samplerCube u_pointLightTextures[];

layout(std430, set = 3, binding = 0) buffer UselessNameBecauseItIsNeverUsedAnywhere3
{
	ObjectUniforms u_objectUniforms;
};

layout(push_constant) uniform constants
{
	mat4 u_viewProjectionInv;
	vec3 u_viewPos;
	uint u_frameIndex;
};

/* ------ outputs ------ */
layout(location = 0) out vec4 o_color;
layout(location = 1) out int o_objectIndex;

/* ------ function declarations ------ */
float getDepth(vec2 texCoords);
vec2 POM(vec2 texCoords, vec3 viewDir);

vec2 VogelDiskSample(int sampleIndex, int samplesCount, float phi);
vec4 getRandom();

float isInDirectionalShadow(int lightIndex, vec3 fragPos, vec3 geometryNormal);
float isInPointShadow(int lightIndex, vec3 fragPos, vec3 geometryNormal);
vec3 calculateLighting(vec3 radiance, vec3 lightDir, vec3 viewDir, vec3 halfwayDir, vec3 albedo, vec3 normal, float roughness, float metalness, vec3 F0);
vec3 calculateBRDF(vec3 lightDir, vec3 viewDir, vec3 halfwayDir, vec3 albedo, vec3 normal, float roughness, float metalness, vec3 F0);

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

/* ------ code ------ */
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
	
	vec3 albedo = texture(u_textures[u_objectUniforms.albedoIndex], texCoords).rgb;
	
	// ----------------- normal -----------------
	
	vec3 normal = vec3(0);
	normal.xy = texture(u_textures[u_objectUniforms.normalIndex], texCoords).rg * 2.0 - 1.0;
	normal.z = sqrt(1 - min(dot(normal.xy, normal.xy), 1));
	normal = tangentToWorld * normal;
	
	// ----------------- roughness -----------------
	
	float roughness = texture(u_textures[u_objectUniforms.roughnessIndex], texCoords).r;
	
	// ----------------- metalness -----------------
	
	float metalness = texture(u_textures[u_objectUniforms.metalnessIndex], texCoords).r;
	
	// ----------------- emissive -----------------
	
	float emissive = texture(u_textures[u_objectUniforms.emissiveIndex], texCoords).r * u_objectUniforms.emissiveScale;
	
	// ----------------- object index -----------------
	
	o_objectIndex = u_objectUniforms.objectIndex;
	
	// ----------------- geometry normal -----------------
	
	vec3 geometryNormal = normalize(cross(dFdy(i_fragPos), dFdx(i_fragPos)));
	
	// ----------------- position -----------------
	
	vec3 fragPos = i_fragPos;
	
	
	vec3 F0 = mix(vec3(0.04), albedo, metalness);
	
	// aka Lo
	vec3 finalColor = albedo * emissive;

	// Directional Light calculation
	for (int i = 0; i < u_directionalLightUniforms.length(); ++i)
	{
		float shadow = u_directionalLightUniforms[i].castShadows ? isInDirectionalShadow(i, fragPos, geometryNormal) : 0;

		// calculate light parameters
		vec3 lightDir    = u_directionalLightUniforms[i].fragToLightDirection;
		vec3 halfwayDir  = normalize(viewDir + lightDir);
		vec3 radiance    = u_directionalLightUniforms[i].color * u_directionalLightUniforms[i].intensity;

		finalColor += calculateLighting(radiance, lightDir, viewDir, halfwayDir, albedo, normal, roughness, metalness, F0) * (1 - shadow);
	}
	
	// Point Light calculation
	for (int i = 0; i < u_pointLightUniforms.length(); ++i)
	{
		float shadow = u_pointLightUniforms[i].castShadows ? isInPointShadow(i, fragPos, geometryNormal) : 0;
		
		// calculate light parameters
		vec3  lightDir    = normalize(u_pointLightUniforms[i].pos - fragPos);
		vec3  halfwayDir  = normalize(viewDir + lightDir);
		float distance    = length(u_pointLightUniforms[i].pos - fragPos);
		float attenuation = 1.0 / (1 + distance * distance);
		vec3  radiance    = u_pointLightUniforms[i].color * u_pointLightUniforms[i].intensity * attenuation;
		
		finalColor += calculateLighting(radiance, lightDir, viewDir, halfwayDir, albedo, normal, roughness, metalness, F0) * (1 - shadow);
	}
	
	o_color = vec4(finalColor, 1);
}

float getDepth(vec2 texCoords)
{
	return 1 - texture(u_textures[u_objectUniforms.displacementIndex], texCoords).r;
}

vec2 POM(vec2 texCoords, vec3 viewDir)
{
	const float depthScale     = 0.05;
	const int   linearSamples  = 8;
	const int   binarySamples  = 6;
	
	// Initial sampling pass
	vec2 currentTexCoords = texCoords;
	
	float currentTexDepth  = getDepth(currentTexCoords);
	float previousTexDepth;
	
	if (currentTexDepth == 0 || linearSamples == 0) return texCoords;
	
	if (viewDir.z <= 0) return texCoords;
	
	// Offsets applied at each steps
	vec2  texCoordsStepOffset = -(viewDir.xy / viewDir.z) / linearSamples * depthScale;
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
	float GoldenAngle = 2.4;
	
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
	float angle = acos(dot(fragNormal, lightDir));
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
	vec3 up = vec3(0, 1, 0);
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
		
		// it is currently in linear range between [0,1]. Re-transform back to original value
		sampleDepth *= u_pointLightUniforms[lightIndex].far;
		
		if (fragDepth_SMV - bias > sampleDepth)
		{
			shadow++;
		}
	}
	shadow /= sampleCount;
	
	return shadow;
}

vec3 calculateLighting(vec3 radiance, vec3 lightDir, vec3 viewDir, vec3 halfwayDir, vec3 albedo, vec3 normal, float roughness, float metalness, vec3 F0)
{
	vec3 brdf = calculateBRDF(lightDir, viewDir, halfwayDir, albedo, normal, roughness, metalness, F0);
	
	return brdf * radiance * max(dot(normal, lightDir), 0.0);
}

// Cook-Torrance
vec3 calculateBRDF(vec3 lightDir, vec3 viewDir, vec3 halfwayDir, vec3 albedo, vec3 normal, float roughness, float metalness, vec3 F0)
{
	// ---------- Specular part ----------
	
	float D = DistributionGGX(normal, halfwayDir, roughness);
	vec3  F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);
	float G = GeometrySmith(normal, viewDir, lightDir, roughness);
	
	vec3  numerator   = D * F * G;
	float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0);
	vec3  specular    = numerator / max(denominator, 0.001);
	
	vec3 specularWeight = F;
	
	// ---------- Diffuse part ----------
	
	vec3 diffuse = albedo / PI;
	
	vec3 diffuseWeight = vec3(1.0) - specularWeight;
	diffuseWeight *= 1.0 - metalness; // Metals have no diffuse
	
	// ---------- Final result ----------
	
	// specularWeight already integrated to specular
	return (diffuseWeight * diffuse) + (specular);
}

// Normal Distribution Function
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a      = roughness*roughness;
	float a2     = a*a;
	float NdotH  = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;
	
	float num   = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	
	return num / max(denom, 0.0000001);
}

// Geometry Shadowing Function
float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;
	
	float num   = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	
	return num / max(denom, 0.0000001);
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2  = GeometrySchlickGGX(NdotV, roughness);
	float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
	return ggx1 * ggx2;
}

// Fresnel Function
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}