#pragma once

#include <vector>

class VectorHelper
{
public:
	template<typename T>
	static void removeAll(std::vector<T>& vector, const T& item)
	{
		vector.erase(
			std::remove(vector.begin(), vector.end(), item),
			vector.end()
		);
	}

	template<typename T>
	static void removeAt(std::vector<T>& vector, int index)
	{
		vector.erase(vector.begin() + index);
	}
};