#version 460 core

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : require

#include "../common/colorspace.glsl"

struct HitPayload
{
	uint randomOffset;
	vec3 light;
	vec3 throughput;
	bool hit;
	vec3 rayPosition;
	vec3 rayDirection;
};

layout(set = 0, binding = 0) uniform samplerCube u_textures[];

layout(shaderRecordEXT) buffer uniforms
{
	uint u_skyboxIndex;
	mat4 u_skyboxRotation;
};

layout(location = 0) rayPayloadInEXT HitPayload hitPayload;

void main()
{
	vec3 rayDir = gl_WorldRayDirectionEXT;
	rayDir *= vec3(1, 1, -1);
	rayDir = (u_skyboxRotation * vec4(rayDir, 1.0)).xyz;
	vec3 skyboxColor = linearToAP1(texture(u_textures[u_skyboxIndex], rayDir).rgb);
	
	hitPayload.light += hitPayload.throughput * skyboxColor;
	hitPayload.hit = false;
	hitPayload.rayPosition = vec3(0);
	hitPayload.rayDirection = vec3(0);
}