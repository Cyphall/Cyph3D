#version 460 core
#extension GL_ARB_bindless_texture : enable

/* ------ consts ------ */
const float PI = 3.14159265359;
const float TWO_PI = PI*2;
const float SQRT_2 = 1.41421356237;

/* ------ inputs from vertex shader ------ */
in V2F {
	vec2  texCoords;
} v2f;

/* ------ data structures ------ */
layout(bindless_sampler) struct PointLight
{
	vec3  pos;
	float intensity;
	vec3  color;
	bool  castShadows;
	samplerCube shadowMap;
	float far;
	float maxTexelSizeAtUnitDistance;
};

layout(bindless_sampler) struct DirectionalLight
{
	vec3  fragToLightDirection;
	float intensity;
	vec3  color;
	bool  castShadows;
	mat4  lightViewProjection;
	sampler2D shadowMap;
	float mapSize;
	float mapDepth;
};

struct FragData
{
	vec2  texCoords;
	float depth;
	vec3  pos;
	vec3  viewDir;
	vec3  normal;
	vec3  geometryNormal;
	float roughness;
	float metalness;
	vec3  color;
	float emissiveIntensity;
	vec3  F0;
} fragData;

/* ------ uniforms ------ */
layout(std430, binding = 0) buffer UselessNameBecauseItIsNeverUsedAnywhere1
{
	PointLight pointLights[];
};

layout(std430, binding = 1) buffer UselessNameBecauseItIsNeverUsedAnywhere2
{
	DirectionalLight directionalLights[];
};

layout(bindless_sampler) uniform sampler2D u_normalTexture;
layout(bindless_sampler) uniform sampler2D u_colorTexture;
layout(bindless_sampler) uniform sampler2D u_materialTexture;
layout(bindless_sampler) uniform sampler2D u_geometryNormalTexture;
layout(bindless_sampler) uniform sampler2D u_depthTexture;
layout(bindless_sampler) uniform sampler2D u_position;

uniform mat4  u_viewProjectionInv;
uniform vec3  u_viewPos;
uniform float u_time;

/* ------ outputs ------ */
layout(location = 0) out vec4 o_color;

/* ------ function declarations ------ */
vec4 lighting();

vec2 VogelDiskSample(int sampleIndex, int samplesCount, float phi);
float InterleavedGradientNoise(vec2 position_screen);

float isInDirectionalShadow(int lightIndex);
float isInPointShadow(int lightIndex);
vec3 calculateLighting(vec3 radiance, vec3 lightDir, vec3 halfwayDir);
vec3 calculateBRDF(vec3 lightDir, vec3 halfwayDir);

vec3 getPosition();
vec3 getColor();
vec3 getNormal();
vec3 getGeometryNormal();
float getRoughness();
float getMetallic();
float getEmissive();
float getDepth();
int isLit();

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

/* ------ code ------ */
void main()
{
	// Mandatory data
	fragData.texCoords = v2f.texCoords;
	
	if (isLit() == 0)
	{
		o_color = vec4(getColor(), 1);
	}
	else
	{
		o_color = lighting();
	}
}

