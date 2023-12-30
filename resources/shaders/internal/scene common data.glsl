#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

struct PositionVertex
{
	vec3 position;
};

layout(buffer_reference, scalar) readonly buffer PositionVertexBuffer
{
	PositionVertex vertices[];
};

struct FullVertex
{
	vec3 position;
	vec2 uv;
	vec3 normal;
	vec3 tangent;
};

layout(buffer_reference, scalar) readonly buffer FullVertexBuffer
{
	FullVertex vertices[];
};

layout(buffer_reference, scalar) readonly buffer IndexBuffer
{
	uint indices[];
};

struct Model
{
	mat4 modelMatrix;
	mat4 normalMatrix;
	PositionVertexBuffer positionVertexBuffer;
	FullVertexBuffer fullVertexBuffer;
	IndexBuffer indexBuffer;
	int albedoIndex;
	int normalIndex;
	int roughnessIndex;
	int metalnessIndex;
	int displacementIndex;
	int emissiveIndex;
	vec3 albedoValue;
	float roughnessValue;
	float metalnessValue;
	float displacementScale;
	float emissiveScale;
};

layout(buffer_reference, std430) readonly buffer ModelBuffer
{
	Model models[];
};