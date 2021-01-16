#version 460 core

layout (location = 0) in vec3 a_position;

out V2F
{
    vec3 texCoords;
} v2f;

uniform mat4 u_mvp;

void main()
{
	v2f.texCoords = a_position * vec3(-1, -1, 1);
	vec4 pos = u_mvp * vec4(a_position, 1.0);
	gl_Position = pos.xyww;
} 