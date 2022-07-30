#pragma once

#include <imgui.h>
#include <queue>
#include <optional>
#include "Cyph3D/Entity/Entity.h"

class Transform;

class UIHierarchy
{
public:
	static void show();

private:
	static constexpr ImGuiTreeNodeFlags BASE_FLAGS = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding;
	
	static std::optional<std::pair<Transform*, Transform*>> _entityToReparent;
	static Entity* _entityToDelete;
	static Entity* _entityToDuplicate;
	static bool _createEntityRequested;
	
	static void processHierarchyChanges();
	static void addRootToTree();
	static void addObjectToTree(Transform* transform);
};