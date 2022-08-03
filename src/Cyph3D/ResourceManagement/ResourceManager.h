#pragma once

#include "Cyph3D/Enums/ImageType.h"
#include "Cyph3D/GLObject/CreateInfo/ShaderProgramCreateInfo.h"

#include <BS_thread_pool.hpp>
#include <unordered_map>
#include <list>
#include <functional>
#include <mutex>

class Model;
class Image;
class Skybox;
class GLShaderProgram;
class Material;

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
	
	Model* requestModel(const std::string& name);
	Image* requestImage(const std::string& name, ImageType type);
	Skybox* requestSkybox(const std::string& name);
	GLShaderProgram* requestShaderProgram(const ShaderProgramCreateInfo& createInfo);
	Material* requestMaterial(const std::string& name);
	
	void onUpdate();
	
	template <typename TTask, typename... TArgs>
	void addThreadPoolTask(TTask&& task, TArgs&&... args)
	{
		_threadPool.push_task(std::forward<TTask>(task), std::forward<TArgs>(args)...);
	}

	template <typename TTask, typename... TArgs>
	void addMainThreadTask(TTask&& task, TArgs&&... args)
	{
		_mainThreadTasksMutex.lock();
		_mainThreadTasks.push_back(std::bind(std::forward<TTask>(task), std::forward<TArgs>(args)...));
		_mainThreadTasksMutex.unlock();
	}
	
private:
	std::unordered_map<std::string, std::unique_ptr<Model>> _models;
	
	std::unordered_map<std::string, std::unique_ptr<Image>> _images;
	
	std::unordered_map<std::string, std::unique_ptr<Skybox>> _skyboxes;
	
	std::unordered_map<ShaderProgramCreateInfo, std::unique_ptr<GLShaderProgram>> _shaderPrograms;
	
	std::unordered_map<std::string, std::unique_ptr<Material>> _materials;
	
	BS::thread_pool _threadPool;
	
	std::list<std::function<bool()>> _mainThreadTasks;
	
	std::mutex _mainThreadTasksMutex;
};