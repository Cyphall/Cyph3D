#version 460 core
#extension GL_ARB_bindless_texture : enable

/* ------ consts ------ */
const float PI = 3.14159265359;
const float TWO_PI = PI*2;
const float SQRT_2 = 1.41421356237;

/* ------ data structures ------ */
struct PointLight
{
	vec3  pos;
	float intensity;
	vec3  color;
	bool  castShadows;
	layout(bindless_sampler) samplerCube shadowMap;
	float far;
	float maxTexelSizeAtUnitDistance;
};

struct DirectionalLight
{
	vec3  fragToLightDirection;
	float intensity;
	vec3  color;
	bool  castShadows;
	mat4  lightViewProjection;
	layout(bindless_sampler) sampler2D shadowMap;
	float mapSize;
	float mapDepth;
};

/* ------ inputs ------ */
in V2F
{
	vec3 fragPos;
	vec2 texCoords;
	vec3 T;
	vec3 N;
} v2f;

/* ------ uniforms ------ */
layout(std430, binding = 0) buffer UselessNameBecauseItIsNeverUsedAnywhere1
{
	PointLight pointLights[];
};

layout(std430, binding = 1) buffer UselessNameBecauseItIsNeverUsedAnywhere2
{
	DirectionalLight directionalLights[];
};

layout(bindless_sampler) uniform sampler2D u_albedoMap;
layout(bindless_sampler) uniform sampler2D u_normalMap;
layout(bindless_sampler) uniform sampler2D u_roughnessMap;
layout(bindless_sampler) uniform sampler2D u_metalnessMap;
layout(bindless_sampler) uniform sampler2D u_displacementMap;
layout(bindless_sampler) uniform sampler2D u_emissiveMap;

uniform mat4  u_viewProjectionInv;
uniform vec3  u_viewPos;
uniform int   u_objectIndex;

/* ------ outputs ------ */
layout(location = 0) out vec4 o_color;
layout(location = 1) out int o_objectIndex;

/* ------ function declarations ------ */
float getDepth(vec2 texCoords);
vec2 POM(vec2 texCoords, vec3 viewDir);

vec2 VogelDiskSample(int sampleIndex, int samplesCount, float phi);
float InterleavedGradientNoise(vec2 position_screen);

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
	vec3 T = normalize(v2f.T);
	vec3 N = normalize(v2f.N);
	vec3 B = normalize(cross(v2f.N, v2f.T));
	mat3 tangentToWorld = mat3(T, B, N);
	mat3 worldToTangent = transpose(tangentToWorld);
	vec3 viewDir = normalize(u_viewPos - v2f.fragPos);
	
	// ----------------- displacement -----------------
	
	vec2 texCoords = POM(v2f.texCoords, normalize(worldToTangent * viewDir));
	
	// ----------------- albedo -----------------
	
	vec3 albedo = texture(u_albedoMap, texCoords).rgb;
	
	// ----------------- normal -----------------
	
	vec3 normal = vec3(0);
	normal.xy = texture(u_normalMap, texCoords).rg * 2.0 - 1.0;
	normal.z = sqrt(1 - min(dot(normal.xy, normal.xy), 1));
	normal = tangentToWorld * normal;
	
	// ----------------- roughness -----------------
	
	float roughness = texture(u_roughnessMap, texCoords).r;
	
	// ----------------- metalness -----------------
	
	float metalness = texture(u_metalnessMap, texCoords).r;
	
	// ----------------- emissive -----------------
	
	float emissive = texture(u_emissiveMap, texCoords).r;
	
	// ----------------- object index -----------------
	
	o_objectIndex = u_objectIndex;
	
	// ----------------- geometry normal -----------------
	
	vec3 geometryNormal = normalize(cross(dFdx(v2f.fragPos), dFdy(v2f.fragPos)));
	
	// ----------------- position -----------------
	
	vec3 fragPos = v2f.fragPos;
	
	// ----------------- position -----------------
	
	float depth = gl_FragCoord.z;
	
	
	vec3 F0 = mix(vec3(0.04), albedo, metalness);
	
	// aka Lo
	vec3 finalColor = albedo * emissive;
	
	// Point Light calculation
	for (int i = 0; i < pointLights.length(); ++i)
	{
		float shadow = pointLights[i].castShadows ? isInPointShadow(i, fragPos, geometryNormal) : 0;
		
		// calculate light parameters
		vec3  lightDir    = normalize(pointLights[i].pos - fragPos);
		vec3  halfwayDir  = normalize(viewDir + lightDir);
		float distance    = length(pointLights[i].pos - fragPos);
		float attenuation = 1.0 / (1 + distance * distance);
		vec3  radiance    = pointLights[i].color * pointLights[i].intensity * attenuation;
		
		finalColor += calculateLighting(radiance, lightDir, viewDir, halfwayDir, albedo, normal, roughness, metalness, F0) * (1 - shadow);
	}
	
	// Directional Light calculation
	for (int i = 0; i < directionalLights.length(); ++i)
	{
		float shadow = directionalLights[i].castShadows ? isInDirectionalShadow(i, fragPos, geometryNormal) : 0;
		
		// calculate light parameters
		vec3 lightDir    = directionalLights[i].fragToLightDirection;
		vec3 halfwayDir  = normalize(viewDir + lightDir);
		vec3 radiance    = directionalLights[i].color * directionalLights[i].intensity;
		
		finalColor += calculateLighting(radiance, lightDir, viewDir, halfwayDir, albedo, normal, roughness, metalness, F0) * (1 - shadow);
	}
	
	o_color = vec4(finalColor, 1);
}

