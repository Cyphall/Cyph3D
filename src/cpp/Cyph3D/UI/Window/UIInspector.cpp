#include "UIInspector.h"

#include <imgui.h>

c3d::IInspectable* c3d::UIInspector::_selected = nullptr;

c3d::IInspectable* c3d::UIInspector::getSelected()
{
	return _selected;
}

void c3d::UIInspector::setSelected(IInspectable* selected)
{
	_selected = selected;
}

void c3d::UIInspector::show()
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