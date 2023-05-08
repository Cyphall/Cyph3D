#version 460 core
#extension GL_EXT_ray_tracing : require

struct HitPayload
{
	vec3 value;
};

layout(location = 0) rayPayloadInEXT HitPayload hitPayload;

void main()
{
	hitPayload.value = vec3(0.0, 0.1, 0.3);
}