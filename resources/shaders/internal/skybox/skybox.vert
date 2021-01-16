layout (location = 0) in vec3 in_Vertex;

out vec3 TexCoords;

uniform mat4 mvp;

void main()
{
	TexCoords = in_Vertex * vec3(-1, -1, 1);
	vec4 pos = mvp * vec4(in_Vertex, 1.0);
	gl_Position = pos.xyww;
} 