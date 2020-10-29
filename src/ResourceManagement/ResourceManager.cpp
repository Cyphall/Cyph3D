#include "ResourceManager.h"
#include "../Logger.h"
#include <stb_image.h>
#include <fmt/core.h>

ResourceManager::~ResourceManager()
{
	{
		std::tuple<Image*, ImageLoadingData> data;
		while (_imageLoadingQueue.try_pop(data))
		{
			stbi_image_free(std::get<1>(data).data);
		}
	}
	{
		std::tuple<Skybox*, SkyboxLoadingData> data;
		while (_skyboxLoadingQueue.try_pop(data))
		{
			SkyboxLoadingData& loadingData = std::get<1>(data);
			for (uint8_t* ptr : loadingData.data)
			{
				stbi_image_free(ptr);
			}
		}
	}
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
	
	// threaded
	ModelLoadingData modelData = Model::loadFromFile(name);
	_modelLoadingQueue.push(std::make_tuple(model, modelData));
	// end threaded
}

void ResourceManager::finishModelLoading()
{
	std::tuple<Model*, ModelLoadingData> data;
	while (_modelLoadingQueue.try_pop(data))
	{
		Model* model = std::get<0>(data);
		ModelLoadingData& modelData = std::get<1>(data);
		model->finishLoading(modelData);
		Logger::Info(fmt::format("Model \"{}\" loaded", model->getName()));
	}
}

Image* ResourceManager::requestImage(const std::string& name, bool sRGB, bool compressed)
{
	if (!_images.contains(name))
	{
		_images[name] = std::make_unique<Image>(name);
		loadImage(_images[name].get(), name, sRGB, compressed);
	}
	
	return _images[name].get();
}

void ResourceManager::loadImage(Image* image, const std::string& name, bool sRGB, bool compressed)
{
	Logger::Info(fmt::format("Loading image \"{}\"", name));
	
	// threaded
	ImageLoadingData imageData = Image::loadFromFile(name, sRGB, compressed);
	_imageLoadingQueue.push(std::make_tuple(image, imageData));
	// end threaded
}

void ResourceManager::finishImageLoading()
{
	std::tuple<Image*, ImageLoadingData> data;
	while (_imageLoadingQueue.try_pop(data))
	{
		Image* image = std::get<0>(data);
		ImageLoadingData& imageData = std::get<1>(data);
		
		image->finishLoading(imageData);
		stbi_image_free(imageData.data);
		
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
	
	// threaded
	SkyboxLoadingData skyboxData = Skybox::loadFromFile(name);
	_skyboxLoadingQueue.push(std::make_tuple(skybox, skyboxData));
	// end threaded
}

void ResourceManager::finishSkyboxLoading()
{
	std::tuple<Skybox*, SkyboxLoadingData> data;
	while (_skyboxLoadingQueue.try_pop(data))
	{
		Skybox* skybox = std::get<0>(data);
		SkyboxLoadingData& skyboxData = std::get<1>(data);
		
		skybox->finishLoading(skyboxData);
		for (uint8_t* ptr : skyboxData.data)
		{
			stbi_image_free(ptr);
		}
		
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
