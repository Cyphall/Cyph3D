#version 460 core

layout(set = 0, binding = 0) uniform sampler2D u_colorTexture;

layout(location = 0) out vec4 o_color;

// https://iolite-engine.com/blog_posts/minimal_agx_implementation

// Mean error^2: 1.85907662e-06
vec3 agxDefaultContrastApprox(vec3 x)
{
	vec3 x2 = x * x;
	vec3 x4 = x2 * x2;
	vec3 x6 = x4 * x2;
	
	return - 17.86     * x6 * x
	       + 78.01     * x6
	       - 126.7     * x4 * x
	       + 92.06     * x4
	       - 28.72     * x2 * x
	       + 4.361     * x2
	       - 0.1718    * x
	       + 0.002857;
}

vec3 agx(vec3 val)
{
	const mat3 agx_mat = mat3(
		0.842479062253094,  0.0423282422610123, 0.0423756549057051,
		0.0784335999999992, 0.878468636469772,  0.0784336,
		0.0792237451477643, 0.0791661274605434, 0.879142973793104
	);
	
	const float min_ev = -12.47393f;
	const float max_ev = 4.026069f;
	
	// Input transform (inset)
	val = agx_mat * val;
	
	// Log2 space encoding
	val = clamp(log2(val), min_ev, max_ev);
	val = (val - min_ev) / (max_ev - min_ev);
	
	// Apply sigmoid function approximation
	val = agxDefaultContrastApprox(val);
	
	const mat3 agx_mat_inv = mat3(
		 1.19687900512017,   -0.0528968517574562, -0.0529716355144438,
		-0.0980208811401368,  1.15190312990417,   -0.0980434501171241,
		-0.0990297440797205, -0.0989611768448433,  1.15107367264116
	);
	
	// Inverse input transform (outset)
	val = agx_mat_inv * val;
	
	// sRGB IEC 61966-2-1 2.2 Exponent Reference EOTF Display
	// NOTE: We're linearizing the output here. Comment/adjust when
	// *not* using a sRGB render target
	val = pow(val, vec3(2.2));
	
	return val;
}

void main()
{
	vec3 color = texelFetch(u_colorTexture, ivec2(gl_FragCoord.xy), 0).rgb;
	
	color = agx(color);
	
	o_color = vec4(clamp(color, vec3(0.0), vec3(1.0)), 1);
}