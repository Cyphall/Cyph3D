#include "StringHelper.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void StringHelper::remove(std::string& string, const char* stringToRemove)
{
	size_t pos = string.find(stringToRemove);
	if (pos != std::string::npos)
	{
		string.erase(pos, strlen(stringToRemove));
	}
}

std::string StringHelper::convert(const std::wstring& wstring)
{
	if (wstring.empty())
	{
		return {};
	}

	int requiredSize = WideCharToMultiByte(CP_UTF8, 0, wstring.data(), wstring.size(), nullptr, 0, nullptr, nullptr);
	std::string string(requiredSize, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstring.data(), wstring.size(), string.data(), string.size(), nullptr, nullptr);

	return string;
}

std::wstring StringHelper::convert(const std::string& string)
{
	if (string.empty())
	{
		return {};
	}

	int requiredSize = MultiByteToWideChar(CP_UTF8, 0, string.data(), string.size(), nullptr, 0);
	std::wstring wstring(requiredSize, 0);
	MultiByteToWideChar(CP_UTF8, 0, string.data(), string.size(), wstring.data(), wstring.size());

	return wstring;
}