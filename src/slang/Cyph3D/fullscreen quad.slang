struct VIn
{
	uint vertex_index : SV_VulkanVertexID;
};

struct VOut
{
	float4 rasterPosition : SV_Position;
	float2 texCoord;
};

// https://www.saschawillems.de/blog/2016/08/13/vulkan-tutorial-on-rendering-a-fullscreen-quad-without-buffers/
[[shader("vertex")]]
VOut main(VIn input)
{
	float2 texCoord = float2(input.vertex_index & 2, (input.vertex_index << 1) & 2);

	VOut output;
	output.rasterPosition = float4(texCoord * 2.0 + -1.0, 0.0, 1.0);
	output.texCoord = texCoord;
	return output;
}