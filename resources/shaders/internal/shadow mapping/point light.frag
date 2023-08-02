#version 460 core

layout(location = 0) in G2F
{
	vec3 i_fragPos;
};

layout(std430, set = 0, binding = 0) readonly buffer uniforms
{
	mat4 u_viewProjections[6];
	vec3 u_lightPos;
	float u_far;
};

void main()
{
	// get distance between fragment and light source
	float lightDistance = length(i_fragPos.xyz - u_lightPos);
	
	// map to [0;1] range by dividing by far_plane
	lightDistance /= u_far;
	
	// write this as modified depth
	gl_FragDepth = lightDistance;
} 