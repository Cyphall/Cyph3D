#pragma once

#include <type_traits>

enum class LogColorFlags
{
	FOREGROUND_BLUE      = 0x01,
	FOREGROUND_GREEN     = 0x02,
	FOREGROUND_RED       = 0x04,
	FOREGROUND_INTENSITY = 0x08,
	BACKGROUND_BLUE      = 0x10,
	BACKGROUND_GREEN     = 0x20,
	BACKGROUND_RED       = 0x40,
	BACKGROUND_INTENSITY = 0x80,
};

inline LogColorFlags operator|(LogColorFlags lhs, LogColorFlags rhs)
{
	using T = std::underlying_type_t<LogColorFlags>;
	return static_cast<LogColorFlags>(static_cast<T>(lhs) | static_cast<T>(rhs));
}