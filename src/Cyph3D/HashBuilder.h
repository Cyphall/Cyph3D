#pragma once

#include <cstddef>
#include <bit>

// Based on https://stackoverflow.com/a/50978188/9962106
class HashBuilder
{
public:
	template<typename T>
	HashBuilder& hash(const T& data)
	{
		_value = std::rotl(_value, std::numeric_limits<size_t>::digits/3) ^ distribute(std::hash<T>{}(data));
		return *this;
	}
	
	std::size_t get() const
	{
		return _value;
	}
	
private:
	template<typename T>
	inline static T xorshift(const T& n, int i)
	{
		return n^(n>>i);
	}
	
	inline static uint64_t distribute(const uint64_t& n)
	{
		uint64_t p = 0x5555555555555555ull; // pattern of alternating 0 and 1
		uint64_t c = 17316035218449499591ull; // random uneven integer constant;
		return c * xorshift(p * xorshift(n, 32), 32);
	}
	
	std::size_t _value = 0;
};