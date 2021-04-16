#pragma once

#include <imgui.h>
#include <ImGuizmo.h>

class UIGizmo
{
public:
	static void show();
	
private:
	static ImGuizmo::OPERATION _operation;
	static ImGuizmo::MODE _mode;
	
	static void drawGizmo();
};
