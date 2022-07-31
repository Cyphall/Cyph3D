#pragma once

#include "Cyph3D/Logging/LogColorFlags.h"

class ILoggerColor
{
public:
	virtual ~ILoggerColor() = default;
	
	virtual void setColor(LogColorFlags color) = 0;
	virtual void resetColor() = 0;
};