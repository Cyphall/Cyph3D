#pragma once

float3 srgbToLinear(float3 color)
{
	bool3 cutoff = color < float3(0.04045);
	float3 lower = color/float3(12.92);
	float3 higher = pow((color + float3(0.055))/float3(1.055), float3(2.4));

	return select(cutoff, lower, higher);
}

float3 linearToSrgb(float3 color)
{
	bool3 cutoff = color < float3(0.0031308);
	float3 lower = color * float3(12.92);
	float3 higher = float3(1.055)*pow(color, float3(1.0/2.4)) - float3(0.055);

	return select(cutoff, lower, higher);
}

float3 ST2084ToLinear(float3 color)
{
	const float m1 = 2610.0 / 16384.0;
	const float m2 = 128.0 * (2523.0 / 4096.0);
	const float c1 = 3424.0 / 4096.0;
	const float c2 = 32.0 * (2413.0 / 4096.0);
	const float c3 = 32.0 * (2392.0 / 4096.0);

	float3 E = clamp(color, float3(0.0), float3(1.0));
	float3 EPower = pow(E, float3(1.0 / m2));

	float3 numerator = max(EPower - c1, float3(0.0));
	float3 denominator = c2 - c3 * EPower;

	return 10000.0 * pow(numerator / denominator, float3(1.0 / m1));
}

float3 linearToST2084(float3 color)
{
	const float m1 = 2610.0 / 16384.0;
	const float m2 = 128.0 * (2523.0 / 4096.0);
	const float c1 = 3424.0 / 4096.0;
	const float c2 = 32.0 * (2413.0 / 4096.0);
	const float c3 = 32.0 * (2392.0 / 4096.0);

	float3 Y = clamp(color / 10000.0, float3(0.0), float3(1.0));
	float3 YPower = pow(Y, float3(m1));

	float3 numerator = c1 + c2 * YPower;
	float3 denominator = 1.0 + c3 * YPower;

	return pow(numerator / denominator, float3(m2));
}