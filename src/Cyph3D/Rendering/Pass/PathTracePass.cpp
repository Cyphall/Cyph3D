#include "PathTracePass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/VKObject/CommandBuffer/VKRenderingInfo.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/Asset/AssetManager.h"
#include "Cyph3D/Asset/BindlessTextureManager.h"
#include "Cyph3D/Asset/RuntimeAsset/MaterialAsset.h"
#include "Cyph3D/Asset/RuntimeAsset/MeshAsset.h"
#include "Cyph3D/Helper/MathHelper.h"
#include "Cyph3D/VKObject/VKHelper.h"
#include "Cyph3D/VKObject/AccelerationStructure/VKAccelerationStructure.h"
#include "Cyph3D/VKObject/AccelerationStructure/VKTopLevelAccelerationStructureBuildInfo.h"
#include "Cyph3D/VKObject/Buffer/VKBuffer.h"
#include "Cyph3D/VKObject/Buffer/VKResizableBuffer.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Pipeline/VKRayTracingPipeline.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

PathTracePass::PathTracePass(const glm::uvec2& size):
	RenderPass(size, "Path trace pass")
{
	createBuffers();
	createDescriptorSetLayout();
	createPipelineLayout();
	createPipeline();
	createImage();
}

PathTracePassOutput PathTracePass::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, PathTracePassInput& input)
{
	if (input.resetAccumulation)
	{
		_accumulatedSamples = 0;
	}
	
	commandBuffer->imageMemoryBarrier(
		_rawRenderImage,
		0,
		0,
		vk::PipelineStageFlagBits2::eFragmentShader,
		vk::AccessFlagBits2::eShaderSampledRead,
		vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
		vk::AccessFlagBits2::eShaderStorageRead | vk::AccessFlagBits2::eShaderStorageWrite,
		vk::ImageLayout::eGeneral);
	
	VKTopLevelAccelerationStructureBuildInfo buildInfo;
	buildInfo.instancesInfos.reserve(input.registry.getModelRenderRequests().size());
	_objectUniforms->resizeSmart(input.registry.getModelRenderRequests().size());
	ObjectUniforms* objectUniformsPtr = _objectUniforms->getHostPointer();
	int drawCount = 0;
	for (const ModelRenderer::RenderData& renderData : input.registry.getModelRenderRequests())
	{
		VKTopLevelAccelerationStructureBuildInfo::InstanceInfo& instanceInfo = buildInfo.instancesInfos.emplace_back();
		instanceInfo.localToWorld = renderData.matrix;
		instanceInfo.customIndex = drawCount;
		instanceInfo.accelerationStructure = renderData.mesh->getAccelerationStructure();
		
		ObjectUniforms uniforms{};
		uniforms.normalMatrix = glm::inverseTranspose(glm::mat3(renderData.matrix));
		uniforms.model = renderData.matrix;
		uniforms.vertexBuffer = renderData.mesh->getVertexBuffer()->getDeviceAddress();
		uniforms.indexBuffer = renderData.mesh->getIndexBuffer()->getDeviceAddress();
		uniforms.albedoIndex = renderData.material->getAlbedoTextureBindlessIndex();
		uniforms.normalIndex = renderData.material->getNormalTextureBindlessIndex();
		uniforms.roughnessIndex = renderData.material->getRoughnessTextureBindlessIndex();
		uniforms.metalnessIndex = renderData.material->getMetalnessTextureBindlessIndex();
		uniforms.displacementIndex = renderData.material->getDisplacementTextureBindlessIndex();
		uniforms.emissiveIndex = renderData.material->getEmissiveTextureBindlessIndex();
		uniforms.emissiveScale = renderData.material->getEmissiveScale();
		std::memcpy(objectUniformsPtr, &uniforms, sizeof(ObjectUniforms));
		objectUniformsPtr++;
		
		drawCount++;
	}
	
	vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfo = VKAccelerationStructure::getTopLevelBuildSizesInfo(Engine::getVKContext(), buildInfo);
	
	_tlasBackingBuffer->resizeSmart(buildSizesInfo.accelerationStructureSize);
	
	VKPtr<VKAccelerationStructure> tlas = VKAccelerationStructure::create(
		Engine::getVKContext(),
		vk::AccelerationStructureTypeKHR::eTopLevel,
		buildSizesInfo.accelerationStructureSize,
		_tlasBackingBuffer->getBuffer());
	
	_tlasScratchBuffer->resizeSmart(buildSizesInfo.buildScratchSize);
	
	commandBuffer->buildTopLevelAccelerationStructure(tlas, _tlasScratchBuffer->getBuffer(), buildInfo, _tlasInstancesBuffer.getCurrent());
	
	commandBuffer->bufferMemoryBarrier(
		_tlasBackingBuffer->getBuffer(),
		vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
		vk::AccessFlagBits2::eAccelerationStructureWriteKHR,
		vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
		vk::AccessFlagBits2::eAccelerationStructureReadKHR);
	
	GlobalUniforms globalUniforms;
	globalUniforms.cameraPosition = input.camera.getPosition();
	globalUniforms.cameraRayTL = input.camera.getCornerRays()[0];
	globalUniforms.cameraRayTR = input.camera.getCornerRays()[1];
	globalUniforms.cameraRayBL = input.camera.getCornerRays()[2];
	globalUniforms.cameraRayBR = input.camera.getCornerRays()[3];
	globalUniforms.sampleIndex = _sampleIndex;
	globalUniforms.sampleCount = input.sampleCount;
	globalUniforms.resetAccumulation = _accumulatedSamples == 0;

	SkyboxAsset* skybox = Engine::getScene().getSkybox();
	if (skybox && skybox->isLoaded())
	{
		globalUniforms.hasSkybox = true;
		globalUniforms.skyboxIndex = skybox->getBindlessIndex();
		globalUniforms.skyboxRotation = glm::rotate(glm::radians(Engine::getScene().getSkyboxRotation()), glm::vec3(0, 1, 0));
	}
	else
	{
		globalUniforms.hasSkybox = false;
	}
	
	std::memcpy(_globalUniforms->getHostPointer(), &globalUniforms, sizeof(GlobalUniforms));
	
	commandBuffer->bindPipeline(_pipeline);
	
	commandBuffer->bindDescriptorSet(0, Engine::getAssetManager().getBindlessTextureManager().getDescriptorSet());
	commandBuffer->pushDescriptor(1, 0, tlas);
	commandBuffer->pushDescriptor(1, 1, _rawRenderImageView);
	commandBuffer->pushDescriptor(1, 2, _globalUniforms.getCurrent(), 0, 1);
	commandBuffer->pushDescriptor(1, 3, _objectUniforms->getBuffer(), 0, drawCount);
	
	VKHelper::buildRaygenShaderBindingTable(Engine::getVKContext(), _pipeline, _raygenSBT.getCurrent());
	VKHelper::buildMissShaderBindingTable(Engine::getVKContext(), _pipeline, _missSBT.getCurrent());
	VKHelper::buildHitShaderBindingTable(Engine::getVKContext(), _pipeline, _hitSBT.getCurrent());
	
	commandBuffer->traceRays(_raygenSBT->getBuffer(), _missSBT->getBuffer(), _hitSBT->getBuffer(), _size);
	
	_accumulatedSamples += input.sampleCount;
	_sampleIndex += input.sampleCount;
	
	commandBuffer->unbindPipeline();
	
	return {
		.rawRenderImageView = _rawRenderImageView,
		.accumulatedSamples = _accumulatedSamples
	};
}

