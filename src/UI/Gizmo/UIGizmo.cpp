#include "UIGizmo.h"
#include "../Window/UIInspector.h"
#include "../../Scene/Transform.h"
#include "../../Scene/Scene.h"
#include "../../Window.h"
#include "../../Engine.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

ImGuizmo::OPERATION UIGizmo::_operation = ImGuizmo::TRANSLATE;
ImGuizmo::MODE UIGizmo::_mode = ImGuizmo::LOCAL;

void UIGizmo::show()
{
	drawGizmo();
	
	ImGui::SetNextWindowPos(glm::vec2(400, 0));
	
	if (!ImGui::Begin("Gizmo Settings", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
	{
		ImGui::End();
		return;
	}
	
	if (ImGui::Button("T"))
	{
		_operation = ImGuizmo::TRANSLATE;
	}
	ImGui::SameLine();
	if (ImGui::Button("R"))
	{
		_operation = ImGuizmo::ROTATE;
	}
	ImGui::SameLine();
	if (ImGui::Button("S"))
	{
		_operation = ImGuizmo::SCALE;
	}
	ImGui::SameLine(0, 30);
	
	if (ImGui::Button(_mode == ImGuizmo::LOCAL ? "Local" : "Global"))
	{
		if (_mode == ImGuizmo::LOCAL)
		{
			_mode = ImGuizmo::WORLD;
		}
		else
		{
			_mode = ImGuizmo::LOCAL;
		}
		
	}
	
	ImGui::End();
}

void UIGizmo::drawGizmo()
{
	std::any selected = UIInspector::getSelected();
	if (!selected.has_value()) return;
	if (selected.type() != typeid(Transform*)) return;
	
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	
	Transform* transform = std::any_cast<Transform*>(selected);
	glm::mat4 worldModel = transform->getLocalToWorldMatrix();
	
	glm::mat4 view = Engine::getScene().getCamera().getView();
	glm::mat4 projection = Engine::getScene().getCamera().getProjection();
	
	bool changed = ImGuizmo::Manipulate(
		glm::value_ptr(view),
		glm::value_ptr(projection),
		_operation,
		_mode,
		glm::value_ptr(worldModel));
	
	if (changed)
	{
		glm::mat4 localModel = transform->getParent()->getWorldToLocalMatrix() * worldModel;
		
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;
		
		ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(localModel), glm::value_ptr(position), glm::value_ptr(rotation), glm::value_ptr(scale));
		
		if (position != transform->getLocalPosition())
		{
			transform->setLocalPosition(position);
		}
		else if (rotation != transform->getEulerLocalRotation())
		{
			transform->setEulerLocalRotation(rotation);
		}
		else if (scale != transform->getLocalScale())
		{
			transform->setLocalScale(scale);
		}
	}
}
