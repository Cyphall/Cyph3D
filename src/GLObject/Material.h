#pragma once

#include "MaterialShaderProgram.h"
#include "../ResourceManagement/Image.h"
#include <map>
#include <tuple>

class ResourceManager;

class Material
{
public:
	void bind(const glm::mat4& model, const glm::mat4& vp, const glm::vec3& cameraPos, int objectIndex);
	
	const std::string& getName() const;
	void setName(std::string name);
	
	static void initialize();
	
	static Material* getDefault();
	static Material* getMissing();

private:
	MaterialShaderProgram* _shaderProgram = nullptr;
	std::map<std::string, std::tuple<std::unique_ptr<Texture>, Image*>> _textures;
	bool _loaded = false;
	std::string _name;
	
	static Material* _default;
	static Material* _missing;
	
	explicit Material(std::string name, ResourceManager* resourceManager);
	
	friend class ResourceManager;
};
