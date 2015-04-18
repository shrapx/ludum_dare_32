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
		_events();

		if ( base_state == _END )
		{
			_quit();
			window.close();
			return 0;
		}

		if ( base_time.update() )
		{
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
			mouse_x = event.mouseMove.x;
			mouse_y = event.mouseMove.y;
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

void App::_update()
{
	double frame_length = base_time.get_step();



}

void App::_render()
{
	window.setView(view);
	view = window.getDefaultView();

	window.clear(sf::Color::Black);


	//double ipo = base_time.get_interpolation();

	window.setView(view);


	/// draw tiles

	_world.draw(window);

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

