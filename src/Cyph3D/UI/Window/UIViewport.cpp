#include "UIViewport.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/Rendering/SceneRenderer/PathTracingSceneRenderer.h"
#include "Cyph3D/Rendering/SceneRenderer/RasterizationSceneRenderer.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/UI/Window/UIInspector.h"
#include "Cyph3D/UI/Window/UIMisc.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/Window.h"

#include <chrono>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui_internal.h>
#include <magic_enum.hpp>
#include <stb_image_write.h>

enum class RenderToFileStatus
{
	eRendering,
	eRenderFinished,
	eSaveFinished
};

struct UIViewport::RenderToFileData
{
	uint32_t renderedSamples = 0;
	uint32_t totalSamples = 0;
	std::unique_ptr<PathTracingSceneRenderer> renderer;
	Camera camera;
	RenderRegistry registry;
	std::filesystem::path outputFile;
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> lastBatchTime;
	VKPtr<VKImage> lastRenderedTexture;
	RenderToFileStatus status = RenderToFileStatus::eRendering;
};

std::unique_ptr<SceneRenderer> UIViewport::_sceneRenderer;
UIViewport::RendererType UIViewport::_sceneRendererType = UIViewport::RendererType::Rasterization;
uint64_t UIViewport::_sceneChangeVersion = -1;

glm::uvec2 UIViewport::_previousViewportSize = {0, 0};

Camera UIViewport::_camera;
bool UIViewport::_cameraFocused = false;
bool UIViewport::_cameraChanged = true;
glm::vec2 UIViewport::_lockedCursorPos;

bool UIViewport::_fullscreen = false;

bool UIViewport::_leftClickPressedOnViewport = false;
glm::vec2 UIViewport::_leftClickPressPos;

ImGuizmo::OPERATION UIViewport::_gizmoMode = ImGuizmo::TRANSLATE;
ImGuizmo::MODE UIViewport::_gizmoSpace = ImGuizmo::LOCAL;

RenderRegistry UIViewport::_renderRegistry;

std::unique_ptr<ObjectPicker> UIViewport::_objectPicker;

std::unique_ptr<UIViewport::RenderToFileData> UIViewport::_renderToFileData;
bool UIViewport::_showRenderToFilePopup = false;
VKPtr<VKImage> UIViewport::_lastViewportImage;

