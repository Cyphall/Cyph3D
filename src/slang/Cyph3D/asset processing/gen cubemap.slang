#include "../common/colorspace.slang"

cbuffer Parameters
{
	float4x4 u_viewProjectionInv;
	Texture2D::Handle u_equirectangularImage;
	WTexture2D::Handle u_cubemapImage;
	SamplerState::Handle u_sampler;
	uint2 u_size;
	float2 u_invSizeF;
	bool u_srgb;
};

float3 sampleColor(float2 cubemapUV)
{
	float4 direction4D = mul(u_viewProjectionInv, float4(cubemapUV * 2.0 - 1.0, 0.5, 1.0));
	float3 direction = normalize(direction4D.xyz / direction4D.w);

	float2 sphericalCoords = float2(
		atan2(direction.x, -direction.z) + float.getPi(),
		acos(direction.y)
	);

	float2 equirectangularUV = float2(
		sphericalCoords.x / (float.getPi() * 2.0),
		sphericalCoords.y / float.getPi()
	);

	return u_equirectangularImage.SampleLevel(u_sampler, equirectangularUV, 0.0).rgb;
}

[[shader("compute")]]
[[numthreads(8, 8, 1)]]
void main(uint3 threadID : SV_DispatchThreadID)
{
	uint2 tid = threadID.xy;
	if (any(tid >= u_size))
		return;

	float3 color = float3(0);

	color += sampleColor((tid + float2(0.25, 0.25)) * u_invSizeF);
	color += sampleColor((tid + float2(0.75, 0.25)) * u_invSizeF);
	color += sampleColor((tid + float2(0.25, 0.75)) * u_invSizeF);
	color += sampleColor((tid + float2(0.75, 0.75)) * u_invSizeF);

	color *= 0.25;

	if (u_srgb)
		color = linearToSrgb(color);

	u_cubemapImage.Store(tid, float4(color, 1.0));
}