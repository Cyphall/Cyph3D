#include "Skybox.h"

#include "Cyph3D/GLObject/CreateInfo/CubemapCreateInfo.h"
#include "Cyph3D/Helper/JsonHelper.h"
#include "Cyph3D/Helper/TextureHelper.h"
#include "Cyph3D/ResourceManagement/ResourceManager.h"
#include "Cyph3D/ResourceManagement/StbImage.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/GLObject/GLFence.h"
#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/GLObject/GLTexture.h"
#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"

#include <chrono>
#include <format>
#include <array>
#include <filesystem>

struct CubemapFace
{
	std::vector<uint8_t> imageData;
	GLenum format;
	glm::ivec2 size;
};

struct Skybox::LoadData
{
	ResourceManager* rm;

	// step 1: read the image and fill the properties
	std::array<CubemapFace, 6> compressedData;

	// step 2: create GLTexture and fill it
	std::unique_ptr<GLFence> fence;

	// step 3: wait for the fence
};

Skybox::Skybox(const std::string& path, ResourceManager& rm):
	Resource(path)
{
	_loadData = std::make_unique<LoadData>();
	_loadData->rm = &rm;

	Logger::info(std::format("Loading skybox \"{}\"", _name));
	_loadData->rm->addMainThreadTask(&Skybox::load_step1_mt, this);
}

Skybox::~Skybox()
{}

float Skybox::getRotation() const
{
	return _rotation;
}

void Skybox::setRotation(float rotation)
{
	_rotation = rotation;
}

static void writeProcessedCubemapFace(const std::filesystem::path& path, const CubemapFace& cubemapData)
{
	std::filesystem::create_directories(path.parent_path());
	std::ofstream file(path, std::ios::out | std::ios::binary);

	uint8_t version = 1;
	FileHelper::write(file, &version);

	FileHelper::write(file, &cubemapData.size.x);

	FileHelper::write(file, &cubemapData.size.y);
	
	FileHelper::write(file, &cubemapData.format);

	FileHelper::write(file, cubemapData.imageData);
}

static bool readProcessedCubemapFace(const std::filesystem::path& path, CubemapFace& cubemapData)
{
	std::ifstream file(path, std::ios::in | std::ios::binary);

	uint8_t version;
	FileHelper::read(file, &version);

	if (version > 1)
	{
		return false;
	}

	FileHelper::read(file, &cubemapData.size.x);

	FileHelper::read(file, &cubemapData.size.y);
	
	FileHelper::read(file, &cubemapData.format);

	FileHelper::read(file, cubemapData.imageData);

	return true;
}

bool Skybox::load_step1_mt()
{
	std::filesystem::path jsonAbsolutePath = FileHelper::getResourcePath() / _name;

	nlohmann::ordered_json root = JsonHelper::loadJsonFromFile(jsonAbsolutePath);

	std::string facePaths[6] = {
		root["pos_x"].get<std::string>(),
		root["neg_x"].get<std::string>(),
		root["neg_y"].get<std::string>(),
		root["pos_y"].get<std::string>(),
		root["pos_z"].get<std::string>(),
		root["neg_z"].get<std::string>()
	};
	
	for (int i = 0; i < 6; ++i)
	{
		std::filesystem::path absolutePath = FileHelper::getResourcePath() / facePaths[i];
		std::filesystem::path cachePath = FileHelper::getResourceCachePath() / (facePaths[i] + ".c3dcache");
		
		if (!std::filesystem::exists(cachePath) || !readProcessedCubemapFace(cachePath, _loadData->compressedData[i]))
		{
			StbImage imageData(absolutePath);

			if (!imageData.isValid())
			{
				throw std::runtime_error(std::format("Unable to load image {} from disk", facePaths[i]));
			}

			TextureProperties textureProperties = TextureHelper::getTextureProperties(COLOR_SRGB);

			TextureCreateInfo threadTextureCreateInfo;
			threadTextureCreateInfo.size = imageData.getSize();
			threadTextureCreateInfo.internalFormat = textureProperties.internalFormat;

			PixelProperties pixelProperties = TextureHelper::getPixelProperties(imageData.getChannelCount(), imageData.getBitsPerChannel());

			GLTexture threadTexture(threadTextureCreateInfo);
			threadTexture.setData(imageData.getPtr(), 0, pixelProperties.format, pixelProperties.type);

			GLint actualFormat = -1;
			glGetTextureLevelParameteriv(threadTexture.getHandle(), 0, GL_TEXTURE_INTERNAL_FORMAT, &actualFormat);
			_loadData->compressedData[i].format = actualFormat;

			GLint compressedSize = -1;
			glGetTextureLevelParameteriv(threadTexture.getHandle(), 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressedSize);

			_loadData->compressedData[i].imageData.resize(compressedSize);
			glGetCompressedTextureImage(threadTexture.getHandle(), 0, compressedSize, _loadData->compressedData[i].imageData.data());

			_loadData->compressedData[i].size = imageData.getSize();

			writeProcessedCubemapFace(cachePath, _loadData->compressedData[i]);
		}
	}

	glm::ivec2 skyboxSize = _loadData->compressedData[0].size;
	
	for (int i = 1; i < 6; i++)
	{
		if (_loadData->compressedData[i].size != skyboxSize)
		{
			throw std::runtime_error(std::format("Skybox {} have images with different formats", _name));
		}
	}

	TextureProperties textureProperties = TextureHelper::getTextureProperties(COLOR_SRGB);

	CubemapCreateInfo cubemapCreateInfo;
	cubemapCreateInfo.size = skyboxSize;
	cubemapCreateInfo.internalFormat = textureProperties.internalFormat;
	cubemapCreateInfo.swizzle = textureProperties.swizzle;
	cubemapCreateInfo.wrapS = GL_CLAMP_TO_EDGE;
	cubemapCreateInfo.wrapT = GL_CLAMP_TO_EDGE;
	cubemapCreateInfo.wrapR = GL_CLAMP_TO_EDGE;
	cubemapCreateInfo.levels = _loadData->compressedData.size();

	_resource = std::make_unique<GLCubemap>(cubemapCreateInfo);

	for (int i = 0; i < 6; i++)
	{
		_resource->setCompressedData(
			_loadData->compressedData[i].imageData.data(),
			_loadData->compressedData[i].imageData.size(),
			_loadData->compressedData[i].size,
			i,
			0,
			_loadData->compressedData[i].format);
	}

	_loadData->fence = std::make_unique<GLFence>();

	_loadData->rm->addMainThreadTask(&Skybox::load_step2_mt, this);
	return true;
}

bool Skybox::load_step2_mt()
{
	if (!_loadData->fence->isSignaled())
	{
		return false;
	}

	_loadData.reset();
	_ready = true;
	Logger::info(std::format("Skybox \"{}\" loaded", _name));
	return true;
}