#pragma once

#include "Cyph3D/VKObject/VKPtr.h"

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
	static const VKPtr<VKSemaphore>& render(const VKPtr<VKImage>& destImage, const VKPtr<VKSemaphore>& imageAvailableSemaphore);
	static void shutdown();
	static void onNewFrame();

private:
	static ImGuiContext* _context;

	static std::unique_ptr<UIAssetBrowser> _assetBrowser;
	static ImFont* _bigFont;

	static bool _dockingLayoutInitialized;

	static std::unique_ptr<ImGuiVulkanBackend> _vulkanBackend;

	static VKPtr<VKSemaphore> _presentSemaphore;

	static void initDockingLayout(ImGuiID dockspaceId);
	static void initStyles();
	static void initFonts();
};