#include "VKImageView.h"

#include "Cyph3D/VKObject/VKContext.h"
#include "Cyph3D/VKObject/Image/VKImage.h"

VKPtr<VKImageView> VKImageView::create(
	VKContext& context,
	const VKPtr<VKImage>& image,
	vk::ImageViewType viewType,
	std::optional<std::array<vk::ComponentSwizzle, 4>> swizzle,
	std::optional<vk::Format> viewFormat,
	std::optional<glm::uvec2> referencedLayerRange,
	std::optional<glm::uvec2> referencedLevelRange)
{
	return VKPtr<VKImageView>(new VKImageView(context, image, viewType, swizzle, viewFormat, referencedLayerRange, referencedLevelRange));
}

VKImageView::VKImageView(
	VKContext& context,
	const VKPtr<VKImage>& image,
	vk::ImageViewType viewType,
	std::optional<std::array<vk::ComponentSwizzle, 4>> swizzle,
	std::optional<vk::Format> viewFormat,
	std::optional<glm::uvec2> referencedLayerRange,
	std::optional<glm::uvec2> referencedLevelRange):
	VKObject(context),
	_image(image)
{
	vk::ImageViewCreateInfo imageViewCreateInfo;
	imageViewCreateInfo.image = image->getHandle();
	imageViewCreateInfo.viewType = viewType;
	imageViewCreateInfo.format = viewFormat ? viewFormat.value() : image->getFormat();
	
	if (swizzle.has_value())
	{
		imageViewCreateInfo.components.r = swizzle->at(0);
		imageViewCreateInfo.components.g = swizzle->at(1);
		imageViewCreateInfo.components.b = swizzle->at(2);
		imageViewCreateInfo.components.a = swizzle->at(3);
	}
	else
	{
		imageViewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
		imageViewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
	}
	
	imageViewCreateInfo.subresourceRange.aspectMask = image->getAspect();
	
	if (referencedLayerRange)
	{
		imageViewCreateInfo.subresourceRange.baseArrayLayer = referencedLayerRange->x;
		imageViewCreateInfo.subresourceRange.layerCount = referencedLayerRange->y - referencedLayerRange->x + 1;
		
		_referencedLayerRange = *referencedLayerRange;
	}
	else
	{
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = image->getLayers();
		
		_referencedLayerRange.x = 0;
		_referencedLayerRange.y = image->getLayers() - 1;
	}
	
	if (referencedLevelRange)
	{
		imageViewCreateInfo.subresourceRange.baseMipLevel = referencedLevelRange->x;
		imageViewCreateInfo.subresourceRange.levelCount = referencedLevelRange->y - referencedLevelRange->x + 1;
		
		_referencedLevelRange = *referencedLevelRange;
	}
	else
	{
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = image->getLevels();
		
		_referencedLevelRange.x = 0;
		_referencedLevelRange.y = image->getLevels() - 1;
	}
	
	_handle = _context.getDevice().createImageView(imageViewCreateInfo);
}

VKImageView::~VKImageView()
{
	_context.getDevice().destroyImageView(_handle);
}

const VKPtr<VKImage>& VKImageView::getImage()
{
	return _image;
}

vk::ImageView& VKImageView::getHandle()
{
	return _handle;
}

uint32_t VKImageView::getFirstReferencedLayer() const
{
	return _referencedLayerRange.x;
}

uint32_t VKImageView::getLastReferencedLayer() const
{
	return _referencedLayerRange.y;
}

uint32_t VKImageView::getFirstReferencedLevel() const
{
	return _referencedLevelRange.x;
}

uint32_t VKImageView::getLastReferencedLevel() const
{
	return _referencedLevelRange.y;
}