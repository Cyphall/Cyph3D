#version 460 core

#extension GL_GOOGLE_include_directive : require

#include "../scene common data.glsl"

layout(push_constant) uniform constants
{
	mat4 u_viewProjection;
	ModelBuffer u_modelBuffer;
	vec3 u_lightPos;
};

layout(location = 0) out V2F
{
	vec3 o_fragPos;
};

void main()
{
	Model model = u_modelBuffer.models[gl_DrawID];
	uint index = model.indexBuffer.indices[gl_VertexIndex];
	PositionVertex vertex = model.positionVertexBuffer.vertices[index];

	vec4 fragPos = model.modelMatrix * vec4(vertex.position, 1.0);
	gl_Position = u_viewProjection * fragPos;
	o_fragPos = fragPos.xyz;
}