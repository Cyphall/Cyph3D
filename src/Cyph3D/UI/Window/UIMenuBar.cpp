#include "UIMenuBar.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/UI/Window/UIViewport.h"

#include <imgui.h>
#include <filesystem>

bool UIMenuBar::_showDemoWindow = false;

static bool isResourcePath(const std::filesystem::path& path)
{
	std::filesystem::path resourcePathCanonial = std::filesystem::weakly_canonical(FileHelper::getResourcePath());
	std::filesystem::path pathCanonial = std::filesystem::weakly_canonical(std::filesystem::absolute(path));

	auto it = std::search(pathCanonial.begin(), pathCanonial.end(), resourcePathCanonial.begin(), resourcePathCanonial.end());

	return it == pathCanonial.begin();
}

void UIMenuBar::show()
{
	bool showPathOutsideError = false;
	
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
						.fileTypeExtensions = L"*.c3dscene"
					}
				}, "resources/scenes");
				
				if (filePath.has_value())
				{
					if (isResourcePath(filePath.value()))
					{
						Scene::load(filePath.value());
					}
					else
					{
						showPathOutsideError = true;
					}
				}
			}
			
			if (ImGui::MenuItem("Save Scene"))
			{
				Scene& scene = Engine::getScene();
				std::optional<std::filesystem::path> filePath = FileHelper::fileDialogSave({
					FileDialogFilter{
						.fileTypeDisplayName = L"Cyph3D Scene",
						.fileTypeExtensions = L"*.c3dscene"
					}
				}, "resources/scenes", scene.getName());
				
				if (filePath.has_value())
				{
					if (isResourcePath(filePath.value()))
					{
						scene.save(filePath.value());
					}
					else
					{
						showPathOutsideError = true;
					}
				}
			}
			ImGui::EndMenu();
		}
		
		if (ImGui::BeginMenu("Debug"))
		{
			ImGui::Checkbox("Show ImGui Demo Window", &_showDemoWindow);
			
			ImGui::EndMenu();
		}

		if (showPathOutsideError)
			ImGui::OpenPopup("Error###path_is_outside");

		if (ImGui::BeginPopupModal("Error###path_is_outside", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("The selected path is outside the \"resources\" folder.");
			
			ImGui::NewLine();
			ImGui::NewLine();

			ImGui::SameLine(0, ImGui::GetContentRegionAvail().x / 2.0f - 40);
			if (ImGui::Button("OK", ImVec2(80, 0)))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}
		
		ImGui::EndMainMenuBar();
	}
	
	if (_showDemoWindow)
		ImGui::ShowDemoWindow();
}