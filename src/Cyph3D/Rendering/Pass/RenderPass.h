#pragma once

#include "Cyph3D/VKObject/CommandBuffer/VKCommandBuffer.h"
#include "Cyph3D/VKObject/VKPtr.h"

#include <glm/glm.hpp>

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

	TOutput render(const VKPtr<VKCommandBuffer>& commandBuffer, TInput& input)
	{
		commandBuffer->pushDebugGroup(_name);
		TOutput output = onRender(commandBuffer, input);
		commandBuffer->popDebugGroup();

		return output;
	}

	void resize(glm::uvec2 size)
	{
		_size = size;
		onResize();
	}

protected:
	glm::uvec2 _size;

	virtual TOutput onRender(const VKPtr<VKCommandBuffer>& commandBuffer, TInput& input) = 0;
	virtual void onResize() = 0;

private:
	const char* _name;
};