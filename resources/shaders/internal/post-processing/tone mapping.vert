#version 460 core

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_uv;

out V2F
{
	vec2 texCoords;
} v2f;

void main()
{
	gl_Position = vec4(a_position, 0, 1);
	
	v2f.texCoords = a_uv;
}