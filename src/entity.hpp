#ifndef _ENTITY_HPP_
#define _ENTITY_HPP_

using namespace std;

class EntityBase : public sf::Sprite
{

public:

	sf::Vector2f tile_position;

	sf::Vector2f tile_normal;

  float tile_angle;

  int rotate_frames = 0;
  int anim_frames = 0;

};

class Entity : public EntityBase
{
};

#endif
