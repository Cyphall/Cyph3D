#include "ShadowMapPass.h"

#include "Cyph3D/Engine.h"
#include "Cyph3D/Asset/RuntimeAsset/MeshAsset.h"
#include "Cyph3D/Entity/Component/DirectionalLight.h"
#include "Cyph3D/VKObject/CommandBuffer/VKRenderingInfo.h"
#include "Cyph3D/VKObject/Buffer/VKResizableBuffer.h"
#include "Cyph3D/VKObject/DescriptorSet/VKDescriptorSetLayout.h"
#include "Cyph3D/VKObject/Pipeline/VKPipelineLayout.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipelineInfo.h"
#include "Cyph3D/VKObject/Pipeline/VKGraphicsPipeline.h"
#include "Cyph3D/VKObject/Image/VKImage.h"
#include "Cyph3D/VKObject/Image/VKImageView.h"
#include "Cyph3D/Rendering/RenderRegistry.h"
#include "Cyph3D/Rendering/SceneRenderer/SceneRenderer.h"
#include "Cyph3D/Scene/Camera.h"
#include "Cyph3D/Scene/Transform.h"

#include <glm/gtc/matrix_inverse.hpp>

static const float DIRECTIONAL_SHADOW_MAP_WORLD_SIZE = 60.0f;
static const float DIRECTIONAL_SHADOW_MAP_WORLD_DEPTH = 100.0f;

static const glm::mat4 DIRECTIONAL_SHADOW_MAP_PROJECTION = []()
{
	glm::mat4 projection = glm::ortho(
		-DIRECTIONAL_SHADOW_MAP_WORLD_SIZE / 2, DIRECTIONAL_SHADOW_MAP_WORLD_SIZE / 2,
		-DIRECTIONAL_SHADOW_MAP_WORLD_SIZE / 2, DIRECTIONAL_SHADOW_MAP_WORLD_SIZE / 2,
		-DIRECTIONAL_SHADOW_MAP_WORLD_DEPTH / 2, DIRECTIONAL_SHADOW_MAP_WORLD_DEPTH / 2);
	
	projection[1][1] *= -1;
	
	return projection;
}();

static const float POINT_SHADOW_MAP_NEAR = 0.01f;
static const float POINT_SHADOW_MAP_FAR = 100.0f;

static const glm::mat4 POINT_SHADOW_MAP_PROJECTION = []()
{
	glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, POINT_SHADOW_MAP_NEAR, POINT_SHADOW_MAP_FAR);
	
	projection[1][1] *= -1;
	
	return projection;
}();

static glm::mat4 calcDirectionalShadowMapView(const DirectionalLight::RenderData& light, const Camera& camera)
{
	float texelWorldSize = DIRECTIONAL_SHADOW_MAP_WORLD_SIZE / light.shadowMapResolution;
	glm::mat4 worldToShadowMapTexel = glm::ortho<float>(-texelWorldSize, texelWorldSize, -texelWorldSize, texelWorldSize, 0, 1) *
	                                  glm::lookAt(glm::vec3(0, 0, 0), light.transform.getDown(), light.transform.getForward());
	
	glm::vec4 shadowMapTexelPos4D = worldToShadowMapTexel * glm::vec4(camera.getPosition(), 1);
	glm::vec3 shadowMapTexelPos = glm::vec3(shadowMapTexelPos4D) / shadowMapTexelPos4D.w;
	
	glm::vec3 roundedShadowMapTexelPos = glm::vec3(glm::round(glm::vec2(shadowMapTexelPos)), shadowMapTexelPos.z);
	
	glm::vec4 shadowMapRoundedWorldPos4D = glm::affineInverse(worldToShadowMapTexel) * glm::vec4(roundedShadowMapTexelPos, 1);
	glm::vec3 shadowMapRoundedWorldPos = glm::vec3(shadowMapRoundedWorldPos4D) / shadowMapRoundedWorldPos4D.w;
	
	return glm::lookAt(
		shadowMapRoundedWorldPos,
		shadowMapRoundedWorldPos + light.transform.getDown(),
		light.transform.getForward());
}

