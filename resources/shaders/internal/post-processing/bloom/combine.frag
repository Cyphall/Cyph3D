#version 460 core
#extension GL_ARB_bindless_texture : enable

layout(bindless_sampler) uniform sampler2D u_colorTexture1;
layout(bindless_sampler) uniform sampler2D u_colorTexture2;

in V2F
{
	vec2 texCoords;
} v2f;

out vec3 o_color;

void main()
{
	o_color = texture(u_colorTexture1, v2f.texCoords).rgb + texture(u_colorTexture2, v2f.texCoords).rgb;
}
