#pragma once

#include "Cyph3D/Entity/Component/Component.h"

#include <glm/glm.hpp>
#include <functional>
#include <map>

class Material;
class Shape;

class ShapeRenderer : public Component
{
public:
	struct RenderData
	{
		const Shape* shape;
		Material* material;
		glm::mat4 matrix;
		bool contributeShadows;
		Entity* owner;
	};
	
	ShapeRenderer(Entity& entity);
	
	Material* getMaterial() const;
	void setMaterial(Material* material);
	
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
	
	void onPreRender(RenderContext& context) override;
	void onDrawUi() override;
	
	void duplicate(Entity& targetEntity) const override;
	
	static const char* identifier;
	const char* getIdentifier() const override;
	
	ObjectSerialization serialize() const override;
	void deserialize(const ObjectSerialization& shapeRendererSerialization) override;

private:
	Material* _material = nullptr;
	std::unique_ptr<Shape> _shape;
	bool _contributeShadows = true;
	std::string _selectedShape;
	
	Shape& setShapeByIdentifier(const std::string& shapeIdentifier);
	
	static std::map<std::string, std::function<Shape&(ShapeRenderer&)>> _allocators;
	static void initAllocators();
	
	Material* getDrawingMaterial() const;
	
	friend class Engine;
};