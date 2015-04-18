#ifndef _ENTITY_HPP_
#define _ENTITY_HPP_

using namespace std;

class EntityBase : public sf::Sprite
{
public:
	float x,y,z;
	float angle;
};

class Entity : public EntityBase
{
};

#endif
