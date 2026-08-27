#include "../../common/colorspace.slang"

cbuffer Parameters
{
	Texture2D<float3>::Handle u_srcImage;
	WTexture2D::Handle u_dstImage;
	uint32_t u_accumulatedSamples;
	uint2 u_size;
};

[[shader("compute")]]
[[numthreads(8, 8, 1)]]
void main(uint3 threadID : SV_DispatchThreadID)
{
	uint2 tid = threadID.xy;
	if (any(tid >= u_size))
		return;

	float3 light = u_srcImage.Load(uint3(tid, 0));

	light /= u_accumulatedSamples;

	u_dstImage.Store(tid, float4(light, 1));
}