static std::array<glm::mat4, 6> calcPointShadowMapView(const PointLight::RenderData& light)
{
	glm::vec3 position = light.transform.getWorldPosition();
	
	return {
		glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, 1, 0)),
		glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0)),
		glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
		glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
		glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, 1, 0)),
		glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, 1, 0))
	};
}

ShadowMapPass::ShadowMapPass(glm::uvec2 size):
	RenderPass(size, "Shadow map pass")
{
	createDescriptorSetLayout();
	createBuffer();
	createPipelineLayouts();
	createPipelines();
}

ShadowMapPassOutput ShadowMapPass::onRender(const VKPtr<VKCommandBuffer>& commandBuffer, ShadowMapPassInput& input)
{
	if (input.sceneChanged || input.cameraChanged)
	{
		_shadowMapManager.resetDirectionalShadowMapAllocations();
		_directionalShadowMapInfos.clear();
		
		int shadowCastingDirectionalLightIndex = 0;
		for (const DirectionalLight::RenderData& light : input.registry.getDirectionalLightRenderRequests())
		{
			if (!light.castShadows)
			{
				continue;
			}
			
			commandBuffer->pushDebugGroup(std::format("Directional light ({})", shadowCastingDirectionalLightIndex));
			renderDirectionalShadowMap(commandBuffer, light, input.registry.getModelRenderRequests(), input.camera);
			commandBuffer->popDebugGroup();
			
			shadowCastingDirectionalLightIndex++;
		}
	}
	
	if (input.sceneChanged)
	{
		_shadowMapManager.resetPointShadowMapAllocations();
		_pointShadowMapInfos.clear();
		
		int shadowCastingPointLights = 0;
		for (const PointLight::RenderData& light : input.registry.getPointLightRenderRequests())
		{
			if (light.castShadows)
			{
				shadowCastingPointLights++;
			}
		}
		
		_pointLightUniformBuffer->resizeSmart(shadowCastingPointLights);
		
		int shadowCastingPointLightIndex = 0;
		for (const PointLight::RenderData& light : input.registry.getPointLightRenderRequests())
		{
			if (!light.castShadows)
			{
				continue;
			}
			
			commandBuffer->pushDebugGroup(std::format("Point light ({})", shadowCastingPointLightIndex));
			renderPointShadowMap(commandBuffer, light, input.registry.getModelRenderRequests(), shadowCastingPointLightIndex);
			commandBuffer->popDebugGroup();
			
			shadowCastingPointLightIndex++;
		}
	}
	
	return {
		.directionalShadowMapInfos = _directionalShadowMapInfos,
		.pointShadowMapInfos = _pointShadowMapInfos
	};
}

void ShadowMapPass::onResize()
{

}

void ShadowMapPass::createDescriptorSetLayout()
{
	VKDescriptorSetLayoutInfo info(true);
	info.addBinding(vk::DescriptorType::eStorageBuffer, 1);
	
	_pointLightDescriptorSetLayout = VKDescriptorSetLayout::create(Engine::getVKContext(), info);
}

void ShadowMapPass::createBuffer()
{
	VKResizableBufferInfo bufferInfo(vk::BufferUsageFlagBits::eStorageBuffer);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eDeviceLocal);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostVisible);
	bufferInfo.addRequiredMemoryProperty(vk::MemoryPropertyFlagBits::eHostCoherent);
	
	_pointLightUniformBuffer = VKDynamic<VKResizableBuffer<PointLightUniforms>>(Engine::getVKContext(), [&](VKContext& context, int index)
	{
		return VKResizableBuffer<PointLightUniforms>::create(context, bufferInfo);
	});
}

void ShadowMapPass::createPipelineLayouts()
{
	{
		VKPipelineLayoutInfo info;
		info.setPushConstantLayout<DirectionalLightPushConstantData>();
		
		_directionalLightPipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
	}
	
	{
		VKPipelineLayoutInfo info;
		info.addDescriptorSetLayout(_pointLightDescriptorSetLayout);
		info.setPushConstantLayout<PointLightPushConstantData>();
		
		_pointLightPipelineLayout = VKPipelineLayout::create(Engine::getVKContext(), info);
	}
}

