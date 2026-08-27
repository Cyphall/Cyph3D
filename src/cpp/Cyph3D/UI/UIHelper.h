#pragma once

#include <CyphGPU/fwd.hpp>
#include <imgui.h>
#include <memory>

struct ImGuiContext;

namespace c3d
{
class UIAssetBrowser;

class UIHelper
{
public:
	static void init();
	static void render(cgpu::CommandContext& commandContext, const cgpu::ImagePtr& destImage);
	static void shutdown();
	static void onNewFrame();

private:
	static ImGuiContext* _context;

	static std::unique_ptr<UIAssetBrowser> _assetBrowser;

	static bool _dockingLayoutInitialized;

	static void initDockingLayout(ImGuiID dockspaceId);
	static void initStyles();
	static void initFonts();
};
}
