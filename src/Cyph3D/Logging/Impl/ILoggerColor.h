#pragma once

#include <string_view>
#include "../LogColorFlags.h"

class ILoggerColor
{
public:
	virtual ~ILoggerColor() = default;
	
	virtual void setColor(LogColorFlags color) = 0;
	virtual void resetColor() = 0;
};