// Based on the code at https://learnopengl.com/PBR/Lighting by Joey de Vries (https://twitter.com/JoeyDeVriez)
vec4 lighting()
{
	fragData.color             = getColor();
	fragData.depth             = getDepth();
	fragData.pos               = getPosition();
	fragData.viewDir           = normalize(u_viewPos - fragData.pos);
	fragData.normal            = getNormal();
	fragData.geometryNormal    = getGeometryNormal();
	fragData.roughness         = getRoughness();
	fragData.metalness         = getMetallic();
	fragData.emissiveIntensity = getEmissive();
	fragData.F0                = mix(vec3(0.04), fragData.color, fragData.metalness);
	
	if (fragData.depth == 1)
	{
		return vec4(0, 0, 0, 1);
	}
	
	// aka Lo
	vec3 finalColor = fragData.color * fragData.emissiveIntensity;
	
	// Point Light calculation
	for (int i = 0; i < pointLights.length(); ++i)
	{
		float shadow = pointLights[i].castShadows ? isInPointShadow(i) : 0;
		
		// calculate light parameters
		vec3  lightDir    = normalize(pointLights[i].pos - fragData.pos);
		vec3  halfwayDir  = normalize(fragData.viewDir + lightDir);
		float distance    = length(pointLights[i].pos - fragData.pos);
		float attenuation = 1.0 / (1 + distance * distance);
		vec3  radiance    = pointLights[i].color * pointLights[i].intensity * attenuation;
		
		finalColor += calculateLighting(radiance, lightDir, halfwayDir) * (1 - shadow);
	}
	
	// Directional Light calculation
	for (int i = 0; i < directionalLights.length(); ++i)
	{
		float shadow = directionalLights[i].castShadows ? isInDirectionalShadow(i) : 0;
		
		// calculate light parameters
		vec3 lightDir    = directionalLights[i].fragToLightDirection;
		vec3 halfwayDir  = normalize(fragData.viewDir + lightDir);
		vec3 radiance    = directionalLights[i].color * directionalLights[i].intensity;
		
		finalColor += calculateLighting(radiance, lightDir, halfwayDir) * (1 - shadow);
	}
	
	return vec4(finalColor, 1);
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

float isInDirectionalShadow(int lightIndex)
{
	float texelSize = 1.0 / textureSize(directionalLights[lightIndex].shadowMap, 0).x;
	float texelSize_WS = texelSize * directionalLights[lightIndex].mapSize;
	
	float samplingRadius = 3;
	
	vec3 fragPos = fragData.pos + calculateNormalBias(fragData.geometryNormal, directionalLights[lightIndex].fragToLightDirection, texelSize_WS, samplingRadius);
	
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

float isInPointShadow(int lightIndex)
{
	// get vector between fragment position and light position
	vec3 lightToFrag = fragData.pos - pointLights[lightIndex].pos;
	float fragDist = length(lightToFrag);
	
	float texelSize_WS = pointLights[lightIndex].maxTexelSizeAtUnitDistance * fragDist;
	
	float samplingRadius = 3;
	
	vec3 fragPos = fragData.pos + calculateNormalBias(fragData.geometryNormal, -lightToFrag / fragDist, texelSize_WS, samplingRadius);
	
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
	
	const int sampleCount = 128;
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

vec3 calculateLighting(vec3 radiance, vec3 lightDir, vec3 halfwayDir)
{
	vec3 brdf = calculateBRDF(lightDir, halfwayDir);
	
	return brdf * radiance * max(dot(fragData.normal, lightDir), 0.0);
}

// Cook-Torrance
vec3 calculateBRDF(vec3 lightDir, vec3 halfwayDir)
{
	// ---------- Specular part ----------
	
	float D = DistributionGGX(fragData.normal, halfwayDir, fragData.roughness);
	vec3  F = fresnelSchlick(max(dot(halfwayDir, fragData.viewDir), 0.0), fragData.F0);
	float G = GeometrySmith(fragData.normal, fragData.viewDir, lightDir, fragData.roughness);
	
	vec3  numerator   = D * F * G;
	float denominator = 4.0 * max(dot(fragData.normal, fragData.viewDir), 0.0) * max(dot(fragData.normal, lightDir), 0.0);
	vec3  specular    = numerator / max(denominator, 0.001);
	
	vec3 specularWeight = F;
	
	// ---------- Diffuse part ----------
	
	vec3 diffuse = fragData.color / PI;
	
	vec3 diffuseWeight = vec3(1.0) - specularWeight;
	diffuseWeight *= 1.0 - fragData.metalness; // Metals have no diffuse
	
	// ---------- Final result ----------
	
	// specularWeight already integrated to specular
	return (diffuseWeight * diffuse) + (specular);
}

vec3 getPosition()
{
	return texture(u_position, fragData.texCoords).rgb;
}

vec3 getColor()
{
	return texture(u_colorTexture, fragData.texCoords).rgb;
}

vec3 getNormal()
{
	return normalize(texture(u_normalTexture, fragData.texCoords).rgb * 2.0 - 1.0);
}

vec3 getGeometryNormal()
{
	return normalize(texture(u_geometryNormalTexture, fragData.texCoords).rgb * 2.0 - 1.0);
}

float getRoughness()
{
	return texture(u_materialTexture, fragData.texCoords).r;
}

float getMetallic()
{
	return texture(u_materialTexture, fragData.texCoords).g;
}

float getEmissive()
{
	return texture(u_materialTexture, fragData.texCoords).b;
}

float getDepth()
{
	return texture(u_depthTexture, fragData.texCoords).r;
}

int isLit()
{
	return int(texture(u_materialTexture, fragData.texCoords).a);
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
	
	return num / denom;
}

// Geometry Shadowing Function
float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;
	
	float num   = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	
	return num / denom;
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
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}