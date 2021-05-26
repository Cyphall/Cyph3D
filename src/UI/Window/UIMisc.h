#pragma once

#include <vector>
#include <string>

class UIMisc
{
public:
	static void init();
	static void show();
	static void rescanFiles();

private:
	static std::vector<std::string> _scenes;
	static const std::string* _selectedScene;
};
