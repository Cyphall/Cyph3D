#include "UIInspector.h"

#include <imgui.h>

IInspectable* UIInspector::_selected = nullptr;

IInspectable* UIInspector::getSelected()
{
	return _selected;
}

void UIInspector::setSelected(IInspectable* selected)
{
	_selected = selected;
}

void UIInspector::show()
{
	if (ImGui::Begin("Inspector", nullptr))
	{
		if (_selected != nullptr)
		{
			_selected->onDrawUi();
		}
	}
	
	ImGui::End();
}