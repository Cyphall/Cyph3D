#pragma once

#include "Cyph3D/Scene/Scene.h"

#include <vector>

class Entity;

class EntityIterator
{
public:
	using iterator_category = std::input_iterator_tag;
	using difference_type = std::ptrdiff_t;
	using value_type = Entity;
	using pointer = value_type*;
	using reference = value_type&;

	EntityIterator() = default;

	explicit EntityIterator(std::vector<Scene::EntityContainer>::iterator it);

	EntityIterator& operator++();
	EntityIterator operator++(int);

	bool operator==(const EntityIterator& other) const;
	bool operator!=(const EntityIterator& other) const;

	Entity& operator*();
	Entity* operator->();

	std::vector<Scene::EntityContainer>::iterator getUnderlyingIterator();

private:
	std::vector<Scene::EntityContainer>::iterator _it;
};