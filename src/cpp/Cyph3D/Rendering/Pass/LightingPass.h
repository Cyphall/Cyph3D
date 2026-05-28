#pragma once

#include <Cyph3D/Rendering/Pass/RenderPass.h>
#include <Cyph3D/Rendering/Pass/ShadowMapPass.h>
#include <Cyph3D/VKObject/VKDynamic.h>

namespace c3d
{
class RenderRegistry;
class Camera;
class VKPipelineLayout;
class VKGraphicsPipeline;
class VKDescriptorSetLayout;
class VKImage;
template<typename T>
class VKResizableBuffer;

struct LightingPassInput
{
	const std::shared_ptr<VKImage>& multisampledDepthImage;
	const RenderRegistry& registry;
	Camera& camera;
	const std::vector<DirectionalShadowMapInfo>& directionalShadowMapInfos;
	const std::vector<PointShadowMapInfo>& pointShadowMapInfos;
	float pointLightMaxDistance;
};

struct LightingPassOutput
{
	const std::shared_ptr<VKImage>& multisampledRawRenderImage;
};

class LightingPass : public RenderPass<LightingPassInput, LightingPassOutput>
{
public:
	explicit LightingPass(glm::uvec2 size);

private:
	struct DirectionalLightUniforms
	{
		glm::vec3 fragToLightDirection;
		float intensity;
		glm::vec3 color;
		vk::Bool32 castShadows;
		glm::mat4 lightViewProjection;
		uint32_t textureIndex;
		float shadowMapTexelWorldSize;
	};

	struct PointLightUniforms
	{
		glm::vec3 pos;
		float intensity;
		glm::vec3 color;
		vk::Bool32 castShadows;
		uint32_t textureIndex;
		float maxTexelSizeAtUnitDistance;
	};

	//FIXME: properly align storage buffer offset
	struct alignas(16) ObjectUniforms
	{
		glm::mat4 normalMatrix;
		glm::mat4 model;
		glm::mat4 mvp;
		int32_t albedoIndex;
		int32_t normalIndex;
		int32_t roughnessIndex;
		int32_t metalnessIndex;
		int32_t displacementIndex;
		int32_t emissiveIndex;
		glm::vec3 albedoValue;
		float roughnessValue;
		float metalnessValue;
		float displacementScale;
		float emissiveScale;
	};

	struct PushConstantData
	{
		glm::vec3 viewPos;
		uint32_t frameIndex;
		int32_t directionalLightCount;
		int32_t pointLightCount;
		float pointLightMaxDistance;
	};

	VKDynamic<VKResizableBuffer<DirectionalLightUniforms>> _directionalLightsUniforms;
	VKDynamic<VKResizableBuffer<PointLightUniforms>> _pointLightsUniforms;

	VKDynamic<VKResizableBuffer<ObjectUniforms>> _objectUniforms;

	std::shared_ptr<VKSampler> _directionalLightSampler;
	std::shared_ptr<VKSampler> _pointLightSampler;

	std::shared_ptr<VKDescriptorSetLayout> _directionalLightDescriptorSetLayout;
	VKDynamic<VKDescriptorSet> _directionalLightDescriptorSet;
	std::shared_ptr<VKDescriptorSetLayout> _pointLightDescriptorSetLayout;
	VKDynamic<VKDescriptorSet> _pointLightDescriptorSet;

	std::shared_ptr<VKDescriptorSetLayout> _objectDescriptorSetLayout;

	std::shared_ptr<VKPipelineLayout> _pipelineLayout;
	std::shared_ptr<VKGraphicsPipeline> _pipeline;

	std::shared_ptr<VKImage> _multisampledRawRenderImage;

	uint32_t _frameIndex = 0;

	LightingPassOutput onRender(const std::shared_ptr<VKCommandBuffer>& commandBuffer, LightingPassInput& input) override;
	void onResize() override;

	void createUniformBuffers();
	void createSamplers();
	void createDescriptorSetLayouts();
	void createPipelineLayout();
	void createPipeline();
	void createImage();

	void descriptorSetsResizeSmart(uint32_t directionalLightShadowsCount, uint32_t pointLightShadowsCount);
};
}