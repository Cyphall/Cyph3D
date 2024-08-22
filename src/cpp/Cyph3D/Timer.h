#pragma once

#include <chrono>

class Timer
{
public:
	void onNewFrame();

	double deltaTime() const;
	double time() const;

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> _startTime = std::chrono::high_resolution_clock::now();
	std::chrono::time_point<std::chrono::high_resolution_clock> _lastFrameTime = _startTime;

	double _deltaTime = 0;
};