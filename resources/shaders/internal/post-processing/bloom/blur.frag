#version 460 core
#extension GL_ARB_bindless_texture : enable

layout(bindless_sampler) uniform sampler2D u_sourceTexture;
uniform bool u_horizontal;
uniform int u_mipmapLevel;

layout(std430, binding = 2) buffer UselessNameBecauseItIsNeverUsedAnywhere1
{
	float weight[];
};

in V2F
{
	vec2 texCoords;
} v2f;

out vec3 o_color;

void main()
{
	vec2 tex_offset = 1.0 / textureSize(u_sourceTexture, u_mipmapLevel); // gets size of single texel
	vec3 result = textureLod(u_sourceTexture, v2f.texCoords, u_mipmapLevel).rgb * weight[0]; // current fragment's contribution
	
	if(u_horizontal)
	{
		for(int i = 1; i < weight.length(); ++i)
		{
			result += textureLod(u_sourceTexture, v2f.texCoords + vec2(tex_offset.x * i, 0.0), u_mipmapLevel).rgb * weight[i];
			result += textureLod(u_sourceTexture, v2f.texCoords - vec2(tex_offset.x * i, 0.0), u_mipmapLevel).rgb * weight[i];
		}
	}
	else
	{
		for(int i = 1; i < weight.length(); ++i)
		{
			result += textureLod(u_sourceTexture, v2f.texCoords + vec2(0.0, tex_offset.y * i), u_mipmapLevel).rgb * weight[i];
			result += textureLod(u_sourceTexture, v2f.texCoords - vec2(0.0, tex_offset.y * i), u_mipmapLevel).rgb * weight[i];
		}
	}
	
	o_color = result;
}
