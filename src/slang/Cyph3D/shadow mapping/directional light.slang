#include "../common/vertex types.slang"

cbuffer Parameters
{
	float4x4 u_mvpMatrix;
	PositionVertexData* u_vertexList;
};

struct VIn
{
	uint vertex_index : SV_VulkanVertexID;
};

struct VOut
{
	float4 rasterPosition : SV_Position;
};

[[shader("vertex")]]
VOut main(VIn input)
{
	PositionVertexData vertex = u_vertexList[input.vertex_index];

	VOut output;
	output.rasterPosition = mul(u_mvpMatrix, float4(vertex.pos, 1.0));
	return output;
}