#include "UIMisc.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/PerfCounter/PerfStep.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/UI/Window/UIViewport.h"
#include "Cyph3D/Window.h"
#include "Cyph3D/Helper/ImGuiHelper.h"
#include "Cyph3D/VKObject/VKContext.h"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>

glm::ivec2 UIMisc::_resolution(1920, 1080);

void UIMisc::show()
{
	if (ImGui::Begin("Misc", nullptr))
	{
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

		std::optional<std::string_view> newPath;
		if (ImGuiHelper::AssetInputWidget(Engine::getScene().getSkyboxPath(), "Skybox", "asset_skybox", newPath))
		{
			Engine::getScene().setSkyboxPath(newPath);
		}

		if (Engine::getScene().getSkybox() != nullptr)
		{
			float skyboxRotation = Engine::getScene().getSkyboxRotation();
			if (ImGui::SliderFloat("Skybox rotation", &skyboxRotation, 0, 360))
			{
				Engine::getScene().setSkyboxRotation(skyboxRotation);
			}
		}
		
		if (Engine::getVKContext().isRayTracingSupported())
		{
			ImGui::Separator();
			
			ImGui::InputInt2("Render Resolution", glm::value_ptr(_resolution));
			
			if (ImGui::Button("Render to file"))
			{
				UIViewport::renderToFile(_resolution);
			}
		}

		ImGui::Separator();

		const PerfStep* perfStep = UIViewport::getPreviousFramePerfStep();
		if (perfStep)
		{
			displayPerfStep(*perfStep);
		}
	}
	
	ImGui::End();
}

void UIMisc::displayPerfStep(const PerfStep& perfStep)
{
	if (!perfStep.getSubsteps().empty())
	{
		if (ImGui::TreeNodeEx(perfStep.getName().c_str(), 0, "%s: %.3fms", perfStep.getName().c_str(), perfStep.getDuration()))
		{
			for (const PerfStep& perfSubstep : perfStep.getSubsteps())
			{
				displayPerfStep(perfSubstep);
			}
			ImGui::TreePop();
		}
	}
	else
	{
		ImGui::TreeNodeEx(perfStep.getName().c_str(), ImGuiTreeNodeFlags_Leaf, "%s: %.3fms", perfStep.getName().c_str(), perfStep.getDuration());
		ImGui::TreePop();
	}
}