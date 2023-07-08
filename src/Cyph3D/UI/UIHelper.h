#pragma once

#include <imgui.h>
#include <memory>

#include "Cyph3D/VKObject/VKPtr.h"

struct ImGuiContext;
class UIAssetBrowser;
class ImGuiVulkanBackend;
template<typename T>
class VKDynamic;
class VKCommandBuffer;
class VKSemaphore;
class VKImageView;

class UIHelper
{
public:
	static void init();
	static const VKPtr<VKSemaphore>& render(const VKPtr<VKImageView>& destImageView, const VKPtr<VKSemaphore>& imageAvailableSemaphore);
	static void shutdown();
	static void onNewFrame();

private:
	static ImGuiContext* _context;
	
	static std::unique_ptr<UIAssetBrowser> _assetBrowser;
	static ImFont* _bigFont;
	
	static bool _dockingLayoutInitialized;
	
	static std::unique_ptr<ImGuiVulkanBackend> _vulkanBackend;
	
	static VKDynamic<VKSemaphore> _submitSemaphore;
	
	static void initDockingLayout(ImGuiID dockspaceId);
	static void initStyles();
	static void initFonts();
};