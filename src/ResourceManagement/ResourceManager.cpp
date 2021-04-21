#include "ResourceManager.h"
#include "../Logger.h"
#include <fmt/core.h>

ResourceManager::ResourceManager(int threadCount):
_threadPool(threadCount)
{
	_threadPool.init();
}

ResourceManager::~ResourceManager()
{
	_threadPool.shutdown();
}

void ResourceManager::update()
{
	finishModelLoading();
	finishImageLoading();
	finishSkyboxLoading();
}

Model* ResourceManager::requestModel(const std::string& name)
{
	if (!_models.contains(name))
	{
		_models[name] = std::make_unique<Model>(name);
		loadModel(_models[name].get(), name);
	}
	
	return _models[name].get();
}

void ResourceManager::loadModel(Model* model, const std::string& name)
{
	Logger::Info(fmt::format("Loading model \"{}\"", name));
	
	_threadPool.submit([&, model, name]{
		ModelLoadingData modelData = Model::loadFromFile(name);
		_modelLoadingQueue.enqueue(std::make_pair(model, std::move(modelData)));
	});
}

void ResourceManager::finishModelLoading()
{
	std::pair<Model*, ModelLoadingData> data;
	while (_modelLoadingQueue.try_dequeue(data))
	{
		auto& [model, modelData] = data;
		
		model->finishLoading(modelData);
		Logger::Info(fmt::format("Model \"{}\" loaded", model->getName()));
	}
}

Image* ResourceManager::requestImage(const std::string& name, ImageType type)
{
	if (!_images.contains(name))
	{
		_images[name] = std::make_unique<Image>(name);
		loadImage(_images[name].get(), name, type);
	}
	
	return _images[name].get();
}

void ResourceManager::loadImage(Image* image, const std::string& name, ImageType type)
{
	Logger::Info(fmt::format("Loading image \"{}\"", name));
	
	_threadPool.submit([&, image, name, type]
	{
		ImageLoadingData imageData = Image::loadFromFile(name, type);
		_imageLoadingQueue.enqueue(std::make_pair(image, std::move(imageData)));
	});
}

void ResourceManager::finishImageLoading()
{
	std::pair<Image*, ImageLoadingData> data;
	while (_imageLoadingQueue.try_dequeue(data))
	{
		auto& [image, imageData] = data;
		
		image->finishLoading(imageData);
		
		Logger::Info(fmt::format("Image \"{}\" loaded", image->getName()));
	}
}

Skybox* ResourceManager::requestSkybox(const std::string& name)
{
	if (!_skyboxes.contains(name))
	{
		_skyboxes[name] = std::make_unique<Skybox>(name);
		loadSkybox(_skyboxes[name].get(), name);
	}
	
	return _skyboxes[name].get();
}

void ResourceManager::loadSkybox(Skybox* skybox, const std::string& name)
{
	Logger::Info(fmt::format("Loading skybox \"{}\"", name));
	
	_threadPool.submit([&, skybox, name]
	{
		SkyboxLoadingData skyboxData = Skybox::loadFromFile(name);
		_skyboxLoadingQueue.enqueue(std::make_pair(skybox, std::move(skyboxData)));
	});
}

void ResourceManager::finishSkyboxLoading()
{
	std::pair<Skybox*, SkyboxLoadingData> data;
	while (_skyboxLoadingQueue.try_dequeue(data))
	{
		auto& [skybox, skyboxData] = data;
		
		skybox->finishLoading(skyboxData);
		
		Logger::Info(fmt::format("Skybox \"{}\" loaded", skybox->getName()));
	}
}

ShaderProgram* ResourceManager::requestShaderProgram(const ShaderProgramCreateInfo& createInfo)
{
	if (!_shaderPrograms.contains(createInfo))
	{
		Logger::Info("Loading shader program");
		_shaderPrograms[createInfo] = std::make_unique<ShaderProgram>(createInfo);
		Logger::Info(fmt::format("Shader program loaded (id: {})", _shaderPrograms[createInfo]->getHandle()));
	}
	
	return _shaderPrograms[createInfo].get();
}

MaterialShaderProgram* ResourceManager::requestMaterialShaderProgram(const std::string& layoutName)
{
	if (!_materialShaderPrograms.contains(layoutName))
	{
		_materialShaderPrograms[layoutName] = std::unique_ptr<MaterialShaderProgram>(new MaterialShaderProgram(layoutName, this));
	}
	
	return _materialShaderPrograms[layoutName].get();
}

Material* ResourceManager::requestMaterial(const std::string& name)
{
	if (!_materials.contains(name))
	{
		Logger::Info(fmt::format("Loading material \"{}\"", name));
		_materials[name] = std::unique_ptr<Material>(new Material(name, this));
	}
	
	return _materials[name].get();
}
