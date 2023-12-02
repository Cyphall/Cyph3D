#include "UIHierarchy.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/Iterator/EntityIterator.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/Scene/Transform.h"
#include "Cyph3D/UI/Window/UIInspector.h"
#include "Cyph3D/Window.h"

#include <imgui.h>

std::function<void()> UIHierarchy::_task;

void UIHierarchy::show()
{
	if (ImGui::Begin("Hierarchy", nullptr))
	{
		// Hierarchy tree creation
		addRootToTree();

		// Main context menu to add elements to the scene
		if (ImGui::BeginPopupContextWindow("HierarchyAction"))
		{
			if (ImGui::MenuItem("Create Entity"))
			{
				_task = []()
				{
					Entity& created = Engine::getScene().createEntity(Engine::getScene().getRoot());
					UIInspector::setSelected(&created);
				};
			}

			IInspectable* selectedObject = UIInspector::getSelected();
			Entity* selectedEntity = dynamic_cast<Entity*>(selectedObject);

			if (ImGui::MenuItem("Delete Entity", nullptr, false, selectedEntity != nullptr))
			{
				_task = [selectedEntity]()
				{
					IInspectable* selected = UIInspector::getSelected();
					if (selected == selectedEntity)
					{
						UIInspector::setSelected(nullptr);
					}

					Scene& scene = Engine::getScene();

					auto it = scene.findEntity(*selectedEntity);
					if (it == scene.end())
					{
						throw;
					}
					scene.removeEntity(it);
				};
			}

			if (ImGui::MenuItem("Duplicate Entity", nullptr, false, selectedEntity != nullptr))
			{
				_task = [selectedEntity]()
				{
					selectedEntity->duplicate(*selectedEntity->getTransform().getParent());
				};
			}

			ImGui::EndPopup();
		}
	}

	if (_task)
	{
		_task();
		_task = {};
	}

	ImGui::End();
}

static void reparent(Transform& reparented, Transform& newParent)
{
	// Check if the new parent is not a child of the dragged Transform
	Transform* parent = &newParent;
	while ((parent = parent->getParent()) != nullptr)
	{
		if (parent == &reparented)
		{
			return;
		}
	}

	// Check if the new parent is not already the current parent
	if (&newParent == reparented.getParent())
	{
		return;
	}

	reparented.setParent(&newParent);
}

void UIHierarchy::addRootToTree()
{
	bool open = ImGui::TreeNodeEx(Engine::getScene().getName().c_str(), BASE_FLAGS | ImGuiTreeNodeFlags_Framed);

	// Make root a dragdrop target for hierarchy change
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HierarchyOrderChange"))
		{
			Transform* dropped = *static_cast<Transform**>(payload->Data);
			_task = [dropped]()
			{
				reparent(*dropped, Engine::getScene().getRoot());
			};
		}
		ImGui::EndDragDropTarget();
	}

	if (open)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 14 * Engine::getWindow().getPixelScale());
		// Add root children
		for (Transform* child : Engine::getScene().getRoot().getChildren())
		{
			addObjectToTree(child);
		}
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();

		ImGui::TreePop();
	}
}

void UIHierarchy::addObjectToTree(Transform* transform)
{
	ImGuiTreeNodeFlags flags = BASE_FLAGS;

	IInspectable* selected = UIInspector::getSelected();
	if (selected == transform->getOwner())
		flags |= ImGuiTreeNodeFlags_Selected;

	if (transform->getChildren().empty())
		flags |= ImGuiTreeNodeFlags_Leaf;

	bool open = ImGui::TreeNodeEx(transform, flags, "%s", transform->getOwner()->getName().c_str());

	// Select the item on click
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right))
	{
		UIInspector::setSelected(transform->getOwner());
	}

	// Make the item a drag drop source and target for hierarchy change
	if (ImGui::BeginDragDropSource())
	{
		ImGui::Text("%s", transform->getOwner()->getName().c_str());

		ImGui::SetDragDropPayload("HierarchyOrderChange", &transform, sizeof(Transform*));
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HierarchyOrderChange"))
		{
			Transform* dropped = *static_cast<Transform**>(payload->Data);
			_task = [dropped, transform]()
			{
				reparent(*dropped, *transform);
			};
		}
		ImGui::EndDragDropTarget();
	}

	// Draw item children if the item is opened
	if (open)
	{
		for (Transform* child : transform->getChildren())
		{
			addObjectToTree(child);
		}

		ImGui::TreePop();
	}
}