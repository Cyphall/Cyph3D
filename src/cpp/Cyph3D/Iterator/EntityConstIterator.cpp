#include "EntityConstIterator.h"

c3d::EntityConstIterator::EntityConstIterator(std::vector<Scene::EntityContainer>::const_iterator it):
	_it(it)
{}

c3d::EntityConstIterator& c3d::EntityConstIterator::operator++()
{
	_it++;
	return *this;
}

c3d::EntityConstIterator c3d::EntityConstIterator::operator++(int)
{
	EntityConstIterator temp = *this;
	++(*this);
	return temp;
}

bool c3d::EntityConstIterator::operator==(const EntityConstIterator& other) const
{
	return _it == other._it;
}

bool c3d::EntityConstIterator::operator!=(const EntityConstIterator& other) const
{
	return !this->operator==(other);
}

const c3d::Entity& c3d::EntityConstIterator::operator*()
{
	return *(this->operator->());
}

const c3d::Entity* c3d::EntityConstIterator::operator->()
{
	return _it->entity.get();
}

std::vector<c3d::Scene::EntityContainer>::const_iterator c3d::EntityConstIterator::getUnderlyingIterator()
{
	return _it;
}