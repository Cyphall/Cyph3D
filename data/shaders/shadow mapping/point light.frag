#version 460 core

layout(location = 0) in V2F
{
	vec3 i_fragPos;
};

layout(std430, set = 0, binding = 0) readonly buffer uniforms
{
	mat4 u_viewProjection;
	vec3 u_lightPos;
	float u_maxDistance;
};

void main()
{
	gl_FragDepth = clamp(length(i_fragPos.xyz - u_lightPos) / u_maxDistance, 0.0, 1.0);
} 