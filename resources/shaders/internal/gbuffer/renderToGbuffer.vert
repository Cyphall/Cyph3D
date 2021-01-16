layout(location = 0) in vec3 in_Vertex;
layout(location = 1) in vec2 in_UV;
layout(location = 2) in vec3 in_Normals;
layout(location = 3) in vec3 in_tangents;

uniform mat3 normalMatrix;
uniform mat4 model;
uniform mat4 mvp;

out FRAG {
	vec2 TexCoords;
	mat3 TangentToWorld;
	mat3 WorldToTangent;
	vec3 FragPos;
} frag;

void main()
{
	frag.TexCoords = in_UV;
	frag.FragPos = vec3(model * vec4(in_Vertex, 1.0));

	vec3 T = normalize(normalMatrix * in_tangents);
	vec3 N = normalize(normalMatrix * in_Normals);
	vec3 B = cross(N, T);

	frag.TangentToWorld = mat3(T, B, N);
	frag.WorldToTangent = transpose(frag.TangentToWorld);

	gl_Position = mvp * vec4(in_Vertex, 1.0);
}