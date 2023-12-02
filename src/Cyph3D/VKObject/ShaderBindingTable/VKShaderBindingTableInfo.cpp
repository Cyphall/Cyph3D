#include "VKShaderBindingTableInfo.h"

VKShaderBindingTableInfo::VKShaderBindingTableInfo(const std::array<std::byte, 32>& raygenGroupHandle):
	VKShaderBindingTableInfo(raygenGroupHandle, nullptr, 0)
{
}

const std::vector<std::byte>& VKShaderBindingTableInfo::getRaygenRecord() const
{
	return _raygenRecord;
}

void VKShaderBindingTableInfo::addTriangleHitRecord(uint32_t recordIndex, uint32_t rayType, const std::array<std::byte, 32>& triangleHitGroupHandle)
{
	addTriangleHitRecord(recordIndex, rayType, triangleHitGroupHandle, nullptr, 0);
}

const std::vector<std::vector<std::vector<std::byte>>>& VKShaderBindingTableInfo::getTriangleHitRecords() const
{
	return _triangleHitRecords;
}

const size_t& VKShaderBindingTableInfo::getMaxTriangleHitRecordSize() const
{
	return _maxTrignaleHitRecordSize;
}

void VKShaderBindingTableInfo::addMissRecord(const std::array<std::byte, 32>& missGroupHandle)
{
	addMissRecord(missGroupHandle, nullptr, 0);
}

const std::vector<std::vector<std::byte>>& VKShaderBindingTableInfo::getMissRecords() const
{
	return _missRecords;
}

const size_t& VKShaderBindingTableInfo::getMaxMissRecordSize() const
{
	return _maxMissRecordSize;
}

VKShaderBindingTableInfo::VKShaderBindingTableInfo(const std::array<std::byte, 32>& raygenGroupHandle, const void* uniformData, size_t uniformSize)
{
	_raygenRecord.resize(32 + uniformSize);
	std::memcpy(_raygenRecord.data(), raygenGroupHandle.data(), raygenGroupHandle.size());
	std::memcpy(_raygenRecord.data() + raygenGroupHandle.size(), uniformData, uniformSize);
}

void VKShaderBindingTableInfo::addTriangleHitRecord(uint32_t recordIndex, uint32_t rayType, const std::array<std::byte, 32>& triangleHitGroupHandle, const void* uniformData, size_t uniformSize)
{
	if (_triangleHitRecords.size() <= recordIndex)
	{
		_triangleHitRecords.resize(recordIndex + 1);
	}

	std::vector<std::vector<std::byte>>& recordGroup = _triangleHitRecords[recordIndex];

	if (recordGroup.size() <= rayType)
	{
		recordGroup.resize(rayType + 1);
	}

	std::vector<std::byte>& record = recordGroup[rayType];

	record.resize(32 + uniformSize);
	std::memcpy(record.data(), triangleHitGroupHandle.data(), triangleHitGroupHandle.size());
	std::memcpy(record.data() + triangleHitGroupHandle.size(), uniformData, uniformSize);

	_maxTrignaleHitRecordSize = std::max(_maxTrignaleHitRecordSize, record.size());
}

void VKShaderBindingTableInfo::addMissRecord(const std::array<std::byte, 32>& missGroupHandle, const void* uniformData, size_t uniformSize)
{
	std::vector<std::byte>& record = _missRecords.emplace_back();

	record.resize(32 + uniformSize);
	std::memcpy(record.data(), missGroupHandle.data(), missGroupHandle.size());
	std::memcpy(record.data() + missGroupHandle.size(), uniformData, uniformSize);

	_maxMissRecordSize = std::max(_maxMissRecordSize, record.size());
}