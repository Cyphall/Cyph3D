#include "VKShaderBindingTableInfo.h"

c3d::VKShaderBindingTableInfo::Record::Record(std::span<const std::byte> groupHandle, const void* uniformData, size_t uniformSize):
	groupHandle(groupHandle)
{
	uniforms.resize(uniformSize);
	std::memcpy(uniforms.data(), uniformData, uniformSize);
}

c3d::VKShaderBindingTableInfo::VKShaderBindingTableInfo(std::span<const std::byte> raygenGroupHandle):
	VKShaderBindingTableInfo(raygenGroupHandle, nullptr, 0)
{
}

c3d::VKShaderBindingTableInfo::VKShaderBindingTableInfo(std::span<const std::byte> raygenGroupHandle, const void* raygenUniformData, size_t raygenUniformSize):
	_raygenRecord(raygenGroupHandle, raygenUniformData, raygenUniformSize)
{
	_maxRaygenRecordSize = std::max(_maxRaygenRecordSize, raygenGroupHandle.size() + raygenUniformSize);
}

const c3d::VKShaderBindingTableInfo::Record& c3d::VKShaderBindingTableInfo::getRaygenRecord() const
{
	return _raygenRecord;
}

const size_t& c3d::VKShaderBindingTableInfo::getMaxRaygenRecordSize() const
{
	return _maxRaygenRecordSize;
}

void c3d::VKShaderBindingTableInfo::addTriangleHitRecord(std::span<const std::byte> triangleHitGroupHandle)
{
	addTriangleHitRecord(triangleHitGroupHandle, nullptr, 0);
}

void c3d::VKShaderBindingTableInfo::addTriangleHitRecord(std::span<const std::byte> triangleHitGroupHandle, const void* triangleHitUniformData, size_t triangleHitUniformSize)
{
	_triangleHitRecords.emplace_back(triangleHitGroupHandle, triangleHitUniformData, triangleHitUniformSize);
	_maxTrignaleHitRecordSize = std::max(_maxTrignaleHitRecordSize, triangleHitGroupHandle.size() + triangleHitUniformSize);
}

const std::vector<c3d::VKShaderBindingTableInfo::Record>& c3d::VKShaderBindingTableInfo::getTriangleHitRecords() const
{
	return _triangleHitRecords;
}

const size_t& c3d::VKShaderBindingTableInfo::getMaxTriangleHitRecordSize() const
{
	return _maxTrignaleHitRecordSize;
}

void c3d::VKShaderBindingTableInfo::addMissRecord(std::span<const std::byte> missGroupHandle)
{
	addMissRecord(missGroupHandle, nullptr, 0);
}

void c3d::VKShaderBindingTableInfo::addMissRecord(std::span<const std::byte> missGroupHandle, const void* missUniformData, size_t missUniformSize)
{
	_missRecords.emplace_back(missGroupHandle, missUniformData, missUniformSize);
	_maxMissRecordSize = std::max(_maxMissRecordSize, missGroupHandle.size() + missUniformSize);
}

const std::vector<c3d::VKShaderBindingTableInfo::Record>& c3d::VKShaderBindingTableInfo::getMissRecords() const
{
	return _missRecords;
}

const size_t& c3d::VKShaderBindingTableInfo::getMaxMissRecordSize() const
{
	return _maxMissRecordSize;
}