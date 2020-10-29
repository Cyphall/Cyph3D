#include "StringHelper.h"

void remove(std::string& string, const char* stringToRemove)
{
	size_t pos = string.find(stringToRemove);
	if (pos != std::string::npos)
	{
		string.erase(pos, strlen(stringToRemove));
	}
}
