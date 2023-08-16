#version 460 core
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require

struct HitPayload
{
	uint randomOffset;
	vec3 light;
	vec3 contribution;
	bool hit;
	vec3 rayPosition;
	vec3 rayDirection;
};

layout(location = 0) rayPayloadInEXT HitPayload hitPayload;

void main()
{
	hitPayload.hit = false;
	hitPayload.rayPosition = vec3(0);
	hitPayload.rayDirection = vec3(0);
}