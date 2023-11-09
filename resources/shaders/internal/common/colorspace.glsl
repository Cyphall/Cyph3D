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