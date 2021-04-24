#version 460 core
#extension GL_ARB_bindless_texture : enable

layout(bindless_sampler) uniform sampler2D u_colorTexture;

in V2F
{
	vec2 texCoords;
} v2f;

out vec3 o_color;

void main()
{
	vec3 color = texture(u_colorTexture, v2f.texCoords).rgb;
	
	float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
	
	o_color = brightness > 1.0 ? color : vec3(0.0, 0.0, 0.0);
}
