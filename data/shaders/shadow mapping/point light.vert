#version 460 core

layout(location = 0) in vec3 a_position;

layout(push_constant) uniform constants
{
	mat4 u_model;
};

layout(std430, set = 0, binding = 0) readonly buffer uniforms
{
	mat4 u_viewProjection;
	vec3 u_lightPos;
	float u_maxDistance;
};

layout(location = 0) out V2F
{
	vec3 o_fragPos;
};

void main()
{
	vec4 fragPos = u_model * vec4(a_position, 1.0);
	gl_Position = u_viewProjection * fragPos;
	o_fragPos = fragPos.xyz;
}