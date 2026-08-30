#include "../../common/colorspace.slang"

cbuffer Parameters
{
	uint2 u_size;
	Texture2D::Handle u_srcImage;
	WTexture2D::Handle u_dstImage;
};

// https://iolite-engine.com/blog_posts/minimal_agx_implementation

// Mean error^2: 1.85907662e-06
float3 agxDefaultContrastApprox(float3 x)
{
	float3 x2 = x * x;
	float3 x4 = x2 * x2;
	float3 x6 = x4 * x2;

	return - 17.86     * x6 * x
	       + 78.01     * x6
	       - 126.7     * x4 * x
	       + 92.06     * x4
	       - 28.72     * x2 * x
	       + 4.361     * x2
	       - 0.1718    * x
	       + 0.002857;
}

float3 agx(float3 val)
{
	const float3x3 agx_mat = float3x3(
		0.842479062253094,  0.0784335999999992, 0.0792237451477643,
		0.0423282422610123, 0.878468636469772,  0.0791661274605434,
		0.0423756549057051, 0.0784336,          0.879142973793104,
	);

	const float min_ev = -12.47393f;
	const float max_ev = 4.026069f;

	// Input transform (inset)
	val = mul(agx_mat, val);

	// Log2 space encoding
	val = clamp(log2(val), min_ev, max_ev);
	val = (val - min_ev) / (max_ev - min_ev);

	// Apply sigmoid function approximation
	val = agxDefaultContrastApprox(val);

	const float3x3 agx_mat_inv = float3x3(
		 1.19687900512017,   -0.0980208811401368, -0.0990297440797205,
		-0.0528968517574562,  1.15190312990417,   -0.0989611768448433,
		-0.0529716355144438, -0.0980434501171241,  1.15107367264116,
	);

	// Inverse input transform (outset)
	val = mul(agx_mat_inv, val);

	// sRGB IEC 61966-2-1 2.2 Exponent Reference EOTF Display
	// NOTE: We're linearizing the output here. Comment/adjust when
	// *not* using a sRGB render target
	val = pow(val, float3(2.2));

	return val;
}

[[shader("compute")]]
[[numthreads(8, 8, 1)]]
void main(uint2 tid : SV_DispatchThreadID)
{
	if (any(tid >= u_size))
		return;

	float3 color = u_srcImage.Load(uint3(tid, 0)).rgb;

	color = agx(color);

	color = linearToSrgb(color);

	u_dstImage.Store(tid, float4(color, 1.0));
}