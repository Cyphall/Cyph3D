#include "../common/vertex types.slang"

cbuffer Parameters
{
	float4x4 u_mvpMatrices[6];
	float4x4 u_mMatrix;
	PositionVertexData* u_vertexList;
	float3 u_lightPos;
	float u_invMaxDistance;
};

struct VIn
{
	uint vertexIndex : SV_VulkanVertexID;
	uint viewIndex : SV_ViewID;
};

struct V2F
{
	float4 rasterPosition : SV_Position;
	float3 pos;
};

struct FOut
{
	float depth : SV_Depth;
};

[[shader("vertex")]]
V2F main(VIn input)
{
	PositionVertexData vertex = u_vertexList[input.vertexIndex];

	V2F output;
	output.rasterPosition = mul(u_mvpMatrices[input.viewIndex], float4(vertex.pos, 1.0));
	output.pos = mul(u_mMatrix, float4(vertex.pos, 1.0)).xyz;
	return output;
}

[[shader("fragment")]]
FOut main(V2F input)
{
	FOut output;
	output.depth = clamp(length(input.pos.xyz - u_lightPos) * u_invMaxDistance, 0.0, 1.0);
	return output;
}