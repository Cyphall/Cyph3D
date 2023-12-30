#version 460 core

#extension GL_GOOGLE_include_directive : require

#include "../scene common data.glsl"

layout(location = 0) in V2F
{
	vec3 i_fragPos;
};

layout(push_constant) uniform constants
{
	mat4 u_viewProjection;
	ModelBuffer u_modelBuffer;
	vec3 u_lightPos;
};

void main()
{
	gl_FragDepth = length(i_fragPos.xyz - u_lightPos);
} 