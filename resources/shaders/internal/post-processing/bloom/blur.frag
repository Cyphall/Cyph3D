#version 460 core
#extension GL_ARB_bindless_texture : enable

layout(bindless_sampler) uniform sampler2D u_sourceTexture;
uniform bool u_horizontal;
uniform int u_mipmapLevel;

struct KernelSampleInfo
{
	float weight;
	float offset;
};

layout(std430, binding = 2) buffer UselessNameBecauseItIsNeverUsedAnywhere1
{
	KernelSampleInfo samples[];
};

out vec3 o_color;

void main()
{
	vec2 pixelSize = 1.0f / textureSize(u_sourceTexture, u_mipmapLevel);
	vec2 fragmentPos = vec2(gl_FragCoord) * pixelSize;
	
	vec3 result = textureLod(u_sourceTexture, fragmentPos, u_mipmapLevel).rgb * samples[0].weight; // current fragment's contribution
	
	if(u_horizontal)
	{
		for(int i = 1; i < samples.length(); ++i)
		{
			result += textureLod(u_sourceTexture, fragmentPos + vec2(samples[i].offset * pixelSize.x, 0), u_mipmapLevel).rgb * samples[i].weight;
			result += textureLod(u_sourceTexture, fragmentPos - vec2(samples[i].offset * pixelSize.x, 0), u_mipmapLevel).rgb * samples[i].weight;
		}
	}
	else
	{
		for(int i = 1; i < samples.length(); ++i)
		{
			result += textureLod(u_sourceTexture, fragmentPos + vec2(0, samples[i].offset * pixelSize.y), u_mipmapLevel).rgb * samples[i].weight;
			result += textureLod(u_sourceTexture, fragmentPos - vec2(0, samples[i].offset * pixelSize.y), u_mipmapLevel).rgb * samples[i].weight;
		}
	}
	
	o_color = result;
}