#include "AssetManager.h"

#include <Cyph3D/Engine.h>

#include <CyphGPU/CommandContext.hpp>
#include <CyphGPU/Sampler.hpp>

namespace
{
void threadInit()
{
	BS::this_thread::set_os_thread_priority(BS::os_thread_priority::below_normal);

	c3d::assetCommandContext = std::make_unique<cgpu::CommandContext>(c3d::Engine::getDeviceSession());
}

void threadShutdown()
{
	c3d::assetCommandContext.reset();
}
}

c3d::AssetManager::AssetManager():
	_threadPool(threadInit)
{
	_threadPool.set_cleanup_func(threadShutdown);

	_textureSampler = cgpu::Sampler::create(
		Engine::getDeviceSession(),
		{
			.min_filter = vk::Filter::eLinear,
			.mag_filter = vk::Filter::eLinear,
			.mipmap_mode = vk::SamplerMipmapMode::eLinear,
			.anisotropy = 16.0f,
		}
	);

	_cubemapSampler = cgpu::Sampler::create(
		Engine::getDeviceSession(),
		{
			.min_filter = vk::Filter::eLinear,
			.mag_filter = vk::Filter::eLinear,
			.mipmap_mode = vk::SamplerMipmapMode::eLinear,
			.wrapping_u = vk::SamplerAddressMode::eClampToEdge,
			.wrapping_v = vk::SamplerAddressMode::eClampToEdge,
			.wrapping_w = vk::SamplerAddressMode::eClampToEdge,
			.anisotropy = 16.0f,
		}
	);
}

const cgpu::SamplerPtr& c3d::AssetManager::getTextureSampler()
{
	return _textureSampler;
}

const cgpu::SamplerPtr& c3d::AssetManager::getCubemapSampler()
{
	return _cubemapSampler;
}

c3d::AssetProcessor& c3d::AssetManager::getAssetProcessor()
{
	return _assetProcessor;
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
