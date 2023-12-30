#version 460 core

#extension GL_GOOGLE_include_directive : require

#include "../scene common data.glsl"

layout(push_constant) uniform constants
{
	mat4 u_viewProjection;
	ModelBuffer u_modelBuffer;
};

void main()
{
	Model model = u_modelBuffer.models[gl_DrawID];
	uint index = model.indexBuffer.indices[gl_VertexIndex];
	PositionVertex vertex = model.positionVertexBuffer.vertices[index];

	gl_Position = u_viewProjection * model.modelMatrix * vec4(vertex.position, 1.0);
}