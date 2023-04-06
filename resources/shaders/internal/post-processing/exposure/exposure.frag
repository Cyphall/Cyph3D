#version 460 core
#extension GL_ARB_bindless_texture : enable

layout(bindless_sampler) uniform sampler2D u_colorTexture;
uniform float u_exposure;

out vec4 o_color;

void main()
{
	vec3 color = texelFetch(u_colorTexture, ivec2(gl_FragCoord.xy), 0).rgb;
	
	color *= pow(2, u_exposure);
	
	o_color = vec4(color, 1);
}