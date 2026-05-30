#pragma once

#include <imgui.h>
#include <memory>

struct ImGuiContext;

namespace c3d
{
class UIAssetBrowser;
class ImGuiVulkanBackend;
template<typename T>
class VKDynamic;
class VKCommandBuffer;
class VKSemaphore;
class VKImage;

class UIHelper
{
public:
	static void init();
	static void render(const std::shared_ptr<VKImage>& destImage, const std::shared_ptr<VKSemaphore>& renderFinishedSemaphore);
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