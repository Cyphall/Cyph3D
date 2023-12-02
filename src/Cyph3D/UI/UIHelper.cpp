#include "UIHelper.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/UI/ImGuiVulkanBackend.h"
#include "Cyph3D/UI/Window/UIAssetBrowser.h"
#include "Cyph3D/UI/Window/UIHierarchy.h"
#include "Cyph3D/UI/Window/UIInspector.h"
#include "Cyph3D/UI/Window/UIMenuBar.h"
#include "Cyph3D/UI/Window/UIMisc.h"
#include "Cyph3D/UI/Window/UIViewport.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Queue/VKQueue.h"
#include "Cyph3D/VKObject/Semaphore/VKSemaphore.h"
#include "Cyph3D/VKObject/VKDynamic.h"
#include "Cyph3D/Window.h"

#include <ImGuizmo.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>

ImGuiContext* UIHelper::_context = nullptr;

std::unique_ptr<UIAssetBrowser> UIHelper::_assetBrowser;
ImFont* UIHelper::_bigFont = nullptr;

bool UIHelper::_dockingLayoutInitialized = false;

std::unique_ptr<ImGuiVulkanBackend> UIHelper::_vulkanBackend;

VKPtr<VKSemaphore> UIHelper::_presentSemaphore;
VKPtr<VKSemaphore> UIHelper::_nextSubmitSemaphore;

bool UIHelper::_firstFrame = true;

void UIHelper::init()
{
	_context = ImGui::CreateContext();
	ImGui::SetCurrentContext(_context);
	ImGuizmo::SetImGuiContext(_context);

	ImGuiIO& io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui_ImplGlfw_InitForVulkan(Engine::getWindow().getHandle(), true);

	initStyles();
	initFonts();

	_assetBrowser = std::make_unique<UIAssetBrowser>(_bigFont);

	_vulkanBackend = std::make_unique<ImGuiVulkanBackend>();

	UIViewport::init();

	vk::SemaphoreCreateInfo semaphoreCreateInfo;
	_presentSemaphore = VKSemaphore::create(Engine::getVKContext(), semaphoreCreateInfo);
	_nextSubmitSemaphore = VKSemaphore::create(Engine::getVKContext(), semaphoreCreateInfo);
}

const VKPtr<VKSemaphore>& UIHelper::render(const VKPtr<VKImage>& destImage, const VKPtr<VKSemaphore>& imageAvailableSemaphore)
{
	ImGuiID dockspaceId = ImGui::DockSpaceOverViewport();

	if (!_dockingLayoutInitialized)
	{
		initDockingLayout(dockspaceId);
		_dockingLayoutInitialized = true;
	}

	const VKPtr<VKCommandBuffer>& commandBuffer = Engine::getVKContext().getDefaultCommandBuffer();

	commandBuffer->begin();

	UIViewport::show();
	UIMenuBar::show();
	if (!UIViewport::isFullscreen())
	{
		UIMisc::show();
		UIHierarchy::show();
		UIInspector::show();
		_assetBrowser->draw();
	}

	ImGui::Render();

	commandBuffer->imageMemoryBarrier(
		destImage,
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::ImageLayout::eColorAttachmentOptimal);

	_vulkanBackend->renderDrawData(ImGui::GetDrawData(), commandBuffer, destImage);

	commandBuffer->imageMemoryBarrier(
		destImage,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::ImageLayout::ePresentSrcKHR);

	commandBuffer->end();

	if (_firstFrame)
	{
		Engine::getVKContext().getMainQueue().submit(
			commandBuffer,
			{imageAvailableSemaphore},
			{_presentSemaphore, _nextSubmitSemaphore});

		_firstFrame = false;
	}
	else
	{
		Engine::getVKContext().getMainQueue().submit(
			commandBuffer,
			{imageAvailableSemaphore, _nextSubmitSemaphore},
			{_presentSemaphore, _nextSubmitSemaphore});
	}

	return _presentSemaphore;
}

void UIHelper::shutdown()
{
	_presentSemaphore = {};
	_nextSubmitSemaphore = {};

	UIViewport::shutdown();
	_vulkanBackend.reset();
	ImGui_ImplGlfw_Shutdown();

	ImGui::DestroyContext(_context);
	_context = nullptr;
}

void UIHelper::onNewFrame()
{
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
}

