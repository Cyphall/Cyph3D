#include "UIViewport.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Entity/Entity.h"
#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/RenderContext.h"
#include "Cyph3D/Rendering/Renderer/RasterizationRenderer.h"
#include "Cyph3D/Rendering/Renderer/RaytracingRenderer.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/UI/Window/UIInspector.h"
#include "Cyph3D/Window.h"

#include <imgui_internal.h>
#include <stb_image_write.h>
#include <GLFW/glfw3.h>

std::unique_ptr<Renderer> UIViewport::_renderer;
Camera UIViewport::_camera;
bool UIViewport::_cameraFocused = false;
glm::dvec2 UIViewport::_lockedCursorPos;
glm::vec2 UIViewport::_rendererSize(0);
bool UIViewport::_currentlyClicking = false;
glm::vec2 UIViewport::_clickPos;
bool UIViewport::_fullscreen = false;

bool UIViewport::_rendererIsInvalidated = true;

std::string UIViewport::_rendererType = RasterizationRenderer::identifier;
const PerfStep* UIViewport::_perfStep = nullptr;

std::map<std::string, std::function<std::unique_ptr<Renderer>(void)>> UIViewport::_rendererFactories;

ImGuizmo::OPERATION UIViewport::_gizmoMode = ImGuizmo::TRANSLATE;
ImGuizmo::MODE UIViewport::_gizmoSpace = ImGuizmo::LOCAL;

void UIViewport::show()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	
	bool open = ImGui::Begin("Viewport");
	
	ImGui::PopStyleVar();
	
	if (!open)
	{
		ImGui::End();
		return;
	}
	
	drawHeader();
	
	glm::vec2 viewportStartLocal = ImGui::GetCursorPos();
	glm::vec2 viewportEndLocal = ImGui::GetWindowContentRegionMax();
	
	glm::vec2 viewportSize = viewportEndLocal - viewportStartLocal;
	
	glm::vec2 viewportStartGlobal = ImGui::GetCursorScreenPos();
	glm::vec2 viewportEndGlobal = viewportStartGlobal + viewportSize;
	
	if (viewportSize != _rendererSize)
	{
		if (viewportSize.x <= 0 || viewportSize.y <= 0)
		{
			ImGui::End();
			return;
		}
		invalidateRenderer();
		_camera.setAspectRatio(viewportSize.x / viewportSize.y);
		_rendererSize = viewportSize;
	}
	
	glm::vec2 viewportCursorPos = glm::vec2(ImGui::GetIO().MousePos) - viewportStartGlobal;
	
	if (Engine::getWindow().getMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
	{
		_cameraFocused = false;
		_lockedCursorPos = Engine::getWindow().getCursorPos();
		Engine::getWindow().setInputMode(GLFW_CURSOR_NORMAL);
	}
	
	if (_cameraFocused)
	{
		_camera.update(Engine::getWindow().getCursorPos() - _lockedCursorPos);
		Engine::getWindow().setCursorPos(_lockedCursorPos);
	}
	
	if (_rendererIsInvalidated)
	{
		_renderer.reset();
		_renderer = _rendererFactories[_rendererType]();
		_rendererIsInvalidated = false;
	}
	
	_renderer->onNewFrame();
	
	RenderContext context
	{
		.renderer = *_renderer,
		.camera = _camera
	};
	Engine::getScene().onPreRender(context);
	
	auto [texture, perfStep] = _renderer->render(_camera);
	
	_perfStep = perfStep;
	
	ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<intptr_t>(texture->getHandle())), glm::vec2(texture->getSize()), ImVec2(0, 1), ImVec2(1, 0));
	
	if (Engine::getWindow().getMouseButton(GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && ImGui::IsItemHovered())
	{
		_cameraFocused = true;
		_currentlyClicking = false;
		Engine::getWindow().setInputMode(GLFW_CURSOR_DISABLED);
	}
	
	if (!_cameraFocused && !_currentlyClicking && Engine::getWindow().getMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && ImGui::IsItemHovered())
	{
		_currentlyClicking = true;
		_clickPos = viewportCursorPos;
	}
	
	if (_currentlyClicking && Engine::getWindow().getMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		_currentlyClicking = false;
		if (ImGui::IsItemHovered() && glm::distance(_clickPos, viewportCursorPos) < 5)
		{
			Entity* clickedEntity = _renderer->getClickedEntity(_clickPos);
			UIInspector::setSelected(clickedEntity);
		}
	}
	
	drawGizmo(viewportStartGlobal, viewportSize);
	
	ImGui::End();
}

