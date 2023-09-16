#pragma once

#include <glm/glm.hpp>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>

class StbImage
{
public:
	enum class Channels
	{
		eAny,
		eGrey,
		eGreyAlpha,
		eRedGreenBlue,
		eRedGreenBlueAlpha
	};
	
	enum class BitDepthFlags
	{
		e8 = 1 << 0,
		e16 = 1 << 1,
		e32 = 1 << 2
	};
	
	explicit StbImage(const std::filesystem::path& path, Channels desiredChannels, BitDepthFlags acceptedBitDepths);
	
	const std::byte* getPtr() const;
	uint32_t getBitsPerChannel() const;
	uint32_t getBitsPerPixel() const;
	uint32_t getChannelCount() const;
	glm::uvec2 getSize() const;
	uint64_t getByteSize() const;
	bool isValid() const;
	
private:
	std::unique_ptr<uint8_t[], std::function<void(void*)>> _data8bit;
	std::unique_ptr<uint16_t[], std::function<void(void*)>> _data16bit;
	std::unique_ptr<float[], std::function<void(void*)>> _data32bit;
	
	uint32_t _bitPerChannel = 0;
	uint32_t _channelCount = 0;
	glm::uvec2 _size = {0, 0};
};

inline StbImage::BitDepthFlags operator|(StbImage::BitDepthFlags a, StbImage::BitDepthFlags b)
{
	return static_cast<StbImage::BitDepthFlags>(static_cast<int>(a) | static_cast<int>(b));
}

inline StbImage::BitDepthFlags operator&(StbImage::BitDepthFlags a, StbImage::BitDepthFlags b)
{
	return static_cast<StbImage::BitDepthFlags>(static_cast<int>(a) & static_cast<int>(b));
}