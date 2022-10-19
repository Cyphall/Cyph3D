#include "TextureAsset.h"

#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"
#include "Cyph3D/GLObject/GLTexture.h"
#include "Cyph3D/Helper/TextureHelper.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/Asset/AssetManager.h"

#include <format>
#include <magic_enum.hpp>

TextureAsset::TextureAsset(AssetManager& manager, const TextureAssetSignature& signature):
	GPUAsset(manager, signature)
{
	Logger::info(std::format("Loading texture {} with type {}", _signature.path, magic_enum::enum_name(_signature.type)));
	_manager.addMainThreadTask(&TextureAsset::load_step1_mt, this);
}

TextureAsset::~TextureAsset()
{}

const GLTexture& TextureAsset::getGLTexture() const
{
	checkLoaded();
	return *_glTexture;
}

bool TextureAsset::load_step1_mt()
{
	TextureProperties textureProperties = TextureHelper::getTextureProperties(_signature.type);
	
	ImageData imageData = _manager.readImageData(_signature.path, textureProperties.internalFormat);

	TextureCreateInfo textureCreateInfo;
	textureCreateInfo.size = imageData.levels.front().size;
	textureCreateInfo.internalFormat = textureProperties.internalFormat;
	textureCreateInfo.minFilter = GL_LINEAR_MIPMAP_LINEAR;
	textureCreateInfo.magFilter = GL_LINEAR;
	textureCreateInfo.anisotropicFiltering = true;
	textureCreateInfo.swizzle = textureProperties.swizzle;
	textureCreateInfo.levels = imageData.levels.size();

	_glTexture = std::make_unique<GLTexture>(textureCreateInfo);
	for (int i = 0; i < imageData.levels.size(); i++)
	{
		_glTexture->setCompressedData(
			imageData.levels[i].data.data(),
			imageData.levels[i].data.size(),
			imageData.levels[i].size,
			i,
			imageData.levels[i].format
		);
	}

	_loaded = true;
	Logger::info(std::format("Texture {} with type {} loaded", _signature.path, magic_enum::enum_name(_signature.type)));
	
	return true;
}