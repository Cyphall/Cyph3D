#pragma once

#include "Cyph3D/VKObject/VKPtr.h"
#include "Cyph3D/GLSL_types.h"

#include <glm/glm.hpp>
#include <memory>
#include <imgui.h>

class Entity;
class VKCommandBuffer;
class Camera;
class RenderRegistry;
class VKDescriptorSetLayout;
class VKPipelineLayout;
class VKGraphicsPipeline;
class VKImage;
class VKImageView;
template<typename T>
class VKBuffer;

class ObjectPicker
{
public:
	ObjectPicker();
	~ObjectPicker();
	
	Entity* getPickedEntity(Camera& camera, const RenderRegistry& renderRegistry, const glm::uvec2& viewportSize, const glm::uvec2& clickPos);

private:
	struct PushConstantData
	{
		GLSL_mat4 mvp;
		GLSL_int objectIndex;
	};
	
	glm::uvec2 _currentSize = {0, 0};
	
	VKPtr<VKDescriptorSetLayout> _descriptorSetLayout;
	
	VKPtr<VKPipelineLayout> _pipelineLayout;
	
	VKPtr<VKGraphicsPipeline> _pipeline;
	
	VKPtr<VKBuffer<int32_t>> _readbackBuffer;
	
	VKPtr<VKImage> _objectIndexImage;
	VKPtr<VKImageView> _objectIndexImageView;
	VKPtr<VKImage> _depthImage;
	VKPtr<VKImageView> _depthImageView;
	
	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createPipeline();
	void createBuffer();
	void createImage();
};