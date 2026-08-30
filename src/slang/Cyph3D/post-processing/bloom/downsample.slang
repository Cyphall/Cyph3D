cbuffer Parameters
{
	uint2 u_size;
	Texture2D::Handle u_srcImage;
	SamplerState::Handle u_sampler;
	WTexture2D::Handle u_dstImage;
	float2 u_srcPixelSize;
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
	blurredLight += u_srcImage.SampleLevel(u_sampler, texCoord + float2(-1, -1) * u_srcPixelSize, 0).rgb * (0.5 * 0.25 * 1);
	blurredLight += u_srcImage.SampleLevel(u_sampler, texCoord + float2(+1, -1) * u_srcPixelSize, 0).rgb * (0.5 * 0.25 * 1);
	blurredLight += u_srcImage.SampleLevel(u_sampler, texCoord + float2(-1, +1) * u_srcPixelSize, 0).rgb * (0.5 * 0.25 * 1);
	blurredLight += u_srcImage.SampleLevel(u_sampler, texCoord + float2(+1, +1) * u_srcPixelSize, 0).rgb * (0.5 * 0.25 * 1);

	// low weight samples
	blurredLight += u_srcImage.SampleLevel(u_sampler, texCoord + float2(-2, -2) * u_srcPixelSize, 0).rgb * (0.125 * 0.25 * 1);
	blurredLight += u_srcImage.SampleLevel(u_sampler, texCoord + float2( 0, -2) * u_srcPixelSize, 0).rgb * (0.125 * 0.25 * 2);
	blurredLight += u_srcImage.SampleLevel(u_sampler, texCoord + float2(+2, -2) * u_srcPixelSize, 0).rgb * (0.125 * 0.25 * 1);
	blurredLight += u_srcImage.SampleLevel(u_sampler, texCoord + float2(-2,  0) * u_srcPixelSize, 0).rgb * (0.125 * 0.25 * 2);
	blurredLight += u_srcImage.SampleLevel(u_sampler, texCoord + float2( 0,  0) * u_srcPixelSize, 0).rgb * (0.125 * 0.25 * 4);
	blurredLight += u_srcImage.SampleLevel(u_sampler, texCoord + float2(+2,  0) * u_srcPixelSize, 0).rgb * (0.125 * 0.25 * 2);
	blurredLight += u_srcImage.SampleLevel(u_sampler, texCoord + float2(-2, +2) * u_srcPixelSize, 0).rgb * (0.125 * 0.25 * 1);
	blurredLight += u_srcImage.SampleLevel(u_sampler, texCoord + float2( 0, +2) * u_srcPixelSize, 0).rgb * (0.125 * 0.25 * 2);
	blurredLight += u_srcImage.SampleLevel(u_sampler, texCoord + float2(+2, +2) * u_srcPixelSize, 0).rgb * (0.125 * 0.25 * 1);

	u_dstImage.Store(tid, float4(blurredLight, 1.0));
}