cbuffer Parameters
{
	Texture2D::Handle u_image;
	float u_exposure;
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
	float3 color = u_image.Load(uint3(uint2(input.rasterPosition.xy), 0)).rgb;

	color *= pow(2, u_exposure);

	FOut output;
	output.color = float4(color, 1);
	return output;
}