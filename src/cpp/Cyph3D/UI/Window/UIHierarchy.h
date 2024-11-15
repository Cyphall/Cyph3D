#pragma once

#include <functional>
#include <imgui.h>

class Transform;
class Entity;

class UIHierarchy
{
public:
	static void show();

private:
	static constexpr ImGuiTreeNodeFlags BASE_FLAGS = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding;

	static std::function<void()> _task;

	static void addRootToTree();
	static void addObjectToTree(Transform* transform);
};