void ShadowMapPass::createPipelines()
{
	{
		VKGraphicsPipelineInfo info(
			_directionalLightPipelineLayout,
			"resources/shaders/internal/shadow mapping/directional light.vert",
			vk::PrimitiveTopology::eTriangleList,
			vk::CullModeFlagBits::eBack,
			vk::FrontFace::eCounterClockwise);
		
		info.getVertexInputLayoutInfo().defineSlot(0, sizeof(PositionVertexData), vk::VertexInputRate::eVertex);
		info.getVertexInputLayoutInfo().defineAttribute(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(PositionVertexData, position));
		
		info.getPipelineAttachmentInfo().setDepthAttachment(SceneRenderer::DIRECTIONAL_SHADOW_MAP_DEPTH_FORMAT, vk::CompareOp::eLess, true);
		
		_directionalLightPipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
	}
	
	{
		VKGraphicsPipelineInfo info(
			_pointLightPipelineLayout,
			"resources/shaders/internal/shadow mapping/point light.vert",
			vk::PrimitiveTopology::eTriangleList,
			vk::CullModeFlagBits::eBack,
			vk::FrontFace::eCounterClockwise);
		
		info.setFragmentShader("resources/shaders/internal/shadow mapping/point light.frag");
		
		info.setViewMask(0b111111);
		
		info.getVertexInputLayoutInfo().defineSlot(0, sizeof(PositionVertexData), vk::VertexInputRate::eVertex);
		info.getVertexInputLayoutInfo().defineAttribute(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(PositionVertexData, position));
		
		info.getPipelineAttachmentInfo().setDepthAttachment(SceneRenderer::POINT_SHADOW_MAP_DEPTH_FORMAT, vk::CompareOp::eLess, true);
		
		_pointLightPipeline = VKGraphicsPipeline::create(Engine::getVKContext(), info);
	}
}

void ShadowMapPass::renderDirectionalShadowMap(
	const VKPtr<VKCommandBuffer>& commandBuffer,
	const DirectionalLight::RenderData& light,
	const std::vector<ModelRenderer::RenderData>& models,
	const Camera& camera)
{
	VKPtr<VKImageView> shadowMap = _shadowMapManager.allocateDirectionalShadowMap(light.shadowMapResolution);
	
	commandBuffer->imageMemoryBarrier(
		shadowMap->getInfo().getImage(),
		0,
		0,
		vk::PipelineStageFlagBits2::eNone,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::ImageLayout::eDepthAttachmentOptimal);
	
	VKRenderingInfo renderingInfo({light.shadowMapResolution, light.shadowMapResolution});
	
	renderingInfo.setDepthAttachment(shadowMap)
		.setLoadOpClear(1.0f)
		.setStoreOpStore();
	
	commandBuffer->beginRendering(renderingInfo);
	
	commandBuffer->bindPipeline(_directionalLightPipeline);
	
	VKPipelineViewport viewport;
	viewport.offset = {0, 0};
	viewport.size = {light.shadowMapResolution, light.shadowMapResolution};
	viewport.depthRange = {0.0f, 1.0f};
	commandBuffer->setViewport(viewport);
	
	VKPipelineScissor scissor;
	scissor.offset = {0, 0};
	scissor.size = {light.shadowMapResolution, light.shadowMapResolution};
	commandBuffer->setScissor(scissor);
	
	glm::mat4 viewProjection = DIRECTIONAL_SHADOW_MAP_PROJECTION * calcDirectionalShadowMapView(light, camera);
	
	for (const ModelRenderer::RenderData& model : models)
	{
		if (!model.contributeShadows)
		{
			continue;
		}
		
		const VKPtr<VKBuffer<PositionVertexData>>& vertexBuffer = model.mesh.getPositionVertexBuffer();
		const VKPtr<VKBuffer<uint32_t>>& indexBuffer = model.mesh.getIndexBuffer();
		
		commandBuffer->bindVertexBuffer(0, vertexBuffer);
		commandBuffer->bindIndexBuffer(indexBuffer);
		
		DirectionalLightPushConstantData pushConstantData{};
		pushConstantData.mvp = viewProjection * model.transform.getLocalToWorldMatrix();
		commandBuffer->pushConstants(pushConstantData);
		
		commandBuffer->drawIndexed(indexBuffer->getSize(), 0, 0);
	}
	
	commandBuffer->unbindPipeline();
	
	commandBuffer->endRendering();
	
	_directionalShadowMapInfos.push_back(DirectionalShadowMapInfo{
		.worldSize = DIRECTIONAL_SHADOW_MAP_WORLD_SIZE,
		.worldDepth = DIRECTIONAL_SHADOW_MAP_WORLD_DEPTH,
		.viewProjection = viewProjection,
		.imageView = shadowMap
	});
}

