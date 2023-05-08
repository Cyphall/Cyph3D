#version 460 core
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require

struct HitPayload
{
	vec3 value;
	int objectIndex;
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
};

layout(set = 0, binding = 0) uniform sampler2D u_textures[];

layout(std430, set = 1, binding = 4) buffer UselessNameBecauseItIsNeverUsedAnywhere1
{
	ObjectUniforms u_objectUniforms[];
};

layout(location = 0) rayPayloadInEXT HitPayload hitPayload;
hitAttributeEXT vec2 attribs;

void main()
{
	uvec3 indices = u_objectUniforms[gl_InstanceID].indexBuffer.indices[gl_PrimitiveID];
	
	Vertex v1 = u_objectUniforms[gl_InstanceID].vertexBuffer.vertices[indices.x];
	Vertex v2 = u_objectUniforms[gl_InstanceID].vertexBuffer.vertices[indices.y];
	Vertex v3 = u_objectUniforms[gl_InstanceID].vertexBuffer.vertices[indices.z];
	
	vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
	
	vec2 uv = v1.uv * barycentrics.x + v2.uv * barycentrics.y + v3.uv * barycentrics.z;
	
	vec3 albedo = texture(u_textures[u_objectUniforms[gl_InstanceID].albedoIndex], uv).rgb;
	
	hitPayload.value = albedo;
	hitPayload.objectIndex = gl_InstanceCustomIndexEXT;
}
