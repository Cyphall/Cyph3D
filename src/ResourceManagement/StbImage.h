#pragma once

#include <memory>
#include <string>
#include <glm/glm.hpp>

class StbImage
{
public:
	StbImage();
	explicit StbImage(const std::string& path, int desiredChannels = 0);
	
	void* getPtr() const;
	int getBitPerChannel() const;
	int getBitPerPixel() const;
	int getChannels() const;
	glm::ivec2 getSize() const;
	bool isValid() const;
	
private:
	std::unique_ptr<uint8_t[]> _data8bit;
	std::unique_ptr<uint16_t[]> _data16bit;
	
	int _bitPerChannel = -1;
	int _channels = -1;
	glm::ivec2 _size = glm::ivec2(-1);
};
