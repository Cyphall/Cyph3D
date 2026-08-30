cbuffer Parameters
{
	uint2 u_size;
	RWTexture2D::Handle u_image;
	float u_exposure;
};

[[shader("compute")]]
[[numthreads(8, 8, 1)]]
void main(uint2 tid : SV_DispatchThreadID)
{
	if (any(tid >= u_size))
		return;

	float3 color = u_image.Load(tid).rgb;

	color *= pow(2.0, u_exposure);

	u_image.Store(tid, float4(color, 1.0));
}