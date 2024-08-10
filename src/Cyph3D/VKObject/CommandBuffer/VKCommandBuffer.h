#pragma once

#include "Cyph3D/VKObject/CommandBuffer/VKRenderingInfo.h"
#include "Cyph3D/VKObject/VKObject.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

class VKQueue;
class VKBufferBase;
template<typename T>
class VKResizableBuffer;
template<typename T>
class VKBuffer;
class VKImage;
class VKSampler;
class VKDescriptorSet;
class VKPipeline;
class VKFence;
class VKAccelerationStructure;
class VKAccelerationStructureCompactedSizeQuery;
class VKShaderBindingTable;
struct VKPipelineViewport;
struct VKPipelineScissor;
struct VKBottomLevelAccelerationStructureBuildInfo;
struct VKTopLevelAccelerationStructureBuildInfo;

class VKCommandBuffer : public VKObject
{
public:
	static std::shared_ptr<VKCommandBuffer> create(VKContext& context, const VKQueue& queue);

	~VKCommandBuffer() override;

	const vk::CommandBuffer& getHandle();

	void waitExecution() const;

	void begin();
	void end();
	void reset();

	void memoryBarrier(vk::PipelineStageFlags2 srcStageMask, vk::AccessFlags2 srcAccessMask, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlags2 dstAccessMask);
	void bufferMemoryBarrier(const std::shared_ptr<VKBufferBase>& buffer, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlags2 dstAccessMask);
	void imageMemoryBarrier(const std::shared_ptr<VKImage>& image, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlags2 dstAccessMask, vk::ImageLayout newImageLayout);
	void imageMemoryBarrier(const std::shared_ptr<VKImage>& image, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlags2 dstAccessMask, vk::ImageLayout newImageLayout, glm::uvec2 layerRange, glm::uvec2 levelRange);

	void acquireBufferOwnership(const std::shared_ptr<VKBufferBase>& buffer, const VKQueue& previousOwner, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlags2 dstAccessMask);
	void releaseBufferOwnership(const std::shared_ptr<VKBufferBase>& buffer, const VKQueue& nextOwner);
	void acquireImageOwnership(const std::shared_ptr<VKImage>& image, const VKQueue& previousOwner, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlags2 dstAccessMask, vk::ImageLayout newImageLayout);
	void acquireImageOwnership(const std::shared_ptr<VKImage>& image, const VKQueue& previousOwner, vk::PipelineStageFlags2 dstStageMask, vk::AccessFlags2 dstAccessMask, vk::ImageLayout newImageLayout, glm::uvec2 layerRange, glm::uvec2 levelRange);
	void releaseImageOwnership(const std::shared_ptr<VKImage>& image, const VKQueue& nextOwner, vk::ImageLayout newImageLayout);
	void releaseImageOwnership(const std::shared_ptr<VKImage>& image, const VKQueue& nextOwner, vk::ImageLayout newImageLayout, glm::uvec2 layerRange, glm::uvec2 levelRange);

	void beginRendering(const VKRenderingInfo& renderingInfo);
	void endRendering();

	void bindPipeline(const std::shared_ptr<VKPipeline>& pipeline);
	void unbindPipeline();

	void bindDescriptorSet(uint32_t setIndex, const std::shared_ptr<VKDescriptorSet>& descriptorSet);
	void bindDescriptorSet(uint32_t setIndex, const std::shared_ptr<VKDescriptorSet>& descriptorSet, uint32_t dynamicOffset);

	template<typename T>
	void pushConstants(const T& data)
	{
		pushConstants(&data, sizeof(T));
	}

	void pushDescriptor(uint32_t setIndex, uint32_t bindingIndex, const std::shared_ptr<VKBufferBase>& buffer, size_t offset, size_t size, uint32_t arrayIndex = 0);
	void pushDescriptor(uint32_t setIndex, uint32_t bindingIndex, const std::shared_ptr<VKSampler>& sampler, uint32_t arrayIndex = 0);
	void pushDescriptor(uint32_t setIndex, uint32_t bindingIndex, const std::shared_ptr<VKImage>& image, uint32_t arrayIndex = 0);
	void pushDescriptor(uint32_t setIndex, uint32_t bindingIndex, const std::shared_ptr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format, uint32_t arrayIndex = 0);
	void pushDescriptor(uint32_t setIndex, uint32_t bindingIndex, const std::shared_ptr<VKImage>& image, const std::shared_ptr<VKSampler>& sampler, uint32_t arrayIndex = 0);
	void pushDescriptor(uint32_t setIndex, uint32_t bindingIndex, const std::shared_ptr<VKImage>& image, vk::ImageViewType type, glm::uvec2 layerRange, glm::uvec2 levelRange, vk::Format format, const std::shared_ptr<VKSampler>& sampler, uint32_t arrayIndex = 0);
	void pushDescriptor(uint32_t setIndex, uint32_t bindingIndex, const std::shared_ptr<VKAccelerationStructure>& accelerationStructure, uint32_t arrayIndex = 0);

