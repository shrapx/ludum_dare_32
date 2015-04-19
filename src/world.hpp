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

#define COLOUR_LOWER 204
#define COLOUR_RANGE 51

struct Tile
{
	Tile()
	{
		type = rand() % 8;
		//life = (rand() % 10) * 0.1f;

	}

	sf::IntRect rect() const
	{
		int frame_y = type * TILE_HEIGHT * 2;
		int frame_x = int(life * (TILE_NUM-1)) * TILE_WIDTH * 2;

		return sf::IntRect(frame_x, frame_y, TILE_WIDTH * 2, TILE_HEIGHT * 2);
	}

	void update()
	{
		/// convert water store to growth
		if (water > 0.001f)
		{
			/// grow

			life += water*0.002f;
			water -= 0.002f;

			uint8_t range = 255-204;

			if (water < 0.001f)
			{
				life += water;
				water = 0.0f;

				colour = sf::Color(
					COLOUR_LOWER,
					COLOUR_LOWER,
					COLOUR_LOWER, 255);
			}
			else
			{
				colour = sf::Color(
					COLOUR_LOWER - (COLOUR_RANGE * water * 0.75f),
					COLOUR_LOWER - (COLOUR_RANGE * water * 0.75f),
					COLOUR_LOWER + (COLOUR_RANGE * water), 255);
			}
			/// max limit life;
			life = life > 1.0f ? 1.0f : life;
		}
	}

	void add_water(float val)
	{
		water += val;
		water = water > 1.0f ? 1.0f : water;
	}

	sf::Color colour = sf::Color::White;

	int biome_type = 0;
	int wall_type = 0;
	int floor_type = 2;
	int type = 2;
	float life = 0;
	float water = 0;
};

struct Chunk
{
	array<Tile, CHUNK_AREA> _tiles;
};

class World
{

public:

	array<float,9> water_distribution = {{0.25,0.5,0.25,0.5,1.0,0.5,0.25,0.5,0.25}};
	array<sf::Vector2f,9> lazy_offset = {{
		{-1,-1}, { 0,-1}, {+1,-1},
		{-1, 0}, { 0, 0}, {+1, 0},
		{-1,+1}, { 0,+1}, {+1,+1} }};
	// order of chunk drawing, and depth of each chunk
	array<int,25> chunk_draw_order = {{0,5,1,10,6,2,15,11,7,3,20,16,12,8,4,21,17,13,9,22,18,14,23,19,24}};
	array<int,25> chunk_draw_depths = {{0,1,1,2,2,2,3,3,3,3,4,4,4,4,4,5,5,5,5,6,6,6,7,7,8}};

	// order of tile drawing, and depth of each tile
	array<int,16> tile_draw_order_16 = {{0,4,1,8,5,2,12,9,6,3,13,10,7,14,11,15}};
	array<int,16> tile_draw_depths = {{0,1,1,2,2,2,3,3,3,3,4,4,4,5,5,6}};

	vector<int> tile_draw_order = World::get_iso_order(32);
	int tile_draw_order_square = 32;
	int tile_draw_order_halfsquare = 16;

	shared_ptr<EntityBase> _player;

	vector< shared_ptr<EntityBase>> water_drops;

	unordered_map<string, shared_ptr<EntityBase>> _entities;
	unordered_map<string, shared_ptr<sf::Texture>> _textures;

	shared_ptr<sf::Sprite> tile_sprite;

	unordered_map<int, unordered_map<int, Chunk>> _map;

	sf::Vector2f chunk_address;

	float life_compare = 0.0f;

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

