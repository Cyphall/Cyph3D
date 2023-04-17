#version 460 core

layout(location = 0) in vec2 i_position;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec4 i_color;

layout(push_constant) uniform constants
{
    vec2 u_scale;
    vec2 u_translate;
};

layout(location = 0) out V2F
{
    vec4 o_color;
    vec2 o_uv;
};

void main()
{
    o_color = i_color;
    o_uv = i_uv;
    gl_Position = vec4(i_position * u_scale + u_translate, 0, 1);
}