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
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSet.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Pipeline/VKRayTracingPipeline.h"
#include "Cyph3D/VKObject/ShaderBindingTable/VKShaderBindingTable.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

PathTracePass::PathTracePass(const glm::uvec2& size):
	RenderPass(size, "Path trace pass")
{
	createDescriptorSetLayout();
	createPipelineLayout();
	createPipeline();
	createImage();
}

PathTracePassOutput PathTracePass::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, PathTracePassInput& input)
{
	if (input.sceneChanged || input.cameraChanged)
	{
		_accumulatedBatches = 0;
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
	
	bool recreateDescriptorSet = false;
	
	if (input.sceneChanged)
	{
		setupTLAS(commandBuffer, input);
		
		recreateDescriptorSet = true;
	}
	
	if (input.sceneChanged || input.cameraChanged)
	{
		setupSBT(input);
		
		recreateDescriptorSet = true;
	}
	
	if (recreateDescriptorSet)
	{
		VKDescriptorSetInfo info(_descriptorSetLayout);
		
		_descriptorSet = VKDescriptorSet::create(Engine::getVKContext(), info);
		
		_descriptorSet->bindAccelerationStructure(0, _tlas);
		_descriptorSet->bindImage(1, _rawRenderImageView);
	}
	
	commandBuffer->bindPipeline(_pipeline);
	
	commandBuffer->bindDescriptorSet(0, Engine::getAssetManager().getBindlessTextureManager().getDescriptorSet());
	commandBuffer->bindDescriptorSet(1, _descriptorSet);
	
	FramePushConstants framePushConstants{
		.batchIndex = _batchIndex,
		.sampleCount = input.sampleCount,
		.resetAccumulation = _accumulatedBatches == 0
	};
	
	commandBuffer->pushConstants(framePushConstants);
	
	commandBuffer->traceRays(_sbt, _size);
	
	_accumulatedBatches++;
	_batchIndex++;
	
	commandBuffer->unbindPipeline();
	
	return {
		.rawRenderImageView = _rawRenderImageView,
		.accumulatedBatches = _accumulatedBatches
	};
}

void PathTracePass::onResize()
{
	createImage();
}

void PathTracePass::setupTLAS(const VKPtr<VKCommandBuffer>& commandBuffer, const PathTracePassInput& input)
{
	VKTopLevelAccelerationStructureBuildInfo buildInfo;
	buildInfo.instancesInfos.reserve(input.registry.getModelRenderRequests().size());
	for (int i = 0; i < input.registry.getModelRenderRequests().size(); i++)
	{
		const ModelRenderer::RenderData& renderData = input.registry.getModelRenderRequests()[i];
		
		VKTopLevelAccelerationStructureBuildInfo::InstanceInfo& instanceInfo = buildInfo.instancesInfos.emplace_back();
		instanceInfo.localToWorld = renderData.matrix;
		instanceInfo.customIndex = 0;
		instanceInfo.recordIndex = i;
		instanceInfo.accelerationStructure = renderData.mesh->getAccelerationStructure();
	}
	
	vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfo = VKAccelerationStructure::getTopLevelBuildSizesInfo(Engine::getVKContext(), buildInfo);
	
	VKBufferInfo backingBufferInfo(buildSizesInfo.accelerationStructureSize, vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR);
	backingBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	VKPtr<VKBuffer<std::byte>> backingBuffer = VKBuffer<std::byte>::create(Engine::getVKContext(), backingBufferInfo);
	
	_tlas = VKAccelerationStructure::create(
		Engine::getVKContext(),
		vk::AccelerationStructureTypeKHR::eTopLevel,
		buildSizesInfo.accelerationStructureSize,
		backingBuffer);
	
	VKBufferInfo scratchBufferInfo(buildSizesInfo.buildScratchSize, vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer);
	scratchBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	scratchBufferInfo.setRequiredAlignment(Engine::getVKContext().getAccelerationStructureProperties().minAccelerationStructureScratchOffsetAlignment);
	VKPtr<VKBuffer<std::byte>> scratchBuffer = VKBuffer<std::byte>::create(Engine::getVKContext(), scratchBufferInfo);
	
	VKResizableBufferInfo instancesBufferInfo(vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);
	instancesBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	instancesBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	instancesBufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	instancesBufferInfo.setRequiredAlignment(16);
	VKPtr<VKResizableBuffer<vk::AccelerationStructureInstanceKHR>> instancesBuffer = VKResizableBuffer<vk::AccelerationStructureInstanceKHR>::create(Engine::getVKContext(), instancesBufferInfo);
	
	commandBuffer->buildTopLevelAccelerationStructure(_tlas, scratchBuffer, buildInfo, instancesBuffer);
	
	commandBuffer->bufferMemoryBarrier(
		backingBuffer,
		vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR,
		vk::AccessFlagBits2::eAccelerationStructureWriteKHR,
		vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
		vk::AccessFlagBits2::eAccelerationStructureReadKHR);
}

void PathTracePass::setupSBT(const PathTracePassInput& input)
{
	CameraUniforms cameraUniforms{
		.cameraPosition = input.camera.getPosition(),
		.cameraRayTL = input.camera.getCornerRays()[0],
		.cameraRayTR = input.camera.getCornerRays()[1],
		.cameraRayBL = input.camera.getCornerRays()[2],
		.cameraRayBR = input.camera.getCornerRays()[3]
	};
	
	VKShaderBindingTableInfo info(_pipeline->getRaygenGroupHandle(0), cameraUniforms);
	
	for (int i = 0; i < input.registry.getModelRenderRequests().size(); i++)
	{
		const ModelRenderer::RenderData& renderData = input.registry.getModelRenderRequests()[i];
		
		ObjectUniforms objectUniforms{
			.normalMatrix = glm::inverseTranspose(glm::mat3(renderData.matrix)),
			.vertexBuffer = renderData.mesh->getFullVertexBuffer()->getDeviceAddress(),
			.indexBuffer = renderData.mesh->getIndexBuffer()->getDeviceAddress(),
			.albedoIndex = renderData.material->getAlbedoTextureBindlessIndex(),
			.normalIndex = renderData.material->getNormalTextureBindlessIndex(),
			.roughnessIndex = renderData.material->getRoughnessTextureBindlessIndex(),
			.metalnessIndex = renderData.material->getMetalnessTextureBindlessIndex(),
			.displacementIndex = renderData.material->getDisplacementTextureBindlessIndex(),
			.emissiveIndex = renderData.material->getEmissiveTextureBindlessIndex(),
			.emissiveScale = renderData.material->getEmissiveScale()
		};
		
		info.addTriangleHitRecord(i, 0, _pipeline->getTriangleHitGroupHandle(0), objectUniforms);
	}
	
	SkyboxAsset* skybox = Engine::getScene().getSkybox();
	
	if (skybox && skybox->isLoaded())
	{
		CubemapSkyboxUniforms cubemapSkyboxUniforms{
			.skyboxIndex = skybox->getCubemap()->getBindlessIndex(),
			.skyboxRotation = glm::rotate(glm::radians(Engine::getScene().getSkyboxRotation()), glm::vec3(0, 1, 0))
		};
		
		info.addMissRecord(_pipeline->getMissGroupHandle(1), cubemapSkyboxUniforms);
	}
	else
	{
		info.addMissRecord(_pipeline->getMissGroupHandle(0));
	}
	
	_sbt = VKShaderBindingTable::create(Engine::getVKContext(), info);
}

void PathTracePass::createDescriptorSetLayout()
{
	VKDescriptorSetLayoutInfo info(false);
	info.addBinding(vk::DescriptorType::eAccelerationStructureKHR, 1, vk::ShaderStageFlagBits::eRaygenKHR);
	info.addBinding(vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eRaygenKHR);
	
	_descriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
}

void PathTracePass::createPipelineLayout()
{
	VKPipelineLayoutInfo info;
	info.addDescriptorSetLayout(Engine::getAssetManager().getBindlessTextureManager().getDescriptorSetLayout());
	info.addDescriptorSetLayout(_descriptorSetLayout);
	info.setPushConstantLayout<FramePushConstants>(vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR);
	
	_pipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
}

void PathTracePass::createPipeline()
{
	VKRayTracingPipelineInfo info(_pipelineLayout);
	
	info.addRaygenGroupsInfos("resources/shaders/internal/path tracing/path trace.rgen");
	info.addTriangleHitGroupsInfos("resources/shaders/internal/path tracing/path trace.rchit", std::nullopt);
	info.addMissGroupsInfos("resources/shaders/internal/path tracing/black skybox.rmiss");
	info.addMissGroupsInfos("resources/shaders/internal/path tracing/cubemap skybox.rmiss");
	
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
	
	_accumulatedBatches = 0;
}