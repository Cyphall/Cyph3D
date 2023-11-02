#version 460 core

layout(set = 0, binding = 0) uniform sampler2D u_colorTexture;

layout(location = 0) out vec4 o_color;

// AP1 => RRT_SAT
const mat3 RRT_SAT = mat3(
	0.970889, 0.026963, 0.002148,
	0.010889, 0.986963, 0.002148,
	0.010889, 0.026963, 0.962148
);

// ODT_SAT => XYZ => D60_2_D65 => sRGB
const mat3 ACESOutputMat = mat3(
	 1.60475, -0.53108, -0.07367,
	-0.10208,  1.10813, -0.00605,
	-0.00327, -0.07276,  1.07602
);

vec3 RRTAndODTFit(vec3 x)
{
	vec3 a = (x            + 0.0245786) * x;
	vec3 b = (x * 0.983729 + 0.4329510) * x + 0.238081;
	
	return a / b;
}

vec3 ACESFittedFromACEScg(vec3 v)
{
	v = v * RRT_SAT;
	v = RRTAndODTFit(v);
	return v * ACESOutputMat;
}

void main()
{
	vec3 color = texelFetch(u_colorTexture, ivec2(gl_FragCoord.xy), 0).rgb;
	
	o_color = vec4(ACESFittedFromACEScg(color), 1);
}