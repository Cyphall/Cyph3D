#include "AssetManager.h"

#include "Cyph3D/Asset/Processor/ImageProcessor.h"
#include "Cyph3D/Asset/Processor/MeshProcessor.h"

AssetManager::AssetManager(int threadCount):
	_threadPool(threadCount)
{

}

AssetManager::~AssetManager()
{}

ImageData AssetManager::readImageData(std::string_view path, ImageType type)
{
	std::string cachePath = _database.getImageCachePath(path, type);
	return ImageProcessor::readImageData(path, type, cachePath);
}

MeshData AssetManager::readMeshData(std::string_view path)
{
	std::string cachePath = _database.getMeshCachePath(path);
	return MeshProcessor::readMeshData(path, cachePath);
}

TextureAsset* AssetManager::loadTexture(std::string_view path, ImageType type)
{
	TextureAssetSignature signature;
	signature.path = path;
	signature.type = type;
	
	auto it = _textures.find(signature);
	if (it == _textures.end())
	{
		it = _textures.try_emplace(signature, std::unique_ptr<TextureAsset>(new TextureAsset(*this, signature))).first;
	}

	return it->second.get();
}

CubemapAsset* AssetManager::loadCubemap(std::string_view xposPath, std::string_view xnegPath, std::string_view yposPath, std::string_view ynegPath, std::string_view zposPath, std::string_view znegPath)
{
	CubemapAssetSignature signature;
	signature.xposPath = xposPath;
	signature.xnegPath = xnegPath;
	signature.yposPath = yposPath;
	signature.ynegPath = ynegPath;
	signature.zposPath = zposPath;
	signature.znegPath = znegPath;

	auto it = _cubemaps.find(signature);
	if (it == _cubemaps.end())
	{
		it = _cubemaps.try_emplace(signature, std::unique_ptr<CubemapAsset>(new CubemapAsset(*this, signature))).first;
	}

	return it->second.get();
}

ModelAsset* AssetManager::loadModel(std::string_view path)
{
	ModelAssetSignature signature;
	signature.path = path;

	auto it = _models.find(signature);
	if (it == _models.end())
	{
		it = _models.try_emplace(signature, std::unique_ptr<ModelAsset>(new ModelAsset(*this, signature))).first;
	}

	return it->second.get();
}

MaterialAsset* AssetManager::loadMaterial(std::string_view path)
{
	MaterialAssetSignature signature;
	signature.path = path;

	auto it = _materials.find(signature);
	if (it == _materials.end())
	{
		it = _materials.try_emplace(signature, std::unique_ptr<MaterialAsset>(new MaterialAsset(*this, signature))).first;
	}

	return it->second.get();
}

SkyboxAsset* AssetManager::loadSkybox(std::string_view path)
{
	SkyboxAssetSignature signature;
	signature.path = path;

	auto it = _skyboxes.find(signature);
	if (it == _skyboxes.end())
	{
		it = _skyboxes.try_emplace(signature, std::unique_ptr<SkyboxAsset>(new SkyboxAsset(*this, signature))).first;
	}

	return it->second.get();
}

void AssetManager::onUpdate()
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