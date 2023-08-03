#pragma once

#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/UI/ObjectPicker.h"

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
	
	static void init();
	static void shutdown();

private:
	enum class RendererType
	{
		Rasterization,
		PathTracing
	};
	
	struct RenderToFileData;
	
	static std::unique_ptr<SceneRenderer> _sceneRenderer;
	static RendererType _sceneRendererType;
	static uint64_t _sceneChangeVersion;
	
	static glm::uvec2 _previousViewportSize;
	
	static Camera _camera;
	static bool _cameraFocused;
	static glm::vec2 _lockedCursorPos;
	
	static bool _fullscreen;
	
	static bool _leftClickPressedOnViewport;
	static glm::vec2 _leftClickPressPos;
	
	static ImGuizmo::OPERATION _gizmoMode;
	static ImGuizmo::MODE _gizmoSpace;
	
	static RenderRegistry _renderRegistry;
	
	static std::unique_ptr<ObjectPicker> _objectPicker;
	
	static std::unique_ptr<RenderToFileData> _renderToFileData;
	static bool _showRenderToFilePopup;
	static VKPtr<VKImageView> _lastViewportImageView;
	
	static void drawGizmo(glm::vec2 viewportStart, glm::vec2 viewportSize);
	static void drawHeader();
	static void drawRenderToFilePopup();
};