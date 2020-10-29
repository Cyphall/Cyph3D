#pragma once

#include <imgui.h>
#include <queue>
#include "../../Enums/ObjectType.h"

class Transform;

class UIHierarchy
{
public:
	static void show();

private:
	static constexpr ImGuiTreeNodeFlags BASE_FLAGS = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
	
	static std::queue<std::tuple<Transform*, Transform*>> _hierarchyOrderChangeQueue;
	static std::queue<Transform*> _hierarchyDeleteQueue;
	static std::queue<ObjectType> _hierarchyAddQueue;
	
	static void processHierarchyChanges();
	static void addRootToTree();
	static void addObjectToTree(Transform* transform);
};
