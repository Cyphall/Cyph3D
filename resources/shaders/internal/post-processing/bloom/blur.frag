#version 460 core
#extension GL_ARB_bindless_texture : enable

layout(bindless_sampler) uniform sampler2D u_sourceTexture;
uniform bool u_horizontal;
uniform int u_mipmapLevel;

layout(std430, binding = 2) buffer UselessNameBecauseItIsNeverUsedAnywhere1
{
	float weight[];
};

out vec3 o_color;

void main()
{
	ivec2 texelPos = ivec2(gl_FragCoord);
	vec3 result = texelFetch(u_sourceTexture, texelPos, u_mipmapLevel).rgb * weight[0]; // current fragment's contribution
	
	if(u_horizontal)
	{
		for(int i = 1; i < weight.length(); ++i)
		{
			result += texelFetch(u_sourceTexture, texelPos + ivec2(i, 0), u_mipmapLevel).rgb * weight[i];
			result += texelFetch(u_sourceTexture, texelPos - ivec2(i, 0), u_mipmapLevel).rgb * weight[i];
		}
	}
	else
	{
		for(int i = 1; i < weight.length(); ++i)
		{
			result += texelFetch(u_sourceTexture, texelPos + ivec2(0, i), u_mipmapLevel).rgb * weight[i];
			result += texelFetch(u_sourceTexture, texelPos - ivec2(0, i), u_mipmapLevel).rgb * weight[i];
		}
	}
	
	o_color = result;
}
