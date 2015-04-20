#include "app.hpp"
#include <string>

int main(int argc, char **argv) {

#ifdef _WIN64
	std::string path = "data\\config.json";
#else 
	std::string path = "data/config.json";
#endif

	App a;

	a._config( argc>1 ? argv[1] : path );

  return a._execute();
}
