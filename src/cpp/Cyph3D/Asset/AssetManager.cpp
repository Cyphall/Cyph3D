#include "AssetManager.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/Sampler/VKSampler.h"
#include "Cyph3D/VKObject/VKContext.h"

static AssetManagerWorkerData threadInit()
{
	return {
		.graphicsCommandBuffer = VKCommandBuffer::create(Engine::getVKContext(), Engine::getVKContext().getMainQueue()),
		.computeCommandBuffer = VKCommandBuffer::create(Engine::getVKContext(), Engine::getVKContext().getComputeQueue()),
		.transferCommandBuffer = VKCommandBuffer::create(Engine::getVKContext(), Engine::getVKContext().getTransferQueue())
	};
}

AssetManager::AssetManager(int threadCount):
	_threadPool(threadInit, threadCount)
{
	{
		vk::SamplerCreateInfo createInfo;
		createInfo.flags = {};
		createInfo.magFilter = vk::Filter::eLinear;
		createInfo.minFilter = vk::Filter::eLinear;
		createInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		createInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
		createInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
		createInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
		createInfo.mipLodBias = 0.0f;
		createInfo.anisotropyEnable = true;
		createInfo.maxAnisotropy = 16;
		createInfo.compareEnable = false;
		createInfo.compareOp = vk::CompareOp::eNever;
		createInfo.minLod = -1000.0f;
		createInfo.maxLod = 1000.0f;
		createInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		createInfo.unnormalizedCoordinates = false;

		_textureSampler = VKSampler::create(Engine::getVKContext(), createInfo);
	}

	{
		vk::SamplerCreateInfo createInfo;
		createInfo.flags = {};
		createInfo.magFilter = vk::Filter::eLinear;
		createInfo.minFilter = vk::Filter::eLinear;
		createInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		createInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
		createInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
		createInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
		createInfo.mipLodBias = 0.0f;
		createInfo.anisotropyEnable = true;
		createInfo.maxAnisotropy = 16;
		createInfo.compareEnable = false;
		createInfo.compareOp = vk::CompareOp::eNever;
		createInfo.minLod = -1000.0f;
		createInfo.maxLod = 1000.0f;
		createInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		createInfo.unnormalizedCoordinates = false;

		_cubemapSampler = VKSampler::create(Engine::getVKContext(), createInfo);
	}
}

AssetManager::~AssetManager()
{
	_threadPool.pause();
	_threadPool.wait_for_tasks();
}

const std::shared_ptr<VKSampler>& AssetManager::getTextureSampler()
{
	return _textureSampler;
}

const std::shared_ptr<VKSampler>& AssetManager::getCubemapSampler()
{
	return _cubemapSampler;
}

AssetProcessor& AssetManager::getAssetProcessor()
{
	return _assetProcessor;
}

BindlessTextureManager& AssetManager::getBindlessTextureManager()
{
	return _bindlessTextureManager;
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

CubemapAsset* AssetManager::loadCubemap(std::string_view xposPath, std::string_view xnegPath, std::string_view yposPath, std::string_view ynegPath, std::string_view zposPath, std::string_view znegPath, ImageType type)
{
	CubemapAssetSignature signature;
	signature.xposPath = xposPath;
	signature.xnegPath = xnegPath;
	signature.yposPath = yposPath;
	signature.ynegPath = ynegPath;
	signature.zposPath = zposPath;
	signature.znegPath = znegPath;
	signature.type = type;

	auto it = _cubemaps.find(signature);
	if (it == _cubemaps.end())
	{
		it = _cubemaps.try_emplace(signature, std::unique_ptr<CubemapAsset>(new CubemapAsset(*this, signature))).first;
	}

	return it->second.get();
}

CubemapAsset* AssetManager::loadCubemap(std::string_view equirectangularPath)
{
	CubemapAssetSignature signature;
	signature.equirectangularPath = equirectangularPath;

	auto it = _cubemaps.find(signature);
	if (it == _cubemaps.end())
	{
		it = _cubemaps.try_emplace(signature, std::unique_ptr<CubemapAsset>(new CubemapAsset(*this, signature))).first;
	}

	return it->second.get();
}

MeshAsset* AssetManager::loadMesh(std::string_view path)
{
	MeshAssetSignature signature;
	signature.path = path;

	auto it = _meshes.find(signature);
	if (it == _meshes.end())
	{
		it = _meshes.try_emplace(signature, std::unique_ptr<MeshAsset>(new MeshAsset(*this, signature))).first;
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

void AssetManager::onNewFrame()
{
	_bindlessTextureManager.onNewFrame();
}