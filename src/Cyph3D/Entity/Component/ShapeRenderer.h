#pragma once

#include "Cyph3D/Entity/Component/Component.h"

#include <glm/glm.hpp>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <memory>

class MaterialAsset;
class Shape;

class ShapeRenderer : public Component
{
public:
	struct RenderData
	{
		const Shape* shape;
		MaterialAsset* material;
		glm::mat4 matrix;
		bool contributeShadows;
		Entity* owner;
	};
	
	explicit ShapeRenderer(Entity& entity);

	const std::string* getMaterialPath() const;
	void setMaterialPath(std::optional<std::string_view> path);
	MaterialAsset* getMaterial() const;
	
	Shape& getShape();
	const Shape& getShape() const;
	
	template<typename T>
	T& setShape()
	{
		T* newShape = new T(*this);
		_shape.reset(newShape);
		_selectedShape = newShape->getIdentifier();
		return static_cast<T&>(*newShape);
	}
	
	bool getContributeShadows() const;
	void setContributeShadows(bool contributeShadows);
	
	void onPreRender(SceneRenderer& sceneRenderer, Camera& camera) override;
	void onDrawUi() override;
	
	void duplicate(Entity& targetEntity) const override;
	
	static const char* identifier;
	const char* getIdentifier() const override;
	
	ObjectSerialization serialize() const override;
	void deserialize(const ObjectSerialization& shapeRendererSerialization) override;

private:
	std::optional<std::string> _materialPath;
	MaterialAsset* _material = nullptr;
	
	std::unique_ptr<Shape> _shape;
	bool _contributeShadows = true;
	std::string _selectedShape;
	
	Shape& setShapeByIdentifier(const std::string& shapeIdentifier);
	
	static std::map<std::string, std::function<Shape&(ShapeRenderer&)>> _shapeFactories;
	static void initShapeFactories();
	
	friend class Engine;
};