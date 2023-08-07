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

layout(set = 0, binding = 0) uniform samplerCube u_textures[];

layout(std430, set = 1, binding = 3) uniform uniforms
{
	bool u_hasSkybox;
	uint u_skyboxIndex;
	mat4 u_skyboxRotation;
};

layout(location = 0) rayPayloadInEXT HitPayload hitPayload;

void main()
{
	vec3 skyboxColor;
	if (u_hasSkybox)
	{
		vec3 rayDir = gl_WorldRayDirectionEXT;
		rayDir *= vec3(1, 1, -1);
		rayDir = (u_skyboxRotation * vec4(rayDir, 1.0)).xyz;
		skyboxColor = texture(u_textures[u_skyboxIndex], rayDir).rgb;
	}
	else
	{
		skyboxColor = vec3(0);
	}
	
	hitPayload.light += hitPayload.contribution * skyboxColor;
	hitPayload.hit = false;
	hitPayload.rayPosition = vec3(0);
	hitPayload.rayDirection = vec3(0);
}