#version 460 core

layout(location = 0) in V2F
{
	vec3 i_texCoords;
};

layout(set = 0, binding = 0) uniform samplerCube u_skybox;

layout(location = 0) out vec4 o_color;

void main()
{
	o_color = vec4(texture(u_skybox, i_texCoords).xyz, 1);
}