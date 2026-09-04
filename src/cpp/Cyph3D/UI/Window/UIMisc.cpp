#include "UIMisc.h"

#include <Cyph3D/Asset/RuntimeAsset/SkyboxAsset.h>
#include <Cyph3D/Engine.h>
#include <Cyph3D/Helper/FileHelper.h>
#include <Cyph3D/Helper/ImGuiHelper.h>
#include <Cyph3D/Rendering/OfflineRenderer.h>
#include <Cyph3D/Scene/Scene.h>
#include <Cyph3D/UI/Window/UIViewport.h>
#include <Cyph3D/Window.h>

#include <CyphGPU/Device.hpp>
#include <CyphGPU/DeviceSession.hpp>
#include <CyphGPU/ImGuiBackend.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/component_wise.hpp>
#include <imgui.h>
#include <stb_image_write.h>

glm::ivec2 c3d::UIMisc::_resolution = {1920, 1080};
uint32_t c3d::UIMisc::_renderSampleCount = 1024;
bool c3d::UIMisc::_simulationEnabled = true;
int c3d::UIMisc::_viewportSampleCount = 8;
std::array<float, 512> c3d::UIMisc::_frametimes{};
uint32_t c3d::UIMisc::_lastFrametimeIndex = 0;
float c3d::UIMisc::_overlayFrametime = 0.0f;
float c3d::UIMisc::_timeUntilOverlayUpdate = 0.0f;
std::optional<c3d::UIMisc::RenderToFileData> c3d::UIMisc::_renderToFileData{};

void c3d::UIMisc::show()
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

		if (Engine::getDeviceSession()->getDevice()->getCapabilities() & cgpu::Device::Capability::eRayTracing)
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
				renderToFile(_resolution, _renderSampleCount);
			}

			ImGui::SetNextWindowPos(glm::vec2{ImGui::GetMainViewport()->WorkSize} / 2.0f, ImGuiCond_Appearing, {0.5f, 0.5f});
			if (ImGui::BeginPopupModal("Rendering status", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
			{
				auto& state = _renderToFileData->state;

				ImGui::TextUnformatted(state.finished ? "Rendering finished" : "Rendering in progress...");
				ImGui::ProgressBar(static_cast<float>(state.renderedSamples) / static_cast<float>(state.totalSamples));
				ImGui::TextUnformatted(std::format("Rendered samples: {}/{}", state.renderedSamples.load(), state.totalSamples.load()).c_str());

				auto duration = state.lastTraceTime.load() - state.startTime;
				auto durationRounded = std::chrono::floor<std::chrono::duration<long long, std::deci>>(duration);
				ImGui::Text("%s", std::format("Elapsed time: {:%H:%M:%S}", durationRounded).c_str());

				glm::vec2 targetPreviewExtent = glm::vec2{640.0f, 360.0f} * Engine::getWindow().getPixelScale();
				glm::vec2 previewExtent = _renderToFileData->state.extent;
				previewExtent /= glm::compMax(previewExtent / targetPreviewExtent);
				if (state.previewImage.load())
				{
					ImGui::Image(
						ImGui_ImplCyphGPU_ToTextureID(state.previewImage.load()),
						previewExtent,
						ImVec2(0, 0),
						ImVec2(1, 1)
					);
				}
				else
				{
					ImGui::GetWindowDrawList()->AddRectFilled(
						ImGui::GetCursorScreenPos(),
						glm::vec2{ImGui::GetCursorScreenPos()} + previewExtent,
						IM_COL32(0, 0, 0, 255)
					);
					ImGui::Dummy(previewExtent);
				}

				if (state.finished)
				{
					if (ImGui::Button("Close"))
					{
						ImGui::CloseCurrentPopup();
						_renderToFileData = std::nullopt;
					}
				}
				else
				{
					if (ImGui::Button("Finish now"))
					{
						state.forceFinish = true;
					}
				}

				ImGui::EndPopup();
			}
		}
	}

	ImGui::End();
}

bool c3d::UIMisc::isSimulationEnabled()
{
	return _simulationEnabled;
}

int c3d::UIMisc::viewportSampleCount()
{
	return _viewportSampleCount;
}

bool c3d::UIMisc::isRenderToFileInProgress()
{
	return _renderToFileData.has_value();
}