void PathTracePass::onResize()
{
	createImage();
}

void PathTracePass::createBuffers()
{
	VKBufferInfo globalUniformsBufferInfo(1, vk::BufferUsageFlagBits::eUniformBuffer);
	globalUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	globalUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	globalUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	
	_globalUniforms = VKDynamic<VKBuffer<GlobalUniforms>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKBuffer<GlobalUniforms>::create(context, globalUniformsBufferInfo);
	});
	
	VKResizableBufferInfo objectUniformsBufferInfo(vk::BufferUsageFlagBits::eStorageBuffer);
	objectUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	objectUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	objectUniformsBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	
	_objectUniforms = VKDynamic<VKResizableBuffer<ObjectUniforms>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<ObjectUniforms>::create(context, objectUniformsBufferInfo);
	});
	
	VKResizableBufferInfo tlasBackingBufferInfo(vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR);
	tlasBackingBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	_tlasBackingBuffer = VKDynamic<VKResizableBuffer<std::byte>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<std::byte>::create(context, tlasBackingBufferInfo);
	});
	
	VKResizableBufferInfo tlasScratchBufferInfo(vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer);
	tlasScratchBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	tlasScratchBufferInfo.setRequiredAlignment(Engine::getVKContext().getAccelerationStructureProperties().minAccelerationStructureScratchOffsetAlignment);
	
	_tlasScratchBuffer = VKDynamic<VKResizableBuffer<std::byte>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<std::byte>::create(context, tlasScratchBufferInfo);
	});
	
	VKResizableBufferInfo tlasInstancesBufferInfo(vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);
	tlasInstancesBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	tlasInstancesBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	tlasInstancesBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	tlasInstancesBufferInfo.setRequiredAlignment(16);
	
	_tlasInstancesBuffer = VKDynamic<VKResizableBuffer<vk::AccelerationStructureInstanceKHR>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<vk::AccelerationStructureInstanceKHR>::create(context, tlasInstancesBufferInfo);
	});
	
	VKResizableBufferInfo raygenSBTInfo(vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR);
	raygenSBTInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	raygenSBTInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	raygenSBTInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	raygenSBTInfo.setRequiredAlignment(Engine::getVKContext().getRayTracingPipelineProperties().shaderGroupBaseAlignment);
	
	_raygenSBT = VKDynamic<VKResizableBuffer<std::byte>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<std::byte>::create(context, raygenSBTInfo);
	});
	
	VKResizableBufferInfo missSBTInfo(vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR);
	missSBTInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	missSBTInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	missSBTInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	missSBTInfo.setRequiredAlignment(Engine::getVKContext().getRayTracingPipelineProperties().shaderGroupBaseAlignment);
	
	_missSBT = VKDynamic<VKResizableBuffer<std::byte>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<std::byte>::create(context, missSBTInfo);
	});
	
	VKResizableBufferInfo hitSBTInfo(vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR);
	hitSBTInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	hitSBTInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	hitSBTInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	hitSBTInfo.setRequiredAlignment(Engine::getVKContext().getRayTracingPipelineProperties().shaderGroupBaseAlignment);
	
	_hitSBT = VKDynamic<VKResizableBuffer<std::byte>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<std::byte>::create(context, hitSBTInfo);
	});
}

