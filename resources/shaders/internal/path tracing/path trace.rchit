#version 460 core
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require

struct HitPayload
{
	bool hit;
	vec3 position;
	vec3 normal;
	vec3 tangent;
	vec3 albedo;
	float roughness;
	float metalness;
	float emissive;
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

struct ObjectUniforms
{
	mat4 normalMatrix;
	mat4 model;
	VertexBuffer vertexBuffer;
	IndexBuffer indexBuffer;
	uint albedoIndex;
	uint normalIndex;
	uint roughnessIndex;
	uint metalnessIndex;
	uint displacementIndex;
	uint emissiveIndex;
	float emissiveScale;
};

layout(set = 0, binding = 0) uniform sampler2D u_textures[];

layout(std430, set = 1, binding = 3) readonly buffer UselessNameBecauseItIsNeverUsedAnywhere1
{
	ObjectUniforms u_objectUniforms[];
};

layout(location = 0) rayPayloadInEXT HitPayload hitPayload;
hitAttributeEXT vec2 attribs;

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
	ObjectUniforms uniforms = u_objectUniforms[nonuniformEXT(gl_InstanceID)];
	
	uvec3 indices = uniforms.indexBuffer.indices[nonuniformEXT(gl_PrimitiveID)];
	
	Vertex v1 = uniforms.vertexBuffer.vertices[nonuniformEXT(indices.x)];
	Vertex v2 = uniforms.vertexBuffer.vertices[nonuniformEXT(indices.y)];
	Vertex v3 = uniforms.vertexBuffer.vertices[nonuniformEXT(indices.z)];
	
	vec3 position1 = gl_ObjectToWorldEXT * vec4(v1.position, 1);
	vec3 position2 = gl_ObjectToWorldEXT * vec4(v2.position, 1);
	vec3 position3 = gl_ObjectToWorldEXT * vec4(v3.position, 1);
	
	mat3 normalMatrix = mat3(uniforms.normalMatrix);
	
	vec3 normal1 = normalize(normalMatrix * v1.normal);
	vec3 normal2 = normalize(normalMatrix * v2.normal);
	vec3 normal3 = normalize(normalMatrix * v3.normal);
	
	vec3 tangent1 = normalize(normalMatrix * v1.tangent);
	vec3 tangent2 = normalize(normalMatrix * v2.tangent);
	vec3 tangent3 = normalize(normalMatrix * v3.tangent);
	
	vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
	
	vec2 uv = interpolateBarycentrics(v1.uv, v2.uv, v3.uv, barycentrics);
	
	hitPayload.hit = true;
	hitPayload.position = interpolateBarycentrics(position1, position2, position3, barycentrics);
	hitPayload.normal = normalize(interpolateBarycentrics(normal1, normal2, normal3, barycentrics));
	hitPayload.tangent = normalize(interpolateBarycentrics(tangent1, tangent2, tangent3, barycentrics));
	hitPayload.albedo = texture(u_textures[nonuniformEXT(uniforms.albedoIndex)], uv).rgb;
	hitPayload.roughness = texture(u_textures[nonuniformEXT(uniforms.roughnessIndex)], uv).r;
	hitPayload.metalness = texture(u_textures[nonuniformEXT(uniforms.metalnessIndex)], uv).r;
	hitPayload.emissive = texture(u_textures[nonuniformEXT(uniforms.emissiveIndex)], uv).r * uniforms.emissiveScale;
}
