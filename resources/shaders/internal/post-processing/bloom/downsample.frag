#version 460 core

layout(location = 0) in V2F
{
	vec2 i_texCoords;
};

layout(set = 0, binding = 0) uniform sampler2D u_srcTexture;

layout(push_constant) uniform constants
{
	vec2 u_srcPixelSize;
	int u_srcLevel;
};

layout(location = 0) out vec4 o_color;

void main()
{
	vec3 result = vec3(0);

	// high weight samples
	result += textureLod(u_srcTexture, i_texCoords + u_srcPixelSize * vec2(-1, -1), u_srcLevel).rgb * (0.5 * 0.25 * 1);
	result += textureLod(u_srcTexture, i_texCoords + u_srcPixelSize * vec2(+1, -1), u_srcLevel).rgb * (0.5 * 0.25 * 1);
	result += textureLod(u_srcTexture, i_texCoords + u_srcPixelSize * vec2(-1, +1), u_srcLevel).rgb * (0.5 * 0.25 * 1);
	result += textureLod(u_srcTexture, i_texCoords + u_srcPixelSize * vec2(+1, +1), u_srcLevel).rgb * (0.5 * 0.25 * 1);

	// low weight samples
	result += textureLod(u_srcTexture, i_texCoords + u_srcPixelSize * vec2(-2, -2), u_srcLevel).rgb * (0.125 * 0.25 * 1);
	result += textureLod(u_srcTexture, i_texCoords + u_srcPixelSize * vec2( 0, -2), u_srcLevel).rgb * (0.125 * 0.25 * 2);
	result += textureLod(u_srcTexture, i_texCoords + u_srcPixelSize * vec2(+2, -2), u_srcLevel).rgb * (0.125 * 0.25 * 1);
	result += textureLod(u_srcTexture, i_texCoords + u_srcPixelSize * vec2(-2,  0), u_srcLevel).rgb * (0.125 * 0.25 * 2);
	result += textureLod(u_srcTexture, i_texCoords + u_srcPixelSize * vec2( 0,  0), u_srcLevel).rgb * (0.125 * 0.25 * 4);
	result += textureLod(u_srcTexture, i_texCoords + u_srcPixelSize * vec2(+2,  0), u_srcLevel).rgb * (0.125 * 0.25 * 2);
	result += textureLod(u_srcTexture, i_texCoords + u_srcPixelSize * vec2(-2, +2), u_srcLevel).rgb * (0.125 * 0.25 * 1);
	result += textureLod(u_srcTexture, i_texCoords + u_srcPixelSize * vec2( 0, +2), u_srcLevel).rgb * (0.125 * 0.25 * 2);
	result += textureLod(u_srcTexture, i_texCoords + u_srcPixelSize * vec2(+2, +2), u_srcLevel).rgb * (0.125 * 0.25 * 1);

	o_color = vec4(result, 1);
}