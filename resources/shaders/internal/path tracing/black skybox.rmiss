#version 460 core

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_ARB_gpu_shader_int64 : require

struct HitPayload
{
	uint randomOffset;
	u64vec3 light;
	vec3 contribution;
	bool hit;
	vec3 rayPosition;
	vec3 rayDirection;
};

layout(push_constant) uniform constants
{
	uint u_batchIndex;
	uint u_sampleCount;
	bool u_resetAccumulation;
	uint u_fixedPointDecimals;
};

layout(location = 0) rayPayloadInEXT HitPayload hitPayload;

void main()
{
	hitPayload.hit = false;
	hitPayload.rayPosition = vec3(0);
	hitPayload.rayDirection = vec3(0);
}