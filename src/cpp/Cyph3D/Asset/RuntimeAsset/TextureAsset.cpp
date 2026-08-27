#include "TextureAsset.h"

#include <Cyph3D/Asset/AssetManager.h>
#include <Cyph3D/Engine.h>

#include <CyphGPU/Buffer.hpp>
#include <CyphGPU/CommandContext.hpp>
#include <CyphGPU/CommandRecorder.hpp>
#include <CyphGPU/DeviceSession.hpp>
#include <CyphGPU/Image.hpp>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

const cgpu::ImagePtr& c3d::TextureAsset::getImage() const
{
	checkLoaded();
	return _image;
}

c3d::TextureAsset::TextureAsset(AssetManager& manager, const TextureAssetSignature& signature):
	GPUAsset(manager, signature)
{
	_manager.addThreadPoolTask(&TextureAsset::load_async, this);
}

void c3d::TextureAsset::load_async()
{
	spdlog::info("Loading texture [{} ({})]...", _signature.path, magic_enum::enum_name(_signature.type));

	ImageData imageData = _manager.getAssetProcessor().readImageData(_signature.path, _signature.type);

	// create image
	_image = cgpu::Image::create(
		Engine::getDeviceSession(),
		{
			.name = _signature.path,
			.format = imageData.format,
			.extent = {imageData.extent, 1},
			.usages =
				vk::ImageUsageFlagBits::eTransferDst |
				vk::ImageUsageFlagBits::eSampled,
			.levels = static_cast<uint32_t>(imageData.levels.size()),
		}
	);

	// create staging buffer
	cgpu::BufferPtr stagingBuffer = cgpu::Buffer::create(
		Engine::getDeviceSession(),
		{
			.name = std::format("{} (staging buffer)", _image->getDesc().name),
			.size = _image->calcByteSize({0, _image->getDesc().levels}, 1),
			.usages = vk::BufferUsageFlagBits2::eTransferSrc,
			.memory_type = cgpu::MemoryType::eCPUUncached,
		}
	);

	// copy texture data to staging buffer
	std::byte* ptr = stagingBuffer->getHostPtr();
	for (uint32_t level = 0; level < _image->getDesc().levels; level++)
	{
		std::copy_n(imageData.levels[level].data(), imageData.levels[level].size(), ptr);
		ptr += imageData.levels[level].size();
	}

	// upload staging buffer to texture
	auto commandRecorder = assetCommandContext->createRecorder(Engine::getDeviceSession()->getAsyncTransferQueue());

	std::vector<cgpu::CommandRecorder::CopyBufferToImageParams::Range> ranges;
	vk::DeviceSize bufferOffset = 0;
	for (uint32_t level = 0; level < _image->getDesc().levels; level++)
	{
		size_t size = _image->calcByteSize({level, 1}, 1);

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
	spdlog::info("Texture [{} ({})] uploaded succesfully", _signature.path, magic_enum::enum_name(_signature.type));

	_changed();
}
