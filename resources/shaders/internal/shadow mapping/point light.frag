#version 460 core

in V2F
{
	vec3 fragPos;
} v2f;

uniform vec3 u_lightPos;
uniform float u_far;

void main()
{
	// get distance between fragment and light source
	float lightDistance = length(v2f.fragPos.xyz - u_lightPos);
	
	// map to [0;1] range by dividing by far_plane
	lightDistance /= u_far;
	
	// write this as modified depth
	gl_FragDepth = lightDistance;
} 