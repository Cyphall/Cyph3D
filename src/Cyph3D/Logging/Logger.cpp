#include "Logger.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Logging/Impl/Win32LoggerColor.h"
#include "Cyph3D/Helper/FileHelper.h"

#include <iomanip>
#include <iostream>

Logger::LogLevel Logger::_logLevel = LogLevel::DEBUG;
std::mutex Logger::_mtx;
std::unique_ptr<ILoggerColor> Logger::_loggerColor = std::make_unique<Win32LoggerColor>();
std::ofstream Logger::_logFile = FileHelper::openFileForWriting("Cyph3D.log");

static const glm::u8vec3 DEFAULT_LOG_COLOR = {224, 224, 224};

void Logger::print(std::string_view message, std::string_view context, std::string_view prefix, glm::u8vec3 prefixColor)
{
	_mtx.lock();
	
	_loggerColor->setColor(DEFAULT_LOG_COLOR);
	
	std::cout << std::fixed << std::setprecision(4) << Engine::getTimer().time() << " [" << context << "] ";
	_logFile << std::fixed << std::setprecision(4) << Engine::getTimer().time() << " [" << context << "] ";
	
	_loggerColor->setColor(prefixColor);
	
	std::cout << prefix;
	_logFile << prefix;
	
	_loggerColor->setColor(DEFAULT_LOG_COLOR);
	
	std::cout << " > " << message << std::endl;
	_logFile << " > " << message << std::endl;
	
	_loggerColor->resetColor();
	
	_mtx.unlock();
}

void Logger::error(std::string_view message, std::string_view context)
{
	if (_logLevel < LogLevel::ERROR) return;
	
	print(message, context, "ERROR", {255, 51, 102});
}

void Logger::warning(std::string_view message, std::string_view context)
{
	if (_logLevel < LogLevel::WARNING) return;
	
	print(message, context, "WARNING", {255, 204, 85});
}

void Logger::info(std::string_view message, std::string_view context)
{
	if (_logLevel < LogLevel::INFO) return;
	
	print(message, context, "INFO", {22, 198, 12});
}

void Logger::debug(std::string_view message, std::string_view context)
{
	if (_logLevel < LogLevel::DEBUG) return;
	
	print(message, context, "DEBUG", {160, 160, 160});
}

void Logger::setLogLevel(LogLevel logLevel)
{
	_logLevel = logLevel;
}