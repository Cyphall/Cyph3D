#pragma once

#include "glad/glad.h"

class GpuPerfCounter
{
public:
	GpuPerfCounter();
	~GpuPerfCounter();
	
	void start();
	void stop();
	
	double retrieve() const;
	
private:
	GLuint _queryBegin = 0;
	GLuint _queryEnd = 0;
};

