#version 460 core
#extension GL_ARB_bindless_texture : enable

layout(bindless_sampler) uniform sampler2D u_brightOnlyTexture;
uniform bool u_horizontal;

in V2F
{
	vec2 texCoords;
} v2f;

out vec3 o_color;

const float weight[5] = {0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216};

void main()
{
	vec2 tex_offset = 1.0 / textureSize(u_brightOnlyTexture, 0); // gets size of single texel
	vec3 result = texture(u_brightOnlyTexture, v2f.texCoords).rgb * weight[0]; // current fragment's contribution
	
	if(u_horizontal)
	{
		for(int i = 1; i < 5; ++i)
		{
			result += texture(u_brightOnlyTexture, v2f.texCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
			result += texture(u_brightOnlyTexture, v2f.texCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
		}
	}
	else
	{
		for(int i = 1; i < 5; ++i)
		{
			result += texture(u_brightOnlyTexture, v2f.texCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
			result += texture(u_brightOnlyTexture, v2f.texCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
		}
	}
	
	o_color = result;
}
