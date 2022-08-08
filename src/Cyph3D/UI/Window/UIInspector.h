#pragma once

#include <any>

class UIInspector
{
public:
	static void show();
	
	static const std::any& getSelected();
	static void setSelected(const std::any& selected);

private:
	static std::any _selected;
};