void PathTracePass::createDescriptorSetLayout()
{
	VKDescriptorSetLayoutInfo info(true);
	info.addBinding(vk::DescriptorType::eAccelerationStructureKHR, 1);
	info.addBinding(vk::DescriptorType::eStorageImage, 1);
	info.addBinding(vk::DescriptorType::eUniformBuffer, 1);
	info.addBinding(vk::DescriptorType::eStorageBuffer, 1);
	
	_descriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
}

void PathTracePass::createPipelineLayout()
{
	VKPipelineLayoutInfo info;
	info.addDescriptorSetLayout(Engine::getAssetManager().getBindlessTextureManager().getDescriptorSetLayout());
	info.addDescriptorSetLayout(_descriptorSetLayout);
	
	_pipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
}

void PathTracePass::createPipeline()
{
	VKRayTracingPipelineInfo info(
		_pipelineLayout,
		"resources/shaders/internal/path tracing/path trace.rgen");
	
	info.addRayType("resources/shaders/internal/path tracing/path trace.rmiss");
	info.addObjectTypeForRayType(0, "resources/shaders/internal/path tracing/path trace.rchit", std::nullopt, std::nullopt);
	
	_pipeline = VKRayTracingPipeline::create(Engine::getVKContext(), info);
}

void PathTracePass::createImage()
{
	VKImageInfo imageInfo(
		SceneRenderer::ACCUMULATION_FORMAT,
		_size,
		1,
		1,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled);
	imageInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	_rawRenderImage = VKImage::create(Engine::getVKContext(), imageInfo);
	
	VKImageViewInfo imageViewInfo(
		_rawRenderImage,
		vk::ImageViewType::e2D);
	
	_rawRenderImageView = VKImageView::create(Engine::getVKContext(), imageViewInfo);
	
	_accumulatedSamples = 0;
}