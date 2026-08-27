#pragma once

#include <CyphGPU/CommandRecorder.hpp>
#include <glm/glm.hpp>

namespace c3d
{
template<typename TInput, typename TOutput>
class RenderPass
{
public:
	RenderPass(glm::uvec2 size, const char* name):
		_size(size),
		_name(name)
	{
	}

	virtual ~RenderPass() = default;

	TOutput render(cgpu::CommandRecorder& commandRecorder, TInput& input)
	{
		cgpu::ScopedDebugRegion debugRegion{commandRecorder, _name};
		return onRender(commandRecorder, input);
	}

	void resize(glm::uvec2 size)
	{
		_size = size;
		onResize();
	}

protected:
	glm::uvec2 _size;

	virtual TOutput onRender(cgpu::CommandRecorder& commandRecorder, TInput& input) = 0;
	virtual void onResize() = 0;

private:
	const char* _name;
};
}
