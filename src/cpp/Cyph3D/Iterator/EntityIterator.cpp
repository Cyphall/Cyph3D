#include "EntityIterator.h"

c3d::EntityIterator::EntityIterator(std::vector<Scene::EntityContainer>::iterator it):
	_it(it)
{}

c3d::EntityIterator& c3d::EntityIterator::operator++()
{
	_it++;
	return *this;
}

c3d::EntityIterator c3d::EntityIterator::operator++(int)
{
	EntityIterator temp = *this;
	++(*this);
	return temp;
}

bool c3d::EntityIterator::operator==(const EntityIterator& other) const
{
	return _it == other._it;
}

bool c3d::EntityIterator::operator!=(const EntityIterator& other) const
{
	return !this->operator==(other);
}

c3d::Entity& c3d::EntityIterator::operator*()
{
	return *(this->operator->());
}

c3d::Entity* c3d::EntityIterator::operator->()
{
	return _it->entity.get();
}

std::vector<c3d::Scene::EntityContainer>::iterator c3d::EntityIterator::getUnderlyingIterator()
{
	return _it;
}