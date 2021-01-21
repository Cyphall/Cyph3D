#include "UIHierarchy.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include "../../Window.h"
#include "../../Scene/Scene.h"
#include "../../Scene/PointLight.h"
#include "../../Scene/DirectionalLight.h"
#include "../../Scene/MeshObject.h"
#include "../../Engine.h"
#include "UIInspector.h"

std::queue<std::tuple<Transform*, Transform*>> UIHierarchy::_hierarchyOrderChangeQueue;
std::queue<Transform*> UIHierarchy::_hierarchyDeleteQueue;
std::queue<ObjectType> UIHierarchy::_hierarchyAddQueue;

void UIHierarchy::show()
{
	ImGui::SetNextWindowSize(glm::vec2(400, Engine::getWindow().getSize().y / 2));
	ImGui::SetNextWindowPos(glm::vec2(0));
	
	if (!ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
	{
		ImGui::End();
		return;
	}
	
	// Main context menu to add elements to the scene
	if (ImGui::BeginPopupContextWindow("HierarchyAction"))
	{
		if (ImGui::BeginMenu("Create"))
		{
			if (ImGui::MenuItem("PointLight"))
			{
				_hierarchyAddQueue.push(PointLightType);
			}
			if (ImGui::MenuItem("DirectionalLight"))
			{
				_hierarchyAddQueue.push(DirectionalLightType);
			}
			if (ImGui::MenuItem("MeshObject"))
			{
				_hierarchyAddQueue.push(MeshObjectType);
			}
			ImGui::EndMenu();
		}
		
		std::any selected = UIInspector::getSelected();
		bool canDelete = selected.has_value() && selected.type() == typeid(Transform*);
		if (ImGui::MenuItem("Delete", nullptr, false, canDelete))
		{
			_hierarchyDeleteQueue.push(std::any_cast<Transform*>(selected));
		}
		
		ImGui::EndPopup();
	}
	
	//Hierarchy tree creation
	addRootToTree();
	
	processHierarchyChanges();
	
	
	ImGui::End();
}

void UIHierarchy::processHierarchyChanges()
{
	while (!_hierarchyOrderChangeQueue.empty())
	{
		auto [dragged, newParent] = _hierarchyOrderChangeQueue.front();
		_hierarchyOrderChangeQueue.pop();
		
		// Check if the new parent is not a child of the dragged Transform
		bool hierarchyLoop = false;
		Transform* parent = newParent;
		while ((parent = parent->getParent()) != nullptr)
		{
			if (parent == dragged)
			{
				hierarchyLoop = true;
				break;
			}
		}
		if (hierarchyLoop) continue;
		
		// Check if the new parent is not already the current parent
		if (newParent == dragged->getParent()) continue;
		
		dragged->setParent(newParent);
	}
	
	while (!_hierarchyDeleteQueue.empty())
	{
		SceneObject* sceneObjectToRemove = _hierarchyDeleteQueue.front()->getOwner();
		_hierarchyDeleteQueue.pop();
		
		std::any selected = UIInspector::getSelected();
		if (selected.has_value() && selected.type() == typeid(Transform*) && std::any_cast<Transform*>(selected) == &sceneObjectToRemove->getTransform())
		{
			UIInspector::setSelected(std::any());
		}
		Engine::getScene().remove(sceneObjectToRemove);
	}
	
	while (!_hierarchyAddQueue.empty())
	{
		ObjectType type = _hierarchyAddQueue.front();
		_hierarchyAddQueue.pop();
		
		switch (type)
		{
			case PointLightType:
				Engine::getScene().add(std::make_unique<PointLight>(Engine::getScene().getRoot(), "PointLight", glm::vec3(0), glm::vec3(0), glm::vec3(1)));
				break;
			case DirectionalLightType:
				Engine::getScene().add(std::make_unique<DirectionalLight>(Engine::getScene().getRoot(), "DirectionalLight", glm::vec3(0), glm::vec3(0), glm::vec3(1)));
				break;
			case MeshObjectType:
				Engine::getScene().add(std::make_unique<MeshObject>(Engine::getScene().getRoot(), Material::getDefault(), nullptr, "MeshObject", glm::vec3(0), glm::vec3(0), glm::vec3(1), glm::vec3(0), glm::vec3(0)));
				break;
		}
	}
}

void UIHierarchy::addRootToTree()
{
	bool open = ImGui::TreeNodeEx(Engine::getScene().getName().c_str(), BASE_FLAGS | ImGuiTreeNodeFlags_Framed);
	
	//Make root a dragdrop target for hierarchy change
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HierarchyOrderChange");
		if (payload)
		{
			Transform* dropped = *static_cast<Transform**>(payload->Data);
			_hierarchyOrderChangeQueue.emplace(dropped, Engine::getScene().getRoot());
		}
		ImGui::EndDragDropTarget();
	}
	
	if (open)
	{
		//Add root children
		for (Transform* child : Engine::getScene().getRoot()->getChildren())
		{
			addObjectToTree(child);
		}
		
		ImGui::TreePop();
	}
}

void UIHierarchy::addObjectToTree(Transform* transform)
{
	ImGuiTreeNodeFlags flags = BASE_FLAGS;
	
	std::any selected = UIInspector::getSelected();
	if (selected.has_value() && selected.type() == typeid(Transform*) && std::any_cast<Transform*>(selected) == transform)
		flags |= ImGuiTreeNodeFlags_Selected;
	
	if (transform->getChildren().empty())
		flags |= ImGuiTreeNodeFlags_Leaf;
	
	bool open = ImGui::TreeNodeEx(transform, flags, "%s", transform->getOwner()->getName().c_str());
	
	//Select the item on click
	if (ImGui::IsItemClicked())
	{
		UIInspector::setSelected(transform);
	}
	
	//Make the item a drag drop source and target for hierarchy change
	if (ImGui::BeginDragDropSource())
	{
		ImGui::Text("%s", transform->getOwner()->getName().c_str());
		
		ImGui::SetDragDropPayload("HierarchyOrderChange", &transform, sizeof(Transform*));
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HierarchyOrderChange");
		if (payload)
		{
			Transform* dropped = *static_cast<Transform**>(payload->Data);
			_hierarchyOrderChangeQueue.emplace(dropped, transform);
		}
		ImGui::EndDragDropTarget();
	}
	
	//Draw item children if the item is opened
	if (open)
	{
		for (Transform* child : transform->getChildren())
		{
			addObjectToTree(child);
		}
		
		ImGui::TreePop();
	}
}
