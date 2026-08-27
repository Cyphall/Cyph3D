#include "../common/vertex types.slang"

struct GPUInstance
{
	float3x3 normalMatrix;
	PositionVertexData* positionVertexBuffer;
	MaterialVertexData* materialVertexBuffer;
	uint3* indexBuffer;
	Texture2D<float3>::Handle albedoImage;
	Texture2D<float2>::Handle normalImage;
	Texture2D<float>::Handle roughnessImage;
	Texture2D<float>::Handle metalnessImage;
	Texture2D<float>::Handle displacementImage;
	Texture2D<float>::Handle emissiveImage;
	float3 albedoValue;
	float roughnessValue;
	float metalnessValue;
	float emissiveScale;
};

cbuffer Parameters
{
	uint2 u_size;
	uint64_t u_tlas;
	GPUInstance* u_instanceBuffer;
	SamplerState::Handle u_sampler;
	SamplerState::Handle u_skyboxSampler;
	RWTexture2D<float4>::Handle u_lightImage;
	TextureCube<float3>::Handle u_skyboxImage;
	float3x3 u_skyboxRotation;
	float3 u_camPosition;
	float3 u_camRayTL;
	float3 u_camRayTR;
	float3 u_camRayBL;
	float3 u_camRayBR;
	uint32_t u_batchIndex;
	uint32_t u_sampleCount;
	bool u_resetAccumulation;
};

static const uint MAX_DEPTH = 4;

float4 getRandom(uint4 seed)
{
	uint4 v = seed;

	v = v * 1664525u + 1013904223u;
	v.x += v.y*v.w; v.y += v.z*v.x; v.z += v.x*v.y; v.w += v.y*v.z;
	v ^= v >> 16u;
	v.x += v.y*v.w; v.y += v.z*v.x; v.z += v.x*v.y; v.w += v.y*v.z;

	return clamp(v * (1.0 / float(0xffffffffu)), 0.0, 1.0);
}

