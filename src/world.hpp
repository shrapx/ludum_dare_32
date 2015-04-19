#ifndef WWORLD_HPP_
#define WWORLD_HPP_

#include <vector>
#include <memory>

#include <jsoncpp/json/json.h>

#include "entity.hpp"

using namespace std;

#define CHUNK_LENGTH 4
#define CHUNK_AREA 16

#define TILE_WIDTH 32
#define TILE_HEIGHT 16
#define TILE_NUM 8

struct Tile
{
	Tile()
	{
		type = rand() % 6;
		//life = (rand() % 10) * 0.1f;
	}

	sf::IntRect rect() const
	{
		int frame_y = type * TILE_HEIGHT * 2;
		int frame_x = int(life * (TILE_NUM-1)) * TILE_WIDTH * 2;

		return sf::IntRect(frame_x, frame_y, TILE_WIDTH * 2, TILE_HEIGHT * 2);
	}

	void water(float val)
	{
		life += val;
		life = life > 1.0f ? 1.0f : life;
	}

	int type = 2;
	float life = 0;
};

struct Chunk
{
	array<Tile, CHUNK_AREA> _tiles;
};

class World
{

public:

	// order of chunk drawing, and depth of each chunk
	array<int,25> chunk_draw_order = {{0,5,1,10,6,2,15,11,7,3,20,16,12,8,4,21,17,13,9,22,18,14,23,19,24}};
	array<int,25> chunk_draw_depths = {{0,1,1,2,2,2,3,3,3,3,4,4,4,4,4,5,5,5,5,6,6,6,7,7,8}};

	// order of tile drawing, and depth of each tile
	array<int,16> tile_draw_order = {{0,4,1,8,5,2,12,9,6,3,13,10,7,14,11,15}};
	array<int,16> tile_draw_depths = {{0,1,1,2,2,2,3,3,3,3,4,4,4,5,5,6}};

	shared_ptr<EntityBase> _player;

	unordered_map<string, shared_ptr<EntityBase>> _entities;
	unordered_map<string, shared_ptr<sf::Texture>> _textures;

	shared_ptr<sf::Sprite> tile_sprite;

	unordered_map<int, unordered_map<int, Chunk>> _map;

	sf::Vector2f chunk_address;

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

		tile_sprite->setOrigin(32,16);

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
		if (data.isMember("origin"))
			ent->setOrigin( data["origin"][0].asFloat(), data["origin"][1].asFloat() );

		if (data.isMember("pos"))
			ent->tile_position = sf::Vector2f( data["pos"][0].asFloat(), data["pos"][1].asFloat() );

		if (data.isMember("rot"))
			ent->tile_angle = data["rot"].asFloat();
		if (data.isMember("frames"))
		{
			ent->rotate_frames = data["frames"][0].asInt();
			ent->anim_frames = data["frames"][0].asInt();
		}

