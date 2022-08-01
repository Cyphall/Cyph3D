#include "ResourceManager.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/GLObject/Material/Material.h"
#include "Cyph3D/GLObject/GLShaderProgram.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/ResourceManagement/Image.h"
#include "Cyph3D/ResourceManagement/Model.h"
#include "Cyph3D/ResourceManagement/Skybox.h"
#include "Cyph3D/Window.h"

#include <GLFW/glfw3.h>
#include <format>

ResourceManager::ResourceManager(int threadCount):
_threadPool(threadCount, Engine::getWindow().getHandle())
{

}

ResourceManager::~ResourceManager()
{}

Model* ResourceManager::requestModel(const std::string& name)
{
	auto it = _models.find(name);
	if (it == _models.end())
	{
		it = _models.try_emplace(name, std::make_unique<Model>(name, *this)).first;
	}
	
	return it->second.get();
}

Image* ResourceManager::requestImage(const std::string& name, ImageType type)
{
	if (!_images.contains(name))
	{
		_images[name] = std::make_unique<Image>(name);
		
		Image* image = _images[name].get();
		
		_threadPool.push_task([](Image* image, ImageType type)
		{
			Logger::info(std::format("Loading image \"{}\"", image->getName()));
			image->loadResource(type);
			Logger::info(std::format("Image \"{}\" loaded", image->getName()));
		}, image, type);
	}
	
	return _images[name].get();
}

Skybox* ResourceManager::requestSkybox(const std::string& name)
{
	if (!_skyboxes.contains(name))
	{
		_skyboxes[name] = std::make_unique<Skybox>(name);
		
		Skybox* skybox = _skyboxes[name].get();
		
		_threadPool.push_task([](Skybox* skybox)
		{
			Logger::info(std::format("Loading skybox \"{}\"", skybox->getName()));
			skybox->loadResource();
			Logger::info(std::format("Skybox \"{}\" loaded", skybox->getName()));
		}, skybox);
	}
	
	return _skyboxes[name].get();
}

GLShaderProgram* ResourceManager::requestShaderProgram(const ShaderProgramCreateInfo& createInfo)
{
	if (!_shaderPrograms.contains(createInfo))
	{
		Logger::info("Loading shader program");
		_shaderPrograms[createInfo] = std::make_unique<GLShaderProgram>(createInfo);
		Logger::info(std::format("Shader program loaded (id: {})", _shaderPrograms[createInfo]->getHandle()));
	}
	
	return _shaderPrograms[createInfo].get();
}

Material* ResourceManager::requestMaterial(const std::string& name)
{
	if (!_materials.contains(name))
	{
		_materials[name] = std::unique_ptr<Material>(new Material(name, this));
	}
	
	return _materials[name].get();
}

void ResourceManager::onUpdate()
{
	auto it = _mainThreadTasks.begin();
	while (it != _mainThreadTasks.end())
	{
		std::function<bool()>& task = *it;
		
		bool completed = task();
		if (completed)
		{
			it = _mainThreadTasks.erase(it);
		}
		else
		{
			it++;
		}
	}
}

void ResourceManager::addThreadPoolTask(const std::function<void()>& task)
{
	_threadPool.push_task(task);
}

void ResourceManager::addMainThreadTask(const std::function<bool()>& task)
{
	_mainThreadTasks.push_back(task);
}