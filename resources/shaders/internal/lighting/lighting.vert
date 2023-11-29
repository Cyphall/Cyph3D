#version 460 core

struct ObjectUniforms
{
	mat4 normalMatrix;
	mat4 model;
	mat4 mvp;
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

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec3 a_normal;
layout(location = 3) in vec3 a_tangent;

layout(std430, set = 3, binding = 0) readonly buffer UselessNameBecauseItIsNeverUsedAnywhere4
{
	ObjectUniforms u_objectUniforms;
};

layout(location = 0) out V2F
{
	vec3 o_fragPos;
	vec2 o_texCoords;
	vec3 o_T;
	vec3 o_N;
};

void main()
{
	o_texCoords = a_uv;
	o_fragPos = (u_objectUniforms.model * vec4(a_position, 1.0)).xyz;

	o_T = normalize(mat3(u_objectUniforms.normalMatrix) * a_tangent);
	o_N = normalize(mat3(u_objectUniforms.normalMatrix) * a_normal);
	
	gl_Position = u_objectUniforms.mvp * vec4(a_position, 1.0);
}