void UIHelper::initDockingLayout(ImGuiID dockspaceId)
{
	ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
	ImVec2 dockspaceSize = ImGui::GetMainViewport()->WorkSize;
	ImGui::DockBuilderSetNodeSize(dockspaceId, dockspaceSize);

	ImGuiID remainingId;

	ImGuiID inspectorId;
	float remainingWidth = 1920;
	float panelWidth = 380;
	ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Right, panelWidth / remainingWidth, &inspectorId, &remainingId);
	remainingWidth -= panelWidth;

	ImGuiID miscId;
	ImGui::DockBuilderSplitNode(inspectorId, ImGuiDir_Up, 0.31f, &miscId, &inspectorId);
	ImGui::DockBuilderDockWindow("Inspector", inspectorId);
	ImGui::DockBuilderDockWindow("Misc", miscId);

	ImGuiID hierarchyId;
	ImGui::DockBuilderSplitNode(remainingId, ImGuiDir_Left, panelWidth / remainingWidth, &hierarchyId, &remainingId);
	remainingWidth -= panelWidth;
	ImGui::DockBuilderDockWindow("Hierarchy", hierarchyId);

	ImGuiID resourcesId;
	ImGui::DockBuilderSplitNode(remainingId, ImGuiDir_Down, 0.3f, &resourcesId, &remainingId);
	ImGui::DockBuilderDockWindow("Asset Browser", resourcesId);

	ImGui::DockBuilderDockWindow("Viewport", remainingId);

	ImGui::DockBuilderFinish(dockspaceId);
}

static ImVec4 normalizeColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
}

void UIHelper::initStyles()
{
	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();

	style.FrameBorderSize = 1;

	style.FramePadding = ImVec2(5, 4);

	style.FrameRounding = 2;
	style.WindowRounding = 3;
	style.GrabRounding = 3;

	style.WindowMenuButtonPosition = ImGuiDir_None;

	style.HoverStationaryDelay = 1.0f;
	style.HoverFlagsForTooltipMouse = ImGuiHoveredFlags_Stationary;

	style.Colors[ImGuiCol_WindowBg] = normalizeColor(56, 56, 56, 255);

	style.Colors[ImGuiCol_FrameBg] = normalizeColor(42, 42, 42, 255);
	style.Colors[ImGuiCol_FrameBgHovered] = normalizeColor(55, 55, 55, 255);
	style.Colors[ImGuiCol_FrameBgActive] = normalizeColor(65, 65, 65, 255);

	style.Colors[ImGuiCol_Border] = normalizeColor(30, 30, 30, 255);

	style.Colors[ImGuiCol_TitleBg] = normalizeColor(40, 40, 40, 255);
	style.Colors[ImGuiCol_TitleBgActive] = normalizeColor(40, 40, 40, 255);

	style.Colors[ImGuiCol_CheckMark] = normalizeColor(230, 230, 230, 255);

	style.Colors[ImGuiCol_Button] = normalizeColor(88, 88, 88, 255);
	style.Colors[ImGuiCol_ButtonHovered] = normalizeColor(110, 110, 110, 255);
	style.Colors[ImGuiCol_ButtonActive] = normalizeColor(53, 53, 53, 255);

	style.Colors[ImGuiCol_Header] = normalizeColor(90, 90, 90, 255);
	style.Colors[ImGuiCol_HeaderHovered] = normalizeColor(110, 110, 110, 255);
	style.Colors[ImGuiCol_HeaderActive] = normalizeColor(70, 70, 70, 255);

	style.Colors[ImGuiCol_ResizeGrip] = normalizeColor(50, 50, 50, 255);
	style.Colors[ImGuiCol_ResizeGripHovered] = normalizeColor(40, 40, 40, 255);
	style.Colors[ImGuiCol_ResizeGripActive] = normalizeColor(45, 45, 45, 255);

	style.Colors[ImGuiCol_Separator] = normalizeColor(110, 110, 110, 255);
	style.Colors[ImGuiCol_SeparatorHovered] = normalizeColor(130, 130, 130, 255);
	style.Colors[ImGuiCol_SeparatorActive] = normalizeColor(160, 160, 160, 255);

	float pixelScale = Engine::getWindow().getPixelScale();
	style.ScaleAllSizes(pixelScale);
}

void UIHelper::initFonts()
{
	ImGuiIO& io = ImGui::GetIO();

	float pixelScale = Engine::getWindow().getPixelScale();

	ImFontConfig config;

	io.Fonts->AddFontFromFileTTF("resources/fonts/Roboto-Regular.ttf", 14.0f * pixelScale, &config, io.Fonts->GetGlyphRangesDefault());

	config.MergeMode = true;
	config.GlyphOffset = ImVec2(0.0f, 1.0f * pixelScale);

	static const ImWchar smallIconRange[] = {
		0xF00D, 0xF00D,
		0xF021, 0xF021,
		0xF062, 0xF062,
		0xF07B, 0xF07B,
		0};
	io.Fonts->AddFontFromFileTTF("resources/fonts/Font Awesome 6 Free-Solid-900.otf", 14.0f * pixelScale, &config, smallIconRange);

	io.Fonts->Build();

	static const ImWchar largeIconRange[] = {
		0xE209, 0xE209,
		0xE52f, 0xE52f,
		0xF03E, 0xF03E,
		0xF07B, 0xF07B,
		0xF15B, 0xF15B,
		0xF1B2, 0xF1B2,
		0xF43C, 0xF43C,
		0};
	_bigFont = io.Fonts->AddFontFromFileTTF("resources/fonts/Font Awesome 6 Free-Solid-900.otf", 48.0f * pixelScale, nullptr, largeIconRange);
}