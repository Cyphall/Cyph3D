#pragma once

#include "Cyph3D/Asset/RuntimeAsset/GPUAsset.h"
#include "Cyph3D/Enums/TextureType.h"
#include "Cyph3D/HashBuilder.h"

#include <string>
#include <memory>

class GLTexture;

struct TextureAssetSignature
{
	std::string path;
	TextureType type;

	bool operator==(const TextureAssetSignature& other) const = default;
};

template<>
struct std::hash<TextureAssetSignature>
{
	std::size_t operator()(const TextureAssetSignature& key) const
	{
		return HashBuilder()
			.hash(key.path)
			.hash(key.type)
			.get();
	}
};

class TextureAsset : public GPUAsset<TextureAssetSignature>
{
public:
	~TextureAsset() override;
	
	const GLTexture& getGLTexture() const;

private:
	friend class AssetManager;

	TextureAsset(AssetManager& manager, const TextureAssetSignature& signature);

	bool load_step1_mt();
	
	std::unique_ptr<GLTexture> _glTexture;
};