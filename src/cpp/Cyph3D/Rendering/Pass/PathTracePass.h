#pragma once

#include <Cyph3D/Rendering/Pass/RenderPass.h>

namespace c3d
{
struct RenderRegistry;
class Camera;
class VKDescriptorSetLayout;
class VKDescriptorSet;
class VKPipelineLayout;
class VKRayTracingPipeline;
class VKImage;
class VKShaderBindingTable;

struct PathTracePassInput
{
	const RenderRegistry& registry;
	Camera& camera;
	uint32_t sampleCount;
	bool sceneChanged;
	bool cameraChanged;
};

struct PathTracePassOutput
{
	std::array<std::shared_ptr<VKImage>, 3> rawRenderImage;
	uint32_t accumulatedSamples;
};

class PathTracePass : public RenderPass<PathTracePassInput, PathTracePassOutput>
{
public:
	explicit PathTracePass(const glm::uvec2& size);

private:
	struct RayGenUniforms
	{
		glm::vec3 cameraPosition;
		glm::vec3 cameraRayTL;
		glm::vec3 cameraRayTR;
		glm::vec3 cameraRayBL;
		glm::vec3 cameraRayBR;
	};

	struct RayClosestHitUniforms
	{
		glm::mat4 normalMatrix;
		vk::DeviceAddress positionVertexBuffer;
		vk::DeviceAddress materialVertexBuffer;
		vk::DeviceAddress indexBuffer;
		int32_t albedoIndex;
		int32_t normalIndex;
		int32_t roughnessIndex;
		int32_t metalnessIndex;
		int32_t displacementIndex;
		int32_t emissiveIndex;
		glm::vec3 albedoValue;
		float roughnessValue;
		float metalnessValue;
		float emissiveScale;
	};

	struct RayMissUniforms
	{
		vk::Bool32 hasSkybox;
		uint32_t skyboxIndex;
		glm::mat3 skyboxRotation;
	};

	struct FramePushConstants
	{
		glm::uvec2 topLevelAS;
		uint32_t batchIndex;
		uint32_t sampleCount;
		vk::Bool32 resetAccumulation;
	};

	std::shared_ptr<VKAccelerationStructure> _tlas;

	std::shared_ptr<VKShaderBindingTable> _sbt;

	std::shared_ptr<VKDescriptorSetLayout> _descriptorSetLayout;

	std::shared_ptr<VKDescriptorSet> _descriptorSet;

	std::shared_ptr<VKPipelineLayout> _pipelineLayout;
	std::shared_ptr<VKRayTracingPipeline> _pipeline;

	std::array<std::shared_ptr<VKImage>, 3> _rawRenderImage;

	uint32_t _batchIndex = 0;
	uint32_t _accumulatedSamples = 0;

	PathTracePassOutput onRender(const std::shared_ptr<VKCommandBuffer>& commandBuffer, PathTracePassInput& input) override;
	void onResize() override;

	void setupTLAS(const std::shared_ptr<VKCommandBuffer>& commandBuffer, const PathTracePassInput& input);
	void setupSBT(const std::shared_ptr<VKCommandBuffer>& commandBuffer, const PathTracePassInput& input);

	void createDescriptorSetLayout();
	void createPipelineLayout();
	void createPipeline();
	void createImage();
};
}