#pragma once

#include <array>
#include <vector>

class VKShaderBindingTableInfo
{
public:
	explicit VKShaderBindingTableInfo(const std::array<std::byte, 32>& raygenGroupHandle);
	
	template<typename TUniforms>
	VKShaderBindingTableInfo(const std::array<std::byte, 32>& raygenGroupHandle, const TUniforms& uniforms):
		VKShaderBindingTableInfo(raygenGroupHandle, &uniforms, sizeof(TUniforms))
	{
	
	}
	
	const std::vector<std::byte>& getRaygenRecord() const;
	
	void addTriangleHitRecord(uint32_t recordIndex, uint32_t rayType, const std::array<std::byte, 32>& triangleHitGroupHandle);
	
	template<typename TUniforms>
	void addTriangleHitRecord(uint32_t recordIndex, uint32_t rayType, const std::array<std::byte, 32>& triangleHitGroupHandle, const TUniforms& uniforms)
	{
		addTriangleHitRecord(recordIndex, rayType, triangleHitGroupHandle, &uniforms, sizeof(TUniforms));
	}
	
	const std::vector<std::vector<std::vector<std::byte>>>& getTriangleHitRecords() const;
	const size_t& getMaxTriangleHitRecordSize() const;
	
	void addMissRecord(const std::array<std::byte, 32>& missGroupHandle);
	
	template<typename TUniforms>
	void addMissRecord(const std::array<std::byte, 32>& missGroupHandle, const TUniforms& uniforms)
	{
		addMissRecord(missGroupHandle, &uniforms, sizeof(TUniforms));
	}
	
	const std::vector<std::vector<std::byte>>& getMissRecords() const;
	const size_t& getMaxMissRecordSize() const;

private:
	std::vector<std::byte> _raygenRecord;
	std::vector<std::vector<std::vector<std::byte>>> _triangleHitRecords;
	std::vector<std::vector<std::byte>> _missRecords;
	
	size_t _maxTrignaleHitRecordSize = 0;
	size_t _maxMissRecordSize = 0;
	
	explicit VKShaderBindingTableInfo(const std::array<std::byte, 32>& raygenGroupHandle, const void* uniformData, size_t uniformSize);
	void addTriangleHitRecord(uint32_t recordIndex, uint32_t rayType, const std::array<std::byte, 32>& triangleHitGroupHandle, const void* uniformData, size_t uniformSize);
	void addMissRecord(const std::array<std::byte, 32>& missGroupHandle, const void* uniformData, size_t uniformSize);
};