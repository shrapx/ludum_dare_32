#include <iostream>
#include "app.hpp"

using namespace std;
using namespace Json;

#ifdef _WIN64
#define M_PI 3.1415926535f
#endif

App::App()
{
	base_time.set_refresh_rate(refresh_rate);

	window.create(sf::VideoMode(window_width,window_height),
								window_title,
								window_style);

	window.setKeyRepeatEnabled(false);
}

void App::_config(const string& filename = "data/config.json")
{
	// window
	window.setTitle(window_title);
	view = window.getDefaultView();
	view.zoom(zoom_factor);
	//view.move(0,0.5);
	view.setCenter(0,0);

	// load config
	Value config_json = Loader::load(filename);

	// keyboard mouse input
	Value binds = config_json["binds"];

	for(ValueIterator it=binds.begin(); it != binds.end(); it++ )
	{
		string key = it.key().asString();
		string cmd = (*it).asString();
		for (unsigned short i=0; i<keys.size(); ++i)
		{
			string ikey = keys.at(i);
			if (ikey == key )
			{
				/*cout << " key found " << key << endl;*/
				key_cmd[i] = cmd;
				cmd_state[cmd] = 0;
				break;
			}
		}
	}


	if (config_json.isMember("music_volume"))
	{
		music_volume = config_json["music_volume"].asFloat();
	}

	if (config_json.isMember("effect_volume"))
	{
		effect_volume = config_json["effect_volume"].asFloat();
	}

	Json::Value jmusic = config_json["music"];
	/*
	if (jmusic.isMember("robot"))
	{
		auto m = make_shared<sf::Music>();
		if ( m->openFromFile(jmusic["robot"].asString()))
		{
			m->play();
			m->setLoop(true);
			m->setVolume(0);
			music["robot"] = m;
		}
	}

	if (jmusic.isMember("nature"))
	{
		auto m = make_shared<sf::Music>();
		if ( m->openFromFile(jmusic["nature"].asString()))
		{
			m->play();
			m->setLoop(true);
			m->setVolume(0);
			music["nature"] = m;
		}
	}*/

	for ( string& name : jmusic.getMemberNames())
	{
		auto m = make_shared<sf::Music>();
		if ( m->openFromFile(jmusic[name].asString()))
		{
			m->play();
			m->setLoop(true);
			m->setVolume(0);
			music[name] = m;
		}
	}

	_world.load(config_json);
}

void App::_load()
{
	view.setCenter(World::tile_to_screen(_world._player->tile_position));
}

int App::_execute()
{
	_load();

	for (;;)
	{


		if ( base_state == _END )
		{
			_quit();
			window.close();
			return 0;
		}

		if ( base_time.update() )
		{
			_events();
			_update();
		}
		else
		{
			_render();
		}
	}
}

void App::_events()
{
	// reset trigger style values
	mouse_moved = false;

	sf::Event event;
	while(window.pollEvent(event))
	{
		switch (event.type)
		{
			case sf::Event::Closed:
			{
				base_state = _END;
				return;
			}
			default: _event(event);
		}
	}
}

void App::_event(sf::Event& event)
{
	switch (event.type)
	{
		case sf::Event::KeyPressed:
		{
			key_input(event, true);
			break;
		}
		case sf::Event::KeyReleased:
		{
			key_input(event, false);
			break;
		}
		case sf::Event::MouseButtonPressed:
		{
			mouse_input(event, true);
			break;
		}
		case sf::Event::MouseButtonReleased:
		{
			mouse_input(event, false);
			break;
		}
		case sf::Event::MouseMoved:
		{
			mouse_moved = true;
			mouse.x = event.mouseMove.x;
			mouse.y = event.mouseMove.y;

			break;
		}
		default: ;
	}
}

void App::key_input(sf::Event& event, bool value)
{
	auto it = key_cmd.find(event.key.code);
	if ( it != key_cmd.end() )
	{
		const string& cmd = it->second;
		cmd_state[ cmd ] = value;
		/*cout << "key id: " << it->first
			<< " cmd name: " << cmd
			<< " val: " << value << endl;*/
	}
}

