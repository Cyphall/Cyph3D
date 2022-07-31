#pragma once

#include "Cyph3D/Logging/LogColorFlags.h"

#include <string_view>

class ILoggerColor
{
public:
	virtual ~ILoggerColor() = default;
	
	virtual void setColor(LogColorFlags color) = 0;
	virtual void resetColor() = 0;
};