#pragma once

#include <memory>
#include "../../GLObject/Texture.h"
#include "../../Scene/Camera.h"
#include <imgui.h>
#include <ImGuizmo.h>

class Renderer;

class UIViewport
{
public:
	static void show();
	
	static Camera& getCamera();
	static void setCamera(Camera camera);

private:
	static std::unique_ptr<Renderer> _renderer;
	static Camera _camera;
	static glm::vec2 _lastViewportSize;
	static bool _currentlyClicking;
	static glm::vec2 _clickPos;
	static bool _cameraFocused;
	static glm::dvec2 _lockedCursorPos;
	
	static ImGuizmo::OPERATION _gizmoMode;
	static ImGuizmo::MODE _gizmoSpace;
	
	static void onWindowSizeChanged(glm::vec2 newSize);
	
	static void drawGizmo(glm::vec2 viewportStart, glm::vec2 viewportSize);
	static void drawGizmoWindow();
};