void App::mouse_input(sf::Event& event, bool value)
{
	uint16_t mbutton =
		sf::Keyboard::KeyCount+event.mouseButton.button;

	auto it = key_cmd.find(mbutton);
	if ( it != key_cmd.end() )
	{
		const string& cmd = it->second;
		cmd_state[ cmd ] = value;
		/*cout << "mouse id: " << it->first
		<< " cmd name: " << cmd
		<< " val: " << value << endl;*/
	}
}

sf::Vector2f normalize(sf::Vector2f vec)
{
	float length = vec.x*vec.x + vec.y*vec.y;
	if (length != 0)
	{
		length = sqrt(1.0f/length);
		vec.x *= length;
		vec.y *= length;
	}
	return vec;
}

float get_angle(sf::Vector2f vec)
{
	if (vec.y == 0)
			return vec.x < 0 ? 180 : 0;
	else if (vec.x == 0)
			return vec.y < 0 ? 270 : 90;

	if ( vec.y > 0)
			if (vec.x > 0)
					return atan(vec.y/vec.x) * 180.f/3.14159f;
			else
					return 180.0-atan(vec.y/-vec.x) * 180.f/3.14159f;
	else
			if (vec.x > 0)
					return 360.0-atan(-vec.y/vec.x) * 180.f/3.14159f;
			else
					return 180.0+atan(-vec.y/-vec.x) * 180.f/3.14159f;
}

