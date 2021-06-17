#include "UIMenuBar.h"
#include "../../Engine.h"
#include "../../Scene/Scene.h"
#include "../../Rendering/Renderer.h"
#include "../../Helper/FileHelper.h"
#include <imgui.h>
#include <filesystem>

bool UIMenuBar::_showDemoWindow = false;

void UIMenuBar::show()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Scene"))
			{
				Engine::setScene(std::make_unique<Scene>());
			}
			
			if (ImGui::MenuItem("Open Scene"))
			{
				std::optional<std::string> filePath = FileHelper::fileDialogOpen({
					FileDialogFilter{
						.fileTypeDisplayName = L"Cyph3D Scene",
						.fileTypeExtensions = L"*.json"
					}
				}, "resources/scenes");
				
				if (filePath.has_value())
				{
					Scene::load(std::filesystem::path(filePath.value()).filename().replace_extension().generic_string());
				}
			}
			
			if (ImGui::MenuItem("Save Scene"))
			{
				const Scene& scene = Engine::getScene();
				std::optional<std::string> filePath = FileHelper::fileDialogSave({
					FileDialogFilter{
						.fileTypeDisplayName = L"Cyph3D Scene",
						.fileTypeExtensions = L"*.json"
					}
				}, "resources/scenes", scene.getName());
				
				if (filePath.has_value())
				{
					scene.save(std::filesystem::path(filePath.value()).filename().replace_extension().generic_string());
				}
			}
			
			ImGui::EndMenu();
		}
		
		if (ImGui::BeginMenu("Debug"))
		{
			ImGui::Checkbox("Show ImGui Demo Window", &_showDemoWindow);
			
//			bool gbufferDebug = Engine::getRenderer().getDebug();
//			if (ImGui::Checkbox("Show GBuffer Debug View", &gbufferDebug))
//			{
//				Engine::getRenderer().setDebug(gbufferDebug);
//			}
			
			ImGui::EndMenu();
		}
		
		ImGui::EndMainMenuBar();
	}
	
	if (_showDemoWindow)
		ImGui::ShowDemoWindow();
}
