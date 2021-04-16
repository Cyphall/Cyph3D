#version 460 core
#extension GL_ARB_bindless_texture : enable

in V2F {
	vec2  texCoords;
} v2f;

layout(bindless_sampler) uniform sampler2D u_normalTexture;
layout(bindless_sampler) uniform sampler2D u_colorTexture;
layout(bindless_sampler) uniform sampler2D u_materialTexture;
layout(bindless_sampler) uniform sampler2D u_geometryNormalTexture;
layout(bindless_sampler) uniform sampler2D u_depthTexture;
layout(bindless_sampler) uniform sampler2D u_positionTexture;
uniform mat4 u_viewProjectionInv;

out vec4 o_color;

void main()
{
	vec2 texCoords = v2f.texCoords;
	
	if (texCoords.x <= 1.0/3.0 && texCoords.y >= 2.0/3.0)
	{
		texCoords.x = (texCoords.x - 0.0/3.0) * 3;
		texCoords.y = (texCoords.y - 2.0/3.0) * 3;
		
		o_color = texture(u_positionTexture, texCoords);
	}
	else if (texCoords.x <= 2.0/3.0 && texCoords.y >= 2.0/3.0)
	{
		texCoords.x = (texCoords.x - 1.0/3.0) * 3;
		texCoords.y = (texCoords.y - 2.0/3.0) * 3;
		o_color = texture(u_normalTexture, texCoords);
	}
	else if (texCoords.x <= 3.0/3.0 && texCoords.y >= 2.0/3.0)
	{
		texCoords.x = (texCoords.x - 2.0/3.0) * 3;
		texCoords.y = (texCoords.y - 2.0/3.0) * 3;
		o_color = texture(u_geometryNormalTexture, texCoords);
	}
	else if (texCoords.x <= 1.0/3.0 && texCoords.y >= 1.0/3.0)
	{
		texCoords.x = (texCoords.x - 0.0/3.0) * 3;
		texCoords.y = (texCoords.y - 1.0/3.0) * 3;
		o_color = texture(u_materialTexture, texCoords);
	}
	else if (texCoords.x <= 2.0/3.0 && texCoords.y >= 1.0/3.0)
	{
		texCoords.x = (texCoords.x - 1.0/3.0) * 3;
		texCoords.y = (texCoords.y - 1.0/3.0) * 3;
		o_color = texture(u_colorTexture, texCoords);
	}
	else if (texCoords.x <= 3.0/3.0 && texCoords.y >= 1.0/3.0)
	{
		texCoords.x = (texCoords.x - 2.0/3.0) * 3;
		texCoords.y = (texCoords.y - 1.0/3.0) * 3;
		
		float depth = texture(u_depthTexture, texCoords).r;
		o_color = vec4(depth, depth, depth, 1);
	}
	else
	{
		o_color = vec4(0);
	}
}