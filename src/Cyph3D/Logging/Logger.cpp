#include "Logger.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Logging/Impl/Win32LoggerColor.h"

#include <iomanip>
#include <iostream>

Logger::LogLevel Logger::_logLevel = LogLevel::FULL;
std::mutex Logger::_mtx;
std::unique_ptr<ILoggerColor> Logger::_loggerColor = std::make_unique<Win32LoggerColor>();
std::ofstream Logger::_logFile = std::ofstream("Cyph3D.log", std::ios::out);

void Logger::print(std::string_view message, std::string_view context, std::string_view prefix, LogColorFlags color)
{
	_mtx.lock();
	
	std::cout << std::fixed << std::setprecision(4) << Engine::getTimer().time() << ' ';
	_logFile << std::fixed << std::setprecision(4) << Engine::getTimer().time() << ' ';
	
	if (!context.empty())
	{
		std::cout << '[' << context << "] ";
		_logFile << '[' << context << "] ";
	}
	
	_loggerColor->setColor(color);
	std::cout << prefix;
	_logFile << prefix;
	_loggerColor->resetColor();
	
	std::cout << " > " << message << std::endl;
	_logFile << " > " << message << std::endl;
	_mtx.unlock();
}

void Logger::error(std::string_view message, std::string_view context)
{
	if (_logLevel < LogLevel::ERROR) return;
	
	print(message, context, "ERROR", LogColorFlags::FOREGROUND_RED | LogColorFlags::FOREGROUND_INTENSITY);
	__debugbreak();
}

void Logger::warning(std::string_view message, std::string_view context)
{
	if (_logLevel < LogLevel::WARNING) return;
	
	print(message, context, "WARN ", LogColorFlags::FOREGROUND_RED | LogColorFlags::FOREGROUND_GREEN);
}

void Logger::info(std::string_view message, std::string_view context)
{
	if (_logLevel < LogLevel::FULL) return;
	
	print(message, context, "INFO ", LogColorFlags::FOREGROUND_GREEN | LogColorFlags::FOREGROUND_INTENSITY);
}

void Logger::setLogLevel(LogLevel logLevel)
{
	_logLevel = logLevel;
}