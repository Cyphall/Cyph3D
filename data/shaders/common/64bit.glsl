double uint64BitsToDouble(uint64_t value)
{
	return packDouble2x32(unpackUint2x32(value));
}

dvec2 uint64BitsToDouble(u64vec2 value)
{
	return dvec2(
	uint64BitsToDouble(value.x),
	uint64BitsToDouble(value.y)
	);
}

dvec3 uint64BitsToDouble(u64vec3 value)
{
	return dvec3(
	uint64BitsToDouble(value.x),
	uint64BitsToDouble(value.y),
	uint64BitsToDouble(value.z)
	);
}

dvec4 uint64BitsToDouble(u64vec4 value)
{
	return dvec4(
	uint64BitsToDouble(value.x),
	uint64BitsToDouble(value.y),
	uint64BitsToDouble(value.z),
	uint64BitsToDouble(value.w)
	);
}

uint64_t doubleBitsToUint64(double value)
{
	return packUint2x32(unpackDouble2x32(value));
}

u64vec2 doubleBitsToUint64(dvec2 value)
{
	return u64vec2(
	doubleBitsToUint64(value.x),
	doubleBitsToUint64(value.y)
	);
}

u64vec3 doubleBitsToUint64(dvec3 value)
{
	return u64vec3(
	doubleBitsToUint64(value.x),
	doubleBitsToUint64(value.y),
	doubleBitsToUint64(value.z)
	);
}

u64vec4 doubleBitsToUint64(dvec4 value)
{
	return u64vec4(
	doubleBitsToUint64(value.x),
	doubleBitsToUint64(value.y),
	doubleBitsToUint64(value.z),
	doubleBitsToUint64(value.w)
	);
}