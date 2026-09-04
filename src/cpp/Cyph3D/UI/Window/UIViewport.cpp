#include "UIViewport.h"

#include <Cyph3D/Engine.h>
#include <Cyph3D/Entity/Entity.h>
#include <Cyph3D/Helper/FileHelper.h>
#include <Cyph3D/Rendering/SceneRenderer/PathTracingSceneRenderer.h>
#include <Cyph3D/Rendering/SceneRenderer/RasterizationSceneRenderer.h>
#include <Cyph3D/Scene/Scene.h>
#include <Cyph3D/UI/Window/UIInspector.h>
#include <Cyph3D/UI/Window/UIMisc.h>
#include <Cyph3D/Window.h>

#include <chrono>
#include <CyphGPU/Device.hpp>
#include <CyphGPU/DeviceSession.hpp>
#include <CyphGPU/ImGuiBackend.hpp>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui_internal.h>
#include <magic_enum/magic_enum.hpp>

std::unique_ptr<c3d::SceneRenderer> c3d::UIViewport::_sceneRenderer;
c3d::UIViewport::RendererType c3d::UIViewport::_sceneRendererType = UIViewport::RendererType::Rasterization;
uint64_t c3d::UIViewport::_sceneChangeVersion = -1;

glm::uvec2 c3d::UIViewport::_previousViewportSize = {0, 0};

c3d::Camera c3d::UIViewport::_camera;
bool c3d::UIViewport::_cameraFocused = false;
bool c3d::UIViewport::_cameraChanged = true;
glm::vec2 c3d::UIViewport::_lockedCursorPos;

bool c3d::UIViewport::_fullscreen = false;

bool c3d::UIViewport::_leftClickPressedOnViewport = false;
glm::vec2 c3d::UIViewport::_leftClickPressPos;

ImGuizmo::OPERATION c3d::UIViewport::_gizmoMode = ImGuizmo::TRANSLATE;
ImGuizmo::MODE c3d::UIViewport::_gizmoSpace = ImGuizmo::LOCAL;

c3d::RenderRegistry c3d::UIViewport::_renderRegistry;

std::unique_ptr<c3d::ObjectPicker> c3d::UIViewport::_objectPicker;

cgpu::ImagePtr c3d::UIViewport::_lastViewportImage;

void c3d::UIViewport::show(cgpu::CommandRecorder& commandRecorder)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	bool open = ImGui::Begin("Viewport");

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();

	if (open)
	{
		drawHeader();

		Window& window = Engine::getWindow();

		if (_cameraFocused && window.getMouseButtonState(GLFW_MOUSE_BUTTON_RIGHT) == Window::MouseButtonState::eReleased)
		{
			_cameraFocused = false;
			window.setInputMode(GLFW_CURSOR_NORMAL);
			window.setCursorPos(_lockedCursorPos);
		}

		// all of this need to be calculated before calling ImGui::Image()
		glm::ivec2 viewportStartLocal = glm::vec2(ImGui::GetCursorPos());
		glm::ivec2 viewportEndLocal = glm::vec2(ImGui::GetContentRegionAvail());

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

			if (!UIMisc::isRenderToFileInProgress())
			{
				uint64_t currentSceneChangeVersion = Scene::getChangeVersion();
				bool sceneChanged = currentSceneChangeVersion != _sceneChangeVersion;

				if (sceneChanged)
				{
					_renderRegistry.clear();
					Engine::getScene().onPreRender(_renderRegistry, _camera);
				}

				_lastViewportImage = _sceneRenderer->render(commandRecorder, _camera, _renderRegistry, sceneChanged, cameraChanged);

				_sceneChangeVersion = currentSceneChangeVersion;
			}

			ImGui::Image(
				ImGui_ImplCyphGPU_ToTextureID(_lastViewportImage),
				glm::vec2(_lastViewportImage->getDesc().extent),
				ImVec2(0, 0),
				ImVec2(1, 1)
			);

			drawGizmo(viewportStartGlobal, viewportSize);

			if (window.getMouseButtonState(GLFW_MOUSE_BUTTON_RIGHT) == Window::MouseButtonState::eClicked && ImGui::IsItemHovered())
			{
				_cameraFocused = true;
				_lockedCursorPos = window.getCursorPos();
				window.setInputMode(GLFW_CURSOR_DISABLED);

				_leftClickPressedOnViewport = false;
			}

			glm::vec2 viewportCursorPos = window.getCursorPos() - glm::vec2(viewportStartGlobal);

			if (!_cameraFocused && window.getMouseButtonState(GLFW_MOUSE_BUTTON_LEFT) == Window::MouseButtonState::eClicked && ImGui::IsItemHovered())
			{
				_leftClickPressedOnViewport = true;
				_leftClickPressPos = viewportCursorPos;
			}

			if (_leftClickPressedOnViewport && window.getMouseButtonState(GLFW_MOUSE_BUTTON_LEFT) == Window::MouseButtonState::eReleased)
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

c3d::Camera& c3d::UIViewport::getCamera()
{
	return _camera;
}

void c3d::UIViewport::setCamera(const Camera& camera)
{
	_camera = camera;
	_camera.setAspectRatio(static_cast<float>(_previousViewportSize.x) / static_cast<float>(_previousViewportSize.y));
	_cameraChanged = true;
}

void c3d::UIViewport::drawGizmo(glm::vec2 viewportStart, glm::vec2 viewportSize)
{
	IInspectable* selected = UIInspector::getSelected();
	if (selected == nullptr)
	{
		return;
	}

	Entity* entity = dynamic_cast<Entity*>(selected);
	if (entity == nullptr)
	{
		return;
	}

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

void c3d::UIViewport::drawHeader()
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
			if (sceneRendererType == RendererType::PathTracing && !(Engine::getDeviceSession()->getDevice()->getCapabilities() & cgpu::Device::Capability::eRayTracing))
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
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	ImGui::EndChild();

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - style.ItemSpacing.y);
}

bool c3d::UIViewport::isFullscreen()
{
	return _fullscreen;
}

void c3d::UIViewport::init()
{
	_objectPicker = std::make_unique<ObjectPicker>();
}

void c3d::UIViewport::shutdown()
{
	_sceneRenderer = {};
	_objectPicker = {};
}
