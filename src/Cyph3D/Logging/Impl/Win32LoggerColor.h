#pragma once

#include "Cyph3D/Logging/Impl/ILoggerColor.h"

class Win32LoggerColor : public ILoggerColor
{
public:
	Win32LoggerColor();
	
	void setColor(LogColorFlags color) override;
	void resetColor() override;

private:
	void* _consoleHandle = nullptr;
	uint16_t _defaultConsoleAttributes = 0;
};