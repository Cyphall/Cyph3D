#include "ResourceManager.h"

#include "Cyph3D/GLObject/Material/Material.h"
#include "Cyph3D/GLObject/GLShaderProgram.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/ResourceManagement/Image.h"
#include "Cyph3D/ResourceManagement/Model.h"
#include "Cyph3D/ResourceManagement/Skybox.h"

#include <format>

ResourceManager::ResourceManager(int threadCount):
_threadPool(threadCount)
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
	auto it = _images.find(name);
	if (it == _images.end())
	{
		it = _images.try_emplace(name, std::make_unique<Image>(name, type, *this)).first;
	}

	return it->second.get();
}

Skybox* ResourceManager::requestSkybox(const std::string& name)
{
	auto it = _skyboxes.find(name);
	if (it == _skyboxes.end())
	{
		it = _skyboxes.try_emplace(name, std::make_unique<Skybox>(name, *this)).first;
	}

	return it->second.get();
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
	_mainThreadTasksMutex.lock();
	
	auto it = _mainThreadTasks.begin();
	while (it != _mainThreadTasks.end())
	{
		std::function<bool()>& task = *it;

		_mainThreadTasksMutex.unlock();
		bool completed = task();
		_mainThreadTasksMutex.lock();
		
		if (completed)
		{
			it = _mainThreadTasks.erase(it);
		}
		else
		{
			it++;
		}
	}

	_mainThreadTasksMutex.unlock();
}