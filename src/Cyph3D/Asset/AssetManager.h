#pragma once

#include "Cyph3D/Asset/AssetManagerWorkerData.h"
#include "Cyph3D/Asset/BindlessTextureManager.h"
#include "Cyph3D/Asset/Processing/AssetProcessor.h"
#include "Cyph3D/Asset/Processing/ImageData.h"
#include "Cyph3D/Asset/Processing/MeshData.h"
#include "Cyph3D/Asset/RuntimeAsset/TextureAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/CubemapAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/MeshAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/MaterialAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/SkyboxAsset.h"

#include <BS_thread_pool.hpp>
#include <unordered_map>
#include <list>
#include <functional>
#include <mutex>
#include <array>

class VKSampler;

class AssetManager
{
public:
	explicit AssetManager(int threadCount);
	~AssetManager();
	
	const VKPtr<VKSampler>& getTextureSampler();
	const VKPtr<VKSampler>& getCubemapSampler();
	
	AssetProcessor& getAssetProcessor();
	
	BindlessTextureManager& getBindlessTextureManager();

	TextureAsset* loadTexture(std::string_view path, ImageType type);
	CubemapAsset* loadCubemap(std::string_view xposPath, std::string_view xnegPath, std::string_view yposPath, std::string_view ynegPath, std::string_view zposPath, std::string_view znegPath, ImageType type);
	MeshAsset* loadMesh(std::string_view path);
	MaterialAsset* loadMaterial(std::string_view path);
	SkyboxAsset* loadSkybox(std::string_view path);

	void onNewFrame();
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
	AssetProcessor _assetProcessor;
	
	VKPtr<VKSampler> _textureSampler;
	VKPtr<VKSampler> _cubemapSampler;
	
	BindlessTextureManager _bindlessTextureManager;

	std::unordered_map<TextureAssetSignature, std::unique_ptr<TextureAsset>> _textures;
	std::unordered_map<CubemapAssetSignature, std::unique_ptr<CubemapAsset>> _cubemaps;
	std::unordered_map<MeshAssetSignature, std::unique_ptr<MeshAsset>> _meshes;
	std::unordered_map<MaterialAssetSignature, std::unique_ptr<MaterialAsset>> _materials;
	std::unordered_map<SkyboxAssetSignature, std::unique_ptr<SkyboxAsset>> _skyboxes;

	BS::thread_pool<AssetManagerWorkerData> _threadPool;

	std::list<std::function<bool()>> _mainThreadTasks;

	std::mutex _mainThreadTasksMutex;
};