#version 460 core
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in V2F
{
	vec3 i_texCoords;
};

layout(push_constant) uniform constants
{
	mat4 u_mvp;
	uint u_textureIndex;
	float u_exposure;
};

layout(set = 0, binding = 0) uniform samplerCube u_textures[];

layout(location = 0) out vec4 o_color;

void main()
{
	o_color = vec4(texture(u_textures[u_textureIndex], i_texCoords).rgb * 30000 * u_exposure, 1);
}