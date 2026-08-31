cbuffer Parameters
{
	float4x4 u_invMvpMatrix;
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
	float2 texCoord;
};

struct FOut
{
	float4 color : SV_Target0;
};

// https://www.saschawillems.de/blog/2016/08/13/vulkan-tutorial-on-rendering-a-fullscreen-quad-without-buffers/
[[shader("vertex")]]
V2F main(VIn input)
{
	float2 texCoord = float2(input.vertex_index & 2, (input.vertex_index << 1) & 2);

	V2F output;
	output.rasterPosition = float4(texCoord * 2.0 - 1.0, 1.0, 1.0);
	output.texCoord = texCoord;
	return output;
}

[[shader("fragment")]]
FOut main(V2F input)
{
	float3 ndcPosNear = {input.texCoord * 2.0 - 1.0, 0.0};
	float3 ndcPosFar = {input.texCoord * 2.0 - 1.0, 1.0};

	float4 localPosNear = mul(u_invMvpMatrix, float4(ndcPosNear, 1.0));
	localPosNear /= localPosNear.w;
	float4 localPosFar = mul(u_invMvpMatrix, float4(ndcPosFar, 1.0));
	localPosFar /= localPosFar.w;

	float3 dir = localPosFar.xyz - localPosNear.xyz;
	dir.z *= -1.0;

	FOut output;
	output.color = float4(u_image.Sample(u_sampler, dir).rgb, 1);
	return output;
}