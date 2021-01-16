#include "UIHelper.h"
#include "Impl/imgui_impl_glfw.h"
#include "Impl/imgui_impl_opengl3.h"
#include "../Engine.h"
#include "Window/UIMisc.h"
#include "Window/UIHierarchy.h"
#include "Window/UIInspector.h"
#include "Window/UIResourceExplorer.h"
#include <imgui.h>
#include "../Window.h"

ImGuiContext* UIHelper::_context = nullptr;

void UIHelper::init()
{
	_context = ImGui::CreateContext();
	ImGui::SetCurrentContext(_context);
	
	ImGui_ImplGlfw_InitForOpenGL(Engine::getWindow().getHandle(), true);
	ImGui_ImplOpenGL3_Init("#version 460 core");
	
	ImGui::StyleColorsDark();
	
	UIMisc::init();
	UIResourceExplorer::init();
}

void UIHelper::render()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIHelper::update(double deltaTime)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	
	if (!Engine::getWindow().isGuiOpen()) return;
	
	UIMisc::show(deltaTime);
	UIHierarchy::show();
	UIInspector::show();
	UIResourceExplorer::show();
}

void UIHelper::shutdown()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	
	ImGui::DestroyContext(_context);
	_context = nullptr;
}
