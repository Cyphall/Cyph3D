#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

in V2G
{
    vec3 fragPos;
	vec2 texCoords;
	vec3 T;
	vec3 N;
} v2g[];

out G2F
{
    vec3 fragPos;
	vec2 texCoords;
	vec3 T;
	vec3 N;
	vec3 flatNormal_WS;
} g2f;

void main()
{
	vec3 a = vec3(v2g[0].fragPos) - vec3(v2g[1].fragPos);
	vec3 b = vec3(v2g[2].fragPos) - vec3(v2g[1].fragPos);
   	vec3 flatNormal = normalize(cross(b, a));

	for(int i = 0; i < 3; i++)
	{
		gl_Position = gl_in[i].gl_Position;

		g2f.fragPos = v2g[i].fragPos;
		g2f.texCoords = v2g[i].texCoords;
		g2f.T = v2g[i].T;
		g2f.N = v2g[i].N;
		g2f.flatNormal_WS = flatNormal;

		EmitVertex();
	}
	EndPrimitive();
}  