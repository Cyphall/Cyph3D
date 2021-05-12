#version 460 core
#extension GL_ARB_bindless_texture : enable

layout(bindless_sampler) uniform sampler2D u_colorTexture;
uniform int u_level;

in V2F
{
	vec2 texCoords;
} v2f;

out vec4 o_color;

void main()
{
	o_color = textureLod(u_colorTexture, v2f.texCoords, u_level);
}
