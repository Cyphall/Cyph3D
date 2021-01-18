#version 460 core
#extension GL_ARB_bindless_texture : enable

/* ------ consts ------ */
const float PI = 3.14159265359;
const float TWO_PI = PI*2;

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
	float padding;
};

layout(bindless_sampler) struct DirectionalLight
{
	vec3  fragToLightDirection;
	float intensity;
	vec3  color;
	bool  castShadows;
	mat4  lightViewProjection;
	sampler2D shadowMap;
	vec2  padding;
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

uniform bool u_debug;
uniform mat4 u_viewProjectionInv;
uniform vec3 u_viewPos;

/* ------ outputs ------ */
out vec4 o_color;

/* ------ function declarations ------ */
vec4 debugView();
vec4 lighting();

vec2 VogelDiskSample(int sampleIndex, int samplesCount, float phi);
float InterleavedGradientNoise(vec2 position_screen);
float AvgBlockersDepthToPenumbra(float z_shadowMapView, float avgBlockersDepth);
float Penumbra(float phi, vec2 fragUV_SMV, float fragDepth_SMV, float texelSize, int lightIndex);

float isInDirectionalShadow(int lightIndex);
float isInPointShadow(int lightIndex);
vec3 calculateLighting(vec3 radiance, vec3 L, vec3 H);

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
	
	if (u_debug)
	{
		o_color = debugView();
	}
	else if (isLit() == 0)
	{
		o_color = vec4(getColor(), 1);
	}
	else
	{
		o_color = lighting();
	}
}

vec4 debugView()
{
	if (fragData.texCoords.x <= 1.0/3.0 && fragData.texCoords.y >= 2.0/3.0)
	{
		fragData.texCoords.x = (fragData.texCoords.x - 0.0/3.0) * 3;
		fragData.texCoords.y = (fragData.texCoords.y - 2.0/3.0) * 3;
		fragData.depth = getDepth();
		return fragData.depth < 1 ? vec4(getPosition(), 1) : vec4(0);
	}
	else if (fragData.texCoords.x <= 2.0/3.0 && fragData.texCoords.y >= 2.0/3.0)
	{
		fragData.texCoords.x = (fragData.texCoords.x - 1.0/3.0) * 3;
		fragData.texCoords.y = (fragData.texCoords.y - 2.0/3.0) * 3;
		fragData.depth = getDepth();
		return fragData.depth < 1 ? texture(u_normalTexture, fragData.texCoords) : vec4(0);
	}
	else if (fragData.texCoords.x <= 3.0/3.0 && fragData.texCoords.y >= 2.0/3.0)
	{
		fragData.texCoords.x = (fragData.texCoords.x - 2.0/3.0) * 3;
		fragData.texCoords.y = (fragData.texCoords.y - 2.0/3.0) * 3;
		fragData.depth = getDepth();
		return fragData.depth < 1 ? texture(u_geometryNormalTexture, fragData.texCoords) : vec4(0);
	}
	else if (fragData.texCoords.x <= 1.0/3.0 && fragData.texCoords.y >= 1.0/3.0)
	{
		fragData.texCoords.x = (fragData.texCoords.x - 0.0/3.0) * 3;
		fragData.texCoords.y = (fragData.texCoords.y - 1.0/3.0) * 3;
		fragData.depth = getDepth();
		return fragData.depth < 1 ? texture(u_materialTexture, fragData.texCoords) : vec4(0);
	}
	else if (fragData.texCoords.x <= 2.0/3.0 && fragData.texCoords.y >= 1.0/3.0)
	{
		fragData.texCoords.x = (fragData.texCoords.x - 1.0/3.0) * 3;
		fragData.texCoords.y = (fragData.texCoords.y - 1.0/3.0) * 3;
		fragData.depth = getDepth();
		return fragData.depth < 1 ? texture(u_colorTexture, fragData.texCoords) : vec4(0);
	}
	else if (fragData.texCoords.x <= 3.0/3.0 && fragData.texCoords.y >= 1.0/3.0)
	{
		fragData.texCoords.x = (fragData.texCoords.x - 2.0/3.0) * 3;
		fragData.texCoords.y = (fragData.texCoords.y - 1.0/3.0) * 3;
		fragData.depth = getDepth();
		return vec4(1 - fragData.depth, 1 - fragData.depth, 1 - fragData.depth, 1);
	}
	
	return vec4(0);
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
	for(int i = 0; i < pointLights.length(); ++i)
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
	for(int i = 0; i < directionalLights.length(); ++i)
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
	float GoldenAngle = 2.4f;

	float r = sqrt(sampleIndex + 0.5f) / sqrt(samplesCount);
	float theta = sampleIndex * GoldenAngle + phi;

	float sine = sin(theta);
	float cosine = cos(theta);

	return vec2(r * cosine, r * sine);
}

