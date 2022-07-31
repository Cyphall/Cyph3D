#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "Cyph3D/Timer.h"

class Window;
class ResourceManager;
class Scene;

class Engine
{
public:
	static void init();
	static void run();
	static void shutdown();
	
	static Window& getWindow();
	static ResourceManager& getGlobalRM();
	static Scene& getScene();
	static void setScene(std::unique_ptr<Scene>&& scene);
	static Timer& getTimer();
	
private:
	static std::unique_ptr<Window> _window;
	static std::unique_ptr<ResourceManager> _globalResourceManager;
	static std::unique_ptr<Scene> _scene;
	
	static Timer _timer;
};