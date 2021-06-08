#version 460 core
#extension GL_ARB_bindless_texture : enable

in G2F
{
	vec3 fragPos;
	vec2 texCoords;
	vec3 T;
	vec3 N;
	vec3 flatNormal_WS;
} g2f;

uniform vec3 u_viewPos;
uniform int  u_objectIndex;
layout(bindless_sampler) uniform sampler2D u_colorMap;
layout(bindless_sampler) uniform sampler2D u_normalMap;
layout(bindless_sampler) uniform sampler2D u_roughnessMap;
layout(bindless_sampler) uniform sampler2D u_displacementMap;
layout(bindless_sampler) uniform sampler2D u_metallicMap;
layout(bindless_sampler) uniform sampler2D u_emissiveMap;

layout(location = 0) out vec3 o_normal;
layout(location = 1) out vec3 o_color;
layout(location = 2) out vec4 o_material;
layout(location = 3) out vec3 o_geometryNormal;
layout(location = 4) out int  o_objectIndex;
layout(location = 5) out vec3 o_position;

float getDepth(vec2 texCoords);
vec2 POM(vec2 texCoords, vec3 viewDir);

void main()
{
	vec3 T = normalize(g2f.T);
	vec3 N = normalize(g2f.N);
	vec3 B = normalize(cross(g2f.N, g2f.T));
	mat3 tangentToWorld = mat3(T, B, N);
	mat3 worldToTangent = transpose(tangentToWorld);
	
	vec3 viewDir = normalize(u_viewPos - g2f.fragPos);
	vec2 texCoords = POM(g2f.texCoords, normalize(worldToTangent * viewDir));
	
	o_color = texture(u_colorMap, texCoords).rgb;
	
	o_normal.xy = texture(u_normalMap, texCoords).rg * 2.0 - 1.0;
	o_normal.z = sqrt(1 - min(dot(o_normal.xy, o_normal.xy), 1));
	o_normal = tangentToWorld * o_normal;
	o_normal = (o_normal + 1) * 0.5;
	
	o_material.r = texture(u_roughnessMap, texCoords).r;
	o_material.g = texture(u_metallicMap, texCoords).r;
	o_material.b = texture(u_emissiveMap, texCoords).r;
	o_material.a = 1;
	
	o_objectIndex = u_objectIndex;
	
	o_geometryNormal = g2f.flatNormal_WS;
	o_geometryNormal = (o_geometryNormal + 1) * 0.5;
	
	o_position = g2f.fragPos;
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

// POM algorithm supporting height + depth (height range from -1 to 1)
/*
float getHeight(vec2 texCoords)
{
	return texture(displacementMap, texCoords).r * 2 - 1;
}

vec2 POM(vec2 texCoords, vec3 viewDir)
{
	const float maxHeight      = 0.1;
	const float minHeight      = -0.1;
	const int   linearSamples  = 12;
	const int   binarySamples  = 6;

	// Initial sampling pass
	vec2 currentTexCoords = texCoords + (viewDir.xy / viewDir.z) * maxHeight;

	float currentTexHeight = getHeight(currentTexCoords);
	float previousTexHeight;

	if (currentTexHeight == 0 || linearSamples == 0) return texCoords;

	if (viewDir.z <= 0) return texCoords;

	// Offsets applied at each steps
	vec2  texCoordsStepOffset = -(viewDir.xy / viewDir.z) / linearSamples * (minHeight - maxHeight);
	float heightStepOffset     = -2.0 / linearSamples;

	float currentHeight = 1;

	while (currentHeight > currentTexHeight)
	{
		currentTexCoords += texCoordsStepOffset;

		previousTexHeight = currentTexHeight;
		currentTexHeight = getHeight(currentTexCoords);

		currentHeight += heightStepOffset;
	}

	vec2 previousTexCoords = currentTexCoords - texCoordsStepOffset;
	float previousHeight = currentHeight - heightStepOffset;

	// Resampling pass

	for (int i = 0; i < binarySamples; i++)
	{
		texCoordsStepOffset *= 0.5;
		heightStepOffset *= 0.5;

		vec2  halfwayTexCoords = previousTexCoords + texCoordsStepOffset;
		float halfwayTexHeight  = getHeight(halfwayTexCoords);
		float halfwayHeight     = previousHeight + heightStepOffset;

		// If we are still above the surface
		if (halfwayHeight > halfwayTexHeight)
		{
			previousTexCoords = halfwayTexCoords;
			previousTexHeight  = halfwayTexHeight;
			previousHeight     = halfwayHeight;
		}
		else
		{
			currentTexCoords = halfwayTexCoords;
			currentTexHeight  = halfwayTexHeight;
			currentHeight     = halfwayHeight;
		}
	}

	// Interpolation
	float afterHeight  = currentTexHeight - currentHeight;
	float beforeHeight = previousTexHeight - currentHeight + heightStepOffset;

	float weight = afterHeight / (afterHeight - beforeHeight);
	texCoords = previousTexCoords * weight + currentTexCoords * (1.0 - weight);

	return texCoords;
}
*/