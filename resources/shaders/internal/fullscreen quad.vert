#version 460 core

layout(location = 0) out V2F
{
	vec2 o_texCoords;
};

// https://www.saschawillems.de/blog/2016/08/13/vulkan-tutorial-on-rendering-a-fullscreen-quad-without-buffers/
void main()
{
	o_texCoords = vec2(gl_VertexIndex & 2, (gl_VertexIndex << 1) & 2);
	gl_Position = vec4(o_texCoords * 2.0 + -1.0, 0.0, 1.0);
}