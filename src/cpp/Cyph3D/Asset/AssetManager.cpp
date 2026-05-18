#include "AssetManager.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/Sampler/VKSampler.h"
#include "Cyph3D/VKObject/VKContext.h"

static void threadInit()
{
	BS::this_thread::set_os_thread_priority(BS::os_thread_priority::below_normal);

	c3d::assetGraphicsCommandBuffer = c3d::VKCommandBuffer::create(c3d::Engine::getVKContext(), c3d::Engine::getVKContext().getMainQueue());
	c3d::assetComputeCommandBuffer = c3d::VKCommandBuffer::create(c3d::Engine::getVKContext(), c3d::Engine::getVKContext().getComputeQueue());
	c3d::assetTransferCommandBuffer = c3d::VKCommandBuffer::create(c3d::Engine::getVKContext(), c3d::Engine::getVKContext().getTransferQueue());
}

static void threadShutdown()
{
	c3d::assetGraphicsCommandBuffer.reset();
	c3d::assetComputeCommandBuffer.reset();
	c3d::assetTransferCommandBuffer.reset();
}

c3d::AssetManager::AssetManager():
	_threadPool(threadInit)
{
	_threadPool.set_cleanup_func(threadShutdown);

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

const std::shared_ptr<c3d::VKSampler>& c3d::AssetManager::getTextureSampler()
{
	return _textureSampler;
}

const std::shared_ptr<c3d::VKSampler>& c3d::AssetManager::getCubemapSampler()
{
	return _cubemapSampler;
}

c3d::AssetProcessor& c3d::AssetManager::getAssetProcessor()
{
	return _assetProcessor;
}

c3d::BindlessTextureManager& c3d::AssetManager::getBindlessTextureManager()
{
	return _bindlessTextureManager;
}

c3d::TextureAsset* c3d::AssetManager::loadTexture(std::string_view path, ImageType type)
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

c3d::CubemapAsset* c3d::AssetManager::loadCubemap(std::string_view xposPath, std::string_view xnegPath, std::string_view yposPath, std::string_view ynegPath, std::string_view zposPath, std::string_view znegPath, ImageType type)
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

c3d::CubemapAsset* c3d::AssetManager::loadCubemap(std::string_view equirectangularPath)
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

c3d::MeshAsset* c3d::AssetManager::loadMesh(std::string_view path)
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

c3d::MaterialAsset* c3d::AssetManager::loadMaterial(std::string_view path)
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

c3d::SkyboxAsset* c3d::AssetManager::loadSkybox(std::string_view path)
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

void c3d::AssetManager::onNewFrame()
{
	_bindlessTextureManager.onNewFrame();
}