		for ( auto maps : data["maps"] )
		{
			auto origin = sf::Vector2f(maps["origin"][0].asInt(), maps["origin"][1].asInt());
			auto dimensions = sf::Vector2f(maps["dimensions"][0].asInt(), maps["dimensions"][1].asInt());

			int area = dimensions.x * dimensions.y;

			for (int i=0; i < area; ++i)
			{
				int x = i % int(dimensions.x);
				int y = i / int(dimensions.y);

				auto tile_pos = sf::Vector2f(origin.x+x,origin.y+y);

				if (tile_pos.x < 0.0f || tile_pos.y < 0.0f) continue;

				Tile& tile = get_tile(tile_pos);
				tile.type = maps["array"][i].asInt();
			}
		}

	};

	shared_ptr<EntityBase> create_entity(Json::Value& data)
	{
		shared_ptr<Entity> ent = make_shared<Entity>();

		if (data.isMember("origin"))
			ent->setOrigin( data["origin"][0].asFloat(), data["origin"][1].asFloat() );

		if (data.isMember("pos"))
		{
			auto tile_pos = sf::Vector2f(data["pos"][0].asFloat(), data["pos"][1].asFloat());
			ent->setPosition(World::tile_to_screen(tile_pos));
			ent->tile_position = tile_pos;
			ent->tile_unit_pos = sf::Vector2f(floor(tile_pos.x),floor(tile_pos.y));
		}

		if (data.isMember("water"))
		{
			ent->water = data["water"].asFloat();
		}

		if (data.isMember("rot"))
			ent->tile_angle = data["rot"].asFloat();

		if (data.isMember("frames"))
		{
			ent->rotate_frames = data["frames"][0].asInt();
			ent->anim_frames = data["frames"][0].asInt();
		}

		return ent;
	}

	void update()
	{
		for (auto& xpair : _map)
		{
			for(auto& ypair : xpair.second)
			{
				Chunk& chunk = ypair.second;
				for(auto& tile : chunk._tiles)
				{
					tile.update();
				}
			}
		}

		//for (auto& ent : water_drops)

		for (auto it = water_drops.begin(); it != water_drops.end(); )
		{
			auto ent = (*it);
			if (!ent || ent->water_age > 1.0f)
			{
				/// explicit next iteration
				it = water_drops.erase(it);
			}
			else
			{


				sf::Vector2f tile_pos = ent->tile_position;

				tile_pos += ent->water_vector;

				ent->water_vector *= 0.8f;
				ent->water_y *= 0.87f;

				ent->tile_position = tile_pos;
				//ent->tile_unit_pos = sf::Vector2f(floor(tile_pos.x),floor(tile_pos.y));

				auto screen_pos = World::tile_to_screen(tile_pos);
				screen_pos.y -= ent->water_y;

				ent->setPosition(screen_pos);

				ent->setColor( sf::Color(255,255,255, 255-(ent->water_age*255)) );
				ent->water_age += 0.02f;

				/// explicit next iteration
				++it;
			}
		}
	}

	void shoot_water_effect()
	{

		float jit1 = (rand() % 100) * 0.01f;
		float jit2 = (rand() % 100) * 0.01f;

		sf::Vector2f jitter = sf::Vector2f(jit1,jit2);

		shared_ptr<EntityBase> ent = make_shared<Entity>();
		water_drops.push_back(ent);

		auto it = _textures.find("water");
		if (it != _textures.end()) ent->setTexture( *(it->second) );
		ent->setOrigin(1,1);
		/// set position

		ent->tile_position = _player->tile_position + _player->tile_normal*0.1f;

		//ent->setPosition(World::tile_to_screen(_player->tile_position));

		/// direction of water shooting

		ent->tile_normal = _player->tile_normal + (jitter*0.333f);
		ent->water_vector = ent->tile_normal;

		ent->water_y = 16.0f;

		int pixelx = rand() % 4;
		int pixely = rand() % 4;
		ent->setTextureRect( sf::IntRect(pixelx, pixely, 1, 1));
	}

	/// in_pos is a global tile position
	void water_area(const sf::Vector2f& in_pos, float life)
	{
		for (int i=0; i < 9; ++i)
		{
			water_tile( in_pos + lazy_offset[i], water_distribution[i]*life);
		}
	}

	void water_tile(const sf::Vector2f& in_pos, float life)
	{
		if (in_pos.x < 0.0f || in_pos.y < 0.0f) return;

		/// extract chunk xy
		int chunk_x = in_pos.x / CHUNK_LENGTH;
		int chunk_y = in_pos.y / CHUNK_LENGTH;

		/// extract tile xy (relative to chunk)
		int tile_x = fmod(in_pos.x, CHUNK_LENGTH);
		int tile_y = fmod(in_pos.y, CHUNK_LENGTH);

		_map[chunk_x][chunk_y]._tiles[tile_x+(tile_y*CHUNK_LENGTH) ].add_water(life);

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

		return pos.y;
	}

	void set_entity_position(const string& name, const sf::Vector2f& tile_pos)
	{
		/// log the entities tile position
		_entities[name]->tile_position = tile_pos;
		_entities[name]->tile_unit_pos = sf::Vector2f(int(tile_pos.x),int(tile_pos.y));
		_entities[name]->setPosition(World::tile_to_screen(tile_pos));
	}

	void draw(sf::RenderWindow& window)
	{
		int offset_x = _player->tile_position.x-tile_draw_order_halfsquare;
		int offset_y = _player->tile_position.y-tile_draw_order_halfsquare;

		/// tally robot vs nature here as its iterating all
		life_compare = 0.0f;
		int num = 0;
		for (int i=0; i<tile_draw_order.size(); ++i)
		{
			int x = tile_draw_order[i] % tile_draw_order_square;
			int y = tile_draw_order[i] / tile_draw_order_square;

			auto tile_pos = sf::Vector2f(offset_x+x,offset_y+y);

			if (tile_pos.x < 0.0f || tile_pos.y < 0.0f) continue;

			num++;

			Tile& tile = get_tile(tile_pos);
			life_compare += tile.life;
			sf::Vector2f screen_pos = tile_to_screen(tile_pos);

			draw_floor_tile(window, tile, screen_pos);

			for (auto it : _entities)
			{
				if (it.second->tile_unit_pos == tile_pos ) //sf::Vector2f(tile_pos.x,tile_pos.y-1)
				{
					window.draw( *it.second );
				}
			}

			//draw_wall_tile(window, tile, screen_pos);
		}

		life_compare /= float(num);

		/*for (auto it : _entities)
		{
			window.draw( *(it.second) );
		}*/

		for (auto it : water_drops)
		{
			window.draw( *it );
		}
	}

	Tile& get_tile(const sf::Vector2f& pos)
	{
		//assert(pos.x >= 0.0f || pos.y >= 0.0f);

		/// extract chunk xy
		int chunk_x = pos.x / CHUNK_LENGTH;
		int chunk_y = pos.y / CHUNK_LENGTH;

		/// extract tile xy (relative to chunk)
		int tile_x = fmod(pos.x, CHUNK_LENGTH);
		int tile_y = fmod(pos.y, CHUNK_LENGTH);

		Chunk& chunk = _map[chunk_x][chunk_y];

		return chunk._tiles[tile_x+(tile_y*CHUNK_LENGTH) ];
	}

	void draw_floor_tile(sf::RenderWindow& window, Tile& tile, const sf::Vector2f& screen_pos)
	{
		//Tile& tile = get_tile(pos);
		//sf::Vector2f screen_pos = tile_to_screen(sf::Vector2f(int(pos.x),int(pos.y)));

		tile_sprite->setColor(tile.colour);
		tile_sprite->setPosition(screen_pos);
		tile_sprite->setTextureRect(tile.rect());

		window.draw(*tile_sprite);
	}

	void draw_wall_tile(sf::RenderWindow& window, Tile& tile, const sf::Vector2f& screen_pos)
	{
		//Tile& tile = get_tile(pos);
		//sf::Vector2f screen_pos = tile_to_screen(sf::Vector2f(int(pos.x),int(pos.y)));

		tile_sprite->setColor(tile.colour);
		tile_sprite->setPosition(screen_pos);
		tile_sprite->setTextureRect(tile.rect());

		window.draw(*tile_sprite);
	}

	static vector<int> get_iso_order(int square)
	{
		int area = square*square;
		int depth = 0;
		int num_last_depth = 0;
		int pos = 0;
		vector<int> arr;

		while (depth < (square*2))
		{
			if (pos < square)
			{
				num_last_depth = (depth * square);
				depth += 1;
				pos = num_last_depth;
			}
			else
			{
				pos = pos - square + 1;
			}

			if (pos < area)
			{
				bool new_val = true;
				for (int a : arr)
				{
					if (a == pos) new_val = false;
				}
				if (new_val)
				{
					arr.push_back(pos);
				}
			}
		}
		return arr;
	}
};

#endif