void c3d::UIMisc::displayFrametime()
{
	double deltaTime = Engine::getTimer().deltaTime();
	_timeUntilOverlayUpdate -= deltaTime;
	if (_timeUntilOverlayUpdate < 0.0f)
	{
		_overlayFrametime = deltaTime * 1000.0f;
		_timeUntilOverlayUpdate += 0.5f;
	}

	_frametimes[_lastFrametimeIndex] = deltaTime * 1000.0;
	_lastFrametimeIndex = (_lastFrametimeIndex + 1) % _frametimes.size();

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

void c3d::UIMisc::renderToFile(glm::uvec2 resolution, uint32_t sampleCount)
{
	std::optional<std::filesystem::path> filePath = FileHelper::fileDialogSave(
		{{
			{"PNG Image", "png"},
			{"JPG Image", "jpg"},
		}},
		".",
		"Render"
	);

	if (!filePath)
	{
		return;
	}

	_renderToFileData.emplace();
	_renderToFileData->state.totalSamples = sampleCount;
	_renderToFileData->state.extent = resolution;
	_renderToFileData->state.camera = UIViewport::getCamera();
	_renderToFileData->state.camera.setAspectRatio(static_cast<float>(resolution.x) / static_cast<float>(resolution.y));
	_renderToFileData->state.outputFile = *filePath;
	_renderToFileData->state.startTime = std::chrono::high_resolution_clock::now();

	Engine::getScene().onPreRender(_renderToFileData->state.registry, _renderToFileData->state.camera);

	_renderToFileData->thread = std::jthread{
		[](RenderToFileState* state) {
			cgpu::CommandContext cmdCtx{Engine::getDeviceSession()};
			OfflineRenderer renderer{state->extent, state->camera, state->registry};

			std::optional<cgpu::CommandRecorder::SubmitHandle> previousSubmit;
			std::optional<cgpu::CommandRecorder::SubmitHandle> currentSubmit;
			while (state->renderedSamples < state->totalSamples)
			{
				if (currentSubmit)
				{
					currentSubmit->waitFinished();
				}

				uint32_t samples = std::min(state->totalSamples - state->renderedSamples, 64u);

				{
					cgpu::CommandRecorder cmdRec = cmdCtx.createRecorder(Engine::getDeviceSession()->getAsyncComputeQueue());
					renderer.traceRays(cmdRec, samples);
					currentSubmit = cmdRec.submit();
				}

				cmdCtx.finish();

				state->renderedSamples += samples;
				state->lastTraceTime = std::chrono::high_resolution_clock::now();

				if (state->renderedSamples % 1024 == 0)
				{
					currentSubmit->waitFinished();
					cgpu::CommandRecorder cmdRec = cmdCtx.createRecorder(Engine::getDeviceSession()->getAsyncGraphicsQueue());
					state->previewImage = renderer.postProcess(cmdRec);
					cmdRec.submit();
				}

				if (state->forceFinish)
				{
					state->totalSamples = state->renderedSamples.load();
					break;
				}

				std::swap(currentSubmit, previousSubmit);
			}

			cgpu::BufferPtr stagingBuffer;
			{
				cgpu::CommandRecorder cmdRec = cmdCtx.createRecorder(Engine::getDeviceSession()->getAsyncGraphicsQueue());

				cgpu::ImagePtr renderImage = renderer.postProcess(cmdRec);

				cgpu::ImagePtr conversionImage = cgpu::Image::create(
					Engine::getDeviceSession(),
					{
						.name = "Render-to-file conversion image",
						.format = vk::Format::eR8G8B8A8Unorm,
						.extent = {state->extent, 1},
						.usages =
							vk::ImageUsageFlagBits::eTransferDst |
							vk::ImageUsageFlagBits::eTransferSrc,
					}
				);

				cmdRec.blit({
					.src_image = renderImage,
					.dst_image = conversionImage,
				});

				stagingBuffer = cgpu::Buffer::create(
					Engine::getDeviceSession(),
					{
						.name = "Render-to-file staging buffer",
						.size = conversionImage->calcByteSize({0, 1}, 1),
						.usages = vk::BufferUsageFlagBits2::eTransferDst,
						.memory_type = cgpu::MemoryType::eCPUCached,
					}
				);

				cmdRec.copyImageToBuffer({
					.src_image = conversionImage,
					.dst_buffer = stagingBuffer,
				});

				cmdRec.submit().waitFinished();
			}

			cmdCtx.finish();

			if (state->outputFile.extension() == ".png")
			{
				stbi_write_png(state->outputFile.generic_string().c_str(), state->extent.x, state->extent.y, 4, stagingBuffer->getHostPtr(), state->extent.x * 4);
			}
			else if (state->outputFile.extension() == ".jpg")
			{
				stbi_write_jpg(state->outputFile.generic_string().c_str(), state->extent.x, state->extent.y, 4, stagingBuffer->getHostPtr(), 95);
			}

			state->finished = true;
		},
		&_renderToFileData->state
	};

	ImGui::OpenPopup("Rendering status");
}
