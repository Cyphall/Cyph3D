#pragma once

class Window;
class ResourceManager;
class Scene;
class Renderer;

#include <memory>

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
	static Renderer& getRenderer();
	
private:
	static std::unique_ptr<Window> _window;
	static std::unique_ptr<ResourceManager> _globalResourceManager;
	static std::unique_ptr<Scene> _scene;
	static std::unique_ptr<Renderer> _renderer;
	
	static double _previousTime;
};
