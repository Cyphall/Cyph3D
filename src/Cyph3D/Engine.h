#pragma once

#include "Cyph3D/Timer.h"

#include <memory>

class Window;
class AssetManager;
class Scene;

class Engine
{
public:
	static void init();
	static void run();
	static void shutdown();
	
	static Window& getWindow();
	static AssetManager& getAssetManager();
	static Scene& getScene();
	static void setScene(std::unique_ptr<Scene>&& scene);
	static Timer& getTimer();
	
private:
	static std::unique_ptr<Window> _window;
	static std::unique_ptr<AssetManager> _assetManager;
	static std::unique_ptr<Scene> _scene;
	
	static Timer _timer;
};