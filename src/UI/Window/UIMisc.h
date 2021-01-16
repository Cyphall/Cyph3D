#pragma once

#include <vector>
#include <string>

class UIMisc
{
public:
	static void init();
	static void show(double deltaTime);
	static void rescanFiles();

private:
	static bool _showDemoWindow;

	static std::vector<std::string> _scenes;
	static const std::string* _selectedScene;
};
