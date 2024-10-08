#pragma once

#include <string>

class StringHelper
{
public:
	static void remove(std::string& string, const char* stringToRemove);
	static std::string convert(const std::wstring& wstring);
	static std::wstring convert(const std::string& string);
};