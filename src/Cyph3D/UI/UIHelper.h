#pragma once

#include <imgui.h>

struct ImGuiContext;
class Scene;

class UIHelper
{
public:
	static void init();
	static void render();
	static void shutdown();
	static void onNewFrame();

private:
	static ImGuiContext* _context;
	
	static bool _dockingLayoutInitialized;
	static void initDockingLayout(ImGuiID dockspaceId);
	static void initStyles();
};