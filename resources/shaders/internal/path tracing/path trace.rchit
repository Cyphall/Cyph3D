#version 460 core
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require

const float PI = 3.14159265359;
const float TWO_PI = PI * 2.0;

struct HitPayload
{
	uint randomOffset;
	vec3 light;
	vec3 contribution;
	bool hit;
	vec3 rayPosition;
	vec3 rayDirection;
};

struct Vertex
{
	vec3 position;
	vec2 uv;
	vec3 normal;
	vec3 tangent;
};

layout(buffer_reference, std430, scalar) readonly buffer VertexBuffer
{
	Vertex vertices[];
};

layout(buffer_reference, std430, scalar) readonly buffer IndexBuffer
{
	uvec3 indices[];
};

layout(set = 0, binding = 0) uniform sampler2D u_textures[];

layout(shaderRecordEXT) buffer uniforms
{
	mat4 u_normalMatrix;
	VertexBuffer u_vertexBuffer;
	IndexBuffer u_indexBuffer;
	uint u_albedoIndex;
	uint u_normalIndex;
	uint u_roughnessIndex;
	uint u_metalnessIndex;
	uint u_displacementIndex;
	uint u_emissiveIndex;
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

void main()
{
	uvec3 indices = u_indexBuffer.indices[gl_PrimitiveID];
	
	Vertex v1 = u_vertexBuffer.vertices[indices.x];
	Vertex v2 = u_vertexBuffer.vertices[indices.y];
	Vertex v3 = u_vertexBuffer.vertices[indices.z];
	
	vec3 position1 = gl_ObjectToWorldEXT * vec4(v1.position, 1);
	vec3 position2 = gl_ObjectToWorldEXT * vec4(v2.position, 1);
	vec3 position3 = gl_ObjectToWorldEXT * vec4(v3.position, 1);
	
	mat3 normalMatrix = mat3(u_normalMatrix);
	
	vec3 normal1 = normalize(normalMatrix * v1.normal);
	vec3 normal2 = normalize(normalMatrix * v2.normal);
	vec3 normal3 = normalize(normalMatrix * v3.normal);
	
	vec3 tangent1 = normalize(normalMatrix * v1.tangent);
	vec3 tangent2 = normalize(normalMatrix * v2.tangent);
	vec3 tangent3 = normalize(normalMatrix * v3.tangent);
	
	vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
	
	vec3 position = interpolateBarycentrics(position1, position2, position3, barycentrics);
	vec3 normal = normalize(interpolateBarycentrics(normal1, normal2, normal3, barycentrics));
	vec3 tangent = normalize(interpolateBarycentrics(tangent1, tangent2, tangent3, barycentrics));
	
	vec2 uv = interpolateBarycentrics(v1.uv, v2.uv, v3.uv, barycentrics);
	
	vec3 albedo = texture(u_textures[nonuniformEXT(u_albedoIndex)], uv).rgb;
	float roughness = texture(u_textures[nonuniformEXT(u_roughnessIndex)], uv).r;
	float metalness = texture(u_textures[nonuniformEXT(u_metalnessIndex)], uv).r;
	float emissive = texture(u_textures[nonuniformEXT(u_emissiveIndex)], uv).r * u_emissiveScale;
	
	
	hitPayload.light += hitPayload.contribution * albedo * emissive;
	
	
	hitPayload.hit = true;
	
	vec3 bitangent = cross(normal, tangent);
	mat3 tangentToWorld = mat3(tangent, bitangent, normal);
	mat3 worldToTangent = transpose(tangentToWorld);
	
	hitPayload.rayPosition = position + normal * 0.001;
	
	vec3 localRayDir = worldToTangent * hitPayload.rayDirection;
	float alpha = roughness * roughness;
	vec2 alpha2D = vec2(alpha, alpha);
	vec4 rand = getRandom();
	vec3 microfacetNormal = normalize(tangentToWorld * sampleGGXVNDF(-localRayDir, alpha2D, rand.xy));
	
	vec3 F0 = mix(vec3(0.04), albedo, metalness);
	vec3 specularWeight = fresnelSchlick(max(dot(microfacetNormal, -hitPayload.rayDirection), 0.0), F0);
	vec3 diffuseWeight = 1.0 - specularWeight;
	
	if (rand.z > 0.5)
	{
		// next bounce is specular
		
		hitPayload.rayDirection = reflect(hitPayload.rayDirection, microfacetNormal);
		
		hitPayload.contribution *= specularWeight * 2;
	}
	else
	{
		// next bounce is diffuse
		
		hitPayload.rayDirection = tangentToWorld * calcRandomHemisphereDirectionCosWeighted(getRandom().xy);
		
		hitPayload.contribution *= albedo * diffuseWeight * (1.0 - metalness) * 2;
	}
}
