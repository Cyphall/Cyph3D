#include "RaytracePass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Scene/Scene.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
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

RaytracePass::RaytracePass(const glm::uvec2& size):
	RenderPass(size, "Raytrace pass")
{
	createBuffers();
	createDescriptorSetLayout();
	createPipelineLayout();
	createPipeline();
	createImages();
}

RaytracePassOutput RaytracePass::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, RaytracePassInput& input)
{
	commandBuffer->imageMemoryBarrier(
		_rawRenderImage.getVKPtr(),
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
		vk::AccessFlagBits2::eShaderStorageWrite,
		vk::ImageLayout::eGeneral,
		0,
		0);
	
	commandBuffer->imageMemoryBarrier(
		_objectIndexImage.getVKPtr(),
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eRayTracingShaderKHR,
		vk::AccessFlagBits2::eShaderStorageWrite,
		vk::ImageLayout::eGeneral,
		0,
		0);
	
	VKTopLevelAccelerationStructureBuildInfo buildInfo;
	buildInfo.instancesInfos.reserve(input.registry.models.size());
	_objectUniforms->resizeSmart(input.registry.models.size());
	ObjectUniforms* objectUniformsPtr = _objectUniforms->map();
	int actualObjectCountToBeDrawn = 0;
	for (int i = 0; i < input.registry.models.size(); i++)
	{
		ModelRenderer::RenderData modelRenderData = input.registry.models[i];
		
		MaterialAsset* material = modelRenderData.material;
		if (material == nullptr)
		{
			material = MaterialAsset::getMissingMaterial();
		}
		else if (!material->isLoaded())
		{
			material = MaterialAsset::getDefaultMaterial();
		}
		
		if (material == nullptr || !material->isLoaded())
		{
			continue;
		}
		
		MeshAsset* mesh = modelRenderData.mesh;
		if (mesh == nullptr || !mesh->isLoaded())
		{
			continue;
		}
		
		VKTopLevelAccelerationStructureBuildInfo::InstanceInfo& instanceInfo = buildInfo.instancesInfos.emplace_back();
		instanceInfo.localToWorld = modelRenderData.matrix;
		instanceInfo.customIndex = i;
		instanceInfo.accelerationStructure = mesh->getAccelerationStructure();
		
		ObjectUniforms uniforms{};
		uniforms.normalMatrix = glm::inverseTranspose(glm::mat3(modelRenderData.matrix));
		uniforms.model = modelRenderData.matrix;
		uniforms.vertexBuffer = mesh->getVertexBuffer()->getDeviceAddress();
		uniforms.indexBuffer = mesh->getIndexBuffer()->getDeviceAddress();
		uniforms.albedoIndex = material->getAlbedoTextureBindlessIndex();
		uniforms.normalIndex = material->getNormalTextureBindlessIndex();
		uniforms.roughnessIndex = material->getRoughnessTextureBindlessIndex();
		uniforms.metalnessIndex = material->getMetalnessTextureBindlessIndex();
		uniforms.displacementIndex = material->getDisplacementTextureBindlessIndex();
		uniforms.emissiveIndex = material->getEmissiveTextureBindlessIndex();
		std::memcpy(objectUniformsPtr, &uniforms, sizeof(ObjectUniforms));
		objectUniformsPtr++;
		
		actualObjectCountToBeDrawn++;
	}
	_objectUniforms->unmap();
	
	vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfo = VKAccelerationStructure::getTopLevelBuildSizesInfo(Engine::getVKContext(), buildInfo);
	
	_tlasBackingBuffer->resizeSmart(buildSizesInfo.accelerationStructureSize);
	
	VKPtr<VKAccelerationStructure> tlas = VKAccelerationStructure::create(
		Engine::getVKContext(),
		vk::AccelerationStructureTypeKHR::eTopLevel,
		buildSizesInfo.accelerationStructureSize,
		_tlasBackingBuffer->getBuffer());
	
	_tlasScratchBuffer->resizeSmart(buildSizesInfo.buildScratchSize);
	
	commandBuffer->buildTopLevelAccelerationStructure(tlas, _tlasScratchBuffer->getBuffer(), buildInfo, _tlasInstancesBuffer.getVKPtr());
	
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
	
	GlobalUniforms* globalUniformsPtr = _globalUniforms->map();
	std::memcpy(globalUniformsPtr, &globalUniforms, sizeof(GlobalUniforms));
	_globalUniforms->unmap();
	
	commandBuffer->bindPipeline(_pipeline);
	
	commandBuffer->bindDescriptorSet(0, Engine::getAssetManager().getBindlessTextureManager().getDescriptorSet());
	commandBuffer->pushDescriptor(1, 0, tlas);
	commandBuffer->pushDescriptor(1, 1, _rawRenderImageView.getVKPtr());
	commandBuffer->pushDescriptor(1, 2, _objectIndexImageView.getVKPtr());
	commandBuffer->pushDescriptor(1, 3, _globalUniforms.getVKPtr(), 0, 1);
	commandBuffer->pushDescriptor(1, 4, _objectUniforms->getBuffer(), 0, actualObjectCountToBeDrawn);
	
	VKHelper::buildRaygenShaderBindingTable(Engine::getVKContext(), _pipeline, _raygenSBT.getVKPtr());
	VKHelper::buildMissShaderBindingTable(Engine::getVKContext(), _pipeline, _missSBT.getVKPtr());
	VKHelper::buildHitShaderBindingTable(Engine::getVKContext(), _pipeline, _hitSBT.getVKPtr());
	
	commandBuffer->traceRays(_raygenSBT->getBuffer(), _missSBT->getBuffer(), _hitSBT->getBuffer(), _size);
	
	commandBuffer->unbindPipeline();
	
	return {
		.rawRenderImageView = _rawRenderImageView.getVKPtr(),
		.objectIndexImageView = _objectIndexImageView.getVKPtr()
	};
}

