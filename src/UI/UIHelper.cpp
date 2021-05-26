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
	
	ImGui::StyleColorsDark();
	
	UIMisc::init();
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
	ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Right, 0.2f, &inspectorId, &remainingId);
	ImGuiID miscId;
	ImGui::DockBuilderSplitNode(inspectorId, ImGuiDir_Up, 0.25f, &miscId, &inspectorId);
	ImGui::DockBuilderDockWindow("Inspector", inspectorId);
	ImGui::DockBuilderDockWindow("Misc", miscId);
	
	ImGuiID resourcesId;
	ImGui::DockBuilderSplitNode(remainingId, ImGuiDir_Down, 0.35f, &resourcesId, &remainingId);
	ImGui::DockBuilderDockWindow("Resources", resourcesId);
	
	ImGuiID hierarchyId;
	ImGui::DockBuilderSplitNode(remainingId, ImGuiDir_Left, 0.25f, &hierarchyId, &remainingId);
	ImGui::DockBuilderDockWindow("Hierarchy", hierarchyId);
	
	ImGui::DockBuilderDockWindow("Viewport", remainingId);
	
	ImGui::DockBuilderFinish(dockspaceId);
}
