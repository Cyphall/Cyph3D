#pragma once

#include <CyphGPU/CommandContext.hpp>
#include <CyphGPU/fwd.hpp>
#include <glm/glm.hpp>
#include <imgui.h>
#include <memory>

namespace c3d
{
class Entity;
class Camera;
class RenderRegistry;

class ObjectPicker
{
public:
	ObjectPicker();
	~ObjectPicker();

	Entity* getPickedEntity(const Camera& camera, const RenderRegistry& renderRegistry, const glm::uvec2& viewportSize, const glm::uvec2& clickPos);

private:
	glm::uvec2 _currentSize = {0, 0};

	cgpu::CommandContext _commandContext;

	cgpu::ImagePtr _objectIndexImage;
	cgpu::ImagePtr _depthImage;

	cgpu::VertexInputStatePtr _vertexInputState;
	cgpu::PreRasterizationShaderStatePtr _preRasterizationShaderState;
	cgpu::FragmentShaderStatePtr _fragmentShaderState;
	cgpu::FragmentOutputStatePtr _fragmentOutputState;

	void createImages();
	void createPipelineStates();
};
}
