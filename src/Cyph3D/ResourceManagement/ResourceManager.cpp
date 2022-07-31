#include "ResourceManager.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/GLObject/Material/Material.h"
#include "Cyph3D/GLObject/ShaderProgram.h"
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

Model* ResourceManager::requestModel(const std::string& name)
{
	if (!_models.contains(name))
	{
		_models[name] = std::make_unique<Model>(name);
		
		Model* model = _models[name].get();
		
		_threadPool.push_task([](Model* model)
		{
			Logger::info(std::format("Loading model \"{}\"", model->getName()));
			model->loadResource();
			Logger::info(std::format("Model \"{}\" loaded", model->getName()));
		}, model);
	}
	
	return _models[name].get();
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

ShaderProgram* ResourceManager::requestShaderProgram(const ShaderProgramCreateInfo& createInfo)
{
	if (!_shaderPrograms.contains(createInfo))
	{
		Logger::info("Loading shader program");
		_shaderPrograms[createInfo] = std::make_unique<ShaderProgram>(createInfo);
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