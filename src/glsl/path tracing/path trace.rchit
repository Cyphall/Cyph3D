#version 460 core

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_ray_tracing : require
#extension GL_ARB_gpu_shader_fp64 : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

const float PI = 3.14159265359;
const float TWO_PI = PI * 2.0;

struct HitPayload
{
	uint randomOffset;
    dvec3 light;
	vec3 throughput;
	bool hit;
	vec3 rayPosition;
	vec3 rayDirection;
};

struct PositionVertex
{
	vec3 position;
};

struct MaterialVertex
{
	vec2 uv;
	vec3 normal;
	vec4 tangent;
};

layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer PositionVertexBuffer
{
    PositionVertex vertices[];
};

layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer MaterialVertexBuffer
{
	MaterialVertex vertices[];
};

layout(buffer_reference, scalar, buffer_reference_align = 4) readonly buffer IndexBuffer
{
	uvec3 indices[];
};

layout(set = 0, binding = 0) uniform sampler2D u_textures[];

layout(shaderRecordEXT) buffer uniforms
{
	mat4 u_normalMatrix;
    PositionVertexBuffer u_positionVertexBuffer;
    MaterialVertexBuffer u_materialVertexBuffer;
	IndexBuffer u_indexBuffer;
	int u_albedoIndex;
	int u_normalIndex;
	int u_roughnessIndex;
	int u_metalnessIndex;
	int u_displacementIndex;
	int u_emissiveIndex;
	vec3 u_albedoValue;
	float u_roughnessValue;
	float u_metalnessValue;
	float u_emissiveScale;
};

layout(push_constant) uniform constants
{
	uint u_batchIndex;
	uint u_sampleCount;
	bool u_resetAccumulation;
};

layout(location = 0) rayPayloadInEXT HitPayload hitPayload;
hitAttributeEXT vec2 attribs;

