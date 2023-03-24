#include "Timer.h"

void Timer::onNewFrame()
{
	auto currentTime = std::chrono::high_resolution_clock::now();

	_deltaTime = std::chrono::duration<double>(currentTime - _lastFrameTime).count();
	
	_lastFrameTime = currentTime;
}

double Timer::deltaTime() const
{
	return _deltaTime;
}

double Timer::time() const
{
	return std::chrono::duration<double>(std::chrono::steady_clock::now() - _startTime).count();
}