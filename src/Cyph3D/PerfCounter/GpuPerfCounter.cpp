#include "GpuPerfCounter.h"

GpuPerfCounter::GpuPerfCounter()
{
	glCreateQueries(GL_TIMESTAMP, 1, &_queryBegin);
	glCreateQueries(GL_TIMESTAMP, 1, &_queryEnd);
}

GpuPerfCounter::~GpuPerfCounter()
{
	glDeleteQueries(1, &_queryBegin);
	glDeleteQueries(1, &_queryEnd);
}

void GpuPerfCounter::start()
{
	glQueryCounter(_queryBegin, GL_TIMESTAMP);
}

void GpuPerfCounter::stop()
{
	glQueryCounter(_queryEnd, GL_TIMESTAMP);
}

double GpuPerfCounter::retrieve() const
{
	GLuint64 timeBegin;
	glGetQueryObjectui64v(_queryBegin, GL_QUERY_RESULT, &timeBegin);
	GLuint64 timeEnd;
	glGetQueryObjectui64v(_queryEnd, GL_QUERY_RESULT, &timeEnd);
	
	return static_cast<double>(timeEnd - timeBegin) / 1000000.0;
}