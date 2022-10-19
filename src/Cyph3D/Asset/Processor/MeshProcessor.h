#pragma once

#include "Cyph3D/Asset/Processor/MeshData.h"

#include <crossguid/guid.hpp>

class MeshProcessor
{
private:
	friend class AssetManager;

	static MeshData readMeshData(const xg::Guid& guid, std::string_view path);
};