float3 calcRandomHemisphereDirectionCosWeighted(float2 rand)
{
	float a = sqrt(rand.x);
	float b = float.getPi() * 2.0 * rand.y;

	return float3(
	a * cos(b),
	a * sin(b),
	sqrt(1.0 - rand.x)
	);
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float3 sampleGGXVNDF(float3 Ve, float2 alpha2D, float2 rand)
{
	// Section 3.2: transforming the view direction to the hemisphere configuration
	float3 Vh = normalize(float3(alpha2D.x * Ve.x, alpha2D.y * Ve.y, Ve.z));

	// Section 4.1: orthonormal basis (with special case if cross product is zero)
	float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
	float3 T1 = lensq > 0.0 ? float3(-Vh.y, Vh.x, 0.0) * rsqrt(lensq) : float3(1.0, 0.0, 0.0);
	float3 T2 = cross(Vh, T1);

	// Section 4.2: parameterization of the projected area
	float r = sqrt(rand.x);
	float phi = float.getPi() * 2.0 * rand.y;
	float t1 = r * cos(phi);
	float t2 = r * sin(phi);
	float s = 0.5 * (1.0 + Vh.z);
	t2 = lerp(sqrt(1.0 - t1 * t1), t2, s);

	// Section 4.3: reprojection onto hemisphere
	float3 Nh = t1 * T1 + t2 * T2 + sqrt(max(0.0, 1.0 - t1 * t1 - t2 * t2)) * Vh;

	// Section 3.4: transforming the normal back to the ellipsoid configuration
	return normalize(float3(alpha2D.x * Nh.x, alpha2D.y * Nh.y, max(0.0, Nh.z)));
}

float2 interpolateBarycentrics(float2 a, float2 b, float2 c, float3 barycentrics)
{
	return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

float3 interpolateBarycentrics(float3 a, float3 b, float3 c, float3 barycentrics)
{
	return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

void dielectricBRDF(float NdotL, out float3 diffuseWeight, out float3 specularWeight)
{
	float3 F0 = float3(0.04);
	specularWeight = fresnelSchlick(max(NdotL, 0.0), F0);
	diffuseWeight = 1.0 - specularWeight;
}

void conductorBRDF(float3 albedo, float NdotL, out float3 diffuseWeight, out float3 specularWeight)
{
	float3 F0 = albedo;
	specularWeight = fresnelSchlick(max(NdotL, 0.0), F0);
	diffuseWeight = float3(0);
}

float3 offsetRay(const float3 p, const float3 n)
{
	const float origin = 1.0 / 32.0;
	const float float_scale = 1.0 / 65536.0;
	const float int_scale = 256.0;

	int3 of_i = int3(n * int_scale);

	float3 p_i = float3(
		bit_cast<float>(bit_cast<int>(p.x) + ((p.x < 0) ? -of_i.x : of_i.x)),
		bit_cast<float>(bit_cast<int>(p.y) + ((p.y < 0) ? -of_i.y : of_i.y)),
		bit_cast<float>(bit_cast<int>(p.z) + ((p.z < 0) ? -of_i.z : of_i.z))
	);

	return float3(
		abs(p.x) < origin ? p.x+float_scale*n.x : p_i.x,
		abs(p.y) < origin ? p.y+float_scale*n.y : p_i.y,
		abs(p.z) < origin ? p.z+float_scale*n.z : p_i.z
	);
}

void handleHit(uint2 tid, RayQuery query, inout uint randomOffset, inout RayDesc ray, inout float3 light, inout float3 throughput)
{
	GPUInstance instance = u_instanceBuffer[query.CommittedInstanceIndex()];

	// flip normals if ray hit a back face
	float normalScale = query.CommittedTriangleFrontFace() ? 1.0 : -1.0;

	uint3 indices = instance.indexBuffer[query.CommittedPrimitiveIndex()];

	PositionVertexData pv1 = instance.positionVertexBuffer[indices.x];
	PositionVertexData pv2 = instance.positionVertexBuffer[indices.y];
	PositionVertexData pv3 = instance.positionVertexBuffer[indices.z];

	MaterialVertexData mv1 = instance.materialVertexBuffer[indices.x];
	MaterialVertexData mv2 = instance.materialVertexBuffer[indices.y];
	MaterialVertexData mv3 = instance.materialVertexBuffer[indices.z];

	float3 position1 = mul(query.CommittedObjectToWorld3x4(), float4(pv1.pos, 1.0));
	float3 position2 = mul(query.CommittedObjectToWorld3x4(), float4(pv2.pos, 1.0));
	float3 position3 = mul(query.CommittedObjectToWorld3x4(), float4(pv3.pos, 1.0));

	float3 normal1 = normalize(mul(instance.normalMatrix, mv1.normal));
	float3 normal2 = normalize(mul(instance.normalMatrix, mv2.normal));
	float3 normal3 = normalize(mul(instance.normalMatrix, mv3.normal));

	float3 tangent1 = normalize(mul(instance.normalMatrix, mv1.tangent.xyz));
	float3 tangent2 = normalize(mul(instance.normalMatrix, mv2.tangent.xyz));
	float3 tangent3 = normalize(mul(instance.normalMatrix, mv3.tangent.xyz));

	float3 bitangent1 = cross(normal1, tangent1) * mv1.tangent.w;
	float3 bitangent2 = cross(normal2, tangent2) * mv2.tangent.w;
	float3 bitangent3 = cross(normal3, tangent3) * mv3.tangent.w;

	float2 barycentricsXY = query.CommittedRayBarycentrics();
	float3 barycentrics = float3(1.0 - barycentricsXY.x - barycentricsXY.y, barycentricsXY.x, barycentricsXY.y);

	float3 position = interpolateBarycentrics(position1, position2, position3, barycentrics);
	float3 normal = normalize(interpolateBarycentrics(normal1, normal2, normal3, barycentrics)) * normalScale;
	float3 tangent = normalize(interpolateBarycentrics(tangent1, tangent2, tangent3, barycentrics)) * normalScale;
	float3 bitangent = normalize(interpolateBarycentrics(bitangent1, bitangent2, bitangent3, barycentrics)) * normalScale;
	float3 geometryNormal = normalize(cross(position3 - position2, position1 - position2));

	geometryNormal = dot(geometryNormal, ray.Direction) < 0.0 ? geometryNormal : -geometryNormal;

	float2 texCoord = interpolateBarycentrics(mv1.uv, mv2.uv, mv3.uv, barycentrics);

	float3 albedo = uint2(instance.albedoImage).x > 0 ? instance.albedoImage.SampleLevel(u_sampler, texCoord, 0.0) : instance.albedoValue;
	float roughness = uint2(instance.roughnessImage).x > 0 ? instance.roughnessImage.SampleLevel(u_sampler, texCoord, 0.0) : instance.roughnessValue;
	float metalness = uint2(instance.metalnessImage).x > 0 ? instance.metalnessImage.SampleLevel(u_sampler, texCoord, 0.0) : instance.metalnessValue;
	float emissive = (uint2(instance.emissiveImage).x > 0 ? instance.emissiveImage.SampleLevel(u_sampler, texCoord, 0.0) : 1.0) * instance.emissiveScale;

	float3 textureNormal = float3(0.0);
	textureNormal.xy = uint2(instance.normalImage).x > 0 ? instance.normalImage.SampleLevel(u_sampler, texCoord, 0.0) * 2.0 - 1.0 : float2(0.0, 0.0);
	textureNormal.z = sqrt(1.0 - min(dot(textureNormal.xy, textureNormal.xy), 1.0));

	normal = normalize(mul(transpose(float3x3(tangent, bitangent, normal)), textureNormal));
	tangent = normalize(tangent - normal * dot(tangent, normal));
	bitangent = cross(tangent, normal);

	float3x3 worldToTangent = float3x3(tangent, bitangent, normal);
	float3x3 tangentToWorld = transpose(worldToTangent);


	light += throughput * (albedo * emissive);


	ray.Origin = offsetRay(position, geometryNormal);

	float4 rand = getRandom(uint4(tid, u_batchIndex, randomOffset++));
	float3 localMicrofacetNormal;
	if (roughness > 0.0)
	{
		float3 localRayDir = mul(worldToTangent, ray.Direction);
		float alpha = roughness * roughness;
		float2 alpha2D = float2(alpha, alpha);
		localMicrofacetNormal = sampleGGXVNDF(-localRayDir, alpha2D, rand.xy);
	}
	else
	{
		localMicrofacetNormal = float3(0.0, 0.0, 1.0);
	}

	float3 microfacetNormal = normalize(mul(tangentToWorld, localMicrofacetNormal));

	float NdotL = max(dot(microfacetNormal, -ray.Direction), 0.0);

	float3 dielectricDiffuseWeight = float3(0);
	float3 dielectricSpecularWeight = float3(0);
	if (metalness < 1.0) dielectricBRDF(NdotL, dielectricDiffuseWeight, dielectricSpecularWeight);

	float3 conductorDiffuseWeight = float3(0);
	float3 conductorSpecularWeight = float3(0);
	if (metalness > 0.0) conductorBRDF(albedo, NdotL, conductorDiffuseWeight, conductorSpecularWeight);

	if (rand.z > 0.5)
	{
		// next bounce is specular

		ray.Direction = reflect(ray.Direction, microfacetNormal);

		float3 dielectric = dielectricSpecularWeight;
		float3 conductor  = conductorSpecularWeight;
		throughput *= lerp(dielectric, conductor, metalness) * 2.0;
	}
	else
	{
		// next bounce is diffuse

		ray.Direction = mul(tangentToWorld, calcRandomHemisphereDirectionCosWeighted(getRandom(uint4(tid, u_batchIndex, randomOffset++)).xy));

		float3 dielectric = albedo * dielectricDiffuseWeight;
		float3 conductor  = albedo * conductorDiffuseWeight;
		throughput *= lerp(dielectric, conductor, metalness) * 2.0;
	}
}

void handleMiss(RayDesc ray, inout float3 light, inout float3 throughput)
{
	if (uint2(u_skyboxImage).x != 0)
	{
		float3 skyboxDir = ray.Direction * float3(1, 1, -1);
		skyboxDir = mul(u_skyboxRotation, skyboxDir);
		float3 skyboxLight = u_skyboxImage.SampleLevel(u_skyboxSampler, skyboxDir, 0.0).rgb;
		light += float3(throughput * skyboxLight);
	}
}

[[shader("compute")]]
[[numthreads(8, 8, 1)]]
void main(uint3 threadID : SV_DispatchThreadID)
{
	uint2 tid = threadID.xy;
	if (any(tid >= u_size))
		return;

	uint randomOffset = 0;

	RaytracingAccelerationStructure tlas = {u_tlas};

	float3 light = 0.0;
	if (!u_resetAccumulation)
		light = u_lightImage.Load(tid).rgb;

	for (uint s = 0; s < u_sampleCount; s++)
	{
		RayDesc ray;
		ray.TMin = 0.0;
		ray.TMax = 1#INF;

		float3 throughput = 1.0;
		ray.Origin = u_camPosition;

		float2 PS_rayPos = float2(tid) + 0.5 + (getRandom(uint4(tid, u_batchIndex, randomOffset++)).xy - 0.5);
		float2 UVS_rayPos = PS_rayPos / float2(u_size);

		float3 topRayDir = lerp(u_camRayTL, u_camRayTR, UVS_rayPos.x);
		float3 bottomRayDir = lerp(u_camRayBL, u_camRayBR, UVS_rayPos.x);
		ray.Direction = normalize(lerp(topRayDir, bottomRayDir, UVS_rayPos.y));

		for (uint d = 0; d < MAX_DEPTH; d++)
		{
			RayQuery query;

			query.TraceRayInline(
				tlas,
				RAY_FLAG_FORCE_OPAQUE |
				RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES,
				0xFF,
				ray
			);

			while (query.Proceed()){}
			
			if (query.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
			{
				handleHit(tid, query, randomOffset, ray, light, throughput);
			}
			else
			{
				handleMiss(ray, light, throughput);
				break;
			}
		}
	}

	u_lightImage.Store(tid, float4(light, 1));
}