Camera& UIViewport::getCamera()
{
	return _camera;
}

void UIViewport::setCamera(Camera camera)
{
	_camera = camera;
	_camera.setAspectRatio(_rendererSize.x / _rendererSize.y);
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
	glm::mat4 worldModel = transform.getLocalToWorldMatrix();
	
	glm::mat4 view = _camera.getView();
	glm::mat4 projection = _camera.getProjection();
	
	ImGui::PushClipRect(viewportStart, viewportStart + viewportSize, false);
	
	ImGuizmo::SetDrawlist();
	bool changed = ImGuizmo::Manipulate(
		glm::value_ptr(view),
		glm::value_ptr(projection),
		_gizmoMode,
		_gizmoSpace,
		glm::value_ptr(worldModel));
	
	ImGui::PopClipRect();
	
	if (changed)
	{
		glm::mat4 localModel = transform.getParent()->getWorldToLocalMatrix() * worldModel;
		
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;
		
		ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(localModel), glm::value_ptr(position), glm::value_ptr(rotation), glm::value_ptr(scale));
		
		if (position != transform.getLocalPosition())
		{
			transform.setLocalPosition(position);
		}
		else if (rotation != transform.getEulerLocalRotation())
		{
			transform.setEulerLocalRotation(rotation);
		}
		else if (scale != transform.getLocalScale())
		{
			transform.setLocalScale(scale);
		}
	}
}

void UIViewport::drawHeader()
{
	float pixelScale = Engine::getWindow().getPixelScale();
	
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::BeginChild("ViewportHeader", ImVec2(0, ImGui::GetFontSize() + style.FramePadding.y * 2.0f + style.WindowPadding.y * 2.0f), false, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar);
	
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
	
	if (ImGui::Button(_gizmoSpace == ImGuizmo::LOCAL ? "Local" : "Global"))
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
	if (ImGui::BeginCombo("Renderer", _rendererType.c_str()))
	{
		for (auto& [name, _] : _rendererFactories)
		{
			const bool is_selected = (name == _rendererType);
			if (ImGui::Selectable(name.c_str(), is_selected))
			{
				_rendererType = name;
				invalidateRenderer();
			}
			
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	
	ImGui::EndChild();
	
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - style.ItemSpacing.y);
}

bool UIViewport::isFullscreen()
{
	return _fullscreen;
}

void UIViewport::invalidateRenderer()
{
	_rendererIsInvalidated = true;
}

void UIViewport::initRendererFactories()
{
	_rendererFactories[RasterizationRenderer::identifier] = []() -> decltype(auto) { return std::make_unique<RasterizationRenderer>(UIViewport::_rendererSize);};
	_rendererFactories[RaytracingRenderer::identifier] = []() -> decltype(auto) { return std::make_unique<RaytracingRenderer>(UIViewport::_rendererSize);};
}

void UIViewport::renderToFile(glm::ivec2 resolution)
{
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
	
	if (!filePath)
	{
		return;
	}
	
	RaytracingRenderer renderer(resolution);
	
	Camera camera(_camera);
	camera.setAspectRatio(static_cast<float>(resolution.x) / static_cast<float>(resolution.y));
	
	renderer.onNewFrame();
	
	RenderContext context
	{
		.renderer = renderer,
		.camera = camera
	};
	Engine::getScene().onPreRender(context);
	
	auto [texture, perfStep] = renderer.render(camera);
	
	glm::ivec2 textureSize = texture->getSize();
	std::vector<glm::u8vec3> textureData(textureSize.x * textureSize.y);
	
	glGetTextureImage(texture->getHandle(), 0, GL_RGB, GL_UNSIGNED_BYTE, textureData.size() * sizeof(glm::u8vec3), textureData.data());
	
	if (filePath->extension() == ".png")
	{
		stbi_write_png(filePath->generic_string().c_str(), textureSize.x, textureSize.y, 3, textureData.data(), textureSize.x * 3);
	}
	else if (filePath->extension() == ".jpg")
	{
		stbi_write_jpg(filePath->generic_string().c_str(), textureSize.x, textureSize.y, 3, textureData.data(), 95);
	}
}

const PerfStep* UIViewport::getPreviousFramePerfStep()
{
	return _perfStep;
}