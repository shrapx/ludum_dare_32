#include <iostream>
#include "app.hpp"

using namespace std;
using namespace Json;

#define M_PI 3.1415926535f

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

	_world.load(config_json);
}

void App::_load()
{

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

	if (_world._player)
	{
		sf::Vector2f pos = _world._player->tile_position;

		int updown = cmd_state["down"] - cmd_state["up"];
		int leftright = cmd_state["right"] - cmd_state["left"];


		pos.y += (updown * 0.05f) - (leftright * 0.025f);
		pos.x += (updown * 0.05f) + (leftright * 0.025f);

		sf::Vector2f vec(updown - leftright, updown + leftright);

		_world._player->tile_normal = normalize(vec);
		_world._player->tile_angle = get_angle(vec);

		_world._player->tile_position = pos;

		auto screenpos = World::tile_to_screen(pos);
		_world._player->setPosition(screenpos);
		view.setCenter(screenpos);

		/// get global unit of tile pos
		sf::Vector2f tile_pos;
		tile_pos.x = floor(pos.x);
		tile_pos.y = floor(pos.y);

		_world.chunk_address = World::get_chunk_address(pos);

		//cout << "depth: " << World::tile_to_depth(pos) << endl;

		if (cmd_state["fire"])
		{
			/// shoot water from players direction and position

			//_world._player->tile_angle;
			//_world._player->tile_normal;

			_world.water_tile(tile_pos, 0.01f);
		}

		_world._entities["target"]->setPosition(World::tile_to_screen(tile_pos) );


		float walk_anim = fmod(base_time.get_current()*10.f,_world._player->anim_frames);
		//cout << walk_anim << endl;
		int frame_x = int(walk_anim) * 32;

		float turn_anim = (_world._player->tile_angle/360.0f) * _world._player->rotate_frames;

		int frame_y =    int(turn_anim) * 32;

		auto frame_rect = sf::IntRect(frame_x, frame_y, 32, 32);

		_world._player->setTextureRect(frame_rect);

	}

	if (mouse_moved)
	{

		//_world._entities["target"]->setPosition( World::screen_to_world(mouse));

		_world._entities["mouse"]->setPosition(mouse);

		//_world._entities["mouse"]->setPosition(
		//	World::world_to_screen( World::screen_to_world(mouse) ));

		//view.setCenter(mouse*zoom_factor);

	}
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
	//_world.draw_wall_tiles(window);

	for (auto it : _world._entities)
	{
		window.draw( *(it.second) );
	}

  //window.draw(text);
	window.display();
}

void App::_quit()
{

}

