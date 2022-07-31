#include "Win32LoggerColor.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void Win32LoggerColor::setColor(LogColorFlags color)
{
	uint16_t attributes = _defaultConsoleAttributes & 0b1111111111110000;
	SetConsoleTextAttribute(_consoleHandle, attributes | static_cast<uint16_t>(color));
}

void Win32LoggerColor::resetColor()
{
	SetConsoleTextAttribute(_consoleHandle, _defaultConsoleAttributes);
}

Win32LoggerColor::Win32LoggerColor()
{
	_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(_consoleHandle, &info);
	_defaultConsoleAttributes = info.wAttributes;
}