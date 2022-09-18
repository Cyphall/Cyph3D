#pragma once

#include <imgui.h>
#include <memory>

struct ImGuiContext;
class UIAssetBrowser;

class UIHelper
{
public:
	static void init();
	static void render();
	static void shutdown();
	static void onNewFrame();

private:
	static ImGuiContext* _context;
	
	static std::unique_ptr<UIAssetBrowser> _assetBrowser;
	static ImFont* _bigFont;
	
	static bool _dockingLayoutInitialized;
	static void initDockingLayout(ImGuiID dockspaceId);
	static void initStyles();
	static void initFonts();
};