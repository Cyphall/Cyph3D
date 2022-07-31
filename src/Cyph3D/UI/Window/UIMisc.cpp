#include "UIMisc.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/PerfCounter/PerfStep.h"
#include "Cyph3D/ResourceManagement/Skybox.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/UI/Window/UIViewport.h"
#include "Cyph3D/Window.h"

#include <imgui.h>
#include <imgui_stdlib.h>

glm::ivec2 UIMisc::_resolution(1920, 1080);

void UIMisc::show()
{
	if (!ImGui::Begin("Misc", nullptr))
	{
		ImGui::End();
		return;
	}
	
	int fps = 1 / Engine::getTimer().deltaTime();
	ImGui::Text("FPS: %d", fps);
	
	float cameraSpeed = UIViewport::getCamera().getSpeed();;
	if (ImGui::SliderFloat("Camera speed", &cameraSpeed, 0, 10))
	{
		UIViewport::getCamera().setSpeed(cameraSpeed);
	}
	
	float exposure = UIViewport::getCamera().getExposure();
	if (ImGui::SliderFloat("Exposure", &exposure, -10, 10))
	{
		UIViewport::getCamera().setExposure(exposure);
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
	
	ImGui::Separator();
	
	ImGui::InputInt2("Render Resolution", glm::value_ptr(_resolution));
	
	if (ImGui::Button("Render to file"))
	{
		UIViewport::renderToFile(_resolution);
	}
	
	ImGui::Separator();
	
	const PerfStep* perfStep = UIViewport::getPreviousFramePerfStep();
	if (perfStep)
		displayPerfStep(*perfStep);
	
	ImGui::End();
}

void UIMisc::displayPerfStep(const PerfStep& perfStep)
{
	if (!perfStep.subSteps.empty())
	{
		if (ImGui::TreeNodeEx(perfStep.name, 0, "%s: %.3fms", perfStep.name, perfStep.durationInMs))
		{
			for (const PerfStep& perfSubstep : perfStep.subSteps)
			{
				displayPerfStep(perfSubstep);
			}
			ImGui::TreePop();
		}
	}
	else
	{
		ImGui::TreeNodeEx(perfStep.name, ImGuiTreeNodeFlags_Leaf, "%s: %.3fms", perfStep.name, perfStep.durationInMs);
		ImGui::TreePop();
	}
}