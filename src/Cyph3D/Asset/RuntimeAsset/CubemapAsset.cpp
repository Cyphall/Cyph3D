#include "CubemapAsset.h"

#include "Cyph3D/GLObject/CreateInfo/CubemapCreateInfo.h"
#include "Cyph3D/GLObject/GLCubemap.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/Asset/AssetManager.h"

#include <format>

CubemapAsset::CubemapAsset(AssetManager& manager, const CubemapAssetSignature& signature):
	GPUAsset(manager, signature)
{
	Logger::info(std::format("Loading cubemap (xpos: {}, xneg: {}, ypos: {}, yneg: {}, zpos: {}, zneg: {})",
		_signature.xposPath,
		_signature.xnegPath,
		_signature.yposPath,
		_signature.ynegPath,
		_signature.zposPath,
		_signature.znegPath));
	_manager.addMainThreadTask(&CubemapAsset::load_step1_mt, this);
}

CubemapAsset::~CubemapAsset()
{}

const GLCubemap& CubemapAsset::getGLCubemap() const
{
	checkLoaded();
	return *_glCubemap;
}

bool CubemapAsset::load_step1_mt()
{
	GLenum internalFormat = GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;
	
	std::reference_wrapper<std::string> paths[6] = {
		_signature.xposPath,
		_signature.xnegPath,
		_signature.yposPath,
		_signature.ynegPath,
		_signature.zposPath,
		_signature.znegPath,
	};
	
	glm::ivec2 size;
	int levelCount;
	std::array<ImageData, 6> imageDataList;
	for (int i = 0; i < 6; i++)
	{
		imageDataList[i] = _manager.readImageData(paths[i].get(), internalFormat);
		
		if (i == 0)
		{
			size = imageDataList[i].levels.front().size;
			levelCount = imageDataList[i].levels.size();
		}
		else
		{
			if (size != imageDataList[i].levels.front().size)
				throw;
			if (levelCount != imageDataList[i].levels.size())
				throw;
		}
	}

	CubemapCreateInfo cubemapCreateInfo;
	cubemapCreateInfo.size = size;
	cubemapCreateInfo.internalFormat = internalFormat;
	cubemapCreateInfo.wrapS = GL_CLAMP_TO_EDGE;
	cubemapCreateInfo.wrapT = GL_CLAMP_TO_EDGE;
	cubemapCreateInfo.wrapR = GL_CLAMP_TO_EDGE;
	cubemapCreateInfo.levels = levelCount;

	_glCubemap = std::make_unique<GLCubemap>(cubemapCreateInfo);
	for (int face = 0; face < 6; face++)
	{
		for (int level = 0; level < levelCount; level++)
		{
			ImageLevel& levelData = imageDataList[face].levels[level];
			
			_glCubemap->setCompressedData(
				levelData.data.data(),
				levelData.data.size(),
				levelData.size,
				face,
				level,
				levelData.format);
		}
	}

	_loaded = true;
	Logger::info(std::format("Cubemap (xpos: {}, xneg: {}, ypos: {}, yneg: {}, zpos: {}, zneg: {}) loaded",
		_signature.xposPath,
		_signature.xnegPath,
		_signature.yposPath,
		_signature.ynegPath,
		_signature.zposPath,
		_signature.znegPath));

	return true;
}