void UIViewport::show()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	bool open = ImGui::Begin("Viewport");

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();

	if (open)
	{
		drawHeader();

		if (_showRenderToFilePopup)
		{
			ImGui::OpenPopup("Rendering status");
			_showRenderToFilePopup = false;
		}

		if (_renderToFileData)
		{
			drawRenderToFilePopup();
		}

		Window& window = Engine::getWindow();

		if (_cameraFocused && window.getMouseButtonState(GLFW_MOUSE_BUTTON_RIGHT) == Window::MouseButtonState::Released)
		{
			_cameraFocused = false;
			window.setInputMode(GLFW_CURSOR_NORMAL);
			window.setCursorPos(_lockedCursorPos);
		}

		// all of this need to be calculated before calling ImGui::Image()
		glm::ivec2 viewportStartLocal = glm::vec2(ImGui::GetCursorPos());
		glm::ivec2 viewportEndLocal = glm::vec2(ImGui::GetWindowContentRegionMax());

		glm::uvec2 viewportSize = glm::max(viewportEndLocal - viewportStartLocal, glm::ivec2(0));

		glm::ivec2 viewportStartGlobal = glm::vec2(ImGui::GetCursorScreenPos());
		glm::ivec2 viewportEndGlobal = viewportStartGlobal + glm::ivec2(viewportSize);

		if (viewportSize.x > 0 && viewportSize.y > 0)
		{
			if (!_sceneRenderer)
			{
				switch (_sceneRendererType)
				{
				case RendererType::Rasterization:
					_sceneRenderer = std::make_unique<RasterizationSceneRenderer>(viewportSize);
					break;
				case RendererType::PathTracing:
					_sceneRenderer = std::make_unique<PathTracingSceneRenderer>(viewportSize);
					break;
				}
			}

			bool cameraChanged = _cameraChanged;
			_cameraChanged = false;

			if (_previousViewportSize != viewportSize)
			{
				if (_sceneRenderer->getSize() != viewportSize)
				{
					_sceneRenderer->resize(viewportSize);
				}

				_camera.setAspectRatio(static_cast<float>(viewportSize.x) / static_cast<float>(viewportSize.y));
				cameraChanged = true;
			}

			if (_cameraFocused)
			{
				cameraChanged |= _camera.update(window.getCursorPos() - _lockedCursorPos);
				window.setCursorPos(_lockedCursorPos);
			}

			if (PathTracingSceneRenderer* pathTracingSceneRenderer = dynamic_cast<PathTracingSceneRenderer*>(_sceneRenderer.get()))
			{
				pathTracingSceneRenderer->setSampleCountPerRender(UIMisc::viewportSampleCount());
			}

			if (_renderToFileData && _renderToFileData->status == RenderToFileStatus::eRendering)
			{
				uint32_t remainingSamples = _renderToFileData->totalSamples - _renderToFileData->renderedSamples;
				uint32_t thisBatchSamples = std::min(remainingSamples, 16u);

				_renderToFileData->renderer->setSampleCountPerRender(thisBatchSamples);

				Engine::getVKContext().executeImmediate(
					[&](const VKPtr<VKCommandBuffer>& commandBuffer)
					{
						_renderToFileData->lastRenderedTexture = _renderToFileData->renderer->render(commandBuffer, _renderToFileData->camera, _renderToFileData->registry, false, false);
					}
				);

				_renderToFileData->renderedSamples += thisBatchSamples;

				_renderToFileData->lastBatchTime = std::chrono::high_resolution_clock::now();

				if (_renderToFileData->renderedSamples == _renderToFileData->totalSamples)
				{
					_renderToFileData->status = RenderToFileStatus::eRenderFinished;
				}
			}

			if (_renderToFileData && _renderToFileData->status == RenderToFileStatus::eRenderFinished)
			{
				if (_renderToFileData->lastRenderedTexture)
				{
					VKBufferInfo bufferInfo(_renderToFileData->lastRenderedTexture->getLevelByteSize(0), vk::BufferUsageFlagBits::eTransferDst);
					bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
					bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
					bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCached);

					VKPtr<VKBuffer<std::byte>> stagingBuffer = VKBuffer<std::byte>::create(Engine::getVKContext(), bufferInfo);

					Engine::getVKContext().executeImmediate(
						[&](const VKPtr<VKCommandBuffer>& commandBuffer)
						{
							commandBuffer->imageMemoryBarrier(
								_renderToFileData->lastRenderedTexture,
								vk::PipelineStageFlagBits2::eColorAttachmentOutput,
								vk::AccessFlagBits2::eColorAttachmentWrite,
								vk::PipelineStageFlagBits2::eCopy,
								vk::AccessFlagBits2::eTransferRead,
								vk::ImageLayout::eTransferSrcOptimal
							);

							commandBuffer->copyImageToBuffer(_renderToFileData->lastRenderedTexture, 0, 0, stagingBuffer, 0);
						}
					);

					glm::ivec2 textureSize = _renderToFileData->lastRenderedTexture->getSize(0);

					if (_renderToFileData->outputFile.extension() == ".png")
					{
						stbi_write_png(_renderToFileData->outputFile.generic_string().c_str(), textureSize.x, textureSize.y, 4, stagingBuffer->getHostPointer(), textureSize.x * 4);
					}
					else if (_renderToFileData->outputFile.extension() == ".jpg")
					{
						stbi_write_jpg(_renderToFileData->outputFile.generic_string().c_str(), textureSize.x, textureSize.y, 4, stagingBuffer->getHostPointer(), 95);
					}
				}
				else
				{
					Logger::error("Could not save render to file, no rendered image has been created yet");
				}

				_renderToFileData->status = RenderToFileStatus::eSaveFinished;
			}

			if (!_renderToFileData)
			{
				uint64_t currentSceneChangeVersion = Scene::getChangeVersion();
				bool sceneChanged = currentSceneChangeVersion != _sceneChangeVersion;

				if (sceneChanged)
				{
					_renderRegistry.clear();
					Engine::getScene().onPreRender(_renderRegistry, _camera);
				}

				_lastViewportImage = _sceneRenderer->render(Engine::getVKContext().getDefaultCommandBuffer(), _camera, _renderRegistry, sceneChanged, cameraChanged);

				_sceneChangeVersion = currentSceneChangeVersion;
			}

			ImGui::Image(
				&_lastViewportImage,
				glm::vec2(_lastViewportImage->getSize(0)),
				ImVec2(0, 0),
				ImVec2(1, 1)
			);

			if (!_renderToFileData)
			{
				drawGizmo(viewportStartGlobal, viewportSize);
			}

			if (window.getMouseButtonState(GLFW_MOUSE_BUTTON_RIGHT) == Window::MouseButtonState::Clicked && ImGui::IsItemHovered())
			{
				_cameraFocused = true;
				_lockedCursorPos = window.getCursorPos();
				window.setInputMode(GLFW_CURSOR_DISABLED);

				_leftClickPressedOnViewport = false;
			}

			glm::vec2 viewportCursorPos = window.getCursorPos() - glm::vec2(viewportStartGlobal);

			if (!_cameraFocused && window.getMouseButtonState(GLFW_MOUSE_BUTTON_LEFT) == Window::MouseButtonState::Clicked && ImGui::IsItemHovered())
			{
				_leftClickPressedOnViewport = true;
				_leftClickPressPos = viewportCursorPos;
			}

			if (_leftClickPressedOnViewport && window.getMouseButtonState(GLFW_MOUSE_BUTTON_LEFT) == Window::MouseButtonState::Released)
			{
				_leftClickPressedOnViewport = false;
				if (ImGui::IsItemHovered() && glm::distance(_leftClickPressPos, viewportCursorPos) < 5.0f)
				{
					RenderRegistry renderRegistry;
					Engine::getScene().onPreRender(renderRegistry, _camera);

					Entity* clickedEntity = _objectPicker->getPickedEntity(_camera, renderRegistry, viewportSize, glm::uvec2(viewportCursorPos));
					UIInspector::setSelected(clickedEntity);
				}
			}
		}

		_previousViewportSize = viewportSize;
	}

	ImGui::End();
}

