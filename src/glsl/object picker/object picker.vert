#version 460 core

#extension GL_EXT_scalar_block_layout : require

layout(location = 0) in vec3 a_position;

layout(push_constant, scalar) uniform constants
{
	mat4 u_mvp;
	uint u_objectIndex;
};

void main()
{
	gl_Position = u_mvp * vec4(a_position, 1.0);
}