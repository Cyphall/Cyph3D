#include "Image.h"

#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"
#include "Cyph3D/Helper/TextureHelper.h"
#include "Cyph3D/ResourceManagement/StbImage.h"
#include "Cyph3D/Logging/Logger.h"
#include "Cyph3D/ResourceManagement/ResourceManager.h"
#include "Cyph3D/GLObject/GLFence.h"
#include "Cyph3D/Helper/FileHelper.h"

#include <chrono>
#include <format>
#include <stdexcept>
#include <filesystem>

struct TextureLevel
{
	std::vector<uint8_t> imageData;
	glm::ivec2 size;
	GLenum format;
};

struct Image::LoadData
{
	ResourceManager* rm;

	ImageType type;

	// step 1: read the image and fill the properties
	TextureCreateInfo textureCreateInfo;
	std::vector<TextureLevel> compressedData;

	// step 2: create GLTexture and fill it
	std::unique_ptr<GLFence> fence;

	// step 3: wait for the fence
};

Image::Image(const std::string& name, ImageType type, ResourceManager& rm):
	Resource(name)
{
	_loadData = std::make_unique<LoadData>();
	_loadData->rm = &rm;
	_loadData->type = type;

	Logger::info(std::format("Loading image \"{}\"", getName()));
	_loadData->rm->addMainThreadTask(&Image::load_step1_mt, this);
}

Image::~Image()
{}

static void writeProcessedTexture(const std::filesystem::path& path, const std::vector<TextureLevel>& textureData)
{
	std::filesystem::create_directories(path.parent_path());
	std::ofstream file(path, std::ios::out | std::ios::binary);

	uint8_t version = 1;
	FileHelper::write(file, &version);
	
	uint8_t levelCount = textureData.size();
	FileHelper::write(file, &levelCount);
	
	for (int i = 0; i < levelCount; i++)
	{
		FileHelper::write(file, &textureData[i].format);
		
		FileHelper::write(file, &textureData[i].size.x);
		
		FileHelper::write(file, &textureData[i].size.y);

		FileHelper::write(file, textureData[i].imageData);
	}
}

static bool readProcessedTexture(const std::filesystem::path& path, std::vector<TextureLevel>& textureData)
{
	std::ifstream file(path, std::ios::in | std::ios::binary);

	uint8_t version;
	FileHelper::read(file, &version);

	if (version > 1)
	{
		return false;
	}

	uint8_t levelCount;
	FileHelper::read(file, &levelCount);
	textureData.resize(levelCount);

	for (int i = 0; i < levelCount; i++)
	{
		FileHelper::read(file, &textureData[i].format);

		FileHelper::read(file, &textureData[i].size.x);

		FileHelper::read(file, &textureData[i].size.y);

		FileHelper::read(file, textureData[i].imageData);
	}

	return true;
}

bool Image::load_step1_mt()
{
	std::filesystem::path path = std::format("resources/materials/{}", _name);
	std::filesystem::path processedTexturePath = ("cache" / path).replace_extension(".c3da");

	if (!std::filesystem::exists(processedTexturePath) || !readProcessedTexture(processedTexturePath, _loadData->compressedData))
	{
		StbImage imageData = StbImage(path.generic_string());

		if (!imageData.isValid())
		{
			throw std::runtime_error(std::format("Unable to load image {} from disk", path.generic_string()));
		}

		TextureProperties textureProperties = TextureHelper::getTextureProperties(_loadData->type);

		_loadData->textureCreateInfo.size = imageData.getSize();
		_loadData->textureCreateInfo.internalFormat = textureProperties.internalFormat;
		_loadData->textureCreateInfo.minFilter = GL_LINEAR_MIPMAP_LINEAR;
		_loadData->textureCreateInfo.magFilter = GL_LINEAR;
		_loadData->textureCreateInfo.anisotropicFiltering = true;
		_loadData->textureCreateInfo.swizzle = textureProperties.swizzle;

		PixelProperties pixelProperties = TextureHelper::getPixelProperties(imageData.getChannelCount(), imageData.getBitPerChannel());

		GLTexture compressorTexture(_loadData->textureCreateInfo);
		compressorTexture.setData(imageData.getPtr(), pixelProperties.format, pixelProperties.type);

		int levels = compressorTexture.getLevels();
		_loadData->compressedData.resize(levels);
		for (int i = 0; i < levels; i++)
		{
			GLint actualFormat = -1;
			glGetTextureLevelParameteriv(compressorTexture.getHandle(), i, GL_TEXTURE_INTERNAL_FORMAT, &actualFormat);
			_loadData->compressedData[i].format = actualFormat;

			GLint compressedSize = -1;
			glGetTextureLevelParameteriv(compressorTexture.getHandle(), i, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressedSize);

			_loadData->compressedData[i].imageData.resize(compressedSize);
			glGetCompressedTextureImage(compressorTexture.getHandle(), i, compressedSize, _loadData->compressedData[i].imageData.data());

			glGetTextureLevelParameteriv(compressorTexture.getHandle(), i, GL_TEXTURE_WIDTH, &_loadData->compressedData[i].size.x);
			glGetTextureLevelParameteriv(compressorTexture.getHandle(), i, GL_TEXTURE_HEIGHT, &_loadData->compressedData[i].size.y);
		}

		writeProcessedTexture(processedTexturePath, _loadData->compressedData);
	}

	TextureProperties textureProperties = TextureHelper::getTextureProperties(_loadData->type);

	_loadData->textureCreateInfo.size = _loadData->compressedData[0].size;
	_loadData->textureCreateInfo.internalFormat = textureProperties.internalFormat;
	_loadData->textureCreateInfo.minFilter = GL_LINEAR_MIPMAP_LINEAR;
	_loadData->textureCreateInfo.magFilter = GL_LINEAR;
	_loadData->textureCreateInfo.anisotropicFiltering = true;
	_loadData->textureCreateInfo.swizzle = textureProperties.swizzle;

	_resource = std::make_unique<GLTexture>(_loadData->textureCreateInfo);
	for (int i = 0; i < _loadData->compressedData.size(); i++)
	{
		_resource->setCompressedData(
			_loadData->compressedData[i].imageData.data(),
			_loadData->compressedData[i].imageData.size(),
			_loadData->compressedData[i].size,
			i,
			_loadData->compressedData[i].format
		);
	}

	_loadData->fence = std::make_unique<GLFence>();

	_loadData->rm->addMainThreadTask(&Image::load_step2_mt, this);
	return true;
}

bool Image::load_step2_mt()
{
	if (!_loadData->fence->isSignaled())
	{
		return false;
	}

	_loadData.reset();
	_ready = true;
	Logger::info(std::format("Image \"{}\" loaded", getName()));
	return true;
}