vec3 toSRGB(vec3 linear);

vec3 ACESFilm(vec3 x);

layout(bindless_sampler) uniform sampler2D colorTexture;

uniform float exposure;

in vec2 TexCoords;

out vec4 outColor;

void main()
{
	vec4 rawColor = texture(colorTexture, TexCoords);

	vec3 color = rawColor.rgb;

	color *= pow(2, exposure);

	outColor = vec4(toSRGB(ACESFilm(color)), rawColor.a);
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
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0f, 1f);
}