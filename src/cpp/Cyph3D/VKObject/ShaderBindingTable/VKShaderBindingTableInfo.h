#pragma once

#include <array>
#include <vector>

namespace c3d
{
class VKShaderBindingTableInfo
{
public:
	struct Record
	{
		std::span<const std::byte> groupHandle;
		std::vector<std::byte> uniforms;

		Record() = default;

		explicit Record(std::span<const std::byte> groupHandle, const void* uniformData, size_t uniformSize);
	};

	explicit VKShaderBindingTableInfo(std::span<const std::byte> raygenGroupHandle);

	template<typename TUniforms>
	VKShaderBindingTableInfo(std::span<const std::byte> raygenGroupHandle, const TUniforms& raygenUniforms):
		VKShaderBindingTableInfo(raygenGroupHandle, &raygenUniforms, sizeof(TUniforms))
	{
	}

	explicit VKShaderBindingTableInfo(std::span<const std::byte> raygenGroupHandle, const void* raygenUniformData, size_t raygenUniformSize);

	const Record& getRaygenRecord() const;
	const size_t& getMaxRaygenRecordSize() const;

	void addTriangleHitRecord(std::span<const std::byte> triangleHitGroupHandle);

	template<typename TUniforms>
	void addTriangleHitRecord(std::span<const std::byte> triangleHitGroupHandle, const TUniforms& triangleHitUniforms)
	{
		addTriangleHitRecord(triangleHitGroupHandle, &triangleHitUniforms, sizeof(TUniforms));
	}

	void addTriangleHitRecord(std::span<const std::byte> triangleHitGroupHandle, const void* triangleHitUniformData, size_t triangleHitUniformSize);

	const std::vector<Record>& getTriangleHitRecords() const;
	const size_t& getMaxTriangleHitRecordSize() const;

	void addMissRecord(std::span<const std::byte> missGroupHandle);

	template<typename TUniforms>
	void addMissRecord(std::span<const std::byte> missGroupHandle, const TUniforms& missUniforms)
	{
		addMissRecord(missGroupHandle, &missUniforms, sizeof(TUniforms));
	}

	void addMissRecord(std::span<const std::byte> missGroupHandle, const void* missUniformData, size_t missUniformSize);

	const std::vector<Record>& getMissRecords() const;
	const size_t& getMaxMissRecordSize() const;

private:
	Record _raygenRecord;
	std::vector<Record> _triangleHitRecords;
	std::vector<Record> _missRecords;

	size_t _maxRaygenRecordSize = 0;
	size_t _maxTrignaleHitRecordSize = 0;
	size_t _maxMissRecordSize = 0;
};
}