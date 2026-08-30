cbuffer Parameters
{
	uint2 u_size;
	Texture2D::Handle u_srcImage;
	SamplerState::Handle u_sampler;
	RWTexture2D::Handle u_dstImage;
	float2 u_srcPixelSize;
	float u_weight;
};

[[shader("compute")]]
[[numthreads(8, 8, 1)]]
void main(uint2 tid : SV_DispatchThreadID)
{
	if (any(tid >= u_size))
		return;

	float2 texCoord = (tid + 0.5) / u_size;

	float3 blurredLight = 0.0;

	// high weight samples
	blurredLight += u_srcImage.SampleLevel(u_sampler, texCoord + float2(-1, -1) * u_srcPixelSize, 0).rgb * (0.0625 * 1);
	blurredLight += u_srcImage.SampleLevel(u_sampler, texCoord + float2( 0, -1) * u_srcPixelSize, 0).rgb * (0.0625 * 2);
	blurredLight += u_srcImage.SampleLevel(u_sampler, texCoord + float2(+1, -1) * u_srcPixelSize, 0).rgb * (0.0625 * 1);
	blurredLight += u_srcImage.SampleLevel(u_sampler, texCoord + float2(-1,  0) * u_srcPixelSize, 0).rgb * (0.0625 * 2);
	blurredLight += u_srcImage.SampleLevel(u_sampler, texCoord + float2( 0,  0) * u_srcPixelSize, 0).rgb * (0.0625 * 4);
	blurredLight += u_srcImage.SampleLevel(u_sampler, texCoord + float2(+1,  0) * u_srcPixelSize, 0).rgb * (0.0625 * 2);
	blurredLight += u_srcImage.SampleLevel(u_sampler, texCoord + float2(-1, +1) * u_srcPixelSize, 0).rgb * (0.0625 * 1);
	blurredLight += u_srcImage.SampleLevel(u_sampler, texCoord + float2( 0, +1) * u_srcPixelSize, 0).rgb * (0.0625 * 2);
	blurredLight += u_srcImage.SampleLevel(u_sampler, texCoord + float2(+1, +1) * u_srcPixelSize, 0).rgb * (0.0625 * 1);

	float3 light = u_dstImage.Load(tid).rgb;

	light = lerp(light, blurredLight, u_weight);

	u_dstImage.Store(tid, float4(light, 1.0));
}