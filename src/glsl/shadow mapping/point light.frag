#version 460 core

#extension GL_EXT_scalar_block_layout : require

layout(location = 0) in V2F
{
	vec3 i_fragPos;
};

layout(set = 0, binding = 0, scalar) readonly buffer uniforms
{
	mat4 u_viewProjection;
	vec3 u_lightPos;
	float u_maxDistance;
};

void main()
{
	gl_FragDepth = clamp(length(i_fragPos.xyz - u_lightPos) / u_maxDistance, 0.0, 1.0);
} 