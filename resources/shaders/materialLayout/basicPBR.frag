in FRAG {
	vec2 TexCoords;
	mat3 TangentToWorld;
	mat3 WorldToTangent;
	vec3 FragPos;
} frag;

uniform vec3 viewPos;

layout(bindless_sampler) uniform sampler2D colorMap;
layout(bindless_sampler) uniform sampler2D normalMap;
layout(bindless_sampler) uniform sampler2D roughnessMap;
layout(bindless_sampler) uniform sampler2D displacementMap;
layout(bindless_sampler) uniform sampler2D metallicMap;
layout(bindless_sampler) uniform sampler2D emissiveMap;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 color;
layout(location = 2) out vec4 material;
layout(location = 3) out vec3 geometryNormal;

float getDepth(vec2 texCoords);
vec2 POM(vec2 texCoords, vec3 viewDir);

void main()
{
	vec2 texCoords = POM(frag.TexCoords, normalize(frag.WorldToTangent * (viewPos - frag.FragPos)));

	color = texture(colorMap, texCoords).rgb;

	normal = normalize(texture(normalMap, texCoords).rgb * 2.0 - 1.0);
	normal = frag.TangentToWorld * normal;
	normal = (normal + 1) * 0.5;

	material.r = texture(roughnessMap, texCoords).r;
	material.g = texture(metallicMap, texCoords).r;
	material.b = texture(emissiveMap, texCoords).r;
	material.a = 1;
	
	geometryNormal = frag.TangentToWorld * vec3(0, 0, 1);
	geometryNormal = (geometryNormal + 1) * 0.5;
}

float getDepth(vec2 texCoords)
{
	return 1 - texture(displacementMap, texCoords).r;
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