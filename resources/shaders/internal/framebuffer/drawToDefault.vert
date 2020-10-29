layout(location = 0) in vec2 in_Vertex;
layout(location = 1) in vec2 in_UV;

out vec2 TexCoords;

void main()
{
	gl_Position = vec4(in_Vertex, 0, 1);

	TexCoords = in_UV;
}