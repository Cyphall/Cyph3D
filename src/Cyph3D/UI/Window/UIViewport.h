#pragma once

#include "Cyph3D/Scene/Camera.h"

#include <imgui.h>
#include <ImGuizmo.h>
#include <functional>
#include <map>
#include <memory>
#include <string>

class SceneRenderer;
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
	
	static void initSceneRendererFactories();

private:
	static std::unique_ptr<SceneRenderer> _sceneRenderer;
	static Camera _camera;
	static glm::vec2 _sceneRendererSize;
	static bool _currentlyClicking;
	static glm::vec2 _clickPos;
	static bool _cameraFocused;
	static glm::dvec2 _lockedCursorPos;
	static bool _fullscreen;
	
	static bool _sceneRendererIsInvalidated;
	
	static std::string _sceneRendererType;
	
	static const PerfStep* _perfStep;
	
	static std::map<std::string, std::function<std::unique_ptr<SceneRenderer>(void)>> _sceneRendererFactories;
	
	static ImGuizmo::OPERATION _gizmoMode;
	static ImGuizmo::MODE _gizmoSpace;
	
	static void drawGizmo(glm::vec2 viewportStart, glm::vec2 viewportSize);
	static void drawHeader();
	static void invalidateSceneRenderer();
};