#version 460 core

layout(location = 0) in V2F
{
    vec4 i_color;
    vec2 i_uv;
};

layout(set = 0, binding = 0) uniform texture2D u_texture;
layout(set = 0, binding = 1) uniform sampler u_sampler;

layout(location = 0) out vec4 o_color;

void main()
{
    o_color = i_color * texture(sampler2D(u_texture, u_sampler), i_uv);
}