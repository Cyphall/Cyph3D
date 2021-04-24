#version 460 core
#extension GL_ARB_bindless_texture : enable

layout(bindless_sampler) uniform sampler2D u_colorTexture;
uniform float u_exposure;

in V2F
{
	vec2 texCoords;
} v2f;

out vec4 o_color;

void main()
{
	vec4 rawColor = texture(u_colorTexture, v2f.texCoords);
	
	vec3 color = rawColor.rgb;
	float alpha = rawColor.a;
	
	color *= pow(2, u_exposure);
	
	o_color = vec4(color, alpha);
}
