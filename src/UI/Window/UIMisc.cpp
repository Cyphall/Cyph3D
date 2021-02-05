#include "UIMisc.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include "../../Rendering/Renderer.h"
#include "../../Engine.h"
#include "../../Window.h"
#include "../../Scene/Scene.h"
#include "../../Helper/FileHelper.h"
#include "UIInspector.h"
#include <filesystem>

bool UIMisc::_showDemoWindow = false;
std::vector<std::string> UIMisc::_scenes;
const std::string* UIMisc::_selectedScene = nullptr;

void UIMisc::init()
{
	rescanFiles();
}

void UIMisc::show()
{
	ImGui::SetNextWindowSize(glm::vec2(400, 300));
	ImGui::SetNextWindowPos(glm::vec2(Engine::getWindow().getSize().x - 400, 0));
	
	if (!ImGui::Begin("Misc", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
	{
		ImGui::End();
		return;
	}
	
	int fps = 1 / Engine::getTimer().deltaTime();
	ImGui::Text("FPS: %d", fps);
	
	bool gbufferDebug = Engine::getRenderer().getDebug();
	if (ImGui::Checkbox("GBuffer Debug View", &gbufferDebug))
	{
		Engine::getRenderer().setDebug(gbufferDebug);
	}
	
	ImGui::Checkbox("Show Demo Window", &_showDemoWindow);
	
	if (_showDemoWindow)
		ImGui::ShowDemoWindow();
	
	bool showRawQuaternion = UIInspector::getShowRawQuaternion();
	if (ImGui::Checkbox("Show Raw Quaternion", &showRawQuaternion))
	{
		UIInspector::setShowRawQuaternion(showRawQuaternion);
	}
	
	float cameraSpeed = Engine::getScene().getCamera().getSpeed();
	if (ImGui::SliderFloat("Camera speed", &cameraSpeed, 0, 10))
	{
		Engine::getScene().getCamera().setSpeed(cameraSpeed);
	}
	
	float exposure = Engine::getScene().getCamera().getExposure();
	if (ImGui::SliderFloat("Exposure", &exposure, 0, 10))
	{
		Engine::getScene().getCamera().setExposure(exposure);
	}
	
	ImGui::Separator();
	
	if (ImGui::BeginCombo("Scene", _selectedScene != nullptr ? _selectedScene->c_str() : ""))
	{
		for (const std::string& scene : _scenes)
		{
			bool selected = &scene == _selectedScene;
			if (ImGui::Selectable(scene.c_str(), selected))
			{
#pragma clang diagnostic push
#pragma ide diagnostic ignored "LocalValueEscapesScope"
				_selectedScene = &scene;
#pragma clang diagnostic pop
			}
			
			if (selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		
		ImGui::EndCombo();
	}
	
	ImGui::SameLine();
	
	if (ImGui::Button("Refresh"))
	{
		rescanFiles();
	}
	
	if (ImGui::Button("Load scene"))
	{
		if (_selectedScene != nullptr)
			Scene::load(*_selectedScene);
	}
	
	ImGui::SameLine();
	
	if (ImGui::Button("Save scene"))
	{
		Engine::getScene().save();
	}
	
	ImGui::Separator();
	
	Skybox* currentSkybox = Engine::getScene().getSkybox();
	const std::string& skyboxName = currentSkybox != nullptr ? currentSkybox->getName() : "None";
	
	// Field is read-only anyway, we can safely remove the const from skyboxName
	ImGui::InputText("Skybox", &const_cast<std::string&>(skyboxName), ImGuiInputTextFlags_ReadOnly);
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SkyboxDragDrop");
		if (payload)
		{
			Engine::getScene().setSkybox(Engine::getScene().getRM().requestSkybox(*(*static_cast<const std::string**>(payload->Data))));
		}
		ImGui::EndDragDropTarget();
	}
	
	if (Engine::getScene().getSkybox() != nullptr)
	{
		float skyboxRotation = Engine::getScene().getSkybox()->getRotation();
		if (ImGui::SliderFloat("Skybox rotation", &skyboxRotation, 0, 360))
		{
			Engine::getScene().getSkybox()->setRotation(skyboxRotation);
		}
	}
	
	
	ImGui::End();
}

void UIMisc::rescanFiles()
{
	_scenes.clear();
	
	for (const auto& entry : std::filesystem::directory_iterator("resources/scenes"))
	{
		if (entry.is_directory()) continue;
		
		_scenes.push_back(entry.path().stem().generic_string());
	}
	
	_selectedScene = !_scenes.empty() ? &_scenes.front() : nullptr;
}
