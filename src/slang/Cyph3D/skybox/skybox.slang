cbuffer Parameters
{
	float4x4 u_mvpMatrix;
	float3* u_positionList;
	TextureCube::Handle u_image;
	SamplerState::Handle u_sampler;
};

struct VIn
{
	uint vertex_index : SV_VulkanVertexID;
};

struct V2F
{
	float4 rasterPosition : SV_Position;
	float3 dir;
};

struct FOut
{
	float4 color : SV_Target0;
};

[[shader("vertex")]]
V2F main(VIn input)
{
	float3 pos = u_positionList[input.vertex_index];

	V2F output;
	output.rasterPosition = mul(u_mvpMatrix, float4(pos, 1.0)).xyww;
	output.dir = pos * float3(1.0, 1.0, -1.0);
	return output;
}

[[shader("fragment")]]
FOut main(V2F input)
{
	FOut output;
	output.color = float4(u_image.Sample(u_sampler, input.dir).rgb, 1);
	return output;
}