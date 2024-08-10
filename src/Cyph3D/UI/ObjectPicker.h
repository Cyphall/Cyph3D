#pragma once

#include "Cyph3D/GLSL_types.h"

#include <glm/glm.hpp>
#include <imgui.h>

class Entity;
class VKCommandBuffer;
class Camera;
class RenderRegistry;
class VKDescriptorSetLayout;
class VKPipelineLayout;
class VKGraphicsPipeline;
class VKImage;
template<typename T>
class VKBuffer;

class ObjectPicker
{
public:
	ObjectPicker();
	~ObjectPicker();

	Entity* getPickedEntity(const Camera& camera, const RenderRegistry& renderRegistry, const glm::uvec2& viewportSize, const glm::uvec2& clickPos);

private:
	struct PushConstantData
	{
		GLSL_mat4 mvp;
		GLSL_int objectIndex;
	};

	glm::uvec2 _currentSize = {0, 0};

	std::shared_ptr<VKDescriptorSetLayout> _descriptorSetLayout;

	std::shared_ptr<VKPipelineLayout> _pipelineLayout;

	std::shared_ptr<VKGraphicsPipeline> _pipeline;

	std::shared_ptr<VKBuffer<int32_t>> _readbackBuffer;

	std::shared_ptr<VKImage> _objectIndexImage;
	std::shared_ptr<VKImage> _depthImage;

	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createPipeline();
	void createBuffer();
	void createImage();
};