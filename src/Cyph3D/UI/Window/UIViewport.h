#pragma once

#include "Cyph3D/Scene/Camera.h"

#include <imgui.h>
#include <ImGuizmo.h>
#include <functional>
#include <map>
#include <memory>
#include <string>

class SceneRenderer;
class PerfStep;

class UIViewport
{
public:
	static void show();
	
	static Camera& getCamera();
	static void setCamera(Camera camera);
	
	static bool isFullscreen();
	
	static void renderToFile(glm::uvec2 resolution, uint32_t sampleCount);
	
	static const PerfStep* getPreviousFramePerfStep();
	
	static void shutdown();

private:
	enum class RendererType
	{
		Rasterization,
		Raytracing
	};
	
	static std::unique_ptr<SceneRenderer> _sceneRenderer;
	static RendererType _sceneRendererType;
	
	static glm::uvec2 _previousViewportSize;
	
	static Camera _camera;
	static bool _cameraFocused;
	static glm::vec2 _lockedCursorPos;
	
	static bool _fullscreen;
	
	static bool _leftClickPressedOnViewport;
	static glm::vec2 _leftClickPressPos;
	
	static ImGuizmo::OPERATION _gizmoMode;
	static ImGuizmo::MODE _gizmoSpace;
	
	static void drawGizmo(glm::vec2 viewportStart, glm::vec2 viewportSize);
	static void drawHeader();
};