void ShadowMapPass::renderPointShadowMap(
	const VKPtr<VKCommandBuffer>& commandBuffer,
	const PointLight::RenderData& light,
	const std::vector<ModelRenderer::RenderData>& models,
	int uniformIndex)
{
	VKPtr<VKImageView> shadowMap = _shadowMapManager.allocatePointShadowMap(light.shadowMapResolution);
	
	for (int i = 0; i < 6; i++)
	{
		commandBuffer->imageMemoryBarrier(
			shadowMap->getInfo().getImage(),
			i,
			0,
			vk::PipelineStageFlagBits2::eNone,
			vk::AccessFlagBits2::eNone,
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::AccessFlagBits2::eDepthStencilAttachmentRead,
			vk::ImageLayout::eDepthAttachmentOptimal);
	}
	
	VKRenderingInfo renderingInfo({light.shadowMapResolution, light.shadowMapResolution});
	renderingInfo.setLayers(6);
	renderingInfo.setViewMask(0b111111);
	
	renderingInfo.setDepthAttachment(shadowMap)
		.setLoadOpClear(std::numeric_limits<float>::max())
		.setStoreOpStore();
	
	commandBuffer->beginRendering(renderingInfo);
	
	commandBuffer->bindPipeline(_pointLightPipeline);
	
	VKPipelineViewport viewport;
	viewport.offset = {0, 0};
	viewport.size = {light.shadowMapResolution, light.shadowMapResolution};
	viewport.depthRange = {0.0f, 1.0f};
	commandBuffer->setViewport(viewport);
	
	VKPipelineScissor scissor;
	scissor.offset = {0, 0};
	scissor.size = {light.shadowMapResolution, light.shadowMapResolution};
	commandBuffer->setScissor(scissor);
	
	std::array<glm::mat4, 6> views = calcPointShadowMapView(light);
	
	PointLightUniforms uniforms{
		.viewProjections = {
			POINT_SHADOW_MAP_PROJECTION * views[0],
			POINT_SHADOW_MAP_PROJECTION * views[1],
			POINT_SHADOW_MAP_PROJECTION * views[2],
			POINT_SHADOW_MAP_PROJECTION * views[3],
			POINT_SHADOW_MAP_PROJECTION * views[4],
			POINT_SHADOW_MAP_PROJECTION * views[5]
		},
		.lightPos = light.transform.getWorldPosition()
	};
	std::memcpy(_pointLightUniformBuffer->getHostPointer() + uniformIndex, &uniforms, sizeof(PointLightUniforms));
	
	commandBuffer->pushDescriptor(0, 0, _pointLightUniformBuffer.getCurrent()->getBuffer(), uniformIndex, 1);
	
	for (const ModelRenderer::RenderData& model : models)
	{
		if (!model.contributeShadows)
		{
			continue;
		}
		
		const VKPtr<VKBuffer<PositionVertexData>>& vertexBuffer = model.mesh.getPositionVertexBuffer();
		const VKPtr<VKBuffer<uint32_t>>& indexBuffer = model.mesh.getIndexBuffer();
		
		commandBuffer->bindVertexBuffer(0, vertexBuffer);
		commandBuffer->bindIndexBuffer(indexBuffer);
		
		PointLightPushConstantData pushConstantData{};
		pushConstantData.model = model.transform.getLocalToWorldMatrix();
		commandBuffer->pushConstants(pushConstantData);
		
		commandBuffer->drawIndexed(indexBuffer->getSize(), 0, 0);
	}
	
	commandBuffer->unbindPipeline();
	
	commandBuffer->endRendering();
	
	_pointShadowMapInfos.push_back(PointShadowMapInfo{
		.imageView = shadowMap
	});
}
