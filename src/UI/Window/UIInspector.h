#pragma once

#include <any>
#include <glm/glm.hpp>

class UIInspector
{
public:
	static void show();
	
	static std::any getSelected();
	static void setSelected(std::any selected);

private:
	static std::any _selected;
	static bool _currentlyClicking;
	static glm::dvec2 _clickPos;
};
