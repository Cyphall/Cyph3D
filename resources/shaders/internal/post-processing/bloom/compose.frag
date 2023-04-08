#version 460 core

layout(location = 0) in V2F
{
	vec2 i_texCoords;
};

layout(set = 0, binding = 0) uniform sampler2D u_srcATexture;
layout(set = 0, binding = 1) uniform sampler2D u_srcBTexture;

layout(push_constant) uniform constants
{
	float u_factor;
};

layout(location = 0) out vec4 o_color;

void main()
{
	vec3 a = textureLod(u_srcATexture, i_texCoords, 0).xyz;
	vec3 b = textureLod(u_srcBTexture, i_texCoords, 0).xyz;
	
	o_color = vec4(mix(a, b, u_factor), 1);
}