void App::_update()
{
	float frame_length = base_time.get_step();

	if (music_volume > 0.0f)
	{
		/// music mashup
		float robotvol  = (0.66f-_world.life_compare) * music_volume;// * 0.333f;
		float naturevol =  (_world.life_compare-0.05f) * music_volume * 1.5f;// * 1.5f;

		robotvol = robotvol > music_volume ? music_volume : robotvol;
		robotvol = robotvol < 0.0f ? 0.0f: robotvol;
		naturevol = naturevol > music_volume ? music_volume : naturevol;
		naturevol = naturevol < 0.0f ? 0.0f: naturevol;

		music["robot"]->setVolume(robotvol);
		music["nature"]->setVolume(naturevol);
	}
	//cout << "robot" << robotvol << endl; cout << "nature" << naturevol << endl;

	if (_world._player)
	{


		int updown = cmd_state["down"] - cmd_state["up"];
		int leftright = cmd_state["right"] - cmd_state["left"];

		if (updown || leftright)
		{
			/// prepare to update players position

			sf::Vector2f tile_pos = _world._player->tile_position;

			sf::Vector2f newdir(updown + leftright, updown - leftright);

			/// neighbouring tiles
			for (auto& offset : _world.lazy_offset)
			{
				Tile& tile = _world.get_tile(tile_pos + offset);
				if(tile.type > 7)
				{

					newdir += offset * -2.0f;
					//tile_pos += offset * -0.05f;
				}
			}


			_world._player->tile_normal = _world._player->tile_normal*0.9f + normalize(newdir)*0.1f ;

			_world._player->tile_angle = get_angle(_world._player->tile_normal);
			//_world._player->tile_angle = get_angle(newdir);

			tile_pos += _world._player->tile_normal*0.1f;

			//_world._player->tile_position = tile_pos;

			_world.chunk_address = World::get_chunk_address(tile_pos);

			_world.set_entity_position("player", tile_pos);

			view.setCenter(World::tile_to_screen(tile_pos));

			/// marks the tile the player is attributed to
			/*sf::Vector2f unit_pos;
			unit_pos.x = floor(tile_pos.x);
			unit_pos.y = floor(tile_pos.y);
			_world.set_entity_position("tile_target", unit_pos);*/

			_world._player->walk_anim_marker += base_time.get_step()*4.0f;
		}

		if (cmd_state["fire"])
		{
			/// how much water to spray
			if (_world._player->water > 0.0f)
			{
				float water_amt = _world._player->water > 0.25f ? 0.25f : _world._player->water;
				_world._player->water -= water_amt;

				/// player shoot water
				sf::Vector2f water_aim = _world._player->tile_position + _world._player->tile_normal*6.0f;

				_world.set_entity_position("target", water_aim);

				/// marks the tile that is targetted for watering
				sf::Vector2f tile_pos;
				tile_pos.x = floor(water_aim.x);
				tile_pos.y = floor(water_aim.y);
				_world.set_entity_position("tile_target", tile_pos);

				_world.water_area(water_aim, water_amt);

				for (int i = 0; i < 10; ++i)
				{
					_world.shoot_water_effect();
				}

				if (effect_volume > 0.0f)
				{
					if (_world.water_level > 0.75f)
					{
						music["water1"]->setVolume(0);
						music["water2"]->setVolume(0);
						music["water3"]->setVolume(0);
						music["water4"]->setVolume(effect_volume);

					}
					else if (_world.water_level > 0.5f)
					{
						music["water1"]->setVolume(0);
						music["water2"]->setVolume(0);
						music["water3"]->setVolume(effect_volume);
						music["water4"]->setVolume(0);
					}
					else if (_world.water_level > 0.25f)
					{
						music["water1"]->setVolume(0);
						music["water2"]->setVolume(effect_volume);
						music["water3"]->setVolume(0);
						music["water4"]->setVolume(0);
					}
					else
					{
						music["water1"]->setVolume(effect_volume);
						music["water2"]->setVolume(0);
						music["water3"]->setVolume(0);
						music["water4"]->setVolume(0);
					}
				}
			}
			else
			{
				if (effect_volume > 0.0f)
				{
					music["water1"]->setVolume(0);
					music["water2"]->setVolume(0);
					music["water3"]->setVolume(0);
					music["water4"]->setVolume(0);
					music["watercharge"]->setVolume(0);
				}
			}
		}
		else if ( cmd_state["charge"] && _world._player->water < _world._player->water_max )
		{
			/// todo try any water holding entity
			for (auto it : _world._entities)
			{
				auto& ent = it.second;

				if (ent == _world._player) continue;

				sf::Vector2f distsqr = ent->tile_position - _world._player->tile_position;

				if ( abs(distsqr.x) < 3 && abs(distsqr.y) < 3)
				{
					if (ent->water > 0.0f)
					{
						if (effect_volume > 0.0f)
						{
							music["water1"]->setVolume(0);
							music["water2"]->setVolume(0);
							music["water3"]->setVolume(0);
							music["water4"]->setVolume(0);
							music["watercharge"]->setVolume(effect_volume);
						}
						/// charging water
						float water_amt =	ent->water > 0.5f ? 0.5f : ent->water;
						_world._player->water += water_amt;
						ent->water -= water_amt;
						if (_world._player->water > _world._player->water_max)
						{
							_world._player->water = _world._player->water_max;
						}
						break;
					}
				}
			}
		}
		else
		{
			if (music_volume > 0.0f)
			{
				music["water1"]->setVolume(0);
				music["water2"]->setVolume(0);
				music["water3"]->setVolume(0);
				music["water4"]->setVolume(0);
				music["watercharge"]->setVolume(0);
			}
		}


		float walk_anim = fmod(_world._player->walk_anim_marker, _world._player->anim_frames);
		int frame_x = int(walk_anim) * 32;

		float turn_anim = (_world._player->tile_angle/360.0f) * _world._player->rotate_frames;
		int frame_y =    int(turn_anim) * 32;

		auto frame_rect = sf::IntRect(frame_x, frame_y, 32, 32);

		_world._player->setTextureRect(frame_rect);

	}

	if (mouse_moved)
	{
		_world._entities["mouse"]->setPosition(mouse);
	}

	_world.update();
}

void App::_render()
{
	window.setView(view);
	//view = window.getDefaultView();
	//window.setView(view);

	window.clear(sf::Color::Black);


	//double ipo = base_time.get_interpolation();

	/// draw tiles

	//_world.draw_floor_tiles(window, World::tile_to_depth(_world._player->tile_position), true);


	_world.draw(window);

	auto ui_pos = view.getCenter();
	ui_pos.y += 64;
	_world.water_level_sprite->setPosition(ui_pos);
	window.draw( *(_world.water_level_sprite));

  //window.draw(text);
	window.display();
}

void App::_quit()
{

}

