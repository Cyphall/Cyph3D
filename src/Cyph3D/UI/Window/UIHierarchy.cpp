#include "UIHierarchy.h"
#include <imgui.h>
#include "../../Window.h"
#include "../../Scene/Scene.h"
#include "../../Engine.h"
#include "UIInspector.h"

std::optional<std::pair<Transform*, Transform*>> UIHierarchy::_entityToReparent;
Entity* UIHierarchy::_entityToDelete = nullptr;
Entity* UIHierarchy::_entityToDuplicate = nullptr;
bool UIHierarchy::_createEntityRequested = false;

void UIHierarchy::show()
{
	if (!ImGui::Begin("Hierarchy", nullptr))
	{
		ImGui::End();
		return;
	}
	
	// Main context menu to add elements to the scene
	if (ImGui::BeginPopupContextWindow("HierarchyAction"))
	{
		if (ImGui::MenuItem("Create Entity"))
		{
			_createEntityRequested = true;
		}
		
		std::any selected = UIInspector::getSelected();
		bool entitySelected = selected.has_value() && selected.type() == typeid(Transform*);
		
		if (ImGui::MenuItem("Delete Entity", nullptr, false, entitySelected))
		{
			_entityToDelete = std::any_cast<Transform*>(selected)->getOwner();
		}
		
		if (ImGui::MenuItem("Duplicate Entity", nullptr, false, entitySelected))
		{
			_entityToDuplicate = std::any_cast<Transform*>(selected)->getOwner();
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
	Scene& scene = Engine::getScene();
	
	if (_entityToReparent.has_value())
	{
		auto [dragged, newParent] = _entityToReparent.value();
		
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
		
		// Check if the new parent is not a child of the dragged Transform
		if (!hierarchyLoop)
		{
			// Check if the new parent is not already the current parent
			if (newParent != dragged->getParent())
			{
				dragged->setParent(newParent);
			}
		}
		
		_entityToReparent.reset();
	}
	
	if (_entityToDelete != nullptr)
	{
		std::any selected = UIInspector::getSelected();
		if (selected.has_value() && selected.type() == typeid(Transform*) && std::any_cast<Transform*>(selected) == &_entityToDelete->getTransform())
		{
			UIInspector::setSelected(std::any());
		}
		
		for (auto it = scene.entities_begin(); it != scene.entities_end(); it++)
		{
			if (&(*it) == _entityToDelete)
			{
				scene.removeEntity(it);
				break;
			}
		}
		
		_entityToDelete = nullptr;
	}
	
	if (_entityToDuplicate != nullptr)
	{
		_entityToDuplicate->duplicate(*_entityToDuplicate->getTransform().getParent());
		
		_entityToDuplicate = nullptr;
	}
	
	if (_createEntityRequested)
	{
		Entity& created = Engine::getScene().createEntity(Engine::getScene().getRoot());
		UIInspector::setSelected(&created.getTransform());
		_createEntityRequested = false;
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
			_entityToReparent = std::make_optional<std::pair<Transform*, Transform*>>(dropped, &Engine::getScene().getRoot());
		}
		ImGui::EndDragDropTarget();
	}
	
	if (open)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 16);
		//Add root children
		for (Transform* child : Engine::getScene().getRoot().getChildren())
		{
			addObjectToTree(child);
		}
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		
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
			_entityToReparent = std::make_optional<std::pair<Transform*, Transform*>>(dropped, transform);
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
