#pragma once

#include <map>
#include <unordered_map>
#include "Model.h"
#include "Image.h"
#include "Skybox.h"
#include "../GLObject/CreateInfo/ShaderProgramCreateInfo.h"
#include "../GLObject/ShaderProgram.h"
#include "../GLObject/MaterialShaderProgram.h"
#include "../GLObject/Material.h"
#include <ThreadPool.h>

namespace std
{
	template<>
	struct hash<ShaderProgramCreateInfo>
	{
		size_t operator()(const ShaderProgramCreateInfo& key) const noexcept
		{
			size_t result = 17;

			for (const auto& [shaderType, shaderFiles] : key.shadersFiles)
			{
				result = result * 23 + shaderType;

				for (const auto& file : shaderFiles)
				{
					result = result * 23 + hash<string>{}(file);
				}
			}

			return result;
		}
	};
}

class ResourceManager
{
public:
	explicit ResourceManager(int threadCount);
	~ResourceManager();
	
	void update();
	
	Model* requestModel(const std::string& name);
	Image* requestImage(const std::string& name, ImageType type);
	Skybox* requestSkybox(const std::string& name);
	ShaderProgram* requestShaderProgram(const ShaderProgramCreateInfo& createInfo);
	MaterialShaderProgram* requestMaterialShaderProgram(const std::string& layoutName);
	Material* requestMaterial(const std::string& name);
	
private:
	std::map<std::string, std::unique_ptr<Model>> _models;
	SafeQueue<std::pair<Model*, ModelLoadingData>> _modelLoadingQueue;
	void loadModel(Model* model, const std::string& name);
	void finishModelLoading();
	
	std::map<std::string, std::unique_ptr<Image>> _images;
	SafeQueue<std::pair<Image*, ImageLoadingData>> _imageLoadingQueue;
	void loadImage(Image* image, const std::string& name, ImageType type);
	void finishImageLoading();
	
	std::map<std::string, std::unique_ptr<Skybox>> _skyboxes;
	SafeQueue<std::pair<Skybox*, SkyboxLoadingData>> _skyboxLoadingQueue;
	void loadSkybox(Skybox* skybox, const std::string& name);
	void finishSkyboxLoading();
	
	std::unordered_map<ShaderProgramCreateInfo, std::unique_ptr<ShaderProgram>> _shaderPrograms;
	
	std::map<std::string, std::unique_ptr<MaterialShaderProgram>> _materialShaderPrograms;
	
	std::map<std::string, std::unique_ptr<Material>> _materials;
	
	ThreadPool _threadPool;
};
