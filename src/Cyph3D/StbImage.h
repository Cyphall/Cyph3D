#pragma once

#include <glm/glm.hpp>
#include <functional>
#include <memory>
#include <filesystem>
#include <cstddef>

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
	
	enum class BitDepth
	{
		e8,
		e16,
		e32
	};
	
	explicit StbImage(const std::filesystem::path& path, Channels desiredChannels = Channels::eAny, BitDepth maxBitDepth = BitDepth::e32);
	
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