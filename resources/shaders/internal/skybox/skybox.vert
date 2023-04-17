#version 460 core

layout (location = 0) in vec3 a_position;

layout(push_constant) uniform constants
{
	mat4 u_mvp;
};

layout(location = 0) out V2F
{
	vec3 o_texCoords;
};

void main()
{
	o_texCoords = a_position * vec3(1, 1, -1);
	vec4 pos = u_mvp * vec4(a_position, 1.0);
	gl_Position = pos.xyww;
} 