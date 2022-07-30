#pragma once

#include <chrono>

class Timer
{
public:
	void onNewFrame();
	
	double deltaTime() const;
	double time() const;

private:
	std::chrono::time_point<std::chrono::steady_clock> _startTime = std::chrono::steady_clock::now();
	std::chrono::time_point<std::chrono::steady_clock> _lastFrameTime;
	
	bool _firstFrame = true;
	
	double _deltaTime = 0;
};