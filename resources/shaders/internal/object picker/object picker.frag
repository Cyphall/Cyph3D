#version 460 core

layout(location = 0) out int o_color;

layout(push_constant) uniform constants
{
	mat4 u_mvp;
	int u_objectIndex;
};

void main()
{
	o_color = u_objectIndex;
}