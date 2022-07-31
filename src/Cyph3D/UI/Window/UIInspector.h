#pragma once

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