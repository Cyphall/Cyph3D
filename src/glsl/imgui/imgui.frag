#version 460 core

#extension GL_EXT_scalar_block_layout : require

layout(location = 0) in V2F
{
	vec4 i_color;
	vec2 i_uv;
};

layout(set = 0, binding = 0) uniform sampler2D u_texture;

layout(location = 0) out vec4 o_color;

void main()
{
	o_color = i_color * texture(u_texture, i_uv);
}