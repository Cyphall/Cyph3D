#pragma once

#include "Cyph3D/Asset/AssetCacheDatabase.h"
#include "Cyph3D/Asset/Processor/ImageData.h"
#include "Cyph3D/Asset/Processor/MeshData.h"
#include "Cyph3D/Asset/RuntimeAsset/TextureAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/CubemapAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/ModelAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/MaterialAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/SkyboxAsset.h"

#include <BS_thread_pool.hpp>
#include <unordered_map>
#include <list>
#include <functional>
#include <mutex>
#include <array>

class AssetManager
{
public:
	explicit AssetManager(int threadCount);
	~AssetManager();
	
	ImageData readImageData(std::string_view path, const GLenum& format);
	MeshData readMeshData(std::string_view path);

	TextureAsset* loadTexture(std::string_view path, TextureType type);
	CubemapAsset* loadCubemap(std::string_view xposPath, std::string_view xnegPath, std::string_view yposPath, std::string_view ynegPath, std::string_view zposPath, std::string_view znegPath);
	ModelAsset* loadModel(std::string_view path);
	MaterialAsset* loadMaterial(std::string_view path);
	SkyboxAsset* loadSkybox(std::string_view path);

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
	AssetCacheDatabase _database;

	std::unordered_map<TextureAssetSignature, std::unique_ptr<TextureAsset>> _textures;
	std::unordered_map<CubemapAssetSignature, std::unique_ptr<CubemapAsset>> _cubemaps;
	std::unordered_map<ModelAssetSignature, std::unique_ptr<ModelAsset>> _models;
	std::unordered_map<MaterialAssetSignature, std::unique_ptr<MaterialAsset>> _materials;
	std::unordered_map<SkyboxAssetSignature, std::unique_ptr<SkyboxAsset>> _skyboxes;

	BS::thread_pool _threadPool;

	std::list<std::function<bool()>> _mainThreadTasks;

	std::mutex _mainThreadTasksMutex;
};