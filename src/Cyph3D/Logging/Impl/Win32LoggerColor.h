#pragma once

#include "Cyph3D/Logging/Impl/ILoggerColor.h"

class Win32LoggerColor : public ILoggerColor
{
public:
	Win32LoggerColor();
	
	void setColor(glm::u8vec3 color) override;
	void resetColor() override;

private:
	void* _consoleHandle = nullptr;
};