#pragma once

#include <Cyph3D/Asset/Processing/EquirectangularSkyboxData.h>

#include <CyphGPU/fwd.hpp>
#include <filesystem>

namespace c3d
{
class EquirectangularSkyboxProcessor
{
public:
	EquirectangularSkyboxProcessor();

	EquirectangularSkyboxData readEquirectangularSkyboxData(std::string_view path, std::string_view cachePath);

private:
	cgpu::ComputeShaderStatePtr _cubemapComputeShader;
	cgpu::SamplerPtr _cubemapSampler;

	EquirectangularSkyboxData processEquirectangularSkybox(const std::filesystem::path& input, const std::filesystem::path& output);
	EquirectangularSkyboxData genCubemapAndMipmaps(vk::Format format, glm::uvec2 size, std::span<const std::byte> data);
	cgpu::ImagePtr generateCubemap(vk::Format format, const cgpu::ImagePtr& equirectangularImage);
	void generateMipmaps(const cgpu::ImagePtr& cubemapImage);
};
}
