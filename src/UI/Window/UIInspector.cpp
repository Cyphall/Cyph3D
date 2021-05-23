#include "UIInspector.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include "../../Window.h"
#include "../../Scene/Scene.h"
#include "../../Rendering/Renderer.h"
#include "../../Engine.h"
#include "../../Entity/Entity.h"
#include "../../Entity/Component/Animator.h"

std::any UIInspector::_selected;
bool UIInspector::_currentlyClicking = false;
glm::dvec2 UIInspector::_clickPos;

std::any UIInspector::getSelected()
{
	return _selected;
}

void UIInspector::setSelected(std::any selected)
{
	_selected = selected;
}

void UIInspector::show()
{
	ImGui::SetNextWindowSize(glm::vec2(400, Engine::getWindow().getSize().y - Engine::getWindow().getSize().y / 2));
	ImGui::SetNextWindowPos(glm::vec2(0, Engine::getWindow().getSize().y / 2));
	
	if (!ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
	{
		ImGui::End();
		return;
	}
	
	if (_selected.has_value())
	{
		if (_selected.type() == typeid(Transform*))
		{
			Entity& entity = *std::any_cast<Transform*>(_selected)->getOwner();
			entity.onDrawUi();
			
			ImGui::Separator();
			
			if (ImGui::Button("Add"))
				ImGui::OpenPopup("add_component");
			if (ImGui::BeginPopup("add_component"))
			{
				for (auto it = Entity::allocators_begin(); it != Entity::allocators_end(); it++)
				{
					if (ImGui::Selectable(it->first.c_str()))
					{
						it->second(entity);
					}
				}
				ImGui::EndPopup();
			}
		}
	}
	
	ImGui::End();
	
	if (!_currentlyClicking && Engine::getWindow().getMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !ImGui::GetIO().WantCaptureMouse)
	{
		_currentlyClicking = true;
		_clickPos = Engine::getWindow().getCursorPos();
	}
	
	if (_currentlyClicking && Engine::getWindow().getMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		_currentlyClicking = false;
		if (glm::distance(_clickPos, Engine::getWindow().getCursorPos()) < 5)
		{
			Entity* entity = Engine::getRenderer().getClickedEntity(_clickPos);
			setSelected(entity ? &entity->getTransform() : std::any());
		}
	}
}
