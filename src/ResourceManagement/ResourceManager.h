#pragma once

#include <concurrent_queue.h>
#include <map>
#include <unordered_map>
#include "Model.h"
#include "Image.h"
#include "Skybox.h"
#include "../GLObject/CreateInfo/ShaderProgramCreateInfo.h"
#include "../GLObject/ShaderProgram.h"
#include "../GLObject/MaterialShaderProgram.h"
#include "../GLObject/Material.h"

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
	~ResourceManager();
	
	void update();
	
	Model* requestModel(const std::string& name);
	Image* requestImage(const std::string& name, bool sRGB, bool compressed);
	Skybox* requestSkybox(const std::string& name);
	ShaderProgram* requestShaderProgram(const ShaderProgramCreateInfo& createInfo);
	MaterialShaderProgram* requestMaterialShaderProgram(const std::string& layoutName);
	Material* requestMaterial(const std::string& name);
	
private:
	std::map<std::string, std::unique_ptr<Model>> _models;
	concurrency::concurrent_queue<std::tuple<Model*, ModelLoadingData>> _modelLoadingQueue;
	void loadModel(Model* model, const std::string& name);
	void finishModelLoading();
	
	std::map<std::string, std::unique_ptr<Image>> _images;
	concurrency::concurrent_queue<std::tuple<Image*, ImageLoadingData>> _imageLoadingQueue;
	void loadImage(Image* image, const std::string& name, bool sRGB, bool compressed);
	void finishImageLoading();
	
	std::map<std::string, std::unique_ptr<Skybox>> _skyboxes;
	concurrency::concurrent_queue<std::tuple<Skybox*, SkyboxLoadingData>> _skyboxLoadingQueue;
	void loadSkybox(Skybox* skybox, const std::string& name);
	void finishSkyboxLoading();
	
	std::unordered_map<ShaderProgramCreateInfo, std::unique_ptr<ShaderProgram>> _shaderPrograms;
	
	std::map<std::string, std::unique_ptr<MaterialShaderProgram>> _materialShaderPrograms;
	
	std::map<std::string, std::unique_ptr<Material>> _materials;
};
