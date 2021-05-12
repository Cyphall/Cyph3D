#version 460 core
#extension GL_ARB_bindless_texture : enable

layout(bindless_sampler) uniform sampler2D u_colorTexture;

in V2F
{
	vec2 texCoords;
} v2f;

layout(location=0) out vec3 o_nonBrightColor;
layout(location=1) out vec3 o_brightColor;

void main()
{
	vec3 color = texture(u_colorTexture, v2f.texCoords).rgb;
	
	float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
	
	o_nonBrightColor = color / max(brightness, 1);
	o_brightColor = (o_nonBrightColor * max(brightness-1, 0)) * 0.2;
}