	void bindVertexBuffer(uint32_t vertexBufferIndex, const std::shared_ptr<VKBufferBase>& vertexBuffer);
	void bindIndexBuffer(const std::shared_ptr<VKBufferBase>& indexBuffer);

	void draw(uint32_t vertexCount, uint32_t vertexOffset);
	void drawIndexed(uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset);
	void drawIndirect(const std::shared_ptr<VKBuffer<vk::DrawIndirectCommand>>& drawCommandsBuffer);
	void drawIndexedIndirect(const std::shared_ptr<VKBuffer<vk::DrawIndexedIndirectCommand>>& drawCommandsBuffer);

	void copyBufferToImage(const std::shared_ptr<VKBufferBase>& srcBuffer, vk::DeviceSize srcByteOffset, const std::shared_ptr<VKImage>& dstImage, uint32_t dstLayer, uint32_t dstLevel);

	void copyBufferToBuffer(const std::shared_ptr<VKBufferBase>& srcBuffer, vk::DeviceSize srcByteOffset, const std::shared_ptr<VKBufferBase>& dstBuffer, vk::DeviceSize dstByteOffset, vk::DeviceSize size);

	void copyImageToBuffer(const std::shared_ptr<VKImage>& srcImage, uint32_t srcLayer, uint32_t srcLevel, const std::shared_ptr<VKBufferBase>& dstBuffer, vk::DeviceSize dstByteOffset);

	void copyImageToImage(const std::shared_ptr<VKImage>& srcImage, uint32_t srcLayer, uint32_t srcLevel, const std::shared_ptr<VKImage>& dstImage, uint32_t dstLayer, uint32_t dstLevel);

	void copyPixelToBuffer(const std::shared_ptr<VKImage>& srcImage, uint32_t srcLayer, uint32_t srcLevel, glm::uvec2 srcPixel, const std::shared_ptr<VKBufferBase>& dstBuffer, vk::DeviceSize dstByteOffset);

	void blitImage(const std::shared_ptr<VKImage>& srcImage, uint32_t srcLayer, uint32_t srcLevel, const std::shared_ptr<VKImage>& dstImage, uint32_t dstLayer, uint32_t dstLevel);

	void dispatch(glm::uvec3 groupCount);

	void setViewport(const VKPipelineViewport& viewport);

	void setScissor(const VKPipelineScissor& scissor);

	void pushDebugGroup(std::string_view name);
	void popDebugGroup();

	void queryAccelerationStructureCompactedSize(const std::shared_ptr<VKAccelerationStructure>& accelerationStructure, const std::shared_ptr<VKAccelerationStructureCompactedSizeQuery>& accelerationStructureCompactedSizeQuery);

	void clearColorImage(const std::shared_ptr<VKImage>& image, uint32_t layer, uint32_t level, const vk::ClearColorValue& clearColor);

	void buildBottomLevelAccelerationStructure(const std::shared_ptr<VKAccelerationStructure>& accelerationStructure, const std::shared_ptr<VKBufferBase>& scratchBuffer, const VKBottomLevelAccelerationStructureBuildInfo& buildInfo);
	void buildTopLevelAccelerationStructure(const std::shared_ptr<VKAccelerationStructure>& accelerationStructure, const std::shared_ptr<VKBufferBase>& scratchBuffer, const VKTopLevelAccelerationStructureBuildInfo& buildInfo, const std::shared_ptr<VKResizableBuffer<vk::AccelerationStructureInstanceKHR>>& instancesBuffer);

	void compactAccelerationStructure(const std::shared_ptr<VKAccelerationStructure>& src, const std::shared_ptr<VKAccelerationStructure>& dst);

	void traceRays(const std::shared_ptr<VKShaderBindingTable>& sbt, glm::uvec2 size);

	const std::shared_ptr<VKFence>& getStatusFence() const;

	void addExternallyUsedObject(const std::shared_ptr<VKObject>& object);

private:
	explicit VKCommandBuffer(VKContext& context, const VKQueue& queue);

	void pushConstants(const void* data, uint32_t dataSize);

	uint32_t _queueFamily;
	vk::CommandPool _commandPool;
	vk::CommandBuffer _commandBuffer;

	std::shared_ptr<VKFence> _statusFence;

	VKPipeline* _boundPipeline = nullptr;

	std::vector<std::shared_ptr<VKObject>> _usedObjects;
};