#pragma once

#include <glm/glm.hpp>
#include <functional>
#include <memory>
#include <filesystem>

class StbImage
{
public:
	explicit StbImage(const std::filesystem::path& path, int desiredChannels = 0);
	
	const void* getPtr() const;
	uint32_t getBitsPerChannel() const;
	uint32_t getBitsPerPixel() const;
	uint32_t getChannelCount() const;
	glm::uvec2 getSize() const;
	bool isValid() const;
	
private:
	std::unique_ptr<uint8_t[], std::function<void(void*)>> _data8bit;
	std::unique_ptr<uint16_t[], std::function<void(void*)>> _data16bit;
	std::unique_ptr<float[], std::function<void(void*)>> _data32bit;
	
	uint32_t _bitPerChannel = 0;
	uint32_t _channelCount = 0;
	glm::uvec2 _size = {0, 0};
};