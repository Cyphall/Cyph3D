#version 460 core
#extension GL_ARB_bindless_texture : enable

vec3 toSRGB(vec3 linear);
vec3 ACESFilm(vec3 x);

layout(bindless_sampler) uniform sampler2D u_colorTexture;

out vec3 o_color;

void main()
{
	vec3 color = texelFetch(u_colorTexture, ivec2(gl_FragCoord.xy), 0).rgb;
	
	o_color = toSRGB(ACESFilm(color));
}

vec3 toSRGB(vec3 linear)
{
	bvec3 cutoff = lessThan(linear, vec3(0.0031308));
	vec3 higher = vec3(1.055) * pow(linear, vec3(1.0/2.4)) - vec3(0.055);
	vec3 lower = linear * vec3(12.92);
	
	return mix(higher, lower, cutoff);
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