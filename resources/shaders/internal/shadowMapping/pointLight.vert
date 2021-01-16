layout(location = 0) in vec3 in_Vertex;

uniform mat4 model;

void main()
{
	gl_Position = model * vec4(in_Vertex, 1.0);
}