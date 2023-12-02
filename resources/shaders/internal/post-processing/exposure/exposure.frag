#version 460 core

layout(set = 0, binding = 0) uniform sampler2D u_colorTexture;

layout(push_constant) uniform constants
{
	float u_exposure;
};

layout(location = 0) out vec4 o_color;

void main()
{
	vec3 color = texelFetch(u_colorTexture, ivec2(gl_FragCoord.xy), 0).rgb;

	color *= pow(2, u_exposure);

	o_color = vec4(color, 1);
}