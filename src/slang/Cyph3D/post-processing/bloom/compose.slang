cbuffer Parameters
{
	Texture2D::Handle u_srcAImage;
	Texture2D::Handle u_srcBImage;
	SamplerState::Handle u_sampler;
	float u_factor;
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
	float3 a = u_srcAImage.SampleLevel(u_sampler, input.texCoord, 0).rgb;
	float3 b = u_srcBImage.SampleLevel(u_sampler, input.texCoord, 0).rgb;

	FOut output;
	output.color = float4(lerp(a, b, u_factor), 1);
	return output;
}