vec3 calcRandomHemisphereDirectionCosWeighted(vec2 rand)
{
	float a = sqrt(rand.x);
	float b = TWO_PI * rand.y;

	return vec3(
	a * cos(b),
	a * sin(b),
	sqrt(1.0 - rand.x)
	);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 sampleGGXVNDF(vec3 Ve, vec2 alpha2D, vec2 rand)
{
	// Section 3.2: transforming the view direction to the hemisphere configuration
	vec3 Vh = normalize(vec3(alpha2D.x * Ve.x, alpha2D.y * Ve.y, Ve.z));

	// Section 4.1: orthonormal basis (with special case if cross product is zero)
	float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
	vec3 T1 = lensq > 0.0 ? vec3(-Vh.y, Vh.x, 0.0) * inversesqrt(lensq) : vec3(1.0, 0.0, 0.0);
	vec3 T2 = cross(Vh, T1);

	// Section 4.2: parameterization of the projected area
	float r = sqrt(rand.x);
	float phi = TWO_PI * rand.y;
	float t1 = r * cos(phi);
	float t2 = r * sin(phi);
	float s = 0.5 * (1.0 + Vh.z);
	t2 = mix(sqrt(1.0 - t1 * t1), t2, s);

	// Section 4.3: reprojection onto hemisphere
	vec3 Nh = t1 * T1 + t2 * T2 + sqrt(max(0.0, 1.0 - t1 * t1 - t2 * t2)) * Vh;

	// Section 3.4: transforming the normal back to the ellipsoid configuration
	return normalize(vec3(alpha2D.x * Nh.x, alpha2D.y * Nh.y, max(0.0, Nh.z)));
}

vec4 getRandom()
{
	uvec4 v = uvec4(uvec2(gl_LaunchIDEXT.xy), u_batchIndex, hitPayload.randomOffset++);

	v = v * 1664525u + 1013904223u;
	v.x += v.y*v.w; v.y += v.z*v.x; v.z += v.x*v.y; v.w += v.y*v.z;
	v ^= v >> 16u;
	v.x += v.y*v.w; v.y += v.z*v.x; v.z += v.x*v.y; v.w += v.y*v.z;

	return clamp(v * (1.0 / float(0xffffffffu)), 0.0, 1.0);
}

vec2 interpolateBarycentrics(vec2 a, vec2 b, vec2 c, vec3 barycentrics)
{
	return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

vec3 interpolateBarycentrics(vec3 a, vec3 b, vec3 c, vec3 barycentrics)
{
	return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

void dielectricBRDF(float NdotL, out vec3 diffuseWeight, out vec3 specularWeight)
{
	vec3 F0 = vec3(0.04);
	specularWeight = fresnelSchlick(max(NdotL, 0.0), F0);
	diffuseWeight = 1.0 - specularWeight;
}

void conductorBRDF(vec3 albedo, float NdotL, out vec3 diffuseWeight, out vec3 specularWeight)
{
	vec3 F0 = albedo;
	specularWeight = fresnelSchlick(max(NdotL, 0.0), F0);
	diffuseWeight = vec3(0);
}

vec3 offsetRay(const vec3 p, const vec3 n)
{
	const float origin = 1.0 / 32.0;
	const float float_scale = 1.0 / 65536.0;
	const float int_scale = 256.0;

	ivec3 of_i = ivec3(int_scale * n.x, int_scale * n.y, int_scale * n.z);

	vec3 p_i = vec3(
		intBitsToFloat(floatBitsToInt(p.x) + ((p.x < 0) ? -of_i.x : of_i.x)),
		intBitsToFloat(floatBitsToInt(p.y) + ((p.y < 0) ? -of_i.y : of_i.y)),
		intBitsToFloat(floatBitsToInt(p.z) + ((p.z < 0) ? -of_i.z : of_i.z))
	);

	return vec3(
		abs(p.x) < origin ? p.x+float_scale*n.x : p_i.x,
		abs(p.y) < origin ? p.y+float_scale*n.y : p_i.y,
		abs(p.z) < origin ? p.z+float_scale*n.z : p_i.z
	);
}

void main()
{
	// flip normals if ray hit a back face
	float normalScale = gl_HitKindEXT == gl_HitKindBackFacingTriangleEXT ? -1.0 : 1.0;

	uvec3 indices = u_indexBuffer.indices[gl_PrimitiveID];

    PositionVertex pv1 = u_positionVertexBuffer.vertices[indices.x];
    PositionVertex pv2 = u_positionVertexBuffer.vertices[indices.y];
    PositionVertex pv3 = u_positionVertexBuffer.vertices[indices.z];

	MaterialVertex mv1 = u_materialVertexBuffer.vertices[indices.x];
	MaterialVertex mv2 = u_materialVertexBuffer.vertices[indices.y];
	MaterialVertex mv3 = u_materialVertexBuffer.vertices[indices.z];

	vec3 position1 = gl_ObjectToWorldEXT * vec4(pv1.position, 1.0);
	vec3 position2 = gl_ObjectToWorldEXT * vec4(pv2.position, 1.0);
	vec3 position3 = gl_ObjectToWorldEXT * vec4(pv3.position, 1.0);

	mat3 normalMatrix = mat3(u_normalMatrix);

	vec3 normal1 = normalize(normalMatrix * mv1.normal);
	vec3 normal2 = normalize(normalMatrix * mv2.normal);
	vec3 normal3 = normalize(normalMatrix * mv3.normal);

	vec3 tangent1 = normalize(normalMatrix * mv1.tangent.xyz);
	vec3 tangent2 = normalize(normalMatrix * mv2.tangent.xyz);
	vec3 tangent3 = normalize(normalMatrix * mv3.tangent.xyz);

	vec3 bitangent1 = cross(normal1, tangent1) * mv1.tangent.w;
	vec3 bitangent2 = cross(normal2, tangent2) * mv2.tangent.w;
	vec3 bitangent3 = cross(normal3, tangent3) * mv3.tangent.w;

	vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

	vec3 position = interpolateBarycentrics(position1, position2, position3, barycentrics);
	vec3 normal = normalize(interpolateBarycentrics(normal1, normal2, normal3, barycentrics)) * normalScale;
	vec3 tangent = normalize(interpolateBarycentrics(tangent1, tangent2, tangent3, barycentrics)) * normalScale;
	vec3 bitangent = normalize(interpolateBarycentrics(bitangent1, bitangent2, bitangent3, barycentrics)) * normalScale;
	vec3 geometryNormal = normalize(cross(position3 - position2, position1 - position2));

	geometryNormal = dot(geometryNormal, hitPayload.rayDirection) < 0.0 ? geometryNormal : -geometryNormal;

	vec2 uv = interpolateBarycentrics(mv1.uv, mv2.uv, mv3.uv, barycentrics);

	vec3 albedo = u_albedoIndex >= 0 ? texture(u_textures[nonuniformEXT(u_albedoIndex)], uv).rgb : u_albedoValue;
	float roughness = u_roughnessIndex >= 0 ? texture(u_textures[nonuniformEXT(u_roughnessIndex)], uv).r : u_roughnessValue;
	float metalness = u_metalnessIndex >= 0 ? texture(u_textures[nonuniformEXT(u_metalnessIndex)], uv).r : u_metalnessValue;
	float emissive = (u_emissiveIndex >= 0 ? texture(u_textures[nonuniformEXT(u_emissiveIndex)], uv).r : 1.0) * u_emissiveScale;

	vec3 textureNormal = vec3(0.0);
	textureNormal.xy = u_normalIndex >= 0 ? texture(u_textures[nonuniformEXT(u_normalIndex)], uv).rg * 2.0 - 1.0 : vec2(0.0, 0.0);
	textureNormal.z = sqrt(1.0 - min(dot(textureNormal.xy, textureNormal.xy), 1.0));

	normal = normalize(mat3(tangent, bitangent, normal) * textureNormal);
	tangent = normalize(tangent - normal * dot(tangent, normal));
	bitangent = cross(tangent, normal);

	mat3 tangentToWorld = mat3(tangent, bitangent, normal);
	mat3 worldToTangent = transpose(tangentToWorld);


	hitPayload.light += dvec3(hitPayload.throughput * albedo * emissive);


	hitPayload.hit = true;

	hitPayload.rayPosition = offsetRay(position, geometryNormal);

	vec3 localRayDir = worldToTangent * hitPayload.rayDirection;
	float alpha = roughness * roughness;
	vec2 alpha2D = vec2(alpha, alpha);
	vec4 rand = getRandom();
	vec3 microfacetNormal = normalize(tangentToWorld * sampleGGXVNDF(-localRayDir, alpha2D, rand.xy));

	float NdotL = max(dot(microfacetNormal, -hitPayload.rayDirection), 0.0);

	vec3 dielectricDiffuseWeight;
	vec3 dielectricSpecularWeight;
	if (metalness < 1.0) dielectricBRDF(NdotL, dielectricDiffuseWeight, dielectricSpecularWeight);

	vec3 conductorDiffuseWeight;
	vec3 conductorSpecularWeight;
	if (metalness > 0.0) conductorBRDF(albedo, NdotL, conductorDiffuseWeight, conductorSpecularWeight);

	if (rand.z > 0.5)
	{
		// next bounce is specular

		hitPayload.rayDirection = reflect(hitPayload.rayDirection, microfacetNormal);

		vec3 dielectric = dielectricSpecularWeight;
		vec3 conductor  = conductorSpecularWeight;
		hitPayload.throughput *= mix(dielectric, conductor, metalness);

		hitPayload.throughput *= 2.0;
	}
	else
	{
		// next bounce is diffuse

		hitPayload.rayDirection = tangentToWorld * calcRandomHemisphereDirectionCosWeighted(getRandom().xy);

		vec3 dielectric = albedo * dielectricDiffuseWeight;
		vec3 conductor  = albedo * conductorDiffuseWeight;
		hitPayload.throughput *= mix(dielectric, conductor, metalness);

		hitPayload.throughput *= 2.0;
	}
}
