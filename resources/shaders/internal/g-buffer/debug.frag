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
		
		o_color = vec4(texture(u_colorTexture, texCoords).rgb, 1);
	}
	else if (texCoords.x <= 2.0/3.0 && texCoords.y >= 2.0/3.0)
	{
		texCoords.x = (texCoords.x - 1.0/3.0) * 3;
		texCoords.y = (texCoords.y - 2.0/3.0) * 3;
		
		float depth = texture(u_depthTexture, texCoords).r;
		o_color = vec4(depth, depth, depth, 1);
		
	}
	else if (texCoords.x <= 3.0/3.0 && texCoords.y >= 2.0/3.0)
	{
		texCoords.x = (texCoords.x - 2.0/3.0) * 3;
		texCoords.y = (texCoords.y - 2.0/3.0) * 3;
		
		o_color = vec4(texture(u_positionTexture, texCoords).rgb, 1);
	}
	else if (texCoords.x <= 1.0/3.0 && texCoords.y >= 1.0/3.0)
	{
		texCoords.x = (texCoords.x - 0.0/3.0) * 3;
		texCoords.y = (texCoords.y - 1.0/3.0) * 3;
		
		o_color = vec4(texture(u_normalTexture, texCoords).rgb, 1);
	}
	else if (texCoords.x <= 2.0/3.0 && texCoords.y >= 1.0/3.0)
	{
		texCoords.x = (texCoords.x - 1.0/3.0) * 3;
		texCoords.y = (texCoords.y - 1.0/3.0) * 3;
		
		o_color = vec4(texture(u_geometryNormalTexture, texCoords).rgb, 1);
	}
	else if (texCoords.x <= 3.0/3.0 && texCoords.y >= 1.0/3.0)
	{
		texCoords.x = (texCoords.x - 2.0/3.0) * 3;
		texCoords.y = (texCoords.y - 1.0/3.0) * 3;
		
		float roughness = texture(u_materialTexture, texCoords).r;
		o_color = vec4(roughness, roughness, roughness, 1);
	}
	else if (texCoords.x <= 1.0/3.0 && texCoords.y >= 0.0/3.0)
	{
		texCoords.x = (texCoords.x - 0.0/3.0) * 3;
		texCoords.y = (texCoords.y - 0.0/3.0) * 3;
		
		float metallic = texture(u_materialTexture, texCoords).g;
		o_color = vec4(metallic, metallic, metallic, 1);
	}
	else if (texCoords.x <= 2.0/3.0 && texCoords.y >= 0.0/3.0)
	{
		texCoords.x = (texCoords.x - 1.0/3.0) * 3;
		texCoords.y = (texCoords.y - 0.0/3.0) * 3;
		
		float emissive = texture(u_materialTexture, texCoords).b;
		o_color = vec4(emissive, emissive, emissive, 1);
	}
	else if (texCoords.x <= 3.0/3.0 && texCoords.y >= 0.0/3.0)
	{
		texCoords.x = (texCoords.x - 2.0/3.0) * 3;
		texCoords.y = (texCoords.y - 0.0/3.0) * 3;
		
		float isLit = texture(u_materialTexture, texCoords).a;
		o_color = vec4(isLit, isLit, isLit, 1);
	}
}