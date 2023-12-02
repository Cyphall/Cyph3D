#include "UIMisc.h"

#include "Cyph3D/Asset/RuntimeAsset/SkyboxAsset.h"
#include "Cyph3D/Engine.h"
#include "Cyph3D/Helper/ImGuiHelper.h"
#include "Cyph3D/PerfCounter/PerfStep.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/UI/Window/UIViewport.h"
#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/Window.h"

#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

glm::ivec2 UIMisc::_resolution(1920, 1080);
uint32_t UIMisc::_renderSampleCount = 1024;
bool UIMisc::_simulationEnabled = true;
int UIMisc::_viewportSampleCount = 8;
std::array<float, 512> UIMisc::_frametimes{};
uint32_t UIMisc::_lastFrametimeIndex = 0;
float UIMisc::_overlayFrametime = 0.0f;
float UIMisc::_timeUntilOverlayUpdate = 0.0f;

void UIMisc::show()
{
	if (ImGui::Begin("Misc", nullptr))
	{
		displayFrametime();

		ImGui::Separator();

		float cameraSpeed = UIViewport::getCamera().getSpeed();
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
		SkyboxAsset* skybox = Engine::getScene().getSkybox();
		if (ImGuiHelper::AssetInputWidget(skybox ? &skybox->getSignature().path : nullptr, "Skybox", "asset_skybox", newPath))
		{
			Engine::getScene().setSkybox(newPath);
		}

		if (Engine::getScene().getSkybox() != nullptr)
		{
			float skyboxRotation = Engine::getScene().getSkyboxRotation();
			if (ImGui::SliderFloat("Skybox rotation", &skyboxRotation, 0, 360))
			{
				Engine::getScene().setSkyboxRotation(skyboxRotation);
			}
		}

		ImGui::Separator();

		ImGui::Checkbox("Simulate", &_simulationEnabled);

		if (Engine::getVKContext().isRayTracingSupported())
		{
			ImGui::Separator();

			ImGui::SliderInt("Viewport Sample Count", &_viewportSampleCount, 1, 256);

			ImGui::Separator();

			ImGui::InputInt2("Render Resolution", glm::value_ptr(_resolution));

			uint32_t step = 1;
			uint32_t stepFast = 128;
			if (ImGui::InputScalar("Render Sample Count", ImGuiDataType_U32, &_renderSampleCount, &step, &stepFast, "%u"))
			{
				_renderSampleCount = std::max(_renderSampleCount, 1u);
			}

			if (ImGui::Button("Render to file"))
			{
				UIViewport::renderToFile(_resolution, _renderSampleCount);
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

bool UIMisc::isSimulationEnabled()
{
	return _simulationEnabled;
}

int UIMisc::viewportSampleCount()
{
	return _viewportSampleCount;
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

void UIMisc::displayFrametime()
{
	double deltaTime = Engine::getTimer().deltaTime();
	_timeUntilOverlayUpdate -= deltaTime;
	if (_timeUntilOverlayUpdate < 0.0f)
	{
		_overlayFrametime = deltaTime * 1000.0f;
		_timeUntilOverlayUpdate += 0.5f;
	}

	_lastFrametimeIndex = (_lastFrametimeIndex + 1) % _frametimes.size();
	_frametimes[_lastFrametimeIndex] = deltaTime * 1000.0;

	ImGuiStyle& style = ImGui::GetStyle();
	std::string overlay = std::format("{:.1f} ms", _overlayFrametime);
	ImGui::PlotLines(
		"Frametime",
		_frametimes.data(),
		_frametimes.size(),
		_lastFrametimeIndex,
		overlay.c_str(),
		0.0f,
		1000.0f / 30.0f,
		{0, (ImGui::GetFontSize() + style.FramePadding.y * 2.0f) * 3.0f}
	);
}
