#include "StbImage.h"
#include "stb_image.h"

StbImage::StbImage(const std::string& path, int desiredChannels):
_data8bit(nullptr, stbi_image_free),
_data16bit(nullptr, stbi_image_free),
_data32bit(nullptr, stbi_image_free)
{
	if (stbi_is_hdr(path.c_str()))
	{
		_bitPerChannel = 32;
		_data32bit.reset(stbi_loadf(path.c_str(), &_size.x, &_size.y, &_channelCount, desiredChannels));
	}
	else if (stbi_is_16_bit(path.c_str()))
	{
		_bitPerChannel = 16;
		_data16bit.reset(stbi_load_16(path.c_str(), &_size.x, &_size.y, &_channelCount, desiredChannels));
	}
	else
	{
		_bitPerChannel = 8;
		_data8bit.reset(stbi_load(path.c_str(), &_size.x, &_size.y, &_channelCount, desiredChannels));
	}
}

void* StbImage::getPtr() const
{
	switch (_bitPerChannel)
	{
		case 8:
			return _data8bit.get();
		case 16:
			return _data16bit.get();
		case 32:
			return _data32bit.get();
	}
	
	return nullptr;
}

int StbImage::getBitPerChannel() const
{
	return _bitPerChannel;
}

int StbImage::getBitPerPixel() const
{
	return _bitPerChannel * _channelCount;
}

int StbImage::getChannelCount() const
{
	return _channelCount;
}

glm::ivec2 StbImage::getSize() const
{
	return _size;
}

bool StbImage::isValid() const
{
	return _data8bit.get() != nullptr || _data16bit.get() != nullptr || _data32bit.get() != nullptr;
}
