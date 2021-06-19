#include "UIHelper.h"
#include "Impl/imgui_impl_glfw.h"
#include "Impl/imgui_impl_opengl3.h"
#include "../Engine.h"
#include "../Scene/Scene.h"
#include "Window/UIMisc.h"
#include "Window/UIHierarchy.h"
#include "Window/UIInspector.h"
#include "Window/UIResourceExplorer.h"
#include <imgui.h>
#include <ImGuizmo.h>
#include <imgui_internal.h>
#include "../Window.h"
#include "Window/UIMenuBar.h"
#include "Window/UIViewport.h"

ImGuiContext* UIHelper::_context = nullptr;
bool UIHelper::_dockingLayoutInitialized = false;

void UIHelper::init()
{
	_context = ImGui::CreateContext();
	ImGui::SetCurrentContext(_context);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGuizmo::SetImGuiContext(_context);
	
	ImGui_ImplGlfw_InitForOpenGL(Engine::getWindow().getHandle(), true);
	ImGui_ImplOpenGL3_Init("#version 460 core");
	
	initStyles();
	
	UIResourceExplorer::init();
}

void UIHelper::render()
{
	ImGuiID dockspaceId = ImGui::DockSpaceOverViewport();
	
	if (!_dockingLayoutInitialized)
	{
		initDockingLayout(dockspaceId);
		_dockingLayoutInitialized = true;
	}
	
	UIViewport::show();
	UIMenuBar::show();
	UIMisc::show();
	UIHierarchy::show();
	UIInspector::show();
	UIResourceExplorer::show();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIHelper::shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	
	ImGui::DestroyContext(_context);
	_context = nullptr;
}

void UIHelper::onNewFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
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
	ImGui::DockBuilderSplitNode(inspectorId, ImGuiDir_Up, 0.16f, &miscId, &inspectorId);
	ImGui::DockBuilderDockWindow("Inspector", inspectorId);
	ImGui::DockBuilderDockWindow("Misc", miscId);
	
	ImGuiID hierarchyId;
	ImGui::DockBuilderSplitNode(remainingId, ImGuiDir_Left, panelWidth / remainingWidth, &hierarchyId, &remainingId);
	remainingWidth -= panelWidth;
	ImGui::DockBuilderDockWindow("Hierarchy", hierarchyId);
	
	ImGuiID resourcesId;
	ImGui::DockBuilderSplitNode(remainingId, ImGuiDir_Down, 0.3f, &resourcesId, &remainingId);
	ImGui::DockBuilderDockWindow("Resources", resourcesId);
	
	ImGui::DockBuilderDockWindow("Viewport", remainingId);
	
	ImGui::DockBuilderFinish(dockspaceId);
}

ImVec4 normalizeColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
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
	style.Colors[ImGuiCol_HeaderHovered] = normalizeColor(100, 100, 100, 255);
	style.Colors[ImGuiCol_HeaderActive] = normalizeColor(70, 70, 70, 255);
	
	style.Colors[ImGuiCol_ResizeGrip] = normalizeColor(50, 50, 50, 255);
	style.Colors[ImGuiCol_ResizeGripHovered] = normalizeColor(40, 40, 40, 255);
	style.Colors[ImGuiCol_ResizeGripActive] = normalizeColor(45, 45, 45, 255);
	
	style.Colors[ImGuiCol_Separator] = normalizeColor(110, 110, 110, 255);
	style.Colors[ImGuiCol_SeparatorHovered] = normalizeColor(130, 130, 130, 255);
	style.Colors[ImGuiCol_SeparatorActive] = normalizeColor(160, 160, 160, 255);
}
