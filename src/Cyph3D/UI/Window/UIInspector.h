#pragma once

#include <glm/glm.hpp>
#include <any>

class UIInspector
{
public:
	static void show();
	
	static std::any getSelected();
	static void setSelected(std::any selected);

private:
	static std::any _selected;
};