		return ent;
	}


	/// in_pos is a global tile position
	void water_tile(const sf::Vector2f& in_pos, float life)
	{
		if (in_pos.x < 0.0f || in_pos.y < 0.0f) return;

		/// extract chunk xy
		int chunk_x = in_pos.x / CHUNK_LENGTH;
		int chunk_y = in_pos.y / CHUNK_LENGTH;

		/// extract tile xy (relative to chunk)
		int tile_x = fmod(in_pos.x, CHUNK_LENGTH);
		int tile_y = fmod(in_pos.y, CHUNK_LENGTH);

		_map[chunk_x][chunk_y]._tiles[tile_x+(tile_y*CHUNK_LENGTH) ].water(life);

	}

	/// extract chunk address from tile
	static sf::Vector2f get_chunk_address(sf::Vector2f in_pos )
	{
		in_pos.x = floor(in_pos.x);
		in_pos.y = floor(in_pos.y);
		in_pos.x = int(in_pos.x / CHUNK_LENGTH);
		in_pos.y = int(in_pos.y / CHUNK_LENGTH);
		return in_pos;
	}

	static sf::Vector2f screen_to_tile(sf::Vector2f world_in)
	{
		sf::Vector2f pos;

		pos.x = world_in.x - world_in.y;
		pos.y = world_in.x + world_in.y;

		pos.x /= TILE_WIDTH *0.5;
		pos.y /= TILE_HEIGHT*0.5;

		pos.x = int(pos.x);
		pos.y = int(pos.y);

		return pos;
	}

	/// get isometric position from chunk-tile position
	static sf::Vector2f tile_to_screen(const sf::Vector2f& in)
	{
		sf::Vector2f pos;

		pos.x = in.x - in.y;
		pos.y = in.x + in.y;

		pos.x *= TILE_WIDTH * 0.5;
		pos.y *= TILE_HEIGHT* 0.5;

		return pos;
	}

	static float tile_to_depth(const sf::Vector2f& in)
	{
		sf::Vector2f pos;

		pos.y = in.x + in.y;
		//pos.y *= TILE_HEIGHT* 0.5;

		return pos.y;
	}

	/*int x_begin = _player->tile_position.x - 16;
	int y_begin = _player->tile_position.y - 16;
	int x_end   = _player->tile_position.x + 16;
	int y_end   = _player->tile_position.y + 16;
	for (int x=x_begin; x<x_end; ++x)
	{
		for (int y=y_begin; y<y_end; ++y)
		{
			_world.draw_floor_tile(window, sf::Vector2f(x,y));
		}
	}*/

	void draw(sf::RenderWindow& window)
	{
		int offset_x = _player->tile_position.x-2;
		int offset_y = _player->tile_position.y-2;

		for (int i=0; i<chunk_draw_order.size(); ++i)
		{
			int x = offset_x + (chunk_draw_order[i] % 5);
			int y = offset_y + (chunk_draw_order[i] / 5);
			draw_floor_tile(window, sf::Vector2f(x,y));
		}
	}

	void draw_floor_tile(sf::RenderWindow& window, sf::Vector2f pos)
	{
		/// which chunk
		/// which tile

		if (pos.x < 0.0f || pos.y < 0.0f) return;

		/// extract chunk xy
		int chunk_x = pos.x / CHUNK_LENGTH;
		int chunk_y = pos.y / CHUNK_LENGTH;

		/// extract tile xy (relative to chunk)
		int tile_x = fmod(pos.x, CHUNK_LENGTH);
		int tile_y = fmod(pos.y, CHUNK_LENGTH);

		Chunk& chunk = _map[chunk_x][chunk_y];

		Tile& tile = chunk._tiles[tile_x+(tile_y*CHUNK_LENGTH) ];

		//int x = tile_draw_order[i] % CHUNK_LENGTH;
		//int y = tile_draw_order[i] / CHUNK_LENGTH;
		//sf::Vector2f screen_pos = tile_to_screen(sf::Vector2f( x + x_offset, y + y_offset));

		pos.x = int(pos.x);
		pos.y = int(pos.y);

		sf::Vector2f screen_pos = tile_to_screen(pos);

		tile_sprite->setPosition(screen_pos);
		tile_sprite->setTextureRect(tile.rect());
		window.draw(*tile_sprite);

	}

	/*
	void draw_floor_tiles(sf::RenderWindow& window,
		float depth_layer = -1, bool _behind = false)
	{
		int offset_x = chunk_address.x-2;
		int offset_y = chunk_address.y-2;
		offset_x = offset_x < 0.0f ? 0.0f : offset_x;
		offset_y = offset_y < 0.0f ? 0.0f : offset_y;

		//int target_chunk_depth = int(depth_layer/7)*7;

		for (int i=0; i<chunk_draw_order.size(); ++i)
		{
			int x = offset_x + (chunk_draw_order[i] % 5);
			int y = offset_y + (chunk_draw_order[i] / 5);

			int depth_chunk = (x+y)*CHUNK_LENGTH;
			int target_tile_depth = depth_layer - depth_chunk;

			if ((target_tile_depth <= chunk_draw_depths[i]) != _behind)
			{

				//cout << target_tile_depth << endl;
				Chunk& chunk = _map[x][y];
				draw_chunk(window, chunk, x*CHUNK_LENGTH, y*CHUNK_LENGTH, target_tile_depth );
			}
		}
	}*/

	void draw_chunk(sf::RenderWindow& window, Chunk& chunk, int x_offset=0, int y_offset=0,
		float depth_layer=-1, bool _behind = false)
	{
		for (int i=0; i < CHUNK_AREA; ++i)
		{
			//if (depth_layer == -1 || tile_draw_depths[i] == depth_layer)
			if ((depth_layer <= tile_draw_depths[i]) != _behind)
			{
				int x = tile_draw_order[i] % CHUNK_LENGTH;
				int y = tile_draw_order[i] / CHUNK_LENGTH;

				sf::Vector2f iso_pos = tile_to_screen(sf::Vector2f( x + x_offset, y + y_offset));

				tile_sprite->setPosition(iso_pos);

				tile_sprite->setTextureRect( chunk._tiles[ tile_draw_order[i] ].rect());

				window.draw(*tile_sprite);
			}
		}

		/*
		for (int x=0; x < CHUNK_LENGTH; ++x)
		{
			for (int y=0; y < CHUNK_LENGTH; ++y)
			{
				//tile_sprite->setPosition(x*TILE_WIDTH*0.5, y*TILE_HEIGHT*0.5);

				/// iso-ize position

				sf::Vector2f iso_pos = tile_to_screen(sf::Vector2f( x + x_offset, y + y_offset));

				tile_sprite->setPosition(iso_pos);

				tile_sprite->setTextureRect( chunk._tiles[x + (y*CHUNK_LENGTH) ].rect());

				window.draw(*tile_sprite);
			}
		}*/
	};

};

#endif
