#version 460 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec3 a_normal;
layout(location = 3) in vec3 a_tangent;

uniform mat3 u_normalMatrix;
uniform mat4 u_model;
uniform mat4 u_mvp;

out V2F
{
    vec3 fragPos;
	vec2 texCoords;
	mat3 tangentToWorld;
	mat3 worldToTangent;
} v2f;

void main()
{
    v2f.texCoords = a_uv;
    v2f.fragPos = vec3(u_model * vec4(a_position, 1.0));

	vec3 T = normalize(u_normalMatrix * a_tangent);
	vec3 N = normalize(u_normalMatrix * a_normal);
	vec3 B = cross(N, T);

    v2f.tangentToWorld = mat3(T, B, N);
    v2f.worldToTangent = transpose(v2f.tangentToWorld);

	gl_Position = u_mvp * vec4(a_position, 1.0);
}