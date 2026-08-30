cbuffer Parameters
{
	uint2 u_size;
	Texture2D::Handle u_srcImage;
	WTexture2D::Handle u_dstImage;
	float u_exposure;
};

[[shader("compute")]]
[[numthreads(8, 8, 1)]]
void main(uint2 tid : SV_DispatchThreadID)
{
	if (any(tid >= u_size))
		return;

	float3 color = u_srcImage.Load(uint3(tid, 0)).rgb;

	color *= pow(2.0, u_exposure);

	u_dstImage.Store(tid, float4(color, 1.0));
}