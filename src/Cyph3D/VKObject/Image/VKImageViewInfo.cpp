#include "VKImageViewInfo.h"

VKImageViewInfo::VKImageViewInfo(const VKPtr<VKImage>& image, vk::ImageViewType viewType):
	_image(image),
	_viewType(viewType)
{

}

const VKPtr<VKImage>& VKImageViewInfo::getImage() const
{
	return _image;
}

const vk::ImageViewType& VKImageViewInfo::getViewType() const
{
	return _viewType;
}

void VKImageViewInfo::setSwizzle(std::array<vk::ComponentSwizzle, 4> swizzle)
{
	_swizzle = swizzle;
}

bool VKImageViewInfo::hasSwizzle() const
{
	return _swizzle.has_value();
}

const std::array<vk::ComponentSwizzle, 4>& VKImageViewInfo::getSwizzle() const
{
	return _swizzle.value();
}

void VKImageViewInfo::setCustomViewFormat(vk::Format viewFormat)
{
	_customViewFormat = viewFormat;
}

bool VKImageViewInfo::hasCustomViewFormat() const
{
	return _customViewFormat.has_value();
}

const vk::Format& VKImageViewInfo::getCustomViewFormat() const
{
	return _customViewFormat.value();
}

void VKImageViewInfo::setCustomLayerRange(glm::uvec2 layerRange)
{
	_customLayerRange = layerRange;
}

bool VKImageViewInfo::hasCustomLayerRange() const
{
	return _customLayerRange.has_value();
}

const glm::uvec2& VKImageViewInfo::getCustomLayerRange() const
{
	return _customLayerRange.value();
}

void VKImageViewInfo::setCustomLevelRange(glm::uvec2 levelRange)
{
	_customLevelRange = levelRange;
}

bool VKImageViewInfo::hasCustomLevelRange() const
{
	return _customLevelRange.has_value();
}

const glm::uvec2& VKImageViewInfo::getCustomLevelRange() const
{
	return _customLevelRange.value();
}
