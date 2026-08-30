#include "ExposurePass.h"

#include <Cyph3D/Engine.h>
#include <Cyph3D/Rendering/SceneRenderer/SceneRenderer.h>
#include <Cyph3D/Scene/Camera.h>

#include <CyphGPU/ComputePassContext.hpp>
#include <CyphGPU/ComputeShaderState.hpp>

c3d::ExposurePass::ExposurePass(glm::uvec2 size):
	RenderPass(size, "Exposure pass")
{
	createPipelineState();
}

c3d::ExposurePassOutput c3d::ExposurePass::onRender(cgpu::CommandRecorder& commandRecorder, ExposurePassInput& input)
{
	commandRecorder.computePass({
		.callback = [&](cgpu::ComputePassContext& ctx) {
			using namespace cgpu::shader_types;
			struct
			{
				uint2 u_size;
				RWTexture2D<>::Handle u_image;
				float u_exposure;
			} parameters{};

			parameters.u_size = _size;
			parameters.u_image = ctx.getStorageImageDescriptor(input.lightImage, cgpu::StorageAccess::eReadWrite);
			parameters.u_exposure = input.camera.getExposure();

			ctx.dispatch(_computeShaderState, {_size, 1}, {8, 8, 1}, parameters);
		},
	});

	return {
		.lightImage = input.lightImage,
	};
}

void c3d::ExposurePass::createPipelineState()
{
	_computeShaderState = cgpu::ComputeShaderState::create(
		Engine::getDeviceSession(),
		{
			.compute_shader = {.source = "Cyph3D/post-processing/exposure/exposure.slang"},
		}
	);
}
