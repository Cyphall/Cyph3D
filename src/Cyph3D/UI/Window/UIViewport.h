#pragma once

#include "Cyph3D/Scene/Camera.h"

#include <imgui.h>
#include <ImGuizmo.h>
#include <functional>
#include <map>
#include <memory>
#include <string>

class Renderer;
struct PerfStep;

class UIViewport
{
public:
	static void show();
	
	static Camera& getCamera();
	static void setCamera(Camera camera);
	
	static bool isFullscreen();
	
	static void renderToFile(glm::ivec2 resolution);
	
	static const PerfStep* getPreviousFramePerfStep();

private:
	static std::unique_ptr<Renderer> _renderer;
	static Camera _camera;
	static glm::vec2 _rendererSize;
	static bool _currentlyClicking;
	static glm::vec2 _clickPos;
	static bool _cameraFocused;
	static glm::dvec2 _lockedCursorPos;
	static bool _fullscreen;
	
	static bool _rendererIsInvalidated;
	
	static std::string _rendererType;
	
	static const PerfStep* _perfStep;
	
	static std::map<std::string, std::function<void(void)>> _allocators;
	static void initAllocators();
	
	static ImGuizmo::OPERATION _gizmoMode;
	static ImGuizmo::MODE _gizmoSpace;
	
	static void drawGizmo(glm::vec2 viewportStart, glm::vec2 viewportSize);
	static void drawHeader();
	static void invalidateRenderer();
	
	friend class Engine;
};