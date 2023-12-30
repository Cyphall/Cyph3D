#version 460 core

#extension GL_GOOGLE_include_directive : require

#include "../scene common data.glsl"

layout(push_constant) uniform constants
{
	mat4 u_viewProjection;
	ModelBuffer u_modelBuffer;
	vec3 u_viewPos;
	uint u_frameIndex;
};

layout(location = 0) flat out int o_drawID;
layout(location = 1) out V2F
{
	vec3 o_fragPos;
	vec2 o_texCoords;
	vec3 o_T;
	vec3 o_N;
};

void main()
{
	Model model = u_modelBuffer.models[gl_DrawID];
	uint index = model.indexBuffer.indices[gl_VertexIndex];
	FullVertex vertex = model.fullVertexBuffer.vertices[index];

	o_texCoords = vertex.uv;
	o_fragPos = (model.modelMatrix * vec4(vertex.position, 1.0)).xyz;

	o_T = normalize((model.normalMatrix * vec4(vertex.tangent, 0)).xyz);
	o_N = normalize((model.normalMatrix * vec4(vertex.normal, 0)).xyz);

	o_drawID = gl_DrawID;

	gl_Position = u_viewProjection * model.modelMatrix * vec4(vertex.position, 1.0);
}