float InterleavedGradientNoise(vec2 position_screen)
{
	vec3 magic = vec3(0.06711056f, 0.00583715f, 52.9829189f);
	return fract(magic.z * fract(dot(position_screen, magic.xy)));
}

float AvgBlockersDepthToPenumbra(float fragDepth_SMV, float blockersDepth_SMV)
{
	return 200 * (fragDepth_SMV - blockersDepth_SMV) / blockersDepth_SMV;
}

float Penumbra(float phi, vec2 fragUV_SMV, float fragDepth_SMV, float texelSize, int lightIndex)
{
	float avgBlockersDepth = 0.0f;
	int blockersCount = 0;

	const float samplingRadiusNormalized = 30 * texelSize;
	const int sampleCount = 16;

	for(int i = 0; i < sampleCount; i ++)
	{
		vec2 uvOffset = VogelDiskSample(i, sampleCount, phi) * samplingRadiusNormalized;
		float sampleDepth = texture(directionalLights[lightIndex].shadowMap, fragUV_SMV + uvOffset).r;

		if(sampleDepth < fragDepth_SMV)
		{
			avgBlockersDepth += sampleDepth;
			blockersCount++;
		}
	}

	if(blockersCount > 0)
	{
		avgBlockersDepth /= float(blockersCount);
		return AvgBlockersDepthToPenumbra(fragDepth_SMV, avgBlockersDepth);
	}
	else
	{
		return 0.0f;
	}
}

// Based on the code at https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping by Joey de Vries (https://twitter.com/JoeyDeVriez)
float isInDirectionalShadow(int lightIndex)
{
	vec4 shadowMapSpacePos = directionalLights[lightIndex].lightViewProjection * vec4(fragData.pos, 1);
	vec3 projCoords = shadowMapSpacePos.xyz / shadowMapSpacePos.w;
	projCoords = projCoords * 0.5 + 0.5;
	
	if (projCoords.z > 1) return 0.0;

	vec2  fragUV_SMV    = projCoords.xy;
	float fragDepth_SMV = projCoords.z;
	
	float texelSize = 1.0 / textureSize(directionalLights[lightIndex].shadowMap, 0).x;

	float phi = InterleavedGradientNoise(fragData.texCoords * vec2(1600, 900)) * TWO_PI;
	float samplingRadiusNormalized = Penumbra(phi, fragUV_SMV, fragDepth_SMV, texelSize, lightIndex) * texelSize;

	const int sampleCount = 16;
	float shadow = 0.0;
	for (int i = 0; i < sampleCount; i++)
	{
		vec2 uvOffset = VogelDiskSample(i, sampleCount, phi) * samplingRadiusNormalized;
		float sampleDepth = texture(directionalLights[lightIndex].shadowMap, fragUV_SMV + uvOffset).r;

		if (fragDepth_SMV > sampleDepth)
		{
			shadow++;
		}
	}
	shadow /= sampleCount;
	
	return shadow;
}

// Based on the code at https://learnopengl.com/Advanced-Lighting/Shadows/Point-Shadows by Joey de Vries (https://twitter.com/JoeyDeVriez)
float isInPointShadow(int lightIndex)
{
	// get vector between fragment position and light position
	vec3 fragToLight = fragData.pos - pointLights[lightIndex].pos;
	// use the light to fragment vector to sample from the depth map    
	float closestDepth = texture(pointLights[lightIndex].shadowMap, fragToLight).r;
	// it is currently in linear range between [0,1]. Re-transform back to original value
	closestDepth *= pointLights[lightIndex].far;
	// now get current linear depth as the length between the fragment and light position
	float currentDepth = length(fragToLight);
	// now test for shadows
	float shadow = currentDepth > closestDepth ? 1.0 : 0.0;

	return shadow;
}

vec3 calculateLighting(vec3 radiance, vec3 L, vec3 H)
{
	// cook-torrance brdf
	float NDF = DistributionGGX(fragData.normal, H, fragData.roughness);
	float G   = GeometrySmith(fragData.normal, fragData.viewDir, L, fragData.roughness);
	vec3 F    = fresnelSchlick(max(dot(H, fragData.viewDir), 0.0), fragData.F0);

	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - fragData.metalness;

	vec3  numerator    = NDF * G * F;
	float denominator = 4.0 * max(dot(fragData.normal, fragData.viewDir), 0.0) * max(dot(fragData.normal, L), 0.0);
	vec3  specular     = numerator / max(denominator, 0.001);

	// add to outgoing radiance Lo
	float NdotL = max(dot(fragData.normal, L), 0.0);
	return (kD * fragData.color / PI + specular) * radiance * NdotL;
}

vec3 getPosition()
{
	vec4 clipSpacePosition = vec4(fragData.texCoords, fragData.depth, 1) * 2.0 - 1.0;
	vec4 worldSpacePosition = u_viewProjectionInv * clipSpacePosition;
	return worldSpacePosition.xyz / worldSpacePosition.w;
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