float getDepth(vec2 texCoords)
{
	return 1 - texture(u_displacementMap, texCoords).r;
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

float InterleavedGradientNoise(vec2 position_screen)
{
	vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
	return fract(magic.z * fract(dot(position_screen, magic.xy)));
}

vec3 calculateNormalBias(vec3 fragNormal, vec3 lightDir, float texelSize_WS, float samplingRadius)
{
	float angle = acos(dot(fragNormal, lightDir));
	
	float worstCaseTexelSize_WS = texelSize_WS * SQRT_2;
	float worstCaseTexelRadius_WS = worstCaseTexelSize_WS * 0.5;
	
	float finalRadius = worstCaseTexelSize_WS + samplingRadius * texelSize_WS;
	
	return fragNormal * (finalRadius * sin(angle)) + fragNormal * texelSize_WS;
}

float isInDirectionalShadow(int lightIndex, vec3 fragPos, vec3 geometryNormal)
{
	float texelSize = 1.0 / textureSize(directionalLights[lightIndex].shadowMap, 0).x;
	float texelSize_WS = texelSize * directionalLights[lightIndex].mapSize;
	
	float samplingRadius = 3;
	
	fragPos += calculateNormalBias(geometryNormal, directionalLights[lightIndex].fragToLightDirection, texelSize_WS, samplingRadius);
	
	vec4 shadowMapSpacePos = directionalLights[lightIndex].lightViewProjection * vec4(fragPos, 1);
	vec3 projCoords = shadowMapSpacePos.xyz / shadowMapSpacePos.w;
	projCoords = projCoords * 0.5 + 0.5;
	
	if (projCoords.z > 1) return 0.0;
	
	vec2  fragUV_SMV    = projCoords.xy;
	float fragDepth_SMV = projCoords.z;
	
	const float bias = 0.0002;
	float samplingRadiusNormalized = samplingRadius * texelSize;
	float phi = InterleavedGradientNoise(gl_FragCoord.xy) * TWO_PI;
	
	float shadow = 0.0;
	
	const int sampleCount = 16;
	for (int i = 0; i < sampleCount; i++)
	{
		vec2 uvOffset = VogelDiskSample(i, sampleCount, phi) * samplingRadiusNormalized;
		float sampleDepth = texture(directionalLights[lightIndex].shadowMap, fragUV_SMV + uvOffset).r;
		
		if (fragDepth_SMV - bias > sampleDepth)
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
	vec3 lightToFrag = fragPos - pointLights[lightIndex].pos;
	float fragDist = length(lightToFrag);
	
	float texelSize_WS = pointLights[lightIndex].maxTexelSizeAtUnitDistance * fragDist;
	
	float samplingRadius = 3;
	
	fragPos += calculateNormalBias(geometryNormal, -lightToFrag / fragDist, texelSize_WS, samplingRadius);
	
	// recalculate with normal-biased frag position
	lightToFrag = fragPos - pointLights[lightIndex].pos;
	fragDist = length(lightToFrag);
	
	float fragDepth_SMV = fragDist;
	
	vec3 forward = lightToFrag / fragDist;
	vec3 up = vec3(0, 1, 0);
	vec3 left = normalize(cross(forward, up));
	up = cross(left, forward);
	
	const float bias = 0.0002;
	float samplingRadiusNormalized = samplingRadius * pointLights[lightIndex].maxTexelSizeAtUnitDistance;
	float phi = InterleavedGradientNoise(gl_FragCoord.xy) * TWO_PI;
	
	float shadow = 0.0;
	
	const int sampleCount = 16;
	for (int i = 0; i < sampleCount; i++)
	{
		vec2 uvOffset = VogelDiskSample(i, sampleCount, phi) * samplingRadiusNormalized;
		vec3 posOffset = (left * uvOffset.x) + (up * uvOffset.y);
		float sampleDepth = texture(pointLights[lightIndex].shadowMap, forward + posOffset).r;
		
		// it is currently in linear range between [0,1]. Re-transform back to original value
		sampleDepth *= pointLights[lightIndex].far;
		
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