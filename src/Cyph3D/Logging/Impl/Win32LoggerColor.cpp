#include "Win32LoggerColor.h"

#include <format>
#include <string>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static const std::string DEFAULT_COLOR_SEQUENCE = "\x1b[0m";

void Win32LoggerColor::setColor(glm::u8vec3 color)
{
	std::string colorSequence = std::format("\x1b[38;2;{};{};{}m", color.r, color.g, color.b);
	WriteConsole(_consoleHandle, colorSequence.c_str(), colorSequence.size(), nullptr, nullptr);
}

void Win32LoggerColor::resetColor()
{
	WriteConsole(_consoleHandle, DEFAULT_COLOR_SEQUENCE.c_str(), DEFAULT_COLOR_SEQUENCE.size(), nullptr, nullptr);
}

Win32LoggerColor::Win32LoggerColor()
{
	_consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	DWORD mode;
	GetConsoleMode(_consoleHandle, &mode);

	mode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(_consoleHandle, mode);
}