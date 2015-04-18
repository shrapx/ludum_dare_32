#ifndef WWORLD_HPP_
#define WWORLD_HPP_

#include <vector>
#include <memory>

#include <jsoncpp/json/json.h>

#include "entity.hpp"

using namespace std;

#define CHUNK_LENGTH 8
#define CHUNK_AREA 64

#define TILE_WIDTH 64
#define TILE_HEIGHT 32
#define TILE_NUM 4

struct Tile
{
	int type = 0;
	float life = 0;
};

struct Chunk
{
	array<Tile, CHUNK_AREA> _tiles;
};

class World
{

public:

	shared_ptr<EntityBase> _player;

	unordered_map<string, shared_ptr<EntityBase>> _entities;
	unordered_map<string, shared_ptr<sf::Texture>> _textures;

	unordered_map<string, shared_ptr<EntityBase>> _tiles;

	shared_ptr<sf::Sprite> tile_sprite;

	//array<Tile, 14> _map;

	Chunk _map;

	World()
	{

	}

	void load(Json::Value& data)
	{

		/// loading textures
		for (string& image_name : data["textures"].getMemberNames() )
		{
			cout << "loading texture " << image_name;
			auto t = make_shared<sf::Texture>();
			if ( t->loadFromFile( data["textures"][image_name].asString()))
			{
				_textures[image_name] = t;
				 cout << " success." << endl;
			}
			else cout << " failed." << endl;
		}


		// load tile sprite
		tile_sprite = make_shared<sf::Sprite>( *_textures["tiles"] );

		// load entity textures
		for ( string& ent_name : data["entities"].getMemberNames() )
		{
			cout << " name: " << ent_name;

			shared_ptr<EntityBase> ent = create_entity(data["entities"][ent_name]);
			_entities[ent_name] = ent;

			if (ent_name == "player")
			{
				_player = ent;
			}

			auto it = _textures.find(ent_name);
			if (it != _textures.end()) ent->setTexture( *(it->second) );
		};

	};

	shared_ptr<EntityBase> create_entity(Json::Value& data)
	{
		shared_ptr<Entity> ent = make_shared<Entity>();

		ent->setPosition( data["pos"][0].asFloat(), data["pos"][1].asFloat() );
		ent->setRotation( data["rot"].asFloat() );

		return ent;
	}

	void draw(sf::RenderWindow& window)
	{

		for (int x=0; x < CHUNK_LENGTH; ++x)
		{
			for (int y=0; y < CHUNK_LENGTH; ++y)
			{
				/// iso-ize position
				tile_sprite->setPosition(x*TILE_WIDTH, y*TILE_HEIGHT);

				set_tile( _map._tiles[x*y] );

				window.draw(*tile_sprite);
			}
		}
	}

	void set_tile(const Tile& tile)
	{
		int frame_y = tile.type * TILE_HEIGHT;
		int frame_x = int(tile.life * TILE_NUM) * TILE_WIDTH;

		sf::IntRect tex_rect = sf::IntRect(frame_x, frame_y, TILE_WIDTH, TILE_HEIGHT);

		tile_sprite->setTextureRect(tex_rect);
	}
};

#endif
