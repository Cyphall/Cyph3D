#version 460 core
#extension GL_ARB_bindless_texture : enable

layout(bindless_sampler) uniform sampler2D u_colorTexture1;
layout(bindless_sampler) uniform sampler2D u_colorTexture2;

out vec3 o_color;

void main()
{
	o_color = texelFetch(u_colorTexture1, ivec2(gl_FragCoord), 0).rgb + texelFetch(u_colorTexture2, ivec2(gl_FragCoord), 0).rgb;
}
