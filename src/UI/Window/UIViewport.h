#pragma once

#include <memory>
#include "../../GLObject/Texture.h"
#include "../../Scene/Camera.h"
#include <imgui.h>
#include <ImGuizmo.h>
#include <string>
#include <map>
#include <functional>

class Renderer;

class UIViewport
{
public:
	static void show();
	
	static Camera& getCamera();
	static void setCamera(Camera camera);
	
	static bool isFullscreen();
	
	static void renderToFile(glm::ivec2 resolution);

private:
	static std::unique_ptr<Renderer> _renderer;
	static Camera _camera;
	static glm::vec2 _rendererSize;
	static bool _currentlyClicking;
	static glm::vec2 _clickPos;
	static bool _cameraFocused;
	static glm::dvec2 _lockedCursorPos;
	static bool _gbufferDebugView;
	static bool _fullscreen;
	
	static bool _rendererIsInvalidated;
	
	static std::string _rendererType;
	
	static std::map<std::string, std::function<void(void)>> _allocators;
	static void initAllocators();
	
	static ImGuizmo::OPERATION _gizmoMode;
	static ImGuizmo::MODE _gizmoSpace;
	
	static void drawGizmo(glm::vec2 viewportStart, glm::vec2 viewportSize);
	static void drawHeader();
	static void invalidateRenderer();
	
	friend class Engine;
};
