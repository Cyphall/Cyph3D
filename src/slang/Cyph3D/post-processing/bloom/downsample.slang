cbuffer Parameters
{
	Texture2D::Handle u_image;
	SamplerState::Handle u_sampler;
	float2 u_srcPixelSize;
};

struct FIn
{
	float4 rasterPosition : SV_Position;
	float2 texCoord;
};

struct FOut
{
	float4 color : SV_Target0;
};

[[shader("fragment")]]
FOut main(FIn input)
{
	float3 result = 0.0;

	// high weight samples
	result += u_image.SampleLevel(u_sampler, input.texCoord + float2(-1, -1) * u_srcPixelSize, 0).rgb * (0.5 * 0.25 * 1);
	result += u_image.SampleLevel(u_sampler, input.texCoord + float2(+1, -1) * u_srcPixelSize, 0).rgb * (0.5 * 0.25 * 1);
	result += u_image.SampleLevel(u_sampler, input.texCoord + float2(-1, +1) * u_srcPixelSize, 0).rgb * (0.5 * 0.25 * 1);
	result += u_image.SampleLevel(u_sampler, input.texCoord + float2(+1, +1) * u_srcPixelSize, 0).rgb * (0.5 * 0.25 * 1);

	// low weight samples
	result += u_image.SampleLevel(u_sampler, input.texCoord + float2(-2, -2) * u_srcPixelSize, 0).rgb * (0.125 * 0.25 * 1);
	result += u_image.SampleLevel(u_sampler, input.texCoord + float2( 0, -2) * u_srcPixelSize, 0).rgb * (0.125 * 0.25 * 2);
	result += u_image.SampleLevel(u_sampler, input.texCoord + float2(+2, -2) * u_srcPixelSize, 0).rgb * (0.125 * 0.25 * 1);
	result += u_image.SampleLevel(u_sampler, input.texCoord + float2(-2,  0) * u_srcPixelSize, 0).rgb * (0.125 * 0.25 * 2);
	result += u_image.SampleLevel(u_sampler, input.texCoord + float2( 0,  0) * u_srcPixelSize, 0).rgb * (0.125 * 0.25 * 4);
	result += u_image.SampleLevel(u_sampler, input.texCoord + float2(+2,  0) * u_srcPixelSize, 0).rgb * (0.125 * 0.25 * 2);
	result += u_image.SampleLevel(u_sampler, input.texCoord + float2(-2, +2) * u_srcPixelSize, 0).rgb * (0.125 * 0.25 * 1);
	result += u_image.SampleLevel(u_sampler, input.texCoord + float2( 0, +2) * u_srcPixelSize, 0).rgb * (0.125 * 0.25 * 2);
	result += u_image.SampleLevel(u_sampler, input.texCoord + float2(+2, +2) * u_srcPixelSize, 0).rgb * (0.125 * 0.25 * 1);

	FOut output;
	output.color = float4(result, 1);
	return output;
}