void RaytracePass::onResize()
{
	createImages();
}

void RaytracePass::createBuffers()
{
	_globalUniforms = VKBuffer<GlobalUniforms>::createDynamic(
		Engine::getVKContext(),
		1,
		vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	
	_objectUniforms = VKResizableBuffer<ObjectUniforms>::createDynamic(
		Engine::getVKContext(),
		vk::BufferUsageFlagBits::eStorageBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	
	_tlasBackingBuffer = VKResizableBuffer<std::byte>::createDynamic(
		Engine::getVKContext(),
		vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR,
		vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	_tlasScratchBuffer = VKResizableBuffer<std::byte>::createDynamic(
		Engine::getVKContext(),
		vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal);
	
	_tlasInstancesBuffer = VKResizableBuffer<vk::AccelerationStructureInstanceKHR>::createDynamic(
		Engine::getVKContext(),
		vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
		vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	
	_raygenSBT = VKResizableBuffer<std::byte>::createDynamic(
		Engine::getVKContext(),
		vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR,
		vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	
	_missSBT = VKResizableBuffer<std::byte>::createDynamic(
		Engine::getVKContext(),
		vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR,
		vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	
	_hitSBT = VKResizableBuffer<std::byte>::createDynamic(
		Engine::getVKContext(),
		vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eShaderBindingTableKHR,
		vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
}

void RaytracePass::createDescriptorSetLayout()
{
	VKDescriptorSetLayoutInfo info(true);
	info.addBinding(vk::DescriptorType::eAccelerationStructureKHR, 1);
	info.addBinding(vk::DescriptorType::eStorageImage, 1);
	info.addBinding(vk::DescriptorType::eStorageImage, 1);
	info.addBinding(vk::DescriptorType::eUniformBuffer, 1);
	info.addBinding(vk::DescriptorType::eStorageBuffer, 1);
	
	_descriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
}

void RaytracePass::createPipelineLayout()
{
	VKPipelineLayoutInfo info;
	info.addDescriptorSetLayout(Engine::getAssetManager().getBindlessTextureManager().getDescriptorSetLayout());
	info.addDescriptorSetLayout(_descriptorSetLayout);
	
	_pipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
}

void RaytracePass::createPipeline()
{
	VKRayTracingPipelineInfo info(
		_pipelineLayout,
		"resources/shaders/internal/raytracing/raytrace.rgen");
	
	info.addRayType("resources/shaders/internal/raytracing/raytrace.rmiss");
	info.addObjectTypeForRayType(0, "resources/shaders/internal/raytracing/raytrace.rchit", std::nullopt, std::nullopt);
	
	_pipeline = VKRayTracingPipeline::create(Engine::getVKContext(), info);
}

void RaytracePass::createImages()
{
	{
		_rawRenderImage = VKImage::createDynamic(
			Engine::getVKContext(),
			SceneRenderer::HDR_COLOR_FORMAT,
			_size,
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled,
			vk::ImageAspectFlagBits::eColor,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_rawRenderImageView = VKImageView::createDynamic(
			Engine::getVKContext(),
			_rawRenderImage,
			vk::ImageViewType::e2D);
	}
	
	{
		_objectIndexImage = VKImage::createDynamic(
			Engine::getVKContext(),
			SceneRenderer::OBJECT_INDEX_FORMAT,
			_size,
			1,
			1,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc,
			vk::ImageAspectFlagBits::eColor,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
		
		_objectIndexImageView = VKImageView::createDynamic(
			Engine::getVKContext(),
			_objectIndexImage,
			vk::ImageViewType::e2D);
	}
}