#version 460 core
#extension GL_ARB_bindless_texture : enable

in V2F
{
    vec2 texCoords;
} v2f;

layout(bindless_sampler) uniform sampler2D u_texture;

layout(location = 0) out vec4 o_color;

void main()
{
	o_color = texture(u_texture, v2f.texCoords);
}