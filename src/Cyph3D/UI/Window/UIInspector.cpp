#include "UIInspector.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Entity/Component/Animator.h"
#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/Rendering/Renderer/Renderer.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/Window.h"

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
			
			Entity* entity = dynamic_cast<Entity*>(_selected);
			if (entity != nullptr)
			{
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				float availableWidth = ImGui::GetWindowContentRegionWidth();
				float buttonWidth = std::min(180.0f, availableWidth);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ((availableWidth - buttonWidth) / 2));

				if (ImGui::Button("Add Component", ImVec2(buttonWidth, 0)))
					ImGui::OpenPopup("add_component");

				if (ImGui::BeginPopup("add_component"))
				{
					for (auto it = Entity::allocators_begin(); it != Entity::allocators_end(); it++)
					{
						if (ImGui::Selectable(it->first.c_str()))
						{
							it->second(*entity);
						}
					}
					ImGui::EndPopup();
				}
			}
		}
	}
	
	ImGui::End();
}