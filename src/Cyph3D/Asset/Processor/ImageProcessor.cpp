#include "ImageProcessor.h"

#include "Cyph3D/Helper/FileHelper.h"
#include "Cyph3D/StbImage.h"
#include "Cyph3D/Helper/TextureHelper.h"
#include "Cyph3D/GLObject/CreateInfo/TextureCreateInfo.h"
#include "Cyph3D/GLObject/GLTexture.h"
#include "Cyph3D/Logging/Logger.h"

#include <filesystem>

static void writeProcessedImage(const std::filesystem::path& path, const ImageData& imageData)
{
	std::filesystem::create_directories(path.parent_path());
	std::ofstream file(path, std::ios::out | std::ios::binary);

	uint8_t version = 1;
	FileHelper::write(file, &version);

	uint8_t levelCount = imageData.levels.size();
	FileHelper::write(file, &levelCount);

	for (int i = 0; i < levelCount; i++)
	{
		FileHelper::write(file, &imageData.levels[i].format);
		
		FileHelper::write(file, &imageData.levels[i].size.x);

		FileHelper::write(file, &imageData.levels[i].size.y);

		FileHelper::write(file, imageData.levels[i].data);
	}
}

static bool readProcessedImage(const std::filesystem::path& path, ImageData& imageData)
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
	imageData.levels.resize(levelCount);

	for (int i = 0; i < levelCount; i++)
	{
		FileHelper::read(file, &imageData.levels[i].format);
		
		FileHelper::read(file, &imageData.levels[i].size.x);

		FileHelper::read(file, &imageData.levels[i].size.y);

		FileHelper::read(file, imageData.levels[i].data);
	}

	return true;
}

static ImageData processImage(const std::filesystem::path& input, const std::filesystem::path& output, const GLenum& format)
{
	ImageData imageData;
	
	StbImage image(input);

	if (!image.isValid())
	{
		throw std::runtime_error(std::format("Unable to load image {} from disk", input.generic_string()));
	}

	TextureCreateInfo textureCreateInfo;
	textureCreateInfo.size = image.getSize();
	textureCreateInfo.internalFormat = format;
	textureCreateInfo.levels = 0;
	
	GLTexture compressorTexture(textureCreateInfo);

	PixelProperties pixelProperties = TextureHelper::getPixelProperties(image.getChannelCount(), image.getBitsPerChannel());
	compressorTexture.setData(image.getPtr(), 0, pixelProperties.format, pixelProperties.type);

	int levels = compressorTexture.getLevels();
	imageData.levels.resize(levels);
	for (int i = 0; i < levels; i++)
	{
		GLint actualFormat = -1;
		glGetTextureLevelParameteriv(compressorTexture.getHandle(), i, GL_TEXTURE_INTERNAL_FORMAT, &actualFormat);
		imageData.levels[i].format = actualFormat;

		GLint compressedSize = -1;
		glGetTextureLevelParameteriv(compressorTexture.getHandle(), i, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressedSize);
		imageData.levels[i].data.resize(compressedSize);
		
		glGetCompressedTextureImage(compressorTexture.getHandle(), i, compressedSize,imageData.levels[i].data.data());

		glGetTextureLevelParameteriv(compressorTexture.getHandle(), i, GL_TEXTURE_WIDTH, &imageData.levels[i].size.x);
		glGetTextureLevelParameteriv(compressorTexture.getHandle(), i, GL_TEXTURE_HEIGHT, &imageData.levels[i].size.y);
	}

	writeProcessedImage(output, imageData);
	
	return imageData;
}

ImageData ImageProcessor::readImageData(std::string_view path, const GLenum& format, std::string_view cachePath)
{
	std::filesystem::path absolutePath = FileHelper::getAssetDirectoryPath() / path;
	std::filesystem::path cacheAbsolutePath = FileHelper::getCacheAssetDirectoryPath() / cachePath;

	ImageData imageData;
	
	if (std::filesystem::exists(cacheAbsolutePath))
	{
		Logger::info(std::format("Loading image {} with format {} from cache", path, format));
		if (!readProcessedImage(cacheAbsolutePath, imageData))
		{
			Logger::warning(std::format("Cannot parse cached image {} with format {}. Reprocessing...", path, format));
			std::filesystem::remove(cacheAbsolutePath);
			imageData = processImage(absolutePath, cacheAbsolutePath, format);
			Logger::info(std::format("Image {} with format {} reprocessed succesfully", path, format));
		}
	}
	else
	{
		Logger::info(std::format("Processing image {} with format {}", path, format));
		imageData = processImage(absolutePath, cacheAbsolutePath, format);
		Logger::info(std::format("Image {} with format {} processed succesfully", path, format));
	}
	
	return imageData;
}