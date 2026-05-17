#version 460 core

#extension GL_EXT_scalar_block_layout : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_ray_tracing : require
#extension GL_ARB_gpu_shader_fp64 : require
#extension GL_EXT_nonuniform_qualifier : require

struct HitPayload
{
	uint randomOffset;
	vec3 emitted;
	vec3 weight;
	bool hit;
	vec3 rayPosition;
	vec3 rayDirection;
};

layout(set = 0, binding = 0) uniform samplerCube u_textures[];

layout(shaderRecordEXT, scalar) buffer uniforms
{
	bool u_hasSkybox;
	uint u_skyboxIndex;
	mat4 u_skyboxRotation;
};

layout(location = 0) rayPayloadInEXT HitPayload hitPayload;

void main()
{
	hitPayload.emitted = vec3(0);
	if (u_hasSkybox)
	{
		vec3 rayDir = gl_WorldRayDirectionEXT;
		rayDir *= vec3(1, 1, -1);
		rayDir = (u_skyboxRotation * vec4(rayDir, 1.0)).xyz;
		vec3 skyboxColor = texture(u_textures[u_skyboxIndex], rayDir).rgb;

		hitPayload.emitted = skyboxColor;
	}

	hitPayload.hit = false;
}