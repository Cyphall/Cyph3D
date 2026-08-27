#include "CubemapAsset.h"

#include <Cyph3D/Asset/AssetManager.h>
#include <Cyph3D/Engine.h>

#include <CyphGPU/Buffer.hpp>
#include <CyphGPU/CommandContext.hpp>
#include <CyphGPU/CommandRecorder.hpp>
#include <CyphGPU/DeviceSession.hpp>
#include <CyphGPU/Image.hpp>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

const cgpu::ImagePtr& c3d::CubemapAsset::getImage() const
{
	checkLoaded();
	return _image;
}

c3d::CubemapAsset::CubemapAsset(AssetManager& manager, const CubemapAssetSignature& signature):
	GPUAsset(manager, signature)
{
	_manager.addThreadPoolTask(&CubemapAsset::load_async, this);
}

void c3d::CubemapAsset::load_async()
{
	std::reference_wrapper<std::string> paths[6] = {
		_signature.xposPath,
		_signature.xnegPath,
		_signature.yposPath,
		_signature.ynegPath,
		_signature.zposPath,
		_signature.znegPath,
	};

	std::variant<EquirectangularSkyboxData, std::array<ImageData, 6>> data{};
	vk::Format format{};
	glm::uvec2 extent{};
	uint32_t levels{};
	if (!_signature.equirectangularPath.empty())
	{
		spdlog::info("Loading cubemap [equirectangular: {}]...", _signature.equirectangularPath);

		auto& equirectangularSkyboxData = data.emplace<EquirectangularSkyboxData>();
		equirectangularSkyboxData = _manager.getAssetProcessor().readEquirectangularSkyboxData(_signature.equirectangularPath);
		format = equirectangularSkyboxData.format;
		extent = equirectangularSkyboxData.extent;
		levels = equirectangularSkyboxData.levels.size();
	}
	else
	{
		spdlog::info(
			"Loading cubemap [xpos: {}, xneg: {}, ypos: {}, yneg: {}, zpos: {}, zneg: {} ({})]...",
			_signature.xposPath, _signature.xnegPath,
			_signature.yposPath, _signature.ynegPath,
			_signature.zposPath, _signature.znegPath,
			magic_enum::enum_name(_signature.type)
		);

		auto& imageDatas = data.emplace<std::array<ImageData, 6>>();
		for (uint32_t face = 0; face < 6; face++)
		{
			imageDatas[face] = _manager.getAssetProcessor().readImageData(paths[face].get(), _signature.type);

			if (face == 0)
			{
				format = imageDatas[face].format;
				extent = imageDatas[face].extent;
				levels = imageDatas[face].levels.size();
			}
			else if (format != imageDatas[face].format)
			{
				throw std::runtime_error("All 6 faces of a cubemap must have the same format.");
			}
			else if (extent != imageDatas[face].extent)
			{
				throw std::runtime_error("All 6 faces of a cubemap must have the same extent.");
			}
		}
	}

	// create image
	_image = cgpu::Image::create(
		Engine::getDeviceSession(),
		{
			.name =
				_signature.equirectangularPath.empty() ?
					std::format(
						"{}|{}|{}|{}|{}|{}",
						_signature.xposPath, _signature.xnegPath,
						_signature.yposPath, _signature.ynegPath,
						_signature.zposPath, _signature.znegPath
					) :
					_signature.equirectangularPath,
			.format = format,
			.extent = {extent, 1},
			.usages =
				vk::ImageUsageFlagBits::eTransferDst |
				vk::ImageUsageFlagBits::eSampled,
			.levels = levels,
			.layers = 6,
			.allow_cube_view = true,
		}
	);

	// create staging buffer
	cgpu::BufferPtr stagingBuffer = cgpu::Buffer::create(
		Engine::getDeviceSession(),
		{
			.name = std::format("{} (staging buffer)", _image->getDesc().name),
			.size = _image->calcByteSize({0, _image->getDesc().levels}, 6),
			.usages = vk::BufferUsageFlagBits2::eTransferSrc,
			.memory_type = cgpu::MemoryType::eCPUUncached,
		}
	);

	// copy face data to staging buffer
	if (!_signature.equirectangularPath.empty())
	{
		auto& equirectangularSkyboxData = std::get<EquirectangularSkyboxData>(data);

		std::byte* ptr = stagingBuffer->getHostPtr();
		for (uint32_t level = 0; level < _image->getDesc().levels; level++)
		{
			std::copy_n(equirectangularSkyboxData.levels[level].data(), equirectangularSkyboxData.levels[level].size(), ptr);
			ptr += equirectangularSkyboxData.levels[level].size();
		}
	}
	else
	{
		auto& imageDatas = std::get<std::array<ImageData, 6>>(data);

		std::byte* ptr = stagingBuffer->getHostPtr();
		for (uint32_t level = 0; level < _image->getDesc().levels; level++)
		{
			for (uint32_t face = 0; face < 6; face++)
			{
				std::copy_n(imageDatas[face].levels[level].data(), imageDatas[face].levels[level].size(), ptr);
				ptr += imageDatas[face].levels[level].size();
			}
		}
	}

	// upload staging buffer to image
	auto commandRecorder = assetCommandContext->createRecorder(Engine::getDeviceSession()->getAsyncTransferQueue());

	std::vector<cgpu::CommandRecorder::CopyBufferToImageParams::Range> ranges;
	vk::DeviceSize bufferOffset = 0;
	for (uint32_t level = 0; level < _image->getDesc().levels; level++)
	{
		size_t size = _image->calcByteSize({level, 1}, 6);

		ranges.push_back({
			.src = {{
				.byte_range = {{bufferOffset, size}},
			}},
			.dst = {{
				.level = level,
			}},
		});

		bufferOffset += size;
	}

	commandRecorder.copyBufferToImage({
		.src_buffer = stagingBuffer,
		.dst_image = _image,
		.ranges = ranges,
	});

	commandRecorder.submit().waitFinished();

	assetCommandContext->finish();

	_loaded = true;
	if (!_signature.equirectangularPath.empty())
	{
		spdlog::info("Cubemap [equirectangular: {}] uploaded succesfully", _signature.equirectangularPath);
	}
	else
	{
		spdlog::info(
			"Cubemap [xpos: {}, xneg: {}, ypos: {}, yneg: {}, zpos: {}, zneg: {} ({})] uploaded succesfully",
			_signature.xposPath,
			_signature.xnegPath,
			_signature.yposPath,
			_signature.ynegPath,
			_signature.zposPath,
			_signature.znegPath,
			magic_enum::enum_name(_signature.type)
		);
	}

	_changed();
}
