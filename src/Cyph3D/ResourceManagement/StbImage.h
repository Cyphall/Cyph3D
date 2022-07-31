#pragma once

#include <glm/glm.hpp>
#include <functional>
#include <memory>
#include <string>

class StbImage
{
public:
	StbImage() = default;
	explicit StbImage(const std::string& path, int desiredChannels = 0);
	
	void* getPtr() const;
	int getBitPerChannel() const;
	int getBitPerPixel() const;
	int getChannelCount() const;
	glm::ivec2 getSize() const;
	bool isValid() const;
	
private:
	std::unique_ptr<uint8_t[], std::function<void(void*)>> _data8bit;
	std::unique_ptr<uint16_t[], std::function<void(void*)>> _data16bit;
	std::unique_ptr<float[], std::function<void(void*)>> _data32bit;
	
	int _bitPerChannel = -1;
	int _channelCount = -1;
	glm::ivec2 _size = glm::ivec2(-1);
};