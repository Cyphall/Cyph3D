#version 460 core
#extension GL_ARB_bindless_texture : enable

layout(bindless_sampler) uniform sampler2D u_colorTexture;
uniform float u_exposure;

in V2F
{
	vec2 texCoords;
} v2f;

out vec3 o_color;

void main()
{
	vec3 color = texture(u_colorTexture, v2f.texCoords).rgb;
	
	color *= pow(2, u_exposure);
	
	o_color = color;
}
