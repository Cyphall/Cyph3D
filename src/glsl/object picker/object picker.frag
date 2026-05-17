#version 460 core

#extension GL_EXT_scalar_block_layout : require

layout(location = 0) out int o_color;

layout(push_constant, scalar) uniform constants
{
	mat4 u_mvp;
	int u_objectIndex;
};

void main()
{
	o_color = u_objectIndex;
}