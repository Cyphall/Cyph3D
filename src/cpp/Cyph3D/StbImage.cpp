#include "StbImage.h"

#include <stb_image.h>
#include <string>

StbImage::StbImage(const std::filesystem::path& path, Channels desiredChannels, BitDepthFlags acceptedBitDepths)
{
	std::string pathStr = path.generic_string();

	bool is32Bit = stbi_is_hdr(pathStr.c_str());
	bool is16Bit = stbi_is_16_bit(pathStr.c_str());
	bool is8Bit = !is32Bit && !is16Bit;

	int width = 0;
	int height = 0;
	int channelCount = 0;
	if ((acceptedBitDepths & BitDepthFlags::e32) == BitDepthFlags::e32 && is32Bit)
	{
		_bitPerChannel = 32;
		_data = reinterpret_cast<std::byte*>(stbi_loadf(pathStr.c_str(), &width, &height, &channelCount, static_cast<int>(desiredChannels)));
	}
	else if ((acceptedBitDepths & BitDepthFlags::e16) == BitDepthFlags::e16 && is16Bit)
	{
		_bitPerChannel = 16;
		_data = reinterpret_cast<std::byte*>(stbi_load_16(pathStr.c_str(), &width, &height, &channelCount, static_cast<int>(desiredChannels)));
	}
	else if ((acceptedBitDepths & BitDepthFlags::e8) == BitDepthFlags::e8 && (is8Bit || is16Bit))
	{
		_bitPerChannel = 8;
		_data = reinterpret_cast<std::byte*>(stbi_load(pathStr.c_str(), &width, &height, &channelCount, static_cast<int>(desiredChannels)));
	}
	else
	{
		throw;
	}

	_size = {width, height};
	_channelCount = desiredChannels == Channels::eAny ? channelCount : static_cast<int>(desiredChannels);
}

StbImage::~StbImage()
{
	if (_data != nullptr)
		stbi_image_free(_data);
}

const std::byte* StbImage::getPtr() const
{
	return _data;
}

uint32_t StbImage::getBitsPerChannel() const
{
	return _bitPerChannel;
}

uint32_t StbImage::getBitsPerPixel() const
{
	return _bitPerChannel * _channelCount;
}

uint32_t StbImage::getChannelCount() const
{
	return _channelCount;
}

glm::uvec2 StbImage::getSize() const
{
	return _size;
}

size_t StbImage::getByteSize() const
{
	return _size.x * _size.y * (getBitsPerPixel() / 8);
}

bool StbImage::isValid() const
{
	return _data != nullptr;
}