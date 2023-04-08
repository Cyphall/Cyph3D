#version 460 core

vec3 ACESFilm(vec3 x);

layout(set = 0, binding = 0) uniform sampler2D u_colorTexture;

layout(location = 0) out vec4 o_color;

void main()
{
	vec3 color = texelFetch(u_colorTexture, ivec2(gl_FragCoord.xy), 0).rgb;
	
	o_color = vec4(ACESFilm(color), 1);
}

// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/

vec3 ACESFilm(vec3 x)
{
	float a = 2.51;
	float b = 0.03;
	float c = 2.43;
	float d = 0.59;
	float e = 0.14;
	return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}