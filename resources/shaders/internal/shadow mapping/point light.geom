#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 u_viewProjections[6];

out V2F// not exactly v2f as we are in a geometry shader, but we keep the same convention as other shaders
{
	vec3 fragPos;// FragPos from GS (output per emitvertex)
} v2f;

void main()
{
	for (int face = 0; face < 6; ++face)
	{
		gl_Layer = face;// built-in variable that specifies to which face we render.
		for (int i = 0; i < 3; ++i)// for each triangle vertex
		{
			vec4 fragPos = gl_in[i].gl_Position;
			gl_Position = u_viewProjections[face] * fragPos;
			v2f.fragPos = vec3(fragPos);
			
			EmitVertex();
		}
		EndPrimitive();
	}
}  