Camera& UIViewport::getCamera()
{
	return _camera;
}

void UIViewport::setCamera(const Camera& camera)
{
	_camera = camera;
	_camera.setAspectRatio(static_cast<float>(_previousViewportSize.x) / static_cast<float>(_previousViewportSize.y));
	_cameraChanged = true;
}

void UIViewport::drawGizmo(glm::vec2 viewportStart, glm::vec2 viewportSize)
{
	IInspectable* selected = UIInspector::getSelected();
	if (selected == nullptr)
		return;

	Entity* entity = dynamic_cast<Entity*>(selected);
	if (entity == nullptr)
		return;

	ImGuizmo::SetRect(viewportStart.x, viewportStart.y, viewportSize.x, viewportSize.y);

	Transform& transform = entity->getTransform();
	glm::mat4 localToWorld = transform.getLocalToWorldMatrix();

	glm::mat4 view = _camera.getView();
	glm::mat4 projection = _camera.getProjection();
	projection[1][1] *= -1;

	ImGui::PushClipRect(viewportStart, viewportStart + viewportSize, false);

	ImGuizmo::SetDrawlist();
	bool changed = ImGuizmo::Manipulate(
		glm::value_ptr(view),
		glm::value_ptr(projection),
		_gizmoMode,
		_gizmoSpace,
		glm::value_ptr(localToWorld)
	);

	ImGui::PopClipRect();

	if (changed)
	{
		glm::mat4 localToParent = transform.getParent()->getWorldToLocalMatrix() * localToWorld;

		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;

		ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(localToParent), glm::value_ptr(position), glm::value_ptr(rotation), glm::value_ptr(scale));

		transform.setLocalPosition(position);
		transform.setEulerLocalRotation(rotation);
		transform.setLocalScale(scale);
	}
}

