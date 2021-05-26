#include "UIViewport.h"
#include "../../Rendering/Renderer.h"
#include "../../Scene/Scene.h"
#include "../../Window.h"
#include "../../Engine.h"
#include "UIInspector.h"

std::unique_ptr<Renderer> UIViewport::_renderer;
Camera UIViewport::_camera;
bool UIViewport::_cameraFocused = false;
glm::dvec2 UIViewport::_lockedCursorPos;
glm::vec2 UIViewport::_lastViewportSize(0);
bool UIViewport::_currentlyClicking = false;
glm::vec2 UIViewport::_clickPos;

ImGuizmo::OPERATION UIViewport::_gizmoMode = ImGuizmo::TRANSLATE;
ImGuizmo::MODE UIViewport::_gizmoSpace = ImGuizmo::LOCAL;

void UIViewport::show()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	
	if (!ImGui::Begin("Viewport"))
	{
		ImGui::End();
		ImGui::PopStyleVar();
		return;
	}
	
	drawGizmoWindow();
	
	glm::vec2 viewportStartLocal = ImGui::GetCursorPos();
	glm::vec2 viewportEndLocal = ImGui::GetWindowContentRegionMax();
	glm::vec2 viewportSize = viewportEndLocal - viewportStartLocal;
	if (viewportSize != _lastViewportSize)
	{
		if (viewportSize.x <= 0 || viewportSize.y <= 0)
		{
			ImGui::End();
			ImGui::PopStyleVar();
			return;
		}
		onWindowSizeChanged(viewportSize);
	}
	glm::vec2 viewportStartGlobal = ImGui::GetCursorScreenPos();
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
	
	_renderer->onNewFrame();
	
	RenderContext context
	{
		.renderer = *_renderer.get(),
		.camera = _camera
	};
	Engine::getScene().onPreRender(context);
	
	Texture& texture = _renderer->render(_camera);
	
	ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<intptr_t>(texture.getHandle())), glm::vec2(texture.getSize()), ImVec2(0, 1), ImVec2(1, 0));
	
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
			Entity* entity = _renderer->getClickedEntity(_clickPos);
			UIInspector::setSelected(entity ? &entity->getTransform() : std::any());
		}
	}
	
	ImGui::End();
	ImGui::PopStyleVar();
	
	drawGizmo(viewportStartGlobal, viewportSize);
}

void UIViewport::onWindowSizeChanged(glm::vec2 newSize)
{
	_renderer = std::make_unique<Renderer>(newSize);
	_camera.setAspectRatio(newSize.x / newSize.y);
	
	_lastViewportSize = newSize;
}

Camera& UIViewport::getCamera()
{
	return _camera;
}

void UIViewport::setCamera(Camera camera)
{
	_camera = camera;
}

void UIViewport::drawGizmo(glm::vec2 viewportStart, glm::vec2 viewportSize)
{
	std::any selected = UIInspector::getSelected();
	if (!selected.has_value()) return;
	if (selected.type() != typeid(Transform*)) return;
	
	ImGuizmo::SetRect(viewportStart.x, viewportStart.y, viewportSize.x, viewportSize.y);
	
	Transform* transform = std::any_cast<Transform*>(selected);
	glm::mat4 worldModel = transform->getLocalToWorldMatrix();
	
	glm::mat4 view = _camera.getView();
	glm::mat4 projection = _camera.getProjection();
	
	bool changed = ImGuizmo::Manipulate(
		glm::value_ptr(view),
		glm::value_ptr(projection),
		_gizmoMode,
		_gizmoSpace,
		glm::value_ptr(worldModel));
	
	if (changed)
	{
		glm::mat4 localModel = transform->getParent()->getWorldToLocalMatrix() * worldModel;
		
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;
		
		ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(localModel), glm::value_ptr(position), glm::value_ptr(rotation), glm::value_ptr(scale));
		
		if (position != transform->getLocalPosition())
		{
			transform->setLocalPosition(position);
		}
		else if (rotation != transform->getEulerLocalRotation())
		{
			transform->setEulerLocalRotation(rotation);
		}
		else if (scale != transform->getLocalScale())
		{
			transform->setLocalScale(scale);
		}
	}
}

void UIViewport::drawGizmoWindow()
{
	if (ImGui::Button("T"))
	{
		_gizmoMode = ImGuizmo::TRANSLATE;
	}
	ImGui::SameLine();
	if (ImGui::Button("R"))
	{
		_gizmoMode = ImGuizmo::ROTATE;
	}
	ImGui::SameLine();
	if (ImGui::Button("S"))
	{
		_gizmoMode = ImGuizmo::SCALE;
	}
	ImGui::SameLine(0, 30);
	
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
}
