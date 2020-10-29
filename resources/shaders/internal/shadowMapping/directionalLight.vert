layout(location = 0) in vec3 in_Vertex;
layout(location = 1) in vec2 in_UV;
layout(location = 2) in vec3 in_Normals;
layout(location = 3) in vec3 in_tangents;

uniform mat4 model;
uniform mat4 viewProjection;

void main()
{
	gl_Position = viewProjection * model * vec4(in_Vertex, 1.0);
}