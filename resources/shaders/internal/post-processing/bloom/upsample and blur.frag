#version 460 core
#extension GL_ARB_bindless_texture : enable

in V2F
{
	vec2 texCoords;
} v2f;

layout(bindless_sampler) uniform sampler2D u_srcTexture;
uniform int u_srcLevel;
uniform vec2 u_srcPixelSize;
uniform float u_bloomRadius;

out vec4 o_color;

void main()
{
	vec3 result = vec3(0);
	
	// high weight samples
	result += textureLod(u_srcTexture, v2f.texCoords + u_srcPixelSize * vec2(-1, -1), u_srcLevel).rgb * (0.0625 * 1);
	result += textureLod(u_srcTexture, v2f.texCoords + u_srcPixelSize * vec2( 0, -1), u_srcLevel).rgb * (0.0625 * 2);
	result += textureLod(u_srcTexture, v2f.texCoords + u_srcPixelSize * vec2(+1, -1), u_srcLevel).rgb * (0.0625 * 1);
	result += textureLod(u_srcTexture, v2f.texCoords + u_srcPixelSize * vec2(-1,  0), u_srcLevel).rgb * (0.0625 * 2);
	result += textureLod(u_srcTexture, v2f.texCoords + u_srcPixelSize * vec2( 0,  0), u_srcLevel).rgb * (0.0625 * 4);
	result += textureLod(u_srcTexture, v2f.texCoords + u_srcPixelSize * vec2(+1,  0), u_srcLevel).rgb * (0.0625 * 2);
	result += textureLod(u_srcTexture, v2f.texCoords + u_srcPixelSize * vec2(-1, +1), u_srcLevel).rgb * (0.0625 * 1);
	result += textureLod(u_srcTexture, v2f.texCoords + u_srcPixelSize * vec2( 0, +1), u_srcLevel).rgb * (0.0625 * 2);
	result += textureLod(u_srcTexture, v2f.texCoords + u_srcPixelSize * vec2(+1, +1), u_srcLevel).rgb * (0.0625 * 1);
	
	o_color = vec4(result, u_bloomRadius);
}