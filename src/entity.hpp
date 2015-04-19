#ifndef _ENTITY_HPP_
#define _ENTITY_HPP_

using namespace std;

class EntityBase : public sf::Sprite
{

public:

	sf::Vector2f tile_position;
	sf::Vector2f tile_unit_pos;
	sf::Vector2f tile_normal;

	sf::Vector2f water_vector;
	float water_age=0;
	float water_y;

  float tile_angle;

  int rotate_frames = 0;
  int anim_frames = 0;

  float walk_anim_marker;

  float water;

};

class Entity : public EntityBase
{
};

#endif
