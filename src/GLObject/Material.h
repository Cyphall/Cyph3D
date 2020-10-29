#pragma once

#include "MaterialShaderProgram.h"
#include "../ResourceManagement/Image.h"
#include <map>
#include <tuple>

class ResourceManager;

class Material
{
public:
	void bind(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos);
	
	const std::string& getName() const;
	void setName(std::string name);
	
	static void initializeDefault();
	static Material* getDefault();

private:
	MaterialShaderProgram* _shaderProgram = nullptr;
	std::map<std::string, std::tuple<std::unique_ptr<Texture>, Image*>> _textures;
	bool _loaded = false;
	std::string _name;
	static std::unique_ptr<Material> _default;
	
	Material();
	explicit Material(std::string name, ResourceManager* resourceManager);
	
	friend class ResourceManager;
};
