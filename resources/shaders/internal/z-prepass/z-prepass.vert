#version 460 core

layout(location = 0) in vec3 a_position;

layout(push_constant) uniform constants
{
	mat4 u_mvp;
};

void main()
{
	gl_Position = u_mvp * vec4(a_position, 1.0);
}