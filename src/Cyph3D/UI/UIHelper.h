#pragma once


#include <imgui.h>
#include <memory>

struct ImGuiContext;
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
	static const std::shared_ptr<VKSemaphore>& render(const std::shared_ptr<VKImage>& destImage, const std::shared_ptr<VKSemaphore>& imageAvailableSemaphore);
	static void shutdown();
	static void onNewFrame();

private:
	static ImGuiContext* _context;

	static std::unique_ptr<UIAssetBrowser> _assetBrowser;
	static ImFont* _bigFont;

	static bool _dockingLayoutInitialized;

	static std::unique_ptr<ImGuiVulkanBackend> _vulkanBackend;

	static std::shared_ptr<VKSemaphore> _presentSemaphore;

	static void initDockingLayout(ImGuiID dockspaceId);
	static void initStyles();
	static void initFonts();
};