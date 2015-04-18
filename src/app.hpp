#ifndef APP_HPP_
#define APP_HPP_

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#include "SFML/Graphics.hpp"
#include "SFML/Window.hpp"
#include "SFML/System.hpp"
#include "SFML/Audio.hpp"

//#include <Box2D/Box2D.h>

#include <jsoncpp/json/json.h>
#include "loader.hpp"
#include "mytime.hpp"

#include "world.hpp"

using namespace std;

const vector<string> keys =
	{"A","B","C","D","E","F","G","H","I","J","K","L","M","N",
	"O","P","Q","R","S","T","U","V","W","X","Y","Z","Num0","Num1","Num2",
	"Num3","Num4","Num5","Num6","Num7","Num8","Num9","Escape","LControl",
	"LShift","LAlt","LSystem","RControl","RShift","RAlt","RSystem","Menu",
	"LBracket","RBracket","SemiColon","Comma","Period","Quote","Slash",
	"BackSlash","Tilde","Equal","Dash","Space","Return","BackSpace","Tab",
	"PageUp","PageDown","End","Home","Insert","Delete","Add","Subtract",
	"Multiply","Divide","Left","Right","Up","Down","Numpad0","Numpad1",
	"Numpad2","Numpad3","Numpad4","Numpad5","Numpad6","Numpad7","Numpad8",
	"Numpad9","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11",
	"F12","F13","F14","F15","Pause","MouseLeft","MouseRight",
	"MouseMiddle","MouseXButton1","MouseXButton2"};

class App
{
public:

	App();

	int _execute();
  void _config(const string& filename);

private:

	void _load();
	void _events();
  void _event(sf::Event&);
  void _update();
  void _render();
  void _quit();

	enum { _BEGIN, _END, _UPDATE, _RENDER };
	short base_state = _BEGIN;

  double refresh_rate = 50.0f;
  unsigned int window_width  = 1280;
  unsigned int window_height = 960;
  double zoom_factor = 0.25f;

  unsigned int window_style =
		sf::Style::Titlebar | sf::Style::Close;

  /// sf::Style::Fullscreen;

	std::string window_title = "ludum dare 32 - shrapx";
  sf::RenderWindow window;
  sf::View view;
  MyTime<float> base_time;

	unordered_map<unsigned short,string> key_cmd;
	unordered_map<string,bool> cmd_state;

	// app
  void key_input(sf::Event& event, bool value);
	void mouse_input(sf::Event& event, bool value);
	float mouse_x, mouse_y;
	bool mouse_moved = false;
	World _world;
};



#endif
