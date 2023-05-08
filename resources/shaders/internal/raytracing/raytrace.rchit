#version 460 core
#extension GL_EXT_ray_tracing : require

struct HitPayload
{
	vec3 value;
};

layout(location = 0) rayPayloadInEXT HitPayload hitPayload;
hitAttributeEXT vec3 attribs;

void main()
{
	hitPayload.value = vec3(0.2, 0.5, 0.5);
}
