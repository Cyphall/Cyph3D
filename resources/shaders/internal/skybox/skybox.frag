#version 460 core
#extension GL_ARB_bindless_texture : enable

in V2F
{
	vec3 texCoords;
} v2f;

layout(bindless_sampler) uniform samplerCube u_skybox;

layout(location = 0) out vec3 o_color;

void main()
{
	o_color = texture(u_skybox, v2f.texCoords).xyz;
}