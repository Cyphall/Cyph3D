#pragma once

#include "Cyph3D/Scene/Scene.h"

#include <vector>

class Entity;

class EntityConstIterator
{
public:
	using iterator_category = std::input_iterator_tag;
	using difference_type = std::ptrdiff_t;
	using value_type = Entity;
	using pointer = value_type*;
	using reference = value_type&;

	EntityConstIterator() = default;

	explicit EntityConstIterator(std::vector<Scene::EntityContainer>::const_iterator it);

	EntityConstIterator& operator++();
	EntityConstIterator operator++(int);

	bool operator==(const EntityConstIterator& other) const;
	bool operator!=(const EntityConstIterator& other) const;

	const Entity& operator*();
	const Entity* operator->();

	std::vector<Scene::EntityContainer>::const_iterator getUnderlyingIterator();

private:
	std::vector<Scene::EntityContainer>::const_iterator _it;
};