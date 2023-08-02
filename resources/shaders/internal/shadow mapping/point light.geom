#version 460 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

layout(std430, set = 0, binding = 0) readonly buffer uniforms
{
	mat4 u_viewProjections[6];
	vec3 u_lightPos;
	float u_far;
};

layout(location = 0) out G2F
{
	vec3 o_fragPos; // FragPos from GS (output per emitvertex)
};

void main()
{
	for (int face = 0; face < 6; ++face)
	{
		gl_Layer = face;// built-in variable that specifies to which face we render.
		for (int i = 0; i < 3; ++i)// for each triangle vertex
		{
			vec4 fragPos = gl_in[i].gl_Position;
			gl_Position = u_viewProjections[face] * fragPos;
			o_fragPos = vec3(fragPos);
			
			EmitVertex();
		}
		EndPrimitive();
	}
}  