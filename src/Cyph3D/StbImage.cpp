#include "StbImage.h"

#include <stb_image.h>
#include <string>

StbImage::StbImage(const std::filesystem::path& path, Channels desiredChannels, BitDepth maxBitDepth):
_data8bit(nullptr, stbi_image_free),
_data16bit(nullptr, stbi_image_free),
_data32bit(nullptr, stbi_image_free)
{
	std::string pathStr = path.generic_string();
	
	int width;
	int height;
	int channelCount;
	
	bool is32Bit = stbi_is_hdr(pathStr.c_str());
	bool is16Bit = stbi_is_16_bit(pathStr.c_str());
	bool is8Bit = !is32Bit && !is16Bit;
	
	if (maxBitDepth >= BitDepth::e32 && is32Bit)
	{
		_bitPerChannel = 32;
		_data32bit.reset(stbi_loadf(pathStr.c_str(), &width, &height, &channelCount, static_cast<int>(desiredChannels)));
	}
	else if (maxBitDepth >= BitDepth::e16 && is16Bit)
	{
		_bitPerChannel = 16;
		_data16bit.reset(stbi_load_16(pathStr.c_str(), &width, &height, &channelCount, static_cast<int>(desiredChannels)));
	}
	else if (maxBitDepth >= BitDepth::e8 && (is8Bit || is16Bit))
	{
		_bitPerChannel = 8;
		_data8bit.reset(stbi_load(pathStr.c_str(), &width, &height, &channelCount, static_cast<int>(desiredChannels)));
	}
	else
	{
		throw;
	}
	
	_size = {width, height};
	_channelCount = channelCount;
}

const std::byte* StbImage::getPtr() const
{
	switch (_bitPerChannel)
	{
		case 8:
			return reinterpret_cast<std::byte*>(_data8bit.get());
		case 16:
			return reinterpret_cast<std::byte*>(_data16bit.get());
		case 32:
			return reinterpret_cast<std::byte*>(_data32bit.get());
	}
	
	return nullptr;
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

uint64_t StbImage::getByteSize() const
{
	return _size.x * _size.y * (getBitsPerPixel() / 8);
}

bool StbImage::isValid() const
{
	return _data8bit != nullptr || _data16bit != nullptr || _data32bit != nullptr;
}