#pragma once

#include <map>
#include <unordered_map>
#include "Cyph3D/ResourceManagement/Model.h"
#include "Cyph3D/ResourceManagement/Image.h"
#include "Cyph3D/ResourceManagement/Skybox.h"
#include "Cyph3D/GLObject/CreateInfo/ShaderProgramCreateInfo.h"
#include "Cyph3D/GLObject/ShaderProgram.h"
#include "Cyph3D/GLObject/Material/Material.h"
#include <thread_pool.hpp>

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
	
	Model* requestModel(const std::string& name);
	Image* requestImage(const std::string& name, ImageType type);
	Skybox* requestSkybox(const std::string& name);
	ShaderProgram* requestShaderProgram(const ShaderProgramCreateInfo& createInfo);
	Material* requestMaterial(const std::string& name);
	
private:
	std::map<std::string, std::unique_ptr<Model>> _models;
	
	std::map<std::string, std::unique_ptr<Image>> _images;
	
	std::map<std::string, std::unique_ptr<Skybox>> _skyboxes;
	
	std::unordered_map<ShaderProgramCreateInfo, std::unique_ptr<ShaderProgram>> _shaderPrograms;
	
	std::map<std::string, std::unique_ptr<Material>> _materials;
	
	thread_pool _threadPool;
};