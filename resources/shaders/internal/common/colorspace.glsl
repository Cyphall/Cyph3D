vec3 srgbToLinear(vec3 color)
{
	bvec3 cutoff = lessThan(color, vec3(0.04045));
	vec3 higher = pow((color + vec3(0.055))/vec3(1.055), vec3(2.4));
	vec3 lower = color/vec3(12.92);

	return mix(higher, lower, cutoff);
}

vec3 linearToSrgb(vec3 color)
{
	bvec3 cutoff = lessThan(color, vec3(0.0031308));
	vec3 higher = vec3(1.055)*pow(color, vec3(1.0/2.4)) - vec3(0.055);
	vec3 lower = color * vec3(12.92);

	return mix(higher, lower, cutoff);
}

vec3 ST2084ToLinear(vec3 color)
{
	const float m1 = 2610.0 / 16384.0;
	const float m2 = 128.0 * (2523.0 / 4096.0);
	const float c1 = 3424.0 / 4096.0;
	const float c2 = 32.0 * (2413.0 / 4096.0);
	const float c3 = 32.0 * (2392.0 / 4096.0);

	vec3 E = clamp(color, vec3(0.0), vec3(1.0));
	vec3 EPower = pow(E, vec3(1.0 / m2));

	vec3 numerator = max(EPower - c1, vec3(0.0));
	vec3 denominator = c2 - c3 * EPower;

	return 10000.0 * pow(numerator / denominator, vec3(1.0 / m1));
}

vec3 linearToST2084(vec3 color)
{
	const float m1 = 2610.0 / 16384.0;
	const float m2 = 128.0 * (2523.0 / 4096.0);
	const float c1 = 3424.0 / 4096.0;
	const float c2 = 32.0 * (2413.0 / 4096.0);
	const float c3 = 32.0 * (2392.0 / 4096.0);

	vec3 Y = clamp(color / 10000.0, vec3(0.0), vec3(1.0));
	vec3 YPower = pow(Y, vec3(m1));

	vec3 numerator = c1 + c2 * YPower;
	vec3 denominator = 1.0 + c3 * YPower;

	return pow(numerator / denominator, vec3(m2));
}