#include "ImageCompressor.h"

#include <CyphGPU/Utils.hpp>
#include <half.hpp>
#include <ispc_texcomp.h>

namespace
{
template<size_t BlockWidth, size_t BlockHeight, size_t NumChannels, class ComponentType>
void makeAlignedSurface(
	rgba_surface& surface,
	std::vector<uint8_t>& storage
)
{
	if (surface.width % BlockWidth == 0 && surface.height % BlockHeight == 0)
		return;

	rgba_surface original = surface;

	glm::uvec2 alignedExtent = cgpu::alignUp<glm::uvec2>({surface.width, surface.height}, {BlockWidth, BlockHeight});

	storage.resize(alignedExtent.x * alignedExtent.y * NumChannels * sizeof(ComponentType));

	surface.ptr = storage.data();
	surface.width = alignedExtent.x;
	surface.height = alignedExtent.y;
	surface.stride = static_cast<int32_t>(alignedExtent.x * NumChannels * sizeof(ComponentType));
	ReplicateBorders(&surface, &original, 0, 0, NumChannels * sizeof(ComponentType) << 3);
}

void compressImageBC4(std::span<const std::byte> uncompressedImage, const glm::uvec2& uncompressedExtent, std::span<std::byte> compressedImage)
{
	rgba_surface src{
		.ptr = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(uncompressedImage.data())),
		.width = static_cast<int32_t>(uncompressedExtent.x),
		.height = static_cast<int32_t>(uncompressedExtent.y),
		.stride = static_cast<int32_t>(uncompressedExtent.x * sizeof(uint8_t) * 1)
	};

	std::vector<uint8_t> storage;
	makeAlignedSurface<4, 4, 1, uint8_t>(src, storage);

	uint8_t* dst = reinterpret_cast<uint8_t*>(compressedImage.data());

	CompressBlocksBC4(&src, dst);
}

void compressImageBC5(std::span<const std::byte> uncompressedImage, const glm::uvec2& uncompressedExtent, std::span<std::byte> compressedImage)
{
	rgba_surface src{
		.ptr = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(uncompressedImage.data())),
		.width = static_cast<int32_t>(uncompressedExtent.x),
		.height = static_cast<int32_t>(uncompressedExtent.y),
		.stride = static_cast<int32_t>(uncompressedExtent.x * sizeof(uint8_t) * 2)
	};

	std::vector<uint8_t> storage;
	makeAlignedSurface<4, 4, 2, uint8_t>(src, storage);

	uint8_t* dst = reinterpret_cast<uint8_t*>(compressedImage.data());

	CompressBlocksBC5(&src, dst);
}

void compressImageBC6(std::span<const std::byte> uncompressedImage, const glm::uvec2& uncompressedExtent, std::span<std::byte> compressedImage)
{
	bc6h_enc_settings settings{};
	GetProfile_bc6h_veryfast(&settings);

	rgba_surface src{
		.ptr = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(uncompressedImage.data())),
		.width = static_cast<int32_t>(uncompressedExtent.x),
		.height = static_cast<int32_t>(uncompressedExtent.y),
		.stride = static_cast<int32_t>(uncompressedExtent.x * sizeof(half_float::half) * 4)
	};

	std::vector<uint8_t> storage;
	makeAlignedSurface<4, 4, 4, half_float::half>(src, storage);

	uint8_t* dst = reinterpret_cast<uint8_t*>(compressedImage.data());

	CompressBlocksBC6H(&src, dst, &settings);
}

void compressImageBC7(std::span<const std::byte> uncompressedImage, const glm::uvec2& uncompressedExtent, std::span<std::byte> compressedImage)
{
	bc7_enc_settings settings{};
	GetProfile_ultrafast(&settings);

	rgba_surface src{
		.ptr = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(uncompressedImage.data())),
		.width = static_cast<int32_t>(uncompressedExtent.x),
		.height = static_cast<int32_t>(uncompressedExtent.y),
		.stride = static_cast<int32_t>(uncompressedExtent.x * sizeof(uint8_t) * 4)
	};

	std::vector<uint8_t> storage;
	makeAlignedSurface<4, 4, 4, uint8_t>(src, storage);

	uint8_t* dst = reinterpret_cast<uint8_t*>(compressedImage.data());

	CompressBlocksBC7(&src, dst, &settings);
}
}

void c3d::ImageCompressor::compressImage(std::span<const std::byte> uncompressedImage, const glm::uvec2& uncompressedExtent, vk::Format compressedFormat, std::span<std::byte> compressedImage)
{
	switch (compressedFormat)
	{
	case vk::Format::eBc4UnormBlock:
		compressImageBC4(uncompressedImage, uncompressedExtent, compressedImage);
		break;
	case vk::Format::eBc5UnormBlock:
		compressImageBC5(uncompressedImage, uncompressedExtent, compressedImage);
		break;
	case vk::Format::eBc6HUfloatBlock:
		compressImageBC6(uncompressedImage, uncompressedExtent, compressedImage);
		break;
	case vk::Format::eBc7SrgbBlock:
		compressImageBC7(uncompressedImage, uncompressedExtent, compressedImage);
		break;
	default:
		throw std::logic_error(std::format("Unsupported format: {}", vk::to_string(compressedFormat)));
	}
}
