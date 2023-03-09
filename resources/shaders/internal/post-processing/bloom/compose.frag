#version 460 core
#extension GL_ARB_bindless_texture : enable

layout(bindless_sampler) uniform sampler2D u_srcATexture;
layout(bindless_sampler) uniform sampler2D u_srcBTexture;
uniform float u_factor;

in V2F
{
	vec2 texCoords;
} v2f;

out vec4 o_color;

void main()
{
	vec3 a = textureLod(u_srcATexture, v2f.texCoords, 0).xyz;
	vec3 b = textureLod(u_srcBTexture, v2f.texCoords, 0).xyz;
	
	o_color = vec4(mix(a, b, clamp(u_factor, 0.0f, 1.0f)), 1);
}
