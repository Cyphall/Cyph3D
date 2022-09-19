#include "ResourceManager.h"

#include "Cyph3D/GLObject/Material/Material.h"
#include "Cyph3D/ResourceManagement/Image.h"
#include "Cyph3D/ResourceManagement/Model.h"
#include "Cyph3D/ResourceManagement/Skybox.h"

ResourceManager::ResourceManager(int threadCount):
_threadPool(threadCount)
{

}

ResourceManager::~ResourceManager()
{}

Model* ResourceManager::requestModel(const std::string& path)
{
	auto it = _models.find(path);
	if (it == _models.end())
	{
		it = _models.try_emplace(path, std::unique_ptr<Model>(new Model(path, *this))).first;
	}
	
	return it->second.get();
}

Image* ResourceManager::requestImage(const std::string& name, ImageType type)
{
	auto it = _images.find(name);
	if (it == _images.end())
	{
		it = _images.try_emplace(name, std::unique_ptr<Image>(new Image(name, type, *this))).first;
	}

	return it->second.get();
}

Skybox* ResourceManager::requestSkybox(const std::string& path)
{
	auto it = _skyboxes.find(path);
	if (it == _skyboxes.end())
	{
		it = _skyboxes.try_emplace(path, std::unique_ptr<Skybox>(new Skybox(path, *this))).first;
	}

	return it->second.get();
}

Material* ResourceManager::requestMaterial(const std::string& name)
{
	auto it = _materials.find(name);
	if (it == _materials.end())
	{
		it = _materials.try_emplace(name, std::unique_ptr<Material>(new Material(name, this))).first;
	}

	return it->second.get();
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