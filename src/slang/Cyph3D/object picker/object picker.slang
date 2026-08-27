#include "../common/vertex types.slang"

cbuffer Parameters
{
	float4x4 u_mvpMatrix;
	PositionVertexData* u_vertexList;
	int32_t u_objectIndex;
};

struct VIn
{
	uint vertex_index : SV_VulkanVertexID;
};

struct V2F
{
	float4 rasterPosition : SV_Position;
};

struct FOut
{
	int objectIndex : SV_Target0;
};

[[shader("vertex")]]
V2F main(VIn input)
{
	PositionVertexData vertex = u_vertexList[input.vertex_index];

	V2F output;
	output.rasterPosition = mul(u_mvpMatrix, float4(vertex.pos, 1.0));
	return output;
}

[[shader("fragment")]]
FOut main(V2F input)
{
	FOut output;
	output.objectIndex = u_objectIndex;
	return output;
}