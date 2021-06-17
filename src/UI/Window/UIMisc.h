#pragma once

#include <vector>
#include <string>

class UIMisc
{
public:
	static void show();

private:
	static std::vector<std::string> _scenes;
	static const std::string* _selectedScene;
};
