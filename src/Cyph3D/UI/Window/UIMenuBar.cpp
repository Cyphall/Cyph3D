#include "UIMenuBar.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/UI/Window/UIViewport.h"

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
				UIViewport::setCamera(Camera());
			}
			
			if (ImGui::MenuItem("Open Scene"))
			{
				std::optional<std::filesystem::path> filePath = FileHelper::fileDialogOpen({
					FileDialogFilter{
						.fileTypeDisplayName = L"Cyph3D Scene",
						.fileTypeExtensions = L"*.c3ds"
					}
				}, "resources/scenes");
				
				if (filePath.has_value())
				{
					Scene::load(filePath.value());
				}
			}
			
			if (ImGui::MenuItem("Save Scene"))
			{
				Scene& scene = Engine::getScene();
				std::optional<std::filesystem::path> filePath = FileHelper::fileDialogSave({
					FileDialogFilter{
						.fileTypeDisplayName = L"Cyph3D Scene",
						.fileTypeExtensions = L"*.c3ds"
					}
				}, "resources/scenes", scene.getName());
				
				if (filePath.has_value())
				{
					scene.save(filePath.value());
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