void UIViewport::drawHeader()
{
	float pixelScale = Engine::getWindow().getPixelScale();

	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::BeginChild("ViewportHeader", ImVec2(0, ImGui::GetFontSize() + style.FramePadding.y * 2.0f + style.WindowPadding.y * 2.0f), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoScrollbar);

	ImGui::GetCurrentWindow()->DC.LayoutType = ImGuiLayoutType_Horizontal;

	// gizmos
	if (ImGui::Button("T"))
	{
		_gizmoMode = ImGuizmo::TRANSLATE;
	}

	if (ImGui::Button("R"))
	{
		_gizmoMode = ImGuizmo::ROTATE;
	}

	if (ImGui::Button("S"))
	{
		_gizmoMode = ImGuizmo::SCALE;
	}

	ImGui::Dummy({20.0f * pixelScale, 0});

	if (ImGui::Button(_gizmoSpace == ImGuizmo::LOCAL ? "Local" : "World"))
	{
		if (_gizmoSpace == ImGuizmo::LOCAL)
		{
			_gizmoSpace = ImGuizmo::WORLD;
		}
		else
		{
			_gizmoSpace = ImGuizmo::LOCAL;
		}
	}

	ImGui::Separator();

	ImGui::Checkbox("Fullscreen", &_fullscreen);

	ImGui::Separator();

	ImGui::SetNextItemWidth(130.0f * pixelScale);
	if (ImGui::BeginCombo("SceneRenderer", magic_enum::enum_name(_sceneRendererType).data()))
	{
		for (UIViewport::RendererType sceneRendererType : magic_enum::enum_values<UIViewport::RendererType>())
		{
			if (sceneRendererType == RendererType::PathTracing && !Engine::getVKContext().isRayTracingSupported())
			{
				continue;
			}

			const bool is_selected = sceneRendererType == _sceneRendererType;
			if (ImGui::Selectable(magic_enum::enum_name(sceneRendererType).data(), is_selected))
			{
				_sceneRenderer.reset();
				_sceneRendererType = sceneRendererType;
			}

			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::EndChild();

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - style.ItemSpacing.y);
}

void UIViewport::drawRenderToFilePopup()
{
	if (ImGui::BeginPopupModal("Rendering status", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(_renderToFileData->status == RenderToFileStatus::eSaveFinished ? "Rendering finished" : "Rendering in progress...");
		ImGui::ProgressBar(static_cast<float>(_renderToFileData->renderedSamples) / static_cast<float>(_renderToFileData->totalSamples));
		ImGui::Text("Rendered samples: %u/%u", _renderToFileData->renderedSamples, _renderToFileData->totalSamples);

		auto duration = _renderToFileData->lastBatchTime - _renderToFileData->startTime;
		auto durationRounded = std::chrono::floor<std::chrono::duration<long long, std::deci>>(duration);
		ImGui::Text("%s", std::format("Elapsed time: {:%H:%M:%S}", durationRounded).c_str());

		if (_renderToFileData->status == RenderToFileStatus::eRendering)
		{
			if (ImGui::Button("Finish now"))
			{
				_renderToFileData->status = RenderToFileStatus::eRenderFinished;
			}
		}
		else if (_renderToFileData->status == RenderToFileStatus::eSaveFinished)
		{
			bool close = ImGui::Button("Close");
			ImGui::SameLine();
			bool openInExplorer = ImGui::Button("Open in Explorer");

			if (openInExplorer)
			{
				FileHelper::openExplorerAndSelectEntries(_renderToFileData->outputFile.parent_path(), {_renderToFileData->outputFile});
			}

			if (openInExplorer || close)
			{
				ImGui::CloseCurrentPopup();
				_renderToFileData.reset();
			}
		}

		ImGui::EndPopup();
	}
}

bool UIViewport::isFullscreen()
{
	return _fullscreen;
}

void UIViewport::renderToFile(glm::uvec2 resolution, uint32_t sampleCount)
{
	// clang-format off
	std::optional<std::filesystem::path> filePath = FileHelper::fileDialogSave({
		FileDialogFilter{
			.fileTypeDisplayName = L"PNG Image",
			.fileTypeExtensions = L"*.png"
		},
		FileDialogFilter{
			.fileTypeDisplayName = L"JPG Image",
			.fileTypeExtensions = L"*.jpg"
		}
	}, ".", "render");
	// clang-format on

	if (!filePath)
	{
		return;
	}

	_showRenderToFilePopup = true;

	_renderToFileData = std::make_unique<UIViewport::RenderToFileData>();
	_renderToFileData->renderedSamples = 0;
	_renderToFileData->totalSamples = sampleCount;
	_renderToFileData->renderer = std::make_unique<PathTracingSceneRenderer>(resolution);
	_renderToFileData->camera = _camera;
	_renderToFileData->camera.setAspectRatio(static_cast<float>(resolution.x) / static_cast<float>(resolution.y));
	_renderToFileData->outputFile = filePath.value();
	_renderToFileData->startTime = std::chrono::high_resolution_clock::now();

	Engine::getScene().onPreRender(_renderToFileData->registry, _renderToFileData->camera);
}

void UIViewport::init()
{
	_objectPicker = std::make_unique<ObjectPicker>();
}

void UIViewport::shutdown()
{
	_sceneRenderer = {};
	_objectPicker = {};
	_renderToFileData = {};
	_lastViewportImage = {};
}